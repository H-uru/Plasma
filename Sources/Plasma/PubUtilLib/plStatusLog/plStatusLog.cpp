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

#include "plStatusLog.h"
#include "plEncryptLogLine.h"

#include "hsFILELock.h"
#include "plProduct.h"
#include "hsThread.h"
#include "hsTimer.h"
#include "hsWindows.h"

#include "plUnifiedTime/plUnifiedTime.h"

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
    : fDisplays(), fCurrDisplay(), fDrawer(), fLastLogChangeTime()
{
}

plStatusLogMgr::~plStatusLogMgr()
{
    // Unlink all the displays, but don't delete them; leave that to whomever owns them
    while (fDisplays != nullptr)
    {
        plStatusLog *log = fDisplays;

        fDisplays->IUnlink();

        if( log->fFlags & plStatusLog::kDeleteForMe )
            delete log;
    }
}

plStatusLogMgr  &plStatusLogMgr::GetInstance()
{
    static plStatusLogMgr   theManager;
    return theManager;
}

//// Draw ////////////////////////////////////////////////////////////////////

void    plStatusLogMgr::Draw()
{
    /// Just draw current plStatusLog
    if (fCurrDisplay != nullptr && fDrawer != nullptr)
    {
        plStatusLog* firstLog = nullptr;
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
        if (filename.AsString().compare_i((*nextLog)->GetFileName().AsString()) <= 0)
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
        fCurrDisplay = nullptr;
    else
        fCurrDisplay = logToDisplay;

    fLastLogChangeTime = hsTimer::GetSysSeconds();
}

//// SetCurrStatusLog ////////////////////////////////////////////////////////

void plStatusLogMgr::SetCurrStatusLog(const plFileName& logName)
{
    plStatusLog* log = FindLog(logName, false);
    if (log != nullptr)
        fCurrDisplay = log;
}

//// NextStatusLog ///////////////////////////////////////////////////////////

void    plStatusLogMgr::NextStatusLog()
{
    if (fCurrDisplay == nullptr)
        fCurrDisplay = fDisplays;
    else
        fCurrDisplay = fCurrDisplay->fNext;

    fLastLogChangeTime = hsTimer::GetSysSeconds();
}

void    plStatusLogMgr::PrevStatusLog()
{
    if (fCurrDisplay == nullptr)
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

    while (log != nullptr)
    {
        if (log->GetFileName().AsString().compare_i(filename.AsString()) == 0)
            return log;

        log = log->fNext;
    }

    if( !createIfNotFound )
        return nullptr;

    // Didn't find one, so create one! (make it a nice default one :)
    log = CreateStatusLog( kDefaultNumLines, filename, plStatusLog::kFilledBackground |
                                                       plStatusLog::kDeleteForMe );

    return log;
}

//// BounceLogs ///////////////////////////////////////////////////////////////

void plStatusLogMgr::BounceLogs()
{
    plStatusLog *log = fDisplays;

    while (log != nullptr)
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
    : fFileHandle(), fSize(), fForceLog(), fMaxNumLines(numDisplayLines),
      fDisplayPointer()
{
    if (filename.IsValid())
    {
        fFilename = filename;
    }
    else
    {
        fFilename = "";
        flags |= kDontWriteFile;
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

    fLines = new ST::string[fMaxNumLines];
    fColors = new uint32_t[ fMaxNumLines ];
    for( i = 0; i < fMaxNumLines; i++ )
    {
        fColors[ i ] = kWhite;
    }

    fNext = nullptr;
    fBack = nullptr;

}

bool plStatusLog::IReOpen()
{
    if (fFileHandle != nullptr)
    {
        fclose( fFileHandle );
        fFileHandle = nullptr;
    }

    // Open the file, clearing it, if necessary
    if(!(fFlags & kDontWriteFile))
    {
        plFileName fileNoExt;
        ST::string ext;
        IParseFileName(fileNoExt, ext);
        plFileName fileToOpen = ST::format("{}.0.{}", fileNoExt, ext);
        if (!(fFlags & kDontRotateLogs))
        {
            plFileName work, work2;
            work = ST::format("{}.3.{}", fileNoExt, ext);
            plFileSystem::Unlink(work);
            work2 = ST::format("{}.2.{}", fileNoExt, ext);
            plFileSystem::Move(work2, work);
            work = ST::format("{}.1.{}", fileNoExt, ext);
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

    return fFileHandle != nullptr;
}

void    plStatusLog::IFini()
{
    int     i;

    if (fFileHandle != nullptr)
    {
        fclose( fFileHandle );
        fFileHandle = nullptr;
    }

    if( *fDisplayPointer == this )
        *fDisplayPointer = nullptr;

    if (fBack != nullptr || fNext != nullptr)
        IUnlink();

    delete [] fLines;
    delete [] fColors;
}

void plStatusLog::IParseFileName(plFileName& fileNoExt, ST::string& ext) const
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

plStatusLog* plStatusLog::IFindLog(const plFileName& filename)
{
    return plStatusLogMgr::GetInstance().FindLog(filename);
}

//// IUnlink /////////////////////////////////////////////////////////////////

void    plStatusLog::IUnlink()
{
    hsAssert( fBack, "plStatusLog not in list" );
    if( fNext )
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nullptr;
    fNext = nullptr;
}

//// ILink ///////////////////////////////////////////////////////////////////

void    plStatusLog::ILink( plStatusLog **back )
{
    hsAssert(fNext == nullptr && fBack == nullptr, "Trying to link a plStatusLog that's already linked");

    fNext = *back;
    if( *back )
        (*back)->fBack = &fNext;
    fBack = back;
    *back = this;
}

//// IAddLine ////////////////////////////////////////////////////////////////
//  Actually add a stinking line.

bool plStatusLog::IAddLine(const ST::string& line, uint32_t color)
{
    int     i;

    if(fLoggingOff && !fForceLog)
        return true;

    /// Scroll pointers up
    hsFILELock fileLock(fFileHandle);
    hsLockGuard(fileLock);

    if (fMaxNumLines > 0)
    {
        for( i = 0; i < fMaxNumLines - 1; i++ )
        {
            fLines[ i ] = std::move(fLines[ i + 1 ]);
            fColors[ i ] = fColors[ i + 1 ];
        }

        /// Add new
        fLines[i] = line;
        fColors[i] = color;
    }

    bool ret = IPrintLineToFile(line);

    return ret;
}

//// AddLine /////////////////////////////////////////////////////////////////

bool plStatusLog::AddLine(uint32_t color, const ST::string& line)
{
    if(fLoggingOff && !fForceLog)
        return true;

    bool ret = true;

    /// Scan for carriage returns and feed each section into IAddLine()
    size_t startPos = 0;
    ST_ssize_t pos = line.find('\n');
    while (pos != -1)
    {
        // So if we got here, pos points to a carriage return...
        ret &= IAddLine(line.substr(startPos, pos - startPos), color);
        startPos = pos + 1;
        pos = line.find(startPos, '\n');
    }

    /// We might have some left over
    if (startPos < line.size())
    {
        ret &= IAddLine(line.substr(startPos), color);
    }

    return ret;
}

//// Clear ///////////////////////////////////////////////////////////////////

void    plStatusLog::Clear()
{
    int     i;


    for( i = 0; i < fMaxNumLines; i++ )
    {
        fLines[i] = ST::string();
    }
}


//// Bounce //////////////////////////////////////////////////////////////////

void    plStatusLog::Bounce( uint32_t flags)
{
    if (flags)
        fOrigFlags=flags;
    Clear();
    if (fFileHandle != nullptr)
    {
        fclose( fFileHandle );
        fFileHandle = nullptr;
    }
    AddLine( "--------- Bounced Log ---------" );
}

//// IPrintLineToFile ////////////////////////////////////////////////////////

bool plStatusLog::IPrintLineToFile(const ST::string& line)
{
    if( fFlags & kDontWriteFile )
        return true;

    if (!fFileHandle)
        IReOpen();

    bool ret = (fFileHandle != nullptr);

    if (fFileHandle != nullptr)
    {
        ST::string_stream buf;

        //build line to write to log file

        if (!line.empty())
        {
            if ( fFlags & kTimestamp )
            {
                buf << '(' << plUnifiedTime::GetCurrent(plUnifiedTime::kLocal).Format("%m/%d %H:%M:%S") << ") ";
            }
            if ( fFlags & kTimestampGMT )
            {
                buf << '(' << plUnifiedTime::GetCurrent().Format("%m/%d %H:%M:%S UTC") << ") ";
            }
            if ( fFlags & kTimeInSeconds )
            {
                buf << '(' << plUnifiedTime::GetCurrent(plUnifiedTime::kLocal).GetSecs() << ") ";
            }
            if ( fFlags & kTimeAsDouble )
            {
                buf << '(' << plUnifiedTime::GetCurrent(plUnifiedTime::kLocal).GetSecsDouble() << ") ";
            }
            if (fFlags & kRawTimeStamp)
            {
                buf << ST::format("[t={10f}] ", hsTimer::GetSeconds());
            }
            if (fFlags & kThreadID)
            {
                buf << "[t=" << hsThread::ThisThreadHash() << "] ";
            }

            buf << line << '\n';
        }

        {
            int err;
            err = fwrite(buf.raw_buffer(), 1, buf.size(), fFileHandle);
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

    if (fFlags & kDebugOutput)
    {

#if HS_BUILD_FOR_WIN32
#ifndef PLASMA_EXTERNAL_RELEASE
        ST::wchar_buffer buf = line.to_wchar();
        OutputDebugStringW(buf.c_str());
        OutputDebugStringW(L"\n");
#endif
#else
        fwrite(line.c_str(), 1, line.size(), stderr);
        fputc('\n', stderr);
#endif
    }

    if (fFlags & kStdout)
    {
        fwrite(line.c_str(), 1, line.size(), stdout);
        fputc('\n', stdout);
    }

    return ret;
}
