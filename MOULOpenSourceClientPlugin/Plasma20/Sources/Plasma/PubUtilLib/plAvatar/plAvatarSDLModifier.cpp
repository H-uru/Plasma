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
#include "plAvatarSDLModifier.h"
#include "plArmatureMod.h"

#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainGeneric.h"
#include "../plAvatar/plAvBrainClimb.h"
#include "../plAvatar/plAvBrainDrive.h"
#include "../plAvatar/plAnimStage.h"
#include "../plAvatar/plAvCallbackAction.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../plSDL/plSDL.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../pnAsyncCore/pnAsyncCore.h"

// static vars
char	plAvatarPhysicalSDLModifier::kStrPosition[] = "position";
char	plAvatarPhysicalSDLModifier::kStrRotation[] = "rotation";
char	plAvatarPhysicalSDLModifier::kStrSubworld[] = "subworld";

char	plAvatarSDLModifier::kStrBrainStack[] = "brainStack";
char	plAvatarSDLModifier::kStrInvisibilityLevel[] = "invisibilityLevel";

char	plAvatarSDLModifier::StandardStageVarNames::kStrName[]="name";
char	plAvatarSDLModifier::StandardStageVarNames::kStrNumLoops[]="numLoops";
char	plAvatarSDLModifier::StandardStageVarNames::kStrForward[]="forward";
char	plAvatarSDLModifier::StandardStageVarNames::kStrBackward[]="backward";
char	plAvatarSDLModifier::StandardStageVarNames::kStrStageAdvance[]="stageAdvance";
char	plAvatarSDLModifier::StandardStageVarNames::kStrStageRegress[]="stageRegress";
char	plAvatarSDLModifier::StandardStageVarNames::kStrNotifyEnter[]="notifyEnter";
char	plAvatarSDLModifier::StandardStageVarNames::kStrNotifyLoop[]="notifyLoop";
char	plAvatarSDLModifier::StandardStageVarNames::kStrNotifyStageAdvance[]="notifyStageAdvance";
char	plAvatarSDLModifier::StandardStageVarNames::kStrNotifyStageRegress[]="notifyStageRegress";
char	plAvatarSDLModifier::StandardStageVarNames::kStrUseGlobalCoords[]="useGlobalCoords";
char	plAvatarSDLModifier::StandardStageVarNames::kStrLocalTime[]="localTime";
char	plAvatarSDLModifier::StandardStageVarNames::kStrCurrentLoop[]="currentLoop";
char	plAvatarSDLModifier::StandardStageVarNames::kStrIsAttached[]="isAttached";

char	plAvatarSDLModifier::GenericBrainVarNames::kStrStages[]="stages";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrCurrentStage[]="currentStage";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrCallbackRcvr[]="callbackRcvr";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrMovingForward[]="movingForward";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrExitFlags[]="exitFlags";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrType[]="type";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrMode[]="mode";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrFadeIn[]="fadeIn";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrFadeOut[]="fadeOut";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrMoveMode[]="moveMode";
char	plAvatarSDLModifier::GenericBrainVarNames::kStrBodyUsage[]="bodyUsage";

char	plAvatarSDLModifier::ClimbBrainVarNames::kStrCurMode[]="curMode";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrNextMode[]="nextMode";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDirections[]="allowedDirections";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDismounts[]="allowedDismounts";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrVertProbeLength[]="vertProbeLength";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrHorizProbeLength[]="horizProbeLength";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageAttached[]="curStageAttached";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage[]="curStage";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageTime[]="curStageTime";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageStrength[]="curStageStrength";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageAttached[]="exitStageAttached";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStage[]="exitStage";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageTime[]="exitStageTime";
char	plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageStrength[]="exitStageStrength";

char	plAvatarSDLModifier::BrainUnionVarNames::kGenericBrain[]="fGenericBrain";
char	plAvatarSDLModifier::BrainUnionVarNames::kClimbBrain[]="fClimbBrain";
char	plAvatarSDLModifier::BrainUnionVarNames::kDriveBrain[]="fDriveBrain";

void plAvatarPhysicalSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plSceneObject* sObj = GetTarget();
	hsAssert(sObj, "plAvatarPhysicalSDLModifier, nil target");
	if(sObj)
	{
		const plArmatureMod* kAvMod = (plArmatureMod*)sObj->GetModifierByType(plArmatureMod::Index());
		plArmatureMod * avMod = const_cast<plArmatureMod *>(kAvMod);
		if(avMod && !avMod->GetCurrentBrain()->IsRunningTask())
		{
			plAvBrainGeneric* genBrain = plAvBrainGeneric::ConvertNoRef(avMod->GetCurrentBrain());
			if (genBrain && (genBrain->GetType() == plAvBrainGeneric::kLadder || genBrain->GetType() == plAvBrainGeneric::kSit || genBrain->GetType() == plAvBrainGeneric::kSitOnGround))
				return;

			plSimpleStateVariable* worldVar = srcState->FindVar(kStrSubworld);
			if (worldVar->IsDirty() && avMod->fController)
			{
				plKey worldKey;
				worldVar->Get(&worldKey);
				avMod->fController->SetSubworld(worldKey);
			}

			plSimpleStateVariable* rotVar = srcState->FindVar(kStrRotation);
			plSimpleStateVariable* posVar = srcState->FindVar(kStrPosition);
			if ((rotVar->IsDirty() || posVar->IsDirty()) && avMod->GetController())
			{
				hsPoint3 pos;
				hsScalar zRot;
				posVar->Get(&pos.fX);
				rotVar->Get(&zRot);
				avMod->GetController()->SetState(pos, zRot);
			}
		}
	}
}

void plAvatarPhysicalSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	plSceneObject* sObj = GetTarget();
	hsAssert(sObj, "plAvatarPhysicalSDLModifier, nil target");
	const plArmatureMod* kAvMod = (plArmatureMod*)sObj->GetModifierByType(plArmatureMod::Index());
	plArmatureMod * avMod = const_cast<plArmatureMod *>(kAvMod);
	hsAssert(avMod, "nil avMod");

	if(avMod && avMod->GetController())
	{
		hsPoint3 pos;
		hsScalar zRot;
		avMod->GetController()->GetState(pos, zRot);
		dstState->FindVar(kStrRotation)->Set(zRot);
		dstState->FindVar(kStrPosition)->Set(&pos.fX);
		dstState->FindVar(kStrSubworld)->Set(avMod->GetWorldKey());
	}
}

// ISetCurrentStateFrom ---------------------------------------------------------
void plAvatarSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plSceneObject* sObj = GetTarget();
	hsAssert(sObj, "plAvatarSDLModifier, nil target");
	if(sObj)
	{
		const plArmatureMod* kAvMod = (plArmatureMod*)sObj->GetModifierByType(plArmatureMod::Index());
		plArmatureMod * avMod = const_cast<plArmatureMod *>(kAvMod);
		if(avMod)
		{
			ISetBaseAvatarStateFrom(avMod, srcState);
			
			// right now we're just going to destroy all the non-standard brains
			// and rebuild them from the incoming synch state
			// This is acceptable because we should only receive this state when
			// we're loading another player for the first time after entering an age.
			int brainCount = avMod->GetBrainCount();
			if(brainCount > 1)
			{
				// remove all non-default brains
				for(int i = 0; i < brainCount - 1; i++)
				{
					plArmatureBrain* current = avMod->GetCurrentBrain();
					avMod->PopBrain();
					delete current;
				}
			}
			
			plSDStateVariable* brainsVar = srcState->FindSDVar(kStrBrainStack);
			if (brainsVar->IsUsed())
			{
				int nBrains = brainsVar->GetCount();
				for(int i = 0; i < nBrains; i++)
				{
					// get the record containing the brain union structure
					plStateDataRecord* brainUnion = brainsVar->GetStateDataRecord(i);
					
					// get the (one element) generic brain list, if populated
					plSDStateVariable *genBrainListVar = brainUnion->FindSDVar(BrainUnionVarNames::kGenericBrain);
					if(genBrainListVar->IsUsed() && genBrainListVar->GetCount() > 0)
					{
						// get the state for the generic brain from the list (there's only allowed to be one per list
						plStateDataRecord *genBrainVar = genBrainListVar->GetStateDataRecord(0);
						ISetGenericBrainFrom(avMod, genBrainVar);
					} else {
						// get the (one element) climb brain list, if populated
						plSDStateVariable *climbBrainListVar = brainUnion->FindSDVar(BrainUnionVarNames::kClimbBrain);
						if(climbBrainListVar->IsUsed() && climbBrainListVar->GetCount() > 0)
						{
							// TEMP: Not read/writing the climb brain until it gets an overhaul
							// get the state for the climb brain from the list (there's only allowed to be one per list
							//plStateDataRecord *climbBrainVar = climbBrainListVar->GetStateDataRecord(0);
							//ISetClimbBrainFrom(avMod, climbBrainVar);
						} else {
							// get the (one element) drive brain list, if populated
							plSDStateVariable *driveBrainListVar = brainUnion->FindSDVar(BrainUnionVarNames::kDriveBrain);
							if(driveBrainListVar->IsUsed() && driveBrainListVar->GetCount() > 0)
							{
								// get the state for the drive brain from the list (there's only allowed to be one per list
								plStateDataRecord *driveBrainVar = driveBrainListVar->GetStateDataRecord(0);
								ISetDriveBrainFrom(avMod, driveBrainVar);
							}
						}
					}
				}
			}
		}
	}
}

