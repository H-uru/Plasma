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
#ifndef __plErrorMsg_h
#define __plErrorMsg_h

#include "hsTypes.h"

#define PL_ERR_MSG_MAX_MSG 2048

class plErrorMsg
{
public:
    static plErrorMsg *GetNull();

	plErrorMsg(const char* label, const char* msg);
	plErrorMsg(hsBool bogus = false);
	plErrorMsg(hsBool bogus, const char* label, const char* msg);
	plErrorMsg(hsBool bogus, const char* label, const char* format, const char* str);
	plErrorMsg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2);
	plErrorMsg(hsBool bogus, const char* label, const char* format, int n);
	plErrorMsg(hsBool bogus, const char* label, const char* format, int n, int m);
	plErrorMsg(hsBool bogus, const char* label, const char* format, float f);
    virtual ~plErrorMsg() { }

	plErrorMsg &Set(const char* label, const char* msg);
	plErrorMsg &Set(hsBool bogus = false);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* msg);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* format, const char* str);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* format, int n);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* format, int n, int m);
	plErrorMsg &Set(hsBool bogus, const char* label, const char* format, float f);

	hsBool IsBogus() { return GetBogus(); }
    // Ask - If condition is true and user says yes to displayed query, return true, else false
    virtual hsBool Ask() { return false; }

    // CheckAndAsk - If condition is true and user says YES, throw self.  Only asks if condition is true.
    // Returns true if condition is true but user says no, else false.
	virtual hsBool CheckAndAsk() { return false; }

     // CheckAskOrCancel - If condition is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
    virtual hsBool CheckAskOrCancel() { return false; }

    // Show - If condition is true, displays message, returns true
	virtual hsBool Show() { return false; } 

	// CheckAndShow - If condition is true, shows message box then throws self, else return false
	virtual hsBool CheckAndShow() { return false; }

    // Check - If condition was true, throws self, else return false
	virtual hsBool Check() { return false; }

     // Quit - If condition, quietly just throw with no message
	virtual void Quit() { }

protected:
    void SetBogus(hsBool b)   { fBogus = b; }

    hsBool GetBogus()         { return fBogus; }
    char *GetLabel()            { if (!fBogus) *fLabel = 0; return fLabel; }
    char *GetMsg()              { if (!fBogus) *fMsg = 0; return fMsg; }

private:
	hsBool		fBogus;
	char		fLabel[256];
	char		fMsg[PL_ERR_MSG_MAX_MSG];

private:
    // No assignment operator
    plErrorMsg &operator=(const plErrorMsg &msg);
};

#endif
