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
/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

// singular
#include "plAGAnimInstance.h"

// local
#include "plAGAnim.h"
#include "plAGModifier.h"
#include "plAGMasterMod.h"

// global
#include "hsTimer.h"        // just when debugging for GetSysSeconds

// other
#include "pnFactory/plFactory.h"
#include "plInterp/plAnimTimeConvert.h"
#include "pnNetCommon/plSDLTypes.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plOneShotCallbacks.h"
#include "plModifier/plSDLModifier.h"
#include "plSDL/plSDL.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FLAGS
//
/////////////////////////////////////////////////////////////////////////////////////////

// enable this to show blend trees before and after attaches and detaches
// #define SHOW_AG_CHANGES

/////////////////////////////////////////////////////////////////////////////////////////
//
// STATIC
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifdef TRACK_AG_ALLOCS
ST::string gGlobalAnimName;
ST::string gGlobalChannelName;
#endif // TRACK_AG_ALLOCS

/////////////////////////////////////////////////////////////////////////////////////////
//
// plAGAnimInstance
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor -------------------------------------------------------------------
// -----
plAGAnimInstance::plAGAnimInstance(plAGAnim * anim, plAGMasterMod * master,
                                   float blend, uint16_t blendPriority, bool cache,
                                   bool useAmplitude)
    : fAnimation(anim), fMaster(master), fAmplitude(useAmplitude ? 1.0f : -1.0f),
      FadeType(), fFadeDetach(), fFadeAmpGoal(), fFadeAmpRate(),
      fBlend(blend), fFadeBlendGoal(), fFadeBlendRate(),
      fTimeConvert()
{
    int i;
    plScalarChannel *timeChan = nullptr;
#ifdef TRACK_AG_ALLOCS
    gGlobalAnimName = anim->GetName();      // for debug tracking...
#endif // TRACK_AG_ALLOCS

    plATCAnim *atcAnim = plATCAnim::ConvertNoRef(anim);
    if (atcAnim)
    {
        fTimeConvert = new plAnimTimeConvert();
        IInitAnimTimeConvert(fTimeConvert, atcAnim, master);
        //fTimeConvert->Init(atcAnim, this, master);
        timeChan = new plATCChannel(fTimeConvert);
    }
    else
    {
        timeChan = new plScalarSDLChannel(anim->GetLength());
        fSDLChannels.push_back((plScalarSDLChannel *)timeChan);
    }

    int nInChannels = anim->GetChannelCount();

    fCleanupChannels.push_back(timeChan);

#ifdef SHOW_AG_CHANGES
    hsStatusMessageF("\nAbout to Attach anim <%s>", GetName().c_str());
    fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif

    for (i = 0; i < nInChannels; i++)
    {
        plAGApplicator * app = fAnimation->GetApplicator(i);
        plAGChannel * inChannel = app->GetChannel();
        ST::string channelName = app->GetChannelName();
        plAGModifier * channelMod = master->GetChannelMod(channelName);
        
        if(channelMod) {
#ifdef TRACK_AG_ALLOCS
            gGlobalChannelName = channelName;
#endif // TRACK_AG_ALLOCS

            // we're going to be accumulating a chain of channels.
            // curChannel will always point to the top one...
            plAGChannel *topNode = inChannel;

            if(cache)
            {
                topNode = topNode->MakeCacheChannel(fTimeConvert);
                IRegisterDetach(channelName, topNode);
            }

            if(useAmplitude)
            {
                // amplitude is rarely used and expensive, so only alloc if asked
                // first build a static copy of the incoming channel...
                plAGChannel *zeroState = inChannel->MakeZeroState();
                IRegisterDetach(channelName, zeroState);

                // now make a blend node to blend the anim with its static copy
                topNode = zeroState->MakeBlend(topNode, &fAmplitude, -1);
            }

            // make a time scaler to localize time for this instance
            topNode = topNode->MakeTimeScale(timeChan);
                IRegisterDetach(channelName, topNode);

            channelMod->MergeChannel(app, topNode, &fBlend, this, blendPriority);
        }
        else
            hsAssert(false, "Adding an animation with an invalid channel.");
    }
    fFadeBlend = fFadeAmp = false;

#ifdef TRACK_AG_ALLOCS
    gGlobalAnimName = ST::string();
#endif // TRACK_AG_ALLOCS
}

// dtor -----------------------------
// -----
plAGAnimInstance::~plAGAnimInstance()
{
    delete fTimeConvert;
}


void plAGAnimInstance::IInitAnimTimeConvert(plAnimTimeConvert* atc, plATCAnim* anim, plAGMasterMod* master)
{
    // Set up our eval callbacks
    plAGInstanceCallbackMsg* instMsg;

    instMsg = new plAGInstanceCallbackMsg(master->GetKey(), kStart);
    instMsg->fInstance = this;
    atc->AddCallback(instMsg);
    hsRefCnt_SafeUnRef(instMsg);

    instMsg = new plAGInstanceCallbackMsg(master->GetKey(), kStop);
    instMsg->fInstance = this;
    atc->AddCallback(instMsg);
    hsRefCnt_SafeUnRef(instMsg);

    instMsg = new plAGInstanceCallbackMsg(master->GetKey(), kSingleFrameAdjust);
    instMsg->fInstance = this;
    atc->AddCallback(instMsg);
    hsRefCnt_SafeUnRef(instMsg);

    atc->SetOwner(master);
    atc->ClearFlags();

    for (size_t i = 0; i < anim->NumStopPoints(); i++)
    {
        atc->GetStopPoints().emplace_back(anim->GetStopPoint(i));
    }

    atc->SetBegin(anim->GetStart());
    atc->SetEnd(anim->GetEnd());
    atc->SetInitialBegin(atc->GetBegin());
    atc->SetInitialEnd(atc->GetEnd());

    if (anim->GetInitial() != -1)
    {
        atc->SetCurrentAnimTime(anim->GetInitial());
    }
    else
    {
        atc->SetCurrentAnimTime(anim->GetStart());
    }

    atc->SetLoopPoints(anim->GetLoopStart(), anim->GetLoopEnd());
    atc->Loop(anim->GetLoop());
    atc->SetSpeed(1.f);

    atc->SetEase(true, anim->GetEaseInType(), anim->GetEaseInMin(),
                       anim->GetEaseInMax(), anim->GetEaseInLength());

    atc->SetEase(false, anim->GetEaseOutType(), anim->GetEaseOutMin(),
                        anim->GetEaseOutMax(), anim->GetEaseOutLength());


    // set up our time converter based on the animation's specs...
    // ... after we've set all of its other state values.
    if (anim->GetAutoStart())
    {
        plSynchEnabler ps(true);    // enable dirty tracking so that autostart will send out a state update
        atc->Start();
    }
    else
    {
        atc->InitStop();
    }
}

// SearchForGlobals ---------------------
// -----------------
void plAGAnimInstance::SearchForGlobals()
{
    const plAgeGlobalAnim *ageAnim = plAgeGlobalAnim::ConvertNoRef(fAnimation);
    if (ageAnim != nullptr && fSDLChannels.size() > 0)
    {
        extern const plSDLModifier *ExternFindAgeSDL();
        const plSDLModifier *sdlMod = ExternFindAgeSDL();
        if (!sdlMod)
            return;

        plSimpleStateVariable *var = sdlMod->GetStateCache()->FindVar(ageAnim->GetGlobalVarName());
        if (!var)
            return;

        sdlMod->AddNotifyForVar(fMaster->GetKey(), ageAnim->GetGlobalVarName(), 0);
        int i;
        for (i = 0; i < fSDLChannels.size(); i++)
            fSDLChannels[i]->SetVar(var);
    }
}

void plAGAnimInstance::IRegisterDetach(const ST::string &channelName, plAGChannel *channel)
{
    plDetachMap::value_type newPair(channelName, channel);
    fManualDetachChannels.insert(newPair);
}

// SetCurrentTime ---------------------------------------------------------------
// ---------------
void plAGAnimInstance::SetCurrentTime(float localT, bool jump /* = false */)
{
    if (fTimeConvert)
        fTimeConvert->SetCurrentAnimTime(localT, jump);
}

// SeekRelative ------------------------------------
// -------------
void plAGAnimInstance::SeekRelative (float delta, bool jump)
{
    if(fTimeConvert)
    {
        float now = fTimeConvert->CurrentAnimTime();
        fTimeConvert->SetCurrentAnimTime (now + delta, jump);
    }
}

// Detach ---------------------
// -------
void plAGAnimInstance::Detach()
{
    fMaster->DetachAnimation(this);
}

// DetachChannels ---------------------
// ---------------
void plAGAnimInstance::DetachChannels()
{
#ifdef SHOW_AG_CHANGES
    hsStatusMessageF("\nAbout to DETACH anim <%s>", GetName().c_str());
    fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif
    plDetachMap::iterator i = fManualDetachChannels.begin();

    while(i != fManualDetachChannels.end())
    {
        ST::string channelName = (*i).first;
        plAGModifier *channelMod = fMaster->GetChannelMod(channelName, true);

        if(channelMod)
        {
            do {
                plAGChannel *channel = (*i).second;
                channelMod->DetachChannel(channel);
            } while (++i != fManualDetachChannels.end() && i->first == channelName);
        } else {
            do {
            } while (++i != fManualDetachChannels.end() && i->first == channelName);
        }
    }

    int cleanCount = fCleanupChannels.size();
    hsAssert(cleanCount, "No time controls when deleting animation");
    for (int j = 0; j < cleanCount; j++)
    {
        delete fCleanupChannels[j];
    }
    fCleanupChannels.clear();

#ifdef SHOW_AG_CHANGES
    hsStatusMessageF("\nFinished DETACHING anim <%s>", GetName().c_str());
    fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif
}

