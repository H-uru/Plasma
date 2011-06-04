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
#include "../plAvatar/plAvCallbackAction.h"

#include "hsTypes.h"
#include "plCollisionDetector.h"
#include "../plMessage/plCollideMsg.h"
#include "plgDispatch.h"
#include "../plMessage/plActivatorMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../pnInputCore/plControlEventCodes.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetLinkingMgr.h"

#include "plPhysical.h"

#include "../pnMessage/plPlayerPageMsg.h"
#include "../plMessage/plSimStateMsg.h"

#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plAvBrainDrive.h"

#include "../plModifier/plDetectorLog.h"

#define USE_PHYSX_MULTIPLE_CAMREGION_ENTER 1
#define USE_PHYSX_COLLISION_FLUTTER_WORKAROUND 1

plArmatureMod* plCollisionDetector::IGetAvatarModifier(plKey key)
{
	plSceneObject* avObj = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
	if (avObj)
	{
		// search through its modifiers to see if one of them is an avatar modifier
		plArmatureMod* avMod = nil;
		for (int i = 0; i < avObj->GetNumModifiers(); i++)
		{
			const plModifier* mod = avObj->GetModifier(i);
			// see if it is an avatar mod base class
			avMod = const_cast<plArmatureMod*>(plArmatureMod::ConvertNoRef(mod));
			if (avMod)
				return avMod;
		}
	}

	return nil;
}

bool plCollisionDetector::IIsDisabledAvatar(plKey key)
{
	plArmatureMod* avMod = IGetAvatarModifier(key);
	plArmatureBrain* avBrain = avMod ? avMod->GetCurrentBrain() : nil;
	return (plAvBrainDrive::ConvertNoRef(avBrain) != nil);
}

hsBool plCollisionDetector::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		// If the avatar is disabled (flying around), don't trigger
		if (IIsDisabledAvatar(pCollMsg->fOtherKey))
			return false;

		if (fType & kTypeBump) 
		{
			if (!fBumped && !fTriggered)
			{
				for (int i = 0; i < fReceivers.Count(); i++)
				{
					plActivatorMsg* pMsg = TRACKED_NEW plActivatorMsg;
					pMsg->AddReceiver( fReceivers[i] );

					if (fProxyKey)
						pMsg->fHiteeObj = fProxyKey;
					else
						pMsg->fHiteeObj = GetTarget()->GetKey();
					pMsg->fHitterObj = pCollMsg->fOtherKey;
					pMsg->SetSender(GetKey());
					pMsg->SetTriggerType( plActivatorMsg::kCollideContact );
					plgDispatch::MsgSend( pMsg );
				}
				fBumped = true;
				fTriggered = true;
				plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
				return true;
			}
			if (fTriggered)
			{
				fBumped = true;
				return true;
			}
			return false;
		}

		for (int i = 0; i < fReceivers.Count(); i++)
		{
			plActivatorMsg* pMsg = TRACKED_NEW plActivatorMsg;
			pMsg->AddReceiver( fReceivers[i] );
			if (fProxyKey)
				pMsg->fHiteeObj = fProxyKey;
			else
				pMsg->fHiteeObj = GetTarget()->GetKey();
			pMsg->fHitterObj = pCollMsg->fOtherKey;
			pMsg->SetSender(GetKey());
			
			if (fType & kTypeEnter && pCollMsg->fEntering)
			{
				pMsg->SetTriggerType( plActivatorMsg::kCollideEnter );
				plgDispatch::MsgSend( pMsg );
				continue;
			}
			if (fType & kTypeUnEnter && pCollMsg->fEntering)
			{
				pMsg->SetTriggerType( plActivatorMsg::kEnterUnTrigger );
				plgDispatch::MsgSend( pMsg );
				continue;
			}
			if(fType & kTypeExit && !pCollMsg->fEntering)
			{
				pMsg->SetTriggerType( plActivatorMsg::kCollideExit );
				plgDispatch::MsgSend( pMsg );
				continue;
			}
			if(fType & kTypeUnExit && !pCollMsg->fEntering)
			{
				pMsg->SetTriggerType( plActivatorMsg::kExitUnTrigger );
				plgDispatch::MsgSend( pMsg );
				continue;
			}
			if (fType & kTypeAny)
			{
				pMsg->SetTriggerType( plActivatorMsg::kCollideContact );
				plgDispatch::MsgSend( pMsg );
				continue;
			}

			delete (pMsg);
		}
		return true;
	}

	plEvalMsg* pEval = plEvalMsg::ConvertNoRef(msg);
	if (pEval)
	{
		if (!fBumped && fTriggered)
		{
			plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
			for (int i = 0; i < fReceivers.Count(); i++)
			{
				plActivatorMsg* pMsg = TRACKED_NEW plActivatorMsg;
				pMsg->AddReceiver( fReceivers[i] );
				if (fProxyKey)
					pMsg->fHiteeObj = fProxyKey;
				else
					pMsg->fHiteeObj = GetTarget()->GetKey();
				pMsg->SetSender(GetKey());
				pMsg->SetTriggerType( plActivatorMsg::kCollideUnTrigger );
				plgDispatch::MsgSend( pMsg );
				fTriggered = false;
			}
		}
		else
		if (fTriggered && fBumped)
		{
			fBumped = false;
		}
		return true;
	}

	return plDetectorModifier::MsgReceive(msg);
}

void plCollisionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Read(stream, mgr);
	stream->ReadSwap(&fType);
}
void plCollisionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Write(stream, mgr);
	stream->WriteSwap(fType);
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// camera region detector

plCameraRegionDetector::~plCameraRegionDetector()
{
	for(int i = 0; i < fMessages.Count(); i++)
	{	
		plMessage* pMsg = fMessages[i];
		fMessages.Remove(i);
		delete(pMsg);
	}
	fMessages.SetCountAndZero(0);
}

void plCameraRegionDetector::ITrigger(plKey hitter, bool entering, bool immediate)
{

#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
	if (entering && fNumEvals - fLastExitEval <= 1 && fSavingSendMsg)
	{
		DetectorLog("%s: Skipping Camera Entering volume", GetKeyName());
		fLastEnterEval = fNumEvals;
		if (fSavingSendMsg)
		{
			DetectorLog("%s: Dumping saved Camera Exiting volume", GetKeyName());
		}
		fSavingSendMsg = false;
		return;
	}
	if (!entering && fNumEvals - fLastEnterEval <= 1 && fSavingSendMsg)
	{
		DetectorLog("%s: Skipping Exiting volume", GetKeyName());
		fLastExitEval = fNumEvals;
		if (fSavingSendMsg)
		{
			DetectorLog("%s: Dumping saved Camera Entering volume", GetKeyName());
		}
		fSavingSendMsg = false;
		return;
	}

	// get rid of any saved messages... this should happen though
	if (fSavingSendMsg)
	{
		DetectorLog("%s: Killing saved camera message... shouldn't happen", GetKeyName());
	}
	// end PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND

	fSavingSendMsg = true;
	fSavedMsgEnterFlag = entering;
	if (entering)
	{
		//DetectorLog("%s: Saving camera Entering volume - Evals=%d", GetKeyName(),fNumEvals);
		fLastEnterEval = fNumEvals;
	}
	else
	{
		//DetectorLog("%s: Saving camera Exiting volume - Evals=%d", GetKeyName(),fNumEvals);
		fLastExitEval = fNumEvals;
	}

#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
	// we're saving the message to be dispatched later...
	if (immediate)
	{
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND

		ISendSavedTriggerMsgs();

#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	}
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND


}

void plCameraRegionDetector::ISendSavedTriggerMsgs()
{
	if (fSavingSendMsg)
	{
		for (int i = 0; i < fMessages.Count(); i++)
		{	
			char str[256];

			hsRefCnt_SafeRef(fMessages[i]);
			if (fSavedMsgEnterFlag)
			{
				fMessages[i]->SetCmd(plCameraMsg::kEntering);
				sprintf(str, "Entering cameraRegion: %s - Evals=%d -msg %d of %d\n", GetKeyName(),fNumEvals,i+1,fMessages.Count());
				fIsInside = true;
			}
			else
			{
				fMessages[i]->ClearCmd(plCameraMsg::kEntering);
				sprintf(str, "Exiting cameraRegion: %s - Evals=%d -msg %d of %d\n", GetKeyName(),fNumEvals,i+1,fMessages.Count());
				fIsInside = false;
			}
			plgDispatch::MsgSend(fMessages[i]);
			DetectorLog("%s", str);
		}
	}
	fSavingSendMsg = false;
}


hsBool plCameraRegionDetector::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		// camera collisions are only for the local player
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
			return true;
		

#ifdef USE_PHYSX_MULTIPLE_CAMREGION_ENTER
		// first determine if this is a multiple camera region enter (PHYSX BUG WORKAROUND)
		if (!fWaitingForEval)
		{//plObjectInVolumeCollisionDetector::MsgReceive() will flip fWaitingForEval
		// and registers for the Eval, child objects of plObjectInVolumeCollisionDetector
		//must decide when they are no longer interested in Evals. I suggest using IHandleEvals()
		
			fNumEvals = 0;
			fLastEnterEval=-999;
			fLastExitEval=-999;
		}
	
		// end of (PHYSX BUG WORKAROUND)
#endif  // USE_PHYSX_MULTIPLE_CAMREG_ENTER

	}
	return plObjectInVolumeDetector::MsgReceive(msg);
}
void plCameraRegionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Read(stream, mgr);
	int n = stream->ReadSwap32();
	fMessages.SetCountAndZero(n);
	for(int i = 0; i < n; i++ )
	{	
		plCameraMsg* pMsg =  plCameraMsg::ConvertNoRef(mgr->ReadCreatable(stream));
		fMessages[i] = pMsg;
	}

}
void plCameraRegionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Write(stream, mgr);
	stream->WriteSwap32(fMessages.GetCount());
	for(int i = 0; i < fMessages.GetCount(); i++ )
		mgr->WriteCreatable( stream, fMessages[i] );

}
void plCameraRegionDetector::IHandleEval(plEvalMsg *pEval)
{
	fNumEvals++;
	if (fNumEvals - fLastEnterEval > 1 && fNumEvals-fLastExitEval>1)
	{
		ISendSavedTriggerMsgs();
		plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		fWaitingForEval = false;
	}
	else
	{
		if(fSavedActivatorMsg)
			DetectorLog("%s didn't send its message. fNumEvals=%d fLastEnterEval=%d, fLastExit=%d",
				GetKeyName(),fNumEvals, fLastEnterEval, fLastExitEval);
		
	}
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// object-in-volume detector

void plObjectInVolumeDetector::ITrigger(plKey hitter, bool entering, bool immediate)
{
#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
/*	if (entering && fNumEvals - fLastExitEval <= 1 && fSavedActivatorMsg)
	{
		//DetectorLog("%s: Skipping Entering volume", GetKeyName());
		fLastEnterEval = fNumEvals;
		if (fSavedActivatorMsg)
		{
			//DetectorLog("%s: Dumping saved Exiting volume", GetKeyName());
			delete fSavedActivatorMsg;
		}
		fSavedActivatorMsg = nil;
		return;
	}
	if (!entering && fNumEvals - fLastEnterEval <= 1 && fSavedActivatorMsg)
	{
		//DetectorLog("%s: Skipping Exiting volume", GetKeyName());
		fLastExitEval = fNumEvals;
		if (fSavedActivatorMsg)
		{
			//DetectorLog("%s: Dumping saved Entering volume", GetKeyName());
			delete fSavedActivatorMsg;
		}
		fSavedActivatorMsg = nil;
		return;
	}

	// get rid of any saved messages... this should happen though
	if (fSavedActivatorMsg)
	{
		delete fSavedActivatorMsg;
		DetectorLog("%s: Killing saved message... shouldn't happen", GetKeyName());
	}
	// end PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
*/
	if(!immediate)
	{
		
		bookKeepingList::iterator curit=fCollisionList.begin();
		while(curit!=fCollisionList.end())
		{
			if(hitter==(*curit)->hitter)
			{//hey the object is already in my list
				//try and figure out what my real state is
				if(entering)
				{
					(*curit)->enters++;
					if(!(*curit)->fSubStepCurState)
					{//We weren't already in
						(*curit)->fSubStepCurState =true;
					}
				}
				else
				{
					(*curit)->exits++;
					if((*curit)->fSubStepCurState)
					{//We were already in
						(*curit)->fSubStepCurState =false;
					}
				}
				//get out
				break;
			}
			curit++;
		}
		if(curit==fCollisionList.end())
		{
			//hitter was not in the list add him in
			//hitter was not in the current frame list
			//lets find out its state in the begining of the frame
			ResidentSet::iterator curres = fCurrentResidents.find(hitter);
			bool initialState;
			if(curres != fCurrentResidents.end())
				initialState =true;
			else
				initialState =false;

			plCollisionBookKeepingInfo* BookKeeper=TRACKED_NEW plCollisionBookKeepingInfo(hitter);
			if(entering)
			{
				BookKeeper->enters++;
				BookKeeper->fSubStepCurState =true;
			}
			else
			{
				BookKeeper->exits++;
				BookKeeper->fSubStepCurState =false;
			}
			fCollisionList.push_front(BookKeeper);
		}


	}
	else
	{
		plActivatorMsg* ActivatorMsg = TRACKED_NEW plActivatorMsg;
		ActivatorMsg->AddReceivers(fReceivers);
		if (fProxyKey)
			ActivatorMsg->fHiteeObj = fProxyKey;
		else
			ActivatorMsg->fHiteeObj = GetTarget()->GetKey();
	
		ActivatorMsg->fHitterObj = hitter;
		ActivatorMsg->SetSender(GetKey());
		if (entering)
		{
			ActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeEnter);
		}
		else
		{
			ActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeExit);
		}
		
		plgDispatch::MsgSend(ActivatorMsg);		
		

	}
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND

/*	fSavedActivatorMsg = TRACKED_NEW plActivatorMsg;

	fSavedActivatorMsg->AddReceivers(fReceivers);

	if (fProxyKey)
		fSavedActivatorMsg->fHiteeObj = fProxyKey;
	else
		fSavedActivatorMsg->fHiteeObj = GetTarget()->GetKey();

	fSavedActivatorMsg->fHitterObj = hitter;
	fSavedActivatorMsg->SetSender(GetKey());

	if (entering)
	{
		//DetectorLog("%s: Saving Entering volume - Evals=%d", GetKeyName(),fNumEvals);
		fSavedActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeEnter);
		fLastEnterEval = fNumEvals;
	}
	else
	{
		//DetectorLog("%s: Saving Exiting volume - Evals=%d", GetKeyName(),fNumEvals);
		fSavedActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeExit);
		fLastExitEval = fNumEvals;
	}
*/
#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
	// we're saving the message to be dispatched later...
	if (immediate)
	{
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND

		
	//	fSavedActivatorMsg = nil;

#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	}
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
}
/*
void plObjectInVolumeDetector::ISendSavedTriggerMsgs()
{
	if (fSavedActivatorMsg)
	{
		if (fSavedActivatorMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
			DetectorLog("%s: Sending Entering volume - Evals=%d", GetKeyName(),fNumEvals);
		else
			DetectorLog("%s: Sending Exiting volume - Evals=%d", GetKeyName(),fNumEvals);

		// we're saving the message to be dispatched later...
		plgDispatch::MsgSend(fSavedActivatorMsg);
	}
	fSavedActivatorMsg = nil;
}
*/
hsBool plObjectInVolumeDetector::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);
	if (pCollMsg)
	{
		// If the avatar is disabled (flying around), don't trigger
		if (IIsDisabledAvatar(pCollMsg->fOtherKey))
			return false;
       
#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
		// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
		if (!fWaitingForEval)
		{
			plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
			fWaitingForEval = true;
		}
		// end PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
		ITrigger(pCollMsg->fOtherKey, (pCollMsg->fEntering != 0));

		return true;
	}

