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
//	CCR Console Commands and Groups											//
//  These console commands are meant for use by customer care reps.			//
//  Eventually the functionality defined here will be accessed through a GUI//
//																			//
//////////////////////////////////////////////////////////////////////////////

//
// Only calls to the CCRMgr interface are allowed here
// See me if you need to include any other files...
//
#include "pfConsoleCmd.h"
#include "pfConsole.h"
#include "../pfCCR/plCCRMgr.h"
#include "../plNetClient/plNetClientMgr.h"

//// This is here so Microsoft VC won't decide to "optimize" this file out
// YOU ALSO NEED TO CALL THIS FXN
void	pfConsoleCmdGroup::DummyCCR( void )
{
}

void PrintStringF(void pfun(const char *),const char * fmt, ...);


/////////////////////////////////////////////////////////////////
//
// Please see pfConsoleCommands.cpp for detailed instructions on
// how to add console commands.
//
/////////////////////////////////////////////////////////////////

#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }
