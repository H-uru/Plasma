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
//	pfConsoleDirSrc Header													//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	Simple wrapper for parsing an entire directory of files and executing	//
//	each one through the pfConsoleEngine object given.						//
//	I.E. the source for the console commmands is a directory of files,		//
//	hence it's a Console Directory Source, or ConsoleDirSrc. :)				//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleDirSrc_h
#define _pfConsoleDirSrc_h

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "pfConsoleEngine.h"

//// pfConsoleDirSrc Class Definition ////////////////////////////////////////

class pfConsoleDirSrc
{
	protected:
		pfConsoleEngine		*fEngine;
		struct FileName
		{
			std::wstring fPath;
			std::wstring fFile;
			FileName() : fPath(L""), fFile(L"") {}
			FileName(const std::wstring& p, const std::wstring& f) : fPath(p), fFile(f) {}
		};
		std::vector<FileName*> fProcessedFiles;		// list of init files we've already executed
		hsBool fCheckProcessedFiles;		// set to check and skip files init files we've already executed
	public:
		pfConsoleDirSrc(pfConsoleEngine *engine) : fCheckProcessedFiles(false) { fEngine = engine; }
		pfConsoleDirSrc(pfConsoleEngine *engine, const std::string& path, const std::string& mask = "*.ini") :
			fCheckProcessedFiles(false)
		{
			fEngine = engine;
			ParseDirectory(path, mask);
		}
		pfConsoleDirSrc(pfConsoleEngine *engine, const std::wstring& path, const std::wstring& mask = L"*.ini") :
			fCheckProcessedFiles(false)
		{
			fEngine = engine;
			ParseDirectory(path, mask);
		}

		~pfConsoleDirSrc() { ResetProcessedFiles(); }

		// Steps through the given directory and executes all files with the console engine
		hsBool	ParseDirectory(const std::string& path, const std::string& mask = "*.*");
		hsBool	ParseDirectory(const std::wstring& path, const std::wstring& mask = L"*.*");

		void ResetProcessedFiles();
		hsBool AlreadyProcessedFile(const std::wstring& path, const std::wstring& file);
		void AddProcessedFile(const std::wstring& path, const std::wstring& file);
		void SetCheckProcessedFiles(hsBool c) { fCheckProcessedFiles=c; }		
};


#endif //_pfConsoleDirSrc_h
