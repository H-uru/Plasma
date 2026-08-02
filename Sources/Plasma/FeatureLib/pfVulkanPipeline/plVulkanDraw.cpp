/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

// The draw path, ported from plMetalPipeline.cpp. Materials are decomposed into
// fixed-function or programmable passes, with piggybacks, projections, aux
// spans, and shadows layered around the main geometry draw.

#include "plVulkanPipeline.h"

#include "plVulkanMaterialShaderRef.h"
#include "plVulkanPlateManager.h"

#include "hsGMatState.h"

#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plSpanTypes.h"
#include "plGLight/plLightInfo.h"
#include "plPipeline/plFogEnvironment.h"
#include "plPipeDebugFlags.h"
#include "plPipeline/plPlates.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/plShader.h"

#include <string_theory/format>

#include <algorithm>
#include <cstring>

/*** Transforms **************************************************************/

// Ported from plMetalPipeline::ISetupTransforms (plMetalPipeline.cpp:1157-1185).
void plVulkanPipeline::ISetupTransforms(plDrawableSpans* drawable, const plSpan& span,
                                        hsMatrix44& lastL2W)
{
    if (span.fNumMatrices <= 1) {
        if (lastL2W != span.fLocalToWorld) {
            ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);
            lastL2W = span.fLocalToWorld;
        } else {
            fView.fLocalToWorldLeftHanded = lastL2W.GetParity();
        }
    } else if (span.fNumMatrices == 2) {
        ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);
        lastL2W = span.fLocalToWorld;

        // Two matrices is the hardware path: the vertex's single weight blends
        // between the span transform and the next matrix in the palette.
        IToShaderMatrix(drawable->GetPaletteMatrix(span.fBaseMatrix + 1),
                        fCurrentUniforms.blendMatrix1);
    } else {
        // More than two matrices means ISoftwareVertexBlend already folded the
        // transform into the vertices, so the world matrix has to be identity.
        if (!lastL2W.IsIdentity()) {
            lastL2W.Reset();
            ISetLocalToWorld(lastL2W, lastL2W);
        }
    }

    // Snapshot the device's current matrices into the uniforms this draw will
    // upload. Metal keeps one mutable VertexUniforms and flushes it per draw;
    // the equivalent here is a scratch suballocation in IRenderBufferSpan.
    fCurrentUniforms.projectionMatrix = fDevice.fMatrixProj;
    fCurrentUniforms.localToWorldMatrix = fDevice.fMatrixL2W;
    fCurrentUniforms.worldToCameraMatrix = fDevice.fMatrixW2C;
    fCurrentUniforms.cameraToWorldMatrix = fDevice.fMatrixC2W;
}

/*** Dynamic buffers *********************************************************/

// Ported from plMetalPipeline::ICheckDynBuffers (plMetalPipeline.cpp:2055-2085).
bool plVulkanPipeline::ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group,
                                        const plSpan* spanBase)
{
    if (!(spanBase->fTypeMask & plSpan::kVertexSpan))
        return false;
    if (!(spanBase->fTypeMask & plSpan::kIcicleSpan))
        return false;

    const plIcicle* span = static_cast<const plIcicle*>(spanBase);

    plVulkanVertexBufferRef* vRef =
        static_cast<plVulkanVertexBufferRef*>(group->GetVertexBufferRef(span->fVBufferIdx));
    if (!vRef)
        return true;

    plVulkanIndexBufferRef* iRef =
        static_cast<plVulkanIndexBufferRef*>(group->GetIndexBufferRef(span->fIBufferIdx));
    if (!iRef)
        return true;

    if (vRef->Expired(fVtxRefTime))
        IRefreshDynVertices(group, vRef);

    if (iRef->IsDirty()) {
        fDevice.FillIndexBufferRef(iRef, group, span->fIBufferIdx);
        iRef->SetRebuiltSinceUsed(true);
    }

    return false;
}

// Ported from plMetalPipeline::IRefreshDynVertices (plMetalPipeline.cpp:2087-2137).
bool plVulkanPipeline::IRefreshDynVertices(plGBufferGroup* group, plVulkanVertexBufferRef* vRef)
{
    const size_t size = (group->GetVertBufferEnd(vRef->fIndex) -
                         group->GetVertBufferStart(vRef->fIndex)) * vRef->fVertexSize;
    if (!size)
        return false;

    uint8_t* src = vRef->fData;
    if (!src)
        src = group->GetVertBufferData(vRef->fIndex) +
              group->GetVertBufferStart(vRef->fIndex) * vRef->fVertexSize;

    vRef->PrepareForWrite();

    plVulkanBuffer buffer = vRef->GetBuffer();
    if (!buffer.IsValid() || buffer.fSize < size) {
        buffer = fDevice.CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                      ST::format("skinned vertex buffer {}", vRef->fIndex));
        if (!buffer.IsValid())
            return true;
        vRef->SetBuffer(&fDevice, buffer);
    }

    memcpy(buffer.fMapped, src, size);

    vRef->fRefTime = fVtxRefTime;
    vRef->SetDirty(false);

    return false;
}

/*** One span ****************************************************************/

