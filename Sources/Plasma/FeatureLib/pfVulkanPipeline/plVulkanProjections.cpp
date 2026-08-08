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

// Projected lights.
//
// A projecting light carries a texture instead of just a color. Plasma draws it
// as extra geometry passes over the span it lights, with only that one light
// enabled, so the texture lands on the surface the way a slide projector would.
//
// Two kinds, split by plLightInfo::OverAll(): "each" projectors are drawn once
// per material pass, "all" projectors once after every pass is done.
//
// Ported from plMetalPipeline.cpp:1306-1420.

#include "plVulkanPipeline.h"

#include "plVulkanDevice.h"
#include "plVulkanDeviceRef.h"
#include "plVulkanMaterialShaderRef.h"

#include "hsGMatState.h"
#include "plGLight/plLightInfo.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/plLayerShadowBase.h"

#include <cstring>

void plVulkanPipeline::SaveCurrentLightSources()
{
    // The projection pass wants a light set of its own; the span's set has to
    // survive it, because the next pass is lit exactly as the last one was.
    fLightStack.push_back(fLights);
    fLights.clear();
}

void plVulkanPipeline::RestoreCurrentLightSources()
{
    hsAssert(!fLightStack.empty(), "Asked to pop light sources but none on stack");
    if (fLightStack.empty())
        return;

    fLights = fLightStack.back();
    fLightStack.pop_back();
}

// Ported from plMetalPipeline::IRenderProjection (:1320-1375).
void plVulkanPipeline::IRenderProjection(VkCommandBuffer cmd, plLightInfo* li,
                                         const plVulkanVertexBufferRef* vRef,
                                         uint32_t iStart, uint32_t iLength)
{
    plLayerInterface* proj = li->GetProjection();
    if (!proj || !proj->GetTexture())
        return;

    // Only the projecting light contributes.
    ILoadLight(li);

    CheckTextureRef(proj);
    plVulkanTextureRef* tex =
        static_cast<plVulkanTextureRef*>(proj->GetTexture()->GetDeviceRef());
    if (!tex)
        return;

    IScaleLight(li, true);

    // The projection is the light's own color times its texture, so the material
    // contributes nothing: no ambient, no emissive, white diffuse.
    fCurrentMaterial.ambientSrc = 1;
    fCurrentMaterial.diffuseSrc = 1;
    fCurrentMaterial.emissiveSrc = 1;
    fCurrentMaterial.specularSrc = 1;
    fCurrentMaterial.globalAmb = { 1.f, 1.f, 1.f, 1.f };
    fCurrentMaterial.ambientCol = { 0.f, 0.f, 0.f };
    fCurrentMaterial.emissiveCol = { 0.f, 0.f, 0.f };
    fCurrentMaterial.specularCol = { 0.f, 0.f, 0.f };
    fCurrentMaterial.diffuseCol = { 1.f, 1.f, 1.f, 1.f };
    fCurrentMaterial.lightCount = uint32_t(fLights.size());
    for (uint32_t i = 0; i < fCurrentMaterial.lightCount; i++)
        fCurrentMaterial.activeLights[i] = fLights[i];

    fCurrentUniforms.fogColor = { 0.f, 0.f, 0.f };

    fCurrentUniforms.uvTransforms[0].UVWSrc = proj->GetUVWSrc();
    IToShaderMatrix(proj->GetTransform(), fCurrentUniforms.uvTransforms[0].transform);

    fCurrNumLayers = 1;

    plVulkanPipelineKey key{};
    key.fPassKind = plVulkanPipelineKey::kPassMaterial;
    plVulkanFillVertexKey(key, vRef);
    key.fNumLayers = 1;
    key.fPassTypes[0] = tex->fIsCubic ? kPassTypeCubicTexture : kPassTypeTexture;
    key.fBlendFlags[0] = proj->GetBlendFlags();
    key.fMiscFlags[0] = proj->GetMiscFlags();

    // DX inverts the combiner's color when the final color is to be inverted.
    if (proj->GetBlendFlags() & hsGMatState::kBlendInvertFinalColor)
        key.fBlendFlags[0] |= hsGMatState::kBlendInvertColor;

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

    // These should have been exported with kZNoZWrite, but were not.
    hsGMatState zState;
    zState.fZFlags = hsGMatState::kZNoZWrite;
    IHandleZMode(cmd, zState);
    vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);

    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!vertexAlloc.IsValid() || !materialAlloc.IsValid())
        return;

    memcpy(vertexAlloc.fMapped, &fCurrentUniforms, sizeof(VertexUniforms));
    memcpy(materialAlloc.fMapped, &fCurrentMaterial, sizeof(plMaterialLightingDescriptor));

    const plVulkanTextureRef* layers[1] = { tex };
    const uint8_t clampFlags[1] = { uint8_t(proj->GetClampFlags()) };

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    VkDescriptorSet textureSet = fDevice.GetTextureDescriptorSet(
        layers, clampFlags, 1, nullptr, nullptr, ST_LITERAL("a light projection"));
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return;

    const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, materialAlloc.fOffset,
                                         fLightBuffer.fOffset, fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset,
                                         fEmptyShadowState.fOffset };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

    vkCmdDrawIndexed(cmd, iLength, 1, iStart, 0, 0);
}

