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
#ifndef plNetObjectDebugger_inc
#define plNetObjectDebugger_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnNetCommon/plNetApp.h"

class hsKeyedObject;
class plStatusLog;
class plNetObjectDebugger : public plNetObjectDebuggerBase
{
public:
	enum Flags
	{
		kExactStringMatch	= 0x1,
		kEndStringMatch		= 0x2,
		kStartStringMatch	= 0x4,
		kSubStringMatch		= 0x8,
		kPageMatch			= 0x10		// has page info specified
	};
private:
	struct DebugObject
	{
		std::string fObjName;
		plLocation fLoc;
		UInt32 fFlags;
		bool StringMatches(const char* str) const;	// return true if string matches objName according to flags
		bool ObjectMatches(const hsKeyedObject* obj);
		bool ObjectMatches(const char* objName, const char* pageName);
		DebugObject(const char* objName, plLocation& loc, UInt32 flags);
	};
	typedef std::vector<DebugObject*> DebugObjectList;
	DebugObjectList fDebugObjects;
	mutable plStatusLog* fStatusLog;
	bool	fDebugging;

	void ICreateStatusLog() const;
public:
	plNetObjectDebugger();
	~plNetObjectDebugger();

	static plNetObjectDebugger* GetInstance();

	bool GetDebugging() const { return fDebugging;	}
	void SetDebugging(bool b) { fDebugging=b;	}

	// object fxns
	bool AddDebugObject(const char* objName, const char* pageName=nil);
	bool RemoveDebugObject(const char* objName, const char* pageName=nil);
	void ClearAllDebugObjects();
	int GetNumDebugObjects() const { return fDebugObjects.size(); }
	bool IsDebugObject(const hsKeyedObject* obj) const;

	void LogMsgIfMatch(const char* msg) const;		// write to status log if there's a string match
	void LogMsg(const char* msg) const;
};

#endif		// plNetObjectDebugger_inc