// IPutCurrentStateIn ---------------------------------------------------
// ------------------
void plAvatarSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	plSceneObject* sObj = GetTarget();
	hsAssert(sObj, "plAvatarSDLModifier, nil target");
	const plArmatureMod* kAvMod = (plArmatureMod*)sObj->GetModifierByType(plArmatureMod::Index());
	plArmatureMod * avMod = const_cast<plArmatureMod *>(kAvMod);
	hsAssert(avMod, "nil avMod");

	if(avMod)
	{
		IPutBaseAvatarStateIn(avMod, dstState);
		// create a brainUnion nested record
		plSDStateVariable* brainUnionArray = dstState->FindSDVar(plAvatarSDLModifier::kStrBrainStack);
		int nBrains = avMod->GetBrainCount();
		brainUnionArray->Resize(nBrains - 1);		// we skip the base brain
		
		for (int i = 1; i < nBrains; i++)
		{
			plStateDataRecord *brainUnion = brainUnionArray->GetStateDataRecord(i - 1);

			// create (or get) a nested array of climbBrain records (we'll only populate the first)
			plSDStateVariable* climbStateVar = brainUnion->FindSDVar(plAvatarSDLModifier::BrainUnionVarNames::kClimbBrain);
			climbStateVar->Resize(0);

			// create (or get) a nested array of genbrain records (we'll only populate the first)
			plSDStateVariable *genStateVar = brainUnion->FindSDVar(plAvatarSDLModifier::BrainUnionVarNames::kGenericBrain);
			genStateVar->Resize(0);

			// create (or get) a nested array of driveBrain records (we'll only populate the first)
			plSDStateVariable* driveStateVar = brainUnion->FindSDVar(plAvatarSDLModifier::BrainUnionVarNames::kDriveBrain);
			driveStateVar->Resize(0);

			plArmatureBrain *brain = avMod->GetBrain(i);
			if (plAvBrainClimb *climbBrain = plAvBrainClimb::ConvertNoRef(brain))
			{
				// TEMP: Not read/writing the climb brain until it gets an overhaul
				//climbStateVar->Resize(1);
				//IPutClimbBrainIn(avMod, climbBrain, climbStateVar->GetStateDataRecord(0));
			}
			else
			if(plAvBrainGeneric *genBrain = plAvBrainGeneric::ConvertNoRef(brain))
			{
				genStateVar->Resize(1);
				// put the brain into the first slot in the array
				IPutGenericBrainIn(avMod, genBrain, genStateVar->GetStateDataRecord(0));
			}
			else 
			if (plAvBrainDrive *driveBrain = plAvBrainDrive::ConvertNoRef(brain))
			{
				driveStateVar->Resize(1);
				IPutDriveBrainIn(avMod, driveBrain, driveStateVar->GetStateDataRecord(0));
			}
		}		
	}
}

void plAvatarSDLModifier::IPutClimbBrainIn(plArmatureMod *avMod, plAvBrainClimb *brain, plStateDataRecord* dstState)
{
	brain->SaveToSDL(dstState);
}

void plAvatarSDLModifier::ISetClimbBrainFrom(plArmatureMod *avMod, const plStateDataRecord* srcState)
{
	plAvBrainClimb *climbBrain = TRACKED_NEW plAvBrainClimb();
	avMod->PushBrain(climbBrain);
	climbBrain->LoadFromSDL(srcState);
}

