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

#include "plVulkanMaterialShaderRef.h"

#include "plVulkanDevice.h"
#include "plVulkanDeviceRef.h"
#include "plVulkanPipeline.h"

#include "hsGMatState.h"
#include "plDrawable/plGeometrySpan.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plDebugText.h"
#include "plPipeDebugFlags.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include <string_theory/format>

#include <cstring>
#include <set>

plVulkanMaterialShaderRef::plVulkanMaterialShaderRef(hsGMaterial* material, plVulkanPipeline* pipeline)
    : fMaterial(material), fPipeline(pipeline)
{
    CheckMaterialRef();
}

plVulkanMaterialShaderRef::~plVulkanMaterialShaderRef()
{
    Release();
}

void plVulkanMaterialShaderRef::Release()
{
    fPasses.clear();
}

void plVulkanMaterialShaderRef::CheckMaterialRef()
{
    // Something like an avatar may have swapped our textures out from under us.
    if (IsDirty())
        fPasses.clear();

    if (fPasses.empty()) {
        ILoopOverLayers();

        // Everything the material can draw needs an uploaded texture, whether
        // or not this frame's passes reach it.
        for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
            if (plLayerInterface* layer = fMaterial->GetLayer(i))
                fPipeline->CheckTextureRef(layer);
        }
    }

    SetDirty(false);
}

// Ported from plMetalMaterialShaderRef::ILoopOverLayers (:238-300).
void plVulkanMaterialShaderRef::ILoopOverLayers()
{
    for (uint32_t j = 0; j < fMaterial->GetNumLayers();) {
        const uint32_t currLayer = j;

        j = IHandleMaterial(currLayer, nullptr, false, nullptr);
        if (j == uint32_t(-1))
            break;

        fPasses.push_back({ currLayer, j - currLayer });
    }
}

bool plVulkanMaterialShaderRef::ResolvePass(size_t pass,
                                            const std::vector<plLayerInterface*>* piggybacks,
                                            bool applyOverrides, plVulkanPassInfo* out)
{
    return IHandleMaterial(fPasses[pass].fFirstLayer, piggybacks, applyOverrides, out)
           != uint32_t(-1);
}

hsGMatState plVulkanMaterialShaderRef::ICompositeLayerState(const plLayerInterface* layer) const
{
    hsGMatState state;
    state.Composite(layer->GetState(), fPipeline->GetMaterialOverride(true),
                    fPipeline->GetMaterialOverride(false));
    return state;
}

/**
 * Records one layer's contribution to the pass.
 *
 * `layer` may be an override wrapper that is about to be unwound, so everything
 * it carries has to be read out here and now.
 */
void plVulkanMaterialShaderRef::IResolveLayer(plVulkanPassInfo* out, plLayerInterface* layer,
                                              uint32_t index)
{
    if (!out || index >= kMaxLayers)
        return;

    out->fKey.fBlendFlags[index] = layer->GetBlendFlags();
    out->fKey.fMiscFlags[index] = layer->GetMiscFlags();

    out->fClampFlags[index] = uint8_t(layer->GetClampFlags());
    out->fUVTransforms[index].UVWSrc = layer->GetUVWSrc();
    IToShaderMatrix(layer->GetTransform(), out->fUVTransforms[index].transform);

    // Ported from plMetalFragmentShaderDescription::PopulateTextureInfo (:301-316).
    plBitmap* texture = layer->GetTexture();
    if (!texture) {
        out->fKey.fPassTypes[index] = kPassTypeColor;
        out->fTextures[index] = nullptr;
        out->fRenderTargets[index] = nullptr;
        return;
    }

    fPipeline->CheckTextureRef(layer);
    plRenderTarget* renderTarget = plRenderTarget::ConvertNoRef(texture);
    plVulkanTextureRef* texRef = renderTarget ? nullptr
        : static_cast<plVulkanTextureRef*>(texture->GetDeviceRef());
    plVulkanRenderTargetRef* targetRef = renderTarget
        ? static_cast<plVulkanRenderTargetRef*>(texture->GetDeviceRef()) : nullptr;

    // A texture the device never accepted has no view to bind. Keeping a texture
    // pass type here would build a pipeline that samples a descriptor slot
    // GetTextureDescriptorSet leaves unwritten -- the binding is PARTIALLY_BOUND,
    // so nothing complains and the layer renders from undefined contents. Fall
    // back to an untextured layer instead, and say why once.
    const VkImageView view = texRef ? texRef->fImageView
                                    : (targetRef ? targetRef->fImageView : VK_NULL_HANDLE);
    if (view == VK_NULL_HANDLE) {
        IWarnMissingTexture(texture);
        out->fKey.fPassTypes[index] = kPassTypeColor;
        out->fTextures[index] = nullptr;
        out->fRenderTargets[index] = nullptr;
        return;
    }

    if (plCubicEnvironmap::ConvertNoRef(texture) || plCubicRenderTarget::ConvertNoRef(texture))
        out->fKey.fPassTypes[index] = kPassTypeCubicTexture;
    else if (plMipmap::ConvertNoRef(texture) || plRenderTarget::ConvertNoRef(texture))
        out->fKey.fPassTypes[index] = kPassTypeTexture;
    else
        out->fKey.fPassTypes[index] = kPassTypeColor;

    out->fTextures[index] = texRef;
    out->fRenderTargets[index] = targetRef;
}