void plVulkanPipeline::IRenderBufferSpan(const plIcicle& span, hsGDeviceRef* vb, hsGDeviceRef* ib,
                                         hsGMaterial* material, uint32_t vStart, uint32_t vLength,
                                         uint32_t iStart, uint32_t iLength)
{
    if (iLength == 0 || !material)
        return;

    VkCommandBuffer cmd = fDevice.CurrentCommandBuffer();
    if (cmd == VK_NULL_HANDLE)
        return;

    // Animated layers (including GUI checkbox checkmarks) only advance when
    // their material is visible. Evaluate before reading any layer state.
    // fTime is constant for the frame, and plLayerAnimation suppresses repeat
    // work for identical evaluation times.
    material->Eval(fTime, fFrame);

    plVulkanVertexBufferRef* vRef = static_cast<plVulkanVertexBufferRef*>(vb);
    plVulkanIndexBufferRef*  iRef = static_cast<plVulkanIndexBufferRef*>(ib);
    if (!vRef || !iRef || !vRef->GetBuffer().IsValid() || !iRef->GetBuffer().IsValid())
        return;

    plVulkanMaterialShaderRef* mRef =
        static_cast<plVulkanMaterialShaderRef*>(material->GetDeviceRef());
    if (!mRef) {
        mRef = new plVulkanMaterialShaderRef(material, this);
        material->SetDeviceRef(mRef);
        mRef->UnRef();
    }
    if (!mRef->IsLinked())
        mRef->Link(&fMaterialRefList);

    mRef->CheckMaterialRef();

    // Everything below is one span's worth of work, named after the material a
    // capture can then match to the scene. Same grouping as
    // plMetalPipeline::IRenderSpan (:1335).
    plVulkanDebugLabel spanLabel(fDevice, cmd, material->GetKeyName());

    // Turn on this span's lights and turn off the rest. Done once for the span,
    // not per pass, because every pass of a span is lit the same.
    ISelectLights(&span, false);

    //
    // Geometry. Bound once; the pass loop only changes state above it.
    //
    const VkBuffer vertexBuffer = vRef->GetBuffer().fBuffer;
    if (fState.fVertexBuffer != vertexBuffer) {
        fState.fVertexBuffer = vertexBuffer;
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &offset);
    }

    const VkBuffer indexBuffer = iRef->GetBuffer().fBuffer;
    if (fState.fIndexBuffer != indexBuffer) {
        fState.fIndexBuffer = indexBuffer;
        vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }

    IPushPiggyBacks(material);
    hsRefCnt_SafeAssign(fCurrMaterial, material);

    for (size_t pass = 0; pass < mRef->GetNumPasses(); pass++) {
        if (IHandleMaterialPass(cmd, material, pass, &span, vRef)) {
            // Metal passes a byte offset into the index buffer; Vulkan's
            // firstIndex is in index units. Indices are absolute into the
            // group's vertex buffer, so vertexOffset stays zero.
            vkCmdDrawIndexed(cmd, iLength, 1, iStart, 0, 0);
        }

        // Projections light themselves, so the span's lighting is set aside and
        // put back rather than recomputed for the next pass.
        SaveCurrentLightSources();
        ISelectLights(&span, true);

        if (!fProjEach.empty() && !(fView.fRenderState & kRenderNoProjection))
            IRenderProjectionEach(cmd, material, pass, span, vRef, iStart, iLength);

        RestoreCurrentLightSources();

        if (IsDebugFlagSet(plPipeDbg::kFlagNoUpperLayers))
            break;
    }

    IPopPiggyBacks();

    if (!fProjAll.empty() && !(fView.fRenderState & kRenderNoProjection))
        IRenderProjections(cmd, vRef, iStart, iLength);

    // Aux geometry -- decals, ripples, footprints -- draws right after its span,
    // and rebinds both buffers, so the span's have to go back afterwards.
    if (span.GetNumAuxSpans()) {
        IRenderAuxSpans(cmd, span);

        fState.fVertexBuffer = vertexBuffer;
        const VkDeviceSize zero = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &zero);

        fState.fIndexBuffer = indexBuffer;
        vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }

    // Shadows go on after the span is fully drawn, because they darken what is
    // already in the frame buffer rather than contributing to it.
    if (!fShadows.empty())
        IRenderShadowsOntoSpan(cmd, &span, material, vRef, iStart, iLength);
}

/**
 * Sets up one pass of a material and says whether it should be drawn.
 *
 * Ported from plMetalPipeline::IHandleMaterialPass (:1525-1712).
 */
