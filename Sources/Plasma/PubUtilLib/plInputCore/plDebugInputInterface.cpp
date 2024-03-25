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
//  plDebugInputInterface                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plDebugInputInterface.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "pnKeyedObject/plKey.h"
#include "pnInputCore/plKeyMap.h"

#include "plgDispatch.h"
#include "plPipeline.h"


plDebugInputInterface   *plDebugInputInterface::fInstance = nullptr;


//// Constructor/Destructor //////////////////////////////////////////////////

plDebugInputInterface::plDebugInputInterface()
    : fButtonState()
{
    fInstance = this;

    // Add our control codes to our control map. Do NOT add the key bindings yet.
    // Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
    // This part basically declares us master of the bindings for these commands.
    
    // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
    // RestoreDefaultKeyMappings()!!!!

#ifndef PLASMA_EXTERNAL_RELEASE
//  fControlMap->AddCode( B_CONTROL_MODIFIER_FAST,      kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_DRIVE_SPEED_UP,      kControlFlagNormal );
    fControlMap->AddCode( B_CAMERA_DRIVE_SPEED_DOWN,    kControlFlagNormal );
    fControlMap->AddCode( B_CAMERA_MOVE_FORWARD,        kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_MOVE_BACKWARD,       kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_MOVE_LEFT,           kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_MOVE_RIGHT,          kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_MOVE_UP,             kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CAMERA_MOVE_DOWN,           kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_TOGGLE_DRIVE_MODE,          kControlFlagNormal | kControlFlagNoRepeat );
#endif

    // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
    // RestoreDefaultKeyMappings()!!!!
}

plDebugInputInterface::~plDebugInputInterface()
{
    fInstance = nullptr;
}

//// Init/Shutdown ///////////////////////////////////////////////////////////

void    plDebugInputInterface::Init( plInputInterfaceMgr *manager )
{
    plInputInterface::Init( manager );
}

void    plDebugInputInterface::Shutdown()
{
}

//// RestoreDefaultKeyMappings ///////////////////////////////////////////////

void    plDebugInputInterface::RestoreDefaultKeyMappings()
{
    if (fControlMap == nullptr)
        return;

    fControlMap->UnmapAllBindings();

#ifndef PLASMA_EXTERNAL_RELEASE
//  fControlMap->BindKey( KEY_SHIFT,    B_CONTROL_MODIFIER_FAST );
    fControlMap->BindKey( plShiftKeyCombo( KEY_EQUAL ),     B_CAMERA_DRIVE_SPEED_UP );
    fControlMap->BindKey( plShiftKeyCombo( KEY_DASH ),      B_CAMERA_DRIVE_SPEED_DOWN );
    fControlMap->BindKey( KEY_W,                            B_CAMERA_MOVE_FORWARD );
    fControlMap->BindKey( KEY_S,                            B_CAMERA_MOVE_BACKWARD );
    fControlMap->BindKey( KEY_A,                            B_CAMERA_MOVE_LEFT );
    fControlMap->BindKey( KEY_D,                            B_CAMERA_MOVE_RIGHT );
    fControlMap->BindKey( KEY_I,                            B_CAMERA_MOVE_UP );
    fControlMap->BindKey( KEY_K,                            B_CAMERA_MOVE_DOWN );
    fControlMap->BindKey( plCtrlKeyCombo( KEY_D ),          B_TOGGLE_DRIVE_MODE );
#endif
}


//// IEval ///////////////////////////////////////////////////////////////////

bool plDebugInputInterface::IEval( double secs, float del, uint32_t dirty )
{
    return true;
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    plDebugInputInterface::MsgReceive( plMessage *msg )
{
    return plInputInterface::MsgReceive(msg);
}

//// cursorinbox /////////////////////////////////////////////////////
bool plDebugInputInterface::CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box)
{
    return ( pMsg->GetXPos() >= box.fX && pMsg->GetXPos() <= box.fY && pMsg->GetYPos() >= box.fZ && pMsg->GetYPos() <= box.fW ); 
}


//// InterpretInputEvent /////////////////////////////////////////////////////

