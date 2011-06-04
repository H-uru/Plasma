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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plDebugInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsConfig.h"
#include "hsWindows.h"
#include "hsTypes.h"
#include "plDebugInputInterface.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnInputCore/plKeyMap.h"

#include "plgDispatch.h"
#include "plPipeline.h"
#include "hsConfig.h"


plDebugInputInterface	*plDebugInputInterface::fInstance = nil;


//// Constructor/Destructor //////////////////////////////////////////////////

plDebugInputInterface::plDebugInputInterface()
{
	fInstance = this;

	// Add our control codes to our control map. Do NOT add the key bindings yet.
	// Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
	// This part basically declares us master of the bindings for these commands.
	
	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!

#ifndef PLASMA_EXTERNAL_RELEASE
//	fControlMap->AddCode( B_CONTROL_MODIFIER_FAST,		kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_DRIVE_SPEED_UP,		kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_DRIVE_SPEED_DOWN,	kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_MOVE_FORWARD,		kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_MOVE_BACKWARD,		kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_MOVE_LEFT,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_MOVE_RIGHT,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_MOVE_UP,				kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_MOVE_DOWN,			kControlFlagNormal | kControlFlagNoRepeat );
//	fControlMap->AddCode( B_TOGGLE_DRIVE_MODE,			kControlFlagNormal | kControlFlagNoRepeat | kControlFlagShift );
#endif

	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!
}

plDebugInputInterface::~plDebugInputInterface()
{
	fInstance = nil;
}

//// Init/Shutdown ///////////////////////////////////////////////////////////

void	plDebugInputInterface::Init( plInputInterfaceMgr *manager )
{
	plInputInterface::Init( manager );
}

void	plDebugInputInterface::Shutdown( void )
{
}

//// RestoreDefaultKeyMappings ///////////////////////////////////////////////

void	plDebugInputInterface::RestoreDefaultKeyMappings( void )
{
	if( fControlMap == nil )
		return;

	fControlMap->UnmapAllBindings();

#ifndef PLASMA_EXTERNAL_RELEASE
//	fControlMap->BindKey( KEY_SHIFT,	B_CONTROL_MODIFIER_FAST );
	fControlMap->BindKey( plShiftKeyCombo( KEY_EQUAL ),		B_CAMERA_DRIVE_SPEED_UP );
	fControlMap->BindKey( plShiftKeyCombo( KEY_DASH ),		B_CAMERA_DRIVE_SPEED_DOWN );
	fControlMap->BindKey( KEY_W,							B_CAMERA_MOVE_FORWARD );
	fControlMap->BindKey( KEY_S,							B_CAMERA_MOVE_BACKWARD );
	fControlMap->BindKey( KEY_A,							B_CAMERA_MOVE_LEFT );
	fControlMap->BindKey( KEY_D,							B_CAMERA_MOVE_RIGHT );
	fControlMap->BindKey( KEY_I,							B_CAMERA_MOVE_UP );
	fControlMap->BindKey( KEY_K,							B_CAMERA_MOVE_DOWN );
//	fControlMap->BindKey( KEY_C,							B_TOGGLE_DRIVE_MODE );
#endif
}


//// IEval ///////////////////////////////////////////////////////////////////

hsBool plDebugInputInterface::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return true;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	plDebugInputInterface::MsgReceive( plMessage *msg )
{
	return plInputInterface::MsgReceive(msg);
}

//// cursorinbox /////////////////////////////////////////////////////
hsBool plDebugInputInterface::CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box)
{
	return ( pMsg->GetXPos() >= box.fX && pMsg->GetXPos() <= box.fY && pMsg->GetYPos() >= box.fZ && pMsg->GetYPos() <= box.fW ); 
}


//// InterpretInputEvent /////////////////////////////////////////////////////