bool plVulkanPipeline::IHandleMaterialPass(VkCommandBuffer cmd, hsGMaterial* material, size_t pass,
                                           const plSpan* span, const plVulkanVertexBufferRef* vRef,
                                           bool allowShaders, bool overrideLiteModel)
{
    plVulkanMaterialShaderRef* mRef =
        static_cast<plVulkanMaterialShaderRef*>(material->GetDeviceRef());

    fCurrLayerIdx = mRef->GetPassIndex(pass);
    plLayerInterface* lay = material->GetLayer(fCurrLayerIdx);
    if (!lay)
        return false;

    hsGMatState state;
    state.Composite(lay->GetState(), fMatOverOn, fMatOverOff);

    //
    // Dynamic state that Metal keeps on the encoder.
    //
    IHandleZMode(cmd, state);
    ISetCullMode(cmd, state);

    // kZIncLayer nudges decals forward so they do not z-fight what they sit on.
    const bool incLayer = (state.fZFlags & hsGMatState::kZIncLayer) != 0;
    if (incLayer && fDevice.SupportsDepthBiasClamp()) {
        // Match Metal's setDepthBias(constant, slope, clamp). Vulkan orders the
        // last two arguments as clamp, slope.
        vkCmdSetDepthBias(cmd, 0.f, -0.00001f, -8.f);
    } else {
        // Without clamp support an -8 slope can pull a steep decal through
        // foreground geometry. Retain the conservative constant-bias fallback.
        vkCmdSetDepthBias(cmd, incLayer ? -8.f : 0.f, 0.f, 0.f);
    }

    //
    // Lighting is calculated from the layer as the pass will see it, which means
    // with the override layers pushed. The pass build below pushes and pops them
    // again for its own reads; this is the one that decides the material colors.
    //
    lay = IPushOverBaseLayer(lay);
    lay = IPushOverAllLayer(lay);

    ICalcLighting(lay, span, state);
    state.Composite(lay->GetState(), fMatOverOn, fMatOverOff);

    // A layer with no opacity is skipped rather than drawn, so that it cannot
    // contribute to the depth buffer -- some models exist only for physics and
    // would otherwise occlude what is behind them. Preshaded spans are exempt
    // because their alpha is vertex data, not the layer's.
    const bool invisible = (state.fBlendFlags & hsGMatState::kBlendAlpha) &&
                           lay->GetOpacity() <= 0.f &&
                           fCurrLightingMethod != plSpan::kLiteVtxPreshaded;

    lay = IPopOverAllLayer(lay);
    lay = IPopOverBaseLayer(lay);

    if (invisible)
        return false;

    //
    // The programmable path, when the layer carries a shader pair we implement.
    // A pair we do not have falls through to the combiner, which draws the
    // geometry with the wrong appearance rather than not at all.
    //
    if (allowShaders && lay->GetVertexShader() && lay->GetPixelShader()) {
        plShader* vShader = lay->GetVertexShader();
        plShader* pShader = lay->GetPixelShader();

        if (IShaderPairSupported(vShader->GetDecl()->GetID(), pShader->GetDecl()->GetID())) {
            fCurrLay = lay;
            fCurrNumLayers = mRef->GetPassLength(pass);

            uint32_t vertexConstOffset = 0;
            uint32_t fragmentConstOffset = 0;
            if (!ISetShaders(cmd, vRef, state, vShader, pShader,
                             &vertexConstOffset, &fragmentConstOffset)) {
                return false;
            }

            return IBindShaderPassResources(cmd, material, pass, span,
                                            vertexConstOffset, fragmentConstOffset);
        }
    }

    //
    // The pass itself.
    //
    size_t numActivePiggyBacks = 0;
    if (!(state.fMiscFlags & hsGMatState::kMiscBumpChans) &&
        !(state.fShadeFlags & hsGMatState::kShadeEmissive)) {
        // Tack the light map onto the last stage if we have one.
        numActivePiggyBacks = size_t(fActivePiggyBacks);
    }

    // Plasma pulls piggybacks from the rear first.
    std::vector<plLayerInterface*> subPiggybacks(fPiggyBackStack.end() - numActivePiggyBacks,
                                                 fPiggyBackStack.end());

    const bool hasOverrides = fOverBaseLayer != nullptr || fOverAllLayer != nullptr;

    plVulkanPassInfo info;
    if (!mRef->ResolvePass(pass, &subPiggybacks, hasOverrides, &info))
        return false;

    //
    // Pipeline state. The material half comes from the pass; the vertex layout
    // and render-pass halves are the span's.
    //
    plVulkanFillVertexKey(info.fKey, vRef);
    info.fKey.fColorFormat = fDevice.CurrentColorFormat();
    info.fKey.fDepthFormat = fDevice.CurrentDepthFormat();
    info.fKey.fSampleCount = fDevice.CurrentSampleCount();

    VkPipeline pipeline = fDevice.GetPipelineState(info.fKey);
    if (pipeline == VK_NULL_HANDLE)
        return false;

    if (fState.fPipeline != pipeline) {
        fState.fPipeline = pipeline;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    //
    // Uniforms. Both blocks come out of this frame's scratch ring and are bound
    // by dynamic offset, so the descriptor set itself never changes.
    //
    fCurrNumLayers = info.fNumLayers;
    fCurrentMaterial.alphaThreshold = info.fAlphaThreshold;

    if (overrideLiteModel) {
        fCurrentMaterial.ambientCol = { 1.f, 1.f, 1.f };
        fCurrentMaterial.diffuseSrc = 1;
        fCurrentMaterial.ambientSrc = 1;
        fCurrentMaterial.emissiveSrc = 0;
        fCurrentMaterial.specularSrc = 1;
    }

    for (uint32_t i = 0; i < info.fNumLayers; i++)
        fCurrentUniforms.uvTransforms[i] = info.fUVTransforms[i];

    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!vertexAlloc.IsValid() || !materialAlloc.IsValid())
        return false;

    memcpy(vertexAlloc.fMapped, &fCurrentUniforms, sizeof(VertexUniforms));
    memcpy(materialAlloc.fMapped, &fCurrentMaterial, sizeof(plMaterialLightingDescriptor));

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    VkDescriptorSet textureSet =
        fDevice.GetTextureDescriptorSet(info.fTextures, info.fClampFlags, info.fNumLayers,
                                        nullptr, info.fRenderTargets,
                                        material->GetKeyName());
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return false;

    // Set 0's layout is the same for every pipeline, so a draw has to name an
    // offset for bindings its own shaders never declare. The zeroed shadow block
    // stands in; nothing reads what it points at.
    const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, materialAlloc.fOffset,
                                         fLightBuffer.fOffset, fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

    return true;
}

