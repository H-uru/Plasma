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
#ifndef hsExceptionStack_inc
#define hsExceptionStack_inc

#include "hsUtils.h"
#include "hsTypes.h"
#include "hsTemplates.h"

class hsExceptionStackDestroyer;

//
// hsExceptionStack - logs function/scope IDs added by hsStackToken
//

class hsExceptionStack
{
	friend class hsExceptionStackDestroyer;
private:
	hsExceptionStack()						{ }
public:
	~hsExceptionStack()						{ }

	static hsExceptionStack& Instance();

	Int32 GetNumEntries() const				{ return fEntries.Count(); }
	const char* GetEntry(Int32 i) const		{ return fEntries[i]; }

	void Push(const char* str);

	// After an exception is caught and stack has been displayed,
	// call continue to flush stack
	void Continue()							{ fEntries.Reset(); }

private:
	static void FreeInstance();

	hsTArray<const char*>					fEntries;

	static hsExceptionStack*				fExceptionStack;
	static hsExceptionStackDestroyer		fExceptionStackDestroyer;
};

inline hsExceptionStack& hsExceptionStack::Instance()
{
	if (!fExceptionStack)
	{
		fExceptionStack = TRACKED_NEW hsExceptionStack;
	}

	return *fExceptionStack;
}

inline void hsExceptionStack::Push(const char* str)
{
	fEntries.Append(str);
}

//
// hsExceptionStackDestroyer - removes the hsExceptionStack instance
//
class hsExceptionStackDestroyer 
{
public:
	~hsExceptionStackDestroyer()
	{
		hsExceptionStack::FreeInstance();
	}
};

#ifdef HS_DEBUGGING
#define HS_NO_TRY
#endif

#ifdef HS_NO_TRY

#define hsGuardBegin(X)
#define hsGuardEnd

#else // HS_NO_TRY

#define hsGuardBegin(X)		{ const char* guardToken = X; try {
#define hsGuardEnd			} catch(...) { hsExceptionStack::Instance().Push(guardToken); throw; } }

#endif // HS_NO_TRY

#endif // hsExceptionStack_inc
