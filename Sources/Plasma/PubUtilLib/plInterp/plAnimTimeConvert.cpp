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
#include "HeadSpin.h"
#include <cmath>
#include <string_theory/format>

#include "plAnimEaseTypes.h"
#include "plAnimTimeConvert.h"

#include "hsTimer.h"
#include "hsStream.h"

#include "pnMessage/plEventCallbackMsg.h"
#include "plMessage/plAnimCmdMsg.h"

#include "pnNetCommon/plSDLTypes.h"
#include "pnNetCommon/plSynchedObject.h"

#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plCreatableIndex.h"


plAnimTimeConvert::~plAnimTimeConvert()
{
    for (plEventCallbackMsg* msg : fCallbackMsgs)
        hsRefCnt_SafeUnRef(msg);

    delete fEaseInCurve;
    delete fEaseOutCurve;
    delete fSpeedEaseCurve;

    IClearAllStates();
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
        fCurrentEaseCurve = nullptr;
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
    if (fCurrentEaseCurve == nullptr)
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
        fStates.push_back(new plATCState);
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

float plAnimTimeConvert::ICalcEaseTime(const plATCEaseCurve *curve, double start, double end)
{
    start -= curve->fBeginWorldTime;
    end -= curve->fBeginWorldTime;

    // Clamp to the curve's range
    if (start < 0)
        start = 0;
    if (end > curve->fLength)
        end = curve->fLength;


    float delSecs = 0;

    if (start < curve->fLength)
    {   
        // Redundant eval... but only when easing.
        delSecs = curve->PositionGivenTime((float)end) - curve->PositionGivenTime((float)start);
    }
    return delSecs;
}

void plAnimTimeConvert::IClearSpeedEase()
{
    
    if (fCurrentEaseCurve == fSpeedEaseCurve)
        fCurrentEaseCurve = nullptr;

    delete fSpeedEaseCurve;
    fSpeedEaseCurve = nullptr;
}

void plAnimTimeConvert::ICheckTimeCallbacks(float frameStart, float frameStop)
{
    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kTime)
        {
            if( ITimeInFrame(fCallbackMsgs[i]->fEventTime, frameStart, frameStop) )
                ISendCallback(i);
        }
        else if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kBegin && ITimeInFrame(fBegin, frameStart, frameStop) )
            ISendCallback(i);
        else if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kEnd && ITimeInFrame(fEnd, frameStart, frameStop) )
            ISendCallback(i);

    }
}

bool plAnimTimeConvert::ITimeInFrame(float secs, float start, float stop)
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

void plAnimTimeConvert::ISendCallback(hsSsize_t i)
{
    // Check if callbacks are disabled this frame (i.e. when we're loading in state)
    if (fFlags & kNoCallbacks)
        return;

    // send callback if msg is local or if we are the local master
    if (!fCallbackMsgs[i]->HasBCastFlag(plMessage::kNetPropagate) ||
        !fOwner || fOwner->IsLocallyOwned()==plSynchedObject::kYes)
    {
        fCallbackMsgs[i]->SetSender(fOwner ? fOwner->GetKey() : nullptr);

        hsRefCnt_SafeRef(fCallbackMsgs[i]);
        plgDispatch::MsgSend(fCallbackMsgs[i]);

        // No more repeats, remove this callback from our list
        if (fCallbackMsgs[i]->fRepeats == 0)
        {
            hsRefCnt_SafeUnRef(fCallbackMsgs[i]);
            fCallbackMsgs.erase(fCallbackMsgs.begin() + i);
        }
        // If this isn't infinite, decrement the number of repeats
        else if (fCallbackMsgs[i]->fRepeats > 0)
            fCallbackMsgs[i]->fRepeats--;
    }
}

plAnimTimeConvert& plAnimTimeConvert::IStop(double time, float animTime)
{
    if( IsStopped() )
        return *this;

    IClearSpeedEase(); // If we had one queued up, clear it. It will automatically take effect when we start
    SetFlag(kEasingIn, false);
    SetFlag(kStopped, true);
    if (fFlags & kNeedsReset)
        ResetWrap();

    IProcessStateChange(time, animTime);

    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kStop)
        {
            ISendCallback(i);
        }
    }
    return *this;
}