/*** Dynamic state helpers ***************************************************/

// Match plDXPipeline::IHandleZMode (plDXPipeline.cpp:5782-5817). In
// particular, kZNoZRead means "always pass and still write", not "disable the
// depth attachment"; preserve that behavior for screen-space drawables too.
void plVulkanPipeline::IHandleZMode(VkCommandBuffer cmd, const hsGMatState& state)
{
    VkBool32 depthWrite = VK_TRUE;
    VkCompareOp compare = VK_COMPARE_OP_LESS_OR_EQUAL;

    switch (state.fZFlags & hsGMatState::kZMask) {
    case hsGMatState::kZClearZ:
    case hsGMatState::kZNoZRead:
    case hsGMatState::kZNoZRead | hsGMatState::kZClearZ:
        compare = VK_COMPARE_OP_ALWAYS;
        depthWrite = VK_TRUE;
        break;
    case hsGMatState::kZNoZWrite:
        compare = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthWrite = VK_FALSE;
        break;
    case hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite:
        compare = VK_COMPARE_OP_ALWAYS;
        depthWrite = VK_FALSE;
        break;
    case 0:
        break;
    case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite:
    case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead:
        hsAssert(false, "Illegal combination of Z Buffer modes (Clear but don't write)");
        return;
    }

    vkCmdSetDepthTestEnable(cmd, VK_TRUE);
    vkCmdSetDepthWriteEnable(cmd, depthWrite);
    vkCmdSetDepthCompareOp(cmd, compare);
}

/**
 * True when local-to-world combined with world-to-camera is left handed.
 *
 * Ported from plMetalPipeline::IIsViewLeftHanded (plMetalPipeline.cpp:3988).
 * The orthogonal term matters: an orthographic projection flips winding on its
 * own, independently of either matrix's parity.
 */
bool plVulkanPipeline::IIsViewLeftHanded()
{
    return fView.GetViewTransform().GetOrthogonal() ^
           (fView.fLocalToWorldLeftHanded ^ fView.fWorldToCamLeftHanded);
}

// Ported from plMetalPipeline::ISetCullMode (plMetalPipeline.cpp:3997-4010),
// folded together with the kMiscTwoSided check plMetalPipeline does separately
// in IHandleMaterialPass (:1544-1546).
void plVulkanPipeline::ISetCullMode(VkCommandBuffer cmd, const hsGMatState& state)
{
    VkCullModeFlags cull = VK_CULL_MODE_NONE;
    if (!(state.fMiscFlags & hsGMatState::kMiscTwoSided)) {
        cull = IIsViewLeftHanded() ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
    }

    vkCmdSetCullMode(cmd, cull);
}

/**
 * Fills the material uniform block: colors, where each comes from, and fog.
 *
 * Ported from plMetalPipeline::ICalcLighting (plMetalPipeline.cpp:2237-2410).
 * The *Src fields are 0/1 selectors the shader mixes with, which is how the
 * three kLiteMask models are expressed without three shader permutations.
 */
