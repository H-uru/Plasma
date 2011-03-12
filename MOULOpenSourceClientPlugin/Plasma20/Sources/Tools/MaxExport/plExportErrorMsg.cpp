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
#include "HeadSpin.h"
#include "hsWindows.h"
#include "hsTypes.h"
#include "plExportErrorMsg.h"
#include "hsExceptions.h"
#include "hsUtils.h"



hsBool plExportErrorMsg::Show()
{
	// If bogus, and we have something to show, show it
	if( GetBogus() && (GetMsg()[0] != 0 || GetLabel()[0] != 0))
	{
		hsMessageBox(GetMsg(), GetLabel(), hsMessageBoxNormal/*|hsMessageBoxIconError*/);
	}
	return GetBogus();
}
hsBool plExportErrorMsg::Ask()
{
	if( GetBogus() )
	{
		return IDYES == hsMessageBox(GetMsg(), GetLabel(), hsMessageBoxYesNo/*|hsMessageBoxIconExclamation*/);
	}
	return false;
}

hsBool plExportErrorMsg::CheckAndAsk()
{
	if( GetBogus() )
	{
		strncat(GetMsg(), " - File corruption possible - ABORT?", 255);
		if( Ask() )
		{
			sprintf(GetMsg(), "!Abort at user response to error!");
			Check();
		}
	}
	return GetBogus();
}

hsBool plExportErrorMsg::CheckAskOrCancel()
{
	if( GetBogus() )
	{
		strncat(GetMsg(), " - ABORT? (Cancel to mute warnings)", 255);
		int ret = hsMessageBox(GetMsg(), GetLabel(), hsMessageBoxYesNoCancel/*|hsMessageBoxIconExclamation*/);
		if( IDYES == ret )
		{
			sprintf(GetMsg(), "!Abort at user response to error!");
			Check();
		}
		else if( IDCANCEL == ret )
			return 1;
	}
	return false;
}

hsBool plExportErrorMsg::CheckAndShow()
{
	if ( GetBogus() )
	{
		Show();
		Check();
	}

	return GetBogus();
}

hsBool plExportErrorMsg::Check()
{
	if( GetBogus() )
	{
		strncat(GetMsg(), " !Output File Corrupt!", 255);
		IDebugThrow();
	}

	return false;
}

void plExportErrorMsg::Quit()
{
	if( GetBogus() )
	{
		SetBogus(false);
		hsThrow( *this );
	}
}

void plExportErrorMsg::IDebugThrow()
{
	try {
#if HS_BUILD_FOR_WIN32
		DebugBreak();
#endif // HS_BUILD_FOR_WIN32
	}
	catch(...)
	{
		hsThrow( *this );
	}
}
