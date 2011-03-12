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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plStatusLog Header														//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	plStatusLogs are our new encapsulated method of outputting text logs	//
//	for status (or other things, like, say, errors). They do more than just	//
//	output to the file, though; they maintain a scrolling buffer that		//
//	can be drawn on to the screen so you can actually view what's being		//
//	outputted to the log file (you can disable file writing if you wish).	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plStatusLog_h
#define _plStatusLog_h

#include "hsTypes.h"
#include "hsThread.h"

#include <string>

class plPipeline;

//// plStatusLog Definition //////////////////////////////////////////////////
//	Currently, they all display in the same location, so only one should 
//	really be visible at any given time.

class plStatusLogMgr;
class hsMutex;
class plStatusLogDrawerStub;
class plStatusLog
{
	friend class plStatusLogMgr;
	friend class plStatusLogDrawerStub;
	friend class plStatusLogDrawer;
	
	protected:


		mutable UInt32		fFlags;		// Mutable so we can change it in IPrintLineToFile() internally
		UInt32	fOrigFlags;

		UInt32		fMaxNumLines;
		std::string fCFilename; // used ONLY by GetFileName()
		std::wstring fFilename;
		char		**fLines;
		UInt32		*fColors;
		hsMutex		fMutex;		// To make multithreaded-safe
		FILE*		fFileHandle;
		UInt32		fSize;
		bool		fEncryptMe;
		bool		fForceLog;

		plStatusLog	*fNext, **fBack;

		plStatusLog	**fDisplayPointer;		// Inside pfConsole
		
		void	IUnlink( void );
		void	ILink( plStatusLog **back );

		bool	IAddLine( const char *line, Int32 count, UInt32 color );
		bool	IPrintLineToFile( const char *line, UInt32 count );
		void	IParseFileName(wchar* file, wchar* fileNoExt, wchar** ext) const;

		void	IInit( void );
		void	IFini( void );
		bool	IReOpen( void );

		plStatusLog( UInt8 numDisplayLines, const wchar *filename, UInt32 flags );

	public:

		static UInt32 fLoggingOff;
		enum StatusFlagType
		{
			kFilledBackground	= 0x00000001,
			kAppendToLast		= 0x00000002,
			kDontWriteFile		= 0x00000004,
			kDeleteForMe		= 0x00000008,	// BE CAREFUL USING THIS!!
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
			kAlignToTop			= 0x00000010,
			kDebugOutput		= 0x00000020,	// Also write string to debug console
			kTimestamp			= 0x00000040,	// Write a timestamp in Local time with each entry.
			kStdout				= 0x00000080,	// Also write string to stdout
			kTimeInSeconds		= 0x00000100,	// Write timestamp as seconds since epoch
			kTimeAsDouble		= 0x00000200,	// Write timestamp as seconds.millis
			kDontRotateLogs		= 0x00000400,	// Don't rename/renumber log fileName
			kServerTimestamp	= 0x00000800,	// Timestamp each entry with the server's time
			kRawTimeStamp		= 0x00001000,	// hsTimer::GetSeconds()
			kThreadID			= 0x00002000,	// ID of current thread
			kTimestampGMT		= 0x00004000,	// Write a timestamp in GMT with each entry.
			kNonFlushedLog		= 0x00008000,	// Do not flush the log after each write
		};

		enum 
		{
			kRed	= 0xffff0000,
			kGreen	= 0xff00ff00,
			kBlue	= 0xff0000ff,
			kYellow	= 0xffffff00,
			kWhite	= 0xffffffff
		};

		enum
		{
			kMaxFileSize	= 300000000,	// 300 megs
		};

		~plStatusLog();

		bool	AddLine( const char *line, UInt32 color = kWhite );

		/// printf-like functions

		bool	AddLineF( const char *format, ... );
		bool	AddLineF( UInt32 color, const char *format, ... );

