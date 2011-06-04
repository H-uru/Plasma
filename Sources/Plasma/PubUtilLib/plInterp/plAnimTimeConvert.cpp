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
#include "hsTypes.h"
#include "plAnimEaseTypes.h"
#include "plAnimTimeConvert.h"
#include "../plAvatar/plAGAnim.h"

#include "hsTimer.h"
#include "hsStream.h"

#include "../pnMessage/plEventCallbackMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGMasterSDLModifier.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plModifier/plLayerSDLModifier.h"
#include "../plSurface/plLayerAnimation.h"

#include "hsResMgr.h"
#include "plgDispatch.h"


plAnimTimeConvert::plAnimTimeConvert()
:	fCurrentAnimTime(0),
	fLastEvalWorldTime(0),
	fBegin(0),
	fEnd(0),
	fLoopEnd(0),
	fLoopBegin(0),
	fSpeed(1.f),
	fFlags(0),
	fOwner(nil),
	fEaseInCurve(nil),
	fEaseOutCurve(nil),
	fSpeedEaseCurve(nil),
	fCurrentEaseCurve(nil),
	fInitialBegin(0),
	fInitialEnd(0),
	fWrapTime(0)
	//fDirtyNotifier(nil)
{
}

plAnimTimeConvert::~plAnimTimeConvert()
{
	int i;
	for( i = 0; i < fCallbackMsgs.GetCount(); i++ )
		hsRefCnt_SafeUnRef(fCallbackMsgs[i]);

	delete fEaseInCurve;
	delete fEaseOutCurve;
	delete fSpeedEaseCurve;
	//delete fDirtyNotifier;

	IClearAllStates();
}

void plAnimTimeConvert::Init(plATCAnim *anim, plAGAnimInstance *instance, plAGMasterMod *master)
{
		// Set up our eval callbacks
	plAGInstanceCallbackMsg *instMsg;
	instMsg = TRACKED_NEW plAGInstanceCallbackMsg(master->GetKey(), kStart); 
	instMsg->fInstance = instance;
	AddCallback(instMsg);
	hsRefCnt_SafeUnRef(instMsg);	
	instMsg = TRACKED_NEW plAGInstanceCallbackMsg(master->GetKey(), kStop); 
	instMsg->fInstance = instance;
	AddCallback(instMsg);
	hsRefCnt_SafeUnRef(instMsg);
	instMsg = TRACKED_NEW plAGInstanceCallbackMsg(master->GetKey(), kSingleFrameAdjust); 
	instMsg->fInstance = instance;
	AddCallback(instMsg);
	hsRefCnt_SafeUnRef(instMsg);
	
	SetOwner(master);
	ClearFlags();

	for (int i = 0; i < anim->NumStopPoints(); i++)
		GetStopPoints().Append(anim->GetStopPoint(i));
	
	SetBegin(anim->GetStart());
	SetEnd(anim->GetEnd());
	fInitialBegin = fBegin;
	fInitialEnd = fEnd;
	
	if (anim->GetInitial() != -1)
		SetCurrentAnimTime(anim->GetInitial());
	else 
		SetCurrentAnimTime(anim->GetStart());
	SetLoopPoints(anim->GetLoopStart(), anim->GetLoopEnd());
	Loop(anim->GetLoop());
	SetSpeed(1.f);
	SetEase(true, anim->GetEaseInType(), anim->GetEaseInMin(), 
						 anim->GetEaseInMax(), anim->GetEaseInLength()); 
	SetEase(false, anim->GetEaseOutType(), anim->GetEaseOutMin(), 
						 anim->GetEaseOutMax(), anim->GetEaseOutLength());

	// set up our time converter based on the animation's specs...
	// ... after we've set all of its other state values.
	if (anim->GetAutoStart())
	{
		plSynchEnabler ps(true);	// enable dirty tracking so that autostart will send out a state update
		Start();
	}
	else
		InitStop();
}

//
// 0=nil, 1=easeIn, 2=easeOut, 3=speed
//
void plAnimTimeConvert::SetCurrentEaseCurve(int x)
{
	switch(x)
	{
	default:
		hsAssert(false, "invalid arg to SetCurrentEaseCurve");
		break;
	case kEaseNone:
		fCurrentEaseCurve=nil;
		break;
	case kEaseIn:
		fCurrentEaseCurve=fEaseInCurve;
		break;
	case kEaseOut:
		fCurrentEaseCurve=fEaseOutCurve;
		break;
	case kEaseSpeed:
		fCurrentEaseCurve=fSpeedEaseCurve;
		break;
	}
}

int plAnimTimeConvert::GetCurrentEaseCurve() const
{
	if (fCurrentEaseCurve==nil)
		return kEaseNone;
	if (fCurrentEaseCurve==fEaseInCurve)
		return kEaseIn;
	if (fCurrentEaseCurve==fEaseOutCurve)
		return kEaseOut;
	if (fCurrentEaseCurve==fSpeedEaseCurve)
		return kEaseSpeed;

	hsAssert(false, "unknown ease curve");
	return 0;
}

