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

#include "plLayerAnimation.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include <cmath>

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnMessage/plSDLNotificationMsg.h"
#include "pnNetCommon/plSDLTypes.h"

#include "plInterp/plController.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plModifier/plLayerSDLModifier.h"
#include "plModifier/plSDLModifier.h"
#include "plNetClient/plLinkEffectsMgr.h"
#include "plSDL/plSDL.h"


plLayerAnimationBase::~plLayerAnimationBase()
{
    delete fPreshadeColorCtl;
    delete fRuntimeColorCtl;
    delete fAmbientColorCtl;
    delete fSpecularColorCtl;
    delete fOpacityCtl;
    delete fTransformCtl;
}

void plLayerAnimationBase::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Read(s, mgr);

    fPreshadeColorCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fRuntimeColorCtl = plController::ConvertNoRef( mgr->ReadCreatable( s ) );
    fAmbientColorCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fSpecularColorCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fOpacityCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fTransformCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));

    if( fOpacityCtl )
    {
        fOwnedChannels |= kOpacity;
        fOpacity = new float;
    }
    if( fPreshadeColorCtl )
    {
        fOwnedChannels |= kPreshadeColor;
        fPreshadeColor = new hsColorRGBA;
    }
    if( fRuntimeColorCtl )
    {
        fOwnedChannels |= kRuntimeColor;
        fRuntimeColor = new hsColorRGBA;
    }
    if( fAmbientColorCtl )
    {
        fOwnedChannels |= kAmbientColor;
        fAmbientColor = new hsColorRGBA;
    }
    if( fSpecularColorCtl )
    {
        fOwnedChannels |= kSpecularColor;
        fSpecularColor = new hsColorRGBA;
    }
    if( fTransformCtl )
    {
        fOwnedChannels |= kTransform;
        fTransform = new hsMatrix44;
    }
    fLength = IMakeUniformLength();
}

void plLayerAnimationBase::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Write(s, mgr);

    mgr->WriteCreatable(s, fPreshadeColorCtl);
    mgr->WriteCreatable(s, fRuntimeColorCtl);
    mgr->WriteCreatable(s, fAmbientColorCtl);
    mgr->WriteCreatable(s, fSpecularColorCtl);
    mgr->WriteCreatable(s, fOpacityCtl);
    mgr->WriteCreatable(s, fTransformCtl);
}

plLayerInterface* plLayerAnimationBase::Attach(plLayerInterface* prev)
{
    return plLayerInterface::Attach(prev);
}

void plLayerAnimationBase::IEvalConvertedTime(float secs, uint32_t passChans, uint32_t evalChans, uint32_t &dirty)
{
    if( evalChans & kPreshadeColor )
    {
        fPreshadeColorCtl->Interp(fCurrentTime, fPreshadeColor);
        dirty |= kPreshadeColor;
    }
    else if( passChans & kPreshadeColor )
    {
        *fPreshadeColor = fUnderLay->GetPreshadeColor();
    }

    if( evalChans & kRuntimeColor )
    {
        fRuntimeColorCtl->Interp( fCurrentTime, fRuntimeColor );
        dirty |= kRuntimeColor;
    }
    else if( passChans & kRuntimeColor )
    {
        *fRuntimeColor = fUnderLay->GetRuntimeColor();
    }

    if( evalChans & kAmbientColor )
    {
        fAmbientColorCtl->Interp(fCurrentTime, fAmbientColor);
        dirty |= kAmbientColor;
    }
    else if( passChans & kAmbientColor )
    {
        *fAmbientColor = fUnderLay->GetAmbientColor();
    }

    if( evalChans & kSpecularColor )
    {
        fSpecularColorCtl->Interp( fCurrentTime, fSpecularColor );
        dirty |= kSpecularColor;
    }
    else if( passChans & kSpecularColor )
    {
        *fSpecularColor = fUnderLay->GetSpecularColor();
    }

    if( evalChans & kOpacity )
    {
        fOpacityCtl->Interp(fCurrentTime, fOpacity);
        *fOpacity *= 1.e-2f;
        dirty |= kOpacity;
    }
    else if( passChans & kOpacity )
    {
        *fOpacity = fUnderLay->GetOpacity();
    }

    if( evalChans & kTransform )
    {
        fTransformCtl->Interp(fCurrentTime, fTransform);
        dirty |= kTransform;
    }
    else if( passChans & kTransform )
    {
        *fTransform = fUnderLay->GetTransform();
    }
    fPassThruChannels = 0; // already handled, don't need to keep passing them through.
}

