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
#include "plLogicModifier.h"
#include "plgDispatch.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../pnModifier/plConditionalObject.h"
#include "../plPhysical/plDetectorModifier.h"
#include "../plMessage/plCondRefMsg.h"
#include "../plMessage/plTimerCallbackMsg.h"
#include "../plMessage/plActivatorMsg.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plFakeOutMsg.h"
#include "../pnMessage/plCursorChangeMsg.h"
#include "../pnMessage/plNotifyMsg.h"

#include "../plModifier/plDetectorLog.h"
#include "../plInputCore/plSceneInputInterface.h"
#include "../../FeatureLib/pfConditional/plFacingConditionalObject.h"
#include "../../FeatureLib/pfConditional/plObjectInBoxConditionalObject.h"


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
hsBool plLogicModifier::VerifyConditions(plMessage* msg)
{
	for (int i = 0; i < fConditionList.Count(); i++)
	{
		if (!fConditionList[i]->Verify(msg))
			return false;
	}
	return true;
}

hsBool plLogicModifier::MsgReceive(plMessage* msg)
{
	hsBool retVal = false;
	
	// read messages:
	plCondRefMsg* pCondMsg = plCondRefMsg::ConvertNoRef(msg);
	if (pCondMsg)
	{
		plConditionalObject* pCond = plConditionalObject::ConvertNoRef( pCondMsg->GetRef() );
		if (pCond && (pCondMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace)))
		{
			if (fConditionList.Count() <= pCondMsg->fWhich)
				fConditionList.ExpandAndZero(pCondMsg->fWhich + 1);
			
 			fConditionList[pCondMsg->fWhich] = pCond;
			pCond->SetLogicMod(this);

			if (pCond->HasFlag(plConditionalObject::kLocalElement))
				SetFlag(kLocalElement);
		}
		retVal = true;
	}
	plTimerCallbackMsg* pTMsg = plTimerCallbackMsg::ConvertNoRef(msg);
	if (pTMsg)
	{
		hsBool netRequest = msg->HasBCastFlag(plMessage::kNetNonLocal);
		Trigger(netRequest);
	}

	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	
	if (pActivateMsg)
	{
//		if (pActivateMsg->fTriggerType == plActivatorMsg::kUnPickedTrigger)
		{
//			UnTrigger();
//			return true;
		}
//		else
		{	
			hsBool ignore=false;

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
				for (int i = 0; i < fConditionList.Count(); i++)
				{
					if (fConditionList[i]->MsgReceive(msg))
						return true;
				}
			}
		}
	}

	plNotifyMsg* pNotify = plNotifyMsg::ConvertNoRef(msg);
	if (pNotify)
	{
		for (int i = 0; i < fConditionList.Count(); i++)
		{
			if (fConditionList[i]->MsgReceive(msg))
				return true;
		}
	}

	plFakeOutMsg* pFakeMsg = plFakeOutMsg::ConvertNoRef(msg);
	if (pFakeMsg)
	{
		plCursorChangeMsg* pMsg = 0;
		if ((VerifyConditions(pFakeMsg) && fMyCursor) && !Disabled())
			pMsg = TRACKED_NEW plCursorChangeMsg(fMyCursor, 1);
		else
		{
#ifndef PLASMA_EXTERNAL_RELEASE
			// try to determine the reasons for not displaying cursor
			if (plSceneInputInterface::fShowLOS)
			{
				if ( Disabled() )
				{
					DetectorLogRed("%s: LogicMod is disabled", GetKeyName());
				}
				else
				{
					for (int i = 0; i < fConditionList.Count(); i++)
					{
						if (!fConditionList[i]->Verify(msg))
						{
							if ( plObjectInBoxConditionalObject::ConvertNoRef(fConditionList[i]) )
								DetectorLogRed("%s: LogicMod InRegion conditional not met", fConditionList[i]->GetKeyName());
							else if ( plFacingConditionalObject::ConvertNoRef(fConditionList[i]) )
								DetectorLogRed("%s: LogicMod Facing conditional not met", fConditionList[i]->GetKeyName());
							else
								DetectorLogRed("%s: LogicMod <unknown> conditional not met", fConditionList[i]->GetKeyName());
						}
					}
				}
			}
#endif	// PLASMA_EXTERNAL_RELEASE
			pMsg = TRACKED_NEW plCursorChangeMsg(plCursorChangeMsg::kNullCursor, 1);
		}
		
		pMsg->AddReceiver( pFakeMsg->GetSender() );
		pMsg->SetSender(GetKey());
		plgDispatch::MsgSend(pMsg);

		return true;
	}

	return (plLogicModBase::MsgReceive(msg));	
}

void plLogicModifier::RequestTrigger(hsBool netRequest)
{
	for (int i = 0; i < fConditionList.Count(); i++)
	{
		if (!fConditionList[i]->Satisfied())
			return;
	}

	plLogicModBase::RequestTrigger(netRequest);
}

void plLogicModifier::PreTrigger(hsBool netRequest)
{
	if (!IEvalCounter())
		return;

	if (fTimer)
	{
		plgTimerCallbackMgr::NewTimer( fTimer, TRACKED_NEW plTimerCallbackMsg( GetKey() ) );
		return;
	}
	plLogicModBase::PreTrigger(netRequest);
}


void plLogicModifier::Reset(bool bCounterReset)
{
	plLogicModBase::Reset(bCounterReset);
	for (int i = 0; i < fConditionList.Count(); i++)
		fConditionList[i]->Reset();
}

void plLogicModifier::Read(hsStream* stream, hsResMgr* mgr)
{
	plLogicModBase::Read(stream, mgr);
	plCondRefMsg* refMsg;
	int n = stream->ReadSwap32();
	fConditionList.SetCountAndZero(n);
	int i;
	for(i = 0; i < n; i++ )
	{	
		refMsg = TRACKED_NEW plCondRefMsg(GetKey(), i);
		mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kActiveRef);
	}
	fMyCursor = stream->ReadSwap32();
}

void plLogicModifier::Write(hsStream* stream, hsResMgr* mgr)
{
	plLogicModBase::Write(stream, mgr);
	stream->WriteSwap32(fConditionList.GetCount());
	for( int i = 0; i < fConditionList.GetCount(); i++ )
		mgr->WriteKey(stream, fConditionList[i]);
	stream->WriteSwap32(fMyCursor);
}

void plLogicModifier::AddCondition(plConditionalObject* c)
{
	plGenRefMsg *msg= TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
	hsgResMgr::ResMgr()->AddViaNotify(c->GetKey(), msg, plRefFlags::kActiveRef); 

	fConditionList.Append(c); 
	c->SetLogicMod(this);
	if (c->HasFlag(plConditionalObject::kLocalElement))
		SetFlag(kLocalElement);
}

void plLogicModifier::VolumeIgnoreExtraEnters(bool ignore /* = true */)
{
	for (int curCondition = 0; curCondition < fConditionList.GetCount(); ++curCondition)
	{
		plVolumeSensorConditionalObject* condition = plVolumeSensorConditionalObject::ConvertNoRef(fConditionList[curCondition]);
		if (condition)
			condition->IgnoreExtraEnters(ignore);
	}
}