/**
 * Reports a layer whose texture has no image on the device.
 *
 * Once per texture, not once per draw: this is evaluated for every pass of every
 * span, so an unreported avatar texture would otherwise fill the log by itself.
 */
void plVulkanMaterialShaderRef::IWarnMissingTexture(const plBitmap* texture) const
{
#ifndef PLASMA_EXTERNAL_RELEASE
    static std::set<ST::string> s_reported;

    const ST::string name = texture->GetKeyName();
    if (!s_reported.insert(name).second)
        return;

    hsStatusMessageF("Vulkan: material '{}' wants texture '{}', which has no image on the "
                     "device; the layer will draw untextured",
                     fMaterial->GetKeyName(), name);
#endif
}

// Ported from plMetalMaterialShaderRef::ILayersAtOnce (:342-388).
uint32_t plVulkanMaterialShaderRef::ILayersAtOnce(uint32_t which)
{
    uint32_t currNumLayers = 1;

    plLayerInterface* lay = fMaterial->GetLayer(which);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoMultitexture))
        return currNumLayers;

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) ||
         fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) &&
        (lay->GetMiscFlags() & hsGMatState::kMiscBumpChans)) {
        return 2;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) ||
        (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner)) {
        return currNumLayers;
    }

    // Reserve room for piggybacks, which are appended to every pass. DX reserves
    // fActivePiggyBacks (plDXPipeline.cpp:7176); this reserves the maximum
    // instead, because the decomposition is cached on the material and the
    // active count changes from span to span.
    uint32_t maxLayers = fPipeline->GetMaxLayersAtOnce() - fPipeline->GetMaxPiggyBacks();
    if (which + maxLayers > fMaterial->GetNumLayers())
        maxLayers = uint32_t(fMaterial->GetNumLayers()) - which;

    for (uint32_t i = currNumLayers; i < maxLayers; i++) {
        plLayerInterface* nextLay = fMaterial->GetLayer(which + i);

        // A layer that binds the next one cannot be the last in the pass.
        if ((nextLay->GetMiscFlags() & hsGMatState::kMiscBindNext) && (i + 1 >= maxLayers))
            break;

        if (nextLay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere)
            break;

        if (!(fMaterial->GetLayer(which + i - 1)->GetMiscFlags() & hsGMatState::kMiscBindNext) &&
            !ICanEatLayer(nextLay)) {
            break;
        }

        currNumLayers++;
    }

    return currNumLayers;
}

// Ported from plMetalMaterialShaderRef::ICanEatLayer (:390-411).
bool plVulkanMaterialShaderRef::ICanEatLayer(plLayerInterface* lay)
{
    if (!lay->GetTexture())
        return false;

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) ||
        (lay->GetBlendFlags() & hsGMatState::kBlendAddColorTimesAlpha) ||
        (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner)) {
        return false;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendAlpha) && (lay->GetAmbientColor().a < 1.f))
        return false;

    // A layer that writes depth has to be its own pass, or the layers folded in
    // with it would each write the same fragment.
    if (!(lay->GetZFlags() & hsGMatState::kZNoZWrite))
        return false;

    return true;
}

