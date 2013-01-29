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

//
// An abstract base class which contains a status log and simple functions 
// for writing Debug and Error msgs to the log
//
class plStatusLog;
class plLoggable
{
protected:
    mutable plStatusLog* fStatusLog;
    mutable bool    fWeCreatedLog;
    mutable bool    fComplainAboutMissingLog;

    virtual void ICreateStatusLog() const { };  // call plStatusLogMgr::CreateStatusLog with the options you want
    void    IDeleteLog();

public:
    plLoggable() : fStatusLog(nil), fWeCreatedLog(false),fComplainAboutMissingLog(true) {   }
    virtual ~plLoggable();

    plStatusLog* GetLog() const;
    void SetLog( plStatusLog * log, bool deleteOnDestruct=false );

    // logging

    virtual bool Log( const char * str ) const;
    virtual bool Log(const plString& str) const;
    virtual bool LogF( const char * fmt, ... ) const;
    virtual bool LogV( const char * fmt, va_list args ) const;
    virtual bool ErrorMsgV(const char* fmt, va_list args) const ;
    virtual bool DebugMsgV(const char* fmt, va_list args) const ;
    virtual bool WarningMsgV(const char* fmt, va_list args) const ;
    virtual bool AppMsgV(const char* fmt, va_list args) const ;
    virtual bool ErrorMsg(const char* fmt, ...) const ;
    virtual bool DebugMsg(const char* fmt, ...) const ;
    virtual bool WarningMsg(const char* fmt, ...) const ;
    virtual bool AppMsg(const char* fmt, ...) const ;
};

#endif  // plLoggable_inc
