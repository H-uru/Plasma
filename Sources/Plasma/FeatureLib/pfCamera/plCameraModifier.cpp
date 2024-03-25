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

#include "plCameraModifier.h"

#include "plgDispatch.h"
#include "plPhysical.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "plCameraBrain.h"
#include "plVirtualCamNeu.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plPhysicalControllerCore.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plInputEventMsg.h"

// new stuff

plCameraModifier1::plCameraModifier1()
    : fBrain(), fSubObj(), fFOVw(45.0f), fFOVh(33.75f),
      fAnimated(), fStartAnimOnPush(), fStopAnimOnPop(),
      fResetAnimOnPop(), fInSubLastUpdate(), fUpdateBrainTarget()
{
}


plCameraModifier1::~plCameraModifier1()
{
    for (CamTrans* trans : fTrans)
        delete trans;
    fTrans.clear();

    for (plMessage* message : fMessageQueue)
        hsRefCnt_SafeUnRef(message);
    fMessageQueue.clear();

    for (plCameraMsg* message : fFOVInstructions)
        hsRefCnt_SafeUnRef(message);
    fFOVInstructions.clear();
}


void plCameraModifier1::AddTarget(plSceneObject* so)
{
    fTarget = so;
    if( plVirtualCam1::Instance() )
        plVirtualCam1::Instance()->AddCameraLoaded(so);
    fFrom = (so->GetWorldToLocal().GetTranslate());
    if (GetBrain())
    {
        if (fTarget->GetCoordinateInterface())
            GetBrain()->AddTarget();
        else
            fUpdateBrainTarget = true; // update the brain later
    }
    if (GetKey())
    {   
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
    }
}

void plCameraModifier1::SetSubject(plSceneObject* pObj)
{ 
    if (GetBrain())
        GetBrain()->SetSubject(pObj); 
    else 
        fSubObj = pObj; 
}

plSceneObject* plCameraModifier1::GetSubject()
{
    if (GetBrain())
        return GetBrain()->GetSubject();
    else
        return fSubObj;
} 

void plCameraModifier1::SetFOV(float w, float h, bool fUpdateVCam) 
{ 
    fFOVw = w;
    fFOVh = h;
    if (plVirtualCam1::Instance() && fUpdateVCam)
        plVirtualCam1::SetFOV(this); 
}

void plCameraModifier1::SetFOVw(float f, bool fUpdateVCam) 
{ 
    fFOVw = f; 
    if (plVirtualCam1::Instance() && fUpdateVCam)
        plVirtualCam1::SetFOV(this); 
}

void plCameraModifier1::SetFOVh(float f, bool fUpdateVCam) 
{ 
    fFOVh = f; 
    if (plVirtualCam1::Instance() && fUpdateVCam)
        plVirtualCam1::SetFOV(this); 
}

bool plCameraModifier1::SetFaded(bool b)
{
    if (GetBrain())
        return GetBrain()->SetFaded(b);
    return false;
}

bool plCameraModifier1::GetFaded()
{
    if (GetBrain())
        return GetBrain()->GetFaded();
    return false;
}
bool plCameraModifier1::MsgReceive(plMessage* msg)
{
    if (GetBrain())
        GetBrain()->MsgReceive(msg);
        
    plCameraMsg* pCamMsg = plCameraMsg::ConvertNoRef(msg);
    if (pCamMsg)
    {
        if (pCamMsg->Cmd(plCameraMsg::kAddFOVKeyframe))
        {
            hsRefCnt_SafeRef(msg);
            fFOVInstructions.emplace_back(pCamMsg);
            return true;
        }
        else
        if (pCamMsg->Cmd(plCameraMsg::kSetAnimated))
        {
            fAnimated = true;
            return true;
        }
    }
    plEventCallbackMsg* pEventMsg = plEventCallbackMsg::ConvertNoRef(msg);
    if (pEventMsg)
    {
        double time = (double)fFOVInstructions[pEventMsg->fIndex]->GetConfig()->fAccel;
        double time2 = (double)pEventMsg->fEventTime;
        time = fabs(time - time2);
        float h = fFOVInstructions[pEventMsg->fIndex]->GetConfig()->fFOVh;
        float w = fFOVInstructions[pEventMsg->fIndex]->GetConfig()->fFOVw;
        if (GetBrain())
            GetBrain()->SetFOVGoal(w, h, time);
    }

    plAnimCmdMsg* pAnimMsg = plAnimCmdMsg::ConvertNoRef(msg);
    if (pAnimMsg)
    {
        hsRefCnt_SafeRef(msg);
        msg->ClearReceivers();
        msg->AddReceiver(msg->GetSender());
        fMessageQueue.emplace_back(msg);
        return true;
    }
    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg )
    {
        if( pRefMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest) )
        {
            if (pRefMsg->fType == kRefBrain)
            {
                plCameraBrain1* pBrain = plCameraBrain1::ConvertNoRef(pRefMsg->GetRef());
                if (pBrain)
                {
                    pBrain->SetCamera(this);
                    fBrain = pBrain;
                    if (fSubObj)
                        fBrain->SetSubject(fSubObj);
                }
            }
            else
            if (pRefMsg->fType == kRefCallbackMsg && fMessageQueue[pRefMsg->fWhich] != nullptr)
            {
                
                plgDispatch::MsgSend(fMessageQueue[pRefMsg->fWhich]);
                fMessageQueue[pRefMsg->fWhich] = nullptr;
            }
        }
        else if( pRefMsg->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove) )
        {
            plCameraBrain1* pBrain = (plCameraBrain1*)(pRefMsg->GetRef());
            if (fBrain == pBrain)
                fBrain = nullptr;
        }
        return true;
     }
    return plSingleModifier::MsgReceive(msg);
}


