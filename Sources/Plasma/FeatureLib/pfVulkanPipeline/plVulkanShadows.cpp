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

// Shadows.
//
// Two halves. First, before the scene is drawn, every submitted shadow slave
// renders its caster into a small offscreen target from the light's point of
// view; the map records distance-from-light in alpha. Then, after a span has
// been drawn normally, every shadow map whose bounds reach that span is
// projected back onto it and darkens it.
//
// Ported from plMetalPipeline.cpp:3040-3980. See plGLight/plShadowMaster.cpp for
// the geometry behind the LUTs.

#include "plVulkanPipeline.h"

#include "plVulkanDeviceRef.h"
#include "plVulkanMaterialShaderRef.h"

#include "hsBitVector.h"
#include "plDrawable/plDrawableSpans.h"
#include "plGLight/plShadowCaster.h"
#include "plGLight/plShadowSlave.h"
#include "plPipeline/plRenderTarget.h"
#include "plPipeDebugFlags.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include <string_theory/format>

#include <cstring>

/*** Render target pools *****************************************************/

void plVulkanPipeline::IClearShadowSlaves()
{
    for (plShadowSlave* slave : fShadows) {
        if (slave && slave->fCaster && slave->fCaster->GetKey())
            slave->fCaster->GetKey()->UnRefObject();
    }
    fShadows.clear();
}

bool plVulkanDevice::BlurCurrentRenderTarget(float sigma)
{
    plVulkanRenderTargetRef* source = fCurrentTarget;
    if (!source || source->fImage == VK_NULL_HANDLE || sigma <= 0.f || !fFrameOpen)
        return false;

    IEndRendering();

    if (!fBlurTarget)
        fBlurTarget = new plVulkanRenderTargetRef();
    if (fBlurTarget->fImage == VK_NULL_HANDLE ||
        fBlurTarget->fWidth != source->fWidth ||
        fBlurTarget->fHeight != source->fHeight) {
        if (!CreateRenderTarget(fBlurTarget, source->fWidth, source->fHeight, false,
                                ST_LITERAL("shadow blur target"))) {
            fCurrentTarget = source;
            return false;
        }
    }

    auto drawPass = [this](plVulkanRenderTargetRef* input,
                           plVulkanRenderTargetRef* output, float axisSigma) {
        fCurrentTarget = output;
        const hsColorRGBA clearColor = hsColorRGBA().Set(0.f, 0.f, 0.f, 0.f);
        IBeginRendering(&clearColor);

        VkCommandBuffer cmd = fFrames[fFrameIndex].fCmd;
        plVulkanPipelineKey key{};
        key.fPassKind = plVulkanPipelineKey::kPassBlur;
        key.fColorFormat = CurrentColorFormat();
        key.fDepthFormat = CurrentDepthFormat();
        key.fSampleCount = CurrentSampleCount();

        VkPipeline pipeline = GetPipelineState(key);
        if (pipeline == VK_NULL_HANDLE) {
            IEndRendering();
            return false;
        }

        plVulkanTextureRef sampled;
        sampled.fImageView = input->fImageView;
        sampled.fCookie = input->fCookie;
        const plVulkanTextureRef* layers[] = { &sampled };
        const uint8_t clamp[] = { hsGMatState::kClampTexture };
        VkDescriptorSet textures = GetTextureDescriptorSet(layers, clamp, 1, nullptr, nullptr,
                                                           ST_LITERAL("the shadow blur"));
        if (textures == VK_NULL_HANDLE) {
            IEndRendering();
            return false;
        }

        // SetViewport follows the pipeline's view transform, which is still the
        // slave's texel-inset rect. The blur has to cover the whole target, or
        // the border it leaves behind keeps the clear's zero alpha and reads as
        // full shadow everywhere outside the map.
        const VkExtent2D extent = CurrentExtent();
        VkViewport viewport{};
        viewport.y = float(extent.height);
        viewport.width = float(extent.width);
        viewport.height = -float(extent.height);
        viewport.maxDepth = 1.f;
        const VkRect2D scissor{ { 0, 0 }, extent };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
        vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
        vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);
        vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                fPipelineLayout, kDescSetTextures, 1,
                                &textures, 0, nullptr);
        vkCmdPushConstants(cmd, fPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(axisSigma), &axisSigma);
        vkCmdDraw(cmd, 3, 1, 0, 0);
        IEndRendering();
        return true;
    };

    if (!drawPass(source, fBlurTarget, sigma) ||
        !drawPass(fBlurTarget, source, -sigma)) {
        fCurrentTarget = source;
        return false;
    }

    fCurrentTarget = source;
    return true;
}