bool plLayerAnimationBase::MsgReceive(plMessage* msg)
{
    return plLayerInterface::MsgReceive(msg);
}

void plLayerAnimationBase::SetPreshadeColorCtl(plController* colCtl)
{
    if( fPreshadeColorCtl )
        delete fPreshadeColorCtl;
    else
        fPreshadeColor = new hsColorRGBA;

    fOwnedChannels |= kPreshadeColor;
    fPreshadeColorCtl = colCtl;
}

void plLayerAnimationBase::SetRuntimeColorCtl(plController* colCtl)
{
    if( fRuntimeColorCtl )
        delete fRuntimeColorCtl;
    else
        fRuntimeColor = new hsColorRGBA;

    fOwnedChannels |= kRuntimeColor;
    fRuntimeColorCtl = colCtl;
}

void plLayerAnimationBase::SetAmbientColorCtl(plController* ambCtl)
{
    if( fAmbientColorCtl )
        delete fAmbientColorCtl;
    else
        fAmbientColor = new hsColorRGBA;

    fOwnedChannels |= kAmbientColor;
    fAmbientColorCtl = ambCtl;
}

void plLayerAnimationBase::SetSpecularColorCtl(plController* ambCtl)
{
    if( fSpecularColorCtl )
        delete fSpecularColorCtl;
    else
        fSpecularColor = new hsColorRGBA;

    fOwnedChannels |= kSpecularColor;
    fSpecularColorCtl = ambCtl;
}

void plLayerAnimationBase::SetOpacityCtl(plController* opaCtl)
{
    if( fOpacityCtl )
        delete fOpacityCtl;
    else
        fOpacity = new float;

    fOwnedChannels |= kOpacity;
    fOpacityCtl = opaCtl;
}

void plLayerAnimationBase::SetTransformCtl(plController* xfmCtl)
{
    if( fTransformCtl )
        delete fTransformCtl;
    else
        fTransform = new hsMatrix44;

    fOwnedChannels |= kTransform;
    fTransformCtl = xfmCtl;
}

float plLayerAnimationBase::IMakeUniformLength()
{
    fLength = 0;
    if( fPreshadeColorCtl && (fPreshadeColorCtl->GetLength() > fLength) )
        fLength = fPreshadeColorCtl->GetLength();
    if( fRuntimeColorCtl && (fRuntimeColorCtl->GetLength() > fLength) )
        fLength = fRuntimeColorCtl->GetLength();
    if( fAmbientColorCtl && (fAmbientColorCtl->GetLength() > fLength) )
        fLength = fAmbientColorCtl->GetLength();
    if( fSpecularColorCtl && (fSpecularColorCtl->GetLength() > fLength) )
        fLength = fSpecularColorCtl->GetLength();
    if( fOpacityCtl && (fOpacityCtl->GetLength() > fLength) )
        fLength = fOpacityCtl->GetLength();
    if( fTransformCtl && (fTransformCtl->GetLength() > fLength) )
        fLength = fTransformCtl->GetLength();

    return fLength;
}

/////////////////////////////////////////////////////////////////////////////////

plLayerAnimation::plLayerAnimation()
:
    plLayerAnimationBase(),
    fLayerSDLMod()
{
    fTimeConvert.SetOwner(this);
}

plLayerAnimation::~plLayerAnimation()
{
    delete fLayerSDLMod;
}

void plLayerAnimation::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimationBase::Read(s, mgr);

    fTimeConvert.Read(s, mgr);  
    if (!(fTimeConvert.IsStopped()))
    {   
        plSynchEnabler ps(true);    // enable dirty tracking so that we send state about
                                    // the anim resetting to start now.
        fTimeConvert.SetCurrentAnimTime(0, true);
    }
    Eval(hsTimer::GetSysSeconds(),0,0);

    // add sdl modifier
    delete fLayerSDLMod;
    fLayerSDLMod = new plLayerSDLModifier;
    fLayerSDLMod->SetLayerAnimation(this);
}


void plLayerAnimation::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimationBase::Write(s, mgr);

    fTimeConvert.Write(s, mgr);
}

plLayerInterface* plLayerAnimation::Attach(plLayerInterface* prev)
{
    fCurrentTime = fTimeConvert.CurrentAnimTime()-1.f;

    return plLayerAnimationBase::Attach(prev);
}

