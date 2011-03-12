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
#include "plModifier.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../pnMessage/plTimeMsg.h"

plModifier::plModifier()
{
}

plModifier::~plModifier()
{
}

plDrawInterface* plModifier::IGetTargetDrawInterface(int iTarg) const
{ 
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileDrawInterface() : nil; 
}

plSimulationInterface* plModifier::IGetTargetSimulationInterface(int iTarg) const
{ 
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileSimulationInterface() : nil; 
}

plCoordinateInterface* plModifier::IGetTargetCoordinateInterface(int iTarg) const
{ 
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileCoordinateInterface() : nil; 
}

plAudioInterface* plModifier::IGetTargetAudioInterface(int iTarg) const
{ 
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileAudioInterface() : nil; 
}

plObjInterface* plModifier::IGetTargetGenericInterface(int iTarg, UInt32 classIdx) const
{
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileGenericInterface((UInt16)classIdx) : nil; 
}

plModifier* plModifier::IGetTargetModifier(int iTarg, int iMod) const
{ 
	return GetTarget(iTarg) ? GetTarget(iTarg)->GetVolatileModifier(iMod) : nil; 
}

hsBool plModifier::MsgReceive(plMessage* msg)
{
	plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
	if( eval )
	{
		UInt32 dirty = ~0L;
		IEval(eval->DSeconds(), eval->DelSeconds(), dirty);
		return true;
	}

	return plSynchedObject::MsgReceive(msg);
}

