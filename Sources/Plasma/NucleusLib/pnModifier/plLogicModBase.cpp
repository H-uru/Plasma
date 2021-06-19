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
#include "plTimerCallbackManager.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnNetCommon/plGenericVar.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plNetSharedState.h"
#include "plNetMessage/plNetMessage.h"  // breaks project dependancy levels
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plServerReplyMsg.h"

uint32_t plLogicModBase::sArbitrationDelayMs = 0;

void plLogicModBase::ConsoleTrigger(plKey playerKey)
{
    // Setup the event data in case this is a OneShot responder that needs it
    proPickedEventData ed;
    ed.fPicker = std::move(playerKey);
    ed.fPicked = nullptr;
    fNotify->AddEvent(&ed);

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
fCounter(),
fCounterLimit(),
fTimer(),
fNotify(),
fDisabled()
{
    fNotify = new plNotifyMsg;
}

plLogicModBase::~plLogicModBase()
{
    for (plMessage* command : fCommandList)
        hsRefCnt_SafeUnRef(command);

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
    plNetClientApp::GetInstance()->DebugMsg("\tLM: Attempting to set logic mod shared lock to {}, t={f}\n",
        triggered ? "Triggered" : "UnTriggered", hsTimer::GetSysSeconds());
}

bool plLogicModBase::MsgReceive(plMessage* msg)
{
    // read messages:
    plServerReplyMsg* pSMsg = plServerReplyMsg::ConvertNoRef(msg);
    if (pSMsg) {
        if (sArbitrationDelayMs == 0 || pSMsg->GetWasDelayed()) {
            IHandleArbitration(pSMsg);
        } else {
            pSMsg->SetWasDelayed(true);
            pSMsg->Ref(); // timer callback manager steals this reference
            plgTimerCallbackMgr::NewTimer(static_cast<float>(sArbitrationDelayMs) / 1000, pSMsg);
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

void plLogicModBase::IHandleArbitration(plServerReplyMsg* pSMsg)
{
    hsAssert(pSMsg->GetType() != plServerReplyMsg::kUnInit, "uninit server reply msg");
    plNetClientApp::GetInstance()->DebugMsg("LM: LogicModifier {} recvd trigger request reply:{}, wasRequesting={}, t={f}\n",
                                            GetKeyName(),
                                            pSMsg->GetType() == plServerReplyMsg::kDeny ? "denied" : "confirmed",
                                            HasFlag(kRequestingTrigger), hsTimer::GetSysSeconds());

    if (pSMsg->GetType() == plServerReplyMsg::kDeny) {
        if (HasFlag(kRequestingTrigger)) {
            plNetClientApp::GetInstance()->DebugMsg("\tLM: Denied, clearing requestingTrigger");
            ClearFlag(kRequestingTrigger);
        } else {
            plNetClientApp::GetInstance()->DebugMsg("\tLM: Denied, but not requesting?");
        }
    } else {
        bool netRequest=false;    // we're triggering as a result of a local activation
        PreTrigger(netRequest);
        IUpdateSharedState(false /* untriggering */);
    }
}

void plLogicModBase::RequestTrigger(bool netRequest)
{
    if (HasFlag(kTriggered))
    {
#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: {} ignoring RequestTrigger(), already triggered, t={f}\n",
            GetKeyName(), hsTimer::GetSysSeconds());
#endif
        return;
    }

    if (HasFlag(kRequestingTrigger))
    {
#if 1
        plNetClientApp::GetInstance()->DebugMsg("LM: {} ignoring RequestTrigger(), already requesting trigger, t={f}\n",
            GetKeyName(), hsTimer::GetSysSeconds());
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
        plNetClientApp::GetInstance()->DebugMsg("LM: {} Setting RequestingTriggert={f}\n",
            GetKeyName(), hsTimer::GetSysSeconds());
#endif

    }
}

//
// return false is counter test fails
//
bool plLogicModBase::IEvalCounter()
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

void plLogicModBase::PreTrigger(bool netRequest)
{
    if (fDisabled)
        return;

    Trigger(netRequest);
}

void plLogicModBase::Trigger(bool netRequest)
{
#if 1
    plNetClientApp::GetInstance()->DebugMsg("LogicModifier {} is triggering, activatorType={}\n",
        GetKeyName(), HasFlag(kTypeActivator));
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
    plNetClientApp::GetInstance()->DebugMsg("LogicModifier {} is Un-triggering, activatorType={}\n",
        GetKeyName(), HasFlag(kTypeActivator));
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
    for (const plKey& receiver : fReceiverList)
        fNotify->AddReceiver(receiver);
}

void plLogicModBase::AddNotifyReceiver(const plKey& receiver)
{
    fReceiverList.emplace_back(receiver);
    fNotify->AddReceiver(receiver);
}

void plLogicModBase::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);
    uint32_t n = stream->ReadLE32();
    fCommandList.resize(n);
    for (uint32_t i = 0; i < n; i++)
        fCommandList[i] = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));

    if (fNotify)
        delete fNotify;
    plNotifyMsg* pNMsg =  plNotifyMsg::ConvertNoRef(mgr->ReadCreatable(stream));
    fNotify = pNMsg;

    fFlags.Read(stream);
    fDisabled = stream->ReadBool();
    for (size_t d = 0; d < fNotify->GetNumReceivers(); d++)
        fReceiverList.emplace_back(fNotify->GetReceiver(d));
}

void plLogicModBase::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);
    stream->WriteLE32((uint32_t)fCommandList.size());
    for (plMessage* command : fCommandList)
        mgr->WriteCreatable(stream, command);
    mgr->WriteCreatable( stream, fNotify );
    fFlags.Write(stream);
    stream->WriteBool(fDisabled);
}