uint32_t plLayerAnimation::Eval(double wSecs, uint32_t frame, uint32_t ignore)
{
    uint32_t dirty = plLayerInterface::Eval(wSecs, frame, ignore);
    if( wSecs != fEvalTime )
    {
        uint32_t evalChans = 0;
        uint32_t passChans = dirty | fPassThruChannels;
        float secs = fTimeConvert.WorldToAnimTime(wSecs);
        if( secs != fCurrentTime )
        {
            evalChans = fOwnedChannels & ~ignore & ~fPassThruChannels;
            fCurrentTime = secs;
        }

        IEvalConvertedTime(secs, passChans, evalChans, dirty);
    }
    fEvalTime = wSecs;
    return dirty;
}

bool plLayerAnimation::MsgReceive(plMessage* msg)
{
    // pass sdl msg to sdlMod
    plSDLModifierMsg* sdlMsg = plSDLModifierMsg::ConvertNoRef(msg);
    if (sdlMsg && fLayerSDLMod)
    {
        if (fLayerSDLMod->MsgReceive(sdlMsg))
            return true;    // msg handled
    }

    bool retVal = false;
    plAnimCmdMsg* cmdMsg = plAnimCmdMsg::ConvertNoRef(msg);
    if( cmdMsg )
    {
        // Evaluate first, so we'll be transitioning from our
        // real current state, whether we've been evaluated (in view)
        // lately or not.
        TopOfStack()->Eval(hsTimer::GetSysSeconds(), 0, 0);
        retVal = fTimeConvert.HandleCmd(cmdMsg);
        DirtySynchState(kSDLLayer, 0);
    }

    if( retVal )
    {
        if( !fTimeConvert.IsStopped() || fTimeConvert.GetFlag(plAnimTimeConvert::kForcedMove) )
        {
            ClaimChannels(fOwnedChannels);
            fCurrentTime = -1.f; // force an eval
        }
    }
    else
    {
        retVal = plLayerAnimationBase::MsgReceive(msg);
    }
    
    return retVal;
}

void plLayerAnimation::DefaultAnimation()
{
    IMakeUniformLength();
    fTimeConvert.SetBegin(0);
    fTimeConvert.SetEnd(fLength);
    fTimeConvert.SetLoopPoints(0,fLength);
    fTimeConvert.Loop();
    fTimeConvert.Start();
}

///////////////////////////////////////////////////////////////////////////////////////

plLayerLinkAnimation::plLayerLinkAnimation() : 
    fLeavingAge(true),
    fEnabled(true), 
    fFadeFlags(),
    fLastFadeFlag(),
    fFadeFlagsDirty()
{ 
    fIFaceCallback = new plEventCallbackMsg();
    fIFaceCallback->fEvent = plEventCallbackMsg::kTime;
    fIFaceCallback->fRepeats = 0;           
}

plLayerLinkAnimation::~plLayerLinkAnimation() 
{ 
    hsRefCnt_SafeUnRef(fIFaceCallback);
}


void plLayerLinkAnimation::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimation::Read(s, mgr);
    
    fLinkKey = mgr->ReadKey(s);
    fLeavingAge = s->ReadBool();
    plgDispatch::Dispatch()->RegisterForExactType(plLinkEffectBCMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plLinkEffectPrepBCMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plCameraTargetFadeMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plAvatarStealthModeMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plIfaceFadeAvatarMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plPseudoLinkAnimTriggerMsg::Index(), GetKey());
    
    fIFaceCallback->AddReceiver(GetKey());  
}


void plLayerLinkAnimation::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimation::Write(s, mgr);
    
    mgr->WriteKey(s, fLinkKey);
    s->WriteBool(fLeavingAge);
}

