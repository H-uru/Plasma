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

// singular
#include "plMultistageBehMod.h"

// local
#include "plAvBrainGeneric.h"
#include "plAnimStage.h"
#include "plArmatureMod.h"

// global
#include "hsResMgr.h"

//other
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plMultistageMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plInputCore/plAvatarInputInterface.h"

#ifdef DEBUG_MULTISTAGE
#include "plAvatarMgr.h"
#include "../plStatusLog/plStatusLog.h"
#endif

plMultistageBehMod::plMultistageBehMod() : fStages(nil), fFreezePhys(false), fSmartSeek(false), fReverseFBControlsOnRelease(false), fNetProp(true), fNetForce(false)
{
}

plMultistageBehMod::plMultistageBehMod(plAnimStageVec* stages,
									   bool freezePhys,
									   bool smartSeek,
									   bool reverseFBControlsOnRelease,
									   std::vector<plKey>* receivers)
	: fStages(stages),
	  fFreezePhys(freezePhys),
	  fSmartSeek(smartSeek),
	  fReverseFBControlsOnRelease(reverseFBControlsOnRelease),
	  fNetProp(true),
	  fNetForce(false)
{
	if (receivers)
		fReceivers = *receivers;
}

plMultistageBehMod::~plMultistageBehMod()
{
	IDeleteStageVec();
}

void plMultistageBehMod::Init(plAnimStageVec *stages,
									  bool freezePhys,
									  bool smartSeek,
									  bool reverseFBControlsOnRelease,
									  std::vector<plKey>* receivers)
{
	fStages = stages;
	fFreezePhys = freezePhys;
	fSmartSeek = smartSeek;
	fReverseFBControlsOnRelease = reverseFBControlsOnRelease;
	if (receivers)
		fReceivers = *receivers;
}

void plMultistageBehMod::IDeleteStageVec()
{
	if (fStages)
	{
		int numStages = fStages->size();
		for (int i = 0; i < numStages; i++)
		{
			plAnimStage* stage = (*fStages)[i];
			delete stage;
		}

		delete fStages;
		fStages = nil;
	}
}

hsBool plMultistageBehMod::MsgReceive(plMessage* msg)
{
	plMultistageModMsg *multiMsg = nil;
	plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(msg);
	if (notifyMsg)
	{
		hsAssert(fStages, "Trying to trigger multistage, but no stages are present.");
		if(fStages)
		{
			plKey avKey = notifyMsg->GetAvatarKey();
			hsAssert(avKey, "Avatar key missing trying to trigger multistage.");
			if(avKey)
			{
				plSceneObject *avObj = plSceneObject::ConvertNoRef(avKey->ObjectIsLoaded());
				hsAssert(avObj, "Avatar not loaded when trying to trigger multistage.");
				if(avObj)
				{
					// Create a copy of our reference anim stages to give to the brain
					plAnimStageVec* stages = TRACKED_NEW plAnimStageVec;
					int numStages = fStages->size();
					stages->reserve(numStages);
					// hack hack hack
					hsBool ladder = false;
					for (int i = 0; i < numStages; i++)
					{
						plAnimStage* stage = TRACKED_NEW plAnimStage;
						*stage = *((*fStages)[i]);
						stages->push_back(stage);
						if (strstr(stage->GetAnimName(),"adder") != nil)
							ladder = true;
					}

					const plArmatureMod *avMod = (plArmatureMod*)avObj->GetModifierByType(plArmatureMod::Index());
					hsAssert(avMod, "Missing armature mod on avatar scene object.");

					if(avMod)
					{
						plKey sender = notifyMsg->GetSender();
						plKey avModKey = avMod->GetKey();
						plKey seekKey = GetTarget()->GetKey();		// our seek point
						
#ifdef DEBUG_MULTISTAGE
						char sbuf[256];
						sprintf(sbuf,"plMultistageModMsg - starting multistage from %s",sender->GetName());
						plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
						plAvSeekMsg *seeker = TRACKED_NEW plAvSeekMsg(nil, avModKey, seekKey, 1.0f, fSmartSeek);
						seeker->Send();

						// these (currently unused) callbacks are for the brain itself, not any of the stages
						plMessage *exitCallback = nil, *enterCallback = nil;
						UInt32 exitFlags = plAvBrainGeneric::kExitNormal;

						plAvBrainGeneric *brain = TRACKED_NEW plAvBrainGeneric(stages, exitCallback, enterCallback, sender, exitFlags, 
																	   plAvBrainGeneric::kDefaultFadeIn, plAvBrainGeneric::kDefaultFadeOut, 
																	   plAvBrainGeneric::kMoveRelative);
						if (ladder)
						{
							brain->SetType(plAvBrainGeneric::kLadder);
						}
						brain->SetReverseFBControlsOnRelease(fReverseFBControlsOnRelease);
						plAvPushBrainMsg* pushBrain = TRACKED_NEW plAvPushBrainMsg(GetKey(), avModKey, brain);
						pushBrain->Send();
					}
				}
			}
		}

		return true;
	} 
	else if (multiMsg = plMultistageModMsg::ConvertNoRef(msg))
	{
		if (multiMsg->GetCommand(plMultistageModMsg::kSetLoopCount))
		{
			((*fStages)[multiMsg->fStageNum])->SetNumLoops(multiMsg->fNumLoops);
		}
		return true;
	}
	else {
		return plSingleModifier::MsgReceive(msg);
	}
}

void plMultistageBehMod::Read(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Read(stream, mgr);

	fFreezePhys = stream->Readbool();
	fSmartSeek = stream->Readbool();
	fReverseFBControlsOnRelease = stream->Readbool();

	IDeleteStageVec();
	fStages = TRACKED_NEW plAnimStageVec;
	int numStages = stream->ReadSwap32();
	fStages->reserve(numStages);

	int i;
	for (i = 0; i < numStages; i++)
	{
		plAnimStage* stage = TRACKED_NEW plAnimStage;
		stage->Read(stream, mgr);
		stage->SetMod(this);
		fStages->push_back(stage);
	}

	int numReceivers = stream->ReadSwap32();
	fReceivers.clear();
	fReceivers.reserve(numReceivers);
	for (i = 0; i < numReceivers; i++)
	{
		plKey key = mgr->ReadKey(stream);
		fReceivers.push_back(key);
	}
}

void plMultistageBehMod::Write(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->Writebool(fFreezePhys);
	stream->Writebool(fSmartSeek);
	stream->Writebool(fReverseFBControlsOnRelease);

	int numStages = fStages->size();
	stream->WriteSwap32(numStages);

	int i;
	for (i = 0; i < numStages; i++)
	{
		plAnimStage* stage = (*fStages)[i];
		stage->Write(stream, mgr);
	}

	int numReceivers = fReceivers.size();
	stream->WriteSwap32(numReceivers);
	for (i = 0; i < numReceivers; i++)
	{
		plKey key = fReceivers[i];
		mgr->WriteKey(stream, key);
	}
}