// Ported from plMetalMaterialShaderRef::IHandleMaterial (:413-537). Metal takes
// pre/post encode callbacks; here the override push and pop are inline, because
// the only two callers want exactly the two behaviours below.
uint32_t plVulkanMaterialShaderRef::IHandleMaterial(uint32_t layer,
                                                    const std::vector<plLayerInterface*>* piggybacks,
                                                    bool applyOverrides, plVulkanPassInfo* out)
{
    if (!fMaterial || layer >= fMaterial->GetNumLayers() || !fMaterial->GetLayer(layer))
        return uint32_t(-1);

    if (out) {
        memset(out, 0, sizeof(plVulkanPassInfo));
        out->fKey.fPassKind = plVulkanPipelineKey::kPassMaterial;
    }

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoDecals) &&
        (fMaterial->GetCompositeFlags() & hsGMaterial::kCompDecal)) {
        return uint32_t(-1);
    }

    plLayerInterface* currLay = fMaterial->GetLayer(layer);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW) &&
        (currLay->GetMiscFlags() & hsGMatState::kMiscBumpDu)) {
        currLay = fMaterial->GetLayer(++layer);
        if (!currLay)
            return uint32_t(-1);
    }

    if (out)
        out->fBaseLayer = currLay;

    hsGMatState state = ICompositeLayerState(currLay);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagDisableSpecular))
        state.fShadeFlags &= ~hsGMatState::kShadeSpecular;

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoAlphaBlending))
        state.fBlendFlags &= ~hsGMatState::kBlendMask;

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) ||
         fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) &&
        (state.fMiscFlags & hsGMatState::kMiscBumpChans)) {
        switch (state.fMiscFlags & hsGMatState::kMiscBumpChans) {
        case hsGMatState::kMiscBumpDv:
            if (!(fMaterial->GetLayer(layer - 2)->GetBlendFlags() & hsGMatState::kBlendAdd)) {
                state.fBlendFlags &= ~hsGMatState::kBlendMask;
                state.fBlendFlags |= hsGMatState::kBlendMADD;
            }
            break;
        case hsGMatState::kMiscBumpDw:
            if (!(fMaterial->GetLayer(layer - 1)->GetBlendFlags() & hsGMatState::kBlendAdd)) {
                state.fBlendFlags &= ~hsGMatState::kBlendMask;
                state.fBlendFlags |= hsGMatState::kBlendMADD;
            }
            break;
        default:
            break;
        }
    }

    const uint32_t currNumLayers = ILayersAtOnce(layer);

    if (out) {
        // The material's own layers.
        uint32_t index = 0;
        for (; index < currNumLayers && index < kMaxLayers; index++) {
            plLayerInterface* layPtr = fMaterial->GetLayer(layer + index);
            if (!layPtr)
                return uint32_t(-1);

            if (applyOverrides) {
                if (index == 0)
                    layPtr = fPipeline->IPushOverBaseLayer(layPtr);
                layPtr = fPipeline->IPushOverAllLayer(layPtr);
            }

            IResolveLayer(out, layPtr, index);

            if (applyOverrides) {
                fPipeline->IPopOverAllLayer(layPtr);
                if (index == 0)
                    fPipeline->IPopOverBaseLayer(layPtr);
            }
        }

        // Then whatever is riding along on every pass.
        if (piggybacks) {
            for (size_t pb = 0; pb < piggybacks->size() && index < kMaxLayers; pb++, index++) {
                plLayerInterface* layPtr = (*piggybacks)[pb];
                if (!layPtr)
                    return uint32_t(-1);

                if (applyOverrides)
                    layPtr = fPipeline->IPushOverAllLayer(layPtr);

                IResolveLayer(out, layPtr, index);

                if (applyOverrides)
                    fPipeline->IPopOverAllLayer(layPtr);
            }
        }

        out->fNumLayers = index;
        out->fKey.fNumLayers = uint8_t(index);

        // The pass's blend against the framebuffer is layer 0's, but its own
        // combiner input is not -- blendFirst reads fBlendFlags[0] too, so the
        // composited state has to be what lands in the key.
        out->fKey.fBlendFlags[0] = state.fBlendFlags;
        out->fKey.fMiscFlags[0] = state.fMiscFlags;
        out->fKey.fWireFrame = (state.fMiscFlags & hsGMatState::kMiscWireFrame) ? 1 : 0;

        // AlphaTestHigh reduces sort artifacts on mostly-opaque textures with a
        // translucent fringe, like a bush billboard: it allows some falloff but
        // stops drawing before halos become visible.
        if ((state.fBlendFlags & (hsGMatState::kBlendTest | hsGMatState::kBlendAlpha |
                                  hsGMatState::kBlendAddColorTimesAlpha)) &&
            !(state.fBlendFlags & hsGMatState::kBlendAlphaAlways)) {
            out->fAlphaThreshold =
                (state.fBlendFlags & hsGMatState::kBlendAlphaTestHigh) ? 64.f / 255.f : 1.f / 255.f;
        } else {
            out->fAlphaThreshold = 0.f;
        }
    }

    return layer + currNumLayers;
}
