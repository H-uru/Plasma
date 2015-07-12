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

#include "plCameraModifier.h"
#include "plCameraBrain.h"
#include "plVirtualCamNeu.h"
#include "hsTimer.h"
#include "plgDispatch.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "hsResMgr.h"
#include "pnMessage/plCameraMsg.h"
#include "plPhysical/plSimDefs.h"

#include "plPhysical.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plPhysicalControllerCore.h"

#include <cmath>

// new stuff

plCameraModifier1::plCameraModifier1() :
fBrain(nil),
fSubObj(nil),
fFOVw(45.0f),
fFOVh(33.75f),
fAnimated(false),
fStartAnimOnPush(false),
fStopAnimOnPop(false),
fResetAnimOnPop(false),
fInSubLastUpdate(false),
fUpdateBrainTarget(false)
{
    fFrom.Set(0,0,0);
    fAt.Set(0,1,0);
}


plCameraModifier1::~plCameraModifier1()
{
    int i;
    for (i = 0; i < GetNumTrans(); i++)
        delete(GetTrans(i));
    fTrans.SetCountAndZero(0);
    
    for (i = 0; i < fMessageQueue.Count(); i++)
        hsRefCnt_SafeUnRef(fMessageQueue[i]);
    fMessageQueue.SetCountAndZero(0);

    for (i = 0; i < fFOVInstructions.Count(); i++)
        hsRefCnt_SafeUnRef(fFOVInstructions[i]);
    fFOVInstructions.SetCountAndZero(0);

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
            fFOVInstructions.Append(pCamMsg);
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
        fMessageQueue.Append(msg);
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
            if (pRefMsg->fType == kRefCallbackMsg && fMessageQueue[pRefMsg->fWhich] != nil)
            {
                
                plgDispatch::MsgSend(fMessageQueue[pRefMsg->fWhich]);
                fMessageQueue[pRefMsg->fWhich] = nil;
            }
        }
        else if( pRefMsg->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove) )
        {
            plCameraBrain1* pBrain = (plCameraBrain1*)(pRefMsg->GetRef());
            if (fBrain == pBrain)
                fBrain = nil;
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
            plKey worldKey = nil;

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
    fBrain = nil;
    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefBrain), plRefFlags::kActiveRef);
    int count = stream->ReadLE32();
    int i;
    for (i = 0; i < count; i++)
    {
        
        plKey key = mgr->ReadKey(stream);
        bool cutpos = stream->ReadBool();
        bool cutpoa = stream->ReadBool();
        bool ignore = stream->ReadBool();
        float v = stream->ReadLEScalar();
        float a = stream->ReadLEScalar();
        float d = stream->ReadLEScalar();
        float pV = stream->ReadLEScalar();
        float pA = stream->ReadLEScalar();
        float pD = stream->ReadLEScalar();

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

        fTrans.Append(camTrans);
    }
    fFOVw = stream->ReadLEFloat();
    fFOVh = stream->ReadLEFloat();
    int n = stream->ReadLE32();
    fMessageQueue.SetCountAndZero(n);
    for(i = 0; i < n; i++ )
    {   
        plMessage* pMsg =  plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
        fMessageQueue[i] = pMsg;
    }
    for(i = 0; i < n; i++ )
    {   
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kRefCallbackMsg), plRefFlags::kActiveRef);
    }

    n = stream->ReadLE32();
    fFOVInstructions.SetCountAndZero(n);
    for(i = 0; i < n; i++ )
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
    
    int i = fTrans.Count();
    stream->WriteLE32(i);
    for (i = 0; i < fTrans.Count(); i++)
    {   
        mgr->WriteKey(stream, fTrans[i]->fTransTo);
        stream->WriteBool(fTrans[i]->fCutPos);
        stream->WriteBool(fTrans[i]->fCutPOA);
        stream->WriteBool(fTrans[i]->fIgnore);
        stream->WriteLEScalar(fTrans[i]->fVelocity);
        stream->WriteLEScalar(fTrans[i]->fAccel);
        stream->WriteLEScalar(fTrans[i]->fDecel);
        stream->WriteLEScalar(fTrans[i]->fPOAVelocity);
        stream->WriteLEScalar(fTrans[i]->fPOAAccel);
        stream->WriteLEScalar(fTrans[i]->fPOADecel);
    }
    stream->WriteLEFloat(fFOVw);
    stream->WriteLEFloat(fFOVh);
    stream->WriteLE32(fMessageQueue.Count());
    for (i = 0; i < fMessageQueue.Count(); i++)
    {
        mgr->WriteCreatable(stream, fMessageQueue[i]);
    }
    for (i = 0; i < fMessageQueue.Count(); i++)
    {
        mgr->WriteKey(stream, fMessageQueue[i]->GetSender());
    }
    stream->WriteLE32(fFOVInstructions.Count());
    for (i = 0; i < fFOVInstructions.Count(); i++)
    {
        mgr->WriteCreatable(stream, fFOVInstructions[i]);
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
    hsVector3 up(0,0,1);
    l2w.Make(&fFrom, &at, &up);
    l2w.GetInverse(&w2l);
    IGetTargetCoordinateInterface(0)->SetTransform( l2w, w2l );
}
