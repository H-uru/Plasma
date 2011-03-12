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
//	pfGUIControl Handler Definitions										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIControlMod.h"
#include "pfGUIDialogMod.h"

#include "../plMessage/plConsoleMsg.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Writeable Stuff /////////////////////////////////////////////////////////

void	pfGUICtrlProcWriteableObject::Write( pfGUICtrlProcWriteableObject *obj, hsStream *s )
{
	if( obj != nil )
	{
		s->WriteSwap32( obj->fType );
		obj->IWrite( s );
	}
	else
		s->WriteSwap32( kNull );
}

pfGUICtrlProcWriteableObject *pfGUICtrlProcWriteableObject::Read( hsStream *s )
{
	pfGUICtrlProcWriteableObject	*obj;

	UInt32 type = s->ReadSwap32();

	switch( type )
	{
		case kConsoleCmd:
			obj = TRACKED_NEW pfGUIConsoleCmdProc;
			break;

		case kPythonScript:
			obj = TRACKED_NEW pfGUIPythonScriptProc;
			break;

		case kCloseDlg:
			obj = TRACKED_NEW pfGUICloseDlgProc;
			break;

		case kNull:
			return nil;

		default:
			hsAssert( false, "Invalid proc type in Read()" );
			return nil;
	}

	obj->IRead( s );
	return obj;
}

//////////////////////////////////////////////////////////////////////////////
//// Predefined Exportables //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// pfGUIConsoleCmdProc /////////////////////////////////////////////////////

pfGUIConsoleCmdProc::pfGUIConsoleCmdProc() : pfGUICtrlProcWriteableObject( kConsoleCmd ) 
{ 
	fCommand = nil; 
}

pfGUIConsoleCmdProc::pfGUIConsoleCmdProc( const char *cmd )
				: pfGUICtrlProcWriteableObject( kConsoleCmd ) 
{
	fCommand = nil;
	SetCommand( cmd );
}

pfGUIConsoleCmdProc::~pfGUIConsoleCmdProc()
{
	delete [] fCommand;
}

void	pfGUIConsoleCmdProc::IRead( hsStream *s )
{
	int	i = s->ReadSwap32();
	if( i > 0 )
	{
		fCommand = TRACKED_NEW char[ i + 1 ];
		memset( fCommand, 0, i + 1 );
		s->Read( i, fCommand );
	}
	else
		fCommand = nil;
}

void	pfGUIConsoleCmdProc::IWrite( hsStream *s )
{
	if( fCommand != nil )
	{
		s->WriteSwap32( strlen( fCommand ) );
		s->Write( strlen( fCommand ), fCommand );
	}
	else
		s->WriteSwap32( 0 );
}

void	pfGUIConsoleCmdProc::DoSomething( pfGUIControlMod *ctrl )
{
	if( fCommand != nil )
	{
		plConsoleMsg *cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kExecuteLine, fCommand );
		plgDispatch::MsgSend( cMsg );
	}
}

void	pfGUIConsoleCmdProc::SetCommand( const char *cmd )
{
	delete [] fCommand;

	if( cmd == nil )
		fCommand = nil;
	else
	{
		fCommand = TRACKED_NEW char[ strlen( cmd ) + 1 ];
		memset( fCommand, 0, strlen( cmd ) + 1 );
		strcpy( fCommand, cmd );
	}
}

//// pfGUIPythonScriptProc ///////////////////////////////////////////////////

pfGUIPythonScriptProc::pfGUIPythonScriptProc() : pfGUICtrlProcWriteableObject( kPythonScript ) 
{ 
}

pfGUIPythonScriptProc::~pfGUIPythonScriptProc()
{
}

void	pfGUIPythonScriptProc::IRead( hsStream *s )
{
}

void	pfGUIPythonScriptProc::IWrite( hsStream *s )
{
}

void	pfGUIPythonScriptProc::DoSomething( pfGUIControlMod *ctrl )
{
}

//////////////////////////////////////////////////////////////////////////////
//// Simple Runtime Ones /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void	pfGUICloseDlgProc::DoSomething( pfGUIControlMod *ctrl )
{
	ctrl->GetOwnerDlg()->Hide();
}