plAnimTimeConvert& plAnimTimeConvert::IProcessStateChange(double worldTime, float animTime /* = -1 */)
{
    if (fStates.size() > 0 && worldTime < fStates.front()->fStartWorldTime)
        return *this; // Sorry... state saves only work in the forward direction

    fLastStateChange = worldTime;
    plATCState *state = new plATCState;

    state->fStartWorldTime = fLastStateChange;
    state->fStartAnimTime = (animTime < 0 ? WorldToAnimTimeNoUpdate(fLastStateChange) : animTime);
    state->fFlags = fFlags;
    state->fBegin = fBegin;
    state->fEnd = fEnd;
    state->fLoopBegin = fLoopBegin;
    state->fLoopEnd = fLoopEnd;
    state->fSpeed = fSpeed;
    state->fWrapTime = fWrapTime;
    state->fEaseCurve = (fCurrentEaseCurve == nullptr ? nullptr : fCurrentEaseCurve->Clone());

    fStates.push_front(state);
    IFlushOldStates();

    const char* sdlName = nullptr;

    // This is a huge hack, but avoids circular linking problems :(
    if (fOwner->GetInterface(CLASS_INDEX_SCOPED(plLayerAnimation)))
        sdlName=kSDLLayer;
    else
    if (fOwner->GetInterface(CLASS_INDEX_SCOPED(plAGMasterMod)))
        sdlName=kSDLAGMaster;
    else
    {
        hsAssert(false, "unknown sdl owner");
    }
    fOwner->DirtySynchState(sdlName, 0);        // Send SDL state update to server

    return *this;
}

// Remove any out-of-date plATCStates
// Where "out-of-date" means, "More than 1 frame old"
void plAnimTimeConvert::IFlushOldStates()
{
    plATCState *state;
    plATCStateList::const_iterator i = fStates.begin();
    uint32_t count = 0;

    for (; i != fStates.end(); i++)
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

    for (; i != fStates.end(); i++)
    {
        state = *i;
        if (wSecs >= state->fStartWorldTime)
            return state;
    }

    return nullptr;
}

plATCState *plAnimTimeConvert::IGetLatestState() const
{
    return fStates.front();
}

void plAnimTimeConvert::SetOwner(plSynchedObject* o) 
{ 
    fOwner = o; 
}

bool plAnimTimeConvert::IIsStoppedAt(double wSecs, uint32_t flags,
                                     const plATCEaseCurve *curve) const
{
    if (flags & kStopped)
        return !(flags & kForcedMove); // If someone called SetCurrentAnimTime(), we need to say we moved.

    return false;
}

bool plAnimTimeConvert::IsStoppedAt(double wSecs) const
{
    if (wSecs > fLastStateChange)
        return IIsStoppedAt(wSecs, fFlags, fCurrentEaseCurve);

    plATCState *state = IGetState(wSecs);

    if ( !state )
        return true;

    return IIsStoppedAt(wSecs, state->fFlags, state->fEaseCurve);
}