void plVulkanPipeline::ICalcLighting(const plLayerInterface* currLayer, const plSpan* currSpan,
                                    const hsGMatState& baseState)
{
    memset(&fCurrentMaterial, 0, sizeof(fCurrentMaterial));

    if (IsDebugFlagSet(plPipeDbg::kFlagAllBright)) {
        fCurrentMaterial.globalAmb = { 1.f, 1.f, 1.f, 1.f };
        fCurrentMaterial.ambientCol = { 1.f, 1.f, 1.f };
        fCurrentMaterial.diffuseCol = { 1.f, 1.f, 1.f, 1.f };
        fCurrentMaterial.emissiveCol = { 1.f, 1.f, 1.f };
        fCurrentMaterial.specularCol = { 1.f, 1.f, 1.f };
        fCurrentMaterial.ambientSrc = 1;
        fCurrentMaterial.diffuseSrc = 1;
        fCurrentMaterial.emissiveSrc = 1;
        fCurrentMaterial.specularSrc = 1;
        return;
    }

    hsGMatState state = baseState;

    uint32_t mode = currSpan ? (currSpan->fProps & plSpan::kLiteMask) : plSpan::kLiteMaterial;

    // Bump channels are lit as flat white material; the bump math supplies the
    // shading.
    if (state.fMiscFlags & hsGMatState::kMiscBumpChans) {
        mode = plSpan::kLiteMaterial;
        state.fShadeFlags |= hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite;
    }

    switch (mode) {
    case plSpan::kLiteMaterial:
        fCurrLightingMethod = plSpan::kLiteMaterial;
        if (state.fShadeFlags & hsGMatState::kShadeWhite) {
            fCurrentMaterial.globalAmb = { 1.f, 1.f, 1.f, 1.f };
            fCurrentMaterial.ambientCol = { 1.f, 1.f, 1.f };
        } else if (IsDebugFlagSet(plPipeDbg::kFlagNoPreShade)) {
            fCurrentMaterial.globalAmb = { 0.f, 0.f, 0.f, 1.f };
            fCurrentMaterial.ambientCol = { 0.f, 0.f, 0.f };
        } else {
            const hsColorRGBA amb = currLayer->GetPreshadeColor();
            fCurrentMaterial.globalAmb = { amb.r, amb.g, amb.b, 1.f };
            fCurrentMaterial.ambientCol = { amb.r, amb.g, amb.b };
        }

        {
            const hsColorRGBA dif = currLayer->GetRuntimeColor();
            fCurrentMaterial.diffuseCol = { dif.r, dif.g, dif.b, currLayer->GetOpacity() };

            const hsColorRGBA em = currLayer->GetAmbientColor();
            fCurrentMaterial.emissiveCol = { em.r, em.g, em.b };
        }

        if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
            const hsColorRGBA spec = currLayer->GetSpecularColor();
            fCurrentMaterial.specularCol = { spec.r, spec.g, spec.b };
        }

        fCurrentMaterial.diffuseSrc = 1;
        fCurrentMaterial.emissiveSrc = 1;
        fCurrentMaterial.specularSrc = 1;
        // kShadeNoShade takes the material ambient rather than accumulating light.
        fCurrentMaterial.ambientSrc = (state.fShadeFlags & hsGMatState::kShadeNoShade) ? 1 : 0;
        break;

    case plSpan::kLiteVtxPreshaded:
        fCurrLightingMethod = plSpan::kLiteVtxPreshaded;
        // Everything comes from the vertex color, which was baked at export.
        fCurrentMaterial.diffuseSrc = 0;
        fCurrentMaterial.ambientSrc = 1;
        fCurrentMaterial.specularSrc = 1;
        fCurrentMaterial.emissiveSrc =
            (state.fShadeFlags & hsGMatState::kShadeEmissive) ? 0 : 1;
        break;

    case plSpan::kLiteVtxNonPreshaded:
        fCurrLightingMethod = plSpan::kLiteVtxNonPreshaded;
        {
            const hsColorRGBA em = currLayer->GetAmbientColor();
            fCurrentMaterial.emissiveCol = { em.r, em.g, em.b };
        }

        if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
            const hsColorRGBA spec = currLayer->GetSpecularColor();
            fCurrentMaterial.specularCol = { spec.r, spec.g, spec.b };
        }

        {
            const hsColorRGBA amb = currLayer->GetPreshadeColor();
            fCurrentMaterial.globalAmb = { amb.r, amb.g, amb.b, amb.a };
        }

        fCurrentMaterial.ambientSrc = 0;
        fCurrentMaterial.diffuseSrc = 0;
        fCurrentMaterial.emissiveSrc = 1;
        fCurrentMaterial.specularSrc = 1;
        break;

    default:
        break;
    }

    fCurrentMaterial.invertAlpha =
        (state.fBlendFlags & hsGMatState::kBlendInvertVtxAlpha) ? 1 : 0;

    // alphaThreshold is not set here: it belongs to the pass, not the layer,
    // and IHandleMaterialPass writes it from plVulkanPassInfo.

    // The active light set for this span, gathered by ISelectLights.
    fCurrentMaterial.lightCount = uint32_t(std::min(fLights.size(), size_t(kMaxActiveLights)));
    for (uint32_t i = 0; i < fCurrentMaterial.lightCount; i++)
        fCurrentMaterial.activeLights[i] = fLights[i];

    //
    // Fog rides along on the lighting pass, as it does in Metal.
    //
    const plFogEnvironment* fog = currSpan
        ? (currSpan->fFogEnvironment ? currSpan->fFogEnvironment : &fView.GetDefaultFog())
        : nullptr;

    if (currLayer && (currLayer->GetShadeFlags() & hsGMatState::kShadeReallyNoFog) &&
        !(fMatOverOff.fShadeFlags & hsGMatState::kShadeReallyNoFog)) {
        fog = nullptr;
    }

    const uint8_t type = fog ? fog->GetType() : plFogEnvironment::kNoFog;
    hsColorRGBA color;

    switch (type) {
    case plFogEnvironment::kLinearFog:
        {
            float start, end;
            fog->GetPipelineParams(&start, &end, &color);
            fCurrentUniforms.fogExponential = 0;
            fCurrentUniforms.fogValues = { start, end };
            fCurrentUniforms.fogColor = { color.r, color.g, color.b };
        }
        break;

    case plFogEnvironment::kExpFog:
    case plFogEnvironment::kExp2Fog:
        {
            float density;
            const float power = (type == plFogEnvironment::kExp2Fog) ? 2.f : 1.f;
            fog->GetPipelineParams(&density, &color);
            fCurrentUniforms.fogExponential = 1;
            fCurrentUniforms.fogValues = { power, density };
            fCurrentUniforms.fogColor = { color.r, color.g, color.b };
        }
        break;

    default:
        fCurrentUniforms.fogExponential = 0;
        fCurrentUniforms.fogValues = { 0.f, 0.f };
        fCurrentUniforms.fogColor = { 0.f, 0.f, 0.f };
        break;
    }

    // Additive blends have to fog toward black, or the fog brightens them.
    if (currLayer->GetBlendFlags() & (hsGMatState::kBlendAdd | hsGMatState::kBlendMADD |
                                      hsGMatState::kBlendAddColorTimesAlpha)) {
        fCurrentUniforms.fogColor = { 0.f, 0.f, 0.f };
    }
}