void plAvatarSDLModifier::IPutDriveBrainIn(plArmatureMod *avMod, plAvBrainDrive *brain, plStateDataRecord* dstState)
{
	dstState->FindVar("unUsed")->Set(0);
}

void plAvatarSDLModifier::ISetDriveBrainFrom(plArmatureMod *avMod, const plStateDataRecord* src)
{
	plAvBrainDrive *driveBrain = TRACKED_NEW plAvBrainDrive();
	avMod->PushBrain(driveBrain);
}

// IPutGenericBrainIn --------------------------------------------------------------------------
void plAvatarSDLModifier::IPutGenericBrainIn(plArmatureMod * avMod, plAvBrainGeneric *genBrain, plStateDataRecord* dstState)
{
	// get state of the brain:
	dstState->FindVar(GenericBrainVarNames::kStrCurrentStage)->Set(genBrain->GetCurStageNum());
	dstState->FindVar(GenericBrainVarNames::kStrMovingForward)->Set(genBrain->GetForward());
	dstState->FindVar(GenericBrainVarNames::kStrCallbackRcvr)->Set(genBrain->GetRecipient());
	dstState->FindVar(GenericBrainVarNames::kStrExitFlags)->Set((int)genBrain->GetExitFlags());
	
	dstState->FindVar(GenericBrainVarNames::kStrType)->Set((int)genBrain->GetType());
	dstState->FindVar(GenericBrainVarNames::kStrMode)->Set((int)genBrain->GetMode());
	dstState->FindVar(GenericBrainVarNames::kStrFadeIn)->Set(genBrain->GetFadeIn());
	dstState->FindVar(GenericBrainVarNames::kStrFadeOut)->Set(genBrain->GetFadeOut());
	dstState->FindVar(GenericBrainVarNames::kStrMoveMode)->Set((int)genBrain->GetMoveMode());
	dstState->FindVar(GenericBrainVarNames::kStrBodyUsage)->Set((int)genBrain->GetBodyUsage());

	// let's fill in the stages
	int stageCount = genBrain->GetStageCount();
	plSDStateVariable* stagesVar = dstState->FindSDVar(GenericBrainVarNames::kStrStages);
	stagesVar->Resize(stageCount);

	for(int i = 0; i < stageCount; i++)
	{
		plAnimStage *stage = genBrain->GetStage(i);
		IPutStageIn(avMod, stage, stagesVar->GetStateDataRecord(i));
	}
}

// ISetGenericBrainFrom -------------------------------------------------------------------------------
bool plAvatarSDLModifier::ISetGenericBrainFrom(plArmatureMod *avMod, const plStateDataRecord* srcState)
{
	int i;
	hsBool success = true;
	int numStages=0;
	plSDStateVariable* stagesVar = srcState->FindSDVar(GenericBrainVarNames::kStrStages);
	if (stagesVar->IsUsed())
	{
		numStages = stagesVar->GetCount();
		if (!numStages)
			return false;
	}
	
	plAnimStageVec * stages = TRACKED_NEW plAnimStageVec();
	for (int j = 0; j < numStages; j++)
	{
		plStateDataRecord* stageState = stagesVar->GetStateDataRecord(j);
		plAnimStage *newStage = IGetStageFrom(avMod, stageState);
		if (!newStage)
			success = false;

		stages->push_back(newStage);
	}

	int curStage;
	srcState->FindVar(GenericBrainVarNames::kStrCurrentStage)->Get(&curStage);
	if (curStage >= numStages)
		success = false;

	plKey callbackRcvr;
	srcState->FindVar(GenericBrainVarNames::kStrCallbackRcvr)->Get(&callbackRcvr);

	int exitFlags;
	srcState->FindVar(GenericBrainVarNames::kStrExitFlags)->Get(&exitFlags);
	if (exitFlags >= plAvBrainGeneric::kExitMaxFlag)
		success = false;

	int type;
	srcState->FindVar(GenericBrainVarNames::kStrType)->Get(&type);
	if (type >= plAvBrainGeneric::kNumBrainTypes)
		success = false;

	int mode;
	srcState->FindVar(GenericBrainVarNames::kStrMode)->Get(&mode);
	if (mode >= plAvBrainGeneric::kMaxMode)
		success = false;

	float fadeIn, fadeOut;
	srcState->FindVar(GenericBrainVarNames::kStrFadeIn)->Get(&fadeIn);
	srcState->FindVar(GenericBrainVarNames::kStrFadeOut)->Get(&fadeOut);

	int moveMode;
	srcState->FindVar(GenericBrainVarNames::kStrMoveMode)->Get(&moveMode);
	if (moveMode >= plAvBrainGeneric::kMaxMoveMode)
		success = false;

	int bodyUsage;
	srcState->FindVar(GenericBrainVarNames::kStrBodyUsage)->Get(&bodyUsage);
	if (bodyUsage >= plAGAnim::kBodyMax)
		success = false;

	if (!success)
	{
		for (i = 0; i < numStages; i++)
		{
			plAnimStage *s = (*stages)[i];
			(*stages)[i] = nil;
			delete s;
		}
		delete stages;
		return false;
	}

	plAvBrainGeneric *newBrain =
		TRACKED_NEW plAvBrainGeneric(stages,
							 nil, nil,
							 callbackRcvr,
							 exitFlags,
							 fadeIn,
							 fadeOut,
							 static_cast<plAvBrainGeneric::MoveMode>(moveMode));
	newBrain->SetType(static_cast<plAvBrainGeneric::BrainType>(type));
	newBrain->SetBodyUsage(static_cast<plAGAnim::BodyUsage>(bodyUsage));
	newBrain->SetCurStageNum(curStage);
	
	avMod->PushBrain(newBrain);

	return true;
}

