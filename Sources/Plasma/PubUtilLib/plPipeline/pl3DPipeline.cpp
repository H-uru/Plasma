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

#include "pl3DPipeline.h"
#include "plProfile.h"

#include "hsGMatState.inl"

#include "plSurface/hsGMaterial.h"
#include "plDrawable/plDrawableSpans.h"
#include "plGLight/plLightInfo.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "plScene/plVisMgr.h"

plProfile_CreateTimer("FindSceneLights", "PipeT", FindSceneLights);

plProfile_CreateCounter("LightOn", "PipeC", LightOn);
plProfile_CreateCounter("LightVis", "PipeC", LightVis);
plProfile_CreateCounter("LightChar", "PipeC", LightChar);
plProfile_CreateCounter("LightActive", "PipeC", LightActive);

pl3DPipeline::pl3DPipeline()
    : fActiveLights(nullptr)
{
}

pl3DPipeline::~pl3DPipeline()
{
    //hsAssert(fCurrMaterial == nullptr, "Current material not unrefed properly");

    // CullProxy is a debugging representation of our CullTree. See plCullTree.cpp, 
    // plScene/plOccluder.cpp and plScene/plOccluderProxy.cpp for more info
    if (fView.HasCullProxy())
        fView.GetCullProxy()->GetKey()->UnRefObject();

    // Tell the light infos to unlink themselves
    while (fActiveLights)
        UnRegisterLight(fActiveLights);
}

void pl3DPipeline::BeginVisMgr(plVisMgr* visMgr)
{
    // Make Light Lists /////////////////////////////////////////////////////
    // Look through all the current lights, and fill out two lists.
    // Only active lights (not disabled, not exactly black, and not
    // ignored because of visibility regions by plVisMgr) will
    // be considered.
    // The first list is lights that will affect the avatar and similar
    // indeterminately mobile (physical) objects - fCharLights.
    // The second list is lights that aren't restricted by light include
    // lists. 
    // These two abbreviated lists will be further refined for each object
    // and avatar to find the strongest 8 lights which affect that object.
    // A light with an include list, or LightGroup Component) has
    // been explicitly told which objects it affects, so they don't
    // need to be in the search lists.
    // These lists are only constructed once per render, but searched
    // multiple times

    plProfile_BeginTiming(FindSceneLights);
    fCharLights.SetCount(0);
    fVisLights.SetCount(0);
    if (visMgr)
    {
        const hsBitVector& visSet = visMgr->GetVisSet();
        const hsBitVector& visNot = visMgr->GetVisNot();
        plLightInfo* light;

        for (light = fActiveLights; light != nullptr; light = light->GetNext())
        {
            plProfile_IncCount(LightActive, 1);
            if (!light->IsIdle() && !light->InVisNot(visNot) && light->InVisSet(visSet))
            {
                plProfile_IncCount(LightOn, 1);
                if (light->GetProperty(plLightInfo::kLPHasIncludes))
                {
                    if (light->GetProperty(plLightInfo::kLPIncludesChars))
                        fCharLights.Append(light);
                }
                else
                {
                    fVisLights.Append(light);
                    fCharLights.Append(light);
                }
            }
        }
    }
    else
    {
        plLightInfo* light;
        for (light = fActiveLights; light != nullptr; light = light->GetNext())
        {
            plProfile_IncCount(LightActive, 1);
            if (!light->IsIdle())
            {
                plProfile_IncCount(LightOn, 1);
                if (light->GetProperty(plLightInfo::kLPHasIncludes))
                {
                    if (light->GetProperty(plLightInfo::kLPIncludesChars))
                        fCharLights.Append(light);
                }
                else
                {
                    fVisLights.Append(light);
                    fCharLights.Append(light);
                }
            }
        }
    }
    plProfile_IncCount(LightVis, fVisLights.GetCount());
    plProfile_IncCount(LightChar, fCharLights.GetCount());

    plProfile_EndTiming(FindSceneLights);
}


void pl3DPipeline::RegisterLight(plLightInfo* liInfo)
{
    if (liInfo->IsLinked())
        return;

    liInfo->Link(&fActiveLights);

    // Override this method to set the light's native device ref!
}


void pl3DPipeline::UnRegisterLight(plLightInfo* liInfo)
{
    liInfo->SetDeviceRef(nullptr);
    liInfo->Unlink();
}


void pl3DPipeline::EndVisMgr(plVisMgr* visMgr)
{
    fCharLights.SetCount(0);
    fVisLights.SetCount(0);
}


hsGMaterial* pl3DPipeline::PushOverrideMaterial(hsGMaterial* mat)
{
    hsGMaterial* ret = GetOverrideMaterial();
    hsRefCnt_SafeRef(mat);
    fOverrideMat.Push(mat);
    fForceMatHandle = true;

    return ret;
}


void pl3DPipeline::PopOverrideMaterial(hsGMaterial* restore)
{
    hsGMaterial *pop = fOverrideMat.Pop();
    hsRefCnt_SafeUnRef(pop);

    if (fCurrMaterial == pop)
    {
        fForceMatHandle = true;
    }
}


hsGMatState pl3DPipeline::PushMaterialOverride(const hsGMatState& state, bool on)
{
    hsGMatState ret = GetMaterialOverride(on);
    if (on)
    {
        fMatOverOn |= state;
        fMatOverOff -= state;
    }
    else
    {
        fMatOverOff |= state;
        fMatOverOn -= state;
    }
    fForceMatHandle = true;
    return ret;
}


hsGMatState pl3DPipeline::PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on)
{
    hsGMatState ret = GetMaterialOverride(on);
    if (on)
    {
        fMatOverOn[cat] |= which;
        fMatOverOff[cat] &= ~which;
    }
    else
    {
        fMatOverOn[cat] &= ~which;
        fMatOverOff[cat] |= which;
    }
    fForceMatHandle = true;
    return ret;
}


void pl3DPipeline::PopMaterialOverride(const hsGMatState& restore, bool on)
{
    if (on)
    {
        fMatOverOn = restore;
        fMatOverOff.Clear(restore);
    }
    else
    {
        fMatOverOff = restore;
        fMatOverOn.Clear(restore);
    }
    fForceMatHandle = true;
}