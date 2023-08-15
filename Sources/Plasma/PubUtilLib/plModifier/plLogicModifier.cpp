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

#include "plLogicModifier.h"

#include "hsTimer.h"
#include "plgDispatch.h"
#include "plTimerCallbackManager.h"
#include "pnModifier/plConditionalObject.h"
#include "plPhysical/plDetectorModifier.h"
#include "plMessage/plCondRefMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
#include "plMessage/plActivatorMsg.h"
#include "pnNetCommon/plGenericVar.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plNetSharedState.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plFakeOutMsg.h"
#include "pnMessage/plCursorChangeMsg.h"
#include "pnMessage/plNotifyMsg.h"

#include "plDetectorLog.h"
#include "plInputCore/plSceneInputInterface.h"
#include "plNetMessage/plNetMessage.h"
#include "pfConditional/plObjectInBoxConditionalObject.h"


plLogicModifier::plLogicModifier() 
{
    fMyCursor = plCursorChangeMsg::kCursorUp;
}

plLogicModifier::~plLogicModifier()
{
}

// this is a special check that asks conditions
// not if they are satisfied, but if they are ready
// to be satisfied.  Used by the activator condition
// to check that any special conditions (like players
// in boxes) are okay or not.
bool plLogicModifier::VerifyConditions(plMessage* msg)
{
    for (plConditionalObject* condition : fConditionList)
    {
        if (!condition->Verify(msg))
            return false;
    }
    return true;
}

bool plLogicModifier::MsgReceive(plMessage* msg)
{
    // read messages:
    plCondRefMsg* pCondMsg = plCondRefMsg::ConvertNoRef(msg);
    if (pCondMsg)
    {
        plConditionalObject* pCond = plConditionalObject::ConvertNoRef( pCondMsg->GetRef() );
        if (pCond && (pCondMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace)))
        {
            if ((hsSsize_t)fConditionList.size() <= pCondMsg->fWhich)
                fConditionList.resize(pCondMsg->fWhich + 1);
            
            fConditionList[pCondMsg->fWhich] = pCond;
            pCond->SetLogicMod(this);

            if (pCond->HasFlag(plConditionalObject::kLocalElement))
                SetFlag(kLocalElement);
        }
    }
    plTimerCallbackMsg* pTMsg = plTimerCallbackMsg::ConvertNoRef(msg);
    if (pTMsg)
    {
        bool netRequest = msg->HasBCastFlag(plMessage::kNetNonLocal);
        Trigger(netRequest);
    }

    plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
    
    if (pActivateMsg)
    {
//      if (pActivateMsg->fTriggerType == plActivatorMsg::kUnPickedTrigger)
        {
//          UnTrigger();
//          return true;
        }
//      else
        {   
            bool ignore=false;

            // Ignore collision activations by remote players
            if (pActivateMsg->fTriggerType==plActivatorMsg::kCollideEnter  || 
            pActivateMsg->fTriggerType==plActivatorMsg::kCollideExit || 
            pActivateMsg->fTriggerType==plActivatorMsg::kCollideContact)
            {
                if (plNetClientApp::GetInstance()->IsRemotePlayerKey(pActivateMsg->fHitterObj))
                    ignore=true;
            }

            if (!ignore)
            {
                for (plConditionalObject* condition : fConditionList)
                {
                    if (condition->MsgReceive(msg))
                        return true;
                }
            }
        }
    }

    plNotifyMsg* pNotify = plNotifyMsg::ConvertNoRef(msg);
    if (pNotify)
    {
        for (plConditionalObject* condition : fConditionList)
        {
            if (condition->MsgReceive(msg))
                return true;
        }
    }

    plFakeOutMsg* pFakeMsg = plFakeOutMsg::ConvertNoRef(msg);
    if (pFakeMsg)
    {
        plCursorChangeMsg* pMsg = nullptr;
        if ((VerifyConditions(pFakeMsg) && fMyCursor) && !Disabled())
            pMsg = new plCursorChangeMsg(fMyCursor, 1);
        else
        {
#ifndef PLASMA_EXTERNAL_RELEASE
            // try to determine the reasons for not displaying cursor
            if (plSceneInputInterface::fShowLOS)
            {
                if ( Disabled() )
                {
                    plDetectorLog::Red("{}: LogicMod is disabled", GetKeyName());
                }
                else
                {
                    for (plConditionalObject* condition : fConditionList)
                    {
                        if (!condition->Verify(msg))
                        {
                            plDetectorLog::Red("{}: LogicMod {} conditional not met", condition->GetKeyName(), condition->ClassName());
                        }
                    }
                }
            }
#endif  // PLASMA_EXTERNAL_RELEASE
            pMsg = new plCursorChangeMsg(plCursorChangeMsg::kNullCursor, 1);
        }
        
        pMsg->AddReceiver( pFakeMsg->GetSender() );
        pMsg->SetSender(GetKey());
        plgDispatch::MsgSend(pMsg);

        return true;
    }

    return (plLogicModBase::MsgReceive(msg));   
}

