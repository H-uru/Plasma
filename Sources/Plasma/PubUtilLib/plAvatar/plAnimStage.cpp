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

// singular
#include "plAnimStage.h"

// local
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainGeneric.h"
#include "plMultistageBehMod.h"

// global
#include "hsResMgr.h"
#include "hsTimer.h"

#include <cstdio>

// other
#include "pnMessage/plNotifyMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGModifier.h"
#include "plAnimation/plMatrixChannel.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plPipeline/plDebugText.h"

// PLANIMSTAGE default ctor
plAnimStage::plAnimStage()
: fNotify(),
  fArmature(),
  fBrain(),
  fForwardType(kForwardNone),
  fBackType(kBackNone),
  fAdvanceType(kAdvanceNone),
  fRegressType(kRegressNone),
  fLoops(),
  fAnimInstance(),
  fLocalTime(),
  fLength(),
  fCurLoop(),
  fAttached(),
  fDoAdvanceTo(),
  fAdvanceTo(),
  fDoRegressTo(),
  fRegressTo(),
  fMod(),
  fSentNotifies(),
  fReverseOnIdle(),
  fDone()
{
}

plAnimStage::plAnimStage(const ST::string &animName, uint8_t notify)
: fAnimName(animName),
  fNotify(notify),
  fArmature(),
  fBrain(),
  fForwardType(kForwardAuto),       // different from default
  fBackType(kBackNone),
  fAdvanceType(kAdvanceAuto),       // different from default
  fRegressType(kRegressNone),
  fLoops(),
  fAnimInstance(),
  fLocalTime(),
  fLength(),
  fCurLoop(),
  fAttached(),
  fDoAdvanceTo(),
  fAdvanceTo(),
  fDoRegressTo(),
  fRegressTo(),
  fMod(),
  fSentNotifies(),
  fReverseOnIdle(),
  fDone()
{
}


// PLANIMSTAGE canonical ctor
plAnimStage::plAnimStage(const ST::string &animName,
                         uint8_t notify,
                         ForwardType forward,
                         BackType back,
                         AdvanceType advance,
                         RegressType regress,
                         int loops)
: fAnimName(animName),
  fArmature(),
  fBrain(),
  fNotify(notify),
  fForwardType(forward),
  fBackType(back),
  fAdvanceType(advance),
  fRegressType(regress),
  fLoops(loops),
  fAnimInstance(),
  fLocalTime(),
  fLength(),
  fCurLoop(),
  fAttached(),
  fDoAdvanceTo(),
  fAdvanceTo(),
  fDoRegressTo(),
  fRegressTo(),
  fMod(),
  fSentNotifies(),
  fReverseOnIdle(),
  fDone()
{
}

plAnimStage::plAnimStage(const ST::string &animName,
                         uint8_t notify,
                         ForwardType forward,
                         BackType back,
                         AdvanceType advance,
                         RegressType regress,
                         int loops,
                         bool doAdvanceTo,
                         int advanceTo,
                         bool doRegressTo,
                         int regressTo)
: fAnimName(animName),
  fArmature(),
  fBrain(),
  fNotify(notify),
  fForwardType(forward),
  fBackType(back),
  fAdvanceType(advance),
  fRegressType(regress),
  fLoops(loops),
  fAnimInstance(),
  fLocalTime(),
  fLength(),
  fCurLoop(),
  fAttached(),
  fDoAdvanceTo(doAdvanceTo),
  fAdvanceTo(advanceTo),
  fDoRegressTo(doRegressTo),
  fRegressTo(regressTo),
  fMod(),
  fSentNotifies(),
  fReverseOnIdle(),
  fDone()
{
}

// PLANIMSTAGE dtor
plAnimStage::~plAnimStage()
{
    hsAssert(fAnimInstance == nullptr, "plAnimStage still has anim instance during destruction. (that's bad.)");
    // we could delete the animation instance here, but it should have been deleted already...
    // *** check back in a while....
}

// operator= ----------------------------------------------------
// ----------
const plAnimStage& plAnimStage::operator=(const plAnimStage& src)
{
    fAnimName = src.fAnimName;
    fNotify = src.fNotify;
    fForwardType = src.fForwardType;
    fBackType = src.fBackType;
    fAdvanceType = src.fAdvanceType;
    fRegressType = src.fRegressType;
    fLoops = src.fLoops;
    fDoAdvanceTo = src.fDoAdvanceTo;
    fAdvanceTo = src.fAdvanceTo;
    fDoRegressTo = src.fDoRegressTo;
    fRegressTo = src.fRegressTo;
    fMod = src.fMod;

    fAnimInstance = nullptr;
    fLocalTime = 0.0f;
    fLength = 0.0f;
    fCurLoop = 0;
    fAttached = false;

    fReverseOnIdle = src.fReverseOnIdle;

    return *this;
}

