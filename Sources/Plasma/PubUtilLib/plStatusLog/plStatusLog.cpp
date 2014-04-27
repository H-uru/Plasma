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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plStatusLog Functions                                                   //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  10.02.2001 mcn  - Created                                               //
//  10.16.2001 mcn  - Added static versions of AddLine() so you don't need  //
//                    to explicitly create a log                            //
//  10.17.2001 mcn  - Added support for carriage returns in printed lines   //
//  10.24.2002 eap  - Added kDebugOutput flag for writing to debug window   //
//  10.25.2002 eap  - Updated to work under unix                            //
//  12.13.2002 eap  - Added kStdout flag                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <cstdarg>
#include <cstdlib>
#include "hsThread.h"
#include "hsTemplates.h"
#include "hsTimer.h"
#include "plStatusLog.h"
#include "plUnifiedTime/plUnifiedTime.h"
#include "plProduct.h"

#include "plEncryptLogLine.h"

#if HS_BUILD_FOR_WIN32
    #include <shlobj.h>
#endif

//////////////////////////////////////////////////////////////////////////////
//// plStatusLogMgr Stuff ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plFileName plStatusLogMgr::IGetBasePath()
{
    static plFileName _basePath = plFileSystem::GetLogPath();
    return _basePath;
}

//// Constructor & Destructor ////////////////////////////////////////////////

plStatusLogMgr::plStatusLogMgr()
{
    fDisplays = nil;
    fCurrDisplay = nil;
    fDrawer = nil;
    fLastLogChangeTime = 0;
}

plStatusLogMgr::~plStatusLogMgr()
{
    // Unlink all the displays, but don't delete them; leave that to whomever owns them
    while( fDisplays != nil )
    {
        plStatusLog *log = fDisplays;

        fDisplays->IUnlink();

        if( log->fFlags & plStatusLog::kDeleteForMe )
            delete log;
    }
}

plStatusLogMgr  &plStatusLogMgr::GetInstance( void )
{
    static plStatusLogMgr   theManager;
    return theManager;
}

//// Draw ////////////////////////////////////////////////////////////////////

void    plStatusLogMgr::Draw( void )
{
    /// Just draw current plStatusLog
    if( fCurrDisplay != nil && fDrawer != nil )
    {
        plStatusLog* firstLog = nil;
        if (hsTimer::GetSysSeconds() - fLastLogChangeTime < 1)
            firstLog = fDisplays;

        fDrawer->Draw(fCurrDisplay, firstLog);
    }
}

//// CreateStatusLog /////////////////////////////////////////////////////////

plStatusLog *plStatusLogMgr::CreateStatusLog( uint8_t numDisplayLines, const plFileName &filename, uint32_t flags )
{
    plFileSystem::CreateDir(IGetBasePath(), true);
    plStatusLog *log = new plStatusLog( numDisplayLines, filename, flags );

    // Put the new log in its alphabetical position
    plStatusLog** nextLog = &fDisplays;
    while (*nextLog)
    {
        if (filename.AsString().CompareI((*nextLog)->GetFileName().AsString()) <= 0)
            break;
        nextLog = &(*nextLog)->fNext;
    }
    log->ILink(nextLog);

    log->fDisplayPointer = &fCurrDisplay;

    return log;
}

//// ToggleStatusLog /////////////////////////////////////////////////////////

void    plStatusLogMgr::ToggleStatusLog( plStatusLog *logToDisplay )
{
    if( fCurrDisplay == logToDisplay )
        fCurrDisplay = nil;
    else
        fCurrDisplay = logToDisplay;

    fLastLogChangeTime = hsTimer::GetSysSeconds();
}

//// SetCurrStatusLog ////////////////////////////////////////////////////////

void plStatusLogMgr::SetCurrStatusLog(const plFileName& logName)
{
    plStatusLog* log = FindLog(logName, false);
    if (log != nil)
        fCurrDisplay = log;
}

//// NextStatusLog ///////////////////////////////////////////////////////////

void    plStatusLogMgr::NextStatusLog( void )
{
    if( fCurrDisplay == nil )
        fCurrDisplay = fDisplays;
    else
        fCurrDisplay = fCurrDisplay->fNext;

    fLastLogChangeTime = hsTimer::GetSysSeconds();
}

void    plStatusLogMgr::PrevStatusLog( void )
{
    if( fCurrDisplay == nil )
    {
        fCurrDisplay = fDisplays;
        while (fCurrDisplay && fCurrDisplay->fNext)
            fCurrDisplay = fCurrDisplay->fNext;
    }
    else
    {
        plStatusLog* lastLog = fDisplays;
        while (lastLog && lastLog->fNext != fCurrDisplay)
            lastLog = lastLog->fNext;

        fCurrDisplay = lastLog;
    }

    fLastLogChangeTime = hsTimer::GetSysSeconds();
}