void plLogicModifier::RequestTrigger(bool netRequest)
{
    for (plConditionalObject* condition : fConditionList)
    {
        if (!condition->Satisfied())
            return;
    }

    plLogicModBase::RequestTrigger(netRequest);
}

void plLogicModifier::PreTrigger(bool netRequest)
{
    if (!IEvalCounter())
        return;

    if (fTimer)
    {
        plgTimerCallbackMgr::NewTimer( fTimer, new plTimerCallbackMsg( GetKey() ) );
        return;
    }
    plLogicModBase::PreTrigger(netRequest);
}

// Update generic shared state (which reflects trigger state) on server
// by sending TestAndSet request.  By locking and unlocking the sharedState,
// we can guarantee that only one logicMod instance can trigger at a time.
// The server will confirm or deny our request to lock and set the state.
void plLogicModifier::UpdateSharedState(bool triggered) const
{
    plNetSharedState ss("TrigState");
    plGenericVar* sv = new plGenericVar("Triggered");
    sv->Value().SetBool(triggered); // attempting to set trig state to true
    ss.AddVar(sv);

    bool lock = triggered;

    // if unlocking, then the server does not need to store this state, since it's back to its default state
    ss.SetServerMayDelete(!lock);

    plNetMsgTestAndSet ts;
    ts.CopySharedState(&ss);
    ts.ObjectInfo()->SetFromKey(GetKey());
    ts.SetLockRequest(lock);        // if triggering, lock state, else unlock state
    plNetClientApp::GetInstance()->SendMsg(&ts);
    plNetClientApp::GetInstance()->DebugMsg("\tLM: Attempting to set logic mod shared lock to {}, t={f}\n",
        triggered ? "Triggered" : "UnTriggered", hsTimer::GetSysSeconds());
}


void plLogicModifier::Reset(bool bCounterReset)
{
    plLogicModBase::Reset(bCounterReset);
    for (plConditionalObject* condition : fConditionList)
        condition->Reset();
}

void plLogicModifier::Read(hsStream* stream, hsResMgr* mgr)
{
    plLogicModBase::Read(stream, mgr);
    plCondRefMsg* refMsg;
    uint32_t n = stream->ReadLE32();
    fConditionList.assign(n, nullptr);
    for (uint32_t i = 0; i < n; i++)
    {
        refMsg = new plCondRefMsg(GetKey(), i);
        mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kActiveRef);
    }
    fMyCursor = stream->ReadLE32();
}

void plLogicModifier::Write(hsStream* stream, hsResMgr* mgr)
{
    plLogicModBase::Write(stream, mgr);
    stream->WriteLE32((uint32_t)fConditionList.size());
    for (plConditionalObject* condition : fConditionList)
        mgr->WriteKey(stream, condition);
    stream->WriteLE32(fMyCursor);
}

void plLogicModifier::AddCondition(plConditionalObject* c)
{
    plGenRefMsg *msg= new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
    hsgResMgr::ResMgr()->AddViaNotify(c->GetKey(), msg, plRefFlags::kActiveRef); 

    fConditionList.emplace_back(c);
    c->SetLogicMod(this);
    if (c->HasFlag(plConditionalObject::kLocalElement))
        SetFlag(kLocalElement);
}

void plLogicModifier::VolumeIgnoreExtraEnters(bool ignore /* = true */)
{
    for (plConditionalObject* curCondition : fConditionList)
    {
        plVolumeSensorConditionalObject* condition = plVolumeSensorConditionalObject::ConvertNoRef(curCondition);
        if (condition)
            condition->IgnoreExtraEnters(ignore);
    }
}

void plLogicModifier::VolumeNoArbitration(bool noArbitration)
{
    for (plConditionalObject* curCondition : fConditionList) {
        plVolumeSensorConditionalObject* condition = plVolumeSensorConditionalObject::ConvertNoRef(curCondition);
        if (condition)
            condition->NoServerArbitration(noArbitration);
    }
}
