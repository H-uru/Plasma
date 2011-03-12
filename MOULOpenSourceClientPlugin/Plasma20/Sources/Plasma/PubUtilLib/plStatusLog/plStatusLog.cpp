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
//	plStatusLog Functions													//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	10.02.2001 mcn	- Created												//
//	10.16.2001 mcn	- Added static versions of AddLine() so you don't need	//
//					  to explicitly create a log							//
//	10.17.2001 mcn	- Added support for carriage returns in printed lines	//
//	10.24.2002 eap	- Added kDebugOutput flag for writing to debug window	//
//	10.25.2002 eap	- Updated to work under unix							//
//	12.13.2002 eap	- Added kStdout flag                                    //
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdlib.h>
#include "hsThread.h"
#include "hsTemplates.h"
#include "hsTimer.h"
#include "hsStlUtils.h"
#include "../plFile/plFileUtils.h"
#include "plStatusLog.h"
#include "plPipeline.h"
#include "../plPipeline/plDebugText.h"
#include "hsStlUtils.h"
#include "../plFile/hsFiles.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnUtils/pnUtils.h"
#include "../pnProduct/pnProduct.h"

#include "plEncryptLogLine.h"

#if HS_BUILD_FOR_UNIX
	#include <limits.h>
	#define MAX_PATH PATH_MAX
#endif

//////////////////////////////////////////////////////////////////////////////
//// plStatusLogMgr Stuff ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

wchar		plStatusLogMgr::fBasePath[ MAX_PATH ] = L"";

//// Constructor & Destructor ////////////////////////////////////////////////

plStatusLogMgr::plStatusLogMgr()
{
	fDisplays = nil;
	fCurrDisplay = nil;
	fDrawer = nil;
	fLastLogChangeTime = 0;

	PathGetLogDirectory(fBasePath, arrsize(fBasePath));
}

plStatusLogMgr::~plStatusLogMgr()
{
	// Unlink all the displays, but don't delete them; leave that to whomever owns them
	while( fDisplays != nil )
	{
		plStatusLog	*log = fDisplays;

		fDisplays->IUnlink();

		if( log->fFlags & plStatusLog::kDeleteForMe )
			DEL(log);
	}
}

plStatusLogMgr	&plStatusLogMgr::GetInstance( void )
{
	static plStatusLogMgr	theManager;
	return theManager;
}

//// IEnsurePathExists ///////////////////////////////////////////////////////

void	plStatusLogMgr::IEnsurePathExists( const wchar *dirName )
{
	// Note: this creates the directory if it doesn't exist, or if it does,
	// returns false
	plFileUtils::CreateDir( dirName );
}

//// IPathAppend /////////////////////////////////////////////////////////////

void	plStatusLogMgr::IPathAppend( wchar *base, const wchar *extra, unsigned maxLen )
{
	if (!base || !extra)
		return;

	unsigned baseLen = wcslen(base);
	unsigned extraLen = wcslen(extra);

	bool needsSeparator = false;
	if (baseLen >= 1)
		needsSeparator = (base[baseLen - 1] != WPATH_SEPARATOR);

	if (needsSeparator)
	{
		if ((baseLen + 1 + 1) >= maxLen)
			return; // abort, buffer isn't big enough
		base[baseLen] = WPATH_SEPARATOR;
		++baseLen;
		base[baseLen] = '\0';
	}
	
	// concat the strings, making sure not to overrun the buffer
	unsigned curExtraPos = 0;
	for (unsigned curBasePos = baseLen; curBasePos < maxLen; ++curBasePos)
	{
		base[curBasePos] = extra[curExtraPos];
		if (extra[curExtraPos] == '\0')
			break; // done
		++curExtraPos;
	}

	// ensure we are null-terminated
	base[maxLen - 1] = '\0';
}

//// Draw ////////////////////////////////////////////////////////////////////

void	plStatusLogMgr::Draw( void )
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

plStatusLog *plStatusLogMgr::CreateStatusLog( UInt8 numDisplayLines, const char *filename, UInt32 flags )
{
	wchar* wFilename = hsStringToWString(filename);
	plStatusLog* ret = CreateStatusLog(numDisplayLines, wFilename, flags);
	delete [] wFilename;
	return ret;
}

plStatusLog *plStatusLogMgr::CreateStatusLog( UInt8 numDisplayLines, const wchar *filename, UInt32 flags )
{
	IEnsurePathExists( fBasePath );
	plStatusLog	*log = NEW(plStatusLog)( numDisplayLines, filename, flags );

	// Put the new log in its alphabetical position
	plStatusLog** nextLog = &fDisplays;
	while (*nextLog)
	{
		if (wcsicmp(filename, (*nextLog)->GetFileNameW()) <= 0)
			break;
		nextLog = &(*nextLog)->fNext;
	}
	log->ILink(nextLog);

	log->fDisplayPointer = &fCurrDisplay;

	return log;
}