		bool	AddLineV( const char *format, va_list arguments );
		bool	AddLineV( UInt32 color, const char *format, va_list arguments );

		/// Static functions that you give a filename to and it searches for a log based on that
		/// (or creates one if it isn't available)
		static bool AddLineS( const char *filename, const char *format, ... );
		static bool AddLineS( const char *filename, UInt32 color, const char *format, ... );

		void	Clear( void );

		// Clear and open a new file.
		void	Bounce( UInt32 flags=0 );

		const char* GetFileName() const {return fCFilename.c_str();}
		const wchar* GetFileNameW() const {return fFilename.c_str();}

		void SetForceLog(bool force) { fForceLog = force; }
};


//// Manager Class Definition ////////////////////////////////////////////////

class plStatusLogMgr
{
	friend class plStatusLog;

	private:

		plStatusLogMgr();
		static plStatusLogMgr	fManager;

	protected:

		plStatusLog		*fDisplays;
		plStatusLog		*fCurrDisplay;

		plStatusLogDrawerStub	*fDrawer;

		double fLastLogChangeTime;

		static wchar			fBasePath[];

		static const wchar	*IGetBasePath( void ) { return fBasePath; }

		void	IEnsurePathExists( const wchar *dirName );
		void	IPathAppend( wchar *base, const wchar *extra, unsigned maxLen );

		hsMutex		fMutex;		// To make multithreaded-safe

	public:

		enum
		{
			kDefaultNumLines	= 40
		};

		~plStatusLogMgr();

		static plStatusLogMgr	&GetInstance( void );

		void		Draw( void );

		plStatusLog	*CreateStatusLog( UInt8 numDisplayLines, const char *filename, UInt32 flags = plStatusLog::kFilledBackground );
		plStatusLog	*CreateStatusLog( UInt8 numDisplayLines, const wchar *filename, UInt32 flags = plStatusLog::kFilledBackground );
		void		ToggleStatusLog( plStatusLog *logToDisplay );
		void		NextStatusLog( void );
		void		PrevStatusLog( void );
		void		SetCurrStatusLog( const char *logName );
		void		SetCurrStatusLog( const wchar *logName );
		plStatusLog	*FindLog( const char *filename, hsBool createIfNotFound = true );
		plStatusLog	*FindLog( const wchar *filename, hsBool createIfNotFound = true );

		void		SetDrawer( plStatusLogDrawerStub *drawer ) { fDrawer = drawer; }
		void		SetBasePath( const char * path );
		void		SetBasePath( const wchar * path );

		void		BounceLogs();

		// Create a new folder and copy all log files into it (returns false on failure)
		bool		DumpLogs( const char *newFolderName );
		bool		DumpLogs( const wchar *newFolderName );
};

//// plStatusLogDrawerStub Class ////////////////////////////////////////////
//	Sometimes, we want to be able to link to plStatusLog without having the
//	pipeline-drawing functionality. So we do it this way: we define a single
//	class of type plStatusLogDrawerStub. If it's allocated and we're given a
//	pointer to it (by, say, the pipeline), then we use it to draw and all
//	is happy. If not, we don't draw.

class plStatusLogDrawerStub
{
	protected:

		UInt32		IGetMaxNumLines( plStatusLog *log ) const { return log->fMaxNumLines; }
		char		**IGetLines( plStatusLog *log ) const { return log->fLines; }
		const char	*IGetFilename( plStatusLog *log ) const { return log->GetFileName(); }
		const wchar	*IGetFilenameW( plStatusLog *log ) const { return log->GetFileNameW(); }
		UInt32		*IGetColors( plStatusLog *log ) const { return log->fColors; }
		UInt32		IGetFlags( plStatusLog *log ) const { return log->fFlags; }
		
	public:
		virtual ~plStatusLogDrawerStub() {}

		virtual void	Draw(plStatusLog* curLog, plStatusLog* firstLog) {}
};

#endif //_plStatusLog_h

