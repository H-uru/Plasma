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

#include "hsGDeviceRef.h"
#include "hsGMatState.inl"
#include "plPipeDebugFlags.h"
#include "plProfile.h"
#include "plTweak.h"

#include "plRenderTarget.h"
#include "plCubicRenderTarget.h"

#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plSpaceTree.h"
#include "plDrawable/plSpanTypes.h"
#include "plGLight/plLightInfo.h"
#include "plGLight/plShadowSlave.h"
#include "plGLight/plShadowCaster.h"
#include "plIntersect/plVolumeIsect.h"
#include "plScene/plRenderRequest.h"
#include "plScene/plVisMgr.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

plProfile_CreateTimer("RenderScene",            "PipeT", RenderScene);
plProfile_CreateTimer("VisEval",                "PipeT", VisEval);
plProfile_CreateTimer("VisSelect",              "PipeT", VisSelect);

plProfile_CreateTimer("FindSceneLights",        "PipeT", FindSceneLights);
plProfile_CreateTimer("  Find Lights",          "PipeT", FindLights);
plProfile_CreateTimer("    Find Perms",         "PipeT", FindPerm);
plProfile_CreateTimer("    FindSpan",           "PipeT", FindSpan);
plProfile_CreateTimer("    FindActiveLights",   "PipeT", FindActiveLights);
plProfile_CreateTimer("    ApplyActiveLights",  "PipeT", ApplyActiveLights);
plProfile_CreateTimer("      ApplyMoving",      "PipeT", ApplyMoving);
plProfile_CreateTimer("      ApplyToSpec",      "PipeT", ApplyToSpec);
plProfile_CreateTimer("      ApplyToMoving",    "PipeT", ApplyToMoving);

plProfile_CreateCounter("LightOn",              "PipeC", LightOn);
plProfile_CreateCounter("LightVis",             "PipeC", LightVis);
plProfile_CreateCounter("LightChar",            "PipeC", LightChar);
plProfile_CreateCounter("LightActive",          "PipeC", LightActive);
plProfile_CreateCounter("Lights Found",         "PipeC", FindLightsFound);
plProfile_CreateCounter("Perms Found",          "PipeC", FindLightsPerm);


PipelineParams plPipeline::fDefaultPipeParams;
PipelineParams plPipeline::fInitialPipeParams;

static const float kPerspLayerScale  = 0.00001f;
static const float kPerspLayerScaleW = 0.001f;
static const float kPerspLayerTrans  = 0.00002f;

pl3DPipeline::pl3DPipeline(const hsG3DDeviceModeRecord* devModeRec)
:   fMaxLayersAtOnce(-1),
    fMaxPiggyBacks(0),
    //fMaxNumLights(kD3DMaxTotalLights),
    //fMaxNumProjectors(kMaxProjectors),
    fOverBaseLayer(nullptr),
    fOverAllLayer(nullptr),
    fMatPiggyBacks(0),
    fActivePiggyBacks(0),
    fVtxBuffRefList(nullptr),
    fIdxBuffRefList(nullptr),
    fCurrMaterial(nullptr),
    fCurrLay(nullptr),
    fCurrNumLayers(0),
    fCurrRenderLayer(0),
    fCurrLightingMethod(plSpan::kLiteMaterial),
    fActiveLights(nullptr),
    fCurrRenderTarget(nullptr),
    fCurrBaseRenderTarget(nullptr),
    fCurrRenderTargetRef(nullptr),
    fColorDepth(32),
    fInSceneDepth(0),
    fTime(0),
    fFrame(0)
{

    fOverLayerStack.Reset();
    fPiggyBackStack.Reset();

    fMatOverOn.Reset();
    fMatOverOff.Reset();

    for (int i = 0; i < 8; i++)
    {
        fLayerRef[i] = nullptr;
    }

    fTweaks.Reset();
    fView.Reset(this);
    fDebugFlags.Clear();


    // Get the requested mode and setup
    const hsG3DDeviceMode *devMode = devModeRec->GetMode();

    if(!fInitialPipeParams.Windowed)
    {
        fOrigWidth = devMode->GetWidth();
        fOrigHeight = devMode->GetHeight();
    }
    else
    {
        // windowed can run in any mode
        fOrigHeight = fInitialPipeParams.Height;
        fOrigWidth = fInitialPipeParams.Width;
    }

    IGetViewTransform().SetScreenSize(uint16_t(fOrigWidth), uint16_t(fOrigHeight));
    fColorDepth = devMode->GetColorDepth();

    fVSync = fInitialPipeParams.VSync;
}

