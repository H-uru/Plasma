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
#ifndef plKeyFinder_h_inc
#define plKeyFinder_h_inc

//----------------------------
// plKeyFinder 
//----------------------------
// provides a way to look up an object (via its plKey)
// Using strings.  The should only be used at Program Init time or console use (cause its not fast)
// The error codes are remembered, and used for subsequent calls to GetLastErrorString(); which can
// be display to tell the user where he screwed up the input.
// If a key is not found it returns nil

//----------------------------
// EXAMPLE OF USE:
//----------------------------
//
//	plKeyFinder *pFind = hsgResMgr::ResMgr()->GetKeyFinder();
//	plKey pKey = pFind->StupidSearch("Global", "Globalx", "1", "plSceneNode", "1");
//	if (!pKey)
//		hsStatusMessage(pFind->GetLastErrorString());
//	delete pFind;
//
//----------------------------

#include "hsTypes.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "hsStlUtils.h"
#include <string>

class plLocation;
class plRegistryPageNode;
class plPageInfo;

class plKeyFinder
{
public:	
	enum eErrCodes
	{
		kOk,
		kAgeNotFound,
		kPageNotFound,
		kInvalidClass,
		kNoClassesInPage,
		kObjectNotFound
	};

	static plKeyFinder& Instance();

	// These are Stupid search because they just do string searchs on the objects.
	plKey StupidSearch(const char * age, const char * rm, const char *className, const char *obName, hsBool subString=false);
	plKey StupidSearch(const char * age, const char * rm, UInt16 objType, const char *obName, hsBool subString=false);

	eErrCodes	GetLastErrorCode() { return fLastError; }
	const char*	GetLastErrorString(); // For Console display

	void ReallyStupidResponderSearch(const char* name, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);
	void ReallyStupidActivatorSearch(const char* name, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);

	void ReallyStupidSubstringSearch(const char* name, UInt16 objType, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);

	void GetActivatorNames(std::vector<std::string>& names);
	void GetResponderNames(std::vector<std::string>& names);

	plKey FindSceneNodeKey(const char* pageOrFullLocName) const;
	plKey FindSceneNodeKey(const char* ageName, const char* pageName) const;
	plKey FindSceneNodeKey(const plLocation& location) const;

	const plLocation& FindLocation(const char* age, const char* page) const;
	const plPageInfo* GetLocationInfo(const plLocation& loc) const;

protected:
	plKeyFinder() {}

	void IGetNames(std::vector<std::string>& names, const char* name, int index);
	plKey IFindSceneNodeKey(plRegistryPageNode* page) const;

	eErrCodes fLastError;
};

#endif // plKeyFinder_h_inc
