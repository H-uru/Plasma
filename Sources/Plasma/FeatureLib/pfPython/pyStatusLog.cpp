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
//////////////////////////////////////////////////////////////////////
//
// pyStatusLog   - a wrapper class to provide interface to the plStatusLog stuff
//
//  and interface to the ChatLog (ptChatStatusLog)
//////////////////////////////////////////////////////////////////////

#include "pyStatusLog.h"

#include "../plStatusLog/plStatusLog.h"

pyStatusLog::pyStatusLog( plStatusLog* log/*=nil */)
: fLog( log )
, fICreatedLog( false )
{
}

pyStatusLog::~pyStatusLog()
{
	Close();
}


hsBool pyStatusLog::Open(const char* logName, UInt32 numLines, UInt32 flags)
{
	// make sure its closed first
	Close();

	// create a status log guy for this
	fICreatedLog = true;
	fLog = plStatusLogMgr::GetInstance().CreateStatusLog( (UInt8)numLines, logName, flags );
	if (fLog)
	{
		fLog->SetForceLog(true);
		return true;
	}
	return false;
}

hsBool pyStatusLog::Write(const char* text)
{
	if (fLog)
	{
		fLog->AddLine(text);
		return true;
	}

	return false;
}

hsBool pyStatusLog::WriteColor(const char* text, pyColor& color)
{
	if (fLog)
	{
		UInt32 st_color = ((UInt32)(color.getAlpha()*255)<<24) +
								((UInt32)(color.getRed()*255)<<16) +
								((UInt32)(color.getGreen()*255)<<8) + 
								((UInt32)(color.getBlue()*255));
		fLog->AddLine( text, st_color );
		return true;
	}

	return false;
}

void pyStatusLog::Close()
{
	if (fLog && fICreatedLog)
	{
		delete fLog;
	}
	fLog = nil;
}

hsBool pyStatusLog::IsOpen()
{
	if (fLog)
		return true;
	return false;
}