void plAGAnimInstance::SetSpeed(float speed)
{
    if (fTimeConvert)
        fTimeConvert->SetSpeed(speed);
}

// SetBlend ---------------------------------------
// ---------
float plAGAnimInstance::SetBlend(float blend)
{
    float oldBlend = fBlend.Value(0.0, true);
    if(oldBlend != blend &&
        (oldBlend == 0.0f ||
         blend == 0.0f ||
         oldBlend == 1.0f ||
         blend == 1.0f))
    {
        fMaster->SetNeedCompile(true);
    }
    fBlend.Set(blend);
    return blend;
}

// GetBlend -------------------------
// ---------
float plAGAnimInstance::GetBlend()
{
    return fBlend.Value(0);
}

// SetAmplitude -------------------------------------
// -------------
float plAGAnimInstance::SetAmplitude(float amp)
{
    if(fAmplitude.Get() != -1.0f)
    {
        fAmplitude.Set(amp);
    }
    return amp;
}

// GetAmplitude -------------------------
// -------------
float plAGAnimInstance::GetAmplitude()
{
    return fAmplitude.Value(0);
}

// GetName -----------------------------
// --------
ST::string plAGAnimInstance::GetName()
{
    if(fAnimation)
        return fAnimation->GetName();
    else
        return ST::string();
}

// SetLoop ----------------------------------
// --------
void plAGAnimInstance::SetLoop(bool status)
{
    if (fTimeConvert)
        fTimeConvert->Loop(status);
}

// HandleCmd ----------------------------------------
// ----------
bool plAGAnimInstance::HandleCmd(plAnimCmdMsg *msg)
{
    if (fTimeConvert)
        return fTimeConvert->HandleCmd(msg);
    return false;
}

// IsFinished -----------------------
// -----------
bool plAGAnimInstance::IsFinished()
{
    if (fTimeConvert)
        return fTimeConvert->IsStopped();
    return false;
}

// IsAtEnd -----------------------
// --------
bool plAGAnimInstance::IsAtEnd()
{
    if(fTimeConvert)
    {
        return fTimeConvert->CurrentAnimTime() == fTimeConvert->GetEnd();
    }
    else
        return false;
}

// Start -----------------------------------
// ------
void plAGAnimInstance::Start(double timeNow)
{
    if (fTimeConvert)
    {
        if (timeNow < 0)
            fTimeConvert->Start();
        else
            fTimeConvert->Start(timeNow);
    }
}

// Stop ---------------------
// -----
void plAGAnimInstance::Stop()
{
    if (fTimeConvert)
        fTimeConvert->Stop();
}

double plAGAnimInstance::WorldToAnimTime(double foo)
{
    return (fTimeConvert ? fTimeConvert->WorldToAnimTimeNoUpdate(foo) : 0.0);
}

// AttachCallbacks --------------------------------------------------
// ----------------
void plAGAnimInstance::AttachCallbacks(plOneShotCallbacks *callbacks)
{
    const plATCAnim *anim = plATCAnim::ConvertNoRef(fAnimation);
    if (callbacks && anim)
    {
        plAnimCmdMsg animMsg;
        animMsg.SetCmd(plAnimCmdMsg::kAddCallbacks);

        for (int i = 0; i < callbacks->GetNumCallbacks(); i++)
        {
            plOneShotCallbacks::plOneShotCallback& cb = callbacks->GetCallback(i);

            plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
            eventMsg->AddReceiver(cb.fReceiver);
            eventMsg->fRepeats = 0;
            eventMsg->fUser = cb.fUser;

            if (!cb.fMarker.empty())
            {
                float marker = anim->GetMarker(cb.fMarker);
                hsAssert(marker != -1, "Bad marker name");
                eventMsg->fEventTime = marker;
                eventMsg->fEvent = kTime;
            }
            else
            {
                eventMsg->fEvent = kStop;
            }
            
            animMsg.AddCallback(eventMsg);
            hsRefCnt_SafeUnRef(eventMsg);
        }
        
        fTimeConvert->HandleCmd(&animMsg);
    }
}

