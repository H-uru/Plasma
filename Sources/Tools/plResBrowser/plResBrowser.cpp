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
#define CLASSNAME   "plResBrowser"  // Used in WinInit()
#define WINDOWNAME  "plResBrowser"

#include "HeadSpin.h"
#include "res/resource.h"

#include "pnAllCreatables.h"
#include "plResMgr/plResMgrCreatable.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plResMgrSettings.h"
#include "plMessage/plResMgrHelperMsg.h"
REGISTER_CREATABLE(plResMgrHelperMsg);


HINSTANCE   gInstance;
char        *gCommandLine = nil;
HWND        gMainWindow = nil;

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
BOOL WinInit( HINSTANCE hInst, int nCmdShow );


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG     msg;
    HACCEL  accelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );


    plResMgrSettings::Get().SetFilterNewerPageVersions( false );
    plResMgrSettings::Get().SetFilterOlderPageVersions( false );

    gCommandLine = (char *)lpCmdLine;
    plResManager *rMgr = new plResManager;
    hsgResMgr::Init( rMgr );

    if( !WinInit( hInstance, nCmdShow ) )
        return -1;

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        if( !TranslateAccelerator( gMainWindow, accelTable, &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    hsgResMgr::Shutdown();

    return 0;
}

BOOL WinInit(HINSTANCE hInst, int nCmdShow)
{
    gInstance = hInst;

    // Fill out WNDCLASS info
    WNDCLASS wndClass;
    wndClass.style              = 0;    // CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc        = WndProc;
    wndClass.cbClsExtra         = 0;
    wndClass.cbWndExtra         = 0;
    wndClass.hInstance          = hInst;
    wndClass.hIcon              = LoadIcon( hInst, MAKEINTRESOURCE( IDI_APPICON ) );

    wndClass.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground      = (HBRUSH)GetSysColorBrush( COLOR_3DFACE );
    wndClass.lpszMenuName       = MAKEINTRESOURCE( IDR_APPMENU );
    wndClass.lpszClassName      = CLASSNAME;
    
    // can only run one at a time anyway, so just quit if another is running
    if (!RegisterClass(&wndClass)) 
        return FALSE;

    DWORD dwStyle = WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
    DWORD dwExStyle = WS_EX_ACCEPTFILES;

    // Create a window
    gMainWindow = CreateWindowEx(dwExStyle, CLASSNAME, WINDOWNAME, 
                    dwStyle, 10, 10, 
                    800,
                    600,
                     NULL, NULL, hInst, NULL);

    return TRUE;
}
