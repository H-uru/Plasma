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

// Console and debug text. Ported from plMetalTextFont.

#include "plVulkanTextFont.h"
#include "plVulkanPipeline.h"

#include <algorithm>
#include <cstring>
#include <memory>

plVulkanTextFont::plVulkanTextFont(plPipeline* pipe, plVulkanDevice* device)
    : plTextFont(pipe), fDevice(device), fVulkanPipeline(static_cast<plVulkanPipeline*>(pipe))
{
    fTexture.fDevice = device;
}

plVulkanTextFont::~plVulkanTextFont()
{
    DestroyObjects();
}

void plVulkanTextFont::ICreateTexture(uint16_t* data)
{
    fTexture.Release();

    fTexture.fDevice = fDevice;
    fTexture.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    fTexture.fWidth = fTextureWidth;
    fTexture.fHeight = fTextureHeight;
    fTexture.fLevels = 0;
    fTexture.fIsCubic = false;

    // plTextFont::IInitFontTexture builds a 4-bit-per-channel atlas. Widen each
    // nibble by replicating it, so 0xF becomes 0xFF rather than being scaled by
    // 255 and truncated -- which is what the Metal port does, and why its glyph
    // edges are subtly wrong.
    struct plARGB4444 { uint8_t a : 4, r : 4, g : 4, b : 4; };

    const size_t texels = size_t(fTextureWidth) * fTextureHeight;
    auto expanded = std::make_unique<uint8_t[]>(texels * 4);

    for (size_t i = 0; i < texels; i++) {
        const plARGB4444* in = reinterpret_cast<const plARGB4444*>(data + i);
        uint8_t* out = expanded.get() + i * 4;
        out[0] = uint8_t(in->r | (in->r << 4));
        out[1] = uint8_t(in->g | (in->g << 4));
        out[2] = uint8_t(in->b | (in->b << 4));
        out[3] = uint8_t(in->a | (in->a << 4));
    }

    fDevice->CreateTextureFromMemory(&fTexture, expanded.get(), texels * 4,
                                     ST_LITERAL("text font atlas"));
}

void plVulkanTextFont::IInitStateBlocks()
{
}

void plVulkanTextFont::DestroyObjects()
{
    fTexture.Release();
    fInitialized = false;
}

void plVulkanTextFont::IDraw(uint32_t vertexCount, plFontVertex* array, bool lines)
{
    if (vertexCount == 0 || !array)
        return;

    VkCommandBuffer cmd = fDevice->CurrentCommandBuffer();
    if (cmd == VK_NULL_HANDLE || fTexture.fImageView == VK_NULL_HANDLE)
        return;

    plVulkanDebugLabel label(*fDevice, cmd, ST_LITERAL("Draw text"));

    plVulkanPipelineKey key{};
    key.fPassKind = lines ? plVulkanPipelineKey::kPassTextLines
                          : plVulkanPipelineKey::kPassText;
    // Text is alpha blended over whatever is behind it.
    key.fBlendFlags[0] = hsGMatState::kBlendAlpha;
    key.fColorFormat = fDevice->CurrentColorFormat();
    key.fDepthFormat = fDevice->CurrentDepthFormat();
    key.fSampleCount = fDevice->CurrentSampleCount();

    VkPipeline pipeline = fDevice->GetPipelineState(key);
    if (pipeline == VK_NULL_HANDLE)
        return;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdSetDepthTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
    vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);
    vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);

    // Screen pixels straight to clip space. Note the text shader multiplies
    // M * v, unlike the rest of the backend, so this is a plain column-major
    // ortho rather than one of Plasma's row-major matrices.
    VertexUniforms uniforms{};
    memset(&uniforms, 0, sizeof(uniforms));
    uniforms.projectionMatrix.m[0] = 2.f / float(fPipe->Width());
    uniforms.projectionMatrix.m[5] = -2.f / float(fPipe->Height());
    uniforms.projectionMatrix.m[10] = 1.f;
    uniforms.projectionMatrix.m[12] = -1.f;
    uniforms.projectionMatrix.m[13] = 1.f;
    uniforms.projectionMatrix.m[15] = 1.f;

    plVulkanDevice::plScratchAlloc uniformAlloc =
        fDevice->AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice->AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!uniformAlloc.IsValid() || !materialAlloc.IsValid())
        return;

    memcpy(uniformAlloc.fMapped, &uniforms, sizeof(uniforms));
    memset(materialAlloc.fMapped, 0, sizeof(plMaterialLightingDescriptor));

    VkDescriptorSet uniformSet = fDevice->GetUniformDescriptorSet();
    const plVulkanTextureRef* layers[1] = { &fTexture };
    VkDescriptorSet textureSet = fDevice->GetTextureDescriptorSet(
        layers, nullptr, 1, nullptr, nullptr, ST_LITERAL("the text font"));
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return;

    // Text is unlit, but the layout still has the light binding, so it needs an
    // offset. Any valid one will do; nothing reads through it.
    const uint32_t dynamicOffsets[6] = { uniformAlloc.fOffset, materialAlloc.fOffset,
                                         0, 0, 0, 0 };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            fDevice->GetPipelineLayout(), 0, 2, sets, 6, dynamicOffsets);

    // The vertices are transient, so they ride the frame's scratch ring rather
    // than a buffer of their own. Batch if a run is bigger than one block.
    const size_t vertexSize = sizeof(plFontVertex);
    uint32_t drawn = 0;
    while (drawn < vertexCount) {
        const uint32_t batch = std::min<uint32_t>(vertexCount - drawn, 2048);

        plVulkanDevice::plScratchAlloc verts = fDevice->AllocateScratch(batch * vertexSize);
        if (!verts.IsValid())
            return;

        memcpy(verts.fMapped, array + drawn, batch * vertexSize);

        const VkDeviceSize offset = verts.fOffset;
        vkCmdBindVertexBuffers(cmd, 0, 1, &verts.fBuffer, &offset);
        vkCmdDraw(cmd, batch, 1, 0, 0);

        drawn += batch;
    }
}

void plVulkanTextFont::IDrawPrimitive(uint32_t count, plFontVertex* array)
{
    // count is triangles, three vertices each.
    IDraw(count * 3, array, false);
}

void plVulkanTextFont::IDrawLines(uint32_t count, plFontVertex* array)
{
    IDraw(count * 2, array, true);
}

void plVulkanTextFont::FlushDraws()
{
    // Nothing is buffered between calls; each IDraw is self-contained.
}

void plVulkanTextFont::SaveStates()
{
    // State is set per draw rather than latched, so there is nothing to save.
    // The pipeline's redundant-bind filter has to be told, though, since we
    // bind our own pipeline and vertex buffer behind its back.
    if (fVulkanPipeline)
        fVulkanPipeline->InvalidateBoundState();
}

void plVulkanTextFont::RestoreStates()
{
    if (fVulkanPipeline)
        fVulkanPipeline->InvalidateBoundState();
}