// ProcessFade -------------------------------------
// ------------
void plAGAnimInstance::ProcessFade(float elapsed)
{
    if (fFadeBlend) {
        float newBlend = ICalcFade(fFadeBlend, GetBlend(), fFadeBlendGoal, fFadeBlendRate, elapsed);
        SetBlend(newBlend);
        if(fFadeDetach && (newBlend == fFadeBlendGoal) && (fFadeBlendGoal == 0.0f) )
        {
            fMaster->DetachAnimation(this);
            return;
        }
    }

    if (fFadeAmp && fAmplitude.Get() != -1.0f) {
        float curAmp = GetAmplitude();
        float newAmp = ICalcFade(fFadeAmp, curAmp, fFadeAmpGoal, fFadeAmpRate, elapsed);
        SetAmplitude(newAmp);
    }
}

// ICalcFade ---------------------------------------------------------------------
// ----------
float plAGAnimInstance::ICalcFade(bool &fade, float curVal, float goal,
                                     float rate, float elapsed)
{
    float newVal;
    float curStep = rate * elapsed;
    if(rate > 0) {
        newVal = std::min(goal, curVal + curStep);
    } else {
        newVal = std::max(goal, curVal + curStep);
    }

    if(newVal == goal)
    {
        fade = false;
        fMaster->DirtySynchState(kSDLAGMaster, 0);  // send SDL state update to server
    }
    return newVal;
}

// FadeAndDetach -------------------------------------------------
// --------------
void plAGAnimInstance::FadeAndDetach(float goal, float rate)
{
    ISetupFade(goal, rate, true, kFadeBlend);
}

// Fade --------------------------------------------------------------------------------
// -----
void plAGAnimInstance::Fade(float goal, float rate, uint8_t type /* = kFadeBlend */)
{
    ISetupFade(goal, rate, false, type);
}

// ISetupFade --------------------------------------------------------------------------
// -----------
void plAGAnimInstance::ISetupFade(float goal, float rate, bool detach, uint8_t type)
{
    if (rate == 0)
    {
        if (type == kFadeBlend)
        {
            SetBlend(goal);
            fFadeBlend = false;
            if(detach) {
                fMaster->DetachAnimation(this);
            }
        }
        else if (type == kFadeAmp)
        {
            SetAmplitude(goal);
            fFadeAmp = false;
        }
        return;
    }

    rate = (rate > 0 ? rate : -rate); // For old code that sends negative values
    
    float curVal = 0;
    switch (type)
    {
    case kFadeBlend:
        curVal = GetBlend();
        break;
    case kFadeAmp:
        curVal = GetAmplitude();
        break;
    }
    if (curVal > goal)
        rate = -rate;
    
    switch (type)
    {
    case kFadeBlend:
        fFadeBlend = true;      
        fFadeBlendGoal = goal;
        fFadeBlendRate = rate;
        fFadeDetach = detach;
        break;
    case kFadeAmp:
        fFadeAmp = true;
        fFadeAmpGoal = goal;
        fFadeAmpRate = rate;
        fFadeDetach = false;    // only detach on blend fades, for the moment.
        break;
    }
}

class agAlloc
{
public:
    agAlloc(plAGChannel *object, const char *chanName, const char *animName, uint16_t classIndex)
        : fObject(object),
          fClassIndex(classIndex)
    {
        fChannelName = hsStrcpy(chanName);
        fAnimName = hsStrcpy(animName);
    }

    ~agAlloc()
    {
        delete[] fChannelName;
        delete[] fAnimName;
    }

    plAGChannel *fObject;
    char *fChannelName;
    char *fAnimName;
    uint16_t fClassIndex;
};

typedef std::map<plAGChannel *, agAlloc *> agAllocMap;
static agAllocMap gAGAllocs;

void RegisterAGAlloc(plAGChannel *object, const char *chanName, const char *animName, uint16_t classIndex)
{
    gAGAllocs[object] = new agAlloc(object, chanName, animName, classIndex);
}

void DumpAGAllocs()
{
    agAllocMap::iterator i = gAGAllocs.begin();
    agAllocMap::iterator theEnd = gAGAllocs.end();

    hsStatusMessage("DUMPING AG ALLOCATIONS ================================================");

    for ( ; i != theEnd; i++)
    {
        agAlloc * al = (*i).second;

        uint16_t realClassIndex = al->fObject->ClassIndex();

        hsStatusMessageF("agAlloc: an: %s ch: %s, cl: %s", al->fAnimName, al->fChannelName, plFactory::GetNameOfClass(realClassIndex));

    }
    // it's not fast but it's safe and simple..
    i = gAGAllocs.begin();
    while(i != gAGAllocs.end())
    {
        agAlloc * al = (*i).second;
        delete al;

        gAGAllocs.erase(i++);
    }
    hsStatusMessage("FINISHED DUMPING AG ALLOCATIONS *********************************************");
}

void UnRegisterAGAlloc(plAGChannel *object)
{
    agAllocMap::iterator i = gAGAllocs.find(object);
    if(i != gAGAllocs.end())
    {
        agAlloc * al = (*i).second;

        gAGAllocs.erase(i);
        delete al;
    }
}