uint32_t plLayerLinkAnimation::Eval(double wSecs, uint32_t frame, uint32_t ignore)
{
    uint32_t dirty = plLayerInterface::Eval(wSecs, frame, ignore);
    if (wSecs != fEvalTime)
    {
        uint32_t evalChans = 0;
        uint32_t passChans = dirty | fPassThruChannels;
        float oldAnimTime = fTimeConvert.CurrentAnimTime();
        float secs = oldAnimTime;
        
        if (fFadeFlagsDirty)
        {
            float goal = 0.f;

            if (fFadeFlags & kFadeLinkPrep)
                secs = goal = fLength;
            else
            {
                float rate = 0.f;
                float delta = (float)(wSecs - fEvalTime);
                
                if (fFadeFlags & kFadeLinking)
                {
                    goal = fLength;
                    rate = 1.f;
                }
                else if (fFadeFlags & kFadeCamera)
                {
                    goal = fLength;
                    rate = 10.f;
                }
                else if (fFadeFlags & (kFadeIFace | kFadeCCR))
                {
                    goal = fLength * 0.4f;
                    rate = 10.f;
                }
                else if (fFadeFlags == 0)
                {
                    goal = 0.f;
                    if (fLastFadeFlag == kFadeLinking)
                        rate = 1.f;
                    else
                        rate = 10.f;
                }

                if (fabs(oldAnimTime - goal) < delta * rate || rate == 0)
                    secs = goal;
                else if (goal > oldAnimTime)
                    secs = oldAnimTime + delta * rate;
                else
                    secs = oldAnimTime - delta * rate;
            }

            if (secs == goal)
                fFadeFlagsDirty = false;
        }
        
        if( secs != fCurrentTime )
        {
            fTimeConvert.SetCurrentAnimTime(secs);
            if (secs == 0.f || oldAnimTime == 0.f)
            {
                // Either we're going opaque, or we were opaque and now we're fading.
                // Tell the armature to re-eval its opacity settings.
                plAvatarOpacityCallbackMsg *opacityMsg = new plAvatarOpacityCallbackMsg(fLinkKey, plEventCallbackMsg::kStop);
                opacityMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
                opacityMsg->Send();
            }               
            evalChans = fOwnedChannels & ~ignore & ~fPassThruChannels;
            fCurrentTime = secs;
        }
        IEvalConvertedTime(secs, passChans, evalChans, dirty);
    }
    fEvalTime = wSecs;
    return dirty;
}

void plLayerLinkAnimation::SetFadeFlag(uint8_t flag, bool val)
{
    if (val)
        fFadeFlags |= flag;
    else
        fFadeFlags &= ~flag;

    if (fFadeFlags == 0)
        fLastFadeFlag = flag;
    
    TopOfStack()->Eval(hsTimer::GetSysSeconds(), 0, 0);
    ClaimChannels(fOwnedChannels);
    fCurrentTime = -1; // force eval
    fFadeFlagsDirty = true;
}