pl3DPipeline::~pl3DPipeline()
{
    fCurrLay = nullptr;
    hsAssert(fCurrMaterial == nullptr, "Current material not unrefed properly");

    // CullProxy is a debugging representation of our CullTree. See plCullTree.cpp, 
    // plScene/plOccluder.cpp and plScene/plOccluderProxy.cpp for more info
    if (fView.HasCullProxy())
        fView.GetCullProxy()->GetKey()->UnRefObject();

    // Tell the light infos to unlink themselves
    while (fActiveLights)
        UnRegisterLight(fActiveLights);
}


void pl3DPipeline::Render(plDrawable* d, const std::vector<int16_t>& visList)
{
    // Reset here, since we can push/pop renderTargets after BeginRender() but
    // before this function, which necessitates this being called
    if (fView.fXformResetFlags != 0)
        ITransformsToDevice();

    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(d);

    if (ds)
    {
        RenderSpans(ds, visList);
    }
}


void pl3DPipeline::Draw(plDrawable* d)
{
    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(d);

    if (ds)
    {
        if (( ds->GetType() & fView.GetDrawableTypeMask()) == 0)
            return;

        static std::vector<int16_t> visList;

        PreRender(ds, visList);
        PrepForRender(ds, visList);
        Render(ds, visList);
    }
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


void pl3DPipeline::PushRenderTarget(plRenderTarget* target)
{
    fCurrRenderTarget = target;
    hsRefCnt_SafeAssign(fCurrRenderTargetRef, (target != nullptr) ? target->GetDeviceRef() : nullptr);

    while (target != nullptr)
    {
        fCurrBaseRenderTarget = target;
        target = target->GetParent();
    }

    fRenderTargets.Push(fCurrRenderTarget);
    fDevice.SetRenderTarget(fCurrRenderTarget);
}


plRenderTarget* pl3DPipeline::PopRenderTarget()
{
    plRenderTarget* old = fRenderTargets.Pop();
    plRenderTarget* temp;
    size_t i = fRenderTargets.GetCount();

    if (i == 0)
    {
        fCurrRenderTarget = nullptr;
        fCurrBaseRenderTarget = nullptr;
        hsRefCnt_SafeUnRef(fCurrRenderTargetRef);
        fCurrRenderTargetRef = nullptr;
    }
    else
    {
        fCurrRenderTarget = fRenderTargets[i - 1];
        temp = fCurrRenderTarget;

        while (temp != nullptr)
        {
            fCurrBaseRenderTarget = temp;
            temp = temp->GetParent();
        }

        hsRefCnt_SafeAssign(fCurrRenderTargetRef, (fCurrRenderTarget != nullptr) ? fCurrRenderTarget->GetDeviceRef() : nullptr);
    }

    fDevice.SetRenderTarget(fCurrRenderTarget);

    return old;
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


void pl3DPipeline::EndVisMgr(plVisMgr* visMgr)
{
    fCharLights.SetCount(0);
    fVisLights.SetCount(0);
}


void pl3DPipeline::SetZBiasScale(float scale)
{
    scale += 1.0f;
    fTweaks.fPerspLayerScale = fTweaks.fDefaultPerspLayerScale * scale;
    fTweaks.fPerspLayerTrans = kPerspLayerTrans * scale;
}


float pl3DPipeline::GetZBiasScale() const
{
    return (fTweaks.fPerspLayerScale / fTweaks.fDefaultPerspLayerScale) - 1.0f;
}


void pl3DPipeline::SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w)
{
    plViewTransform& view_xform = fView.GetViewTransform();

    view_xform.SetCameraTransform(w2c, c2w);

    fView.fCullTreeDirty = true;
    fView.fWorldToCamLeftHanded = fView.GetWorldToCamera().GetParity();

    IWorldToCameraToDevice();
}


void pl3DPipeline::ScreenToWorldPoint(int n, uint32_t stride, int32_t* scrX, int32_t* scrY, float dist, uint32_t strideOut, hsPoint3* worldOut)
{
    while( n-- )
    {
        hsPoint3 scrP;
        scrP.Set(float(*scrX++), float(*scrY++), float(dist));
        *worldOut++ = GetViewTransform().ScreenToWorld(scrP);
    }
}


void pl3DPipeline::RefreshScreenMatrices()
{
    fView.fCullTreeDirty = true;
    IProjectionMatrixToDevice();
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


plLayerInterface* pl3DPipeline::AppendLayerInterface(plLayerInterface* li, bool onAllLayers)
{
    fForceMatHandle = true;
    if (onAllLayers)
        return fOverAllLayer = li->Attach(fOverAllLayer);
    else
        return fOverBaseLayer = li->Attach(fOverBaseLayer);
}


plLayerInterface* pl3DPipeline::RemoveLayerInterface(plLayerInterface* li, bool onAllLayers)
{
    fForceMatHandle = true;

    if (onAllLayers)
    {
        if (!fOverAllLayer)
            return nullptr;
        return fOverAllLayer = fOverAllLayer->Remove(li);
    }

    if (!fOverBaseLayer)
        return nullptr;

    return fOverBaseLayer = fOverBaseLayer->Remove(li);
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


void pl3DPipeline::SubmitShadowSlave(plShadowSlave* slave)
{
    // Check that it's a valid slave.
    if (!(slave && slave->fCaster && slave->fCaster->GetKey()))
        return;

    // Ref the shadow caster so we're sure it will still be around when we go to
    // render it.
    slave->fCaster->GetKey()->RefObject();

    // Keep the shadow slaves in a priority sorted list. For performance reasons,
    // we may want only the strongest N or those of a minimum priority.
    int i;
    for (i = 0; i < fShadows.GetCount(); i++)
    {
        if (slave->fPriority <= fShadows[i]->fPriority)
            break;
    }

    // Note that fIndex is no longer the index in the fShadows list, but
    // is still used as a unique identifier for this slave.
    slave->fIndex = fShadows.GetCount();
    fShadows.Insert(i, slave);
}


plLayerInterface* pl3DPipeline::PushPiggyBackLayer(plLayerInterface* li)
{
    fPiggyBackStack.Push(li);

    fActivePiggyBacks = std::min(static_cast<int>(fMaxPiggyBacks), fPiggyBackStack.GetCount());

    fForceMatHandle = true;

    return li;
}


plLayerInterface* pl3DPipeline::PopPiggyBackLayer(plLayerInterface* li)
{
    int idx = fPiggyBackStack.Find(li);
    if (fPiggyBackStack.kMissingIndex == idx)
        return nullptr;

    fPiggyBackStack.Remove(idx);

    fActivePiggyBacks = std::min(static_cast<int>(fMaxPiggyBacks), fPiggyBackStack.GetCount());

    fForceMatHandle = true;

    return li;
}


void pl3DPipeline::SetViewTransform(const plViewTransform& v)
{
    fView.SetViewTransform(v);

    if (!v.GetScreenWidth() || !v.GetScreenHeight())
    {
        fView.GetViewTransform().SetScreenSize(uint16_t(fOrigWidth), uint16_t(fOrigHeight));
    }

    fView.fCullTreeDirty = true;
    fView.fWorldToCamLeftHanded = fView.GetWorldToCamera().GetParity();

    IWorldToCameraToDevice();
}




/*** PROTECTED METHODS *******************************************************/

void pl3DPipeline::IAttachSlaveToReceivers(int which, plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    plShadowSlave* slave = fShadows[which];

    // Whether the drawable is a character affects which lights/shadows affect it.
    bool isChar = drawable->GetNativeProperty(plDrawable::kPropCharacter);

    // If the shadow is part of a light group, it gets handled in ISetShadowFromGroup.
    // Unless the drawable is a character (something that moves around indeterminately,
    // like the avatar or a physical object), and the shadow affects all characters.
    if (slave->ObeysLightGroups() && !(slave->IncludesChars() && isChar))
        return;

    // Do a space tree harvest looking for spans that are visible and whose bounds
    // intercect the shadow volume.
    plSpaceTree* space = drawable->GetSpaceTree();

    static hsBitVector cache;
    cache.Clear();
    space->EnableLeaves(visList, cache);

    static std::vector<int16_t> hitList;
    hitList.clear();
    space->HarvestEnabledLeaves(slave->fIsect, cache, hitList);

    // For the visible spans that intercect the shadow volume, attach the shadow
    // to all appropriate for receiving this shadow map.
    for (int16_t idx : hitList)
    {
        const plSpan* span = drawable->GetSpan(idx);
        hsGMaterial* mat = drawable->GetMaterial(span->fMaterialIdx);

        // Check that the span isn't flagged as unshadowable, or has
        // a material that we can't shadow onto.
        if (!IReceivesShadows(span, mat))
            continue;

        // Check for self shadowing. If the shadow doesn't want self shadowing,
        // and the span is part of the shadow caster, then skip.
        if (!IAcceptsShadow(span, slave))
            continue;

        // Add it to this span's shadow list for this frame.
        span->AddShadowSlave(fShadows[which]->fIndex);
    }
}


void pl3DPipeline::IAttachShadowsToReceivers(plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    for (size_t i = 0; i < fShadows.GetCount(); i++)
        IAttachSlaveToReceivers(i, drawable, visList);
}


bool pl3DPipeline::IAcceptsShadow(const plSpan* span, plShadowSlave* slave)
{
    // The span's shadow bits records which shadow maps that span was rendered
    // into.
    return slave->SelfShadow() || !span->IsShadowBitSet(slave->fIndex);
}


bool pl3DPipeline::IReceivesShadows(const plSpan* span, hsGMaterial* mat)
{
    if (span->fProps & plSpan::kPropNoShadow)
        return false;

    if (span->fProps & plSpan::kPropForceShadow)
        return true;

    if (span->fProps & (plSpan::kPropSkipProjection | plSpan::kPropProjAsVtx))
        return false;

    if ((fMaxLayersAtOnce < 3) &&
        (mat->GetLayer(0)->GetTexture()) &&
        (mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha))
        return false;

    return true;
}


void pl3DPipeline::ISetShadowFromGroup(plDrawableSpans* drawable, const plSpan* span, plLightInfo* liInfo)
{
    hsGMaterial* mat = drawable->GetMaterial(span->fMaterialIdx);

    // Check that this span/material combo can receive shadows at all.
    if (!IReceivesShadows(span, mat))
        return;

    const hsBitVector& slaveBits = liInfo->GetSlaveBits();

    for (size_t i = 0; i < fShadows.GetCount(); i++)
    {
        if (slaveBits.IsBitSet(fShadows[i]->fIndex))
        {
            // Check self shadowing.
            if (IAcceptsShadow(span, fShadows[i]))
            {
                // Check for overlapping bounds.
                if (fShadows[i]->fIsect->Test(span->fWorldBounds) != kVolumeCulled)
                    span->AddShadowSlave(fShadows[i]->fIndex);
            }
        }
    }
}


void pl3DPipeline::ICheckLighting(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    if (fView.fRenderState & kRenderNoLights)
        return;

    if (visList.empty())
        return;

    plLightInfo* light;

    plProfile_BeginTiming(FindLights);

    // First add in the explicit lights (from LightGroups).
    // Refresh the lights as they are added (actually a lazy eval).
    plProfile_BeginTiming(FindPerm);
    for (int16_t idx : visList)
    {
        drawable->GetSpan(idx)->ClearLights();

        if (IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights))
            continue;

        // Set the bits for the lights added from the permanent lists (during ClearLights()).
        const std::vector<plLightInfo*>& permaLights = drawable->GetSpan(idx)->fPermaLights;
        for (plLightInfo* permaLight : permaLights)
        {
            permaLight->Refresh();
            if (permaLight->GetProperty(plLightInfo::kLPShadowLightGroup) && !permaLight->IsIdle())
            {
                // If it casts a shadow, attach the shadow now.
                ISetShadowFromGroup(drawable, drawable->GetSpan(idx), permaLight);
            }
        }

        const std::vector<plLightInfo*>& permaProjs = drawable->GetSpan(idx)->fPermaProjs;
        for (plLightInfo* permaProj : permaProjs)
        {
            permaProj->Refresh();
            if (permaProj->GetProperty(plLightInfo::kLPShadowLightGroup) && !permaProj->IsIdle())
            {
                // If it casts a shadow, attach the shadow now.
                ISetShadowFromGroup(drawable, drawable->GetSpan(idx), permaProj);
            }
        }
    }
    plProfile_EndTiming(FindPerm);

    if (IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights))
    {
        plProfile_EndTiming(FindLights);
        return;
    }

    // Sort the incoming spans as either
    // A) moving - affected by all lights - moveList
    // B) specular - affected by specular lights - specList
    // C) visible - affected by moving lights - visList
    static std::vector<int16_t> tmpList;
    static std::vector<int16_t> moveList;
    static std::vector<int16_t> specList;

    moveList.clear();
    specList.clear();

    plProfile_BeginTiming(FindSpan);
    for (int16_t idx : visList)
    {
        const plSpan* span = drawable->GetSpan(idx);

        if (span->fProps & plSpan::kPropRunTimeLight)
        {
            moveList.emplace_back(idx);
            specList.emplace_back(idx);
        }
        else if (span->fProps & plSpan::kPropMatHasSpecular)
        {
            specList.emplace_back(idx);
        }
    }
    plProfile_EndTiming(FindSpan);

    // Make a list of lights that can potentially affect spans in this drawable
    // based on the drawables bounds and properties.
    // If the drawable has the PropCharacter property, it is affected by lights
    // in fCharLights, else only by the smaller list of fVisLights.

    plProfile_BeginTiming(FindActiveLights);
    static hsTArray<plLightInfo*> lightList;
    lightList.SetCount(0);

    if (drawable->GetNativeProperty(plDrawable::kPropCharacter))
    {
        for (size_t i = 0; i < fCharLights.GetCount(); i++)
        {
            if (fCharLights[i]->AffectsBound(drawable->GetSpaceTree()->GetWorldBounds()))
                lightList.Append(fCharLights[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < fVisLights.GetCount(); i++)
        {
            if (fVisLights[i]->AffectsBound(drawable->GetSpaceTree()->GetWorldBounds()))
                lightList.Append(fVisLights[i]);
        }
    }
    plProfile_EndTiming(FindActiveLights);

    // Loop over the lights and for each light, extract a list of the spans that light
    // affects. Append the light to each spans list with a scalar strength of how strongly
    // the light affects it. Since the strength is based on the object's center position,
    // it's not very accurate, but good enough for selecting which lights to use.

    plProfile_BeginTiming(ApplyActiveLights);
    for (size_t k = 0; k < lightList.GetCount(); k++)
    {
        light = lightList[k];

        tmpList.clear();
        if (light->GetProperty(plLightInfo::kLPMovable))
        {
            plProfile_BeginTiming(ApplyMoving);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                visList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList)
            {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection)))
                {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0)
                    {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyMoving);
        }
        else if (light->GetProperty(plLightInfo::kLPHasSpecular))
        {
            if (specList.empty())
                continue;

            plProfile_BeginTiming(ApplyToSpec);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                specList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList)
            {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection)))
                {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0)
                    {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyToSpec);
        }
        else
        {
            if (moveList.empty())
                continue;

            plProfile_BeginTiming(ApplyToMoving);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                moveList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList)
            {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection)))
                {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0)
                    {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyToMoving);
        }
    }
    plProfile_EndTiming(ApplyActiveLights);

    IAttachShadowsToReceivers(drawable, visList);

    plProfile_EndTiming(FindLights);
}


