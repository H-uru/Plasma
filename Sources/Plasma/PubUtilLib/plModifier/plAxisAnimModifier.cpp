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

#include "plAxisAnimModifier.h"

#include "hsResMgr.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "plInputCore/plInputInterface.h"
#include "pnNetCommon/plNetApp.h"
#include "plgDispatch.h"

//// plAxisInputInterface ////////////////////////////////////////////////////
//  Small input interface to handle mouse imput while dragging. Supercedes
//  the sceneInteraction Interface

class plAxisInputInterface : public plInputInterface
{
    protected:

        plAxisAnimModifier  *fOwner;

        virtual ControlEventCode    *IGetOwnedCodeList() const
        {
            static ControlEventCode codes[] = { END_CONTROLS };
            return codes;
        }

    public:

        plAxisInputInterface( plAxisAnimModifier *owner ) { fOwner = owner; SetEnabled( true ); }

        uint32_t  GetPriorityLevel() const override { return kSceneInteractionPriority + 10; }
        bool    InterpretInputEvent(plInputEventMsg *pMsg) override
        {
            plMouseEventMsg* pMMsg = plMouseEventMsg::ConvertNoRef( pMsg );
            if (pMMsg )
            {
                if( pMMsg->GetButton() == kLeftButtonUp )
                {
                    // Remove ourselves from the stack
                    plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
                    msg->SetIFace( this );
                    plgDispatch::MsgSend( msg );
                    return true;
                }
                return fOwner->MsgReceive( pMMsg );
            }

            return false;
        }

        uint32_t  GetCurrentCursorID() const override { return kCursorGrab; }
        bool    HasInterestingCursorID() const override { return true; }
};

plAxisAnimModifier::plAxisAnimModifier() : 
fActive(),
fXPos(),
fYPos(),
fIface(),
fAllOrNothing()
{
    fNotify = new plNotifyMsg;
    fInputIface = new plAxisInputInterface( this );
}
plAxisAnimModifier::~plAxisAnimModifier()
{
    hsRefCnt_SafeUnRef(fNotify);
    hsRefCnt_SafeUnRef( fInputIface );
}


bool plAxisAnimModifier::IEval(double secs, float del, uint32_t dirty)
{
    if (!fActive)
        return true;

    return true;
}

