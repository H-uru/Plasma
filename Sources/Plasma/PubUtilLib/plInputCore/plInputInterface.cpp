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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plInputInterface.cpp - A single layer on the input interface stack      //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  2.20.02 mcn - Created.                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plInputInterface.h"
#include "plInputInterfaceMgr.h"

#include "pnInputCore/plKeyMap.h"
#include "plMessage/plInputEventMsg.h"

#include "hsResMgr.h"
#include "plgDispatch.h"


//// Constructor/Destructor //////////////////////////////////////////////////

plInputInterface::plInputInterface()
    : fEnabled(), fControlMap(new plKeyMap),
      fManager(), fMessageQueue()
{ }

plInputInterface::~plInputInterface()
{
    delete fControlMap;
}

void plInputInterface::ClearKeyMap()
{ 
    if (fControlMap != nullptr)
        fControlMap->ClearAll(); 
}

//// Read/Write //////////////////////////////////////////////////////////////

void    plInputInterface::Read( hsStream* s, hsResMgr* mgr )
{
}

void    plInputInterface::Write( hsStream* s, hsResMgr* mgr )
{
}

//// Helper Functions ////////////////////////////////////////////////////////

bool        plInputInterface::IOwnsControlCode( ControlEventCode code )
{
    if (fControlMap->FindBinding(code) != nullptr)
        return true;
    
    return false;
}

//// IVerifyShiftKey /////////////////////////////////////////////////////////
// special logic so the shift key can make everyone totally happy...

bool    plInputInterface::IVerifyShiftKey( plKeyDef key, int index )
{
    // if we are mapped to the actual shift key, return true
    if (key == KEY_SHIFT)
        return true;
    
    // if anything else is mapped to this key + shift, return false
    /*  for (int i=0; i < fControlMap->GetNumBindings(); i++)
    {
    if (index == i)
    continue;
    if (fKeyMap->fMap[i]->fKeyDef == key && fKeyMap->fMap[i]->fKeyFlags & plKeyInfo::kKeyShift )
    return false;
    }
    */  return true;
}

void plInputInterface::IDeactivateBinding(const plKeyBinding *binding)
{
    if( !(binding->GetCodeFlags() & kControlFlagNoDeactivate) && !(binding->GetCodeFlags() & kControlFlagToggle) )
    {
        plCtrlCmd *pCmd = new plCtrlCmd( this );
        pCmd->fControlCode = binding->GetCode();
        pCmd->fControlActivated = false;
        pCmd->SetCmdString( binding->GetExtendedString() );
        pCmd->fNetPropagateToPlayers = ( binding->GetCodeFlags() & kControlFlagNetPropagate ) ? true : false;
    
        fMessageQueue->emplace_back(pCmd);
    }
    IClearKeyControlFlag(binding->GetCode());
}
    
//// ProcessKeyBindings //////////////////////////////////////////////////////
//  Processes the given key event as a key binding, if one exists. If not,
//  returns false.