hsMatrix44 pl3DPipeline::IGetCameraToNDC()
{
    hsMatrix44 cam2ndc = GetViewTransform().GetCameraToNDC();

    if (fView.IsPerspective())
    {
        // Want to scale down W and offset in Z without
        // changing values of x/w, y/w. This is just
        // minimal math for
        // Mproj' * p = Mscaletrans * Mproj * p
        // where Mscaletrans =
        // [ s 0 0 0 ]
        // [ 0 s 0 0 ]
        // [ 0 0 s 0 ]
        // [ 0 0 t s ]
        // Resulting matrix Mproj' is not exactly "Fog Friendly",
        // but is close enough.
        // Resulting point is [sx, sy, sz + tw, sw] and after divide
        // is [x/w, y/w, z/w + t/s, 1/sw]

        float scale = 1.f - float(fCurrRenderLayer) * fTweaks.fPerspLayerScale;
        float zTrans = -scale * float(fCurrRenderLayer) * fTweaks.fPerspLayerTrans;

        cam2ndc.fMap[0][0] *= scale;
        cam2ndc.fMap[1][1] *= scale;

        cam2ndc.fMap[2][2] *= scale;
        cam2ndc.fMap[2][2] += zTrans * cam2ndc.fMap[3][2];
        cam2ndc.fMap[3][2] *= scale;
    }
    else
    {
        plConst(float) kZTrans = -1.e-4f;
        cam2ndc.fMap[2][3] += kZTrans * fCurrRenderLayer;
    }

    return cam2ndc;
}