void plAxisAnimModifier::SetTarget(plSceneObject* so)
{
    plSingleModifier::SetTarget(so);

    if( so )
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

bool plAxisAnimModifier::MsgReceive(plMessage* msg)
{
    plEventCallbackMsg* pCall = plEventCallbackMsg::ConvertNoRef(msg);
    if (pCall)
    {
        // Send our notification to whomever cares;
        float time = 0.0f;
        if (pCall->fEvent == kEnd)
            time = 1.0f;
        fNotify->ClearEvents();
        fNotify->SetSender(fNotificationKey); // so python can handle it.
        fNotify->AddCallbackEvent( pCall->fEvent );
        fNotify->SetState(1.0f);
        fNotify->AddActivateEvent(true);
        fNotify->AddClickDragEvent(GetTarget()->GetKey(), plNetClientApp::GetInstance()->GetLocalPlayerKey(), time);
        hsRefCnt_SafeRef(fNotify);
        plgDispatch::MsgSend( fNotify );
        return true;
    }
        
    plNotifyMsg* pNMsg = plNotifyMsg::ConvertNoRef(msg);
    if (pNMsg)
    {
        for (size_t i = 0; i < pNMsg->GetEventCount(); i++)
        {
            if (pNMsg->GetEventRecord(i)->fEventType == proEventData::kActivate)
            {
                if( ( (proActivateEventData *)pNMsg->GetEventRecord(i) )->fActivate )
                {
                    fActive = true;
                    fXPos = plMouseDevice::Instance()->GetCursorX();
                    fYPos = plMouseDevice::Instance()->GetCursorY();

                    // Insert our input interface onto the input stack
                    plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
                    msg->SetIFace( fInputIface );
                    plgDispatch::MsgSend( msg );
                }
                else
                {
/*                  if (fActive)
                    {
                        fActive = false;

                        // Remove our input interface from the input stack
                        plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
                        msg->SetIFace( fInputIface );
                        plgDispatch::MsgSend( msg );
                    }
*/              }
                break;
            }
        }
        return true;
    }
    
    plMouseEventMsg* pMMsg = plMouseEventMsg::ConvertNoRef(msg);
    if (pMMsg)
    {
        if (fXAnim && pMMsg->GetDX() != 0.0f)
        {
            if (pMMsg->GetDX() > 0.05f)
                return true;
            plAnimCmdMsg* pMsg = new plAnimCmdMsg;
            pMsg->AddReceiver(fXAnim);
            pMsg->SetAnimName(fAnimLabel);
        //  pMsg->SetAnimName()
            if (fXPos < pMMsg->GetXPos())
            {
                pMsg->SetCmd(plAnimCmdMsg::kIncrementForward);
            }
            else
            {
                pMsg->SetCmd(plAnimCmdMsg::kIncrementBackward);
            }
            plgDispatch::MsgSend(pMsg);
            fXPos = pMMsg->GetXPos();
        }
        if (fYAnim && pMMsg->GetDY() != 0.0f)
        {
        
            if (pMMsg->GetDY() > 0.05f)
                return true;
            plAnimCmdMsg* pMsg = new plAnimCmdMsg;
            pMsg->AddReceiver(fYAnim);
            pMsg->SetAnimName(fAnimLabel);
            if (fYPos > pMMsg->GetYPos())
            {
                if (fAllOrNothing)
                {
                    pMsg->SetCmd(plAnimCmdMsg::kContinue);
                    pMsg->SetCmd(plAnimCmdMsg::kSetForewards);
                    fActive = false;

                    // Remove our input interface from the input stack
                    plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
                    msg->SetIFace( fInputIface );
                    plgDispatch::MsgSend( msg );
                    plInputManager::SetRecenterMouse(false);
                }
                else
                {
                    pMsg->SetCmd(plAnimCmdMsg::kIncrementForward);
                }
            }
            else
            {
                if (fAllOrNothing)
                {
                    pMsg->SetCmd(plAnimCmdMsg::kContinue);
                    pMsg->SetCmd(plAnimCmdMsg::kSetBackwards);
                    fActive = false;

                    // Remove our input interface from the input stack
                    plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
                    msg->SetIFace( fInputIface );
                    plgDispatch::MsgSend( msg );

                    plInputManager::SetRecenterMouse(false);
                }
                else
                {
                    pMsg->SetCmd(plAnimCmdMsg::kIncrementBackward);
                }
            }
            plgDispatch::MsgSend(pMsg);
            fYPos = pMMsg->GetYPos();
        }
        
        return true;
    }

    plCmdIfaceModMsg* pCMsg = plCmdIfaceModMsg::ConvertNoRef(msg);
    if (pCMsg && pCMsg->Cmd(plCmdIfaceModMsg::kIndexCallback))
    {
        fIface = pCMsg->fIndex;
        return true;
    }

    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->GetContext() == plRefMsg::kOnCreate )
        {
            if (pRefMsg->fType == kTypeX)
            {   
                fXAnim = pRefMsg->GetRef()->GetKey();

                // add callbacks for beginning and end of animation
                plEventCallbackMsg* pCall1 = new plEventCallbackMsg;
                pCall1->fEvent = kBegin;
                pCall1->fRepeats = -1;
                pCall1->AddReceiver(GetKey());
                
                plEventCallbackMsg* pCall2 = new plEventCallbackMsg;
                pCall2->fEvent = kEnd;
                pCall2->fRepeats = -1;
                pCall2->AddReceiver(GetKey());

                plAnimCmdMsg* pMsg = new plAnimCmdMsg;
                pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
                pMsg->AddCallback(pCall1);
                pMsg->AddCallback(pCall2);
                pMsg->SetAnimName(fAnimLabel);
                pMsg->AddReceiver( fXAnim );

                hsRefCnt_SafeUnRef( pCall1 );
                hsRefCnt_SafeUnRef( pCall2 );

                plgDispatch::MsgSend(pMsg);
            }
            else 
            if (pRefMsg->fType == kTypeY)
            {
                fYAnim = pRefMsg->GetRef()->GetKey();
                
                // add callbacks for beginning and end of animation
                plEventCallbackMsg* pCall1 = new plEventCallbackMsg;
                pCall1->fEvent = kBegin;
                pCall1->fRepeats = -1;
                pCall1->AddReceiver(GetKey());
                
                plEventCallbackMsg* pCall2 = new plEventCallbackMsg;
                pCall2->fEvent = kEnd;
                pCall2->fRepeats = -1;
                pCall2->AddReceiver(GetKey());

                plAnimCmdMsg* pMsg = new plAnimCmdMsg;
                pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
                pMsg->AddCallback(pCall1);
                pMsg->AddCallback(pCall2);
                pMsg->AddReceiver( fYAnim );
                pMsg->SetAnimName(fAnimLabel);

                hsRefCnt_SafeUnRef( pCall1 );
                hsRefCnt_SafeUnRef( pCall2 );

                plgDispatch::MsgSend(pMsg);
            }
            else 
            if (pRefMsg->fType == kTypeLogic)
            {
                SetNotificationKey(pRefMsg->GetRef()->GetKey());
            }
        }
        else
        if (pRefMsg->GetContext() == plRefMsg::kOnDestroy )
        {
            if (pRefMsg->fType == kTypeX)
                fXAnim = nullptr;
            else 
            if (pRefMsg->fType == kTypeY)
                fYAnim = nullptr;
            else 
            if (pRefMsg->fType == kTypeLogic)
                fNotificationKey = nullptr;
        }
        return true;    
    }
    
    return plSingleModifier::MsgReceive(msg);
}

void plAxisAnimModifier::Read(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Read(s, mgr);

    mgr->ReadKeyNotifyMe( s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeX), plRefFlags::kPassiveRef);
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeY), plRefFlags::kPassiveRef);
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeLogic), plRefFlags::kPassiveRef);
    
    fAllOrNothing = s->ReadBool();
    plNotifyMsg* pMsg = plNotifyMsg::ConvertNoRef(mgr->ReadCreatable(s));
    if (fNotify)
        delete(fNotify);
    fNotify = pMsg;
    plMsgStdStringHelper::Peek(fAnimLabel, s);
}

void plAxisAnimModifier::Write(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Write(s, mgr);
    mgr->WriteKey(s, fXAnim);
    mgr->WriteKey(s, fYAnim);
    mgr->WriteKey(s, fNotificationKey);
    s->WriteBool(fAllOrNothing);
    mgr->WriteCreatable(s, fNotify);
    plMsgStdStringHelper::Poke(fAnimLabel, s);
}