// Ported from plMetalPipeline::IMakeRenderTargetPools (:3618-3702).
void plVulkanPipeline::IMakeRenderTargetPools()
{
    IReleaseRenderTargetPools();

    // How many targets of each size to keep. These counts were chosen with a
    // populated age in mind: the avatar is far from the only shadow caster in
    // production assets.
    static const uint32_t kCount[kMaxRenderTargetNext] = {
        0,  // 1x1
        0,  // 2x2
        0,  // 4x4
        0,  // 8x8
        0,  // 16x16
        32, // 32x32
        16, // 64x64
        8,  // 128x128
        4,  // 256x256
        0   // 512x512
    };

    for (uint32_t i = 0; i < kMaxRenderTargetNext; i++) {
        std::vector<plRenderTarget*>* pool = nullptr;
        switch (i) {
        case 5: pool = &fRenderTargetPool32; break;
        case 6: pool = &fRenderTargetPool64; break;
        case 7: pool = &fRenderTargetPool128; break;
        case 8: pool = &fRenderTargetPool256; break;
        case 9: pool = &fRenderTargetPool512; break;
        default: break;
        }
        if (!pool)
            continue;

        // One past the count stays null, which is how IFindRenderTarget knows
        // it has run out.
        pool->assign(kCount[i] + 1, nullptr);

        for (uint32_t j = 0; j < kCount[i]; j++) {
            const uint16_t flags = plRenderTarget::kIsTexture | plRenderTarget::kIsProjected;
            const uint16_t size = uint16_t(1u << i);

            plRenderTarget* rt = new plRenderTarget(flags, size, size, uint8_t(32),
                                                    uint8_t(24), uint8_t(0));

            // A failure here is almost certainly out of video memory. Keep what
            // was made and stop; casters past this point just go unshadowed.
            if (!MakeRenderTargetRef(rt)) {
                delete rt;
                pool->resize(j + 1);
                (*pool)[j] = nullptr;
                break;
            }

            (*pool)[j] = rt;
        }
    }
}

void plVulkanPipeline::IReleaseRenderTargetPools()
{
    for (std::vector<plRenderTarget*>* pool : { &fRenderTargetPool512, &fRenderTargetPool256,
                                                &fRenderTargetPool128, &fRenderTargetPool64,
                                                &fRenderTargetPool32 }) {
        for (plRenderTarget* rt : *pool)
            delete rt;
        pool->clear();
    }

    for (uint32_t i = 0; i < kMaxRenderTargetNext; i++)
        fRenderTargetNext[i] = 0;
}

// Frees nothing: this just says every pooled target is available again, which is
// what makes a target belong to a slave for exactly one frame.
void plVulkanPipeline::IResetRenderTargetPools()
{
    for (uint32_t i = 0; i < kMaxRenderTargetNext; i++)
        fRenderTargetNext[i] = 0;
}

// Ported from plMetalPipeline::IFindRenderTarget (:3417-3455).
plRenderTarget* plVulkanPipeline::IFindRenderTarget(uint32_t& width, uint32_t& height, bool ortho)
{
    std::vector<plRenderTarget*>* pool = nullptr;
    uint32_t* iNext = nullptr;

    // Non-square shadows are not supported, so height alone decides.
    switch (height) {
    case 512: pool = &fRenderTargetPool512; iNext = &fRenderTargetNext[9]; break;
    case 256: pool = &fRenderTargetPool256; iNext = &fRenderTargetNext[8]; break;
    case 128: pool = &fRenderTargetPool128; iNext = &fRenderTargetNext[7]; break;
    case 64:  pool = &fRenderTargetPool64;  iNext = &fRenderTargetNext[6]; break;
    case 32:  pool = &fRenderTargetPool32;  iNext = &fRenderTargetNext[5]; break;
    default:  return nullptr;
    }

    if (*iNext >= pool->size())
        return nullptr;

    plRenderTarget* rt = (*pool)[*iNext];
    if (!rt) {
        // None left at this size; settle for half.
        if (height > 32)
            return IFindRenderTarget(width >>= 1, height >>= 1, ortho);

        return nullptr;
    }

    (*iNext)++;
    return rt;
}