// Ported from plMetalPipeline::IRenderProjections (:1309-1316).
void plVulkanPipeline::IRenderProjections(VkCommandBuffer cmd,
                                          const plVulkanVertexBufferRef* vRef,
                                          uint32_t iStart, uint32_t iLength)
{
    plVulkanDebugLabel label(fDevice, cmd, ST_LITERAL("Render all projections"));

    SaveCurrentLightSources();
    for (plLightInfo* li : fProjAll)
        IRenderProjection(cmd, li, vRef, iStart, iLength);
    RestoreCurrentLightSources();
}

// Ported from plMetalPipeline::IRenderProjectionEach (:1380-1420).
void plVulkanPipeline::IRenderProjectionEach(VkCommandBuffer cmd, hsGMaterial* material,
                                             size_t pass, const plSpan& span,
                                             const plVulkanVertexBufferRef* vRef,
                                             uint32_t iStart, uint32_t iLength)
{
    plVulkanDebugLabel label(fDevice, cmd, ST_LITERAL("Render projections"));

    // Overrides the material's blend to Add against the frame buffer with no
    // depth write, and its ambient to zero -- so the projection adds light
    // rather than replacing what the pass already drew.
    static plLayerLightBase layLightBase;

    for (plLightInfo* li : fProjEach) {
        plLayerInterface* proj = li->GetProjection();
        hsAssert(proj, "A projector with no texture to project?");
        if (!proj)
            continue;

        IPushProjPiggyBack(proj);

        // Only the projecting light contributes.
        ILoadLight(li);

        AppendLayerInterface(&layLightBase, false);

        // Shaders are suppressed: this pass is the fixed-function combiner
        // applying a projected texture, whatever the layer would normally do.
        if (IHandleMaterialPass(cmd, material, pass, &span, vRef, false)) {
            IScaleLight(li, true);
            vkCmdDrawIndexed(cmd, iLength, 1, iStart, 0, 0);
        }

        RemoveLayerInterface(&layLightBase, false);
        IPopProjPiggyBacks();
    }
}

// Ported from plMetalPipeline::IPushProjPiggyBack (:2705-2713).
void plVulkanPipeline::IPushProjPiggyBack(plLayerInterface* li)
{
    if (fView.fRenderState & plPipeline::kRenderNoPiggyBacks)
        return;

    fPiggyBackStack.push_back(li);
    fActivePiggyBacks = uint32_t(fPiggyBackStack.size()) - fMatPiggyBacks;
}

// Ported from plMetalPipeline::IPopProjPiggyBacks (:2717-2725).
void plVulkanPipeline::IPopProjPiggyBacks()
{
    if (fView.fRenderState & plPipeline::kRenderNoPiggyBacks)
        return;

    fPiggyBackStack.resize(fMatPiggyBacks);
    ISetNumActivePiggyBacks();
}