/*** Software skinning *******************************************************/

/**
 * Blends one span's vertices through its matrix palette on the CPU.
 *
 * Ported from plMetalPipeline::IBlendVertBuffer (plMetalPipeline.cpp:4177-4241).
 * Metal does the arithmetic through Accelerate's simd types, which after the
 * row-major to column-major shuffle amounts to plain `matrix * point` -- so this
 * uses hsMatrix44's own operators instead.
 *
 * Only position and normal are written. Colors and UVs are left exactly as
 * FillVolatileVertexBufferRef put them, hence the strides skipped at the end.
 */
void plVulkanPipeline::IBlendVertBuffer(plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                                        const uint8_t* src, uint8_t format, uint32_t srcStride,
                                        uint8_t* dest, uint32_t destStride, uint32_t count,
                                        uint16_t localUVWChans)
{
    hsAssert(localUVWChans == 0, "support for skinned UVWs dropped. reimplement me?");

    const size_t  uvChanSize = plGBufferGroup::CalcNumUVs(format) * sizeof(float) * 3;
    const uint8_t numWeights = (format & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < count; ++i) {
        hsPoint3 pt;
        memcpy(&pt, src, sizeof(hsPoint3));
        src += sizeof(hsPoint3);

        float weights[4] = { 0.f, 0.f, 0.f, 0.f };
        float weightSum = 0.f;
        for (uint8_t j = 0; j < numWeights; ++j) {
            memcpy(&weights[j], src, sizeof(float));
            src += sizeof(float);
            weightSum += weights[j];
        }
        // The last weight is implied, so the set always sums to one.
        weights[numWeights] = 1.f - weightSum;

        uint32_t indices;
        if (format & plGBufferGroup::kSkinIndices) {
            memcpy(&indices, src, sizeof(uint32_t));
            src += sizeof(uint32_t);
        } else {
            // No index data means matrix 0 then matrix 1.
            indices = 1 << 8;
        }

        hsVector3 vec;
        memcpy(&vec, src, sizeof(hsVector3));
        src += sizeof(hsVector3);

        hsPoint3  destPt(0.f, 0.f, 0.f);
        hsVector3 destNorm(0.f, 0.f, 0.f);

        for (uint32_t j = 0; j < uint32_t(numWeights) + 1; ++j) {
            const float weight = weights[j];
            if (weight != 0.f) {
                const hsMatrix44& matrix = matrixPalette[indices & 0xFF];

                const hsPoint3  xPt = matrix * pt;
                const hsVector3 xVec = matrix * vec;

                destPt.fX += xPt.fX * weight;
                destPt.fY += xPt.fY * weight;
                destPt.fZ += xPt.fZ * weight;

                destNorm.fX += xVec.fX * weight;
                destNorm.fY += xVec.fY * weight;
                destNorm.fZ += xVec.fZ * weight;
            }
            indices >>= 8;
        }

        memcpy(dest, &destPt, sizeof(hsPoint3));
        dest += sizeof(hsPoint3);
        memcpy(dest, &destNorm, sizeof(hsVector3));
        dest += sizeof(hsVector3);

        // Step over the two packed colors and the UVs in both buffers.
        dest += sizeof(uint32_t) * 2 + uvChanSize;
        src += sizeof(uint32_t) * 2 + uvChanSize;
    }
}