/*** Generating the maps *****************************************************/

// Ported from plMetalPipeline::IPrepShadowCaster (:3112-3155).
//
// A caster may be several spans across several drawables, and each drawable has
// to be prepped exactly once -- with a visList naming all of its spans, so the
// software skin runs once over the whole set rather than once per span.
bool plVulkanPipeline::IPrepShadowCaster(const plShadowCaster* caster)
{
    static hsBitVector done;
    done.Clear();

    const std::vector<plShadowCastSpan>& castSpans = caster->Spans();

    for (size_t i = 0; i < castSpans.size(); i++) {
        if (done.IsBitSet(i))
            continue;

        plDrawableSpans* drawable = castSpans[i].fDraw;

        static std::vector<int16_t> visList;
        visList.clear();
        visList.push_back(int16_t(castSpans[i].fIndex));
        done.SetBit(i);

        // Gather this drawable's other spans so they are all prepped together.
        for (size_t j = i + 1; j < castSpans.size(); j++) {
            if (!done.IsBitSet(j) && castSpans[j].fDraw == drawable) {
                visList.push_back(int16_t(castSpans[j].fIndex));
                done.SetBit(j);
            }
        }

        drawable->PrepForRender(this);

        if (!ISoftwareVertexBlend(drawable, visList))
            return false;
    }

    return true;
}

// Ported from plMetalPipeline::IPushShadowCastState (:3257-3372).
bool plVulkanPipeline::IPushShadowCastState(plShadowSlave* slave)
{
    plRenderTarget* renderTarg = IFindRenderTarget(slave->fWidth, slave->fHeight,
                                                   slave->fView.GetOrthogonal());
    if (!renderTarg)
        return false;

    // The slave works out its own view transform; everything below just pushes
    // that down as render state.
    if (!slave->SetupViewTransform(this))
        return false;

    fViewStack.push(fView);
    fView.SetMaxCullNodes(0);
    SetViewTransform(slave->fView);
    IProjectionMatrixToDevice();

    PushRenderTarget(renderTarg);

    // The map records the light-space distance to each fragment in alpha, with
    // color left white. uvTransform 0 is what converts camera-space position
    // into that distance; see plShadowMaster::IComputeLUT.
    hsMatrix44 castLUT = slave->fCastLUT;
    if (slave->fFlags & plShadowSlave::kCastInCameraSpace)
        castLUT = castLUT * GetCameraToWorld();

    IToShaderMatrix(castLUT, fCurrentUniforms.uvTransforms[0].transform);
    fCurrentUniforms.uvTransforms[0].UVWSrc = plLayerInterface::kUVWPosition;

    // Alpha white so anywhere without a caster reads as "no shadow", color black
    // so a blur would not pull anything in. A reverse-Z slave compares
    // GREATER_OR_EQUAL, so its "far" is zero, not one.
    const hsColorRGBA clearColor = hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f);
    const float clearDepth = slave->ReverseZ() ? 0.f : 1.f;
    fDevice.Clear(&clearColor, &clearDepth);

    // Inset by a pixel AFTER the clear, so the alpha border survives and the
    // map's edge never bleeds a shadow outward when it is sampled clamped.
    fView.GetViewTransform().SetViewPort(1, 1, float(slave->fWidth - 2),
                                         float(slave->fHeight - 2), false);
    fDevice.SetViewport();

    slave->fPipeData = renderTarg;

    return true;
}