bool    plDebugInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
    bool        handled = false;

    plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(pMsg);
    if (pMouseMsg)
    {
        // check for button presses...
        if (fButtonState & kLeftButtonDown)
        {
            fButtonState |= kLeftButtonRepeat;
        }
        if (fButtonState & kRightButtonDown)
        {
            fButtonState |= kRightButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kLeftButtonDown)
        {
            fButtonState |= kLeftButtonDown;
        }
        if (pMouseMsg->GetButton() == kLeftButtonUp)
        {
            fButtonState &= ~kLeftButtonDown;
            fButtonState &= ~kLeftButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kRightButtonDown)
        {
            fButtonState |= kRightButtonDown;
        }
        if (pMouseMsg->GetButton() == kRightButtonUp)
        {
            fButtonState &= ~kRightButtonDown;
            fButtonState &= ~kRightButtonRepeat;
        }

        for (plMouseInfo *mouseInfo : fMouseMap.fMap)
        {
            // is this control already set?
            if (fControlFlags.IsBitSet(mouseInfo->fCode))
            {
                // can we disable this control?
                bool disable = false;
                
                // can we disable this control based on a button?
                if (mouseInfo->fControlFlags & kControlFlagLeftButton && !(fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButton && !(fButtonState & kRightButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && (fButtonState & kLeftButtonRepeat))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && (fButtonState & kRightButtonRepeat))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && !(fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && !(fButtonState & kRightButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonRepeat && !(fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonRepeat && !(fButtonState & kRightButtonDown))
                    disable = true;

                // can we disable this control based on the cursor position?
                if (!CursorInBox(pMouseMsg, mouseInfo->fBox) && mouseInfo->fControlFlags & kControlFlagBoxDisable)
                    disable = true;
                
                if (disable)
                {
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                    pCmd->fControlActivated = false;
                    pCmd->fControlCode = mouseInfo->fCode;
                    fControlFlags.ClearBit(pCmd->fControlCode);
                    fMessageQueue->emplace_back(pCmd);
                    handled = true;
                    continue;
                }
                // is it a range control?  If so we need to re-send the command
                
                if ((mouseInfo->fControlFlags & kControlFlagRangePos) || (mouseInfo->fControlFlags & kControlFlagRangeNeg))
                {
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fControlActivated = true;
                    pCmd->fControlCode = mouseInfo->fCode;
                    float pct = 0.0f;
                    if (mouseInfo->fControlFlags & kControlFlagRangePos)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fX - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fZ - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    else 
                    if (mouseInfo->fControlFlags & kControlFlagRangeNeg)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fY - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fW - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    pCmd->fPct = pct;
                    if (pct == 1.0f || pct == -1.0f)
                    {
                        delete pCmd;
                        break;
                    }
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                    fMessageQueue->emplace_back(pCmd);
                }
            }
            else // if it is an 'always if in box' command see if it's not in the box
            if ((mouseInfo->fControlFlags & kControlFlagInBox) && (!CursorInBox(pMouseMsg, mouseInfo->fBox)))
            {   
                plCtrlCmd* pCmd = new plCtrlCmd( this );
                pCmd->fControlActivated = false;
                pCmd->fControlCode = mouseInfo->fCode;
                pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                fMessageQueue->emplace_back(pCmd);
                continue;
            }
            else // the control is not set, see if we should set it.
            {
                // is the cursor in the appropriate box?
                if (CursorInBox(pMouseMsg, mouseInfo->fBox))
                {   
                    // do we require a button?
                    if (mouseInfo->fControlFlags & kControlFlagLeftButton && !(fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButton && !(fButtonState & kRightButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && (fButtonState & kLeftButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && (fButtonState & kRightButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonRepeat && !(fButtonState & kLeftButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonRepeat && !(fButtonState & kRightButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && !(fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && !(fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonUp && !(pMouseMsg->GetButton() == kLeftButtonUp))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonUp && !(pMouseMsg->GetButton() == kRightButtonUp))
                        continue;

                    // okay, we're in the box and either we don't require a button or our button is pressed.
                    // so set the command as 'enabled' 
                    // UNLESS it has kControlFlagInBox, which means we want it sent every frame it is in the box
                    if (!(mouseInfo->fControlFlags & kControlFlagInBox))
                        fControlFlags.SetBit(mouseInfo->fCode);
                    // issue the command
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fControlActivated = true;
                    pCmd->fControlCode = mouseInfo->fCode;
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;

                    // figure out what percent (if any)
                    float pct = 0.0f;
                    if (mouseInfo->fControlFlags & kControlFlagRangePos)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fX - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fZ - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    else 
                    if (mouseInfo->fControlFlags & kControlFlagRangeNeg)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fY - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fW - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    pCmd->fPct = pct;
                    if (pct == 1.0f || pct == -1.0f)
                    {
                        delete pCmd;
                        break;
                    }

                    // and add it to the list
                    fMessageQueue->emplace_back(pCmd);
                    handled = true;
                    continue;
                }
            }
        }

        return handled;
    }

    return false;
}   