bool    plInputInterface::ProcessKeyBindings( plInputEventMsg *msg )
{
    bool    activate;
    
    plKeyEventMsg   *keyMsg = plKeyEventMsg::ConvertNoRef( msg );
    if (keyMsg == nullptr)
        return false;


    /// We might have controls that are currently enabled that are triggered in part by 
    /// modifiers (ctrl or shift)...if that is true, then we want to disable them if either
    /// of those modifiers are up, no matter what key this message is for
    std::vector<int16_t> enabledCtrls;
    fKeyControlFlags.Enumerate(enabledCtrls);

    for (int16_t ctrl : enabledCtrls)
    {
        const plKeyBinding *binding = fControlMap->FindBinding((ControlEventCode)ctrl);
        if (binding == nullptr)
            ; // Somehow we lost the binding??
        else
        {
            bool wantShift, wantCtrl;
            if( fKeyControlsFrom2ndKeyFlags.IsBitSet(ctrl) )
            {
                wantShift = ( binding->GetKey2().fFlags & plKeyCombo::kShift ) || ( binding->GetKey2().fKey == KEY_SHIFT );
                wantCtrl = ( binding->GetKey2().fFlags & plKeyCombo::kCtrl ) || ( binding->GetKey2().fKey == KEY_CTRL );
            }
            else
            {
                wantShift = ( binding->GetKey1().fFlags & plKeyCombo::kShift ) || ( binding->GetKey1().fKey == KEY_SHIFT );
                wantCtrl = ( binding->GetKey1().fFlags & plKeyCombo::kCtrl ) || ( binding->GetKey1().fKey == KEY_CTRL );
            }

            if( ( wantShift && !keyMsg->GetShiftKeyDown() ) || ( wantCtrl && !keyMsg->GetCtrlKeyDown() ) )
            {
                IDeactivateBinding(binding);
                fKeyControlsFrom2ndKeyFlags.SetBit(ctrl, false);
            }
        }
    }


    /// Process any binding for this message's key code now
    plKeyCombo  combo( keyMsg->GetKeyCode(), ( keyMsg->GetShiftKeyDown() ? plKeyCombo::kShift : 0 ) |
                                            ( keyMsg->GetCtrlKeyDown() ? plKeyCombo::kCtrl : 0 ) );

    std::vector<const plKeyBinding *> bindings;
    fControlMap->FindAllBindingsByKey(combo, bindings);

    // The first binding is the one we want. (FindAllBindingsByKey guarantees this)
    const plKeyBinding *binding = (bindings.empty() ? nullptr : bindings.front());

    // If other bindings were found, they lose out to the first one.
    for (size_t i = 1; i < bindings.size(); i++)
        IDeactivateBinding(bindings[i]);

    /*
    const plKeyBinding *binding = fControlMap->FindBindingByKey( combo );
    if (binding == nullptr)
    {
        // Don't panic just yet, there are some special cases with the shift key to check first
        if( keyMsg->GetKeyCode() == KEY_SHIFT || keyMsg->GetShiftKeyDown() )
        {
            // See, there are two other cases to consider: 1) we have a binding directly to the shift
            // key, which wouldn't have the shift flag set (so the above search wouldn't have caught it).

            // The second case would be if we have a matching binding without shift...
            // which is VALID so long as no other bindings respond to this key combo + shift, but of course,
            // if there were, we'd have found them already!

            // Either way, we remove the shift flag and try again
            combo.fFlags &= ~plKeyCombo::kShift;

            binding = fControlMap->FindBindingByKey( combo );
        }
    }
    */

    if (!binding)
        return false;

    uint32_t codeFlags = binding->GetCodeFlags();

    // Filter out no-repeat messages
    if( ( codeFlags & kControlFlagNoRepeat ) && keyMsg->GetRepeat() )
        return false;

    if( codeFlags & kControlFlagNormal )
    {
        // "Normal" behavior--enable on key down, disable on key up
        activate = keyMsg->GetKeyDown() ? true : false;
    }
    else if( codeFlags & kControlFlagToggle )
    {
        // Toggle behavior
        if( ( codeFlags & kControlFlagDownEvent ) && !keyMsg->GetKeyDown() )
            return false;
        if( ( codeFlags & kControlFlagUpEvent ) && keyMsg->GetKeyDown() )
            return false;

        if( IHasKeyControlFlag( binding->GetCode() ) )
            activate = false;
        else
            activate = true;
    }
    else
    {
        // Remaining ones are triggered to activate on their flagged event and
        // deactivate when that turns false
        if( ( codeFlags & kControlFlagDownEvent ) && !keyMsg->GetKeyDown() )
            activate = false;
        else if( ( codeFlags & kControlFlagUpEvent ) && keyMsg->GetKeyDown() )
            activate = false;
        else
            activate = true;
    }

    bool wasActive = IHasKeyControlFlag(binding->GetCode());

    // Set or clear our flags, since we do that even if we don't send a message
    if ( !keyMsg->GetKeyChar() )
    {
        if( activate )
        {
            ISetKeyControlFlag( binding->GetCode() );
            fKeyControlsFrom2ndKeyFlags.SetBit( binding->GetCode(), ( binding->GetKey2() == combo ) ? true : false );
        }
        else
        {
            IClearKeyControlFlag( binding->GetCode() );
            fKeyControlsFrom2ndKeyFlags.SetBit( binding->GetCode(), 0 );
        }
    }

    // Filter out codes that only want their activate messages sent (like console commands)
    if( ( codeFlags & kControlFlagNoDeactivate ) && !activate )
        return false;

    if (!IControlCodeEnabled(binding->GetCode()))
    {
        if (activate || (codeFlags & kControlFlagToggle) || !wasActive)
        {
            // It's ok to deactivate a disabled control, but not activate it
            return false;
        }
    }

    // Still here? Only proces bound keys for KEYDOWNS.
    // We'll pretend to process CHARs so they don't get sent on...
    if ( keyMsg->GetKeyChar() )
        return true;

    /// OK, generate the message to send
    plCtrlCmd *pCmd = new plCtrlCmd( this );
    pCmd->fControlCode = binding->GetCode();
    pCmd->fControlActivated = activate;

    pCmd->SetCmdString( binding->GetExtendedString() );
    pCmd->fNetPropagateToPlayers = ( codeFlags & kControlFlagNetPropagate ) ? true : false;

    fMessageQueue->emplace_back(pCmd);

    return true;
}

bool plInputInterface::IControlCodeEnabled(ControlEventCode code )
{ 
    return (!fDisabledControls.IsBitSet(code));
}