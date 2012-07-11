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
#ifndef __plErrorMsg_h
#define __plErrorMsg_h

#include "HeadSpin.h"

#define PL_ERR_MSG_MAX_MSG 2048

class plErrorMsg
{
public:
    static plErrorMsg *GetNull();

    plErrorMsg(const char* label, const char* msg);
    plErrorMsg(bool bogus = false);
    plErrorMsg(bool bogus, const char* label, const char* msg);
    plErrorMsg(bool bogus, const char* label, const char* format, const char* str);
    plErrorMsg(bool bogus, const char* label, const char* format, const char* str1, const char* str2);
    plErrorMsg(bool bogus, const char* label, const char* format, int n);
    plErrorMsg(bool bogus, const char* label, const char* format, int n, int m);
    plErrorMsg(bool bogus, const char* label, const char* format, float f);
    virtual ~plErrorMsg() { }

    plErrorMsg &Set(const char* label, const char* msg);
    plErrorMsg &Set(bool bogus = false);
    plErrorMsg &Set(bool bogus, const char* label, const char* msg);
    plErrorMsg &Set(bool bogus, const char* label, const char* format, const char* str);
    plErrorMsg &Set(bool bogus, const char* label, const char* format, const char* str1, const char* str2);
    plErrorMsg &Set(bool bogus, const char* label, const char* format, int n);
    plErrorMsg &Set(bool bogus, const char* label, const char* format, int n, int m);
    plErrorMsg &Set(bool bogus, const char* label, const char* format, float f);

    bool IsBogus() { return GetBogus(); }
    // Ask - If condition is true and user says yes to displayed query, return true, else false
    virtual bool Ask() { return false; }

    // CheckAndAsk - If condition is true and user says YES, throw self.  Only asks if condition is true.
    // Returns true if condition is true but user says no, else false.
    virtual bool CheckAndAsk() { return false; }

     // CheckAskOrCancel - If condition is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
    virtual bool CheckAskOrCancel() { return false; }

    // Show - If condition is true, displays message, returns true
    virtual bool Show() { return false; } 

    // CheckAndShow - If condition is true, shows message box then throws self, else return false
    virtual bool CheckAndShow() { return false; }

    // Check - If condition was true, throws self, else return false
    virtual bool Check() { return false; }

     // Quit - If condition, quietly just throw with no message
    virtual void Quit() { }

protected:
    void SetBogus(bool b)   { fBogus = b; }

    bool GetBogus()         { return fBogus; }
    char *GetLabel()            { if (!fBogus) *fLabel = 0; return fLabel; }
    char *GetMsg()              { if (!fBogus) *fMsg = 0; return fMsg; }

private:
    bool        fBogus;
    char        fLabel[256];
    char        fMsg[PL_ERR_MSG_MAX_MSG];

private:
    // No assignment operator
    plErrorMsg &operator=(const plErrorMsg &msg);
};

#endif
