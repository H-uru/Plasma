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
#include "plAGMasterSDLModifier.h"
#include "../plSDL/plSDL.h"
#include "../plInterp/plAnimTimeConvert.h"
#include "../pnSceneObject/plSceneObject.h"
#include "plAGMasterMod.h"
#include "plAGAnimInstance.h"
#include "plgDispatch.h"
#include "../pnMessage/plTimeMsg.h"
#include "hsTimer.h"
#include "../plMessage/plAnimCmdMsg.h"

// static vars
char plAGMasterSDLModifier::AGMasterVarNames::kStrAtcs[]="atcs";
char plAGMasterSDLModifier::AGMasterVarNames::kStrBlends[]="blends";

UInt32 plAGMasterSDLModifier::IApplyModFlags(UInt32 sendFlags)
{
	// ugly hack so bug light animation state isn't stored on the server
	if (stricmp(GetTarget()->GetKeyName(), "RTOmni-BugLightTest") == 0)
		return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState);
	// ditto for the KI light
	if (stricmp(GetTarget()->GetKeyName(), "RTOmniKILight") == 0)
		return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState);

	return sendFlags;
}

//
// copy blends values from current state into sdl
//
void plAGMasterSDLModifier::IPutBlends(plStateDataRecord* state, plAGMasterMod* agMaster)
{
	int numBlends = agMaster->GetNumPrivateAnimations();	// each private anim has a blend value
	plSimpleStateVariable* blendsVar = state->FindVar(AGMasterVarNames::kStrBlends);
	if (blendsVar->GetCount() != numBlends)
		blendsVar->Alloc(numBlends);

	// sdl copy
	int i;
	for(i=0;i<numBlends; i++)
	{
		blendsVar->Set((UInt8)(agMaster->GetAnimInstance(i)->GetBlend() * 255), i);
	}
}


//
// Copy atcs from current state into sdl
//
void plAGMasterSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	plSceneObject* sobj=GetTarget();
	hsAssert(sobj, "plAGMasterSDLModifier, nil target");
	
	plAGMasterMod* agMaster=IGetObjectsAGMasterMod(sobj);
	hsAssert(agMaster, "nil AGMasterMod");

	if (agMaster)
	{
		IPutBlends(dstState, agMaster);

		int numAnims = agMaster->GetNumATCAnimations();		
		plSDStateVariable* atcsVar = dstState->FindSDVar(AGMasterVarNames::kStrAtcs);
		if (atcsVar->GetCount() != numAnims)
			atcsVar->Resize(numAnims);

		// copy atcs to sdl 
		int i;
		for(i=0;i<numAnims; i++)
		{
			plStateDataRecord* atcStateDataRec = atcsVar->GetStateDataRecord(i);
			plAnimTimeConvert* animTimeConvert = agMaster->GetATCAnimInstance(i)->GetTimeConvert();

			IPutATC(atcStateDataRec, animTimeConvert);
		}
	}
}

//
// Given a scene object, find and return it's AGMasterMod
//
plAGMasterMod* plAGMasterSDLModifier::IGetObjectsAGMasterMod(plSceneObject* obj)
{
	int count = obj->GetNumModifiers();

	for (int i = 0; i < count; i++)
	{
		plAGMasterMod * avMod = const_cast<plAGMasterMod*>(plAGMasterMod::ConvertNoRef(obj->GetModifier(i)));
		if(avMod)
			return avMod;
	}

	return nil;
}

//
// Apply state in SDL record to current animation state 
//
void plAGMasterSDLModifier::ISetCurrentBlends(const plStateDataRecord* state, plAGMasterMod* objAGMaster)
{
	// Check Blends
	plSimpleStateVariable* blendsVar = state->FindVar(AGMasterVarNames::kStrBlends);
	if (blendsVar->IsUsed())
	{		
		int i;
		if (blendsVar->GetCount() != objAGMaster->GetNumPrivateAnimations())
			return; // bogus state

		for (i=0;i<blendsVar->GetCount();i++)
		{
			UInt8 blend;
			blendsVar->Get(&blend, i);
			objAGMaster->GetAnimInstance(i)->SetBlend(blend / 255.f);
		}
	}
}

//
// Change the object's animation state to reflect what is specified in the 
// stateDataRecord.
//
void plAGMasterSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plSceneObject* sobj=GetTarget();
	hsAssert(sobj, "plAGMasterSDLModifier, nil target");
	
	plAGMasterMod* objAGMaster=IGetObjectsAGMasterMod(sobj);
	hsAssert(objAGMaster, "can't find object's AGMasterSDLState");
	
	ISetCurrentBlends(srcState, objAGMaster);

	plSDStateVariable* atcsVar = srcState->FindSDVar(AGMasterVarNames::kStrAtcs);
	if (atcsVar->IsUsed())
	{
		if (objAGMaster->GetNumATCAnimations() != atcsVar->GetCount())
			return;

		int i;
		for(i=0;i<atcsVar->GetCount(); i++)
		{
			plStateDataRecord* atcStateDataRec = atcsVar->GetStateDataRecord(i);
			plAnimTimeConvert* objAtc = objAGMaster->GetATCAnimInstance(i)->GetTimeConvert();	// dst					
			ISetCurrentATC(atcStateDataRec, objAtc);
			objAtc->EnableCallbacks(false);
		}
		objAGMaster->IRegForEval(objAGMaster->HasRunningAnims());

		// Force one eval, then re-enable all the callbacks
		double time = (hsTimer::GetSysSeconds() - hsTimer::GetDelSysSeconds());

		if (objAGMaster->fIsGrouped && objAGMaster->fMsgForwarder)
		{
			hsScalar animTimeFromWorldTime = (objAGMaster->GetNumATCAnimations() > 0) ? objAGMaster->GetATCAnimInstance(0)->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) : 0.0f;

			plAGCmdMsg *msg = TRACKED_NEW plAGCmdMsg();
			msg->SetCmd(plAGCmdMsg::kSetAnimTime);
			msg->fAnimTime = animTimeFromWorldTime;
			msg->AddReceiver(objAGMaster->fMsgForwarder->GetKey());
			plgDispatch::MsgSend(msg);
		}
		else
		{
			objAGMaster->AdvanceAnimsToTime(time);
		}

		for (i = 0; i < objAGMaster->GetNumATCAnimations(); i++)
			objAGMaster->GetATCAnimInstance(i)->GetTimeConvert()->EnableCallbacks(true);
	}
}

