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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "pyKey.h"

#include "pfGameGUIMgr/pfGUIValueCtrl.h"
#include "pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "pfGameGUIMgr/pfGUIUpDownPairMod.h"
#include "pfGameGUIMgr/pfGUIProgressCtrl.h"

#include "pyGUIControlValue.h"

pyGUIControlValue::pyGUIControlValue(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlValue::pyGUIControlValue(plKey objkey) : pyGUIControl(std::move(objkey))
{
}

bool pyGUIControlValue::IsGUIControlValue(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIValueCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}


float pyGUIControlValue::GetValue()
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

void pyGUIControlValue::SetValue( float v )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pvcmod )
            pvcmod->SetCurrValue(v);
    }
}

float pyGUIControlValue::GetMin()
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

float pyGUIControlValue::GetMax()
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

float pyGUIControlValue::GetStep()
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

void pyGUIControlValue::SetRange( float min, float max )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIValueCtrl* pvcmod = pfGUIValueCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pvcmod )
            pvcmod->SetRange(min,max);
    }
}

void pyGUIControlValue::SetStep( float step )
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

pyGUIControlKnob::pyGUIControlKnob(plKey objkey) : pyGUIControlValue(std::move(objkey))
{
}

bool pyGUIControlKnob::IsGUIControlKnob(pyKey& gckey)
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

pyGUIControlUpDownPair::pyGUIControlUpDownPair(plKey objkey) : pyGUIControlValue(std::move(objkey))
{
}

bool pyGUIControlUpDownPair::IsGUIControlUpDownPair(pyKey& gckey)
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

pyGUIControlProgress::pyGUIControlProgress(plKey objkey) : pyGUIControlValue(std::move(objkey))
{
}

bool pyGUIControlProgress::IsGUIControlProgress(pyKey& gckey)
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