// Ported from plMetalPipeline::IPopShadowCastState (:3707-3718).
bool plVulkanPipeline::IPopShadowCastState(plShadowSlave* slave)
{
    fView = fViewStack.top();
    fViewStack.pop();

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;

    return true;
}

// Ported from plMetalPipeline::IRenderShadowCasterSpan (:3736-3786).
void plVulkanPipeline::IRenderShadowCasterSpan(VkCommandBuffer cmd, plShadowSlave* slave,
                                               plDrawableSpans* drawable, const plIcicle& span)
{
    ICheckDynBuffers(drawable, drawable->GetBufferGroup(span.fGroupIdx), &span);

    plVulkanVertexBufferRef* vRef =
        static_cast<plVulkanVertexBufferRef*>(drawable->GetVertexRef(span.fGroupIdx,
                                                                     span.fVBufferIdx));
    plVulkanIndexBufferRef* iRef =
        static_cast<plVulkanIndexBufferRef*>(drawable->GetIndexRef(span.fGroupIdx,
                                                                   span.fIBufferIdx));

    if (!vRef || !iRef || !vRef->GetBuffer().IsValid() || !iRef->GetBuffer().IsValid())
        return;

    plVulkanPipelineKey key{};
    key.fPassKind = plVulkanPipelineKey::kPassShadowCaster;
    plVulkanFillVertexKey(key, vRef);
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

    // The map wants every face the caster has, front and back alike.
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
    vkCmdSetDepthTestEnable(cmd, VK_TRUE);
    vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
    vkCmdSetDepthCompareOp(cmd, slave->ReverseZ() ? VK_COMPARE_OP_GREATER_OR_EQUAL
                                                  : VK_COMPARE_OP_LESS_OR_EQUAL);
    vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);

    hsMatrix44 lastL2W;
    lastL2W.Reset();
    // Each caster span is rendered independently, so lastL2W does not describe
    // the matrix currently cached by the device. In particular, software-skinned
    // vertices are already in world space and must force an identity transform;
    // otherwise a matrix left by an earlier draw moves them outside the slave's
    // light-space frustum.
    ISetupTransforms(drawable, span, lastL2W, true);

    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    if (!vertexAlloc.IsValid())
        return;

    memcpy(vertexAlloc.fMapped, &fCurrentUniforms, sizeof(VertexUniforms));

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    VkDescriptorSet textureSet = fDevice.GetTextureDescriptorSet(nullptr, nullptr, 0);
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return;

    // Shadow-map generation happens before this frame's light buffer is uploaded.
    // The caster shader does not read the light binding, so use a valid offset in
    // the current frame's scratch buffer instead of the previous frame's offset.
    const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, fEmptyShadowState.fOffset,
                                         0, fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

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

    vkCmdDrawIndexed(cmd, span.fILength, 1, span.fIPackedIdx, 0, 0);
}

// Ported from plMetalPipeline::IRenderShadowCaster (:3160-3212).
bool plVulkanPipeline::IRenderShadowCaster(plShadowSlave* slave)
{
    const plShadowCaster* caster = slave->fCaster;

    if (!IPushShadowCastState(slave))
        return false;

    if (!IPrepShadowCaster(slave->fCaster)) {
        IPopShadowCastState(slave);
        return false;
    }

    // The scope is only open once the target is bound and cleared, so the
    // command buffer has to be fetched after IPushShadowCastState.
    VkCommandBuffer cmd = fDevice.CurrentCommandBuffer();
    if (cmd == VK_NULL_HANDLE) {
        IPopShadowCastState(slave);
        return false;
    }

    // Scoped tightly: a label has to be closed inside the same rendering scope it
    // was opened in, and both the blur and IPopShadowCastState below change the
    // render target, which closes this one.
    {
        plVulkanDebugLabel label(fDevice, cmd,
                                 ST::format("Render shadow caster {}", slave->fIndex));

        for (size_t iSpan = 0; iSpan < caster->Spans().size(); iSpan++) {
            plDrawableSpans* dr = caster->Spans()[iSpan].fDraw;
            const plSpan*    sp = caster->Spans()[iSpan].fSpan;

            hsAssert(sp->fTypeMask & plSpan::kIcicleSpan,
                     "Shadow casting from non-trimeshes not currently supported");

            if (!(sp->fProps & plSpan::kPropNoShadowCast))
                IRenderShadowCasterSpan(cmd, slave, dr, *(const plIcicle*)sp);

            // Records that this span went into this map. With self-shadowing off,
            // that is what keeps the map from being projected back onto its source.
            sp->SetShadowBit(slave->fIndex);
        }
    }

    if (slave->fBlurScale > 0.f) {
        fDevice.BlurCurrentRenderTarget(slave->fBlurScale);
        fState.Reset();
    }

    IPopShadowCastState(slave);

    return true;
}

