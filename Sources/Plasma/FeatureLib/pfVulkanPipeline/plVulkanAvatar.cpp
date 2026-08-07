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

// The avatar clothing composite.
//
// An avatar's skin is not a single texture: it is assembled every time the
// outfit changes, by drawing each clothing element -- shirt, pants, tint layers,
// skin blends -- as a quad into an offscreen target that then becomes the
// material's texture. pl3DPipeline owns the target pool; this is the drawing.
//
// Ported from plMetalPipeline.cpp:2790-2938 and ShaderSrc/Avatar.metal.

#include "plVulkanPipeline.h"

#include "plVulkanDevice.h"
#include "plVulkanDeviceRef.h"
#include "plVulkanMaterialShaderRef.h"

#include "plAvatar/plAvatarClothing.h"
#include "plAvatar/plClothingLayout.h"
#include "plGImage/plMipmap.h"
#include "plPipeline/plRenderTarget.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include <string_theory/format>

#include <cstring>

/** One corner of a clothing element's quad. Matches plAVTexVert. */
struct plVulkanAvatarVert
{
    float fPos[2];
    float fUv[2];
};

// Ported from plMetalPipeline::IPreprocessAvatarTextures (:2790-2915).
void plVulkanPipeline::IPreprocessAvatarTextures()
{
    // Anything used last frame that this frame does not want is handed back.
    IClearClothingOutfits(&fPrevClothingOutfits);

    if (fClothingOutfits.empty())
        return;

    for (size_t oIdx = 0; oIdx < fClothingOutfits.size(); oIdx++) {
        plClothingOutfit* co = fClothingOutfits[oIdx];
        if (!co->fBase || !co->fBase->fBaseTexture)
            continue;

        plRenderTarget* rt = plRenderTarget::ConvertNoRef(co->fTargetLayer->GetTexture());
        if (rt && co->fDirtyItems.Empty()) {
            // Last frame's target is still valid and nothing changed.
            continue;
        }

        if (!rt) {
            // IGetNextAvRT does not check, and the pool may not have been filled
            // yet or may be smaller than the number of avatars on screen.
            // CheckResources notices the shortfall and asks for a reload.
            if (fAvNextFreeRT >= fAvRTPool.size())
                continue;

            rt = IGetNextAvRT();
            if (!rt)
                continue;

            // The material is about to gain a texture it did not have, so its
            // cached pass decomposition no longer describes it.
            if (plVulkanMaterialShaderRef* ref =
                    static_cast<plVulkanMaterialShaderRef*>(co->fMaterial->GetDeviceRef())) {
                ref->SetDirty(true);
            }

            co->fTargetLayer->SetTexture(rt);
        }

        PushRenderTarget(rt);

        // The target is not cleared: the outfit's base texture replaces every
        // pixel before the individual clothing elements are blended over it.
        VkCommandBuffer cmd = fDevice.CurrentCommandBuffer();
        if (cmd == VK_NULL_HANDLE) {
            PopRenderTarget();
            continue;
        }

        // A label has to be closed inside the rendering scope it was opened in, and
        // PopRenderTarget below closes this one.
        {
            // The composite is what an avatar's skin actually samples, so it is worth
            // finding in a capture on its own.
            plVulkanDebugLabel label(fDevice, cmd,
                                     ST::format("Avatar clothing composite {}", oIdx));

            // Half-texel offsets, so each element lands on texel centres.
            const float uOff = 0.5f / rt->GetWidth();
            const float vOff = 0.5f / rt->GetHeight();

            hsColorRGBA white;
            white.Set(1.f, 1.f, 1.f, 1.f);
            IDrawClothingQuad(cmd, plVulkanPipelineKey::kPassAvatarBase,
                              -1.f, -1.f, 2.f, 2.f, uOff, vOff,
                              co->fBase->fBaseTexture, white);

            plClothingLayout* layout =
                plClothingMgr::GetClothingMgr()->GetLayout(co->fBase->fLayoutName);

            for (plClothingItem* item : co->fItems) {
                for (size_t j = 0; j < item->fElements.size(); j++) {
                    for (int k = 0; k < plClothingElement::kLayerMax; k++) {
                        if (!item->fTextures[j][k])
                            continue;

                        hsColorRGBA tint = co->GetItemTint(item, k);
                        if (k >= plClothingElement::kLayerSkinBlend1 &&
                            k <= plClothingElement::kLayerSkinLast) {
                            tint.a = co->fSkinBlends[k - plClothingElement::kLayerSkinBlend1];
                        }

                        // The base element replaces whatever the target held; every
                        // element above it blends over what is already there.
                        const uint8_t passKind = (k == plClothingElement::kLayerBase)
                                               ? plVulkanPipelineKey::kPassAvatarBase
                                               : plVulkanPipelineKey::kPassAvatar;

                        // Element geometry is in layout units; fOrigWidth normalizes
                        // it, and the doubling plus the shift put it in clip space.
                        const float screenW = float(item->fElements[j]->fWidth) /
                                              layout->fOrigWidth * 2.f;
                        const float screenH = float(item->fElements[j]->fHeight) /
                                              layout->fOrigWidth * 2.f;
                        const float screenX = float(item->fElements[j]->fXPos) /
                                              layout->fOrigWidth * 2.f - 1.f;
                        const float screenY =
                            (1.f - float(item->fElements[j]->fYPos) / layout->fOrigWidth) * 2.f -
                            1.f - screenH;

                        IDrawClothingQuad(cmd, passKind, screenX, screenY, screenW, screenH,
                                          uOff, vOff, item->fTextures[j][k], tint);
                    }
                }
            }
        }

        PopRenderTarget();
        co->fDirtyItems.Clear();
    }

    fView.fXformResetFlags = fView.kResetAll;

    // This frame's outfits become next frame's "previous", which is what lets
    // IClearClothingOutfits free the ones that stopped being used.
    fClothingOutfits.swap(fPrevClothingOutfits);

    // The composite changed the render target, the pipeline and the vertex
    // buffer, so nothing cached about the scene render survives.
    fState.Reset();
}

