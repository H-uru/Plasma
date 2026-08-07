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

#ifndef _plVulkanPipelineState_h_
#define _plVulkanPipelineState_h_

#include "ShaderSrc/plVulkanShaderTypes.h"

#include <volk.h>

#include <cstddef>
#include <cstdint>

class plLayerInterface;
class plVulkanVertexBufferRef;

/**
 * Everything a VkPipeline has to be baked with.
 *
 * Derived from plMetalFragmentShaderDescription (plMetalPipelineState.h:144-193)
 * and plMetalPipelineRecord (plMetalDevice.h:213-221), plus the pieces Metal
 * leaves to encoder state but Vulkan bakes in.
 *
 * Deliberately absent, because they are dynamic state or uniforms:
 *   hsGMatZFlags::kZMask   -> dynamic depth test/write/compare
 *   kZIncLayer             -> dynamic depth bias
 *   kMiscTwoSided + parity -> dynamic cull mode
 *   hsGMatClampFlags       -> one of four pre-baked samplers
 *   colors, matrices, UVW src, texture transforms -> uniform buffers
 *
 * Compared bit for bit, so it must have no padding holes; Canonicalize() zeroes
 * anything that does not apply so equivalent states share a cache entry.
 */
struct plVulkanPipelineKey
{
    enum PassKind : uint8_t
    {
        kPassMaterial = 0,
        kPassPlate,
        kPassText,
        kPassTextLines,

        /** A shadow caster rendered into a shadow map. */
        kPassShadowCaster,

        /** A shadow map projected onto a span that receives it. */
        kPassShadowApply,

        /** One clothing element composited onto an avatar's texture. */
        kPassAvatar,

        /** As above, but the base layer, which replaces rather than blends. */
        kPassAvatarBase,

        /** Separable fullscreen Gaussian blur. */
        kPassBlur,

        /** Gamma-LUT resolve from the scene image into the swapchain. */
        kPassGamma
    };

    uint8_t  fPassKind;

    // Vertex layout. Must be read from the buffer ref's format, not the
    // group's: SetupVertexBufferRef strips the skinning bits out.
    uint8_t  fNumUVs;
    uint8_t  fNumWeights;
    uint8_t  fHasSkinIndices;

    // Material.
    uint8_t  fNumLayers;
    uint8_t  fUsePerPixelLighting;
    uint8_t  fWireFrame;

    /** Shadow apply only: a point light's map is projected, so it needs a divide. */
    uint8_t  fPointLightCast;

    uint8_t  fPassTypes[kMaxLayers];
    uint32_t fBlendFlags[kMaxLayers];
    uint32_t fMiscFlags[kMaxLayers];

    // Render pass compatibility.
    uint32_t fColorFormat;
    uint32_t fDepthFormat;
    uint32_t fSampleCount;

    /** Shadow apply only: which material layer supplies alpha, or -1 for none. */
    int32_t  fShadowAlphaSrc;

    /**
     * plShaderID::ID of the programmable pair, or 0 for the fixed pipeline.
     *
     * Unlike everything else here these are not shader inputs; they select which
     * shader modules the pipeline is built from at all.
     */
    uint16_t fVertexShaderID;
    uint16_t fFragmentShaderID;

    /** Zeroes everything that does not apply, so equivalent states collapse. */
    void Canonicalize();

    bool operator==(const plVulkanPipelineKey& rhs) const;
};

static_assert(sizeof(plVulkanPipelineKey) ==
                  8 + kMaxLayers + (2 * 4 * kMaxLayers) + 20,
              "plVulkanPipelineKey must not have padding holes; it is hashed as bytes");

struct plVulkanPipelineKeyHash
{
    size_t operator()(const plVulkanPipelineKey& key) const noexcept;
};

/**
 * Fills in the vertex-layout half of a key from a buffer ref.
 *
 * The offsets this implies must agree with
 * plMetalRenderSpanPipelineState::ConfigureVertexDescriptor
 * (plMetalPipelineState.cpp:125-164), which is the authority on the interleaved
 * layout -- it is not the order plGBufferGroup::ICalcVertexSize declares.
 */
void plVulkanFillVertexKey(plVulkanPipelineKey& key, const plVulkanVertexBufferRef* vRef);

#endif // _plVulkanPipelineState_h_
