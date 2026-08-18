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

// Lighting. Ported from plMetalPipeline's light handling.
//
// Every registered light is flattened into one buffer per frame; a draw then
// names the subset that reaches it by index, so the per-draw payload stays small
// no matter how many lights the age has.

#include "plVulkanPipeline.h"

#include "plPipeDebugFlags.h"

#include "plDrawable/plSpanTypes.h"
#include "plGLight/plLightInfo.h"

#include <algorithm>
#include <cstring>

hsGDeviceRef* plVulkanPipeline::IMakeLightRef(plLightInfo* owner)
{
    plVulkanLightRef* lRef = new plVulkanLightRef();

    lRef->fOwner = owner;
    owner->SetDeviceRef(lRef);

    // The scene graph owns the ref from here; we only hold it through the list.
    lRef->UnRef();
    lRef->Link(&fLightRefList);

    return lRef;
}

void plVulkanPipeline::RegisterLight(plLightInfo* liInfo)
{
    if (liInfo->GetDeviceRef())
        return;

    pl3DPipeline::RegisterLight(liInfo);
    liInfo->SetDeviceRef(IMakeLightRef(liInfo));
}

void plVulkanPipeline::UnRegisterLight(plLightInfo* liInfo)
{
    pl3DPipeline::UnRegisterLight(liInfo);
}

// Ported from plMetalPipeline::LoadLightsOnDevice (plMetalPipeline.cpp:1743).
void plVulkanPipeline::LoadLightsOnDevice()
{
    // Count first: the buffer has to be one suballocation so a single dynamic
    // offset covers the whole array the shader indexes into.
    uint32_t count = 0;
    for (plVulkanLightRef* light = fLightRefList; light; light = light->GetNext()) {
        if (count < kMaxSceneLights)
            count++;
    }

    fLightBufferCount = count;

    // The shader always has the binding, so give it at least one element rather
    // than a null buffer.
    const size_t bytes = std::max<size_t>(count, 1) * sizeof(plShaderLightSource);

    fLightBuffer = fDevice.AllocateScratch(
        bytes, sizeof(plShaderLightSource) * kMaxSceneLights);
    if (!fLightBuffer.IsValid()) {
        fLightBufferCount = 0;
        for (plVulkanLightRef* light = fLightRefList; light; light = light->GetNext())
            light->fBufferIndex = UINT32_MAX;
        return;
    }

    memset(fLightBuffer.fMapped, 0, bytes);

    plShaderLightSource* dst = static_cast<plShaderLightSource*>(fLightBuffer.fMapped);
    uint32_t index = 0;
    for (plVulkanLightRef* light = fLightRefList; light; light = light->GetNext()) {
        if (index < kMaxSceneLights) {
            light->UpdateShaderInfo(dst);
            light->fBufferIndex = index;
            dst++;
            index++;
        } else {
            light->fBufferIndex = UINT32_MAX;
        }
    }
}

void plVulkanPipeline::ILoadLight(plLightInfo* light)
{
    plVulkanLightRef* ref = static_cast<plVulkanLightRef*>(light->GetDeviceRef());
    if (!ref || ref->fBufferIndex == UINT32_MAX || fLights.size() >= kMaxActiveLights)
        return;

    // Remember where it landed so IScaleLight can find it again.
    ref->fPassIndex = fLights.size();
    fLights.push_back(plShaderActiveLight{ ref->fBufferIndex, 1.f });
}

void plVulkanPipeline::IScaleLight(plLightInfo* light, float scale)
{
    plVulkanLightRef* ref = static_cast<plVulkanLightRef*>(light->GetDeviceRef());
    if (!ref || ref->fBufferIndex == UINT32_MAX || ref->fPassIndex >= fLights.size())
        return;

    // Quantized to a tenth, as Metal does, so tiny changes do not churn.
    scale = int(scale * 1.e1f) * 1.e-1f;
    fLights[ref->fPassIndex].scale = scale;
}

/**
 * Picks the lights that light this span.
 *
 * Ported from plMetalPipeline::ISelectLights (plMetalPipeline.cpp:2418-2487).
 * Beyond taking the strongest few, the ones nearest the cut-off are faded toward
 * zero so a light leaving the set dims out instead of popping.
 */
void plVulkanPipeline::ISelectLights(const plSpan* span, bool proj)
{
    static std::vector<plLightInfo*> onLights;
    onLights.clear();

    const bool skip = IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights) ||
                      (IsDebugFlagSet(plPipeDbg::kFlagNoApplyProjLights) && proj) ||
                      (IsDebugFlagSet(plPipeDbg::kFlagOnlyApplyProjLights) && !proj);

    size_t startScale = 0;

    if (!skip) {
        std::vector<plLightInfo*>& spanLights = span->GetLightList(proj);

        if (!proj)
            fLights.clear();

        size_t i = 0;
        for (; i < spanLights.size() && i < kMaxActiveLights; i++) {
            if (!proj)
                ILoadLight(spanLights[i]);
            onLights.push_back(spanLights[i]);
        }
        startScale = i;

        // Fade the tail of the set toward nothing, so membership changes are not
        // visible as a pop. A projected pass enables no lights, so it is exempt.
        if (!proj && i > 0 && i + 1 < spanLights.size()) {
            const float threshhold = span->GetLightStrength(i, proj);
            i--;

            float overHold = threshhold * 1.5f;
            overHold = std::min(overHold, span->GetLightStrength(0, proj));

            for (; i > 0 && span->GetLightStrength(i, proj) < overHold; i--) {
                const float scale = (overHold - span->GetLightStrength(i, proj)) /
                                    (overHold - threshhold);
                IScaleLight(spanLights[i], (1 - scale) * span->GetLightScale(i, proj));
            }
            startScale = i + 1;
        }

        if (!proj) {
            // Everything above the fade band takes its own scale unmodified.
            for (size_t j = 0; j < startScale; j++)
                IScaleLight(spanLights[j], span->GetLightScale(j, proj));
        }
    } else if (!proj) {
        fLights.clear();
    }

    // Projected lights are not enabled here, only remembered for the projection
    // passes, split by whether they cover everything or just this span.
    if (proj) {
        fProjAll.clear();
        fProjEach.clear();
        for (plLightInfo* light : onLights) {
            if (light->OverAll())
                fProjAll.push_back(light);
            else
                fProjEach.push_back(light);
        }
        onLights.clear();
    }
}