//// FindLog ////////////////////////////////////////////////////////////////

plStatusLog *plStatusLogMgr::FindLog( const plFileName &filename, bool createIfNotFound )
{
    plStatusLog *log = fDisplays;

    while( log != nil )
    {
        if (log->GetFileName().AsString().CompareI(filename.AsString()) == 0)
            return log;

        log = log->fNext;
    }

    if( !createIfNotFound )
        return nil;

    // Didn't find one, so create one! (make it a nice default one :)
    log = CreateStatusLog( kDefaultNumLines, filename, plStatusLog::kFilledBackground |
                                                       plStatusLog::kDeleteForMe );

    return log;
}

//// BounceLogs ///////////////////////////////////////////////////////////////

void plStatusLogMgr::BounceLogs()
{
    plStatusLog *log = fDisplays;

    while( log != nil )
    {
        plStatusLog * tmp = log;
        log = log->fNext;
        tmp->Bounce();
    }
}

//// DumpLogs ////////////////////////////////////////////////////////////////

bool plStatusLogMgr::DumpLogs( const plFileName &newFolderName )
{
    bool retVal = true; // assume success
    plFileName newPath;
    plFileName basePath = IGetBasePath();
    if (basePath.IsValid())
        newPath = plFileName::Join(basePath, newFolderName);
    else
        newPath = newFolderName;
    plFileSystem::CreateDir(newPath, true);

    std::vector<plFileName> files = plFileSystem::ListDir(basePath);
    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        plFileName destination = plFileName::Join(newPath, iter->GetFileName());
        retVal = plFileSystem::Copy(*iter, destination);
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
//// plStatusLog ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

uint32_t plStatusLog::fLoggingOff = false;

plStatusLog::plStatusLog( uint8_t numDisplayLines, const plFileName &filename, uint32_t flags )
{
    fFileHandle = nil;
    fSema = nil;
    fSize = 0;
    fForceLog = false;

    fMaxNumLines = numDisplayLines;
    if (filename.IsValid())
    {
        fFilename = filename;
        fSema = new hsGlobalSemaphore(1, fFilename.AsString().c_str());
    }
    else
    {
        fFilename = "";
        flags |= kDontWriteFile;

        fSema = new hsGlobalSemaphore(1);
    }

    fOrigFlags = fFlags = flags;

    IInit();
}

plStatusLog::~plStatusLog()
{
    IFini();
}

void    plStatusLog::IInit()
{
    int i;

    fFlags = fOrigFlags;

    fLines = new char *[ fMaxNumLines ];
    fColors = new uint32_t[ fMaxNumLines ];
    for( i = 0; i < fMaxNumLines; i++ )
    {
        fLines[ i ] = nil;
        fColors[ i ] = kWhite;
    }

    fNext = nil;
    fBack = nil;

}

bool plStatusLog::IReOpen( void )
{
    if( fFileHandle != nil )
    {
        fclose( fFileHandle );
        fFileHandle = nil;
    }

    // Open the file, clearing it, if necessary
    if(!(fFlags & kDontWriteFile))
    {
        plFileName fileNoExt;
        plString ext;
        IParseFileName(fileNoExt, ext);
        plFileName fileToOpen = plString::Format("%s.0.%s", fileNoExt.AsString().c_str(), ext.c_str());
        if (!(fFlags & kDontRotateLogs))
        {
            plFileName work, work2;
            work = plString::Format("%s.3.%s", fileNoExt.AsString().c_str(), ext.c_str());
            plFileSystem::Unlink(work);
            work2 = plString::Format("%s.2.%s", fileNoExt.AsString().c_str(), ext.c_str());
            plFileSystem::Move(work2, work);
            work = plString::Format("%s.1.%s", fileNoExt.AsString().c_str(), ext.c_str());
            plFileSystem::Move(work, work2);
            plFileSystem::Move(fileToOpen, work);
        }
        
        if (fFlags & kAppendToLast)
        {
            fFileHandle = plFileSystem::Open(fileToOpen, "at");
        }
        else
        {
            fFileHandle = plFileSystem::Open(fileToOpen, "wt");
            // if we need to reopen lets just append
            fFlags |= kAppendToLast;
        }
    }

    if (fFileHandle)
        fSize = ftell( fFileHandle );
    else
        fSize = 0;

    return fFileHandle != nil;
}

void    plStatusLog::IFini( void )
{
    int     i;

    if( fFileHandle != nil )
    {
        fclose( fFileHandle );
        fFileHandle = nil;
    }

    if( *fDisplayPointer == this )
        *fDisplayPointer = nil;

    if( fBack != nil || fNext != nil )
        IUnlink();

    for( i = 0; i < fMaxNumLines; i++ )
        delete [] fLines[ i ];

    if (fSema)
        delete fSema;

    delete [] fLines;
    delete [] fColors;
}

void plStatusLog::IParseFileName(plFileName& fileNoExt, plString& ext) const
{
    plFileName base = plStatusLogMgr::IGetBasePath();
    plFileName file;
    if (base.IsValid())
        file = plFileName::Join(base, fFilename);
    else
        file = fFilename;

    plFileSystem::CreateDir(file.StripFileName(), true);

    // apache-style file backup
    fileNoExt = file.StripFileExt();
    ext = file.GetFileExt();
}

//// IUnlink /////////////////////////////////////////////////////////////////

void    plStatusLog::IUnlink( void )
{
    hsAssert( fBack, "plStatusLog not in list" );
    if( fNext )
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nil;
    fNext = nil;
}

//// ILink ///////////////////////////////////////////////////////////////////

void    plStatusLog::ILink( plStatusLog **back )
{
    hsAssert( fNext == nil && fBack == nil, "Trying to link a plStatusLog that's already linked" );

    fNext = *back;
    if( *back )
        (*back)->fBack = &fNext;
    fBack = back;
    *back = this;
}

//// IAddLine ////////////////////////////////////////////////////////////////
//  Actually add a stinking line.

bool plStatusLog::IAddLine( const char *line, int32_t count, uint32_t color )
{
    int     i;

    if(fLoggingOff && !fForceLog)
        return true;

    /// Scroll pointers up
    fSema->Wait();

    bool ret = true;

    if (fMaxNumLines > 0)
    {
        delete [] fLines[ 0 ];
        for( i = 0; i < fMaxNumLines - 1; i++ )
        {
            fLines[ i ] = fLines[ i + 1 ];
            fColors[ i ] = fColors[ i + 1 ];
        }
    }

    /// Add new
    if( line == nil || strlen( line ) == 0 )
    {
        if (fMaxNumLines > 0)
        {
            fColors[ i ] = 0;
            fLines[ i ] = nil;
        }
        ret = IPrintLineToFile( "", 0 );
    }
    else
    {
        if( count < 0 )
            count = strlen( line );

        if (fMaxNumLines > 0)
        {
            fLines[ i ] = new char[ count + 1 ];
            hsStrncpy( fLines[ i ], line, count + 1 );
            fLines[ i ][ count ] = 0;

            char *c = strchr( fLines[ i ], '\n' );
            if( c != nil )
            {
                *c = 0;
                count--;
            }

            fColors[ i ] = color;
        }

        ret = IPrintLineToFile( line, count );
    }

    fSema->Signal();

    return ret;
}

//// AddLine /////////////////////////////////////////////////////////////////

bool plStatusLog::AddLine( const char *line, uint32_t color )
{
    char    *c, *str;
    if(fLoggingOff && !fForceLog)
        return true;

    bool ret = true;

    /// Scan for carriage returns and feed each section into IAddLine()
    for( str = (char *)line; ( c = strchr( str, '\n' ) ) != nil; str = c + 1 )
    {
        // So if we got here, c points to a carriage return...
        ret = IAddLine( str, (uintptr_t)c - (uintptr_t)str, color );
    }

    /// We might have some left over
    if( strlen( str ) > 0 )
    {
        ret &= IAddLine( str, -1, color );
    }

    return ret;
}

//// AddLine printf-style Variations /////////////////////////////////////////

bool plStatusLog::AddLineV( const char *format, va_list arguments )
{
    if(fLoggingOff && !fForceLog)
        return true;
    return AddLineV( kWhite, format, arguments );
}

bool plStatusLog::AddLineV( uint32_t color, const char *format, va_list arguments )
{
    if(fLoggingOff && !fForceLog)
        return true;
    char buffer[2048];
    vsnprintf(buffer, arrsize(buffer), format, arguments);
    return AddLine( buffer, color );
}

bool plStatusLog::AddLineF( const char *format, ... )
{
    if(fLoggingOff && !fForceLog)
        return true;
    va_list arguments;
    va_start( arguments, format );

    return AddLineV( kWhite, format, arguments );
}

bool plStatusLog::AddLineF( uint32_t color, const char *format, ... )
{
    if(fLoggingOff && !fForceLog)
        return true;
    va_list arguments;
    va_start( arguments, format );

    return AddLineV( color, format, arguments );
}

//// AddLine Static Variations ///////////////////////////////////////////////

bool plStatusLog::AddLineS( const plFileName &filename, const char *format, ... )
{
    plStatusLog *log = plStatusLogMgr::GetInstance().FindLog( filename );
    if (!log)
        return false;

    if(fLoggingOff && !log->fForceLog)
        return true;

    va_list arguments;
    va_start( arguments, format );

    return log->AddLineV( format, arguments );
}

bool plStatusLog::AddLineS( const plFileName &filename, uint32_t color, const char *format, ... )
{
    plStatusLog *log = plStatusLogMgr::GetInstance().FindLog( filename );
    if (!log)
        return false;

    if(fLoggingOff && !log->fForceLog)
        return true;

    va_list arguments;
    va_start( arguments, format );

    return log->AddLineV( color, format, arguments );
}

//// Clear ///////////////////////////////////////////////////////////////////

void    plStatusLog::Clear( void )
{
    int     i;


    for( i = 0; i < fMaxNumLines; i++ )
    {
        delete [] fLines[ i ];
        fLines[ i ] = nil;
    }
}


//// Bounce //////////////////////////////////////////////////////////////////

void    plStatusLog::Bounce( uint32_t flags)
{
    if (flags)
        fOrigFlags=flags;
    Clear();
    if( fFileHandle != nil )
    {
        fclose( fFileHandle );
        fFileHandle = nil;
    }
    AddLine( "--------- Bounced Log ---------" );
}

//// IPrintLineToFile ////////////////////////////////////////////////////////

bool plStatusLog::IPrintLineToFile( const char *line, uint32_t count )
{
    if( fFlags & kDontWriteFile )
        return true;

#ifdef PLASMA_EXTERNAL_RELEASE
    uint8_t hint = 0;
    if( fFlags & kAppendToLast )
    {
        hint = (uint8_t)fSize;
    }
#endif

    if (!fFileHandle)
        IReOpen();

    bool ret = ( fFileHandle!=nil );

    if( fFileHandle != nil )
    {
        char work[256];
        char buf[2000];
        buf[0] = 0;

        //build line to encrypt

        if( count != 0 )
        {
            if ( fFlags & kTimestamp )
            {
                snprintf(work, arrsize(work), "(%s) ", plUnifiedTime(kNow).Format("%m/%d %H:%M:%S").c_str());
                strncat(buf, work, arrsize(work));
            }
            if ( fFlags & kTimestampGMT )
            {
                snprintf(work, arrsize(work), "(%s) ", plUnifiedTime::GetCurrent().Format("%m/%d %H:%M:%S UTC").c_str());
                strncat(buf, work, arrsize(work));
            }
            if ( fFlags & kTimeInSeconds )
            {
                snprintf(work, arrsize(work), "(%lu) ", (unsigned long)plUnifiedTime(kNow).GetSecs());
                strncat(buf, work, arrsize(work));
            }
            if ( fFlags & kTimeAsDouble )
            {
                snprintf(work, arrsize(work), "(%f) ", plUnifiedTime(kNow).GetSecsDouble());
                strncat(buf, work, arrsize(work));
            }
            if (fFlags & kRawTimeStamp)
            {
                snprintf(work, arrsize(work), "[t=%10f] ", hsTimer::GetSeconds());
                strncat(buf, work, arrsize(work));
            }
            if (fFlags & kThreadID)
            {
                snprintf(work, arrsize(work), "[t=%lu] ", hsThread::ThisThreadHash());
                strncat(buf, work, arrsize(work));
            }

            size_t remaining = arrsize(buf) - strlen(buf) - 1;
            remaining -= 1;
            if (count <= remaining) {
                strncat(buf, line, count);
            } else {
                strncat(buf, line, remaining);
            }

            strncat(buf, "\n", 1);
        }

        unsigned length = strlen(buf);

        {
            int err;
            err = fwrite(buf,1,length,fFileHandle);
            ret = ( ferror( fFileHandle )==0 );

            if ( ret )
            {
                fSize += err;
                if (!(fFlags & kNonFlushedLog))
                    fflush(fFileHandle);
            }
        }

        if ( fSize>=kMaxFileSize )
        {
            plStatusLogMgr::GetInstance().BounceLogs();
        }

    }

    if ( fFlags & kDebugOutput )
    {
#if HS_BUILD_FOR_WIN32
#ifndef PLASMA_EXTERNAL_RELEASE
        plString str = plString::Format( "%.*s\n", count, line );
        OutputDebugString( str.c_str() );
#endif
#else
        fprintf( stderr, "%.*s\n", count, line );
#endif
    }

    if ( fFlags & kStdout )
    {
        fprintf( stdout, "%.*s\n", count, line );
    }

    return ret;
}
