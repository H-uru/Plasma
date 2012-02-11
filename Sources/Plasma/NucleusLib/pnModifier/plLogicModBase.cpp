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
#include "plLogicModBase.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnNetCommon/plGenericVar.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plNetSharedState.h"
#include "plNetMessage/plNetMessage.h"  // breaks project dependancy levels
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plServerReplyMsg.h"

void plLogicModBase::ConsoleTrigger(plKey playerKey)
{
    // Setup the event data in case this is a OneShot responder that needs it
    proPickedEventData *ed = new proPickedEventData;
    ed->fPicker = playerKey;
    ed->fPicked = nil;
    fNotify->AddEvent(ed);

    Trigger(false);

    // Whoops, trigger and untrigger use the same message, so if we do this right away
    // it will just untrigger twice.  So...uhh, we don't untrigger!
//  UnTrigger();
}

void plLogicModBase::ConsoleRequestTrigger()
{
    RequestTrigger();
}

plLogicModBase::plLogicModBase() :
fCounter(0),
fCounterLimit(0),
fTimer(0.0f),
fNotify(nil),
fDisabled(false)
{
    fNotify = new plNotifyMsg;
}

plLogicModBase::~plLogicModBase()
{
    int i;
    for (i = 0; i < fCommandList.Count(); i++ )
    {
        hsRefCnt_SafeUnRef(fCommandList[i]);
    }

    hsRefCnt_SafeUnRef(fNotify);
}

void plLogicModBase::AddTarget(plSceneObject* so)
{
    plSingleModifier::AddTarget(so);
}

void plLogicModBase::RegisterForMessageType(uint16_t hClass)
{
    plgDispatch::Dispatch()->RegisterForExactType( hClass, GetKey() ); 
}

//
// Update generic shared state (which reflects trigger state) on server 
// by sending TestAndSet request.  By locking and unlocking the sharedState,
// we can guarantee that only one logicMod instance can trigger at a time.
// The server will confirm or deny our request to lock and set the state.
//
void plLogicModBase::IUpdateSharedState(bool triggered) const
{
    plNetSharedState ss("TrigState");
    plGenericVar* sv = new plGenericVar("Triggered");
    sv->Value().SetBool(triggered); // attempting to set trig state to true
    ss.AddVar(sv);
    
    bool lock = triggered;

    // if unlocking, then the server does not need to store this state, since it's back to its default state
    ss.SetServerMayDelete(!lock);       

    plNetMsgTestAndSet ts;
    ts.SetNetProtocol(kNetProtocolCli2Game);
    ts.CopySharedState(&ss);
    ts.ObjectInfo()->SetFromKey(GetKey());
    ts.SetLockRequest(lock);        // if triggering, lock state, else unlock state
    plNetClientApp::GetInstance()->SendMsg(&ts);
    plNetClientApp::GetInstance()->DebugMsg("\tLM: Attempting to set logic mod shared lock to %s, t=%f\n", 
        triggered ? "Triggered" : "UnTriggered", hsTimer::GetSysSeconds());
}

hsBool plLogicModBase::MsgReceive(plMessage* msg)
{
    // read messages:
    plServerReplyMsg* pSMsg = plServerReplyMsg::ConvertNoRef(msg);
    if (pSMsg)
    {
        hsAssert(pSMsg->GetType() != plServerReplyMsg::kUnInit, "uninit server reply msg");

#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: LogicModifier %s recvd trigger request reply:%s, wasRequesting=%d, t=%f\n",
            GetKeyName().c_str(),
            pSMsg->GetType() == plServerReplyMsg::kDeny ? "denied" : "confirmed", 
            HasFlag(kRequestingTrigger), hsTimer::GetSysSeconds());
#endif

        if (pSMsg->GetType() == plServerReplyMsg::kDeny)
        {
            if (HasFlag(kRequestingTrigger))
            {
                plNetClientApp::GetInstance()->DebugMsg("\tLM: Denied, clearing requestingTrigger");
                ClearFlag(kRequestingTrigger);
            }
            else
                plNetClientApp::GetInstance()->DebugMsg("\tLM: Denied, but not requesting?");
        }
        else
        {
            hsBool netRequest=false;    // we're triggering as a result of a local activation
            PreTrigger(netRequest);
            IUpdateSharedState(false /* untriggering */);
        }
        return true;
    }

    plEnableMsg* pEnable = plEnableMsg::ConvertNoRef(msg);
    if (pEnable)
    {
        if (pEnable->Cmd(plEnableMsg::kDisable))
            fDisabled = true;
        else
        if (pEnable->Cmd(plEnableMsg::kEnable))
        {
            ClearFlag(kTriggered);
            ClearFlag(kRequestingTrigger);

            fDisabled = false;
        }
        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}

