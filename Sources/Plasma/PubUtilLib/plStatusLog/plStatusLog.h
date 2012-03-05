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
//  plStatusLog Header                                                      //
//                                                                          //
//// Description /////////////////////////////////////////////////////////////
//                                                                          //
//  plStatusLogs are our new encapsulated method of outputting text logs    //
//  for status (or other things, like, say, errors). They do more than just //
//  output to the file, though; they maintain a scrolling buffer that       //
//  can be drawn on to the screen so you can actually view what's being     //
//  outputted to the log file (you can disable file writing if you wish).   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plStatusLog_h
#define _plStatusLog_h

#include "HeadSpin.h"
#include "hsThread.h"

#include <string>

class plPipeline;

//// plStatusLog Definition //////////////////////////////////////////////////
//  Currently, they all display in the same location, so only one should 
//  really be visible at any given time.

class plStatusLogMgr;
class hsMutex;
class plStatusLogDrawerStub;
class plStatusLog
{
    friend class plStatusLogMgr;
    friend class plStatusLogDrawerStub;
    friend class plStatusLogDrawer;
    
    protected:


        mutable uint32_t      fFlags;     // Mutable so we can change it in IPrintLineToFile() internally
        uint32_t  fOrigFlags;

        uint32_t     fMaxNumLines;
        std::string  fCFilename; // used ONLY by GetFileName()
        std::wstring fFilename;
        char**       fLines;
        uint32_t*    fColors;
        hsSemaphore* fSema;
        FILE*        fFileHandle;
        uint32_t     fSize;
        bool         fEncryptMe;
        bool         fForceLog;

        plStatusLog *fNext, **fBack;

        plStatusLog **fDisplayPointer;      // Inside pfConsole
        
        void    IUnlink( void );
        void    ILink( plStatusLog **back );

        bool    IAddLine( const char *line, int32_t count, uint32_t color );
        bool    IPrintLineToFile( const char *line, uint32_t count );
        void    IParseFileName(wchar_t* file, size_t fnsize, wchar_t* fileNoExt, wchar_t** ext) const;

        void    IInit( void );
        void    IFini( void );
        bool    IReOpen( void );

        plStatusLog( uint8_t numDisplayLines, const wchar_t *filename, uint32_t flags );

    public:

        static uint32_t fLoggingOff;
        enum StatusFlagType
        {
            kFilledBackground   = 0x00000001,
            kAppendToLast       = 0x00000002,
            kDontWriteFile      = 0x00000004,
            kDeleteForMe        = 0x00000008,   // BE CAREFUL USING THIS!!
                                                // kDeleteForMe instructs the statusLogMgr to delete
                                                // this log itself when it destructs, which is at the
                                                // very end of client shutdown (the manager is static).
                                                // Because of this, it's safe to use this so long as you'll
                                                // never reference it at the very end of shutdown (as an
                                                // object is deleted by the resManager, for example, is
                                                // okay because that's done in plClient::Shutdown(), not 
                                                // at the very end of app destruction). If you use this
                                                // and your log is deleted before you, your pointer will
                                                // NOT reflect this; it's up to you 
            kAlignToTop         = 0x00000010,
            kDebugOutput        = 0x00000020,   // Also write string to debug console
            kTimestamp          = 0x00000040,   // Write a timestamp in Local time with each entry.
            kStdout             = 0x00000080,   // Also write string to stdout
            kTimeInSeconds      = 0x00000100,   // Write timestamp as seconds since epoch
            kTimeAsDouble       = 0x00000200,   // Write timestamp as seconds.millis
            kDontRotateLogs     = 0x00000400,   // Don't rename/renumber log fileName
            kServerTimestamp    = 0x00000800,   // Timestamp each entry with the server's time
            kRawTimeStamp       = 0x00001000,   // hsTimer::GetSeconds()
            kThreadID           = 0x00002000,   // ID of current thread
            kTimestampGMT       = 0x00004000,   // Write a timestamp in GMT with each entry.
            kNonFlushedLog      = 0x00008000,   // Do not flush the log after each write
        };

        enum 
        {
            kRed    = 0xffff0000,
            kGreen  = 0xff00ff00,
            kBlue   = 0xff0000ff,
            kYellow = 0xffffff00,
            kWhite  = 0xffffffff
        };

