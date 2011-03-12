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
#include "plExcludeRegionModifier.h"
#include "../plMessage/plExcludeRegionMsg.h"
#include "hsTemplates.h"
#include "hsResMgr.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "plDetectorLog.h"
// For MsgReceive
#include "../plMessage/plCollideMsg.h"
#include "../pnSceneObject/plSceneObject.h"

// For IClear and IRelease
#include "../pnMessage/plWarpMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "plPhysical.h"
#include "../plPhysical/plSimDefs.h"
#include "../plAvatar/plAvCallbackAction.h"

#include "../plAvatar/plAvBrainGeneric.h"

#include "../plSDL/plSDL.h"
#include "../pnMessage/plSDLModifierMsg.h"
//for hack
#include "../plPhysX/plPXPhysical.h"
#include "../plPhysX/plPXPhysicalControllerCore.h"
#include "NxCapsule.h"
static plPhysical* GetPhysical(plSceneObject* obj)
{
	if (obj)
	{
		const plSimulationInterface* si = obj->GetSimulationInterface();
		if (si)
			return si->GetPhysical();
	}
	return nil;
}

plExcludeRegionModifier::plExcludeRegionModifier() :
	fSDLModifier(nil),
	fSeek(false),
	fSeekTime(10.0f)
{
}


plExcludeRegionModifier::~plExcludeRegionModifier()
{
}

void plExcludeRegionModifier::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	int numPoints = stream->ReadSwap32();
	for (int i = 0; i < numPoints; i++)
	{
		fSafePoints.push_back(mgr->ReadKey(stream));
	}
	fSeek = stream->ReadBool();
	fSeekTime = stream->ReadSwapScalar();
}

void plExcludeRegionModifier::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	int numPoints = fSafePoints.size();
	stream->WriteSwap32(numPoints);
	for (int i = 0; i < numPoints; i++)
	{
		mgr->WriteKey(stream,fSafePoints[i]);
	}
	stream->WriteBool(fSeek);
	stream->WriteSwapScalar(fSeekTime);
}

void plExcludeRegionModifier::ISetPhysicalState(bool cleared)
{
	plPhysical* phys = GetPhysical(GetTarget());
	if (phys)
	{
		phys->ExcludeRegionHack(cleared);

		if (cleared)
		{
			phys->AddLOSDB(plSimDefs::kLOSDBUIBlockers);
			if (HasFlag(kBlockCameras))
				phys->AddLOSDB(plSimDefs::kLOSDBCameraBlockers);
		}
		else
		{
			phys->RemoveLOSDB(plSimDefs::kLOSDBUIBlockers);
			if (HasFlag(kBlockCameras))
				phys->RemoveLOSDB(plSimDefs::kLOSDBCameraBlockers);
		}
	}
}

hsBool plExcludeRegionModifier::MsgReceive(plMessage* msg)
{
	plExcludeRegionMsg *exclMsg = plExcludeRegionMsg::ConvertNoRef(msg);
	if (exclMsg)
	{
		if (exclMsg->GetCmd() == plExcludeRegionMsg::kClear)
		{
			DetectorLogSpecial("Clearing exclude region %s", GetKeyName());
			IMoveAvatars();
			fContainedAvatars.Reset();
			ISetPhysicalState(true);
		}
		else if (exclMsg->GetCmd() == plExcludeRegionMsg::kRelease)
		{
			DetectorLogSpecial("Releasing exclude region %s", GetKeyName());
			ISetPhysicalState(false);
		}

		GetTarget()->DirtySynchState(kSDLXRegion, exclMsg->fSynchFlags);
	
		return true;
	}

	// Avatar entering or exiting our volume.  Only the owner of the SO logs this since
	// it does all the moving at clear time.
	plCollideMsg *collideMsg = plCollideMsg::ConvertNoRef(msg);
	
	if (collideMsg)
	{
		if (collideMsg->fEntering)
		{
			//DetectorLogSpecial("Avatar enter exclude region %s",GetKeyName());
			fContainedAvatars.Append(collideMsg->fOtherKey);
		}
		else
		{
			//DetectorLogSpecial("Avatar exit exclude region %s",GetKeyName());
			int idx = fContainedAvatars.Find(collideMsg->fOtherKey);
			if (idx != fContainedAvatars.kMissingIndex)
				fContainedAvatars.Remove(idx);
		}

		return true;
	}

	return plSingleModifier::MsgReceive(msg);
}

void plExcludeRegionModifier::AddTarget(plSceneObject* so)
{
	if (so)
	{
		delete fSDLModifier;
		fSDLModifier = TRACKED_NEW plExcludeRegionSDLModifier(this);
		so->AddModifier(fSDLModifier);
	}
	plSingleModifier::SetTarget(so);
}

void plExcludeRegionModifier::RemoveTarget( plSceneObject *so )
{
	if (so && fSDLModifier)
	{
		so->RemoveModifier(fSDLModifier);
		delete fSDLModifier;
		fSDLModifier = nil;
	}
	plSingleModifier::RemoveTarget(so);
}

void plExcludeRegionModifier::AddSafePoint(plKey& key)
{
	fSafePoints.push_back(key);
}

int plExcludeRegionModifier::IFindClosestSafePoint(plKey avatar)
{
	float closestDist = 0.f;
	int closestIdx = -1;

	plSceneObject* avObj = plSceneObject::ConvertNoRef(avatar->GetObjectPtr());
	if (!avObj)
		return -1;

	hsVector3 avPos;
	avObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&avPos);

	for (int i = 0; i < fSafePoints.size(); i++)
	{
		plSceneObject* safeObj = plSceneObject::ConvertNoRef(fSafePoints[i]->GetObjectPtr());
		hsVector3 safePos;
		safeObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&safePos);

		float dist = (safePos - avPos).Magnitude();

		if (dist < closestDist || closestIdx == -1)
		{
			closestDist = dist;
			closestIdx = i;
		}
	}

	return closestIdx;
}