// Ported from plMetalPipeline::IDrawClothingQuad (:2917-2938).
void plVulkanPipeline::IDrawClothingQuad(VkCommandBuffer cmd, uint8_t passKind,
                                         float x, float y, float w, float h,
                                         float uOff, float vOff, plMipmap* tex,
                                         const hsColorRGBA& tint)
{
    plVulkanTextureRef* ref = static_cast<plVulkanTextureRef*>(tex->GetDeviceRef());
    if (!ref || ref->IsDirty()) {
        CheckTextureRef(tex);
        ref = static_cast<plVulkanTextureRef*>(tex->GetDeviceRef());
    }
    if (!ref)
        return;
    if (ref->fImageView == VK_NULL_HANDLE)
        IReloadTexture(tex, ref);
    if (ref->fImageView == VK_NULL_HANDLE)
        return;

    plVulkanPipelineKey key{};
    key.fPassKind = passKind;
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

    // A 2D composite; the avatar targets have no depth attachment at all.
    vkCmdSetDepthTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
    vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
    vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);

    // Four corners in triangle-strip order, exactly Metal's P1, P2, P0, P3.
    plVulkanAvatarVert verts[4];

    plVulkanAvatarVert corner;
    corner.fPos[0] = x;
    corner.fPos[1] = y;
    corner.fUv[0] = uOff;
    corner.fUv[1] = 1.f + vOff;

    verts[2] = corner;

    verts[0] = corner;
    verts[0].fPos[0] += w;
    verts[0].fUv[0] += 1.f;

    verts[1] = corner;
    verts[1].fPos[0] += w;
    verts[1].fUv[0] += 1.f;
    verts[1].fPos[1] += h;
    verts[1].fUv[1] -= 1.f;

    verts[3] = corner;
    verts[3].fPos[1] += h;
    verts[3].fUv[1] -= 1.f;

    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(verts));

    // The tint rides in the material block's diffuse slot rather than getting a
    // binding of its own; nothing else in this pass reads that block.
    plMaterialLightingDescriptor material{};
    material.diffuseCol = { tint.r, tint.g, tint.b, tint.a };

    plVulkanDevice::plScratchAlloc uniformAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!vertexAlloc.IsValid() || !uniformAlloc.IsValid() || !materialAlloc.IsValid())
        return;

    memcpy(vertexAlloc.fMapped, verts, sizeof(verts));
    memset(uniformAlloc.fMapped, 0, sizeof(VertexUniforms));
    memcpy(materialAlloc.fMapped, &material, sizeof(material));

    const plVulkanTextureRef* layers[1] = { ref };
    const uint8_t clampFlags[1] = { 0 };

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    VkDescriptorSet textureSet = fDevice.GetTextureDescriptorSet(
        layers, clampFlags, 1, nullptr, nullptr, ST_LITERAL("the avatar clothing composite"));
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return;

    // The avatar composite reads only the first two; the rest of set 0's layout
    // still has to be named, and nothing reads where they point.
    const uint32_t dynamicOffsets[6] = { uniformAlloc.fOffset, materialAlloc.fOffset,
                                         0, 0, 0, 0 };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

    const VkDeviceSize offset = vertexAlloc.fOffset;
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexAlloc.fBuffer, &offset);
    fState.fVertexBuffer = vertexAlloc.fBuffer;

    vkCmdDraw(cmd, 4, 1, 0, 0);
}