// attach --------------------------------------------------------------------------------------------------------
// -------
plAGAnimInstance * plAnimStage::Attach(plArmatureMod *armature, plArmatureBrain *brain, float initialBlend, double time)
{
    // NOTE: you need to be able to detach an animstage and then re-attach it and have
    // it wind up in exactly the same state it was in before - for loading and saving.
    fBrain = brain;
    fSentNotifies = 0;
    fArmature = armature;

    if(fAnimInstance)
    {
        fAnimInstance->SetBlend(initialBlend);
    } else {
        plAGAnim *anim = armature->FindCustomAnim(fAnimName);

        if(anim)
        {
            fLength = anim->GetEnd();
            fAnimInstance = armature->AttachAnimationBlended(anim, initialBlend);
            fAnimInstance->SetCurrentTime(fLocalTime);
#ifdef DEBUG_MULTISTAGE
            char sbuf[256];
            snprintf(sbuf, std::size(sbuf), "AnimStage::Attach - attaching stage %s", fAnimName.c_str());
            plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
        } else {
            char buf[256];
            snprintf(buf, std::size(buf), "Can't find animation <%s> for animation stage. Anything could happen.", fAnimName.c_str());
            hsAssert(false, buf);
#ifdef DEBUG_MULTISTAGE
            plAvatarMgr::GetInstance()->GetLog()->AddLine(buf);
#endif
        }
    }

    if(fAnimInstance)
    {
        fAnimInstance->Stop();      // we'll be setting the time directly.
        fAnimatedHandle = (fAnimInstance->GetAnimation()->GetChannel("Handle") != nullptr);
        fAttached = true;
        // this is too early to send the enter notify. we're attached, but we may not
        // have faded in yet.
        // XXX ISendNotify(kNotifyEnter, proEventData::kEnterStage, armature, brain);
    }
        
    return fAnimInstance;
}

// SENDNOTIFY
bool plAnimStage::ISendNotify(uint32_t notifyMask, uint32_t notifyType, plArmatureMod *armature, plArmatureBrain *brain)
{
    // make sure the user has requested this type of notify
    if(fNotify & notifyMask)
    {
        plKey avKey = armature->GetTarget(0)->GetKey();
        if (fMod)
            avKey = fMod->GetKey();
        plNotifyMsg *msg = new plNotifyMsg();
        msg->SetSender(avKey);

        if (fMod)
        {
            msg->SetBCastFlag(plMessage::kNetPropagate, fMod->NetProp());
            msg->SetBCastFlag(plMessage::kNetForce, fMod->NetForce());
        }
        else
        {
            msg->SetBCastFlag(plMessage::kNetPropagate, false);
            msg->SetBCastFlag(plMessage::kNetForce, false);
        }

        plAvBrainGeneric *genBrain = plAvBrainGeneric::ConvertNoRef(brain);
        int stageNum = genBrain ? genBrain->GetStageNum(this) : -1;
        msg->AddMultiStageEvent(stageNum, notifyType, armature->GetTarget(0)->GetKey());

        if (!genBrain || !genBrain->RelayNotifyMsg(msg))
        {
            msg->UnRef();   // couldn't send; destroy...
        }

        return true;
    }

    return false;
}

// DETACH
bool plAnimStage::Detach(plArmatureMod *armature)
{

    bool result = false;

#ifdef DEBUG_MULTISTAGE
    char sbuf[256];
    snprintf(sbuf, std::size(sbuf), "AnimStage::Detach - detaching stage %s", fAnimName.c_str());
    plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
//  hsStatusMessageF("Detaching plAnimStage <%s>", fAnimName.c_str());
    if(fArmature) {
        fArmature = nullptr;

        if(fAnimInstance) {
            armature->DetachAnimation(fAnimInstance);       // detach instantly
            fAnimInstance = nullptr;
            result =  true;
        }
#ifdef DEBUG_MULTISTAGE
    } else {
        plAvatarMgr::GetInstance()->GetLog()->AddLine("AnimStage::Detach: stage already detached");
#endif
//      hsStatusMessageF("Detach: stage already detached.");
    }
    
    fBrain = nullptr;
    fAttached = false;
    return result;
}

