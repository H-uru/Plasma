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
#include "hsUtils.h"
#include "plAxisAnimModifier.h"
#include "hsResMgr.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/plKey.h"
#include "../plNetMessage/plNetMsgHelpers.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plInputManager.h"
#include "../plInputCore/plInputInterface.h"
#include "../pnNetCommon/plNetApp.h"
#include "plgDispatch.h"

//// plAxisInputInterface ////////////////////////////////////////////////////
//	Small input interface to handle mouse imput while dragging. Supercedes
//	the sceneInteraction Interface

class plAxisInputInterface : public plInputInterface
{
	protected:

		plAxisAnimModifier	*fOwner;

		virtual ControlEventCode	*IGetOwnedCodeList( void ) const
		{
			static ControlEventCode	codes[] = { END_CONTROLS };
			return codes;
		}

	public:

		plAxisInputInterface( plAxisAnimModifier *owner ) { fOwner = owner; SetEnabled( true ); }

		virtual UInt32	GetPriorityLevel( void ) const { return kSceneInteractionPriority + 10; }
		virtual hsBool	InterpretInputEvent( plInputEventMsg *pMsg )
		{
			plMouseEventMsg* pMMsg = plMouseEventMsg::ConvertNoRef( pMsg );
			if (pMMsg )
			{
				if( pMMsg->GetButton() == kLeftButtonUp )
				{
					// Remove ourselves from the stack
					plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
					msg->SetIFace( this );
					plgDispatch::MsgSend( msg );
					return true;
				}
				return fOwner->MsgReceive( pMMsg );
			}

			return false;
		}

		virtual UInt32	GetCurrentCursorID( void ) const { return kCursorGrab; }
		virtual hsBool	HasInterestingCursorID( void ) const { return true; }
};

plAxisAnimModifier::plAxisAnimModifier() : 
fXAnim(nil), 
fYAnim(nil), 
fActive(false), 
fXPos(0.0f), 
fYPos(0.0f), 
fIface(0), 
fAllOrNothing(false)
{
	fNotify = TRACKED_NEW plNotifyMsg;
	fInputIface = TRACKED_NEW plAxisInputInterface( this );
}
plAxisAnimModifier::~plAxisAnimModifier()
{
	hsRefCnt_SafeUnRef(fNotify);
	hsRefCnt_SafeUnRef( fInputIface );
}


hsBool plAxisAnimModifier::IEval(double secs, hsScalar del, UInt32 dirty)
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

hsBool plAxisAnimModifier::MsgReceive(plMessage* msg)
{
	plEventCallbackMsg* pCall = plEventCallbackMsg::ConvertNoRef(msg);
	if (pCall)
	{
		// Send our notification to whomever cares;
		hsScalar time = 0.0f;
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
		for (int i = 0; i < pNMsg->GetEventCount(); i++)
		{
			if (pNMsg->GetEventRecord(i)->fEventType == proEventData::kActivate)
			{
				if( ( (proActivateEventData *)pNMsg->GetEventRecord(i) )->fActivate )
				{
					fActive = true;
					fXPos = plMouseDevice::Instance()->GetCursorX();
					fYPos = plMouseDevice::Instance()->GetCursorY();

					// Insert our input interface onto the input stack
					plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
					msg->SetIFace( fInputIface );
					plgDispatch::MsgSend( msg );
				}
				else
				{
/*					if (fActive)
					{
						fActive = false;

						// Remove our input interface from the input stack
						plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
						msg->SetIFace( fInputIface );
						plgDispatch::MsgSend( msg );
					}
*/				}
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
			plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
			pMsg->AddReceiver(fXAnim);
			pMsg->SetAnimName(fAnimLabel.c_str());
		//	pMsg->SetAnimName()
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
			plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
			pMsg->AddReceiver(fYAnim);
			pMsg->SetAnimName(fAnimLabel.c_str());
			if (fYPos > pMMsg->GetYPos())
			{
				if (fAllOrNothing)
				{
					pMsg->SetCmd(plAnimCmdMsg::kContinue);
					pMsg->SetCmd(plAnimCmdMsg::kSetForewards);
					fActive = false;

					// Remove our input interface from the input stack
					plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
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
					plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
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
				plEventCallbackMsg* pCall1 = TRACKED_NEW plEventCallbackMsg;
				pCall1->fEvent = kBegin;
				pCall1->fRepeats = -1;
				pCall1->AddReceiver(GetKey());
				
				plEventCallbackMsg* pCall2 = TRACKED_NEW plEventCallbackMsg;
				pCall2->fEvent = kEnd;
				pCall2->fRepeats = -1;
				pCall2->AddReceiver(GetKey());

				plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
				pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
				pMsg->AddCallback(pCall1);
				pMsg->AddCallback(pCall2);
				pMsg->SetAnimName(fAnimLabel.c_str());
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
				plEventCallbackMsg* pCall1 = TRACKED_NEW plEventCallbackMsg;
				pCall1->fEvent = kBegin;
				pCall1->fRepeats = -1;
				pCall1->AddReceiver(GetKey());
				
				plEventCallbackMsg* pCall2 = TRACKED_NEW plEventCallbackMsg;
				pCall2->fEvent = kEnd;
				pCall2->fRepeats = -1;
				pCall2->AddReceiver(GetKey());

				plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
				pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
				pMsg->AddCallback(pCall1);
				pMsg->AddCallback(pCall2);
				pMsg->AddReceiver( fYAnim );
				pMsg->SetAnimName(fAnimLabel.c_str());

				hsRefCnt_SafeUnRef( pCall1 );
				hsRefCnt_SafeUnRef( pCall2 );

				plgDispatch::MsgSend(pMsg);		
			}
			else 
			if (pRefMsg->fType == kTypeLogic)
			{
				SetNotificationKey(pRefMsg->GetRef());
			}
		}
		else
		if (pRefMsg->GetContext() == plRefMsg::kOnDestroy )
		{
			if (pRefMsg->fType == kTypeX)
				fXAnim = nil;
			else 
			if (pRefMsg->fType == kTypeY)
				fYAnim = nil;
			else 
			if (pRefMsg->fType == kTypeLogic)
				fNotificationKey = nil;
		}
		return true;	
	}
	
	return plSingleModifier::MsgReceive(msg);
}

void plAxisAnimModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeX), plRefFlags::kPassiveRef);
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeY), plRefFlags::kPassiveRef);
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kTypeLogic), plRefFlags::kPassiveRef);
	
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