#ifdef USE_PHYSX_COLLISION_FLUTTER_WORKAROUND
	// PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
	plEvalMsg* pEval = plEvalMsg::ConvertNoRef(msg);
	if (pEval)
	{
		
		//if (fSavedActivatorMsg)
		//	DetectorLog("%s: InVolumeEval=%d with saved message", GetKeyName(), fNumEvals);
		//else
		//	DetectorLog("%s: InVolumeEval=%d without saved message", GetKeyName(), fNumEvals);
		IHandleEval(pEval);	

	}
	// end PHYSX_FIXME hack for PhysX turd that sends bunches of enter/exits over one frame
#endif   // USE_PHYSX_COLLISION_FLUTTER_WORKAROUND

	plPlayerPageMsg* pageMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pageMsg && pageMsg->fUnload)
	{
		ITrigger(pageMsg->fPlayer, false);
	}

	return plCollisionDetector::MsgReceive(msg);
}

void plObjectInVolumeDetector::IHandleEval(plEvalMsg* pEval)
{
		plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		fWaitingForEval = false;
		for(bookKeepingList::iterator it= (--fCollisionList.end());it!=(--fCollisionList.begin()); it--)
		{
			bool alreadyInside;
			ResidentSet::iterator HitIt;
			HitIt = fCurrentResidents.find((*it)->hitter);
			if(HitIt != fCurrentResidents.end()) alreadyInside = true;
			else alreadyInside=false;
			plActivatorMsg* actout=TRACKED_NEW plActivatorMsg;
			actout->fHitterObj=((*it)->hitter);
			actout->SetSender(GetKey());
			if (fProxyKey)
				actout->fHiteeObj = fProxyKey;
			else
				actout->fHiteeObj = GetTarget()->GetKey();
			if((*it)->fSubStepCurState)//current substate says we are entered
			{//different enters and exits
				//figure out what to do
					if(!alreadyInside)
					{//we are actuall entering
						actout->SetTriggerType(plActivatorMsg::kVolumeEnter);
						fCurrentResidents.insert((*it)->hitter);
						actout->AddReceivers(fReceivers);
						actout->Send();
						DetectorLog("%s sent an Enter ActivatorMsg. To: %s", GetKeyName(), GetTarget()->GetKeyName()  );
					}
					else
					{
						DetectorLog("%s squelched an Enter ActivatorMsg.", GetKeyName());
						delete actout;
					}
			}
			else
			{
				//fSubStepCurState says we are outside
					if(alreadyInside)
					{//we are actuall exiting
						actout->SetTriggerType(plActivatorMsg::kVolumeExit);
						fCurrentResidents.erase((*it)->hitter);
						actout->AddReceivers(fReceivers);
						actout->Send();
						DetectorLog("%s sent an Exit ActivatorMsg. To: %s", GetKeyName(), GetTarget()->GetKeyName());
					}
					else
					{
						DetectorLog("%s squelched an Exit ActivatorMsg.", GetKeyName());
						delete actout;
					}
			}
		}
		DetectorLog("*********");
		for(bookKeepingList::iterator it = fCollisionList.begin(); it != fCollisionList.end(); it ++)
		{
			delete (*it);
		}
		DetectorLog("This is the regions inhabitants after the op");
		for(ResidentSet::iterator it = fCurrentResidents.begin(); it!= fCurrentResidents.end(); it++)
		{
			DetectorLog("%s", (*it)->GetName());
		}
		DetectorLog("*********");

		fCollisionList.clear();
		
}