void plAnimStage::Reset(double time, plArmatureMod *avMod, bool atStart)
{
    if(atStart)
        SetLocalTime(0.0f, true);
    else
        SetLocalTime(fLength, true);
    avMod->GetRootAnimator()->Reset(time);
}

void plAnimStage::ResetAtTime(double globalTime, float localTime, plArmatureMod *avMod)
{
    SetLocalTime(localTime, true);
    avMod->GetRootAnimator()->Reset(globalTime);
}


// MoveRelative ------------------------------
// -------------
// A true result means that the stage is done.
bool plAnimStage::MoveRelative(double time, float delta, float &overage, plArmatureMod *avMod)
{
    bool result;        // true means the stage is done

    if(fLocalTime == 0.0f && delta >= 0.0f && !hsCheckBits(fSentNotifies, kNotifyEnter))
    {
        // we send the "enter" notify if we're at the start and either moving forward
        // or standing still.
        ISendNotify(kNotifyEnter, proEventData::kEnterStage, avMod, fBrain);
        hsSetBits(fSentNotifies, kNotifyEnter);
    }

    // aborting...
    if( fAdvanceType == kAdvanceOnMove && (avMod->HasMovementFlag() || avMod->ExitModeKeyDown()))
    {   // special case: advance when any key is pressed, regardless of position in stage.
        ISendNotify(kNotifyAdvance, proEventData::kAdvanceNextStage, avMod, fBrain);
        result = true;
    } else {
        if(delta == 0.0f)
        {
            return false;
        }   
        else
        if(delta < 0.0f)
            result = IMoveBackward(time, delta, overage, avMod);
        else
            result = IMoveForward(time, delta, overage, avMod);
    }

    return result;
}

// IMoveBackward ------------------------------------------------------------------------------
// --------------
bool plAnimStage::IMoveBackward(double time, float delta, float &overrun, plArmatureMod *avMod)
{
    if (fLocalTime <= 0 && fRegressType == kRegressNone)
    {
        // If we're at the beginning, but not allowed to regress, we don't want to keep processing
        // (and firing triggers).
        return false;
    }

    float target = fLocalTime + delta;
    bool infiniteLoop = fLoops == -1;
    bool loopsRemain = fCurLoop > 0 || infiniteLoop;

    // If we don't have this animation, just pretend to have worked.
    // Otherwise, we crash the client.
    if (!fAnimInstance)
    {
        hsAssert(false, "AnimInstance nil");
        return true;
    }

    // This must be here before we set the local time.
    if (fAnimInstance->GetTimeConvert())
        fAnimInstance->GetTimeConvert()->Backwards();

    if(target < 0)
    {
        SetLocalTime(0);                // animation to beginning
        avMod->GetRootAGMod()->Apply(time);             // move avatar to beginning
        if(loopsRemain)
        {
            // If a callback is on the last frame, it'll get triggered twice. Once for setting
            // the anim at the end, and once when we play again. So we don't fire callbacks here.
            SetLocalTime(fLength, true);        // animation wraps to end

            avMod->GetRootAnimator()->Reset(time);          // reset the root animator at the end
            fCurLoop = infiniteLoop ? 0 : fCurLoop - 1;
            target = fLength - (fmodf(-target, fLength));   // modularize negative number to discard 
                                                            // extra loops (only one allowed)
        } else {
            // overrun = target + fLength;
            // now we want to make sure that overrun goes negative when appropriate, rather than modularizing
            overrun = target;
            fDone = true;
            return ITryRegress(avMod);
        }
    }

    overrun = 0.0f;
    fLocalTime = target;
        
    fAnimInstance->SetCurrentTime(fLocalTime);
    return false;   // not done
}