//// ToggleStatusLog /////////////////////////////////////////////////////////

void	plStatusLogMgr::ToggleStatusLog( plStatusLog *logToDisplay )
{
	if( fCurrDisplay == logToDisplay )
		fCurrDisplay = nil;
	else
		fCurrDisplay = logToDisplay;

	fLastLogChangeTime = hsTimer::GetSysSeconds();
}

//// SetCurrStatusLog ////////////////////////////////////////////////////////

void plStatusLogMgr::SetCurrStatusLog(const char* logName)
{
	wchar* wLogName = hsStringToWString(logName);
	SetCurrStatusLog(wLogName);
	delete [] wLogName;
}

void plStatusLogMgr::SetCurrStatusLog(const wchar* logName)
{
	plStatusLog* log = FindLog(logName, false);
	if (log != nil)
		fCurrDisplay = log;
}

//// NextStatusLog ///////////////////////////////////////////////////////////

void	plStatusLogMgr::NextStatusLog( void )
{
	if( fCurrDisplay == nil )
		fCurrDisplay = fDisplays;
	else
		fCurrDisplay = fCurrDisplay->fNext;

	fLastLogChangeTime = hsTimer::GetSysSeconds();
}

void	plStatusLogMgr::PrevStatusLog( void )
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

plStatusLog *plStatusLogMgr::FindLog( const char *filename, hsBool createIfNotFound )
{
	wchar* wFilename = hsStringToWString(filename);
	plStatusLog* ret = FindLog(wFilename, createIfNotFound);
	delete [] wFilename;
	return ret;
}