//
// set the number of ATCStates we have
//
void plAnimTimeConvert::ResizeStates(int cnt)
{
	while (fStates.size() > cnt)
	{
		delete fStates.back();
		fStates.pop_back();
	}

	while (cnt>fStates.size())
	{
		fStates.push_back(TRACKED_NEW plATCState);
	}

	hsAssert(fStates.size()==cnt, "state resize mismatch");
}

void plAnimTimeConvert::ResetWrap()
{ 
	fBegin = fInitialBegin;
	fEnd = fInitialEnd;
	Forewards();

	fFlags &= (~kWrap & ~kNeedsReset);
}

hsScalar plAnimTimeConvert::ICalcEaseTime(const plATCEaseCurve *curve, double start, double end)
{
	start -= curve->fBeginWorldTime;
	end -= curve->fBeginWorldTime;

	// Clamp to the curve's range
	if (start < 0)
		start = 0;
	if (end > curve->fLength)
		end = curve->fLength;


	hsScalar delSecs = 0;

	if (start < curve->fLength)
	{	
		// Redundant eval... but only when easing.
		delSecs = curve->PositionGivenTime((hsScalar)end) - curve->PositionGivenTime((hsScalar)start);
	}
	return delSecs;
}

void plAnimTimeConvert::IClearSpeedEase()
{
	
	if (fCurrentEaseCurve == fSpeedEaseCurve)
		fCurrentEaseCurve = nil;

	delete fSpeedEaseCurve;
	fSpeedEaseCurve = nil;
}

void plAnimTimeConvert::ICheckTimeCallbacks(hsScalar frameStart, hsScalar frameStop)
{
	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kTime)
		{
			if( ITimeInFrame(fCallbackMsgs[i]->fEventTime, frameStart, frameStop) )
				ISendCallback(i);
		}
		else if (fCallbackMsgs[i]->fEvent == kBegin && ITimeInFrame(fBegin, frameStart, frameStop) )
			ISendCallback(i);
		else if (fCallbackMsgs[i]->fEvent == kEnd && ITimeInFrame(fEnd, frameStart, frameStop) )
			ISendCallback(i);

	}
}

hsBool plAnimTimeConvert::ITimeInFrame(hsScalar secs, hsScalar start, hsScalar stop)
{
	if (secs == start && secs == stop)
		return true;
	if( IsBackwards() )
	{
		if( start < stop )
		{
			// We've just wrapped. Careful to exclude markers outside current loop.
			if( ((secs <= start) && (secs >= fLoopBegin))
				|| ((secs >= stop) && (secs <= fLoopEnd)) )
				return true;
		}
		else
		{
			if( (secs <= start) && (secs >= stop) )
				return true;
		}
	}
	else
	{
		if( start > stop )
		{
			// We've just wrapped. Careful to exclude markers outside current loop.
			if( ((secs >= start) && (secs <= fLoopEnd))
				|| ((secs <= stop) && (secs >= fLoopBegin)) )
				return true;
		}
		else
		{
			if( (secs >= start) && (secs <= stop) )
				return true;
		}
	}
	return false;
}

void plAnimTimeConvert::ISendCallback(int i)
{
	// Check if callbacks are disabled this frame (i.e. when we're loading in state)
	if (fFlags & kNoCallbacks)
		return;

	// send callback if msg is local or if we are the local master
	if (!fCallbackMsgs[i]->HasBCastFlag(plMessage::kNetPropagate) ||
		!fOwner || fOwner->IsLocallyOwned()==plSynchedObject::kYes)
	{
		plEventCallbackMsg *temp = fCallbackMsgs[i];

		fCallbackMsgs[i]->SetSender(fOwner ? fOwner->GetKey() : nil);

		hsRefCnt_SafeRef(fCallbackMsgs[i]);
		plgDispatch::MsgSend(fCallbackMsgs[i]);

		// No more repeats, remove this callback from our list
		if (fCallbackMsgs[i]->fRepeats == 0)
		{
			hsRefCnt_SafeUnRef(fCallbackMsgs[i]);
			fCallbackMsgs.Remove(i);
		}
		// If this isn't infinite, decrement the number of repeats
		else if (fCallbackMsgs[i]->fRepeats > 0)
			fCallbackMsgs[i]->fRepeats--;
	}
}

plAnimTimeConvert& plAnimTimeConvert::IStop(double time, hsScalar animTime)
{
	if( IsStopped() )
		return *this;

	IClearSpeedEase(); // If we had one queued up, clear it. It will automatically take effect when we start
	SetFlag(kEasingIn, false);
	SetFlag(kStopped, true);
	if (fFlags & kNeedsReset)
		ResetWrap();

	IProcessStateChange(time, animTime);

	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kStop)
		{
			ISendCallback(i);
		}
	}
	return *this;
}

