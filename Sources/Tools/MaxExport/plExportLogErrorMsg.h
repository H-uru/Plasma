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
#ifndef plExportLogErrorMsg_inc
#define plExportLogErrorMsg_inc

//
// This plErrorMsg derivation will just log the error to a text file, instead of
// asking the user (because this used for unattended exporting).
// If there is no errors then there should be no file created.
//

#include <string.h>
#include "plErrorMsg.h"

#if !HS_BUILD_FOR_WIN32
#define PL_NULL_ERRMSG
#endif !HS_BUILD_FOR_WIN32

#ifndef PL_NULL_ERRMSG

#define ERROR_LOGFILE_NAME_LEN	512
class plExportLogErrorMsg : public plErrorMsg {
public:
    plExportLogErrorMsg(const char* efile, const char* label, const char* msg) : plErrorMsg(label, msg)
		{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus = false) : plErrorMsg(bogus)
		{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* msg) 
		: plErrorMsg(bogus, label, msg)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* format, const char* str) 
		: plErrorMsg(bogus, label, format, str)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plErrorMsg(bogus, label, format, str1, str2)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* format, int n) 
		: plErrorMsg(bogus, label, format, n)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* format, int n, int m) 
		: plErrorMsg(bogus, label, format, n, m)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
    plExportLogErrorMsg(const char* efile, hsBool bogus, const char* label, const char* format, float f) 
		: plErrorMsg(bogus, label, format, f)
			{ strncpy(fErrfile_name,efile,ERROR_LOGFILE_NAME_LEN-1); fErrfile=nil; }
	~plExportLogErrorMsg();

	virtual hsBool Ask(); // if b is true and user says yes to displayed query, return true, else false
	virtual hsBool CheckAndAsk(); // if b is true and user says YES, throw self. only asks if b is true. returns true if b is true but user says no, else false
	virtual hsBool CheckAskOrCancel(); // if b is true ( if YES, throw, else if NO return 0, else (CANCEL) return 1
	virtual hsBool Show(); // if b is true, displays message, returns true
	virtual hsBool Check(); // if b was true, throws self, else return false
	virtual hsBool CheckAndShow(); // if b was true, shows message box then throws self, else return false
	virtual void Quit(); // if b, quietly just throw with no message

protected:
	virtual void IWriteErrorFile(const char* label, const char* msg);
private:
	FILE	*fErrfile;			// the error file to write the nasties
	char	fErrfile_name[ERROR_LOGFILE_NAME_LEN];	// the name of the error file
	Int32	fNumberErrors;

private:
	void		IDebugThrow();
};
#else // PL_NULL_ERRMSG

class plExportLogErrorMsg : public plErrorMsg {
public:
	plExportLogErrorMsg(const char* label, const char* msg) : plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus = false) : plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* msg) 
		: plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* format, const char* str) 
		: plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* format, int n) 
		: plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* format, int n, int m) 
		: plErrorMsg() { }
	plExportLogErrorMsg(hsBool bogus, const char* label, const char* format, float f) 
		: plErrorMsg() { }
};
#endif // PL_NULL_ERRMSG

// Compile out error messages labeled debug
// #define PL_ERR_CHECK_DEGUG_ONLY
#if defined(NDEBUG) && defined(PL_ERR_CHECK_DEGUG_ONLY)

class plExportLogErrorDbg : public plExportLogErrorMsg {
public:
	plExportLogErrorDbg(const char* label, const char* msg) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus = false) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* msg) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, const char* str) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, int n) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, int n, int m) : plExportLogErrorMsg("") { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, float f) : plExportLogErrorMsg("") { }

	hsBool Ask() { return false; }
	hsBool CheckAndAsk() { return false; }
	hsBool CheckAskOrCancel();
	hsBool Show() { return false; }
	hsBool Check() { return false; }
	hsBool CheckAndShow() { return false; }
	void Quit() { }
};

#else // keep them as exactly the same as errormessage

class plExportLogErrorDbg : public plExportLogErrorMsg {
public:
	plExportLogErrorDbg(const char* label, const char* msg) : plExportLogErrorMsg("",label, msg) { }
	plExportLogErrorDbg(hsBool bogus = true) : plExportLogErrorMsg("",bogus) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* msg) 
		: plExportLogErrorMsg("",bogus, label, msg) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, const char* str) 
		: plExportLogErrorMsg("",bogus, label, format, str) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
		: plExportLogErrorMsg("",bogus, label, format, str1, str2) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, int n) 
		: plExportLogErrorMsg("",bogus, label, format, n) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, int n, int m) 
		: plExportLogErrorMsg("",bogus, label, format, n, m) { }
	plExportLogErrorDbg(hsBool bogus, const char* label, const char* format, float f) 
		: plExportLogErrorMsg("",bogus, label, format, f) { }
};

#endif // keep them as exactly the same as errormessage

#endif // plExportLogErrorMsg_inc