// Ported from plMetalPipeline::ISoftwareVertexBlend (plMetalPipeline.cpp:4078-4170).
bool plVulkanPipeline::ISoftwareVertexBlend(plDrawableSpans* drawable,
                                            const std::vector<int16_t>& visList)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoSkinning))
        return true;

    if (drawable->GetSkinTime() == fRenderCnt)
        return true;

    const hsBitVector& blendBits = drawable->GetBlendingSpanVector();
    if (blendBits.Empty()) {
        drawable->SetSkinTime(fRenderCnt);
        return true;
    }

    // Work out which (group, buffer) pairs any visible skinned span touches, so
    // each destination buffer is walked once rather than per span.
    constexpr int kMaxBufferGroups = 20;
    constexpr int kMaxVertexBuffers = 20;
    char blendBuffers[kMaxBufferGroups][kMaxVertexBuffers];
    memset(blendBuffers, 0, sizeof(blendBuffers));

    hsAssert(kMaxBufferGroups >= drawable->GetNumBufferGroups(),
             "Bigger than we counted on num groups skin.");

    const std::vector<plSpan*>& spans = drawable->GetSpanArray();

    for (size_t i = 0; i < visList.size(); i++) {
        if (blendBits.IsBitSet(visList[i])) {
            const plVertexSpan& vSpan = *static_cast<plVertexSpan*>(spans[visList[i]]);
            hsAssert(kMaxVertexBuffers > vSpan.fVBufferIdx,
                     "Bigger than we counted on num buffers skin.");

            blendBuffers[vSpan.fGroupIdx][vSpan.fVBufferIdx] = 1;
            drawable->SetBlendingSpanVectorBit(visList[i], false);
        }
    }

    for (int i = 0; i < kMaxBufferGroups; i++) {
        for (int j = 0; j < kMaxVertexBuffers; j++) {
            if (!blendBuffers[i][j])
                continue;

            plVulkanVertexBufferRef* vRef =
                static_cast<plVulkanVertexBufferRef*>(drawable->GetVertexRef(i, j));
            if (!vRef || !vRef->fData)
                continue;

            uint8_t* destPtr = vRef->fData;

            for (size_t k = 0; k < visList.size(); k++) {
                const plIcicle& span = *static_cast<plIcicle*>(spans[visList[k]]);
                if (span.fGroupIdx != i || span.fVBufferIdx != j)
                    continue;

                hsMatrix44* matrixPalette = drawable->GetMatrixPalette(span.fBaseMatrix);
                matrixPalette[0] = span.fLocalToWorld;

                const uint8_t* srcPtr = vRef->fOwner->GetVertBufferData(vRef->fIndex) +
                                        span.fVStartIdx * vRef->fOwner->GetVertexSize();

                IBlendVertBuffer(const_cast<plSpan*>(static_cast<const plSpan*>(&span)),
                                 matrixPalette, span.fNumMatrices,
                                 srcPtr,
                                 vRef->fOwner->GetVertexFormat(),
                                 vRef->fOwner->GetVertexSize(),
                                 destPtr + span.fVStartIdx * vRef->fVertexSize,
                                 vRef->fVertexSize,
                                 span.fVLength,
                                 span.fLocalUVWChans);

                // Marks the ref Expired, so ICheckDynBuffers uploads fData.
                vRef->SetDirty(true);
            }
        }
    }

    // Only claim the drawable is done if every skinned span was consumed.
    if (drawable->GetBlendingSpanVector().Empty())
        drawable->SetSkinTime(fRenderCnt);

    return true;
}

/*** Plates ******************************************************************/

// Ported from plMetalPipeline::IDrawPlate (plMetalPipeline.cpp:2517-2594).
void plVulkanPipeline::IDrawPlate(plPlate* plate)
{
    VkCommandBuffer cmd = fDevice.CurrentCommandBuffer();
    if (cmd == VK_NULL_HANDLE)
        return;

    hsGMaterial* material = plate->GetMaterial();
    plLayerInterface* layer = material ? material->GetLayer(0) : nullptr;
    if (!layer)
        return;

    hsGMatState state;
    state.Composite(layer->GetState(), fMatOverOn, fMatOverOff);

    plVulkanTextureRef* texRef = nullptr;
    if (plBitmap* bitmap = layer->GetTexture()) {
        CheckTextureRef(layer);
        texRef = static_cast<plVulkanTextureRef*>(bitmap->GetDeviceRef());
    }
    if (!texRef || texRef->fImageView == VK_NULL_HANDLE)
        return;

    plVulkanPipelineKey key{};
    key.fPassKind = plVulkanPipelineKey::kPassPlate;
    key.fBlendFlags[0] = state.fBlendFlags;
    key.fColorFormat = fDevice.CurrentColorFormat();
    key.fDepthFormat = fDevice.CurrentDepthFormat();
    key.fSampleCount = fDevice.CurrentSampleCount();

    VkPipeline pipeline = fDevice.GetPipelineState(key);
    if (pipeline == VK_NULL_HANDLE)
        return;

    if (fState.fPipeline != pipeline) {
        fState.fPipeline = pipeline;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // D3D9 sends plates through the normal material state path. Most plate
    // materials carry kZNoZRead, which means ALWAYS + depth write; custom plate
    // materials are allowed to request the other modes as well.
    IHandleZMode(cmd, state);
    vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);

    // The plate projection from plDXPipeline::IDrawPlate. m[5] puts Plasma's -Y
    // plate origin at the top of the screen, and m[11]/m[15] make clip.w the
    // vertex z, which divides out the fDepth scaling plPlate folds into its
    // transform. The indices are transposed from D3D's because plVkMat4 holds
    // the column-major layout IToShaderMatrix produces: m[4 * r + c] is
    // fMap[r][c], and the shader evaluates v * M.
    VertexUniforms uniforms{};
    memset(&uniforms, 0, sizeof(uniforms));
    uniforms.projectionMatrix.m[0] = 1.f;
    uniforms.projectionMatrix.m[5] = -1.f;
    uniforms.projectionMatrix.m[10] = 2.f;
    uniforms.projectionMatrix.m[11] = -2.f;
    uniforms.projectionMatrix.m[14] = 1.f;
    uniforms.projectionMatrix.m[15] = 0.f;
    IToShaderMatrix(plate->GetTransform(), uniforms.localToWorldMatrix);
    uniforms.uvTransforms[0].UVWSrc = 0;
    uniforms.uvTransforms[0].transform.m[0] = 1.f;
    uniforms.uvTransforms[0].transform.m[5] = 1.f;
    uniforms.uvTransforms[0].transform.m[10] = 1.f;
    uniforms.uvTransforms[0].transform.m[15] = 1.f;

    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!vertexAlloc.IsValid() || !materialAlloc.IsValid())
        return;

    memcpy(vertexAlloc.fMapped, &uniforms, sizeof(uniforms));
    memset(materialAlloc.fMapped, 0, sizeof(plMaterialLightingDescriptor));

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    const plVulkanTextureRef* layers[1] = { texRef };
    const uint8_t clampFlags[1] = { uint8_t(layer->GetClampFlags()) };
    VkDescriptorSet textureSet = fDevice.GetTextureDescriptorSet(
        layers, clampFlags, 1, nullptr, nullptr, layer->GetKeyName());
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return;

    // The shadow-state binding exists for every pipeline layout, so a draw that
    // is not a shadow still has to name something: a zeroed block.
    const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, materialAlloc.fOffset,
                                         fLightBuffer.fOffset, fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

    const float alpha = layer->GetOpacity();
    vkCmdPushConstants(cmd, fDevice.GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(float), &alpha);

    static_cast<plVulkanPlateManager*>(fPlateMgr)->EncodeDraw(cmd);

    // The plate manager binds its own quad, so the span path must rebind.
    fState.fVertexBuffer = VK_NULL_HANDLE;
    fState.fIndexBuffer = VK_NULL_HANDLE;
}

