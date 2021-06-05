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
#ifndef plExportErrorMsg_inc
#define plExportErrorMsg_inc

#include "plErrorMsg.h"

#if !HS_BUILD_FOR_WIN32
#define PL_NULL_ERRMSG
#endif !HS_BUILD_FOR_WIN32

#ifndef PL_NULL_ERRMSG

class plExportErrorMsg : public plErrorMsg {
public:
    bool Ask() override; // if b is true and user says yes to displayed query, return true, else false
    bool CheckAndAsk() override; // if b is true and user says YES, throw self. only asks if b is true. returns true if b is true but user says no, else false
    bool CheckAskOrCancel() override; // if b is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
    bool Show() override; // if b is true, displays message, returns true
    bool Check() override; // if b was true, throws self, else return false
    bool CheckAndShow() override; // if b was true, shows message box then throws self, else return false
    void Quit() override; // if b, quietly just throw with no message

private:
    void        IDebugThrow();
};
#else // PL_NULL_ERRMSG

class plExportErrorMsg : public plErrorMsg {
};
#endif // PL_NULL_ERRMSG

// Compile out error messages labeled debug
// #define PL_ERR_CHECK_DEGUG_ONLY
#if defined(NDEBUG) && defined(PL_ERR_CHECK_DEGUG_ONLY)

class plExportErrorDbg : public plExportErrorMsg {
public:
    bool Ask() override { return false; }
    bool CheckAndAsk() override { return false; }
    bool CheckAskOrCancel() override;
    bool Show() override { return false; }
    bool Check() override { return false; }
    bool CheckAndShow() override { return false; }
    void Quit() override { }
};

#else // keep them as exactly the same as errormessage

class plExportErrorDbg : public plExportErrorMsg {
};

#endif // keep them as exactly the same as errormessage

#endif // plErrMsg_inc