plAnimTimeConvert& plAnimTimeConvert::IProcessStateChange(double worldTime, hsScalar animTime /* = -1 */)
{
	if (fStates.size() > 0 && worldTime < fStates.front()->fStartWorldTime)
		return *this; // Sorry... state saves only work in the forward direction

	fLastStateChange = worldTime;
	plATCState *state = TRACKED_NEW plATCState;

	state->fStartWorldTime = fLastStateChange;
	state->fStartAnimTime = (animTime < 0 ? WorldToAnimTimeNoUpdate(fLastStateChange) : animTime);
	state->fFlags = (UInt8)fFlags;
	state->fBegin = fBegin;
	state->fEnd = fEnd;
	state->fLoopBegin = fLoopBegin;
	state->fLoopEnd = fLoopEnd;
	state->fSpeed = fSpeed;
	state->fWrapTime = fWrapTime;
	state->fEaseCurve = (fCurrentEaseCurve == nil ? nil : fCurrentEaseCurve->Clone());	

	fStates.push_front(state);
	IFlushOldStates();

	const char* sdlName=nil;
	if (plLayerAnimation::ConvertNoRef(fOwner))
		sdlName=kSDLLayer;
	else
	if (plAGMasterMod::ConvertNoRef(fOwner))
		sdlName=kSDLAGMaster;
	else
	{
		hsAssert(false, "unknown sdl owner");
	}
	fOwner->DirtySynchState(sdlName, 0);		// Send SDL state update to server

	return *this;
}

// Remove any out-of-date plATCStates
// Where "out-of-date" means, "More than 1 frame old"
void plAnimTimeConvert::IFlushOldStates()
{
	plATCState *state;
	plATCStateList::const_iterator i = fStates.begin();
	UInt32 count = 0;

	for (i; i != fStates.end(); i++)
	{
		count++;
		state = *i;
		if (fLastEvalWorldTime - hsTimer::GetDelSysSeconds() >= state->fStartWorldTime)
			break;
	}

	while (fStates.size() > count)
	{
		delete fStates.back();
		fStates.pop_back();
	}
}

void plAnimTimeConvert::IClearAllStates()
{
	while (fStates.size() > 0)
	{
		delete fStates.back();
		fStates.pop_back();
	}
}

plATCState *plAnimTimeConvert::IGetState(double wSecs) const
{
	plATCState *state;
	plATCStateList::const_iterator i = fStates.begin();

	for (i; i != fStates.end(); i++)
	{
		state = *i;
		if (wSecs >= state->fStartWorldTime)
			return state;
	}

	return nil;
}

plATCState *plAnimTimeConvert::IGetLatestState() const
{
	return fStates.front();
}

void plAnimTimeConvert::SetOwner(plSynchedObject* o) 
{ 
	fOwner = o; 
}

hsBool plAnimTimeConvert::IIsStoppedAt(const double &wSecs, const UInt32 &flags, 
									   const plATCEaseCurve *curve) const		
{
	if (flags & kStopped)
		return !(flags & kForcedMove); // If someone called SetCurrentAnimTime(), we need to say we moved.

	return false;
}

hsBool plAnimTimeConvert::IsStoppedAt(double wSecs) const
{
	if (wSecs > fLastStateChange)
		return IIsStoppedAt(wSecs, fFlags, fCurrentEaseCurve);

	plATCState *state = IGetState(wSecs);

	if ( !state )
		return true;

	return IIsStoppedAt(wSecs, state->fFlags, state->fEaseCurve);
}

