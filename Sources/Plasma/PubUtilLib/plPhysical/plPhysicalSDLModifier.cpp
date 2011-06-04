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
#include "plPhysicalSDLModifier.h"

#include "hsGeometry3.h"
#include "plPhysical.h"
#include "../plSDL/plSDL.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnNetCommon/plNetApp.h"
#include "hsQuat.h"
//#include "../plHavok1/plSimulationMgr.h"
#include "../plStatusLog/plStatusLog.h"

// static vars
static const char* kStrLinear		= "linear";
static const char* kStrAngular		= "angular";
static const char* kStrPosition		= "position";
static const char* kStrOrientation	= "orientation";

int plPhysicalSDLModifier::fLogLevel = 0;

static void IGetVars(plStateDataRecord::SimpleVarsList& vars,
	hsPoint3& pos, bool& isPosSet,
	hsQuat& rot, bool& isRotSet,
	hsVector3& linV, bool& isLinVSet,
	hsVector3& angV, bool& isAngVSet);

//
// get current state from physical
// fill out state data rec
//
void plPhysicalSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	plPhysical* phys = IGetPhysical();

	// get latest state
	hsPoint3 curPos;
	hsQuat curOrientation;
	hsVector3 curLinear, curAngular;
	phys->GetSyncState(curPos, curOrientation, curLinear, curAngular);

	// put it in sdl state record
	dstState->FindVar(kStrPosition)->Set(&curPos.fX);
	dstState->FindVar(kStrOrientation)->Set(&curOrientation.fX);
	dstState->FindVar(kStrLinear)->Set(&curLinear.fX);
	dstState->FindVar(kStrAngular)->Set(&curAngular.fX);

	if (fLogLevel > 1)
		ILogState(dstState, false, "PUT", plStatusLog::kWhite);
}

void plPhysicalSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plPhysical* phys = IGetPhysical();
	
	// FIXME PHYSX

// 	if(phys->GetBody()->isFixed())
// 	{
// 		plSimulationMgr::Log("Received synch for fixed body %s", phys->GetKey()->GetName());
// 		return;
// 	}
// 	else if (phys->GetProperty(plSimulationInterface::kPinned))
// 	{
// 		// This is mainly intended for avatars. When pinning them (like in a multistage),
// 		// we don't want physical updates to sneak in due to network lag. If necessary,
// 		// this could be made a separate property on the physical, orthagonal to kPinned.
// 		return;
// 	}
// 	else
	{
		hsPoint3 pos;
		bool isPosSet;
		hsQuat rot;
		bool isRotSet;
		hsVector3 linV;
		bool isLinVSet;
		hsVector3 angV;
		bool isAngVSet;

		plStateDataRecord::SimpleVarsList vars;
		srcState->GetUsedVars(&vars);
		IGetVars(vars, pos, isPosSet, rot, isRotSet, linV, isLinVSet, angV, isAngVSet);

		if (fLogLevel > 0)
			ILogState(srcState, false, "RCV", plStatusLog::kGreen);

		phys->SetSyncState(
			isPosSet ? &pos : nil,
			isRotSet ? &rot : nil,
			isLinVSet ? &linV : nil,
			isAngVSet ? &angV : nil);
	}
}

void plPhysicalSDLModifier::ISentState(const plStateDataRecord* sentState)
{
	if (fLogLevel > 0)
	{
		ILogState(sentState, true, "SND", plStatusLog::kYellow);

//		plPhysical* phys = IGetPhysical();
// 		if (!phys->GetBody()->isActive())
// 			IGetLog()->AddLineF("Phys %s sent state because it deactivated", phys->GetKeyName());
	}
}

static void IGetVars(plStateDataRecord::SimpleVarsList& vars,
	hsPoint3& pos, bool& isPosSet,
	hsQuat& rot, bool& isRotSet,
	hsVector3& linV, bool& isLinVSet,
	hsVector3& angV, bool& isAngVSet)
{
	isPosSet = false;
	isRotSet = false;
	isLinVSet = false;
	isAngVSet = false;

	int num = vars.size();
	for (int i = 0; i < num; i++)
	{
		if (vars[i]->IsNamed(kStrPosition))
		{
			vars[i]->Get(&pos.fX);
			isPosSet= true;
		}
		else
		if (vars[i]->IsNamed(kStrOrientation))
		{
			vars[i]->Get(&rot.fX);
			isRotSet = true;
		}
		else
		if (vars[i]->IsNamed(kStrLinear))
		{
			vars[i]->Get(&linV.fX);
			isLinVSet = true;
		}
		else
		if (vars[i]->IsNamed(kStrAngular))
		{
			vars[i]->Get(&angV.fX);
			isAngVSet = true;
		}
		else
		if (vars[i]->IsNamed("subworld"))
		{
			// Unused
		}
		else
		{
			hsAssert(false, "Unknown var name");
		}
	}
}

void plPhysicalSDLModifier::ILogState(const plStateDataRecord* state, bool useDirty, const char* prefix, UInt32 color)
{
	hsPoint3 pos;
	bool isPosSet;
	hsQuat rot;
	bool isRotSet;
	hsVector3 linV;
	bool isLinVSet;
	hsVector3 angV;
	bool isAngVSet;

	plStateDataRecord::SimpleVarsList vars;
	if (useDirty)
		state->GetDirtyVars(&vars);
	else
		state->GetUsedVars(&vars);

	IGetVars(vars, pos, isPosSet, rot, isRotSet, linV, isLinVSet, angV, isAngVSet);

	plPhysical* phys = IGetPhysical();

	std::string log = xtl::format("%s: %s", phys->GetKeyName(), prefix);

	if (isPosSet)
		log += xtl::format(" Pos=%.1f %.1f %.1f", pos.fX, pos.fY, pos.fZ);
	else
		log += " Pos=None";

	if (isLinVSet)
		log += xtl::format(" LinV=%.1f %.1f %.1f", linV.fX, linV.fY, linV.fZ);
	else
		log += " LinV=None";

	if (isAngVSet)
		log += xtl::format(" AngV=%.1f %.1f %.1f", angV.fX, angV.fY, angV.fZ);
	else
		log += " AngV=None";

	if (isRotSet)
		log += xtl::format(" Rot=%.1f %.1f %.1f %.1f", rot.fX, rot.fY, rot.fZ, rot.fW);
	else
		log += " Rot=None";

	IGetLog()->AddLine(log.c_str(), color);
}

plStatusLog* plPhysicalSDLModifier::IGetLog()
{
	static plStatusLog* gLog = nil;
	if (!gLog)
	{
		gLog = plStatusLogMgr::GetInstance().CreateStatusLog(20, "PhysicsSDL.log",
					plStatusLog::kFilledBackground |
					plStatusLog::kTimestamp |
					plStatusLog::kDeleteForMe |
					plStatusLog::kAlignToTop);
	}

	return gLog;
}

plPhysical* plPhysicalSDLModifier::IGetPhysical()
{
	plPhysical* phys = nil;

	plSceneObject* sobj = GetTarget();
	if (sobj)
	{
		const plSimulationInterface* si = sobj->GetSimulationInterface();
		if (si)
			phys = si->GetPhysical();
	}

	hsAssert(phys, "nil hkPhysical");
	return phys;
}