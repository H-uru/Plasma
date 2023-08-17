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

#include "hsMessageBox.h"
#include "hsWindows.h"
#include <string_theory/string>

class hsMinimizeClientGuard
{
    hsWindowHndl fWnd;

public:
    hsMinimizeClientGuard()
    {
        fWnd = GetActiveWindow();
        // If the application's topmost window is fullscreen, minimize it before displaying an error
        if ((GetWindowLong(fWnd, GWL_STYLE) & WS_POPUP) != 0)
            ShowWindow(fWnd, SW_MINIMIZE);
    }

    ~hsMinimizeClientGuard()
    {
        ShowWindow(fWnd, SW_RESTORE);
    }
};


hsMessageBoxResult hsMessageBox(const ST::string& message, const ST::string& caption, hsMessageBoxKind kind, hsMessageBoxIcon icon)
{
    if (hsMessageBox_SuppressPrompts)
        return hsMBoxOk;

    uint32_t flags = 0;

    if (kind == hsMessageBoxNormal)
        flags |= MB_OK;
    else if (kind == hsMessageBoxAbortRetyIgnore)
        flags |= MB_ABORTRETRYIGNORE;
    else if (kind == hsMessageBoxOkCancel)
        flags |= MB_OKCANCEL;
    else if (kind == hsMessageBoxRetryCancel)
        flags |= MB_RETRYCANCEL;
    else if (kind == hsMessageBoxYesNo)
        flags |= MB_YESNO;
    else if (kind == hsMessageBoxYesNoCancel)
        flags |= MB_YESNOCANCEL;
    else
        flags |= MB_OK;

    if (icon == hsMessageBoxIconError)
        flags |= MB_ICONERROR;
    else if (icon == hsMessageBoxIconQuestion)
        flags |= MB_ICONQUESTION;
    else if (icon == hsMessageBoxIconExclamation)
        flags |= MB_ICONEXCLAMATION;
    else if (icon == hsMessageBoxIconAsterisk)
        flags |= MB_ICONASTERISK;
    else
        flags |= MB_ICONERROR;

    hsMinimizeClientGuard guard;
    int ans = MessageBoxW(nullptr, message.to_wchar().data(), caption.to_wchar().data(), flags);

    switch (ans)
    {
    case IDOK:          return hsMBoxOk;
    case IDCANCEL:      return hsMBoxCancel;
    case IDABORT:       return hsMBoxAbort;
    case IDRETRY:       return hsMBoxRetry;
    case IDIGNORE:      return hsMBoxIgnore;
    case IDYES:         return hsMBoxYes;
    case IDNO:          return hsMBoxNo;
    default:            return hsMBoxCancel;
    }
}