hsScalar plAnimTimeConvert::WorldToAnimTime(double wSecs)
{
	//hsAssert(wSecs >= fLastEvalWorldTime, "Tried to eval a time that's earlier than the last eval time.");
	double d = wSecs - fLastEvalWorldTime;
	hsScalar f = fCurrentAnimTime;

	if (wSecs < fLastStateChange)
	{
		fCurrentAnimTime = IWorldToAnimTimeBeforeState(wSecs);
		fLastEvalWorldTime = wSecs;
		return fCurrentAnimTime;
	}

	if (fLastEvalWorldTime <= fLastStateChange) // Crossing into the latest state
	{
		fLastEvalWorldTime = fLastStateChange;
		fCurrentAnimTime = IGetLatestState()->fStartAnimTime;
	}

	if( (fFlags & kStopped) || (wSecs == fLastEvalWorldTime) )
	{
		if (fFlags & kForcedMove)
		{
			int i;
			for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
			{
				if (fCallbackMsgs[i]->fEvent == kSingleFrameEval)
				{
					ISendCallback(i);
				}
			}	
		}
		fFlags &= ~kForcedMove;
		fLastEvalWorldTime = wSecs;
		
		return fCurrentAnimTime;
	}
	hsScalar note = fCurrentAnimTime - f;
	hsScalar secs = 0, delSecs = 0;

	if (fCurrentEaseCurve != nil)
	{
		delSecs += ICalcEaseTime(fCurrentEaseCurve, fLastEvalWorldTime, wSecs);
		if (wSecs > fCurrentEaseCurve->GetEndWorldTime())
		{
			if (fFlags & kEasingIn)
				delSecs += hsScalar(wSecs - fCurrentEaseCurve->GetEndWorldTime()) * fSpeed;

			IClearSpeedEase();
			
			fCurrentEaseCurve = nil;
		}
	}
	else 
	{
		// The easy case... playing the animation at a constant speed.
		delSecs = hsScalar(wSecs - fLastEvalWorldTime) * fSpeed;
	}
	
	if (fFlags & kBackwards)
		delSecs = -delSecs;
	
	secs = fCurrentAnimTime + delSecs;
	// At this point, "secs" is the pre-wrapped (before looping) anim time. 
	// "delSecs" is the change in anim time
	
	// if our speed is < 0, then checking for the kBackwards flag isn't enough
	// so we base our decision on the direction of the actual change we've computed.
	hsBool forewards = delSecs >= 0;

	if (fFlags & kLoop)
	{
		hsBool wrapped = false;

		if (forewards)
		{
			if (IGetLatestState()->fStartAnimTime > fLoopEnd)
			{
				// Our animation started past the loop. Play to the end.
				if (secs > fEnd)
				{
					secs = fEnd;
					IStop(wSecs, secs);
				}
			}
			else
			{
				if (secs > fLoopEnd)
				{
					secs = fmodf(secs - fLoopBegin, fLoopEnd - fLoopBegin) + fLoopBegin;
					wrapped = true;
				}
			}	
		}
		else
		{
			if (IGetLatestState()->fStartAnimTime < fLoopBegin)
			{
				if (secs < fBegin)
				{
					secs = fBegin;
					IStop(wSecs, secs);
				}
			}
			else
			{
				if (secs < fLoopBegin)
				{
					secs = fLoopEnd - fmodf(fLoopEnd - secs, fLoopEnd - fLoopBegin);
					wrapped = true;
				}
			}				
		}		

		if (fFlags & kWrap)
		{
			// possible options, representing each line:
			// 1. We wrapped around the the beginning of the anim, so stop at the wrap point if we're past it.
			// 2. Same as #1, but in the backwards case.
			// 3. We started before the wrap point, now we're after it. Stop.
			// 4. Same as #3, backwards.
			if ((wrapped && (forewards && secs >= fWrapTime) ||
							(!forewards && secs <= fWrapTime)) ||
				(forewards && fCurrentAnimTime < fWrapTime && secs >= fWrapTime) ||
				(!forewards && fCurrentAnimTime > fWrapTime && secs <= fWrapTime))
			{
				secs = fWrapTime;
				IStop(wSecs, secs);
			}				
		}		
	}
	else // Not looping
	{
		if ((secs < fBegin) || (secs > fEnd))
		{
			secs = forewards ? fEnd : fBegin;
			IStop(wSecs, secs);
		}
	}
	
	ICheckTimeCallbacks(fCurrentAnimTime, secs);
	
	fLastEvalWorldTime = wSecs;
	if (fEaseOutCurve != nil && !(fFlags & kEasingIn) && wSecs >= fEaseOutCurve->GetEndWorldTime())
		IStop(wSecs, secs);
	
	return fCurrentAnimTime = secs;
}

hsScalar plAnimTimeConvert::WorldToAnimTimeNoUpdate(double wSecs) const
{
	return IWorldToAnimTimeNoUpdate(wSecs, IGetState(wSecs));
}

hsScalar plAnimTimeConvert::IWorldToAnimTimeNoUpdate(double wSecs, plATCState *state)
{
	//hsAssert(wSecs >= fLastEvalWorldTime, "Tried to eval a time that's earlier than the last eval time.");
	if (state == nil)
		return 0;
	
	if (state->fFlags & kStopped)
		return state->fStartAnimTime;
	
	hsScalar secs = 0, delSecs = 0;
	
	if (state->fEaseCurve != nil)
	{
		delSecs += ICalcEaseTime(state->fEaseCurve, state->fStartWorldTime, wSecs);
		if (wSecs > state->fEaseCurve->GetEndWorldTime())
		{
			if (state->fFlags & kEasingIn)
				delSecs += hsScalar(wSecs - state->fEaseCurve->GetEndWorldTime()) * state->fSpeed;
		}
	}
	else 
	{
		// The easy case... playing the animation at a constant speed.
		delSecs = hsScalar(wSecs - state->fStartWorldTime) * state->fSpeed;
	}
	
	if (state->fFlags & kBackwards)
		delSecs = -delSecs;
	
	secs = state->fStartAnimTime + delSecs;
	// At this point, "secs" is the pre-wrapped (before looping) anim time. 
	// "delSecs" is the change in anim time
	hsBool forewards = delSecs >= 0;

	if (state->fFlags & kLoop)
	{
		hsBool wrapped = false;

		if (forewards)
		{
			if (state->fStartAnimTime > state->fLoopEnd)
			{
				// Our animation started past the loop. Play to the end.
				if (secs > state->fEnd)
				{
					secs = state->fEnd;
				}
			}
			else
			{
				if (secs > state->fLoopEnd)
				{
					secs = fmodf(secs - state->fLoopBegin, state->fLoopEnd - state->fLoopBegin) + state->fLoopBegin;
					wrapped = true;
				}
			}	
		}
		else
		{
			if (state->fStartAnimTime < state->fLoopBegin)
			{
				if (secs < state->fBegin)
				{
					secs = state->fBegin;
				}
			}
			else
			{
				if (secs < state->fLoopBegin)
				{
					secs = state->fLoopEnd - fmodf(state->fLoopEnd - secs, state->fLoopEnd - state->fLoopBegin);
					wrapped = true;
				}
			}				
		}

		if (state->fFlags & kWrap)
		{
			if ((wrapped && (forewards && secs >= state->fWrapTime) ||
				(!forewards && secs <= state->fWrapTime)) ||
				(forewards && state->fStartAnimTime < state->fWrapTime && secs >= state->fWrapTime) ||
				(!forewards && state->fStartAnimTime > state->fWrapTime && secs <= state->fWrapTime))
			{
				secs = state->fWrapTime;
			}					
		}		
	}
	else
	{
		if ((secs < state->fBegin) || (secs > state->fEnd))
		{
			secs = forewards ? state->fEnd : state->fBegin;
		}
	}
	
	return secs;
}

