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
#ifndef plLoggable_inc
#define plLoggable_inc

#include "HeadSpin.h"
#include "plString.h"
#include "plFormat.h"


// Stub interface implemented by plStatusLog
class plLog
{
public:
    virtual ~plLog() { }

    virtual bool AddLine(const plString& line) = 0;
};


// An abstract base class which contains a status log and simple functions 
// for writing Debug and Error msgs to the log
class plLoggable
{
protected:
    mutable plLog*  fStatusLog;
    mutable bool    fWeCreatedLog;
    mutable bool    fComplainAboutMissingLog;


    // call plStatusLogMgr::CreateStatusLog with the options you want
    virtual void ICreateStatusLog() const { };

    void IDeleteLog()
    {
        if (fWeCreatedLog) {
            delete fStatusLog;
        }

        fWeCreatedLog = false;
        fStatusLog = nullptr;
    }

public:
    plLoggable()
        : fStatusLog(nullptr), fWeCreatedLog(false),
          fComplainAboutMissingLog(true)
    { }

    virtual ~plLoggable()
    {
        IDeleteLog();
    }


    plLog* GetLog() const
    {
        // create status log if necessary
        if (fStatusLog == nullptr) {
            ICreateStatusLog(); // Usually overridden by derived class

            if (fStatusLog) {
                fWeCreatedLog = true;
            }
        }

        #ifdef HS_DEBUGGING
        if (fComplainAboutMissingLog) {
            hsAssert(fStatusLog, "null log: override ICreateStatusLog()");
        }
        #endif

        return fStatusLog;
    }


    void SetLog(plLog* log, bool deleteOnDestruct=false)
    {
        IDeleteLog();

        fStatusLog = log;
        fWeCreatedLog = deleteOnDestruct;
    }

    // logging
    virtual bool Log(const plString& str) const
    {
        if (str.IsNull() || str.IsEmpty()) {
            return true;
        }

        GetLog();

        if (fStatusLog) {
            return fStatusLog->AddLine(str);
        }

        return true;
    }


    virtual bool ErrorMsg(const plString& msg) const
    {
        return Log(plFormat("ERR: {}", msg));
    }

    virtual bool WarningMsg(const plString& msg) const
    {
        return Log(plFormat("WRN: {}", msg));
    }

    virtual bool AppMsg(const plString& msg) const
    {
        return Log(plFormat("APP: {}", msg));
    }

    virtual bool DebugMsg(const plString& msg) const
    {
        return Log(plFormat("DBG: {}", msg));
    }



    /////////////////////////////////////////////////////////////////////////
    // DEPRECATED Log methods
    /////////////////////////////////////////////////////////////////////////

    hsDeprecated("plLoggable::LogF is deprecated -- use plFormat instead")
    virtual bool LogF( const char * fmt, ... ) const
    {
        va_list args;
        va_start(args, fmt);
        bool ret = Log(plString::IFormat(fmt, args));
        va_end(args);

        return ret;
    }

    virtual bool LogV( const char * fmt, va_list args ) const
    {
        return Log(plString::IFormat(fmt, args));
    }

    virtual bool ErrorMsgV(const char* fmt, va_list args) const
    {
        return ErrorMsg(plString::IFormat(fmt, args));
    }

    virtual bool DebugMsgV(const char* fmt, va_list args) const
    {
        return DebugMsg(plString::IFormat(fmt, args));
    }

    virtual bool WarningMsgV(const char* fmt, va_list args) const
    {
        return WarningMsg(plString::IFormat(fmt, args));
    }

    virtual bool AppMsgV(const char* fmt, va_list args) const
    {
        return AppMsg(plString::IFormat(fmt, args));
    }

    hsDeprecated("plLoggable::ErrorMsg with format is deprecated -- use plFormat instead")
    virtual bool ErrorMsg(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        bool ret = ErrorMsgV(fmt, args);
        va_end(args);

        return ret;
    }

    hsDeprecated("plLoggable::DebugMsg with format is deprecated -- use plFormat instead")
    virtual bool DebugMsg(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        bool ret = DebugMsgV(fmt, args);
        va_end(args);

        return ret;
    }

    hsDeprecated("plLoggable::WarningMsg with format is deprecated -- use plFormat instead")
    virtual bool WarningMsg(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        bool ret = WarningMsgV(fmt, args);
        va_end(args);

        return ret;
    }

    hsDeprecated("plLoggable::AppMsg with format is deprecated -- use plFormat instead")
    virtual bool AppMsg(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        bool ret = AppMsgV(fmt, args);
        va_end(args);

        return ret;
    }
};

#endif  // plLoggable_inc