// IMoveForward ------------------------------------------------------------------------------
// -------------
// It's currently not supported to advance the animation by more than its length in one frame.
// It wouldn't be too hard to add, but it clutters things up and it hasn't been shown to 
// be necessary.
bool plAnimStage::IMoveForward(double time, float delta, float &overrun, plArmatureMod *avMod)
{
    if (fLocalTime >= fLength && fAdvanceType == kAdvanceNone)
    {
        // If we're at the end, but not allowed to advance, we don't want to keep processing
        // (and firing triggers).
        return false;
    }
    
    // first get the target time in local time, ignoring overruns
    float target = fLocalTime + delta;

    // If we don't have this animation, just pretend to have worked.
    // Otherwise, we crash the client.
    if (!fAnimInstance)
    {
        hsAssert(false, "AnimInstance nil");
        return true;
    }

    if (fAnimInstance->GetTimeConvert())
        fAnimInstance->GetTimeConvert()->Forewards();
    

    if (target > fLength)
    {
        // we're going to the end for sure, so do that first
        SetLocalTime(fLength);
        // we're going to swap in a new animation before the next eval, so force
        // an apply of this one to make sure the avatar gets the necessary movement.
        // we only apply on the root node to get the movement -- no need to animate
        // the fingers as they'll be overwritten when the next animation is swapped in.
        avMod->GetRootAGMod()->Apply(time);

        // are there *any* loops to be had?
        bool loopsRemain = fCurLoop < fLoops || fLoops == -1;
        if(loopsRemain)
        {
            SetLocalTime(0.0f, true);                               // animation back to beginning
            avMod->GetRootAnimator()->Reset(time);              // reset the root animator's frame cache
            fCurLoop++;
            target = fmodf(target, fLength);                // discard extra loops (only one at a time allowed)
            // target -= fLength;
        } else {
            overrun = target - fLength;
            fDone = true;
            return ITryAdvance(avMod);
        }
    }

    overrun = 0.0f;
    fLocalTime = target;
    
    avMod->GetRootAGMod()->Apply(time);
    fAnimInstance->SetCurrentTime(fLocalTime);
    return false;   // not done
}

bool plAnimStage::ITryAdvance(plArmatureMod *avMod)
{
    bool stageDone = false;


    // hsStatusMessageF("Sending advance message for stage <%s>\n", fAnimName.c_str());
    if(fAdvanceType == kAdvanceAuto || fAdvanceType == kAdvanceOnMove) {
        stageDone = true;
    }

    if(!hsCheckBits(fSentNotifies, kNotifyAdvance))
    {
        // we send the advance message at the point where we *would* advance, whether
        // or not we actually do. this is misleading but better suited to actual current usage.
        // we may want to rename this to "ReachedStageEnd"
        ISendNotify(kNotifyAdvance, proEventData::kAdvanceNextStage, avMod, fBrain);
        hsSetBits(fSentNotifies, kNotifyAdvance);
    }

    return stageDone;
}


bool plAnimStage::ITryRegress(plArmatureMod *avMod)
{
    bool stageDone = false;

    // we send the advance message at the point where we *would* advance, whether
    // or not we actually do. this is misleading but better suited to actual current usage.
    // we may want to rename this to "ReachedStageEnd"
    ISendNotify(kNotifyRegress, proEventData::kRegressPrevStage, avMod, fBrain);

    // hsStatusMessageF("Sending regress message for stage <%s>\n", fAnimName.c_str());
    if(fRegressType == kRegressAuto) {
        stageDone = true;
    }
    return stageDone;
}


// GETFORWARDTYPE
plAnimStage::ForwardType plAnimStage::GetForwardType()
{
    return fForwardType;
}

// SETFORWARDTYPE
void plAnimStage::SetForwardType(ForwardType t)
{
    fForwardType = t;
}

// GETBACKTYPE
plAnimStage::BackType plAnimStage::GetBackType()
{
    return fBackType;
}

// SETBACKTYPE
void plAnimStage::SetBackType(BackType t)
{
    fBackType = t;
}

// GETADVANCETYPE
plAnimStage::AdvanceType plAnimStage::GetAdvanceType()
{
    return fAdvanceType;
}

// SETADVANCETYPE
void plAnimStage::SetAdvanceType(AdvanceType t)
{
    fAdvanceType = t;
}

// GETREGRESSTYPE
plAnimStage::RegressType plAnimStage::GetRegressType()
{
    return fRegressType;
}

// SETREGRESSTYPE
void plAnimStage::SetRegresstype(RegressType t)
{
    fRegressType = t;
}

// GETNOTIFYFLAGS
uint32_t plAnimStage::GetNotifyFlags()
{
    return fNotify;
}

// SETNOTIFYFLAGS
void plAnimStage::SetNotifyFlags(uint32_t newFlags)
{
    fNotify = (uint8_t)newFlags;
}

// GETNUMLOOPS
int plAnimStage::GetNumLoops()
{
    return fLoops;
}