/*** Span walking ************************************************************/

bool plVulkanPipeline::PreRender(plDrawable* drawable, std::vector<int16_t>& visList,
                                 plVisMgr* visMgr)
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
    if (!ds)
        return false;

    if ((ds->GetType() & fView.GetDrawableTypeMask()) == 0)
        return false;

    fView.GetVisibleSpans(ds, visList, visMgr);

    return !visList.empty();
}

bool plVulkanPipeline::PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList,
                                     plVisMgr* visMgr)
{
    plDrawableSpans* ice = plDrawableSpans::ConvertNoRef(drawable);
    if (!ice)
        return false;

    // Attach per-span light lists and shadow slaves.
    ICheckLighting(ice, visList, visMgr);

    // The whole light set goes up once; draws index into it.
    LoadLightsOnDevice();

    if (ice->GetNativeProperty(plDrawable::kPropSortFaces))
        ice->SortVisibleSpans(visList, this);

    ice->PrepForRender(this);

    // Matrix-palette spans are skinned on the CPU into the ref's shadow buffer,
    // which ICheckDynBuffers then uploads.
    if (!ISoftwareVertexBlend(ice, visList))
        return false;

    return true;
}

// Ported from plMetalPipeline::RenderSpans (plMetalPipeline.cpp:1059-1155).
void plVulkanPipeline::RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList)
{
    hsMatrix44 lastL2W;
    lastL2W.Reset();
    ISetLocalToWorld(lastL2W, lastL2W);

    const std::vector<plSpan*>& spans = ice->GetSpanArray();

    for (size_t i = 0; i < visList.size();) {
        hsGMaterial* material = GetOverrideMaterial()
                              ? GetOverrideMaterial()
                              : ice->GetMaterial(spans[visList[i]]->fMaterialIdx);

        // Merging is pure CPU batching: consecutive spans with the same material
        // and contiguous index ranges become one draw. Nothing is re-uploaded.
        plIcicle tempIce(*static_cast<plIcicle*>(spans[visList[i]]));

        size_t j = i + 1;
        for (; j < visList.size(); j++) {
            if (GetOverrideMaterial())
                tempIce.fMaterialIdx = spans[visList[j]]->fMaterialIdx;

            if (!spans[visList[j]]->CanMergeInto(&tempIce))
                break;

            spans[visList[j]]->MergeInto(&tempIce);
        }

        if (material) {
            plGBufferGroup* group = ice->GetBufferGroup(tempIce.fGroupIdx);

            ISetupTransforms(ice, tempIce, lastL2W);

            if (!ICheckDynBuffers(ice, group, &tempIce)) {
                CheckVertexBufferRef(group, tempIce.fVBufferIdx);
                CheckIndexBufferRef(group, tempIce.fIBufferIdx);

                IRenderBufferSpan(tempIce,
                                  group->GetVertexBufferRef(tempIce.fVBufferIdx),
                                  group->GetIndexBufferRef(tempIce.fIBufferIdx),
                                  material,
                                  tempIce.fVStartIdx, tempIce.fVLength,
                                  tempIce.fIPackedIdx, tempIce.fILength);
            }
        }

        i = j;
    }
}