hsScalar plAnimTimeConvert::IWorldToAnimTimeBeforeState(double wSecs) const
{
	return IWorldToAnimTimeNoUpdate(wSecs, IGetState(wSecs));
}

void plAnimTimeConvert::SetCurrentAnimTime(hsScalar s, hsBool jump /* = false */)
{
	// We're setting the anim value for whenever we last evaluated.
	fFlags |= kForcedMove;
	if (!jump)
		ICheckTimeCallbacks(fCurrentAnimTime, s);
	fCurrentAnimTime = s;
	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kSingleFrameAdjust)
		{
			ISendCallback(i);
		}
	}	
	IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
}

void plAnimTimeConvert::SetEase(hsBool easeIn, UInt8 type, hsScalar minLength, hsScalar maxLength, hsScalar normLength) 
{ 
	if (easeIn)
	{
		delete fEaseInCurve;
		fEaseInCurve = plATCEaseCurve::CreateEaseCurve(type, minLength, maxLength, normLength, 0, fSpeed);
	}
	else
	{
		delete fEaseOutCurve;
		fEaseOutCurve = plATCEaseCurve::CreateEaseCurve(type, minLength, maxLength, normLength, fSpeed, 0);
	}
}



hsScalar plAnimTimeConvert::GetBestStopDist(hsScalar min, hsScalar max, hsScalar norm, hsScalar time) const
{
	hsScalar bestTime = -1;
	hsScalar bestDist = -1;
	if (fStopPoints.GetCount() == 0)
		return norm;

	hsScalar curTime;
	hsScalar curDist;

	int i;
	for (i = 0; i < fStopPoints.GetCount(); i++)
	{
		hsScalar stop = fStopPoints.Get(i);

		if (IsLooped())
		{
			hsScalar loopDist;
			if (IsBackwards())
			{
				if ((time >= fLoopBegin && stop < fLoopBegin) ||
					(time < fLoopBegin && stop > fLoopBegin))
					continue;
				loopDist = -(fLoopEnd - fLoopBegin);
			}
			else
			{
				if ((time <= fLoopEnd && stop > fLoopEnd) ||
					(time > fLoopEnd && stop < fLoopEnd))
					continue; // we'll never reach it.
				loopDist = fLoopEnd - fLoopBegin;
			}
			if (stop <= fLoopEnd && stop >= fLoopBegin)
			{
				while (true)
				{
					curTime = stop - time;
					if (IsBackwards())
						curTime = -curTime;

					if (curTime > max)
						break;

					curDist = curTime - norm;
					if (curDist < 0)
						curDist = -curDist;

					if (curTime >= min && curTime <= max && (bestDist == -1 || bestDist > curDist))
					{
						bestDist = curDist;
						bestTime = curTime;
					}
					stop += loopDist;
				}

				continue;
			}
		}
		
		curTime = stop - time;
		if (IsBackwards())
			curTime = -curTime;

		curDist = curTime - norm;
		if (curDist < 0)
			curDist = -curDist;

		if (curTime >= min && curTime <= max && (bestDist == -1 || bestDist > curDist))
		{
			bestDist = curDist;
			bestTime = curTime;
		}
	}
	
	hsStatusMessageF("found stop point %f\n", bestTime);

	if (bestTime == -1)
		bestTime = norm;
	return bestTime;
}

// Passing in a rate of zero specifies an immediate change.
void plAnimTimeConvert::SetSpeed(hsScalar goal, hsScalar rate /* = 0 */)
{
	hsScalar curSpeed = fSpeed;
	fSpeed = goal;
	

	if (rate == 0)
	{
		IClearSpeedEase();
		fCurrentEaseCurve = nil;

	}
	// Skip if we're either stopped or stopping. We'll take the new speed into account next time we start up.
	else if ((fFlags & kEasingIn)) 
	{
		double curTime = hsTimer::GetSysSeconds();
		if (fCurrentEaseCurve != nil)
		{
			double easeTime = curTime - fCurrentEaseCurve->fBeginWorldTime;
			curSpeed = fCurrentEaseCurve->VelocityGivenTime((hsScalar)easeTime);
		}
		if (fSpeedEaseCurve != nil)
		{
			fSpeedEaseCurve->RecalcToSpeed(curSpeed, goal);
			fSpeedEaseCurve->SetLengthOnRate(rate);
		}
		else
		{
			hsScalar length;
			length = (goal - curSpeed) / rate;
			if (length < 0)
				length = -length;

			fSpeedEaseCurve = plATCEaseCurve::CreateEaseCurve(plAnimEaseTypes::kConstAccel, length, length, length, 
															  curSpeed, goal);
		}

		fSpeedEaseCurve->fBeginWorldTime = curTime;
		fCurrentEaseCurve = fSpeedEaseCurve;
	}

	IProcessStateChange(hsTimer::GetSysSeconds());
}