hsBool	plDebugInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
	hsBool		handled = false;

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
		

		for (int i=0; i < fMouseMap.fMap.Count(); i++)
		{
			// is this control already set?
			if (fControlFlags.IsBitSet(fMouseMap.fMap[i]->fCode))
			{
				// can we disable this control?
				hsBool disable = false;
				
				// can we disable this control based on a button?
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButton && !(fButtonState & kLeftButtonDown))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButton && !(fButtonState & kRightButtonDown))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonEx && (fButtonState & kLeftButtonRepeat))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonEx && (fButtonState & kRightButtonRepeat))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonEx && !(fButtonState & kLeftButtonDown))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonEx && !(fButtonState & kRightButtonDown))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonRepeat && !(fButtonState & kLeftButtonDown))
					disable = true;
				if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonRepeat && !(fButtonState & kRightButtonDown))
					disable = true;

				// can we disable this control based on the cursor position?
				if (!CursorInBox(pMouseMsg, fMouseMap.fMap[i]->fBox) && fMouseMap.fMap[i]->fControlFlags & kControlFlagBoxDisable)
					disable = true;
				
				if (disable)
				{
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fNetPropagateToPlayers = fMouseMap.fMap[i]->fControlFlags & kControlFlagNetPropagate;
					pCmd->fControlActivated = false;
					pCmd->fControlCode = fMouseMap.fMap[i]->fCode;
					fControlFlags.ClearBit(pCmd->fControlCode);
					fMessageQueue->Append(pCmd);
					handled = true;
					continue;
				}
				// is it a range control?  If so we need to re-send the command
				
				if ((fMouseMap.fMap[i]->fControlFlags & kControlFlagRangePos) || (fMouseMap.fMap[i]->fControlFlags & kControlFlagRangeNeg))
				{
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fControlActivated = true;
					pCmd->fControlCode = fMouseMap.fMap[i]->fCode;
					hsScalar pct = 0.0f;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRangePos)
					{
						if (fMouseMap.fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((fMouseMap.fMap[i]->fBox.fX - pMouseMsg->GetXPos()) / (fMouseMap.fMap[i]->fBox.fY - fMouseMap.fMap[i]->fBox.fX));
						else
							pct = hsABS((fMouseMap.fMap[i]->fBox.fZ - pMouseMsg->GetYPos()) / (fMouseMap.fMap[i]->fBox.fW - fMouseMap.fMap[i]->fBox.fZ));
					}
					else 
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRangeNeg)
					{
						if (fMouseMap.fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((fMouseMap.fMap[i]->fBox.fY - pMouseMsg->GetXPos()) / (fMouseMap.fMap[i]->fBox.fY - fMouseMap.fMap[i]->fBox.fX));
						else
							pct = hsABS((fMouseMap.fMap[i]->fBox.fW - pMouseMsg->GetYPos()) / (fMouseMap.fMap[i]->fBox.fW - fMouseMap.fMap[i]->fBox.fZ));
					}
					pCmd->fPct = pct;
					if (pct == 1.0f || pct == -1.0f)
					{
						delete pCmd;
						break;
					}
					pCmd->fNetPropagateToPlayers = fMouseMap.fMap[i]->fControlFlags & kControlFlagNetPropagate;
					fMessageQueue->Append(pCmd);
				}
			}
			else // if it is an 'always if in box' command see if it's not in the box
			if ( (fMouseMap.fMap[i]->fControlFlags & kControlFlagInBox) && (!CursorInBox(pMouseMsg, fMouseMap.fMap[i]->fBox)) )
			{	
				plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
				pCmd->fControlActivated = false;
				pCmd->fControlCode = fMouseMap.fMap[i]->fCode;
				pCmd->fNetPropagateToPlayers = fMouseMap.fMap[i]->fControlFlags & kControlFlagNetPropagate;
				fMessageQueue->Append(pCmd);
				continue;
			}
			else // the control is not set, see if we should set it.
			{
				// is the cursor in the appropriate box?
				if (CursorInBox(pMouseMsg, fMouseMap.fMap[i]->fBox))
				{	
					// do we require a button?
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButton && !(fButtonState & kLeftButtonDown))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButton && !(fButtonState & kRightButtonDown))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonEx && (fButtonState & kLeftButtonRepeat))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonEx && (fButtonState & kRightButtonRepeat))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonRepeat && !(fButtonState & kLeftButtonRepeat))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonRepeat && !(fButtonState & kRightButtonRepeat))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonEx && !(fButtonState & kLeftButtonDown))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonEx && !(fButtonState & kLeftButtonDown))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagLeftButtonUp && !(pMouseMsg->GetButton() == kLeftButtonUp))
						continue;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRightButtonUp && !(pMouseMsg->GetButton() == kRightButtonUp))
						continue;

					// okay, we're in the box and either we don't require a button or our button is pressed.
					// so set the command as 'enabled' 
					// UNLESS it has kControlFlagInBox, which means we want it sent every frame it is in the box
					if (!(fMouseMap.fMap[i]->fControlFlags & kControlFlagInBox))
						fControlFlags.SetBit(fMouseMap.fMap[i]->fCode);
					// issue the command
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fControlActivated = true;
					pCmd->fControlCode = fMouseMap.fMap[i]->fCode;
					pCmd->fNetPropagateToPlayers = fMouseMap.fMap[i]->fControlFlags & kControlFlagNetPropagate;

					// figure out what percent (if any)
					hsScalar pct = 0.0f;
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRangePos)
					{
						if (fMouseMap.fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((fMouseMap.fMap[i]->fBox.fX - pMouseMsg->GetXPos()) / (fMouseMap.fMap[i]->fBox.fY - fMouseMap.fMap[i]->fBox.fX));
						else
							pct = hsABS((fMouseMap.fMap[i]->fBox.fZ - pMouseMsg->GetYPos()) / (fMouseMap.fMap[i]->fBox.fW - fMouseMap.fMap[i]->fBox.fZ));
					}
					else 
					if (fMouseMap.fMap[i]->fControlFlags & kControlFlagRangeNeg)
					{
						if (fMouseMap.fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((fMouseMap.fMap[i]->fBox.fY - pMouseMsg->GetXPos()) / (fMouseMap.fMap[i]->fBox.fY - fMouseMap.fMap[i]->fBox.fX));
						else
							pct = hsABS((fMouseMap.fMap[i]->fBox.fW - pMouseMsg->GetYPos()) / (fMouseMap.fMap[i]->fBox.fW - fMouseMap.fMap[i]->fBox.fZ));
					}
					pCmd->fPct = pct;
					if (pct == 1.0f || pct == -1.0f)
					{
						delete pCmd;
						break;
					}

					// and add it to the list
					fMessageQueue->Append(pCmd);
					handled = true;
					continue;
				}
			}
		}

		return handled;
	}

	return false;
}	
