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
#include "plResponderSDLModifier.h"
#include "../plSDL/plSDL.h"
#include "plResponderModifier.h"

// static vars
char plResponderSDLModifier::kStrCurState[]="curState";
char plResponderSDLModifier::kStrEnabled[]="enabled";
char plResponderSDLModifier::kStrCurCommand[]="curCommand";
char plResponderSDLModifier::kStrNetRequest[]="netRequest";
char plResponderSDLModifier::kStrCompletedEvents[]="completedEvents";
char plResponderSDLModifier::kStrPlayerKey[]="playerKey";
char plResponderSDLModifier::kStrTriggerer[]="triggerer";

plKey plResponderSDLModifier::GetStateOwnerKey() const
{
	return fResponder ? fResponder->GetKey() : nil;
}

//
// get current state from responder
// fill out state data rec
//
void plResponderSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	hsAssert(fResponder, "nil responder?");

	dstState->FindVar(kStrCurState)->Set((int)fResponder->fCurState);
	dstState->FindVar(kStrEnabled)->Set(fResponder->fEnabled);
	
	dstState->FindVar(kStrCurCommand)->Set((int)fResponder->fCurCommand);	
	dstState->FindVar(kStrNetRequest)->Set(fResponder->fNetRequest);

	int i;
	int num=fResponder->fCompletedEvents.GetNumBitVectors();
	dstState->FindVar(kStrCompletedEvents)->Alloc(num);
	for(i=0;i<num; i++)
	{
		int ev = fResponder->fCompletedEvents.GetBitVector(i);
		dstState->FindVar(kStrCompletedEvents)->Set(ev, i);
	}
	
	dstState->FindVar(kStrPlayerKey)->Set(fResponder->fPlayerKey);
	dstState->FindVar(kStrTriggerer)->Set(fResponder->fTriggerer);
}

//
// apply incoming state to responder
//
void plResponderSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	hsAssert(fResponder, "nil responder?");

	int curState = fResponder->fCurState;
	int curCommand = fResponder->fCurCommand;
	bool enabled = fResponder->fEnabled;
	bool netRequest = fResponder->fNetRequest;
	hsBitVector completedEvents = fResponder->fCompletedEvents;
	plKey playerKey = fResponder->fPlayerKey;
	plKey triggerer = fResponder->fTriggerer;

	plStateDataRecord::SimpleVarsList vars;
	int numVars = srcState->GetUsedVars(&vars);

	for (int i = 0; i < numVars; i++)
	{
		if (vars[i]->IsNamed(kStrCurState))
		{
			vars[i]->Get(&curState);
		}
		else if (vars[i]->IsNamed(kStrEnabled))
		{
			vars[i]->Get(&enabled);
		}
		else if (vars[i]->IsNamed(kStrCurCommand))
		{
			vars[i]->Get(&curCommand);
		}
		else if (vars[i]->IsNamed(kStrNetRequest))
		{
			vars[i]->Get(&netRequest);			
		}
		else if (vars[i]->IsNamed(kStrCompletedEvents))
		{
			int numEvents = vars[i]->GetCount();
			completedEvents.SetNumBitVectors(numEvents);

			for (int j = 0; j < numEvents; j++)
			{
				int bv;
				vars[i]->Get(&bv, j);
				completedEvents.SetBitVector(j, bv);
			}
		}
		else if (vars[i]->IsNamed(kStrPlayerKey))
		{
			vars[i]->Get(&playerKey);
		}
		else if (vars[i]->IsNamed(kStrTriggerer))
		{
			vars[i]->Get(&triggerer);
		}
		else
		{
			hsAssert(false, "Unknown var name");
		}
	}

	if (numVars)
	{
		bool stateValid = (curState >= 0 && curState < fResponder->fStates.Count());
		hsAssert(stateValid, "Received invalid responder state");
		if (!stateValid)
			return;

		bool cmdValid = curCommand == -1 || (curCommand >= 0 && curCommand < fResponder->fStates[curState].fCmds.Count());
		hsAssert(stateValid, "Received invalid responder command");
		if (!cmdValid)
			return;

		// Could try to validate the completed events, but if someone hacked that
		// all they could do is set events that we don't look at

		fResponder->fCurState = curState;
		fResponder->fCurCommand = curCommand;
		fResponder->fEnabled = enabled;
		fResponder->fNetRequest = netRequest;
		fResponder->fCompletedEvents = completedEvents;
		fResponder->fPlayerKey = playerKey;
		fResponder->fTriggerer = triggerer;

		fResponder->Restore();
	}
}


