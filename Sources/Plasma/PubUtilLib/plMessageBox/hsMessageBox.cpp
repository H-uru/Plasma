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

bool hsMessageBox_SuppressPrompts = false;

#if !defined(HS_BUILD_FOR_APPLE) && !defined(HS_BUILD_FOR_WIN32)
#include <string_theory/string>
#include <string_theory/format>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_messagebox.h>

#include <iterator>

/**
 * The buttons for one message box kind, in the order they are handed to SDL.
 *
 * The first entry is the default (activated by Return) and the last is the
 * dismissal (activated by Escape), which for every multi-button kind here is
 * also the answer to fall back on when no dialog can be shown.
 */
static const SDL_MessageBoxButtonData* IButtonsForKind(hsMessageBoxKind kind, int* count)
{
    static const SDL_MessageBoxButtonData kOk[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxOk, "OK" },
    };
    static const SDL_MessageBoxButtonData kOkCancel[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, hsMBoxOk,     "OK" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxCancel, "Cancel" },
    };
    static const SDL_MessageBoxButtonData kRetryCancel[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, hsMBoxRetry,  "Retry" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxCancel, "Cancel" },
    };
    static const SDL_MessageBoxButtonData kYesNo[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, hsMBoxYes, "Yes" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxNo,  "No" },
    };
    static const SDL_MessageBoxButtonData kYesNoCancel[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, hsMBoxYes,    "Yes" },
        { 0,                                       hsMBoxNo,     "No" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxCancel, "Cancel" },
    };
    static const SDL_MessageBoxButtonData kAbortRetryIgnore[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, hsMBoxRetry,  "Retry" },
        { 0,                                       hsMBoxIgnore, "Ignore" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, hsMBoxAbort,  "Abort" },
    };

    switch (kind) {
    case hsMessageBoxOkCancel:
        *count = std::size(kOkCancel);
        return kOkCancel;
    case hsMessageBoxRetryCancel:
        *count = std::size(kRetryCancel);
        return kRetryCancel;
    case hsMessageBoxYesNo:
        *count = std::size(kYesNo);
        return kYesNo;
    case hsMessageBoxYesNoCancel:
        *count = std::size(kYesNoCancel);
        return kYesNoCancel;
    case hsMessageBoxAbortRetyIgnore:
        *count = std::size(kAbortRetryIgnore);
        return kAbortRetryIgnore;
    case hsMessageBoxNormal:
    default:
        *count = std::size(kOk);
        return kOk;
    }
}

hsMessageBoxResult hsMessageBox(const ST::string& message, const ST::string& caption, hsMessageBoxKind kind, hsMessageBoxIcon icon)
{
    if (hsMessageBox_SuppressPrompts)
        return hsMBoxOk;

    // Always log it: this may be called before there is a video subsystem, and
    // a dialog nobody can see is worse than no dialog at all.
    hsStatusMessageF("{}\n{}", message, caption);

    int buttonCount = 0;
    const SDL_MessageBoxButtonData* buttons = IButtonsForKind(kind, &buttonCount);

    // With nothing to show a dialog on, answer as if the user had dismissed it.
    if (!SDL_WasInit(SDL_INIT_VIDEO))
        return static_cast<hsMessageBoxResult>(buttons[buttonCount - 1].buttonID);

    SDL_MessageBoxFlags flags;
    switch (icon) {
    case hsMessageBoxIconError:
        flags = SDL_MESSAGEBOX_ERROR;
        break;
    case hsMessageBoxIconExclamation:
        flags = SDL_MESSAGEBOX_WARNING;
        break;
    default:
        flags = SDL_MESSAGEBOX_INFORMATION;
        break;
    }

    const SDL_MessageBoxData data{
        flags,
        nullptr,
        caption.c_str(),
        message.c_str(),
        buttonCount,
        buttons,
        nullptr
    };

    // A failure here means no dialog was shown, and closing the dialog without
    // picking anything leaves buttonID at -1. Both are a dismissal.
    int buttonID = -1;
    if (!SDL_ShowMessageBox(&data, &buttonID) || buttonID < 0)
        return static_cast<hsMessageBoxResult>(buttons[buttonCount - 1].buttonID);

    return static_cast<hsMessageBoxResult>(buttonID);
}
#endif
