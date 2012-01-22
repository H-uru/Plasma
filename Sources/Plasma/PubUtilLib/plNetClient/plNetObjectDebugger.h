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
#ifndef plNetObjectDebugger_inc
#define plNetObjectDebugger_inc

#include "HeadSpin.h"
#include "hsStlUtils.h"
#include "pnKeyedObject/plUoid.h"
#include "pnNetCommon/plNetApp.h"

class hsKeyedObject;
class plStatusLog;
class plNetObjectDebugger : public plNetObjectDebuggerBase
{
public:
    enum Flags
    {
        kExactStringMatch   = 0x1,
        kEndStringMatch     = 0x2,
        kStartStringMatch   = 0x4,
        kSubStringMatch     = 0x8,
        kPageMatch          = 0x10      // has page info specified
    };
private:
    struct DebugObject
    {
        std::string fObjName;
        plLocation fLoc;
        uint32_t fFlags;
        bool StringMatches(const char* str) const;  // return true if string matches objName according to flags
        bool ObjectMatches(const hsKeyedObject* obj);
        bool ObjectMatches(const char* objName, const char* pageName);
        DebugObject(const char* objName, plLocation& loc, uint32_t flags);
    };
    typedef std::vector<DebugObject*> DebugObjectList;
    DebugObjectList fDebugObjects;
    mutable plStatusLog* fStatusLog;
    bool    fDebugging;

    void ICreateStatusLog() const;
public:
    plNetObjectDebugger();
    ~plNetObjectDebugger();

    static plNetObjectDebugger* GetInstance();

    bool GetDebugging() const { return fDebugging;  }
    void SetDebugging(bool b) { fDebugging=b;   }

    // object fxns
    bool AddDebugObject(const char* objName, const char* pageName=nil);
    bool RemoveDebugObject(const char* objName, const char* pageName=nil);
    void ClearAllDebugObjects();
    int GetNumDebugObjects() const { return fDebugObjects.size(); }
    bool IsDebugObject(const hsKeyedObject* obj) const;

    void LogMsgIfMatch(const char* msg) const;      // write to status log if there's a string match
    void LogMsg(const char* msg) const;
};

#endif      // plNetObjectDebugger_inc