float plAnimTimeConvert::WorldToAnimTime(double wSecs)
{
    //hsAssert(wSecs >= fLastEvalWorldTime, "Tried to eval a time that's earlier than the last eval time.");

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
            for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
            {
                if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kSingleFrameEval)
                {
                    ISendCallback(i);
                }
            }   
        }
        fFlags &= ~kForcedMove;
        fLastEvalWorldTime = wSecs;
        
        return fCurrentAnimTime;
    }
    float secs = 0, delSecs = 0;

    if (fCurrentEaseCurve != nullptr)
    {
        delSecs += ICalcEaseTime(fCurrentEaseCurve, fLastEvalWorldTime, wSecs);
        if (wSecs > fCurrentEaseCurve->GetEndWorldTime())
        {
            if (fFlags & kEasingIn)
                delSecs += float(wSecs - fCurrentEaseCurve->GetEndWorldTime()) * fSpeed;

            IClearSpeedEase();
            
            fCurrentEaseCurve = nullptr;
        }
    }
    else 
    {
        // The easy case... playing the animation at a constant speed.
        delSecs = float(wSecs - fLastEvalWorldTime) * fSpeed;
    }
    
    if (fFlags & kBackwards)
        delSecs = -delSecs;
    
    secs = fCurrentAnimTime + delSecs;
    // At this point, "secs" is the pre-wrapped (before looping) anim time. 
    // "delSecs" is the change in anim time
    
    // if our speed is < 0, then checking for the kBackwards flag isn't enough
    // so we base our decision on the direction of the actual change we've computed.
    bool forewards = delSecs >= 0;

    if (fFlags & kLoop)
    {
        bool wrapped = false;

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
                    float result = fmodf(secs - fLoopBegin, fLoopEnd - fLoopBegin) + fLoopBegin;
                    // are they a dumb ass?
                    if (!std::isnan(result))
                    {
                        secs = result;
                        wrapped = true;
                    }
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
                    float result  = fLoopEnd - fmodf(fLoopEnd - secs, fLoopEnd - fLoopBegin);
                    // are they a dumb ass?
                    if (!std::isnan(result))
                    {
                        secs = result;
                        wrapped = true;
                    }
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
            if (((wrapped && (forewards && secs >= fWrapTime)) ||
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
    if (fEaseOutCurve != nullptr && !(fFlags & kEasingIn) && wSecs >= fEaseOutCurve->GetEndWorldTime())
        IStop(wSecs, secs);
    
    return fCurrentAnimTime = secs;
}

float plAnimTimeConvert::WorldToAnimTimeNoUpdate(double wSecs) const
{
    return IWorldToAnimTimeNoUpdate(wSecs, IGetState(wSecs));
}

float plAnimTimeConvert::IWorldToAnimTimeNoUpdate(double wSecs, plATCState *state)
{
    //hsAssert(wSecs >= fLastEvalWorldTime, "Tried to eval a time that's earlier than the last eval time.");
    if (state == nullptr)
        return 0;
    
    if (state->fFlags & kStopped)
        return state->fStartAnimTime;
    
    float secs = 0, delSecs = 0;
    
    if (state->fEaseCurve != nullptr)
    {
        delSecs += ICalcEaseTime(state->fEaseCurve, state->fStartWorldTime, wSecs);
        if (wSecs > state->fEaseCurve->GetEndWorldTime())
        {
            if (state->fFlags & kEasingIn)
                delSecs += float(wSecs - state->fEaseCurve->GetEndWorldTime()) * state->fSpeed;
        }
    }
    else 
    {
        // The easy case... playing the animation at a constant speed.
        delSecs = float(wSecs - state->fStartWorldTime) * state->fSpeed;
    }
    
    if (state->fFlags & kBackwards)
        delSecs = -delSecs;
    
    secs = state->fStartAnimTime + delSecs;
    // At this point, "secs" is the pre-wrapped (before looping) anim time. 
    // "delSecs" is the change in anim time
    bool forewards = delSecs >= 0;

    if (state->fFlags & kLoop)
    {
        bool wrapped = false;

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
            if (((wrapped && (forewards && secs >= state->fWrapTime)) ||
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

float plAnimTimeConvert::IWorldToAnimTimeBeforeState(double wSecs) const
{
    return IWorldToAnimTimeNoUpdate(wSecs, IGetState(wSecs));
}

void plAnimTimeConvert::SetCurrentAnimTime(float s, bool jump /* = false */)
{
    // We're setting the anim value for whenever we last evaluated.
    fFlags |= kForcedMove;
    if (!jump)
        ICheckTimeCallbacks(fCurrentAnimTime, s);
    fCurrentAnimTime = s;
    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kSingleFrameAdjust)
        {
            ISendCallback(i);
        }
    }   
    IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
}

void plAnimTimeConvert::SetEase(bool easeIn, uint8_t type, float minLength, float maxLength, float normLength) 
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



float plAnimTimeConvert::GetBestStopDist(float min, float max, float norm, float time) const
{
    float bestTime = -1;
    float bestDist = -1;
    if (fStopPoints.empty())
        return norm;

    float curTime;
    float curDist;

    for (float stop : fStopPoints)
    {
        if (IsLooped())
        {
            float loopDist;
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
    
    hsStatusMessageF("found stop point {}", bestTime);

    if (bestTime == -1)
        bestTime = norm;
    return bestTime;
}

// Passing in a rate of zero specifies an immediate change.
void plAnimTimeConvert::SetSpeed(float goal, float rate /* = 0 */)
{
    float curSpeed = fSpeed;
    fSpeed = goal;
    

    if (rate == 0)
    {
        IClearSpeedEase();
        fCurrentEaseCurve = nullptr;

    }
    // Skip if we're either stopped or stopping. We'll take the new speed into account next time we start up.
    else if ((fFlags & kEasingIn)) 
    {
        double curTime = hsTimer::GetSysSeconds();
        if (fCurrentEaseCurve != nullptr)
        {
            double easeTime = curTime - fCurrentEaseCurve->fBeginWorldTime;
            curSpeed = fCurrentEaseCurve->VelocityGivenTime((float)easeTime);
        }
        if (fSpeedEaseCurve != nullptr)
        {
            fSpeedEaseCurve->RecalcToSpeed(curSpeed, goal);
            fSpeedEaseCurve->SetLengthOnRate(rate);
        }
        else
        {
            float length;
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

    fFlags = s->ReadLE32();

    fBegin = fInitialBegin = s->ReadLEFloat();
    fEnd = fInitialEnd = s->ReadLEFloat();
    fLoopEnd = s->ReadLEFloat();
    fLoopBegin = s->ReadLEFloat();
    fSpeed = s->ReadLEFloat();

    fEaseInCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
    fEaseOutCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
    fSpeedEaseCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));

    fCurrentAnimTime = s->ReadLEFloat();
    fLastEvalWorldTime = s->ReadLEDouble();

    // load other non-synched data;
    uint32_t count = s->ReadLE32();
    fCallbackMsgs.resize(count);

    for (uint32_t i = 0; i < count; i++)
    {
        plEventCallbackMsg* msg = plEventCallbackMsg::ConvertNoRef(mgr->ReadCreatable(s));
        fCallbackMsgs[i] = msg;
    }

    count = s->ReadLE32();
    fStopPoints.resize(count);
    s->ReadLEFloat(count, fStopPoints.data());

    IProcessStateChange(0, fBegin);
}

void plAnimTimeConvert::Write(hsStream* s, hsResMgr* mgr)
{
    plCreatable::Write(s, mgr);

    s->WriteLE32(fFlags);

    s->WriteLEFloat(fBegin);
    s->WriteLEFloat(fEnd);
    s->WriteLEFloat(fLoopEnd);
    s->WriteLEFloat(fLoopBegin);
    s->WriteLEFloat(fSpeed);

    mgr->WriteCreatable(s, fEaseInCurve);
    mgr->WriteCreatable(s, fEaseOutCurve);
    mgr->WriteCreatable(s, fSpeedEaseCurve);

    s->WriteLEFloat(fCurrentAnimTime);
    s->WriteLEDouble(fLastEvalWorldTime);

    // save out other non-synched important data
    s->WriteLE32((uint32_t)fCallbackMsgs.size());
    for (plEventCallbackMsg* msg : fCallbackMsgs)
        mgr->WriteCreatable(s, msg);

    s->WriteLE32((uint32_t)fStopPoints.size());
    s->WriteLEFloat(fStopPoints.size(), fStopPoints.data());
}

plAnimTimeConvert& plAnimTimeConvert::InitStop() 
{ 
    return IStop(hsTimer::GetSysSeconds(), fCurrentAnimTime);
}

plAnimTimeConvert& plAnimTimeConvert::Stop(bool on) 
{ 
    if( on )
        return Stop();
    else
        return Start();
}

plAnimTimeConvert& plAnimTimeConvert::Stop(double stopTime) 
{   
    if (IsStopped() || (fEaseOutCurve != nullptr && !(fFlags & kEasingIn)))
        return *this;

    if (stopTime < 0)
        stopTime = hsTimer::GetSysSeconds();
    float stopAnimTime = WorldToAnimTimeNoUpdate(stopTime);
    
    SetFlag(kEasingIn, false);

    if (fEaseOutCurve == nullptr)
    {
        return IStop(stopTime, fCurrentAnimTime);
    }

    float currSpeed;
    if (fCurrentEaseCurve == nullptr || stopTime >= fCurrentEaseCurve->GetEndWorldTime())
        currSpeed = fSpeed;
    else
        currSpeed = fCurrentEaseCurve->VelocityGivenTime((float)(stopTime - fCurrentEaseCurve->fBeginWorldTime));

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
    
    if (fEaseInCurve != nullptr)
    {
        float currSpeed;
        if (fCurrentEaseCurve == nullptr || startTime >= fCurrentEaseCurve->GetEndWorldTime())
            currSpeed = 0;
        else
            currSpeed = fCurrentEaseCurve->VelocityGivenTime((float)(startTime - fCurrentEaseCurve->fBeginWorldTime));

        if (currSpeed <= fSpeed)
        {
            fEaseInCurve->RecalcToSpeed(0, fSpeed);
            fEaseInCurve->fBeginWorldTime = startTime - fEaseInCurve->TimeGivenVelocity(currSpeed);

            fCurrentEaseCurve = fEaseInCurve;
            
        }
        else
        {   // We eased out in the middle of a speed change, but were told to start again before slowing past
            // the target speed, so the "ease in" is really a slow down.
            SetSpeed(fSpeed, fEaseInCurve->fSpeed / fEaseInCurve->fLength);
        }
    }
    
    // check for a start callback
    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kStart)
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

plAnimTimeConvert& plAnimTimeConvert::Backwards(bool on) 
{
    return on ? Backwards() : Forewards();
}

plAnimTimeConvert& plAnimTimeConvert::Backwards() 
{ 
    if( IsBackwards() )
        return *this;

    // check for a reverse callback
    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kReverse)
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
    for (hsSsize_t i = fCallbackMsgs.size() - 1; i >= 0; --i)
    {
        if (fCallbackMsgs[i]->fEvent == plEventCallbackMsg::kReverse)
        {
            ISendCallback(i);
        }
    }
    
    SetFlag(kBackwards, false); 
    
    // Record state changes
    IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
    
    return *this;
}

plAnimTimeConvert& plAnimTimeConvert::Loop(bool on) 
{ 
    SetFlag(kLoop, on); 
    
    // Record state changes
    IProcessStateChange(hsTimer::GetSysSeconds(), fCurrentAnimTime);
    
    return *this;
}

plAnimTimeConvert& plAnimTimeConvert::PlayToTime(float time)
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

plAnimTimeConvert& plAnimTimeConvert::PlayToPercentage(float percent)
{
    return PlayToTime(fBegin + (fEnd - fBegin) * percent);
}

void plAnimTimeConvert::RemoveCallback(plEventCallbackMsg* pMsg)
{
    auto iter = std::find(fCallbackMsgs.cbegin(), fCallbackMsgs.cend(), pMsg);
    if (iter != fCallbackMsgs.cend())
    {
        hsRefCnt_SafeUnRef(*iter);
        fCallbackMsgs.erase(iter);
    }
}

bool plAnimTimeConvert::HandleCmd(plAnimCmdMsg* modMsg)
{
    if (fFlags & kNeedsReset)
        ResetWrap();

    // The net msg screener is already checking for callbacks,
    // I'm just being extra safe.
    if (!modMsg->HasBCastFlag(plMessage::kNetCreatedRemotely))
    {
        if( modMsg->Cmd(plAnimCmdMsg::kAddCallbacks) )
        {
            for (size_t i = 0; i < modMsg->GetNumCallbacks(); i++)
            {
                AddCallback(plEventCallbackMsg::ConvertNoRef(modMsg->GetEventCallback(i)));
            }
        }
        if( modMsg->Cmd(plAnimCmdMsg::kRemoveCallbacks) )
        {
            for (size_t i = 0; i < modMsg->GetNumCallbacks(); i++)
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
        float newTime = fCurrentAnimTime + hsTimer::GetDelSysSeconds();
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
        float newTime = fCurrentAnimTime - hsTimer::GetDelSysSeconds();
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
    fCallbackMsgs.emplace_back(pMsg);
}

void plAnimTimeConvert::ClearCallbacks()
{
    for (plEventCallbackMsg* msg : fCallbackMsgs)
    {
        hsRefCnt_SafeUnRef(msg);
    }
    fCallbackMsgs.clear();
}

void plAnimTimeConvert::EnableCallbacks(bool val)
{
    SetFlag(kNoCallbacks, !val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void plATCState::Read(hsStream *s, hsResMgr *mgr)
{
    fStartWorldTime = s->ReadLEDouble();
    fStartAnimTime = s->ReadLEFloat();

    fFlags = s->ReadLE32();
    fEnd = s->ReadLEFloat();
    fLoopBegin = s->ReadLEFloat();
    fLoopEnd = s->ReadLEFloat();
    fSpeed = s->ReadLEFloat();
    fWrapTime = s->ReadLEFloat();
    if (s->ReadBool())
        fEaseCurve = plATCEaseCurve::ConvertNoRef(mgr->ReadCreatable(s));
}

void plATCState::Write(hsStream *s, hsResMgr *mgr)
{
    s->WriteLEDouble(fStartWorldTime);
    s->WriteLEFloat(fStartAnimTime);

    s->WriteLE32(fFlags);
    s->WriteLEFloat(fEnd);
    s->WriteLEFloat(fLoopBegin);
    s->WriteLEFloat(fLoopEnd);
    s->WriteLEFloat(fSpeed);
    s->WriteLEFloat(fWrapTime);
    if (fEaseCurve != nullptr)
    {
        s->WriteBool(true);
        mgr->WriteCreatable(s, fEaseCurve);
    }
    else
        s->WriteBool(false);
}





















