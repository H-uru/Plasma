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

#ifndef _plVulkanMaterialShaderRef_h_
#define _plVulkanMaterialShaderRef_h_

#include "plVulkanDeviceRef.h"
#include "plVulkanPipelineState.h"

#include "hsGMatState.h"

#include <vector>

class hsGMaterial;
class plBitmap;
class plLayerInterface;
class plVulkanPipeline;
class plVulkanRenderTargetRef;
class plVulkanTextureRef;

/**
 * Everything one draw needs to know about the material it is drawing.
 *
 * The layers this describes may be override-wrapped, and those wrappers are
 * unwound as soon as the pass is built, so nothing here is a plLayerInterface --
 * every layer's contribution is already resolved into flags, a texture and a
 * transform. fBaseLayer is the sole exception: it is the material's own layer,
 * unwrapped, which outlives the pass and is what lighting is calculated from.
 */
struct plVulkanPassInfo
{
    plVulkanPipelineKey       fKey;
    float                     fAlphaThreshold;
    uint32_t                  fNumLayers;
    const plVulkanTextureRef* fTextures[kMaxLayers];
    const plVulkanRenderTargetRef* fRenderTargets[kMaxLayers];
    uint8_t                   fClampFlags[kMaxLayers];
    UVOutDescriptor           fUVTransforms[kMaxLayers];
    plLayerInterface*         fBaseLayer;
};

/**
 * A material decomposed into render passes.
 *
 * Plasma materials carry up to N layers, but only so many can be combined in one
 * draw: some blend modes cannot sit above others, and a layer without a texture
 * ends a pass. This walks the layer list once and records where each pass starts
 * and how long it is, so the draw path can just iterate passes.
 *
 * The cached decomposition assumes no piggybacks and no override layers, which
 * is the common case. When either is active the draw path rebuilds the pass on
 * the spot, because both change what the pass contains.
 *
 * Ported from plMetalMaterialShaderRef. Cached on the material and rebuilt when
 * marked dirty, because plLayerInterface channels are animatable.
 */
class plVulkanMaterialShaderRef : public plVulkanDeviceRef
{
public:
    plVulkanMaterialShaderRef(hsGMaterial* material, plVulkanPipeline* pipeline);
    ~plVulkanMaterialShaderRef() override;

    void Release() override;

    void Link(plVulkanMaterialShaderRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanMaterialShaderRef* GetNext() const { return (plVulkanMaterialShaderRef*)fNext; }

    /** Rebuilds the pass list if the material changed. Call once per span. */
    void CheckMaterialRef();

    size_t GetNumPasses() const { return fPasses.size(); }

    /** Index of the pass's first layer within the material. */
    uint32_t GetPassIndex(size_t pass) const { return fPasses[pass].fFirstLayer; }

    /** How many of the material's own layers the pass combines. */
    uint32_t GetPassLength(size_t pass) const { return fPasses[pass].fNumLayers; }

    /**
     * Resolves a pass for drawing.
     *
     * `piggybacks` is appended after the material's own layers, and when
     * `applyOverrides` the pipeline's over-base and over-all layers wrap each
     * one. Returns false if the pass should not be drawn at all.
     */
    bool ResolvePass(size_t pass, const std::vector<plLayerInterface*>* piggybacks,
                     bool applyOverrides, plVulkanPassInfo* out);

    hsGMaterial* GetMaterial() const { return fMaterial; }

    /** Called when the pipeline goes away, so a surviving material forgets it. */
    void Orphan() { fPipeline = nullptr; }

protected:
    struct plPassRange
    {
        uint32_t fFirstLayer;
        uint32_t fNumLayers;
    };

    /** Walks every layer, carving the material into passes. */
    void ILoopOverLayers();

    /**
     * Builds one pass starting at a layer.
     *
     * Returns the index of the first layer it did not consume, or -1 to stop.
     * `out` may be null, which just decides the extent without resolving.
     */
    uint32_t IHandleMaterial(uint32_t layer, const std::vector<plLayerInterface*>* piggybacks,
                             bool applyOverrides, plVulkanPassInfo* out);

    /** How many layers starting at `which` can be combined in one draw. */
    uint32_t ILayersAtOnce(uint32_t which);

    /** Whether a layer can be folded into the pass above it. */
    bool ICanEatLayer(plLayerInterface* lay);

    /** The layer's state composited with the pipeline's material overrides. */
    hsGMatState ICompositeLayerState(const plLayerInterface* layer) const;

    /** Resolves one layer into the pass's key, texture and UV transform. */
    void IResolveLayer(plVulkanPassInfo* out, plLayerInterface* layer, uint32_t index);

    /** Reports, once per texture, a layer the device has no image for. */
    void IWarnMissingTexture(const plBitmap* texture) const;

    hsGMaterial*      fMaterial;
    plVulkanPipeline* fPipeline;

    std::vector<plPassRange> fPasses;
};

#endif // _plVulkanMaterialShaderRef_h_
