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
#include <float.h>
#include "hsTimer.h"
#include "hsTemplates.h"
#include "plClientUnifiedTime.h"

#if 0
#include "../plNetClient/plNetObjectDebugger.h"
#include "../plNetClient/plNetClientMgr.h"
#endif

// static
plUnifiedTime	plClientUnifiedTime::fFrameStartTime	= plUnifiedTime::GetCurrentTime();	// the 'current time' at the start of each time
double			plClientUnifiedTime::fSysTimeOffset		= 0.0;

//
// static, called once at the start of each frame
//
void plClientUnifiedTime::SetSysTime()
{
	fFrameStartTime.ToCurrentTime();

	if (fSysTimeOffset == 0.0)
		fSysTimeOffset = hsTimer::GetSysSeconds() - fFrameStartTime.GetSecsDouble();
}

//
// convert from game clock to unified time
//
#pragma optimize( "g", off )	// disable global optimizations
void plClientUnifiedTime::SetFromGameTime(double gameTime, double curGameSecs)
{
	hsDoublePrecBegin;
	//double gameTimeOff = curGameSecs-gameTime;	// when did this happen relative to our currrent sysTime
	//*this = GetFrameStartTime() - plUnifiedTime(gameTimeOff);
	SetSecsDouble(gameTime - fSysTimeOffset);
	hsDoublePrecEnd;

#if 0
	extern bool gMooseDump;
	if (gMooseDump)
	{
		plUnifiedTime ct = plUnifiedTime::GetCurrentTime();
		plUnifiedTime ft = GetFrameStartTime();

		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("SFGT: CT=%s\n", ct.PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("SFGT: FT=%s\n", ft.PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("SFGT: gt=%f secs in the past\n", gameTimeOff));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("SFGT: this=%s\n\n", PrintWMillis()));
	}
#endif
}

//
// convert from unified time to game clock
//
void plClientUnifiedTime::ConvertToGameTime(double* gameTimeOut, double curGameSecs)
{
	hsDoublePrecBegin;
	//plUnifiedTime utOff = GetFrameStartTime() - GetAsUnifiedTime();	// compute offset relative to current startFrame time
	//*gameTimeOut = curGameSecs - utOff.GetSecsDouble();
	*gameTimeOut = GetSecsDouble() + fSysTimeOffset;
	hsDoublePrecEnd;

#if 0
	extern bool gMooseDump;
	if (gMooseDump)
	{
		plUnifiedTime ct = plUnifiedTime::GetCurrentTime();
		plUnifiedTime ft = GetFrameStartTime();

		plNetObjectDebugger::GetInstance()->LogMsg(	hsTempStringF("CTGT: this=%s\n", PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("CTGT: CT=%s\n", ct.PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("CTGT: FT=%s\n", ft.PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(hsTempStringF("CTGT: OT=%s\n", utOff.PrintWMillis()));
		plNetObjectDebugger::GetInstance()->LogMsg(
			hsTempStringF("CTGT: ct=%f TO=%f\n\n", curGameSecs, *gameTimeOut));
	}
#endif
}
#pragma optimize( "", on )	// restore optimizations to their defaults

const plClientUnifiedTime & plClientUnifiedTime::operator=(const plUnifiedTime & src) 
{ 
	plUnifiedTime* ut=this;
	*ut=src;
	return *this;
}

const plClientUnifiedTime & plClientUnifiedTime::operator=(const plClientUnifiedTime & src) 
{ 
	plUnifiedTime* ut=this;
	plUnifiedTime* utSrc=this;
	*ut=*utSrc;
	return *this;
}