void plCameraModifier1::Update()
{
    // update the brain

    // this freeze thing is a useful debugging tool...  
    if (plVirtualCam1::Instance()->freeze)
        return;
    
    if (GetBrain())
    {
        if (fUpdateBrainTarget && fTarget->GetCoordinateInterface()) // if we need to update the brain and the target is loaded
        {
            fUpdateBrainTarget = false;
            GetBrain()->AddTarget(); // update the brain's target
        }

        bool moveInSub = !(GetBrain()->HasFlag(plCameraBrain1::kIgnoreSubworldMovement));

        if (moveInSub && GetBrain()->GetSubject())
        {
            plKey worldKey;

            // First check if this is a physical.  If so, grab the subworld from that
            if (GetBrain()->GetSubject()->GetSimulationInterface())
            {
                plPhysical* phys = GetBrain()->GetSubject()->GetSimulationInterface()->GetPhysical();
                if (phys)
                    worldKey = phys->GetWorldKey();
            }
            // Also, check if this is an avatar.  They don't have physicals, you
            // have to ask the avatar controller for the subworld key.
            if (!worldKey)
            {
                plKey subject = plKey(GetBrain()->GetSubject()->GetKey());
                plArmatureMod* armMod = plAvatarMgr::FindAvatar(subject);
                if (armMod && armMod->GetController() )
                    worldKey = armMod->GetController()->GetSubworld();
            }

            if (worldKey)
            {
                // this picks up and moves the camera to it's previous subworld coordinate (so the subworld isn't moving out from underneath us)
                hsMatrix44 l2w, w2l;
                plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
                if (so)
                {
                    l2w = so->GetLocalToWorld();
                    w2l = so->GetWorldToLocal();

                    if (fInSubLastUpdate)
                    {
                        if (!(fLastSubPos == fFrom && fLastSubPOA == fAt))
                        {
                            SetTargetPos(l2w * fLastSubPos);
                            SetTargetPOA(l2w * fLastSubPOA);
                        }
                    }
                    else
                    {
                        fInSubLastUpdate = true;
                    }
                    GetBrain()->Update();
                    fLastSubPos = w2l * GetTargetPos();
                    fLastSubPOA = w2l * GetTargetPOA();
                }
                return;
            }
            else
            {
                fInSubLastUpdate = false;
            }
        }
        GetBrain()->Update();
        fLastSubPos = GetTargetPos();
        fLastSubPOA = GetTargetPOA();
    }
}   

void plCameraModifier1::Read(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Read(stream, mgr);
    fBrain = nullptr;
    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefBrain), plRefFlags::kActiveRef);
    uint32_t count = stream->ReadLE32();
    for (uint32_t i = 0; i < count; i++)
    {
        
        plKey key = mgr->ReadKey(stream);
        bool cutpos = stream->ReadBool();
        bool cutpoa = stream->ReadBool();
        bool ignore = stream->ReadBool();
        float v = stream->ReadLEFloat();
        float a = stream->ReadLEFloat();
        float d = stream->ReadLEFloat();
        float pV = stream->ReadLEFloat();
        float pA = stream->ReadLEFloat();
        float pD = stream->ReadLEFloat();

        CamTrans* camTrans = new CamTrans(key);
        camTrans->fAccel = a;
        camTrans->fDecel = d;
        camTrans->fVelocity = v;
        camTrans->fPOAAccel = pA;
        camTrans->fPOADecel = pD;
        camTrans->fPOAVelocity = pV;
        camTrans->fCutPos = cutpos;
        camTrans->fCutPOA = cutpoa;
        camTrans->fIgnore = ignore;

        fTrans.emplace_back(camTrans);
    }
    fFOVw = stream->ReadLEFloat();
    fFOVh = stream->ReadLEFloat();
    uint32_t n = stream->ReadLE32();
    fMessageQueue.resize(n);
    for (uint32_t i = 0; i < n; i++)
    {   
        plMessage* pMsg =  plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
        fMessageQueue[i] = pMsg;
    }
    for (uint32_t i = 0; i < n; i++)
    {   
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kRefCallbackMsg), plRefFlags::kActiveRef);
    }

    n = stream->ReadLE32();
    fFOVInstructions.resize(n);
    for (uint32_t i = 0; i < n; i++)
    {
        plCameraMsg* pMsg =  plCameraMsg::ConvertNoRef(mgr->ReadCreatable(stream));
        fFOVInstructions[i] = pMsg;
    }
    fAnimated = stream->ReadBool();
    fStartAnimOnPush = stream->ReadBool();
    fStopAnimOnPop = stream->ReadBool();
    fResetAnimOnPop = stream->ReadBool();
}

void plCameraModifier1::Write(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Write(stream, mgr);
    if (fBrain)
        mgr->WriteKey(stream, fBrain );
    
    stream->WriteLE32((uint32_t)fTrans.size());
    for (CamTrans* trans : fTrans)
    {   
        mgr->WriteKey(stream, trans->fTransTo);
        stream->WriteBool(trans->fCutPos);
        stream->WriteBool(trans->fCutPOA);
        stream->WriteBool(trans->fIgnore);
        stream->WriteLEFloat(trans->fVelocity);
        stream->WriteLEFloat(trans->fAccel);
        stream->WriteLEFloat(trans->fDecel);
        stream->WriteLEFloat(trans->fPOAVelocity);
        stream->WriteLEFloat(trans->fPOAAccel);
        stream->WriteLEFloat(trans->fPOADecel);
    }
    stream->WriteLEFloat(fFOVw);
    stream->WriteLEFloat(fFOVh);
    stream->WriteLE32((uint32_t)fMessageQueue.size());
    for (plMessage* message : fMessageQueue)
    {
        mgr->WriteCreatable(stream, message);
    }
    for (plMessage* message : fMessageQueue)
    {
        mgr->WriteKey(stream, message->GetSender());
    }
    stream->WriteLE32((uint32_t)fFOVInstructions.size());
    for (plCameraMsg* message : fFOVInstructions)
    {
        mgr->WriteCreatable(stream, message);
    }
    stream->WriteBool(fAnimated);
    stream->WriteBool(fStartAnimOnPush);
    stream->WriteBool(fStopAnimOnPop);
    stream->WriteBool(fResetAnimOnPop);
}

void plCameraModifier1::Push(bool recenter)
{
    if (fAnimated)
    {
        if (fStartAnimOnPush)
        {
            plAnimCmdMsg* pMsg = new plAnimCmdMsg;
            pMsg->SetCmd(plAnimCmdMsg::kRunForward);
            pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
            pMsg->AddReceiver(GetTarget()->GetKey());
            if (GetBrain() && GetBrain()->GetSubject())
                pMsg->AddReceiver(GetBrain()->GetSubject()->GetKey());
            pMsg->Send();
        }
    }
    if (fBrain)
        fBrain->Push(recenter);


    if (GetKey())
    {   
        plgDispatch::Dispatch()->RegisterForExactType(plMouseEventMsg::Index(), GetKey());
    }
}

void plCameraModifier1::Pop()
{
    if (fAnimated)
    {
        if (fStopAnimOnPop)
        {
            plAnimCmdMsg* pMsg = new plAnimCmdMsg;
            pMsg->SetCmd(plAnimCmdMsg::kStop);
            pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
            pMsg->AddReceiver(GetTarget()->GetKey());
            if (GetBrain() && GetBrain()->GetSubject())
                pMsg->AddReceiver(GetBrain()->GetSubject()->GetKey());
            pMsg->Send();       
        }
        if (fResetAnimOnPop)
        {
            plAnimCmdMsg* pMsg = new plAnimCmdMsg;
            pMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
            pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
            pMsg->AddReceiver(GetTarget()->GetKey());
            if (GetBrain() && GetBrain()->GetSubject())
                pMsg->AddReceiver(GetBrain()->GetSubject()->GetKey());
            pMsg->Send();
        }
    }
    if (fBrain)
        fBrain->Pop();
    if (GetKey()) // the reason we might not have a key is a special run-time POA which doesn't need to receive messages...
    {       
        plgDispatch::Dispatch()->UnRegisterForExactType(plMouseEventMsg::Index(), GetKey());
    }
}   

void plCameraModifier1::SetTransform(hsPoint3 at)
{
    if (!GetTarget())
        return;
    hsMatrix44 l2w;
    hsMatrix44 w2l;
    hsVector3 up(0.f, 0.f, 1.f);
    l2w.Make(&fFrom, &at, &up);
    l2w.GetInverse(&w2l);
    IGetTargetCoordinateInterface(0)->SetTransform( l2w, w2l );
}