// SETNUMLOOPS
void plAnimStage::SetNumLoops(int loops)
{
    // I'm very suspicious of this if statement...
    // 1. It's preceded by an assert whose condition is the opposite the error message
    //    (which I've now commented out)
    // 2. It only matters if the avatar is currently in the stage.
    // 3. It doesn't seem intuitive that if I'm currently on loop 3 and you
    //    change the number of loops to 6, that I should jump to the end.
    // BUT...
    // It's been like this for ages, so I'm not touching it until a break shows a problem.
    //
    //hsAssert(loops < fCurLoop, "Setting loopcount below current loop");
    if(loops >= fCurLoop) {
        fCurLoop = loops;
    }
    fLoops = loops;
}

// GETLOOPVALUE
int plAnimStage::GetLoopValue()
{
    return fCurLoop;
}

// SETLOOPVALUE
void plAnimStage::SetLoopValue(int value)
{
    fCurLoop = value;
}

// GETLOCALTIME
float plAnimStage::GetLocalTime()
{
    return fLocalTime;
}

// SETLOCALTIME
void plAnimStage::SetLocalTime(float time, bool noCallbacks /* = false */)
{
    fLocalTime = time;
    if(fAnimInstance)
        fAnimInstance->SetCurrentTime(time, noCallbacks);
}

// GETLENGTH
float plAnimStage::GetLength()
{
    return fLength;
}

// GETISATTACHED
bool plAnimStage::GetIsAttached()
{
    return fAttached;
}

// SETISATTACHED
void plAnimStage::SetIsAttached(bool status)
{
    fAttached = status;
}

// GETNEXTSTAGE
int plAnimStage::GetNextStage(int curStage)
{
    if(fDoAdvanceTo)
    {
        return fAdvanceTo;
    } else {
        return curStage + 1;
    }
}

// GETPREVSTAGE
int plAnimStage::GetPrevStage(int curStage)
{
    if(fDoRegressTo)
    {
        return fRegressTo;
    } else {
        return curStage - 1;
    }
}

// DUMPDEBUG
void plAnimStage::DumpDebug(bool active, int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    ST::string_stream str;

    str << fAnimName;
    str << " ";

    if(fLoops)
        str << "loop(" << fCurLoop << '/' << fLoops << ')';

    str << "time: (" << fLocalTime << '/' << fLength << ')';

    if(active)
        debugTxt.DrawString(x, y, str.to_string(), 0, 255, 0);
    else if(fAnimInstance)
        debugTxt.DrawString(x, y, str.to_string());
    else
        debugTxt.DrawString(x, y, str.to_string(), 255, 255, 0);

    y += lineHeight;
}

// READ
void plAnimStage::Read(hsStream *stream, hsResMgr *mgr)
{
    fAnimName = stream->ReadSafeString();
    fNotify = stream->ReadByte();
    fForwardType = (ForwardType)stream->ReadLE32();
    fBackType = (BackType)stream->ReadLE32();
    fAdvanceType = (AdvanceType)stream->ReadLE32();
    fRegressType = (RegressType)stream->ReadLE32();
    fLoops = stream->ReadLE32();

    fDoAdvanceTo = stream->ReadBool();
    fAdvanceTo = stream->ReadLE32();
    fDoRegressTo = stream->ReadBool();
    fRegressTo = stream->ReadLE32();
}

void plAnimStage::Write(hsStream *stream, hsResMgr *mgr)
{
    stream->WriteSafeString(fAnimName);
    stream->WriteByte(fNotify);
    stream->WriteLE32((uint32_t)fForwardType);
    stream->WriteLE32((uint32_t)fBackType);
    stream->WriteLE32((uint32_t)fAdvanceType);
    stream->WriteLE32((uint32_t)fRegressType);
    stream->WriteLE32(fLoops);

    stream->WriteBool(fDoAdvanceTo);
    stream->WriteLE32(fAdvanceTo);
    stream->WriteBool(fDoRegressTo);
    stream->WriteLE32(fRegressTo);
}


// SAVEAUX
void plAnimStage::SaveAux(hsStream *stream, hsResMgr *mgr)
{
    stream->WriteLEFloat(fLocalTime);
    stream->WriteLEFloat(fLength);
    stream->WriteLE32(fCurLoop);
    stream->WriteBool(fAttached);
    // no ephemeral stage at the moment
}

// LOADAUX
void plAnimStage::LoadAux(hsStream *stream, hsResMgr *mgr, double time)
{
    fLocalTime = stream->ReadLEFloat();
    fLength = stream->ReadLEFloat();
    fCurLoop = stream->ReadLE32();
    fAttached = stream->ReadBool();
}