plStatusLog	*plStatusLogMgr::FindLog( const wchar *filename, hsBool createIfNotFound )
{
	plStatusLog	*log = fDisplays;

	while( log != nil )
	{
		if( wcsicmp( log->GetFileNameW(), filename ) == 0 )
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

//// SetBasePath ////////////////////////////////////////////////////////////////

void plStatusLogMgr::SetBasePath( const char * path )
{
	wchar* wPath = hsStringToWString(path);
	SetBasePath(wPath);
	delete [] wPath;
}

void plStatusLogMgr::SetBasePath( const wchar * path )
{
	wcscpy( fBasePath, path );
}


//// BounceLogs ///////////////////////////////////////////////////////////////

void plStatusLogMgr::BounceLogs()
{
	plStatusLog	*log = fDisplays;

	while( log != nil )
	{
		plStatusLog * tmp = log;
		log = log->fNext;
		tmp->Bounce();
	}
}

//// DumpLogs ////////////////////////////////////////////////////////////////

bool plStatusLogMgr::DumpLogs( const char *newFolderName )
{
	wchar* wFolderName = hsStringToWString(newFolderName);
	bool ret = DumpLogs(wFolderName);
	delete [] wFolderName;
	return ret;
}

bool plStatusLogMgr::DumpLogs( const wchar *newFolderName )
{
	bool retVal = true; // assume success
	// create root path and make sure it exists
	wchar temp[MAX_PATH];
	std::wstring newPath = L"";
	if (fBasePath)
	{
		wcsncpy(temp, fBasePath, MAX_PATH);
		IPathAppend(temp, newFolderName, MAX_PATH);
		newPath = temp;
	}
	else
		newPath = newFolderName;
	IEnsurePathExists(newPath.c_str());

	hsWFolderIterator folderIterator;
	if (fBasePath)
		folderIterator.SetPath(fBasePath);
	else
		folderIterator.SetPath(L".");

	while (folderIterator.NextFile())
	{
		if (folderIterator.IsDirectory())
			continue;

		std::wstring baseFilename = folderIterator.GetFileName();
		std::wstring source;
		if (fBasePath)
		{
			wcsncpy(temp, fBasePath, MAX_PATH);
			IPathAppend(temp, baseFilename.c_str(), MAX_PATH);
			source = temp;
		}
		else
			source = baseFilename;
		
		std::wstring destination;
		wcsncpy(temp, newPath.c_str(), MAX_PATH);
		IPathAppend(temp, baseFilename.c_str(), MAX_PATH);
		destination = temp;

		bool succeeded = (CopyFileW(source.c_str(), destination.c_str(), FALSE) != 0);
		retVal = retVal && succeeded;
	}
	return retVal;
}

//////////////////////////////////////////////////////////////////////////////
//// plStatusLog ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#if defined(PLASMA_EXTERNAL_RELEASE) && (BUILD_TYPE == BUILD_TYPE_LIVE)
// If this is an external live build then don't write log files
UInt32 plStatusLog::fLoggingOff = true;
#else
UInt32 plStatusLog::fLoggingOff = false;
#endif


plStatusLog::plStatusLog( UInt8 numDisplayLines, const wchar *filename, UInt32 flags )
{
	fFileHandle = nil;
	fSize = 0;
	fForceLog = false;

	fMaxNumLines = numDisplayLines;
	if( filename != nil )
	{
		fFilename = filename;
		char* temp = hsWStringToString(filename);
		fCFilename = temp;
		delete [] temp;
	}
	else
	{
		fFilename = L"";
		fCFilename = "";
		flags |= kDontWriteFile;
	}

	fOrigFlags = fFlags = flags;

	IInit();
}

plStatusLog::~plStatusLog()
{
	IFini();
}

void	plStatusLog::IInit()
{
	int	i;

	fFlags = fOrigFlags;

	fLines = TRACKED_NEW char *[ fMaxNumLines ];
	fColors = TRACKED_NEW UInt32[ fMaxNumLines ];
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
		wchar file[ MAX_PATH ];
		wchar fileNoExt[MAX_PATH];
		wchar* ext=nil;
		IParseFileName(file, fileNoExt, &ext);
		fEncryptMe = false;
#ifdef PLASMA_EXTERNAL_RELEASE
		fEncryptMe = ( wcsicmp( fFilename.c_str(), L"chat.log" ) != 0 ) ? true : false;
		if( fEncryptMe )
			ext = L".elf";
#endif
		wchar fileToOpen[MAX_PATH];
		swprintf(fileToOpen, L"%s.0%s", fileNoExt, ext);
		if (!(fFlags & kDontRotateLogs))
		{
			wchar work[MAX_PATH], work2[MAX_PATH];
			swprintf(work, L"%s.3%s",fileNoExt,ext);
			_wremove(work);
			swprintf(work2, L"%s.2%s",fileNoExt,ext);
			_wrename(work2, work);
			swprintf(work, L"%s.1%s",fileNoExt,ext);
			_wrename(work, work2);
			_wrename(fileToOpen, work);
		}
		
		if (fFlags & kAppendToLast)
		{
			fFileHandle = _wfopen( fileToOpen, fEncryptMe ? L"ab" : L"at" );
		}
		else
		{
			fFileHandle = _wfopen( fileToOpen, fEncryptMe ? L"wb" : L"wt" );
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

void	plStatusLog::IFini( void )
{
	int		i;

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

	delete [] fLines;
	delete [] fColors;
}


void plStatusLog::IParseFileName(wchar* file, wchar* fileNoExt, wchar** ext) const
{
	const wchar *base = plStatusLogMgr::IGetBasePath();
	if( wcslen( base ) != nil )
		swprintf( file, L"%s%s%s", base, WPATH_SEPARATOR_STR, fFilename.c_str() );
	else
		wcscpy( file, fFilename.c_str() );

	plFileUtils::EnsureFilePathExists( file );
	
	// apache-style file backup
	
	*ext = wcsrchr(file, L'.');
	if (*ext)
	{
		int fileLen = *ext - file;
		wcsncpy(fileNoExt, file, fileLen);
		fileNoExt[fileLen] = L'\0';
	}
	else
	{
		wcscpy(fileNoExt, file);
		*ext = L"";
	}
	
}

//// IUnlink /////////////////////////////////////////////////////////////////

void	plStatusLog::IUnlink( void )
{
	hsAssert( fBack, "plStatusLog not in list" );
	if( fNext )
		fNext->fBack = fBack;
	*fBack = fNext;

	fBack = nil;
	fNext = nil;
}

//// ILink ///////////////////////////////////////////////////////////////////

void	plStatusLog::ILink( plStatusLog **back )
{
	hsAssert( fNext == nil && fBack == nil, "Trying to link a plStatusLog that's already linked" );

	fNext = *back;
	if( *back )
		(*back)->fBack = &fNext;
	fBack = back;
	*back = this;
}

//// IAddLine ////////////////////////////////////////////////////////////////
//	Actually add a stinking line.

bool plStatusLog::IAddLine( const char *line, Int32 count, UInt32 color )
{
	int		i;

	if(fLoggingOff && !fForceLog)
		return true;

	/// Scroll pointers up
	hsTempMutexLock lock( fMutex );

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
			fLines[ i ] = TRACKED_NEW char[ count + 1 ];
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

	return ret;
}

//// AddLine /////////////////////////////////////////////////////////////////

bool plStatusLog::AddLine( const char *line, UInt32 color )
{
	char	*c, *str;
	if(fLoggingOff && !fForceLog)
		return true;

	bool ret = true;

	/// Scan for carriage returns and feed each section into IAddLine()
	for( str = (char *)line; ( c = strchr( str, '\n' ) ) != nil; str = c + 1 )
	{
		// So if we got here, c points to a carriage return...
		ret = IAddLine( str, (UInt32)c - (UInt32)str, color );
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

bool plStatusLog::AddLineV( UInt32 color, const char *format, va_list arguments )
{
	if(fLoggingOff && !fForceLog)
		return true;
	char buffer[2000];
	vsprintf(buffer, format, arguments);
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

bool plStatusLog::AddLineF( UInt32 color, const char *format, ... )
{
	if(fLoggingOff && !fForceLog)
		return true;
	va_list arguments;
	va_start( arguments, format );

	return AddLineV( color, format, arguments );
}

//// AddLine Static Variations ///////////////////////////////////////////////

bool plStatusLog::AddLineS( const char *filename, const char *format, ... )
{
	plStatusLog	*log = plStatusLogMgr::GetInstance().FindLog( filename );

	if(fLoggingOff && !log->fForceLog)
		return true;

	va_list arguments;
	va_start( arguments, format );

	return log->AddLineV( format, arguments );
}

bool plStatusLog::AddLineS( const char *filename, UInt32 color, const char *format, ... )
{
	plStatusLog	*log = plStatusLogMgr::GetInstance().FindLog( filename );

    if(fLoggingOff && !log->fForceLog)
		return true;

	va_list arguments;
	va_start( arguments, format );

	return log->AddLineV( color, format, arguments );
}

//// Clear ///////////////////////////////////////////////////////////////////

void	plStatusLog::Clear( void )
{
	int		i;


	for( i = 0; i < fMaxNumLines; i++ )
	{
		delete [] fLines[ i ];
		fLines[ i ] = nil;
	}
}


//// Bounce //////////////////////////////////////////////////////////////////

void	plStatusLog::Bounce( UInt32 flags)
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

bool plStatusLog::IPrintLineToFile( const char *line, UInt32 count )
{
	if( fFlags & kDontWriteFile )
		return true;

#ifdef PLASMA_EXTERNAL_RELEASE
	UInt8 hint = 0;
	if( fFlags & kAppendToLast )
	{
		hint = (UInt8)fSize;
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
				StrPrintf(work, arrsize(work), "(%s) ", plUnifiedTime(kNow).Format("%m/%d %H:%M:%S").c_str());
				StrPack(buf, work, arrsize(buf));
			}
			if ( fFlags & kTimestampGMT )
			{
				StrPrintf(work, arrsize(work), "(%s) ", plUnifiedTime::GetCurrentTime().Format("%m/%d %H:%M:%S UTC").c_str());
				StrPack(buf, work, arrsize(buf));
			}
			if ( fFlags & kTimeInSeconds )
			{
				StrPrintf(work, arrsize(work), "(%u) ", plUnifiedTime(kNow).GetSecs());
				StrPack(buf, work, arrsize(buf));
			}
			if ( fFlags & kTimeAsDouble )
			{
				StrPrintf(work, arrsize(work), "(%f) ", plUnifiedTime(kNow).GetSecsDouble());
				StrPack(buf, work, arrsize(buf));
			}
			if (fFlags & kRawTimeStamp)
			{
				StrPrintf(work, arrsize(work), "[t=%10u] ", hsTimer::GetSeconds());
				StrPack(buf, work, arrsize(buf));
			}
			if (fFlags & kThreadID)
			{
				StrPrintf(work, arrsize(work), "[t=%u] ", hsThread::GetMyThreadId());
				StrPack(buf, work, arrsize(buf));
			}

			// find the size of the buf plus the size of the line and only pack that much
			unsigned BufAndLine = StrLen(buf)+count+1;
			if ( BufAndLine > arrsize(buf) )
				BufAndLine = arrsize(buf);
			
			StrPack(buf, line, BufAndLine );

			if(!fEncryptMe )
			{
				StrPack(buf, "\n", arrsize(buf));
			}
		}

		unsigned length = StrLen(buf);

#ifdef PLASMA_EXTERNAL_RELEASE
		// Print to a separate line, since we have to encrypt it
		if( fEncryptMe )
		{
			// Encrypt!
			plStatusEncrypt::Encrypt( (UInt8 *)buf, hint );

			// xor the line length, then write it out, then the line, no terminating character
			UInt16 encrySize = length ^ ((UInt16)fSize);

			// try the first write, if it fails reopen and try again
			int err;
			err = fputc( encrySize & 0xff, fFileHandle );
			if (err == EOF && IReOpen())
			{
				err = fputc( encrySize & 0xff, fFileHandle );
			}

			if (err != EOF)
			{
				fSize++; // inc for the last putc
				err = fputc( encrySize >> 8, fFileHandle );
				if (err != EOF)
					fSize++; // inc for the last putc
				err = fwrite( buf, 1, length, fFileHandle );
				fSize += err;	
				
				if (!(fFlags & kNonFlushedLog))
					fflush(fFileHandle);
			}
			else
			{
				ret = false;
			}
		}
		else
#endif
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
		std::string str;
		xtl::format( str, "%.*s\n", count, line );
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