void plAnimTimeConvert::Read(hsStream* s, hsResMgr* mgr)
{
	plCreatable::Read(s, mgr);

	fFlags = (UInt16)(s->ReadSwap32());

	fBegin = fInitialBegin = s->ReadSwapScalar();
	fEnd = fInitialEnd = s->ReadSwapScalar();
	fLoopEnd = s->ReadSwapScalar();
	fLoopBegin = s->ReadSwapScalar();
	fSpeed = s->ReadSwapScalar();

	fEaseInCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
	fEaseOutCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
	fSpeedEaseCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));

	fCurrentAnimTime = s->ReadSwapScalar();
	fLastEvalWorldTime = s->ReadSwapDouble();

	// load other non-synched data;
	int count = s->ReadSwap32();
	fCallbackMsgs.SetCountAndZero(count);

	int i;
	for (i = 0; i < count; i++)
	{
		plEventCallbackMsg* msg = plEventCallbackMsg::ConvertNoRef(mgr->ReadCreatable(s));
		fCallbackMsgs[i] = msg;
	}

	count = s->ReadSwap32();
	for (i = 0; i < count; i++)
	{
		fStopPoints.Append(s->ReadSwapScalar());
	}
	IProcessStateChange(0, fBegin);
}

void plAnimTimeConvert::Write(hsStream* s, hsResMgr* mgr)
{
	plCreatable::Write(s, mgr);

	s->WriteSwap32(fFlags);

	s->WriteSwapScalar(fBegin);
	s->WriteSwapScalar(fEnd);
	s->WriteSwapScalar(fLoopEnd);
	s->WriteSwapScalar(fLoopBegin);
	s->WriteSwapScalar(fSpeed);

	mgr->WriteCreatable(s, fEaseInCurve);
	mgr->WriteCreatable(s, fEaseOutCurve);
	mgr->WriteCreatable(s, fSpeedEaseCurve);

	s->WriteSwapScalar(fCurrentAnimTime);
	s->WriteSwapDouble(fLastEvalWorldTime);

	// save out other non-synched important data
	s->WriteSwap32(fCallbackMsgs.Count());
	int i;
	for (i = 0; i < fCallbackMsgs.Count(); i++)
		mgr->WriteCreatable(s, fCallbackMsgs[i]);

	s->WriteSwap32(fStopPoints.GetCount());
	for (i = 0; i < fStopPoints.GetCount(); i++)
	{
		s->WriteSwapScalar(fStopPoints.Get(i));
	}
}

plAnimTimeConvert& plAnimTimeConvert::InitStop() 
{ 
	return IStop(hsTimer::GetSysSeconds(), fCurrentAnimTime);
}

plAnimTimeConvert& plAnimTimeConvert::Stop(hsBool on) 
{ 
	if( on )
		return Stop();
	else
		return Start();
}

plAnimTimeConvert& plAnimTimeConvert::Stop(double stopTime) 
{ 	
	if( IsStopped() || (fEaseOutCurve != nil && !(fFlags & kEasingIn)) )
		return *this;

	if (stopTime < 0)
		stopTime = hsTimer::GetSysSeconds();
	hsScalar stopAnimTime = WorldToAnimTimeNoUpdate(stopTime);
	
	SetFlag(kEasingIn, false);

	if( fEaseOutCurve == nil )
	{
		return IStop(stopTime, fCurrentAnimTime);
	}

	hsScalar currSpeed;
	if (fCurrentEaseCurve == nil || stopTime >= fCurrentEaseCurve->GetEndWorldTime())
		currSpeed = fSpeed;
	else
		currSpeed = fCurrentEaseCurve->VelocityGivenTime((hsScalar)(stopTime - fCurrentEaseCurve->fBeginWorldTime));

	fEaseOutCurve->RecalcToSpeed(currSpeed > fSpeed ? currSpeed : fSpeed, 0);
	fEaseOutCurve->SetLengthOnDistance(GetBestStopDist(fEaseOutCurve->GetMinDistance(), fEaseOutCurve->GetMaxDistance(),
													   fEaseOutCurve->GetNormDistance(), stopAnimTime));
	fEaseOutCurve->fBeginWorldTime = stopTime - fEaseOutCurve->TimeGivenVelocity(currSpeed);

	fCurrentEaseCurve = fEaseOutCurve;
	
	return IProcessStateChange(stopTime);
}

plAnimTimeConvert& plAnimTimeConvert::Start(double startTime) 
{ 
	// If start has not been called since the last stop, kEasingIn will not be set 
	if( (fFlags & kEasingIn) && (startTime == fLastStateChange) )
		return *this;

	SetFlag(kEasingIn, true);	

	if (startTime < 0)
		startTime = hsTimer::GetSysSeconds();
	
	if (fEaseInCurve != nil)
	{
		hsScalar currSpeed;
		if (fCurrentEaseCurve == nil || startTime >= fCurrentEaseCurve->GetEndWorldTime())
			currSpeed = 0;
		else
			currSpeed = fCurrentEaseCurve->VelocityGivenTime((hsScalar)(startTime - fCurrentEaseCurve->fBeginWorldTime));

		if (currSpeed <= fSpeed)
		{
			fEaseInCurve->RecalcToSpeed(0, fSpeed);
			fEaseInCurve->fBeginWorldTime = startTime - fEaseInCurve->TimeGivenVelocity(currSpeed);

			fCurrentEaseCurve = fEaseInCurve;
			
		}
		else
		{	// We eased out in the middle of a speed change, but were told to start again before slowing past
			// the target speed, so the "ease in" is really a slow down.
			SetSpeed(fSpeed, fEaseInCurve->fSpeed / fEaseInCurve->fLength);
		}
	}
	
	// check for a start callback
	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kStart)
		{
			ISendCallback(i);
		}
	}

	SetFlag(kStopped, false);
	if (fFlags & kBackwards)
	{
		if (fCurrentAnimTime == fBegin)
			return IProcessStateChange(startTime, fEnd);
	}
	else
	{
		if (fCurrentAnimTime == fEnd)
			return IProcessStateChange(startTime, fBegin);
	}
	return IProcessStateChange(startTime);
}

