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

#include "plVulkanPipelineState.h"
#include "plVulkanDeviceRef.h"

#include "hsGMatState.h"

#include "plDrawable/plGBufferGroup.h"

#include <cstring>

void plVulkanPipelineKey::Canonicalize()
{
    // Plates have no fixed-pipeline layer combiners, but their base layer still
    // selects how the finished fragment blends with the framebuffer. Preserve
    // that value before removing the otherwise-unused layer fields below.
    const uint32_t framebufferBlend = fBlendFlags[0];

    // Layers past the count contribute nothing, so zero them rather than let
    // stale values split the cache.
    for (uint8_t i = fNumLayers; i < kMaxLayers; i++) {
        fPassTypes[i] = 0;
        fBlendFlags[i] = 0;
        fMiscFlags[i] = 0;
    }

    // Only the base layer's blend reaches the framebuffer blend state
    // (plMetalPipelineState.cpp:288-292); the rest are combined in the shader,
    // which the fragment stage already keys on separately.
    // A programmable pass has its own shaders; only a material pass can.
    if (fPassKind != kPassMaterial) {
        fVertexShaderID = 0;
        fFragmentShaderID = 0;
    }

    if (fPassKind != kPassShadowApply) {
        fPointLightCast = 0;
        fShadowAlphaSrc = 0;
    }

    // These three describe the vertex buffer, not the shader: IBuildPipeline
    // derives the binding stride and the attribute offsets from them
    // (plVulkanPipelineCache.cpp:698-704). Every pass that draws a
    // plGBufferGroup-formatted buffer has to keep them, or its pipeline reads
    // that buffer at the wrong stride. Passes with a vertex layout of their own
    // -- plates, text, avatar, fullscreen -- do not care.
    const bool geometryVertices = fPassKind == kPassMaterial ||
                                  fPassKind == kPassShadowApply ||
                                  fPassKind == kPassShadowCaster;
    if (!geometryVertices) {
        fNumUVs = 0;
        fNumWeights = 0;
        fHasSkinIndices = 0;
    }

    // The shadow-apply shader keys on the same per-layer state a material does,
    // so it keeps its layer fields. Other special passes have no layer-combiner
    // state -- including the shadow caster, whose shader takes no specialization
    // at all; plates retain only the framebuffer blend restored below.
    if (fPassKind != kPassMaterial && fPassKind != kPassShadowApply) {
        fNumLayers = 0;
        fUsePerPixelLighting = 0;
        for (uint8_t i = 0; i < kMaxLayers; i++) {
            fPassTypes[i] = 0;
            fBlendFlags[i] = 0;
            fMiscFlags[i] = 0;
        }

        if (fPassKind == kPassPlate)
            fBlendFlags[0] = framebufferBlend;
    }
}

bool plVulkanPipelineKey::operator==(const plVulkanPipelineKey& rhs) const
{
    return memcmp(this, &rhs, sizeof(plVulkanPipelineKey)) == 0;
}

size_t plVulkanPipelineKeyHash::operator()(const plVulkanPipelineKey& key) const noexcept
{
    // FNV-1a over the whole canonicalized struct. Deliberately not Metal's
    // XOR-fold (plMetalPipelineState.h:166-189), which hashes fNumLayers twice
    // -- cancelling it out -- and has no positional mixing, so permuted layers
    // collide.
    constexpr uint64_t kOffsetBasis = 0xcbf29ce484222325ull;
    constexpr uint64_t kPrime = 0x100000001b3ull;

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&key);
    uint64_t hash = kOffsetBasis;
    for (size_t i = 0; i < sizeof(plVulkanPipelineKey); i++) {
        hash ^= bytes[i];
        hash *= kPrime;
    }

    return size_t(hash);
}

void plVulkanFillVertexKey(plVulkanPipelineKey& key, const plVulkanVertexBufferRef* vRef)
{
    const uint8_t format = vRef->fFormat;

    key.fNumUVs = plGBufferGroup::CalcNumUVs(format);
    key.fNumWeights = (format & plGBufferGroup::kSkinWeightMask) >> 4;
    key.fHasSkinIndices = (format & plGBufferGroup::kSkinIndices) ? 1 : 0;
}
