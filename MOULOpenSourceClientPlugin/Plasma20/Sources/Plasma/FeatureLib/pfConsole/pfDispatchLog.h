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
#ifndef pfDispatchLog_inc
#define pfDispatchLog_inc

#include "../pnDispatch/plDispatchLogBase.h"
#include "hsBitVector.h"

class plStatusLog;

class plDispatchLog : public plDispatchLogBase
{
private:
	hsBitVector fIncludeTypes;	// include/exclude list
	UInt64 fStartTicks;
	plStatusLog* fLog;

public:
	plDispatchLog();
	~plDispatchLog();

	static void InitInstance();

	void AddFilterType(UInt16 type);
	void AddFilterExactType(UInt16 type);

	void RemoveFilterType(UInt16 type);
	void RemoveFilterExactType(UInt16 type);

	void LogStatusBarChange(const char* name, const char* action);
	void LogLongReceive(const char* keyname, const char* className, UInt32 clonePlayerID, plMessage* msg, float ms);

	void DumpMsg(plMessage* msg, int numReceivers, int sendTimeMs, Int32 indent);
};

#endif	// pfDispatchLog_inc