// Ported from plMetalPipeline::IPreprocessShadows (:3216-3252).
void plVulkanPipeline::IPreprocessShadows()
{
    // The pools belong to this frame again.
    IResetRenderTargetPools();

    for (size_t iSlave = 0; iSlave < fShadows.size(); iSlave++) {
        plShadowSlave* slave = fShadows[iSlave];

        // Anything that went wrong means no shadow from this slave this frame.
        if (!IRenderShadowCaster(slave)) {
            if (slave && slave->fCaster && slave->fCaster->GetKey())
                slave->fCaster->GetKey()->UnRefObject();
            fShadows.erase(fShadows.begin() + iSlave);
            iSlave--;
        }
    }

    // The scene render that follows starts from a clean slate: the shadow
    // passes changed the render target, the pipeline and both buffers.
    fState.Reset();
}

/*** Applying the maps *******************************************************/

// Ported from plMetalPipeline::ISetupShadowState (:3389-3410).
//
// The shadow light is not used to generate the map; it modulates the map as it
// is projected. Intensity follows N-dot-L on the receiving surface, so a sphere's
// shadow fades as its normals turn away from the light, and the whole effect is
// attenuated by the light's own strength and distance fade.
void plVulkanPipeline::ISetupShadowState(plShadowSlave* slave, plShadowState& shadowState)
{
    shadowState.power = slave->fPower;

    slave->fSelfShadowOn = false;

    if (slave->Positional()) {
        const hsPoint3 position = slave->fLightPos;
        shadowState.lightPosition = { position.fX, position.fY, position.fZ, 1.f };
        shadowState.directional = 0;
    } else {
        const hsVector3 dir = slave->fLightDir;
        shadowState.lightDirection = { dir.fX, dir.fY, dir.fZ, 0.f };
        shadowState.directional = 1;
    }
}

// Ported from plMetalPipeline::ISetupShadowRcvTextureStages (:3886-3939).
int32_t plVulkanPipeline::ISetupShadowRcvTextureStages(hsGMaterial* mat, uint32_t* miscFlags)
{
    *miscFlags = 0;

    if (!(fMaxLayersAtOnce > 3) || !mat->GetLayer(0)->GetTexture() ||
        !(mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha)) {
        return -1;
    }

    plLayerInterface* layer = mat->GetLayer(0);
    int32_t layerIndex = 0;

    // The "alpha hack": vertex alpha faked by an invisible second layer holding
    // an alpha LUT. When that is what is going on, layer 1 is the one carrying
    // the transparency worth respecting.
    if ((layer->GetMiscFlags() & hsGMatState::kMiscBindNext) && mat->GetLayer(1) &&
        !(mat->GetLayer(1)->GetMiscFlags() & hsGMatState::kMiscNoShadowAlpha) &&
        !(mat->GetLayer(1)->GetBlendFlags() & hsGMatState::kBlendNoTexAlpha) &&
        mat->GetLayer(1)->GetTexture()) {
        layer = mat->GetLayer(1);
        layerIndex = 1;
    }

    // Coordinate set 2 is where the shader looks for the alpha layer's UVs,
    // even though the texture itself is bound at slot 0 or 1.
    fCurrentUniforms.uvTransforms[2].UVWSrc = layer->GetUVWSrc();
    IToShaderMatrix(layer->GetTransform(), fCurrentUniforms.uvTransforms[2].transform);

    // sampleLocation() keys the projection and reflection branches off the same
    // layer's misc flags, so they have to travel with the transform above or the
    // vertex stage generates set 2 as if the layer were unprojected
    // (plMetalPipeline.cpp:3961-3968 populates stage 2 the same way).
    *miscFlags = layer->GetMiscFlags();

    return layerIndex;
}