void plObjectInVolumeDetector::SetTarget(plSceneObject* so)
{
	plCollisionDetector::SetTarget(so);

	if (so)
		plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	else
		plgDispatch::Dispatch()->UnRegisterForExactType(plPlayerPageMsg::Index(), GetKey());
}

void plObjectInVolumeDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plCollisionDetector::Read(stream, mgr);
}

void plObjectInVolumeDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plCollisionDetector::Write(stream, mgr);
}


///////////////////////////////////////////////////////////////////////////////


plObjectInVolumeAndFacingDetector::plObjectInVolumeAndFacingDetector() :
	fFacingTolerance(0),
	fNeedWalkingForward(false),
	fAvatarInVolume(false),
	fTriggered(false)
{
}

plObjectInVolumeAndFacingDetector::~plObjectInVolumeAndFacingDetector()
{
}

void plObjectInVolumeAndFacingDetector::SetFacingTolerance(int degrees)
{
	fFacingTolerance = hsCosine(hsScalarDegToRad(degrees));
}

void plObjectInVolumeAndFacingDetector::ICheckForTrigger()
{
	plArmatureMod* armMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	plSceneObject* avatar = armMod ? armMod->GetTarget(0) : nil;
	plSceneObject* target = GetTarget();

	if (armMod && target)
	{
		hsVector3 playerView = avatar->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
		hsVector3 objView = target->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);

		playerView.Normalize();
		objView.Normalize();

		hsScalar dot = playerView * objView;
//		hsStatusMessageF("Dot: %f Tolerance: %f", dot, fFacingTolerance);
		bool facing = dot >= fFacingTolerance;

		bool movingForward = false;
		if (fNeedWalkingForward)
		{
			// And are we walking towards it?
			plArmatureBrain* abrain =  armMod->FindBrainByClass(plAvBrainHuman::Index()); //armMod->GetCurrentBrain();
			plAvBrainHuman* brain = plAvBrainHuman::ConvertNoRef(abrain);
			if (brain && brain->IsMovingForward() && brain->fCallbackAction->IsOnGround())
				movingForward = true;
		}
		else
			movingForward = true;

		if (facing && movingForward && !fTriggered)
		{
			DetectorLog("%s: Trigger InVolume&Facing", GetKeyName());
			fTriggered = true;
			ITrigger(avatar->GetKey(), true, true);
		}
		else if (!facing && fTriggered)
		{
			DetectorLog("%s: Untrigger InVolume&Facing", GetKeyName());
			fTriggered = false;
			ITrigger(avatar->GetKey(), false, true);
		}
	}
}

