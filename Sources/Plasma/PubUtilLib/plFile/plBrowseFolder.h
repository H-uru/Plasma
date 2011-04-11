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
#ifndef plBrowseFolder_h_inc
#define plBrowseFolder_h_inc

#include "hsConfig.h"

#ifdef HS_BUILD_FOR_WIN32
#include "hsWindows.h"

//
// Gets a directory using the "Browse for Folder" dialog.
//
// path:		Buffer to recieve the path.  Should be MAX_PATH characters.
// startPath:	Initial path.
// title:		Not really the title of the dialog, but it's displayed above the
//				folder list.  Could be used to give instructions.
// hwndOwner:	Owner window for dialog box.
//
// Returns true if path contains a valid path, false otherwise (error or user
// clicked cancel.
//

class plBrowseFolder
{
public:
	static bool GetFolder(char *path, const char *startPath = NULL, const char *title = NULL, HWND hwndOwner = NULL);

protected:
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};

#endif // HS_BUILD_FOR_WIN32

#endif // plBrowseFolder_h_inc
