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

#include "plUIKitDisplayHelper.h"
#include "plPipeline.h"

plUIKitDisplayHelper::plUIKitDisplayHelper() : fCurrentDisplay(nullptr)
{
}

void plUIKitDisplayHelper::SetCurrentScreen(hsDisplayHndl display) const
{
    // For now lock to the main display
    SetCurrentScreen([UIScreen mainScreen]);
}

void plUIKitDisplayHelper::SetCurrentScreen(UIScreen* screen) const
{
    if (fCurrentDisplay == screen)
        return;

    fCurrentDisplay = screen;

    fDesktopDisplayMode.Width = screen.bounds.size.width;
    fDesktopDisplayMode.Height = screen.bounds.size.height;
    fDesktopDisplayMode.ColorDepth = 32;

    // visibleFrame is in points - put into pixels
    fDesktopDisplayMode.Width *= [screen scale];
    fDesktopDisplayMode.Height *= [screen scale];

    fDisplayModes.clear();

    NSArray *displayModes = [screen availableModes];
    for (UIScreenMode *mode in displayModes) {
        // Plasma likes to handle modes from largest to smallest,
        // CG likes to go from smallest to largest. Insert modes
        // at the front.
        fDisplayModes.emplace_back(plDisplayMode{ static_cast<int>(mode.size.width), static_cast<int>(mode.size.height), 32 });
    }

    std::sort(fDisplayModes.begin(), fDisplayModes.end(),
              [](plDisplayMode a, plDisplayMode b) {
        int resolutionA = a.Width * a.Height;
        int resolutionB = b.Width * b.Height;
        return resolutionA > resolutionB;
    });
}

std::vector<plDisplayMode> plUIKitDisplayHelper::GetSupportedDisplayModes(hsDisplayHndl display, int ColorDepth) const
{
    // Cache the current display so we can answer repeat requests quickly.
    // SetCurrentScreen will catch redundant sets.
    SetCurrentScreen(display);
    return fDisplayModes;
}
