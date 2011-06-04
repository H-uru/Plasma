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

// singular
#include "plAGAnimInstance.h"

// local
#include "plAGAnim.h"
#include "plAGModifier.h"
#include "plAGMasterMod.h"

// global
#include "hsTimer.h"		// just when debugging for GetSysSeconds

// other
#include "../pnNetCommon/plSDLTypes.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plOneShotCallbacks.h"
#include "../plModifier/plSDLModifier.h"
#include "../plSDL/plSDL.h"

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
extern const char *gGlobalAnimName = nil;
extern const char *gGlobalChannelName = nil;
#endif // TRACK_AG_ALLOCS

/////////////////////////////////////////////////////////////////////////////////////////
//
// plAGAnimInstance
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor -------------------------------------------------------------------
// -----
plAGAnimInstance::plAGAnimInstance(plAGAnim * anim, plAGMasterMod * master,
								   hsScalar blend, UInt16 blendPriority, hsBool cache,
								   bool useAmplitude)
: fAnimation(anim),
  fMaster(master),
  fBlend(blend),
  fAmplitude(useAmplitude ? 1.0f : -1.0f)
{
	int i;
	fTimeConvert = nil;
	plScalarChannel *timeChan = nil;
#ifdef TRACK_AG_ALLOCS
	gGlobalAnimName = anim->GetName();		// for debug tracking...
#endif // TRACK_AG_ALLOCS

	plATCAnim *atcAnim = plATCAnim::ConvertNoRef(anim);
	if (atcAnim)
	{
		fTimeConvert = TRACKED_NEW plAnimTimeConvert();
		fTimeConvert->Init(atcAnim, this, master);
		timeChan = TRACKED_NEW plATCChannel(fTimeConvert);
	}
	else
	{
		timeChan = TRACKED_NEW plScalarSDLChannel(anim->GetLength());
		fSDLChannels.push_back((plScalarSDLChannel *)timeChan);
	}

	int nInChannels = anim->GetChannelCount();

	fCleanupChannels.push_back(timeChan);

#ifdef SHOW_AG_CHANGES
	hsStatusMessageF("\nAbout to Attach anim <%s>", GetName());
	fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif

	for (i = 0; i < nInChannels; i++)
	{
		plAGApplicator * app = fAnimation->GetApplicator(i);
		plAGChannel * inChannel = app->GetChannel();
		const char * channelName = app->GetChannelName();
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
	gGlobalAnimName = nil;
#endif // TRACK_AG_ALLOCS
}

// dtor -----------------------------
// -----
plAGAnimInstance::~plAGAnimInstance()
{
	delete fTimeConvert;
}

// SearchForGlobals ---------------------
// -----------------
void plAGAnimInstance::SearchForGlobals()
{
	const plAgeGlobalAnim *ageAnim = plAgeGlobalAnim::ConvertNoRef(fAnimation);
	if (ageAnim != nil && fSDLChannels.size() > 0)
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

void plAGAnimInstance::IRegisterDetach(const char *channelName, plAGChannel *channel)
{
	plDetachMap::value_type newPair(channelName, channel);
	fManualDetachChannels.insert(newPair);
}

// SetCurrentTime ---------------------------------------------------------------
// ---------------
void plAGAnimInstance::SetCurrentTime(hsScalar localT, hsBool jump /* = false */)
{
	if (fTimeConvert)
		fTimeConvert->SetCurrentAnimTime(localT, jump);
}

// SeekRelative ------------------------------------
// -------------
void plAGAnimInstance::SeekRelative (hsScalar delta, hsBool jump)
{
	if(fTimeConvert)
	{
		hsScalar now = fTimeConvert->CurrentAnimTime();
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
	hsStatusMessageF("\nAbout to DETACH anim <%s>", GetName());
	fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif
	plDetachMap::iterator i = fManualDetachChannels.begin();

	while(i != fManualDetachChannels.end())
	{
		const char *channelName = (*i).first;
		plAGModifier *channelMod = fMaster->GetChannelMod(channelName, true);

		if(channelMod)
		{
			do {
				plAGChannel *channel = (*i).second;
				channelMod->DetachChannel(channel);
			} while (i != fManualDetachChannels.end() && (*++i).first == channelName);
		} else {
			do {
			} while (i != fManualDetachChannels.end() && (*++i).first == channelName);
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
	hsStatusMessageF("\nFinished DETACHING anim <%s>", GetName());
	fMaster->DumpAniGraph("bone_pelvis", false, hsTimer::GetSysSeconds());
#endif
}

// SetBlend ---------------------------------------
// ---------
hsScalar plAGAnimInstance::SetBlend(hsScalar blend)
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
hsScalar plAGAnimInstance::GetBlend()
{
	return fBlend.Value(0);
}

// SetAmplitude -------------------------------------
// -------------
hsScalar plAGAnimInstance::SetAmplitude(hsScalar amp)
{
	if(fAmplitude.Get() != -1.0f)
	{
		fAmplitude.Set(amp);
	}
	return amp;
}

// GetAmplitude -------------------------
// -------------
hsScalar plAGAnimInstance::GetAmplitude()
{
	return fAmplitude.Value(0);
}

// GetName -----------------------------
// --------
const char * plAGAnimInstance::GetName()
{
	if(fAnimation)
		return fAnimation->GetName();
	else
		return nil;
}

// SetLoop ----------------------------------
// --------
void plAGAnimInstance::SetLoop(hsBool status)
{
	if (fTimeConvert)
		fTimeConvert->Loop(status);
}

// HandleCmd ----------------------------------------
// ----------
hsBool plAGAnimInstance::HandleCmd(plAnimCmdMsg *msg)
{
	if (fTimeConvert)
		return fTimeConvert->HandleCmd(msg);
	return false;
}

// IsFinished -----------------------
// -----------
hsBool plAGAnimInstance::IsFinished()
{
	if (fTimeConvert)
		return fTimeConvert->IsStopped();
	return false;
}

// IsAtEnd -----------------------
// --------
hsBool plAGAnimInstance::IsAtEnd()
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

			plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
			eventMsg->AddReceiver(cb.fReceiver);
			eventMsg->fRepeats = 0;
			eventMsg->fUser = cb.fUser;

			if (cb.fMarker)
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
void plAGAnimInstance::ProcessFade(hsScalar elapsed)
{
	if (fFadeBlend) {
		hsScalar newBlend = ICalcFade(fFadeBlend, GetBlend(), fFadeBlendGoal, fFadeBlendRate, elapsed);
		SetBlend(newBlend);
		if(fFadeDetach && (newBlend == fFadeBlendGoal) && (fFadeBlendGoal == 0.0f) )
		{
			fMaster->DetachAnimation(this);
			return;
		}
	}

	if (fFadeAmp && fAmplitude.Get() != -1.0f) {
		hsScalar curAmp = GetAmplitude();
		hsScalar newAmp = ICalcFade(fFadeAmp, curAmp, fFadeAmpGoal, fFadeAmpRate, elapsed);
		SetAmplitude(newAmp);
	}
}

// ICalcFade ---------------------------------------------------------------------
// ----------
hsScalar plAGAnimInstance::ICalcFade(hsBool &fade, hsScalar curVal, hsScalar goal,
									 hsScalar rate, hsScalar elapsed)
{
	hsScalar newVal;
	hsScalar curStep = rate * elapsed;
	if(rate > 0) {
		newVal = __min(goal, curVal + curStep);
	} else {
		newVal = __max(goal, curVal + curStep);
	}

	if(newVal == goal)
	{
		fade = false;
		fMaster->DirtySynchState(kSDLAGMaster, 0);	// send SDL state update to server
	}
	return newVal;
}

// FadeAndDetach -------------------------------------------------
// --------------
void plAGAnimInstance::FadeAndDetach(hsScalar goal, hsScalar rate)
{
	ISetupFade(goal, rate, true, kFadeBlend);
}

// Fade --------------------------------------------------------------------------------
// -----
void plAGAnimInstance::Fade(hsScalar goal, hsScalar rate, UInt8 type /* = kFadeBlend */)
{
	ISetupFade(goal, rate, false, type);
}

// ISetupFade --------------------------------------------------------------------------
// -----------
void plAGAnimInstance::ISetupFade(hsScalar goal, hsScalar rate, bool detach, UInt8 type)
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
	
	hsScalar curVal = 0;
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
		fFadeDetach = false;	// only detach on blend fades, for the moment.
		break;
	}
}

class agAlloc
{
public:
	agAlloc(plAGChannel *object, const char *chanName, const char *animName, UInt16 classIndex)
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
	UInt16 fClassIndex;
};

typedef std::map<plAGChannel *, agAlloc *> agAllocMap;
static agAllocMap gAGAllocs;

void RegisterAGAlloc(plAGChannel *object, const char *chanName, const char *animName, UInt16 classIndex)
{
	gAGAllocs[object] = TRACKED_NEW agAlloc(object, chanName, animName, classIndex);
}

void DumpAGAllocs()
{
	agAllocMap::iterator i = gAGAllocs.begin();
	agAllocMap::iterator theEnd = gAGAllocs.end();

	hsStatusMessage("DUMPING AG ALLOCATIONS ================================================");

	for ( ; i != theEnd; i++)
	{
		agAlloc * al = (*i).second;

		UInt16 realClassIndex = al->fObject->ClassIndex();

		hsStatusMessageF("agAlloc: an: %s ch: %s, cl: %s", al->fAnimName, al->fChannelName, plFactory::GetNameOfClass(realClassIndex));

	}
	// it's not fast but it's safe and simple..
	i = gAGAllocs.begin();
	while(i != gAGAllocs.end())
	{
		agAlloc * al = (*i).second;
		delete al;

		i = gAGAllocs.erase(i);
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











