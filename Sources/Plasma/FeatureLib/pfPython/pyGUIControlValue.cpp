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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"

#include "../pfGameGUIMgr/pfGUIValueCtrl.h"
#include "../pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "../pfGameGUIMgr/pfGUIUpDownPairMod.h"
#include "../pfGameGUIMgr/pfGUIProgressCtrl.h"

#include "pyGUIControlValue.h"

pyGUIControlValue::pyGUIControlValue(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlValue::pyGUIControlValue(plKey objkey) : pyGUIControl(objkey)
{
}

hsBool pyGUIControlValue::IsGUIControlValue(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIValueCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}


hsScalar pyGUIControlValue::GetValue()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			return pvcmod->GetCurrValue();
	}
	return 0.0;
}

void pyGUIControlValue::SetValue( hsScalar v )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			pvcmod->SetCurrValue(v);
	}
}

hsScalar pyGUIControlValue::GetMin( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			return pvcmod->GetMin();
	}
	return 0.0;
}

hsScalar pyGUIControlValue::GetMax( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			return pvcmod->GetMax();
	}
	return 0.0;
}

hsScalar pyGUIControlValue::GetStep( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			return pvcmod->GetStep();
	}
	return 0.0;
}

void pyGUIControlValue::SetRange( hsScalar min, hsScalar max )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			pvcmod->SetRange(min,max);
	}
}

void pyGUIControlValue::SetStep( hsScalar step )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pvcmod )
			pvcmod->SetStep(step);
	}
}


///////////////////////////////////////////////////////////////////////////
//
// Control Knob, which is identical to the ControlValue
//

pyGUIControlKnob::pyGUIControlKnob(pyKey& gckey) : pyGUIControlValue(gckey)
{
}

pyGUIControlKnob::pyGUIControlKnob(plKey objkey) : pyGUIControlValue(objkey)
{
}

hsBool pyGUIControlKnob::IsGUIControlKnob(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIKnobCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
// Control Up Down Pair, which is identical to the ControlValue
//

pyGUIControlUpDownPair::pyGUIControlUpDownPair(pyKey& gckey) : pyGUIControlValue(gckey)
{
}

pyGUIControlUpDownPair::pyGUIControlUpDownPair(plKey objkey) : pyGUIControlValue(objkey)
{
}

hsBool pyGUIControlUpDownPair::IsGUIControlUpDownPair(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIUpDownPairMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
// Control Progress, which is identical to the ControlValue
//

pyGUIControlProgress::pyGUIControlProgress(pyKey& gckey) : pyGUIControlValue(gckey)
{
}

pyGUIControlProgress::pyGUIControlProgress(plKey objkey) : pyGUIControlValue(objkey)
{
}

hsBool pyGUIControlProgress::IsGUIControlProgress(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIProgressCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}

void pyGUIControlProgress::AnimateToPercentage(float percent)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIProgressCtrl* ppcmod = pfGUIProgressCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ppcmod )
		{
			ppcmod->AnimateToPercentage(percent);
		}
	}
}