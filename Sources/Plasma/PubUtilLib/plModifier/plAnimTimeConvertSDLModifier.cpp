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
#include "hsTimer.h"
#include "plAnimTimeConvertSDLModifier.h"
#include "../plSDL/plSDL.h"
#include "../plInterp/plAnimTimeConvert.h"

// static vars
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrFlags[]="flags";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrLastStateAnimTime[]="lastStateAnimTime";		
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrLoopBegin[]="loopBegin";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrLoopEnd[]="loopEnd";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrSpeed[]="speed";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrCurrentEaseCurve[]="currentEaseCurve";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrCurrentEaseBeginWorldTime[]="currentEaseBeginWorldTime";
char plAnimTimeConvertSDLModifier::AnimTimeConvertVarNames::kStrLastStateChange[]="lastStateChange";

//
// Copy atcs from current state into sdl
//
void plAnimTimeConvertSDLModifier::IPutATC(plStateDataRecord* atcStateDataRec, plAnimTimeConvert* animTimeConvert)
{
	plATCState *lastState = animTimeConvert->fStates.front();
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrFlags)->Set(animTimeConvert->fFlags);		
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrLastStateAnimTime)->Set(lastState->fStartAnimTime);		
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrLoopEnd)->Set(animTimeConvert->fLoopEnd);		
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrLoopBegin)->Set(animTimeConvert->fLoopBegin);		
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrSpeed)->Set(animTimeConvert->fSpeed);		
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrLastStateChange)->Set(lastState->fStartWorldTime);	
	
	int curEaseCurve = animTimeConvert->GetCurrentEaseCurve();
	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrCurrentEaseCurve)->Set(curEaseCurve);		

	atcStateDataRec->FindVar(AnimTimeConvertVarNames::kStrCurrentEaseBeginWorldTime)->Set(curEaseCurve ?
		animTimeConvert->fCurrentEaseCurve->fBeginWorldTime : 0);		
}

//
// Apply state in SDL record to current animation state 
//
//#include "../pnSceneObject/plSceneObject.h"
void plAnimTimeConvertSDLModifier::ISetCurrentATC(const plStateDataRecord* atcStateDataRec, plAnimTimeConvert* objAtc)										
{
//	if ( GetTarget(0)->GetKeyName() && stricmp( GetTarget(0)->GetKeyName(), "RTDirLight01" )==0 )
//	{
//		int xx=0;
//	}
	if (atcStateDataRec->IsUsed())
	{
		plStateDataRecord::SimpleVarsList vars;
		int num=atcStateDataRec->GetUsedVars(&vars);
		int j;
		hsScalar lastStateAnimTime = 0;
		double lastStateChange = 0;
		for(j=0;j<num;j++)
		{
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrFlags))
			{
				int  f;
				vars[j]->Get(&f);
				objAtc->fFlags=f;
			}
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrLastStateAnimTime))
				vars[j]->Get(&lastStateAnimTime);
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrLoopBegin))
				vars[j]->Get(&objAtc->fLoopBegin);
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrLoopEnd))
				vars[j]->Get(&objAtc->fLoopEnd);
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrSpeed))
				vars[j]->Get(&objAtc->fSpeed);
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrLastStateChange))
				vars[j]->Get(&lastStateChange);
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrCurrentEaseCurve))
			{
				int ces;
				vars[j]->Get(&ces);
				if (ces == plAnimTimeConvert::kEaseSpeed)
				{
					// I don't think this ever happens in practice. If it becomes necessary,
					// we can work around it. But unless it's actually used, I don't want
					// to waste the space storing a speed ease curve.
					objAtc->SetCurrentEaseCurve(plAnimTimeConvert::kEaseNone);
				}
				else
					objAtc->SetCurrentEaseCurve(ces); // The ATC will ignore an index out of range
			}
			else
			if (vars[j]->IsNamed(AnimTimeConvertVarNames::kStrCurrentEaseBeginWorldTime) && objAtc->fCurrentEaseCurve)
				vars[j]->Get(&objAtc->fCurrentEaseCurve->fBeginWorldTime);
		}
		objAtc->IClearAllStates();		
		objAtc->IProcessStateChange(lastStateChange, lastStateAnimTime);
		objAtc->fCurrentAnimTime = lastStateAnimTime;
		objAtc->fLastEvalWorldTime = lastStateChange;
		objAtc->SetFlag(plAnimTimeConvert::kForcedMove, true);
	}
}

