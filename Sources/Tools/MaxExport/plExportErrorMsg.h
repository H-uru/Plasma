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

#include <string.h>
#include "plErrorMsg.h"

#if !HS_BUILD_FOR_WIN32
#define PL_NULL_ERRMSG
#endif !HS_BUILD_FOR_WIN32

#ifndef PL_NULL_ERRMSG

class plExportErrorMsg : public plErrorMsg {
public:
    plExportErrorMsg(const char* label, const char* msg) : plErrorMsg(label, msg) { }
    plExportErrorMsg(bool bogus = false) : plErrorMsg(bogus) { }
    plExportErrorMsg(bool bogus, const char* label, const char* msg) 
        : plErrorMsg(bogus, label, msg) { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, const char* str) 
        : plErrorMsg(bogus, label, format, str) { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, const char* str1, const char* str2) 
        : plErrorMsg(bogus, label, format, str1, str2) { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, int n) 
        : plErrorMsg(bogus, label, format, n) { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, int n, int m) 
        : plErrorMsg(bogus, label, format, n, m) { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, float f) 
        : plErrorMsg(bogus, label, format, f) { }

    virtual bool Ask(); // if b is true and user says yes to displayed query, return true, else false
    virtual bool CheckAndAsk(); // if b is true and user says YES, throw self. only asks if b is true. returns true if b is true but user says no, else false
    virtual bool CheckAskOrCancel(); // if b is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
    virtual bool Show(); // if b is true, displays message, returns true
    virtual bool Check(); // if b was true, throws self, else return false
    virtual bool CheckAndShow(); // if b was true, shows message box then throws self, else return false
    virtual void Quit(); // if b, quietly just throw with no message

private:
    void        IDebugThrow();
};
#else // PL_NULL_ERRMSG

class plExportErrorMsg : public plErrorMsg {
public:
    plExportErrorMsg(const char* label, const char* msg) : plErrorMsg() { }
    plExportErrorMsg(bool bogus = false) : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* msg) 
        : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, const char* str) 
        : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, const char* str1, const char* str2) 
        : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, int n) 
        : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, int n, int m) 
        : plErrorMsg() { }
    plExportErrorMsg(bool bogus, const char* label, const char* format, float f) 
        : plErrorMsg() { }
};
#endif // PL_NULL_ERRMSG

// Compile out error messages labeled debug
// #define PL_ERR_CHECK_DEGUG_ONLY
#if defined(NDEBUG) && defined(PL_ERR_CHECK_DEGUG_ONLY)

class plExportErrorDbg : public plExportErrorMsg {
public:
    plExportErrorDbg(const char* label, const char* msg) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus = false) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* msg) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, const char* str) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, const char* str1, const char* str2) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, int n) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, int n, int m) : plExportErrorMsg() { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, float f) : plExportErrorMsg() { }

    bool Ask() { return false; }
    bool CheckAndAsk() { return false; }
    bool CheckAskOrCancel();
    bool Show() { return false; }
    bool Check() { return false; }
    bool CheckAndShow() { return false; }
    void Quit() { }
};

#else // keep them as exactly the same as errormessage

class plExportErrorDbg : public plExportErrorMsg {
public:
    plExportErrorDbg(const char* label, const char* msg) : plExportErrorMsg(label, msg) { }
    plExportErrorDbg(bool bogus = true) : plExportErrorMsg(bogus) { }
    plExportErrorDbg(bool bogus, const char* label, const char* msg) 
        : plExportErrorMsg(bogus, label, msg) { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, const char* str) 
        : plExportErrorMsg(bogus, label, format, str) { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, const char* str1, const char* str2) 
        : plExportErrorMsg(bogus, label, format, str1, str2) { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, int n) 
        : plExportErrorMsg(bogus, label, format, n) { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, int n, int m) 
        : plExportErrorMsg(bogus, label, format, n, m) { }
    plExportErrorDbg(bool bogus, const char* label, const char* format, float f) 
        : plExportErrorMsg(bogus, label, format, f) { }
};

#endif // keep them as exactly the same as errormessage

#endif // plErrMsg_inc
