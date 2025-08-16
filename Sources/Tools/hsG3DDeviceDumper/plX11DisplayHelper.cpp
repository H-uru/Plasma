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

#include "plX11DisplayHelper.h"

// X11 includes MUST be after everything else to avoid contamination!
#include "plX11Functions.h"

plX11DisplayHelper::plX11DisplayHelper() : fCurrentDisplay()
{
}

plX11DisplayHelper::~plX11DisplayHelper()
{
    if (fCurrentDisplay)
        __XCloseDisplay(fCurrentDisplay);
}

void plX11DisplayHelper::SetCurrentScreen(Display* display) const
{
    if (fCurrentDisplay == display)
        return;
    else if (fCurrentDisplay)
        __XCloseDisplay(fCurrentDisplay);

    fCurrentDisplay = display;
    if (!fCurrentDisplay)
        return;

    int screen = DefaultScreen(fCurrentDisplay);

    fDisplayModes.clear();

#ifdef HAS_XRANDR
    if ((bool)__XRRSizes) {
        // We can use the X RandR extension to get resolutions
        int num_sizes;
        XRRScreenSize* xrrs;

        xrrs = *__XRRSizes(fCurrentDisplay, 0, &num_sizes);
        for (int i = 0; i < num_sizes; i++) {
            // Don't include unreasonably small sizes
            if (xrrs[i].width < 800 || xrrs[i].height < 600)
                continue;

            fDisplayModes.emplace_back(plDisplayMode {
                xrrs[i].width,
                xrrs[i].height,
                DefaultDepth(fCurrentDisplay, screen)
            });
        }
    }
    else
#endif
    {
        fDisplayModes.emplace_back(plDisplayMode {
            DisplayWidth(fCurrentDisplay, screen),
            DisplayHeight(fCurrentDisplay, screen),
            DefaultDepth(fCurrentDisplay, screen)
        });
    }

    std::sort(fDisplayModes.begin(), fDisplayModes.end(), std::greater());
    auto last = std::unique(fDisplayModes.begin(), fDisplayModes.end());
    fDisplayModes.erase(last, fDisplayModes.end());
}

std::vector<plDisplayMode> plX11DisplayHelper::GetSupportedDisplayModes(hsDisplayHndl display, int ColorDepth) const
{
    // Cache the current display so we can answer repeat requests quickly.
    // SetCurrentScreen will catch redundant sets.
    SetCurrentScreen(reinterpret_cast<Display*>(display));
    return fDisplayModes;
}

hsDisplayHndl plX11DisplayHelper::DefaultDisplay() const
{
    if (!fCurrentDisplay) {
        Display* display = *__XOpenDisplay(nullptr);
        SetCurrentScreen(display);
    }

    return reinterpret_cast<hsDisplayHndl>(fCurrentDisplay);
}