// IGetStageFrom ----------------------------------------------------------------------------------------
plAnimStage * plAvatarSDLModifier::IGetStageFrom(plArmatureMod *avMod, const plStateDataRecord* srcState)
{
	UInt32 notifyFlags=0;
	
	bool notifyEnter, notifyLoop, notifyAdv, notifyRegress;
	if (srcState->FindVar(StandardStageVarNames::kStrNotifyEnter)->Get(&notifyEnter))
		if(notifyEnter)
			notifyFlags &= plAnimStage::kNotifyEnter;

	if (srcState->FindVar(StandardStageVarNames::kStrNotifyLoop)->Get(&notifyLoop))
		if(notifyLoop)
			notifyFlags &= plAnimStage::kNotifyLoop;

	if (srcState->FindVar(StandardStageVarNames::kStrNotifyStageAdvance)->Get(&notifyAdv))
		if(notifyAdv)
			notifyFlags &= plAnimStage::kNotifyAdvance;

	if (srcState->FindVar(StandardStageVarNames::kStrNotifyStageRegress)->Get(&notifyRegress))
		if(notifyRegress)
			notifyFlags &= plAnimStage::kNotifyRegress;

	bool useLocal;
	if (srcState->FindVar(StandardStageVarNames::kStrUseGlobalCoords)->Get(&useLocal))
		useLocal = !useLocal;

	char name[32];
	srcState->FindVar(StandardStageVarNames::kStrName)->Get(name);
	plAGAnim *anim = avMod->FindCustomAnim(name);
	if (!anim)
		return nil;

	int fwd, bwd, adv, reg;
	srcState->FindVar(StandardStageVarNames::kStrForward)->Get(&fwd);
	if (fwd >= plAnimStage::kForwardMax)
		return nil;

	srcState->FindVar(StandardStageVarNames::kStrBackward)->Get(&bwd);
	if (bwd >= plAnimStage::kBackMax)
		return nil;
	
	srcState->FindVar(StandardStageVarNames::kStrStageAdvance)->Get(&adv);
	if (adv >= plAnimStage::kAdvanceMax)
		return nil;

	srcState->FindVar(StandardStageVarNames::kStrStageRegress)->Get(&reg);
	if (reg >= plAnimStage::kRegressMax)
		return nil;
	
	int numLoops;
	srcState->FindVar(StandardStageVarNames::kStrNumLoops)->Get(&numLoops);

	float localTime;
	srcState->FindVar(StandardStageVarNames::kStrLocalTime)->Get(&localTime);
	
	int curLoop;
	srcState->FindVar(StandardStageVarNames::kStrCurrentLoop)->Get(&curLoop);
	if (curLoop > numLoops && numLoops > 0) // numLoops == -1 means infinite looping
		return nil;
	
	bool isAttached;
	srcState->FindVar(StandardStageVarNames::kStrIsAttached)->Get(&isAttached);
	
	// ***!!! need to capture "advanceTo" and "regressTo" values!!!
	bool hackAdvanceToWrong = false, hackRegressToWrong = false;
	plAnimStage *newStage = TRACKED_NEW plAnimStage(name,
											(UInt8)notifyFlags,
											static_cast<plAnimStage::ForwardType>(fwd),
											static_cast<plAnimStage::BackType>(bwd),
											static_cast<plAnimStage::AdvanceType>(adv),
											static_cast<plAnimStage::RegressType>(reg),
											numLoops, hackAdvanceToWrong, 0, hackRegressToWrong, 0);

	newStage->SetLocalTime(localTime);
	newStage->SetLoopValue(curLoop);
	newStage->SetIsAttached(isAttached);

	return newStage;
}

