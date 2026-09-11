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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
//  plKeyFinder *pFind = hsgResMgr::ResMgr()->GetKeyFinder();
//  plKey pKey = pFind->StupidSearch("Global", "Globalx", "1", "plSceneNode", "1");
//  if (!pKey)
//      hsStatusMessage(pFind->GetLastErrorString());
//  delete pFind;
//
//----------------------------

#include "HeadSpin.h"
#include <vector>

#include "pnKeyedObject/plUoid.h"

class plKey;
class plPageInfo;
class plRegistryPageNode;

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
    plKey StupidSearch(const ST::string & age, const ST::string & rm, const char *className, const ST::string &obName, bool subString=false);
    plKey StupidSearch(const ST::string & age, const ST::string & rm, uint16_t objType, const ST::string &obName, bool subString=false);

    eErrCodes   GetLastErrorCode() { return fLastError; }
    const char* GetLastErrorString(); // For Console display

    void ReallyStupidResponderSearch(const ST::string& name, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);
    void ReallyStupidActivatorSearch(const ST::string& name, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);

    void ReallyStupidSubstringSearch(const ST::string& name, uint16_t objType, std::vector<plKey>& foundKeys, const plLocation& hintLocation = plLocation::kInvalidLoc);

    void GetActivatorNames(std::vector<ST::string>& names);
    void GetResponderNames(std::vector<ST::string>& names);

    plKey FindSceneNodeKey(const ST::string& pageOrFullLocName) const;
    plKey FindSceneNodeKey(const ST::string& ageName, const ST::string& pageName) const;
    plKey FindSceneNodeKey(const plLocation& location) const;

    const plLocation& FindLocation(const ST::string& age, const ST::string& page) const;
    const plPageInfo* GetLocationInfo(const plLocation& loc) const;

protected:
    plKeyFinder() : fLastError(kOk) { }

    void IGetNames(std::vector<ST::string>& names, const ST::string& name, int index);
    plKey IFindSceneNodeKey(plRegistryPageNode* page) const;

    eErrCodes fLastError;
};

#endif // plKeyFinder_h_inc
