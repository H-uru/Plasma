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
#include "plLoggable.h"
#include "plStatusLog.h"
#include "hsStlUtils.h"
#include "hsTemplates.h"


plLoggable::~plLoggable() 
{ 
    IDeleteLog();
}

void plLoggable::IDeleteLog()
{
    if ( fWeCreatedLog )
        delete fStatusLog;  
    fWeCreatedLog = false;
    fStatusLog = nil;
}

plStatusLog* plLoggable::GetLog() const
{ 
    // create status log if necessary
    if(fStatusLog==nil)
    {
        ICreateStatusLog();     // Usually overridden by derived class
        if ( fStatusLog )
            fWeCreatedLog = true;
    }
#ifdef HS_DEBUGGING
    if ( fComplainAboutMissingLog )
    {
        hsAssert(fStatusLog, "nil fStatusLog.  Should override ICreateStatusLog()");
    }
#endif
    return fStatusLog;
}

void plLoggable::SetLog( plStatusLog * log, bool deleteOnDestruct/*=false */)
{
    IDeleteLog();
    fStatusLog = log;
    fWeCreatedLog = deleteOnDestruct;
}


bool plLoggable::Log( const char * str ) const
{
    if ( !str || strlen( str )==0 )
        return true;

    GetLog();

    if ( fStatusLog )
        return fStatusLog->AddLine( str );

    return true;
}

bool plLoggable::LogF( const char * fmt, ... ) const
{
    va_list args;
    va_start(args, fmt);
    return Log( xtl::formatv( fmt, args ).c_str() );
}

bool plLoggable::LogV( const char * fmt, va_list args ) const
{
    return Log( xtl::formatv( fmt, args ).c_str() );
}

bool plLoggable::DebugMsgV(const char* fmt, va_list args) const 
{
    return LogF("DBG: %s", xtl::formatv(fmt,args).c_str());
}

bool plLoggable::ErrorMsgV(const char* fmt, va_list args) const 
{
    return LogF("ERR: %s", xtl::formatv(fmt,args).c_str());
}

bool plLoggable::WarningMsgV(const char* fmt, va_list args) const 
{
    return LogF("WRN: %s", xtl::formatv(fmt,args).c_str());
}

bool plLoggable::AppMsgV(const char* fmt, va_list args) const 
{
    return LogF("APP: %s", xtl::formatv(fmt,args).c_str());
}

///////////////////////////////////////////////////////////////

bool plLoggable::DebugMsg(const char* fmt, ...) const 
{
    va_list args;
    va_start(args, fmt);
    return DebugMsgV(fmt, args);
}

bool plLoggable::ErrorMsg(const char* fmt, ...) const 
{
    va_list args;
    va_start(args, fmt);
    return ErrorMsgV(fmt, args);
}

bool plLoggable::WarningMsg(const char* fmt, ...) const 
{
    va_list args;
    va_start(args, fmt);
    return WarningMsgV(fmt, args);
}

bool plLoggable::AppMsg(const char* fmt, ...) const 
{
    va_list args;
    va_start(args, fmt);
    return AppMsgV(fmt, args);
}
