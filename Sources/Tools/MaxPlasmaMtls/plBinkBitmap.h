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
#ifndef plBinkBitmap_h_inc
#define plBinkBitmap_h_inc

#include <Max.h>
#include <commdlg.h>
#include <bmmlib.h>
#include "Bink.h"

class plBinkBitmapIO : public BitmapIO
{
private:
    HBINK fHBink;           // Handle to currently opened Bink
    TCHAR fName[MAX_PATH];  // Name of currently opened Bink

    BOOL OpenBink(BitmapInfo* fbi);
    void CloseBink();

public:
    plBinkBitmapIO();
    ~plBinkBitmapIO();

    //-- Number of extensions supported, name
    int ExtCount() { return 1; }
    const TCHAR* Ext(int n) { return _T("bik"); }
        
    //-- Descriptions
    const TCHAR* LongDesc() { return "Bink File"; }
    const TCHAR* ShortDesc(){ return "Bink"; }

    //-- Miscellaneous Messages
    const TCHAR* AuthorName()           { return _T("Colin Bonstead"); }
    const TCHAR* CopyrightMessage()     { return _T("Copyright 2004, Cyan Inc."); }
    unsigned int Version()              { return 100; }

    //-- Driver capabilities
    int Capability() { return   BMMIO_READER        | // Reads files
                                BMMIO_MULTIFRAME    | // File contains multiple frames
                                BMMIO_EXTENSION     | // Uses file extension
                                BMMIO_NON_CONCURRENT_ACCESS | // Cannot handle multiple, concurrent requests (necessary?)
                                BMMIO_UNINTERRUPTIBLE // Cannot be started and stopped (necessary?)
                                ;}

    DWORD EvaluateConfigure() { return 0; }
    BOOL LoadConfigure(void* ptr) { return FALSE; }
    BOOL SaveConfigure(void* ptr) { return FALSE; }

    //-- Return info about image
    BMMRES GetImageInfo(BitmapInfo* fbi);

    //-- Image Input
    BitmapStorage* Load(BitmapInfo* fbi, Bitmap* map, BMMRES* status);

    //-- Show DLL's "About..." box
    void ShowAbout(HWND hWnd);
};

#endif // plBinkBitmap_h_inc