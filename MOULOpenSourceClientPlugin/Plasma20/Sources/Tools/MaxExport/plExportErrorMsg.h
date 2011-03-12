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
    plExportErrorMsg(hsBool bogus = false) : plErrorMsg(bogus) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* msg) 
		: plErrorMsg(bogus, label, msg) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* format, const char* str) 
		: plErrorMsg(bogus, label, format, str) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plErrorMsg(bogus, label, format, str1, str2) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* format, int n) 
		: plErrorMsg(bogus, label, format, n) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* format, int n, int m) 
		: plErrorMsg(bogus, label, format, n, m) { }
    plExportErrorMsg(hsBool bogus, const char* label, const char* format, float f) 
		: plErrorMsg(bogus, label, format, f) { }

	virtual hsBool Ask(); // if b is true and user says yes to displayed query, return true, else false
	virtual hsBool CheckAndAsk(); // if b is true and user says YES, throw self. only asks if b is true. returns true if b is true but user says no, else false
	virtual hsBool CheckAskOrCancel(); // if b is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
	virtual hsBool Show(); // if b is true, displays message, returns true
	virtual hsBool Check(); // if b was true, throws self, else return false
	virtual hsBool CheckAndShow(); // if b was true, shows message box then throws self, else return false
	virtual void Quit(); // if b, quietly just throw with no message

private:
	void		IDebugThrow();
};
#else // PL_NULL_ERRMSG

class plExportErrorMsg : public plErrorMsg {
public:
	plExportErrorMsg(const char* label, const char* msg) : plErrorMsg() { }
	plExportErrorMsg(hsBool bogus = false) : plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* msg) 
		: plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* format, const char* str) 
		: plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* format, int n) 
		: plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* format, int n, int m) 
		: plErrorMsg() { }
	plExportErrorMsg(hsBool bogus, const char* label, const char* format, float f) 
		: plErrorMsg() { }
};
#endif // PL_NULL_ERRMSG

// Compile out error messages labeled debug
// #define PL_ERR_CHECK_DEGUG_ONLY
#if defined(NDEBUG) && defined(PL_ERR_CHECK_DEGUG_ONLY)

class plExportErrorDbg : public plExportErrorMsg {
public:
	plExportErrorDbg(const char* label, const char* msg) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus = false) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* msg) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, const char* str) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, int n) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, int n, int m) : plExportErrorMsg() { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, float f) : plExportErrorMsg() { }

	hsBool Ask() { return false; }
	hsBool CheckAndAsk() { return false; }
	hsBool CheckAskOrCancel();
	hsBool Show() { return false; }
	hsBool Check() { return false; }
	hsBool CheckAndShow() { return false; }
	void Quit() { }
};

#else // keep them as exactly the same as errormessage

class plExportErrorDbg : public plExportErrorMsg {
public:
	plExportErrorDbg(const char* label, const char* msg) : plExportErrorMsg(label, msg) { }
	plExportErrorDbg(hsBool bogus = true) : plExportErrorMsg(bogus) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* msg) 
		: plExportErrorMsg(bogus, label, msg) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, const char* str) 
		: plExportErrorMsg(bogus, label, format, str) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plExportErrorMsg(bogus, label, format, str1, str2) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, int n) 
		: plExportErrorMsg(bogus, label, format, n) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, int n, int m) 
		: plExportErrorMsg(bogus, label, format, n, m) { }
	plExportErrorDbg(hsBool bogus, const char* label, const char* format, float f) 
		: plExportErrorMsg(bogus, label, format, f) { }
};

#endif // keep them as exactly the same as errormessage

#endif // plErrMsg_inc