hsBool plObjectInVolumeAndFacingDetector::MsgReceive(plMessage* msg)
{
	// Avatar is entering or exiting our detector box
	plCollideMsg* collMsg = plCollideMsg::ConvertNoRef(msg);
	if (collMsg)
	{
		// make sure this is the local player... the notify will be the thing that propagates over the network
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != collMsg->fOtherKey)
			return true;

		// If the avatar is disabled (flying around), don't trigger
		if (IIsDisabledAvatar(collMsg->fOtherKey))
			return false;

		fAvatarInVolume = (collMsg->fEntering != 0);

		if (fAvatarInVolume)
		{
			plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
			ICheckForTrigger();
		}
		else
		{
			plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());

			// Avatar is leaving the volume, make sure to untrigger if we haven't already
			if (fTriggered)
			{
				fTriggered = false;
				ITrigger(plNetClientApp::GetInstance()->GetLocalPlayerKey(), false, true);
			}
		}

		return true;
	}

	// Avatar is inside our detector box, so every frame we check if we need to trigger
	plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
	if (evalMsg)
	{
		ICheckForTrigger();
		return true;
	}

	return plObjectInVolumeDetector::MsgReceive(msg);
}

void plObjectInVolumeAndFacingDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plObjectInVolumeDetector::Read(stream, mgr);

	fFacingTolerance = stream->ReadSwapScalar();
	fNeedWalkingForward = stream->Readbool();
}

void plObjectInVolumeAndFacingDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plObjectInVolumeDetector::Write(stream, mgr);

	stream->WriteSwapScalar(fFacingTolerance);
	stream->Writebool(fNeedWalkingForward);
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// subworld region detector

plSubworldRegionDetector::~plSubworldRegionDetector()
{
}


hsBool plSubworldRegionDetector::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{	
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
			return true;

		plArmatureMod* avMod = IGetAvatarModifier(pCollMsg->fOtherKey);
		if (avMod)
		{
			DetectorLog("%s subworld detector %s", pCollMsg->fEntering ? "Entering" : "Exiting", GetKeyName());

			if ((pCollMsg->fEntering && !fOnExit) ||
				(!pCollMsg->fEntering && fOnExit))
			{
				if (fSub)
				{
					plSceneObject* SO = plSceneObject::ConvertNoRef(fSub->ObjectIsLoaded());
					if (SO)
					{
						DetectorLogSpecial("Switching to subworld %s", fSub->GetName());

						plKey nilKey;
						plSubWorldMsg* msg = TRACKED_NEW plSubWorldMsg(GetKey(), avMod->GetKey(), fSub);
						msg->Send();
					}
				}
				else
				{
					DetectorLogSpecial("Switching to main subworld");
					plSubWorldMsg* msg = TRACKED_NEW plSubWorldMsg(GetKey(), avMod->GetKey(), nil);
					msg->Send();
				}
			}
		}

		return true;
	}

	return plCollisionDetector::MsgReceive(msg);
}

void plSubworldRegionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Read(stream, mgr);
	fSub = mgr->ReadKey(stream);
	fOnExit = stream->ReadBool();
}
void plSubworldRegionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
	plDetectorModifier::Write(stream, mgr);
	mgr->WriteKey(stream, fSub);
	stream->WriteBool(fOnExit);
}

///////////////////////////////////
///////////////////////////////////
/// plPanicLinkDetector
///////////////////////////////////
hsBool plPanicLinkRegion::MsgReceive(plMessage* msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
			return true;

		if (pCollMsg->fEntering)
		{
			plArmatureMod* avMod = IGetAvatarModifier(pCollMsg->fOtherKey);
			if (avMod)
			{
				hsPoint3 kinPos;
				if (avMod->GetController())
				{
					avMod->GetController()->GetKinematicPosition(kinPos);
					DetectorLogSpecial("Avatar is panic linking. KinPos at %f,%f,%f and is %s",kinPos.fX,kinPos.fY,kinPos.fZ,avMod->GetController()->IsEnabled() ? "enabled" : "disabled");
				}
				avMod->PanicLink(fPlayLinkOutAnim);
			}
		}

		return true;
	}

	return plCollisionDetector::MsgReceive(msg);
}

void plPanicLinkRegion::Read(hsStream* stream, hsResMgr* mgr)
{
	plCollisionDetector::Read(stream, mgr);

	fPlayLinkOutAnim = stream->ReadBool();
}

void plPanicLinkRegion::Write(hsStream* stream, hsResMgr* mgr)
{
	plCollisionDetector::Write(stream, mgr);

	stream->WriteBool(fPlayLinkOutAnim);
}

/////////////////////////////////////////////////////////////////
//
// PLSIMPLEREGIONSENSOR
//
/////////////////////////////////////////////////////////////////

// ctor default
plSimpleRegionSensor::plSimpleRegionSensor()
: fEnterMsg(nil), fExitMsg(nil)
{
}

// ctor canonical
plSimpleRegionSensor::plSimpleRegionSensor(plMessage *enterMsg, plMessage *exitMsg)
: fEnterMsg(enterMsg), fExitMsg(exitMsg)
{
}

// dtor
plSimpleRegionSensor::~plSimpleRegionSensor()
{
	if(fEnterMsg)
		fEnterMsg->UnRef();
	if(fExitMsg)
		fExitMsg->UnRef();
}