// IPutStageIn ---------------------------------------------------------------------------------------------
bool plAvatarSDLModifier::IPutStageIn(plArmatureMod *avMod, plAnimStage *stage, plStateDataRecord* dstState)
{
	if(stage)
	{
		dstState->FindVar(StandardStageVarNames::kStrName)->Set(stage->GetAnimName());
		dstState->FindVar(StandardStageVarNames::kStrNumLoops)->Set(stage->GetNumLoops());
		dstState->FindVar(StandardStageVarNames::kStrForward)->Set((int)stage->GetForwardType());
		dstState->FindVar(StandardStageVarNames::kStrBackward)->Set((int)stage->GetBackType());
		dstState->FindVar(StandardStageVarNames::kStrStageAdvance)->Set((int)stage->GetAdvanceType());
		dstState->FindVar(StandardStageVarNames::kStrStageRegress)->Set((int)stage->GetRegressType());
		
		UInt32 notifies = stage->GetNotifyFlags();
		dstState->FindVar(StandardStageVarNames::kStrNotifyEnter)->Set((notifies & plAnimStage::kNotifyEnter) ? true : false);
		dstState->FindVar(StandardStageVarNames::kStrNotifyLoop)->Set((notifies & plAnimStage::kNotifyLoop) ? true : false);
		dstState->FindVar(StandardStageVarNames::kStrNotifyStageAdvance)->Set((notifies & plAnimStage::kNotifyAdvance) ? true : false);
		dstState->FindVar(StandardStageVarNames::kStrNotifyStageRegress)->Set((notifies & plAnimStage::kNotifyRegress) ? true : false);
		
// XXX		dstState->FindVar(StandardStageVarNames::kStrUseGlobalCoords)->Set(stage->GetIsLocal());
		dstState->FindVar(StandardStageVarNames::kStrLocalTime)->Set(stage->GetLocalTime());

		//dstState->FindVar(StandardStageVarNames::kStrLength)->Set(0.0f);			// NOT CURRENTLY USED
		// Well, if we're not using it, why are we still dirtying it and sending it across? Commented out.

		dstState->FindVar(StandardStageVarNames::kStrCurrentLoop)->Set(stage->GetLoopValue());
		dstState->FindVar(StandardStageVarNames::kStrIsAttached)->Set(stage->GetIsAttached());
		return true;
	}

	return false;
}

// IPutBaseAvatarStateIn -------------------------------------------------------------------------
void plAvatarSDLModifier::IPutBaseAvatarStateIn(plArmatureMod *avMod, plStateDataRecord* dstState)
{
	if (avMod->GetStealthLevel() > 0)
		LogMsg(kLogDebug, "plAvatarSDLModifier::IPutBaseAvatarStateIn - Stealth level being set greater than zero");
	dstState->FindVar(kStrInvisibilityLevel)->Set(avMod->GetStealthLevel());
}

// ISetBaseAvatarStateFrom -------------------------------------------------------------------------------
void plAvatarSDLModifier::ISetBaseAvatarStateFrom(plArmatureMod *avMod, const plStateDataRecord* srcState)
{
	// invisibility
	if (!avMod->IsLocallyOwned())
	{
		int invisLevel;
		srcState->FindVar(kStrInvisibilityLevel)->Get(&invisLevel);

		if (invisLevel > 0)
			LogMsg(kLogDebug, "plAvatarSDLModifier::ISetBaseAvatarStateFrom - Stealth level greater than zero");
		plNetClientMgr::GetInstance()->MakeCCRInvisible(avMod->GetTarget(0)->GetKey(), invisLevel);
	}
}

