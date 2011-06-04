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
//	plTelescopeInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsConfig.h"
#include "hsWindows.h"

#include "hsTypes.h"
#include "plTelescopeInputInterface.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"
#include "../pnInputCore/plKeyMap.h"

#include "plgDispatch.h"



//// Constructor/Destructor //////////////////////////////////////////////////

plTelescopeInputInterface::plTelescopeInputInterface()
{
	SetEnabled( true );			// Always enabled


	// Add our control codes to our control map. Do NOT add the key bindings yet.
	// Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
	// This part basically declares us master of the bindings for these commands.
	
	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!

	fControlMap->AddCode( B_CONTROL_EXIT_MODE,	kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_PAN_LEFT,	kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_PAN_RIGHT,	kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_PAN_UP,		kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_PAN_DOWN,	kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_RECENTER,	kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CAMERA_ZOOM_IN,		kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_ZOOM_OUT,	kControlFlagNormal );

	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!
}

plTelescopeInputInterface::~plTelescopeInputInterface()
{
}

//// Init/Shutdown ///////////////////////////////////////////////////////////

void	plTelescopeInputInterface::Init( plInputInterfaceMgr *manager )
{
	plInputInterface::Init( manager );
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool plTelescopeInputInterface::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return true;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	plTelescopeInputInterface::MsgReceive( plMessage *msg )
{
	return false;
}

//// InterpretInputEvent /////////////////////////////////////////////////////

hsBool plTelescopeInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
	return false;
}

//// RestoreDefaultKeyMappings ///////////////////////////////////////////////

void	plTelescopeInputInterface::RestoreDefaultKeyMappings( void )
{
	if( fControlMap == nil )
		return;

	fControlMap->UnmapAllBindings();

	fControlMap->BindKey( KEY_BACKSPACE,		B_CONTROL_EXIT_MODE );
	fControlMap->BindKey( KEY_NUMPAD5,			B_CAMERA_RECENTER );
	fControlMap->BindKey( KEY_C,				B_CAMERA_RECENTER );
	fControlMap->BindKey( KEY_NUMPAD_ADD,		B_CAMERA_ZOOM_IN );
	fControlMap->BindKey( KEY_NUMPAD_SUBTRACT,	B_CAMERA_ZOOM_OUT );
	fControlMap->BindKey( KEY_NUMPAD4,			B_CAMERA_PAN_LEFT );
	fControlMap->BindKey( KEY_NUMPAD6,			B_CAMERA_PAN_RIGHT );
	fControlMap->BindKey( KEY_NUMPAD8,			B_CAMERA_PAN_UP );
	fControlMap->BindKey( KEY_NUMPAD2,			B_CAMERA_PAN_DOWN );
}

