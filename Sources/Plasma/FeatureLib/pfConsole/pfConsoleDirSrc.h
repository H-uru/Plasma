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
//  pfConsoleDirSrc Header                                                  //
//                                                                          //
//// Description /////////////////////////////////////////////////////////////
//                                                                          //
//  Simple wrapper for parsing an entire directory of files and executing   //
//  each one through the pfConsoleEngine object given.                      //
//  I.E. the source for the console commmands is a directory of files,      //
//  hence it's a Console Directory Source, or ConsoleDirSrc. :)             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleDirSrc_h
#define _pfConsoleDirSrc_h

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <vector>

#include "pfConsoleCore/pfConsoleEngine.h"

//// pfConsoleDirSrc Class Definition ////////////////////////////////////////

class pfConsoleDirSrc
{
    protected:
        pfConsoleEngine     *fEngine;
        struct FileName
        {
            plFileName fPath;
            plFileName fFile;
            FileName() {}
            FileName(const plFileName& p, const plFileName& f) : fPath(p), fFile(f) {}
        };
        std::vector<FileName*> fProcessedFiles;     // list of init files we've already executed
        bool fCheckProcessedFiles;        // set to check and skip files init files we've already executed
    public:
        pfConsoleDirSrc(pfConsoleEngine *engine) : fCheckProcessedFiles(false) { fEngine = engine; }
        pfConsoleDirSrc(pfConsoleEngine *engine, const plFileName& path, const char* mask = "*.ini")
            : fCheckProcessedFiles(false)
        {
            fEngine = engine;
            ParseDirectory(path, mask);
        }

        ~pfConsoleDirSrc() { ResetProcessedFiles(); }

        // Steps through the given directory and executes all files with the console engine
        bool ParseDirectory(const plFileName& path, const char* mask = "*.*");

        void ResetProcessedFiles();
        bool AlreadyProcessedFile(const plFileName& path, const plFileName& file);
        void AddProcessedFile(const plFileName& path, const plFileName& file);
        void SetCheckProcessedFiles(bool c) { fCheckProcessedFiles=c; }       
};


#endif //_pfConsoleDirSrc_h