// Ported from plMetalPipeline::ISetupShadowSlaveTextures (:3944-3978).
const plVulkanRenderTargetRef* plVulkanPipeline::ISetupShadowSlaveTextures(plShadowSlave* slave)
{
    plRenderTarget* renderTarg = static_cast<plRenderTarget*>(slave->fPipeData);
    hsAssert(renderTarg, "Processing a slave that hasn't been rendered");
    if (!renderTarg)
        return nullptr;

    plVulkanRenderTargetRef* ref =
        static_cast<plVulkanRenderTargetRef*>(renderTarg->GetDeviceRef());
    hsAssert(ref, "Shadow map ref should have been made when it was rendered");
    if (!ref)
        return nullptr;

    const hsMatrix44 c2w = GetCameraToWorld();

    // Coordinate set 0: camera space into the shadow map.
    IToShaderMatrix(slave->fWorldToTexture * c2w, fCurrentUniforms.uvTransforms[0].transform);
    fCurrentUniforms.uvTransforms[0].UVWSrc = plLayerInterface::kUVWPosition;

    // Coordinate set 1: the falloff LUT, which is a ramp read straight off the
    // coordinate rather than an actual texture.
    IToShaderMatrix(slave->fRcvLUT * c2w, fCurrentUniforms.uvTransforms[1].transform);
    fCurrentUniforms.uvTransforms[1].UVWSrc = plLayerInterface::kUVWPosition;

    return ref;
}

// Ported from plMetalPipeline::IRenderShadowsOntoSpan (:3790-3882).
void plVulkanPipeline::IRenderShadowsOntoSpan(VkCommandBuffer cmd, const plSpan* span,
                                              hsGMaterial* mat,
                                              const plVulkanVertexBufferRef* vRef,
                                              uint32_t iStart, uint32_t iLength)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (IsDebugFlagSet(plPipeDbg::kFlagNoShadowApply))
        return;
