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
#include "plBrowseFolder.h"

#ifdef HS_BUILD_FOR_WIN32

#include <shlobj.h>

bool plBrowseFolder::GetFolder(char *path, const char *startPath, const char *title, HWND hwndOwner)
{
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner	= hwndOwner;
	bi.lpszTitle	= title;
	bi.lpfn			= BrowseCallbackProc;
	bi.lParam		= (LPARAM) startPath;

	ITEMIDLIST *iil = SHBrowseForFolder(&bi);
	// Browse failed, or cancel was selected
	if (!iil)
		return false;
	// Browse succeded.  Get the path.
	else
		SHGetPathFromIDList(iil, path);

	// Free the memory allocated by SHBrowseForFolder
	LPMALLOC pMalloc;
	SHGetMalloc(&pMalloc);
	pMalloc->Free(iil);
	pMalloc->Release();

	return true;
}

int CALLBACK plBrowseFolder::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		// lpData should be the lParam passed to SHBrowseForFolder, which is the start path.
		if (lpData)
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
	}

	return 0;
}

#endif // HS_BUILD_FOR_WIN32
