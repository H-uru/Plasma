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
#include "OptionalDialog.h"
#include "resource.h"
#include <windowsx.h>

LRESULT CALLBACK OptionalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


bool OptionalDialog(HINSTANCE hInstance, HWND hParent, char* desc, unsigned int timeoutsecs, bool defaultyes)
{
    HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_OPTIONALDIALOG), hParent, (DLGPROC) OptionalDialogProc);

    ShowWindow(hWnd, SW_SHOW);

    if (timeoutsecs > 0)
    {
        // Set up timer
        SetTimer(hWnd,WM_USER+1,timeoutsecs*1000,NULL);
        char timeStr[20];
        Static_SetText(GetDlgItem(hWnd,IDC_TIMELEFT),itoa(timeoutsecs,timeStr,10));
    }

    Static_SetText(GetDlgItem(hWnd,IDC_DESCRIPTION),desc);
    if (defaultyes)
    {
        SetWindowLong(GetDlgItem(hWnd,IDC_BUTTON_YES),GWL_STYLE,
            GetWindowLong(GetDlgItem(hWnd,IDC_BUTTON_YES),GWL_STYLE) | BS_DEFPUSHBUTTON);
        Static_SetText(GetDlgItem(hWnd,IDC_DEFAULT),"Default: Yes");
    }
    else
    {
        SetWindowLong(GetDlgItem(hWnd,IDC_BUTTON_NO),GWL_STYLE,
            GetWindowLong(GetDlgItem(hWnd,IDC_BUTTON_NO),GWL_STYLE) | BS_DEFPUSHBUTTON);
        Static_SetText(GetDlgItem(hWnd,IDC_DEFAULT),"Default: No");
    }

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (!IsWindow(hWnd) || !IsDialogMessage(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyWindow(hWnd);

    return msg.wParam == -1 ? defaultyes : (bool)(msg.wParam);
}

// Mesage handler for about box.
LRESULT CALLBACK OptionalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_TIMER:
            PostQuitMessage(-1);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON_YES:
                PostQuitMessage(1);
                return TRUE;

            case IDC_BUTTON_NO: 
                PostQuitMessage(0);
                return TRUE;

            default:
                return FALSE;
            }
            break;
    }
    return FALSE;
}