plAnimTimeConvert& plAnimTimeConvert::Backwards(hsBool on) 
{
	return on ? Backwards() : Forewards();
}

plAnimTimeConvert& plAnimTimeConvert::Backwards() 
{ 
	if( IsBackwards() )
		return *this;

	// check for a reverse callback
	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kReverse)
		{
			ISendCallback(i);
		}
	}

	SetFlag(kBackwards, true);

	// Record state changes
	IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);

	return *this;
}

plAnimTimeConvert& plAnimTimeConvert::Forewards() 
{ 
	if( !IsBackwards() )
		return *this;
	
	// check for a reverse callback
	int i;
	for( i = fCallbackMsgs.GetCount()-1; i >= 0; --i )
	{
		if (fCallbackMsgs[i]->fEvent == kReverse)
		{
			ISendCallback(i);
		}
	}
	
	SetFlag(kBackwards, false); 
	
	// Record state changes
	IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
	
	return *this;
}

plAnimTimeConvert& plAnimTimeConvert::Loop(hsBool on) 
{ 
	SetFlag(kLoop, on); 
	
	// Record state changes
	IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
	
	return *this;
}

plAnimTimeConvert& plAnimTimeConvert::PlayToTime(hsScalar time)
{
	fFlags |= kNeedsReset;
	if (fCurrentAnimTime > time)
	{
		if (fFlags & kLoop)
		{
			fWrapTime = time;
			fFlags |= kWrap;
		}
		else
		{
			fBegin = time;
			Backwards();
		}
	}
	else
	{
		fEnd = time;
	}
	Start();

	return *this;
}

plAnimTimeConvert& plAnimTimeConvert::PlayToPercentage(hsScalar percent)
{
	return PlayToTime(fBegin + (fEnd - fBegin) * percent);
}

void plAnimTimeConvert::RemoveCallback(plEventCallbackMsg* pMsg)
{
	int idx = fCallbackMsgs.Find(pMsg);
	if( idx != fCallbackMsgs.kMissingIndex )
	{
		hsRefCnt_SafeUnRef(fCallbackMsgs[idx]);
		fCallbackMsgs.Remove(idx);
	}
}
	