bool plLayerLinkAnimation::MsgReceive( plMessage* pMsg )
{
    plLinkEffectPrepBCMsg *bcpMsg = plLinkEffectPrepBCMsg::ConvertNoRef(pMsg);
    if (bcpMsg != nullptr)
    {
        if (bcpMsg->fLinkKey != fLinkKey || bcpMsg->fLeavingAge)
            return true;
    
        SetFadeFlag(kFadeLinkPrep, true);
        return true;
    }
        
        
    plLinkEffectBCMsg *msg = plLinkEffectBCMsg::ConvertNoRef(pMsg);
    if (msg != nullptr)
    {
        if (msg->fLinkKey == fLinkKey)
        {
            SetFadeFlag(kFadeLinkPrep, false);
            if (msg->HasLinkFlag(plLinkEffectBCMsg::kLeavingAge))
                SetFadeFlag(kFadeLinking, true);
            else
                SetFadeFlag(kFadeLinking, false);

            if (msg->HasLinkFlag(plLinkEffectBCMsg::kSendCallback))
            {
                plLinkEffectsMgr *mgr;
                if ((mgr = plLinkEffectsMgr::ConvertNoRef(msg->GetSender()->ObjectIsLoaded())))
                    mgr->WaitForEffect(msg->fLinkKey, fTimeConvert.GetEnd() - fTimeConvert.GetBegin());
            }           
        }
        return true;
    }

    plPseudoLinkAnimTriggerMsg* pSeudoMsg = plPseudoLinkAnimTriggerMsg::ConvertNoRef(pMsg);
    if (pSeudoMsg)
    {
        if (fLinkKey != pSeudoMsg->fAvatarKey)
            return true;

        if (pSeudoMsg->fForward)
            SetFadeFlag(kFadeLinking, true);
        else
            SetFadeFlag(kFadeLinking, false);
        
        // add a callback for when it's done if it's in forward
        plLinkEffectsMgr *mgr;
        if ((mgr = plLinkEffectsMgr::ConvertNoRef(pMsg->GetSender()->ObjectIsLoaded())))
            if (pSeudoMsg->fForward)
                mgr->WaitForPseudoEffect(fLinkKey, fTimeConvert.GetEnd() - fTimeConvert.GetBegin());

        return true;
    }
    // used to fade the player in or out when entering / exiting first person mode
    // or when distance between camera and player is too small...
    plCameraTargetFadeMsg* fMsg = plCameraTargetFadeMsg::ConvertNoRef(pMsg);
    if (fMsg)
    {
        if (fLinkKey != fMsg->GetSubjectKey())
            return true;

        if (fMsg->FadeOut())
            SetFadeFlag(kFadeCamera, true);
        else
            SetFadeFlag(kFadeCamera, false);    

        return true;
    }
    plIfaceFadeAvatarMsg* iMsg = plIfaceFadeAvatarMsg::ConvertNoRef(pMsg);
    if (iMsg)
    {
        if (fLinkKey != iMsg->GetSubjectKey())
            return true;

        if (iMsg->GetEnable())
        {
            Enable(true);
        }
        else if (iMsg->GetDisable())
        {
            Enable(false); // disable and un-fade
            SetFadeFlag(kFadeIFace, false);
        }
        else
        if (fEnabled)
        {
            if (iMsg->FadeOut())
                SetFadeFlag(kFadeIFace, true);
            else
                SetFadeFlag(kFadeIFace, false);
        }
        return true;
    }

    plAvatarStealthModeMsg *sMsg = plAvatarStealthModeMsg::ConvertNoRef(pMsg);
    if (sMsg)
    {
        if (sMsg->GetSender() == fLinkKey)
        {
            if (sMsg->fMode == plAvatarStealthModeMsg::kStealthCloakedButSeen)
            {
                SetFadeFlag(kFadeCCR, true);
            }
            else if (sMsg->fMode == plAvatarStealthModeMsg::kStealthVisible)
            {
                SetFadeFlag(kFadeCCR, false);
            }
            // Don't need to set opacity if we're fully cloaked, since we won't
            // even be drawing the spans (due to plEnableMsg() on the sceneObject)
        }
        return true;
    }

    return plLayerAnimation::MsgReceive( pMsg );
}

///////////////////////////////////////////////////////////////////////////////////////////////

plLayerSDLAnimation::plLayerSDLAnimation() : plLayerAnimationBase(), fVar() { }

uint32_t plLayerSDLAnimation::Eval(double wSecs, uint32_t frame, uint32_t ignore)
{
    uint32_t dirty = plLayerInterface::Eval(wSecs, frame, ignore);
    if( wSecs != fEvalTime )
    {
        uint32_t evalChans = 0;
        uint32_t passChans = dirty | fPassThruChannels;

        if (fEvalTime < 0)
        {
            if (!fVarName.empty())
            {
                extern const plSDLModifier *ExternFindAgeSDL();
                const plSDLModifier *sdlMod = ExternFindAgeSDL();
                if (sdlMod)
                {
                    fVar = sdlMod->GetStateCache()->FindVar(fVarName);
                    if (fVar)
                        sdlMod->AddNotifyForVar(GetKey(), fVarName, 0);
                }
            }
        }
        float secs;
        if (fVar)
            fVar->Get(&secs);
        else
            secs = 0.f;

        // We're guaranteed a 0-1 time. Scale that to our animation length.
        secs *= GetLength();

        if( secs != fCurrentTime )
        {
            evalChans = fOwnedChannels & ~ignore & ~fPassThruChannels;
            fCurrentTime = secs;
        }

        IEvalConvertedTime(secs, passChans, evalChans, dirty);
    }
    fEvalTime = wSecs;
    return dirty;
}

bool plLayerSDLAnimation::MsgReceive(plMessage* msg)
{
    plSDLNotificationMsg* nMsg = plSDLNotificationMsg::ConvertNoRef(msg);
    if (nMsg)
    {
        TopOfStack()->Eval(hsTimer::GetSysSeconds(), 0, 0);
        ClaimChannels(fOwnedChannels);
        return true;
    }

    return plLayerAnimationBase::MsgReceive(msg);
}

void plLayerSDLAnimation::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimationBase::Read(s, mgr);

    fVarName = s->ReadSafeString();
}

void plLayerSDLAnimation::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimationBase::Write(s, mgr);

    s->WriteSafeString(fVarName);
}