// check to make sure that the avatar and the exclude region are in the same subworld
bool plExcludeRegionModifier::ICheckSubworlds(plKey avatar)
{
	plSceneObject* avObj = plSceneObject::ConvertNoRef(avatar->GetObjectPtr());
	if (avObj)
	{
		// get the avatar modifier
		const plArmatureMod *avMod = (plArmatureMod*)avObj->GetModifierByType(plArmatureMod::Index());
		if (avMod)
		{
			// get the avatar controller
			plPhysicalControllerCore* avController = avMod->GetController();
			if (avController)
			{
				// get physical of the detector region
				plPhysical* phys = GetPhysical(GetTarget());
				if (phys)
				{
					// are they in the same subworld?
					if ( phys->GetWorldKey() == avController->GetSubworld() )
						return true;
					else
						return false;
				}
			}
		}
	}
	return false;
}

// Move avatars out of volume
void plExcludeRegionModifier::IMoveAvatars()
{
	//some reason this is not always finding all avatars might have something to do with
	//Physx trigger flutter. Adding in brute force method
	/*
	for (int i = 0; i < fContainedAvatars.Count(); i++)
	{
		if ( ICheckSubworlds(fContainedAvatars[i]) )
		{
			int closestIdx = IFindClosestSafePoint(fContainedAvatars[i]);

			if (closestIdx != -1)
			{
				plAvSeekMsg* msg = TRACKED_NEW plAvSeekMsg;
				msg->SetBCastFlag(plMessage::kPropagateToModifiers);
				msg->AddReceiver(fContainedAvatars[i]);
				msg->fSmartSeek = fSeek;
				msg->fDuration = fSeekTime;
				msg->fSeekPoint = fSafePoints[closestIdx];
				msg->fFlags |= plAvSeekMsg::kSeekFlagUnForce3rdPersonOnFinish;
				msg->Send();
			}
		}
	}
	*/
	
	plPXPhysical* phys =(plPXPhysical*) GetPhysical(GetTarget());
	if (phys)
	{
		plKey DetectorWorldKey = phys->GetWorldKey();
		int numControllers = plPXPhysicalControllerCore::GetNumberOfControllersInThisSubWorld(phys->GetWorldKey());
		if (numControllers > 0)
		{
			plPXPhysicalControllerCore** controllers = TRACKED_NEW plPXPhysicalControllerCore*[numControllers];

			int actualCount = plPXPhysicalControllerCore::GetControllersInThisSubWorld(phys->GetWorldKey(), numControllers, controllers);
			
			for (int i=0;i<actualCount;i++)
			{	
				NxCapsule cap;
				controllers[i]->GetWorldSpaceCapsule(cap);
				if(phys->OverlapWithCapsule(cap))
				{
					plSceneObject* so = plSceneObject::ConvertNoRef(controllers[i]->GetOwner()->ObjectIsLoaded());
					const plArmatureMod* constAvMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
					if(constAvMod)
					{
						plAvBrainGeneric *curGenBrain = (plAvBrainGeneric *)constAvMod->FindBrainByClass(plAvBrainGeneric::Index());
						// *** warning; if there's more than one generic brain active, this will only look at the first
						if (curGenBrain  && curGenBrain->GetType() == plAvBrainGeneric::kLadder)
						{
							plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg(
								nil,
								constAvMod->GetKey(),
								plAvBrainGenericMsg::kGotoStage,
								-1,
								false,
								0.0,
								false,
								false,
								0.0
							);
							pMsg->Send();
						}
						else
						{
							int closestIdx = IFindClosestSafePoint(controllers[i]->GetOwner());
							if (closestIdx != -1)
							{
								plAvSeekMsg* msg = TRACKED_NEW plAvSeekMsg;
								msg->SetBCastFlag(plMessage::kPropagateToModifiers);
								msg->AddReceiver(controllers[i]->GetOwner());
								msg->fSmartSeek = fSeek;
								msg->fDuration = fSeekTime;
								msg->fSeekPoint = fSafePoints[closestIdx];
								msg->fFlags |= plAvSeekMsg::kSeekFlagUnForce3rdPersonOnFinish;
								msg->Send();
							}
						}
					}
				}
			}

			delete[] controllers;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

plExcludeRegionSDLModifier::plExcludeRegionSDLModifier() : fXRegion(nil)
{
}

plExcludeRegionSDLModifier::plExcludeRegionSDLModifier(plExcludeRegionModifier* xregion) :
	fXRegion(xregion)
{
}

static const char* kXRegionSDLCleared = "cleared";

void plExcludeRegionSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	plSimpleStateVariable* var = dstState->FindVar(kXRegionSDLCleared);
	if (var)
	{
		plPhysical* phys = GetPhysical(GetTarget());
		if (phys)
		{
			//bool cleared = phys->GetGroup() == plSimDefs::kGroupStatic;
			bool cleared = phys->GetGroup() == plSimDefs::kGroupExcludeRegion;
			var->Set(cleared);
		}
	}
}

void plExcludeRegionSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plSimpleStateVariable* var = srcState->FindVar(kXRegionSDLCleared);
	if (var)
	{
		bool cleared;
		var->Get(&cleared);

		DetectorLogSpecial("SDL %s exclude region %s", cleared ? "clearing" : "releasing", fXRegion->GetKeyName());
		fXRegion->ISetPhysicalState(cleared);
	}
}