void plLogicModBase::RequestTrigger(hsBool netRequest)
{
    if (HasFlag(kTriggered))
    {
#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: %s ignoring RequestTrigger(), already triggered, t=%f\n",
            GetKeyName().c_str(), hsTimer::GetSysSeconds());
#endif
        return;
    }

    if (HasFlag(kRequestingTrigger))
    {
#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: %s ignoring RequestTrigger(), already requesting trigger, t=%f\n",
            GetKeyName().c_str(), hsTimer::GetSysSeconds());
#endif

        return;
    }
    if ( plNetApp::GetInstance()->GetFlagsBit(plNetClientApp::kLocalTriggers))
    {
        PreTrigger(false);
    }
    else
    {
        IUpdateSharedState(true /* triggering */);  // request arbitration from server
        SetFlag(kRequestingTrigger);

#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: %s Setting RequestingTriggert=%f\n",
            GetKeyName().c_str(), hsTimer::GetSysSeconds());
#endif

    }
}

//
// return false is counter test fails
//
hsBool plLogicModBase::IEvalCounter()
{
    if (fCounterLimit > 0)
    {   
        fCounter = fCounter + 1;
        if (fCounter != fCounterLimit)
        {
            Reset(false);
            return false;
        }   
    }
    return true;
}

void plLogicModBase::PreTrigger(hsBool netRequest)
{
    if (fDisabled)
        return;

    Trigger(netRequest);
}

void plLogicModBase::Trigger(hsBool netRequest)
{
#if 1
    plNetClientApp::GetInstance()->DebugMsg("LogicModifier %s is triggering, activatorType=%d\n",
        GetKeyName().c_str(), HasFlag(kTypeActivator));
#endif

    ClearFlag(kRequestingTrigger);
    if (!HasFlag(kMultiTrigger))
        SetFlag(kTriggered);
    fNotify->SetSender(this->GetKey());
    fNotify->SetState(1.0f);
    fNotify->AddActivateEvent(true);
//  hsRefCnt_SafeRef(fNotify);
    plgDispatch::MsgSend( fNotify );
    CreateNotifyMsg();
    if (HasFlag(kOneShot))
        fDisabled = true;
}

void plLogicModBase::UnTrigger()
{
    if (!HasFlag(kTriggered))
        return;

#ifdef HS_DEBUGGING
    char str[256];
    sprintf(str, "LogicModifier %s is Un-triggering, activatorType=%d\n", 
        GetKeyName(), HasFlag(kTypeActivator));
    plNetClientApp::GetInstance()->DebugMsg(str);
#endif
    fNotify->SetSender(this->GetKey());
    fNotify->SetState(0.0f);
    fNotify->AddActivateEvent(false);
//  hsRefCnt_SafeRef(fNotify);
    plgDispatch::MsgSend( fNotify );
    CreateNotifyMsg();
    Reset(true);
}

void plLogicModBase::Reset(bool bCounterReset)
{
    ClearFlag(kTriggered);
    if (bCounterReset)
        fCounter = 0;
}

void plLogicModBase::CreateNotifyMsg()
{   
    fNotify = new plNotifyMsg;
    for (int i = 0; i < fReceiverList.Count(); i++)
        fNotify->AddReceiver(fReceiverList[i]);
}

void plLogicModBase::AddNotifyReceiver(plKey receiver)
{
    fReceiverList.Append(receiver);
    fNotify->AddReceiver(receiver);
}

void plLogicModBase::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);
    int n = stream->ReadLE32();
    fCommandList.SetCountAndZero(n);
    for(int i = 0; i < n; i++ )
    {   
        plMessage* pMsg =  plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
        fCommandList[i] = pMsg;
    }
    if (fNotify)
        delete fNotify;
    plNotifyMsg* pNMsg =  plNotifyMsg::ConvertNoRef(mgr->ReadCreatable(stream));
    fNotify = pNMsg;

    fFlags.Read(stream);
    fDisabled = stream->Readbool();
    for (int d = 0; d < fNotify->GetNumReceivers(); d++)
        fReceiverList.Append(fNotify->GetReceiver(d));
}

void plLogicModBase::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);
    stream->WriteLE32(fCommandList.GetCount());
    for(int i = 0; i < fCommandList.GetCount(); i++ )
        mgr->WriteCreatable( stream, fCommandList[i] );
    mgr->WriteCreatable( stream, fNotify );
    fFlags.Write(stream);
    stream->Writebool(fDisabled);
}
