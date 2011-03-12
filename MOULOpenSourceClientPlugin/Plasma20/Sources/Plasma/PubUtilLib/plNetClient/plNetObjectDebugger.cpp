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
#include "plNetObjectDebugger.h"
#include "hsResMgr.h"
#include "hsTemplates.h"

#include "../pnUtils/pnUtils.h"
#include "../pnKeyedObject/hsKeyedObject.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plAgeLoader/plAgeLoader.h"

plNetObjectDebugger::DebugObject::DebugObject(const char* objName, plLocation& loc, UInt32 flags) : 
fLoc(loc), 
fFlags(flags)
{ 
	std::string tmp = objName;
	hsStrLower((char*)tmp.c_str());
	fObjName = tmp;
}

//
// return true if string matches objName according to flags
//
bool plNetObjectDebugger::DebugObject::StringMatches(const char* str) const
{
	if (!str)
		return false;

	if (fFlags & kExactStringMatch)
		return !stricmp(str, fObjName.c_str());

	if (fFlags & kEndStringMatch)
	{
		int len=strlen(str);
		if (len>fObjName.size())
			return false;
		return !stricmp(str, fObjName.c_str()+fObjName.size()-len);
	}

	if (fFlags & kStartStringMatch)
	{
		int len=strlen(str);
		if (len>fObjName.size())
			return false;
		return !strnicmp(str, fObjName.c_str(), strlen(str));
	}

	if (fFlags & kSubStringMatch)
	{
		std::string tmp = str;
		hsStrLower((char*)tmp.c_str());
		return (strstr(tmp.c_str(), fObjName.c_str()) != nil);
	}

	hsAssert(false, "missing flags");
	return false;
}

//
// if both objName and pageName are provided, and this object has page info,
//		return true if object matches both string and location.
// else just return true if object matches string
// 
bool plNetObjectDebugger::DebugObject::ObjectMatches(const char* objName, const char* pageName)
{
	if (!objName)
		return false;

	if (!pageName || (fFlags & kPageMatch)==0)
	{
		// only have enough info to match by objName
		return StringMatches(objName);
	}

	plLocation loc;
	loc = plKeyFinder::Instance().FindLocation(NetCommGetAge()->ageDatasetName, pageName);
	return (StringMatches(objName) && loc==fLoc);
}

//
// try to match by plLocation
//
bool plNetObjectDebugger::DebugObject::ObjectMatches(const hsKeyedObject* obj)
{
	if (!obj || !obj->GetKey())
		return false;

	if ((fFlags & kPageMatch)==0)
	{
		// match based on object name only
		return StringMatches(obj->GetKeyName());
	}

	return (obj->GetKey()->GetUoid().GetLocation()==fLoc);
}

/////////////////////////////////////////////////////////////////
// plNetObjectDebugger
/////////////////////////////////////////////////////////////////
plNetObjectDebugger::plNetObjectDebugger() : fStatusLog(nil), fDebugging(false)
{
}

plNetObjectDebugger::~plNetObjectDebugger() 
{ 
	ClearAllDebugObjects();	
	delete fStatusLog;
}

//
// STATIC
//
plNetObjectDebugger* plNetObjectDebugger::GetInstance()
{
	static plNetObjectDebugger gNetObjectDebugger;
	
	if (plNetObjectDebuggerBase::GetInstance()==nil)
		plNetObjectDebuggerBase::SetInstance(&gNetObjectDebugger);

	return &gNetObjectDebugger;
}

//
// create StatusLog if necessary
//
void plNetObjectDebugger::ICreateStatusLog() const
{
	if (!fStatusLog)
	{
		fStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "NetObject.log", 
			plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kTimestamp );
	}
}

bool plNetObjectDebugger::AddDebugObject(const char* objName, const char* pageName)
{
	if (!objName)
		return false;

	int size=strlen(objName)+1;
	hsTempArray<char> tmpObjName(size);
	memset(tmpObjName, 0, size);

	//
	// set string matching flags
	//
	int len = strlen(objName);
	UInt32 flags=0;
	if (objName[0]=='*')
	{
		if (objName[len-1]=='*')
		{
			flags = kSubStringMatch;	// *foo*
			strncpy(tmpObjName, objName+1, strlen(objName)-2);
		}
		else
		{
			flags = kEndStringMatch;	// *foo
			strncpy(tmpObjName, objName+1, strlen(objName)-1);
		}
	}

	if (!flags && objName[len-1]=='*')
	{
		flags = kStartStringMatch;		// foo*
		strncpy(tmpObjName, objName, strlen(objName)-1);
	}

	if (!flags)
	{
		flags = kExactStringMatch;
		strcpy(tmpObjName, objName);
	}

	//
	// set plLocation
	//
	plLocation loc;
	if (pageName)
	{
		loc = plKeyFinder::Instance().FindLocation(NetCommGetAge()->ageDatasetName, pageName);
		flags |= kPageMatch;
	}

	fDebugObjects.push_back(TRACKED_NEW DebugObject(tmpObjName, loc, flags));

	ICreateStatusLog();

	return true;
}

bool plNetObjectDebugger::RemoveDebugObject(const char* objName, const char* pageName)
{
	bool didIt=false;
	if (!pageName)
	{
		DebugObjectList::iterator it =fDebugObjects.begin();
		for( ; it != fDebugObjects.end(); )
		{
			if ( (*it) && (*it)->ObjectMatches(objName, pageName))
			{
				delete *it;
				it = fDebugObjects.erase(it);
				didIt=true;
			}
			else
				it++;
		}
	}

	return didIt;
}

void plNetObjectDebugger::ClearAllDebugObjects()
{
	DebugObjectList::iterator it =fDebugObjects.begin();
	for( ; it != fDebugObjects.end(); it++)
	{
		delete *it;
	}
	fDebugObjects.clear();
}

//
// write to status log if there's a string match
//
void plNetObjectDebugger::LogMsgIfMatch(const char* msg) const
{
	if (GetNumDebugObjects()==0 || !msg)
		return;

	// extract object name from msg, expects '...object:foo,...'
	std::string tmp = msg;
	hsStrLower((char*)tmp.c_str());
	std::string objTag="object";
	char* c=strstr(tmp.c_str(), objTag.c_str());
	if (c && c != tmp.c_str())
	{
		c+=objTag.size();

		// move past spaces
		while ( *c || *c==' ' )
			c++;

		char objName[128];
		int i=0;
		
		// copy objName token
		while(*c && *c != ',' && *c != ' ' && i<127)
			objName[i++] = *c++;	
		objName[i]=0;

		DebugObjectList::const_iterator it = fDebugObjects.begin();
		for( objName[0]; it != fDebugObjects.end(); it++)
		{
			if ((*it) && (*it)->StringMatches(objName))
			{
				LogMsg(msg);
				break;
			}
		}
	}
}

void plNetObjectDebugger::LogMsg(const char* msg) const
{
	DEBUG_MSG(msg);
}
	
bool plNetObjectDebugger::IsDebugObject(const hsKeyedObject* obj) const
{
	DebugObjectList::const_iterator it =fDebugObjects.begin();
	for( ; it != fDebugObjects.end(); it++)
		if ((*it) && (*it)->ObjectMatches(obj))
		{
			return true;
		}

	return false;
}
