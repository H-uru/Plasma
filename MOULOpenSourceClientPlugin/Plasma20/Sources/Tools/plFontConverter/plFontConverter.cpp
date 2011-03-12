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
#define	CLASSNAME	"plFontConverter"	// Used in WinInit()
#define WINDOWNAME	"plFontConverter"

#include "HeadSpin.h"
#include "hsTypes.h"
#include <windows.h>
#include "res/resource.h"

#include "pnAllCreatables.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plResMgrCreatable.h"
#include "../plResMgr/plResMgrSettings.h"
#include "../plMessage/plResMgrHelperMsg.h"
#include "../plUnifiedTime/plUnifiedTimeCreatable.h"
REGISTER_CREATABLE(plResMgrHelperMsg);

#include "../plGImage/plFont.h"
REGISTER_CREATABLE(plFont);

#include "../plGImage/plBitmap.h"
#include "../plGImage/plMipmap.h"
REGISTER_NONCREATABLE(plBitmap);
REGISTER_CREATABLE(plMipmap);


HINSTANCE	gInstance;
char		*gCommandLine = nil;
HWND		gMainWindow = nil;

BOOL CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	HACCEL	accelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );


	gCommandLine = (char *)lpCmdLine;

	gInstance = hInstance;

	plResManager *rMgr = new plResManager;
	hsgResMgr::Init( rMgr );

	DialogBox( gInstance, MAKEINTRESOURCE( IDD_MAINDIALOG ), nil, WndProc );

	hsgResMgr::Shutdown();

	return 0;
}