void pl3DPipeline::ISetLocalToWorld(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    fView.SetLocalToWorld(l2w);
    fView.SetWorldToLocal(w2l);

    fView.fViewVectorsDirty = true;

    // We keep track of parity for winding order culling.
    fView.fLocalToWorldLeftHanded = fView.GetLocalToWorld().GetParity();

    ILocalToWorldToDevice();
}



void pl3DPipeline::ITransformsToDevice()
{
    if (fView.fXformResetFlags & fView.kResetCamera)
        IWorldToCameraToDevice();

    if (fView.fXformResetFlags & fView.kResetL2W)
        ILocalToWorldToDevice();

    if (fView.fXformResetFlags & fView.kResetProjection)
        IProjectionMatrixToDevice();
}


void pl3DPipeline::IProjectionMatrixToDevice()
{
    fDevice.SetProjectionMatrix(IGetCameraToNDC());
    fView.fXformResetFlags &= ~fView.kResetProjection;
}

void pl3DPipeline::IWorldToCameraToDevice()
{
    fDevice.SetWorldToCameraMatrix(fView.GetWorldToCamera());
    fView.fXformResetFlags &= ~fView.kResetCamera;

    fFrame++;
}

void pl3DPipeline::ILocalToWorldToDevice()
{
    fDevice.SetLocalToWorldMatrix(fView.GetLocalToWorld());
    fView.fXformResetFlags &= ~fView.kResetL2W;
}