        enum
        {
            kMaxFileSize    = 300000000,    // 300 megs
        };

        ~plStatusLog();

        bool    AddLine( const char *line, uint32_t color = kWhite );

        /// printf-like functions

        bool    AddLineF( const char *format, ... );
        bool    AddLineF( uint32_t color, const char *format, ... );

        bool    AddLineV( const char *format, va_list arguments );
        bool    AddLineV( uint32_t color, const char *format, va_list arguments );

        /// Static functions that you give a filename to and it searches for a log based on that
        /// (or creates one if it isn't available)
        static bool AddLineS( const char *filename, const char *format, ... );
        static bool AddLineS( const char *filename, uint32_t color, const char *format, ... );

        void    Clear( void );

        // Clear and open a new file.
        void    Bounce( uint32_t flags=0 );

        const char* GetFileName() const {return fCFilename.c_str();}
        const wchar_t* GetFileNameW() const {return fFilename.c_str();}

        void SetForceLog(bool force) { fForceLog = force; }
};


//// Manager Class Definition ////////////////////////////////////////////////

class plStatusLogMgr
{
    friend class plStatusLog;

    private:

        plStatusLogMgr();
        static plStatusLogMgr   fManager;

    protected:

        plStatusLog     *fDisplays;
        plStatusLog     *fCurrDisplay;

        plStatusLogDrawerStub   *fDrawer;

        double fLastLogChangeTime;

        static wchar_t            fBasePath[];

        static const wchar_t  *IGetBasePath( void ) { return fBasePath; }

        void    IEnsurePathExists( const wchar_t *dirName );
        void    IPathAppend( wchar_t *base, const wchar_t *extra, unsigned maxLen );

        hsMutex     fMutex;     // To make multithreaded-safe

    public:

        enum
        {
            kDefaultNumLines    = 40
        };

        ~plStatusLogMgr();

        static plStatusLogMgr   &GetInstance( void );

        void        Draw( void );

        plStatusLog *CreateStatusLog( uint8_t numDisplayLines, const char *filename, uint32_t flags = plStatusLog::kFilledBackground );
        plStatusLog *CreateStatusLog( uint8_t numDisplayLines, const wchar_t *filename, uint32_t flags = plStatusLog::kFilledBackground );
        void        ToggleStatusLog( plStatusLog *logToDisplay );
        void        NextStatusLog( void );
        void        PrevStatusLog( void );
        void        SetCurrStatusLog( const char *logName );
        void        SetCurrStatusLog( const wchar_t *logName );
        plStatusLog *FindLog( const char *filename, hsBool createIfNotFound = true );
        plStatusLog *FindLog( const wchar_t *filename, hsBool createIfNotFound = true );

        void        SetDrawer( plStatusLogDrawerStub *drawer ) { fDrawer = drawer; }
        void        SetBasePath( const char * path );
        void        SetBasePath( const wchar_t * path );

        void        BounceLogs();

        // Create a new folder and copy all log files into it (returns false on failure)
        bool        DumpLogs( const char *newFolderName );
        bool        DumpLogs( const wchar_t *newFolderName );
};

//// plStatusLogDrawerStub Class ////////////////////////////////////////////
//  Sometimes, we want to be able to link to plStatusLog without having the
//  pipeline-drawing functionality. So we do it this way: we define a single
//  class of type plStatusLogDrawerStub. If it's allocated and we're given a
//  pointer to it (by, say, the pipeline), then we use it to draw and all
//  is happy. If not, we don't draw.

class plStatusLogDrawerStub
{
    protected:

        uint32_t      IGetMaxNumLines( plStatusLog *log ) const { return log->fMaxNumLines; }
        char        **IGetLines( plStatusLog *log ) const { return log->fLines; }
        const char  *IGetFilename( plStatusLog *log ) const { return log->GetFileName(); }
        const wchar_t *IGetFilenameW( plStatusLog *log ) const { return log->GetFileNameW(); }
        uint32_t      *IGetColors( plStatusLog *log ) const { return log->fColors; }
        uint32_t      IGetFlags( plStatusLog *log ) const { return log->fFlags; }
        
    public:
        virtual ~plStatusLogDrawerStub() {}

        virtual void    Draw(plStatusLog* curLog, plStatusLog* firstLog) {}
};

#endif //_plStatusLog_h