#endif

    plVulkanDebugLabel label(fDevice, cmd, ST_LITERAL("Apply shadows"));

    // Which shadows reach this span was worked out before the frame started.
    const hsBitVector& slaveBits = span->GetShadowSlaves();

    bool first = true;
    int32_t alphaSrc = -1;
    uint32_t alphaMiscFlags = 0;

    for (size_t i = 0; i < fShadows.size(); i++) {
        if (!slaveBits.IsBitSet(fShadows[i]->fIndex))
            continue;

        // Everything independent of the individual slave, done once.
        if (first)
            alphaSrc = ISetupShadowRcvTextureStages(mat, &alphaMiscFlags);

        const plVulkanRenderTargetRef* shadowMap = ISetupShadowSlaveTextures(fShadows[i]);
        if (!shadowMap)
            continue;

        plShadowState shadowState{};

        // Only the first shadow on a span picks up the material's opacity; the
        // rest are drawn at full strength over what it already darkened. Metal
        // clears its `first` flag before reading it here, so its ternary always
        // takes the 1.f branch -- this is what the ternary was written to do.
        shadowState.opacity = first ? mat->GetLayer(0)->GetOpacity() : 1.f;
        ISetupShadowState(fShadows[i], shadowState);

        first = false;

        const int selfShadowNow = span->IsShadowBitSet(fShadows[i]->fIndex);
        if (selfShadowNow != fShadows[i]->fSelfShadowOn) {
            // Artists crank shadow strength up to get a dark shadow on the
            // environment, which makes the avatar's shadow on itself far too
            // dark. Cap it when a surface is shadowing itself -- the caster and
            // the receiver are the same object, so they are very close together.
            if (selfShadowNow) {
                plConst(float) kMaxSelfPower = 0.3f;
                shadowState.power = std::min(float(fShadows[i]->fPower), float(kMaxSelfPower));
            } else {
                shadowState.power = fShadows[i]->fPower;
            }

            fShadows[i]->fSelfShadowOn = selfShadowNow;
        }

        //
        // The pass has three coordinate sets: the map, the LUT, and the alpha
        // layer's own UVs. The alpha layer's texture is already bound at slot 0
        // or 1 from the material's own render, so it is not rebound here -- but
        // its UVs live in set 2, which is why the key carries layer 2's state.
        //
        plVulkanPipelineKey key{};
        key.fPassKind = plVulkanPipelineKey::kPassShadowApply;
        plVulkanFillVertexKey(key, vRef);
        key.fNumLayers = 3;
        key.fMiscFlags[2] = alphaMiscFlags;
        key.fPointLightCast = fShadows[i]->fView.GetOrthogonal() ? 0 : 1;
        key.fShadowAlphaSrc = alphaSrc;
        key.fColorFormat = fDevice.CurrentColorFormat();
        key.fDepthFormat = fDevice.CurrentDepthFormat();
        key.fSampleCount = fDevice.CurrentSampleCount();

        const plVulkanTextureRef* layers[kMaxLayers] = {};
        const plVulkanRenderTargetRef* renderTargets[kMaxLayers] = {};
        uint8_t clampFlags[kMaxLayers] = {};

        for (uint32_t layerIdx = 0; layerIdx < 2 && layerIdx < mat->GetNumLayers(); layerIdx++) {
            plLayerInterface* layer = mat->GetLayer(layerIdx);
            if (!layer || !layer->GetTexture())
                continue;

            CheckTextureRef(layer);

            // A render target is a sibling of plVulkanTextureRef, not a subclass,
            // and its layout diverges after fAllocation -- so a layer backed by
            // one has to go through the render target array. The avatar's base
            // layer is exactly that: the clothing composite target.
            plBitmap* texture = layer->GetTexture();
            if (plRenderTarget::ConvertNoRef(texture)) {
                renderTargets[layerIdx] =
                    static_cast<plVulkanRenderTargetRef*>(texture->GetDeviceRef());
            } else {
                layers[layerIdx] = static_cast<plVulkanTextureRef*>(texture->GetDeviceRef());
            }
            clampFlags[layerIdx] = uint8_t(layer->GetClampFlags());
        }

        VkPipeline pipeline = fDevice.GetPipelineState(key);
        if (pipeline == VK_NULL_HANDLE)
            continue;

        if (fState.fPipeline != pipeline) {
            fState.fPipeline = pipeline;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }

        // Depth on, but read-only: the span is already in the depth buffer.
        vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
        vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS_OR_EQUAL);

        plVulkanDevice::plScratchAlloc vertexAlloc =
            fDevice.AllocateScratch(sizeof(VertexUniforms));
        plVulkanDevice::plScratchAlloc shadowAlloc = fDevice.AllocateScratch(sizeof(plShadowState));
        plVulkanDevice::plScratchAlloc materialAlloc =
            fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
        if (!vertexAlloc.IsValid() || !shadowAlloc.IsValid() || !materialAlloc.IsValid())
            return;

        memcpy(vertexAlloc.fMapped, &fCurrentUniforms, sizeof(VertexUniforms));
        memcpy(shadowAlloc.fMapped, &shadowState, sizeof(plShadowState));
        memcpy(materialAlloc.fMapped, &fCurrentMaterial, sizeof(plMaterialLightingDescriptor));

        VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
        VkDescriptorSet textureSet =
            fDevice.GetTextureDescriptorSet(layers, clampFlags, 2, shadowMap, renderTargets,
                                            ST_LITERAL("a shadow apply pass"));
        if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
            return;

        const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, materialAlloc.fOffset,
                                             fLightBuffer.fOffset, shadowAlloc.fOffset,
                                             fEmptyShadowState.fOffset,
                                             fEmptyShadowState.fOffset };
        VkDescriptorSet sets[2] = { uniformSet, textureSet };
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                                0, 2, sets, 6, dynamicOffsets);

        vkCmdDrawIndexed(cmd, iLength, 1, iStart, 0, 0);
    }
}