// WRITE
void plSimpleRegionSensor::Write(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Write(stream, mgr);
	if(fEnterMsg)
	{
		stream->Writebool(true);
		mgr->WriteCreatable(stream, fEnterMsg);
	} else {
		stream->Writebool(false);
	}
	if(fExitMsg)
	{
		stream->Writebool(true);
		mgr->WriteCreatable(stream, fExitMsg);
	} else {
		stream->Writebool(false);
	}
}

// READ
void plSimpleRegionSensor::Read(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Read(stream, mgr);
	if(stream->Readbool())
	{
		fEnterMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
	} else {
		fEnterMsg = nil;
	}

	if(stream->Readbool())
	{
		fExitMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
		hsAssert(fExitMsg, "Corrupted plSimpleRegionSensor during read.");
	} else {
		fExitMsg = nil;
	}
}

// MSGRECEIVE
hsBool plSimpleRegionSensor::MsgReceive(plMessage *msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		// make sure this is the local player... the notify will be the thing that propagates over the network
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
			return true;

		plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
		
		if(pCollMsg->fEntering)
		{
			if(fEnterMsg)
			{
				fEnterMsg->ClearReceivers();
				fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
				fEnterMsg->Ref();
				fEnterMsg->Send();
			}
		} 
		else {
			if(fExitMsg)
			{
				fExitMsg->ClearReceivers();
				fExitMsg->AddReceiver(theThingWhatDoneHitUs);
				fExitMsg->Ref();
				fExitMsg->Send();
			}
		}
		return true;
	}
	return plSingleModifier::MsgReceive(msg);
}

// IEVAL
hsBool plSimpleRegionSensor::IEval(double secs, hsScalar del, UInt32 dirty)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////

// Nuke the Read/Write functions on the next file format change
void plSwimDetector::Write(hsStream *stream, hsResMgr *mgr)
{
	plSimpleRegionSensor::Write(stream, mgr);

	stream->WriteByte(0);
	stream->WriteSwapScalar(0);
	stream->WriteSwapScalar(0);
}

void plSwimDetector::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimpleRegionSensor::Read(stream, mgr);

	stream->ReadByte();
	stream->ReadSwapScalar();
	stream->ReadSwapScalar();
}
hsBool plSwimDetector::MsgReceive(plMessage *msg)
{
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		//removed local player check because this will apply the brain to the local 
		//controller of the foreign avatar which we still want.
		//and if we prop swim state by notify messages we still have a chance of missing it from players
		//who were in the region before we linked in
		plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
		if(pCollMsg->fEntering)
		{
			if(fEnterMsg)
			{
				fEnterMsg->ClearReceivers();
				fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
				fEnterMsg->Ref();
				fEnterMsg->Send();
			}
		} 
		else {
		if(fExitMsg)
			{
				fExitMsg->ClearReceivers();
				fExitMsg->AddReceiver(theThingWhatDoneHitUs);
				fExitMsg->Ref();
				fExitMsg->Send();
			}
		}
		return true;
	}
	return plSimpleRegionSensor::MsgReceive(msg);
}
hsBool  plRidingAnimatedPhysicalDetector::MsgReceive(plMessage *msg)
{
	
	plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

	if (pCollMsg)
	{
		//removed local player check because this will apply the brain to the local 
		//controller of the foreign avatar which we still want.
		//and if we prop  state by notify messages we still have a chance of missing it from players
		//who were in the region before we linked in
		plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
		if(pCollMsg->fEntering)
		{
			if(fEnterMsg)
			{
				fEnterMsg->ClearReceivers();
				fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
				fEnterMsg->Ref();
				fEnterMsg->Send();
			}
		} 
		else {
		if(fExitMsg)
			{
				fExitMsg->ClearReceivers();
				fExitMsg->AddReceiver(theThingWhatDoneHitUs);
				fExitMsg->Ref();
				fExitMsg->Send();
			}
		}
		return true;
	}
	return plSimpleRegionSensor::MsgReceive(msg);
}