hsBool plAnimTimeConvert::HandleCmd(plAnimCmdMsg* modMsg)
{
	if (fFlags & kNeedsReset)
		ResetWrap();

	// The net msg screener is already checking for callbacks,
	// I'm just being extra safe.
	if (!modMsg->HasBCastFlag(plMessage::kNetCreatedRemotely))
	{
		if( modMsg->Cmd(plAnimCmdMsg::kAddCallbacks) )
		{
			int i;
			for( i = 0; i < modMsg->GetNumCallbacks(); i++ )
			{
				AddCallback(plEventCallbackMsg::ConvertNoRef(modMsg->GetEventCallback(i)));
			}
		}
		if( modMsg->Cmd(plAnimCmdMsg::kRemoveCallbacks) )
		{
			int i;
			for( i = 0; i < modMsg->GetNumCallbacks(); i++ )
			{
				RemoveCallback(modMsg->GetEventCallback(i));
			}
		}
	}

	if( modMsg->Cmd(plAnimCmdMsg::kSetBackwards) )
	{
		Backwards();
	}
	if( modMsg->Cmd(plAnimCmdMsg::kSetForewards) )
	{	
		Forewards();
	}
	
	if( modMsg->Cmd(plAnimCmdMsg::kStop) )
		Stop();

	if( modMsg->Cmd(plAnimCmdMsg::kSetLooping) )
		Loop();

	if( modMsg->Cmd(plAnimCmdMsg::kUnSetLooping) )
		NoLoop();

	if (modMsg->Cmd(plAnimCmdMsg::kSetBegin))
	{
		if (modMsg->fBegin >= fInitialBegin)
			SetBegin(modMsg->fBegin);
		else
			SetBegin(fInitialBegin);
	}

	if (modMsg->Cmd(plAnimCmdMsg::kSetEnd))
	{
		if (modMsg->fEnd <= fInitialEnd)
			SetEnd(modMsg->fEnd);
		else
			SetEnd(fInitialEnd);
	}

	if (fBegin > fEnd)
	{
		fBegin = fInitialBegin;
		fEnd = fInitialEnd;
	}

	if( modMsg->Cmd(plAnimCmdMsg::kSetLoopEnd) )
		SetLoopEnd(modMsg->fLoopEnd);

	if( modMsg->Cmd(plAnimCmdMsg::kSetLoopBegin) )
		SetLoopBegin(modMsg->fLoopBegin);

	if( modMsg->Cmd(plAnimCmdMsg::kSetSpeed) )
		SetSpeed(modMsg->fSpeed, modMsg->fSpeedChangeRate);

	if( modMsg->Cmd(plAnimCmdMsg::kGoToTime) )
	{
		if (modMsg->fTime < fBegin)
			SetCurrentAnimTime(fBegin, true);
		else if (modMsg->fTime > fEnd)
			SetCurrentAnimTime(fEnd, true);
		else
			SetCurrentAnimTime(modMsg->fTime, true);	
	}

	if ( modMsg->Cmd(plAnimCmdMsg::kGoToPercent) )
		SetCurrentAnimTime(fBegin + (fEnd - fBegin) * modMsg->fTime);

	if( modMsg->Cmd(plAnimCmdMsg::kGoToBegin) )
		SetCurrentAnimTime(fBegin, true);

	if( modMsg->Cmd(plAnimCmdMsg::kGoToEnd) )
		SetCurrentAnimTime(fEnd, true);

	if( modMsg->Cmd(plAnimCmdMsg::kGoToLoopBegin) )
		SetCurrentAnimTime(fLoopBegin, true);

	if( modMsg->Cmd(plAnimCmdMsg::kGoToLoopEnd) )
		SetCurrentAnimTime(fLoopEnd, true);

	if( modMsg->Cmd(plAnimCmdMsg::kToggleState) )
	{
		if( IsStopped() )
		{
			Start();
		}
		else
		{
			Stop();
		}
	}
	if( modMsg->Cmd(plAnimCmdMsg::kContinue) )
	{
		Start();
	}
	if( modMsg->Cmd(plAnimCmdMsg::kIncrementForward) )
	{
		if (fCurrentAnimTime == fEnd)
			return true;
		double currTime = hsTimer::GetSysSeconds();
		hsScalar newTime = fCurrentAnimTime + hsTimer::GetDelSysSeconds();
		if (newTime > fEnd)
		{
			newTime = fEnd;
		}
		Forewards();
		SetCurrentAnimTime(newTime);
	}
	if( modMsg->Cmd(plAnimCmdMsg::kIncrementBackward) )
	{
		if (fCurrentAnimTime == fBegin)
			return true;
		double currTime = hsTimer::GetSysSeconds();
		hsScalar newTime = fCurrentAnimTime - hsTimer::GetDelSysSeconds();
		if (newTime < fBegin)
		{
			newTime = fBegin;
		}
		Backwards();
		SetCurrentAnimTime(newTime);
	}

	if (modMsg->Cmd(plAnimCmdMsg::kPlayToTime))
		PlayToTime(modMsg->fTime);

	if (modMsg->Cmd(plAnimCmdMsg::kPlayToPercentage))
		PlayToPercentage(modMsg->fTime);

	// Basically, simulate what would happen if we played the animation
	if (modMsg->Cmd(plAnimCmdMsg::kFastForward))
	{
		if (IsForewards())
			SetCurrentAnimTime(fEnd, true);
		else
			SetCurrentAnimTime(fBegin, true);
		// but if it should continue to play, statr it.
		if (IsLooped())
			Start();
	}
	
	return true;
}

void plAnimTimeConvert::AddCallback(plEventCallbackMsg* pMsg)
{
	hsRefCnt_SafeRef(pMsg);
	fCallbackMsgs.Append(pMsg);
}

void plAnimTimeConvert::ClearCallbacks()
{
	for (int i = 0; i<fCallbackMsgs.Count(); i++)
	{
		hsRefCnt_SafeUnRef(fCallbackMsgs[i]);
	}
	fCallbackMsgs.Reset();
}

void plAnimTimeConvert::EnableCallbacks(hsBool val)
{
	SetFlag(kNoCallbacks, !val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void plATCState::Read(hsStream *s, hsResMgr *mgr)
{
	fStartWorldTime = s->ReadSwapDouble();
	fStartAnimTime = s->ReadSwapScalar();

	fFlags = (UInt8)(s->ReadSwap32());
	fEnd = s->ReadSwapScalar();
	fLoopBegin = s->ReadSwapScalar();
	fLoopEnd = s->ReadSwapScalar();
	fSpeed = s->ReadSwapScalar();
	fWrapTime = s->ReadSwapScalar();
	if (s->ReadBool())
		fEaseCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
}

void plATCState::Write(hsStream *s, hsResMgr *mgr)
{
	s->WriteSwapDouble(fStartWorldTime);
	s->WriteSwapScalar(fStartAnimTime);

	s->WriteSwap32(fFlags);
	s->WriteSwapScalar(fEnd);
	s->WriteSwapScalar(fLoopBegin);
	s->WriteSwapScalar(fLoopEnd);
	s->WriteSwapScalar(fSpeed);
	s->WriteSwapScalar(fWrapTime);
	if (fEaseCurve != nil)
	{
		s->WriteBool(true);
		mgr->WriteCreatable(s, fEaseCurve);
	}
	else
		s->WriteBool(false);
}





















