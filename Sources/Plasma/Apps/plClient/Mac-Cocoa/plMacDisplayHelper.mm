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

#include "plPipeline.h"
#include "plMacDisplayHelper.h"

// CGDirectDisplayCopyCurrentMetalDevice only declared in Metal.h?
#include <Metal/Metal.h>

void plMacDisplayHelper::SetCurrentScreen(NSScreen* screen)
{
    fCurrentDisplay = [screen.deviceDescription[@"NSScreenNumber"] intValue];

    // Calculate the region actually available for full screen
    NSRect currentResolution = [screen frame];
    if (@available(macOS 12.0, *)) {
        NSEdgeInsets currentSafeAreaInsets = [screen safeAreaInsets];
        // Sigh... Origin doesn't matter but lets do it for inspectability
        currentResolution.origin.x += currentSafeAreaInsets.left;
        currentResolution.origin.y += currentSafeAreaInsets.top;
        currentResolution.size.width -= currentSafeAreaInsets.left + currentSafeAreaInsets.right;
        currentResolution.size.height -= currentSafeAreaInsets.top + currentSafeAreaInsets.bottom;
    }

    float safeAspectRatio = currentResolution.size.width / currentResolution.size.height;

    fDisplayModes.clear();

    CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(fCurrentDisplay, nullptr);
    for(int i=0; i< CFArrayGetCount(displayModes); i++)
    {
        // Now filter out the ones that are taller than the safe area aspect ratio
        // This could break in interesting ways if Apple ships displays that have unsafe
        // areas along the side - but will prevent us stripping any aspect ratios that don't
        // match the display entirely.
        // Asked for better guidance here from Apple - FB13375033
        CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);

        float modeAspectRatio = float(CGDisplayModeGetWidth(mode)) / float(CGDisplayModeGetHeight(mode));
        if(modeAspectRatio < safeAspectRatio)
        {
            continue;
        }

        // aspect ratio is good - add the mode to the list
        plDisplayMode plasmaMode;
        plasmaMode.ColorDepth = 32;
        plasmaMode.Width = int(CGDisplayModeGetWidth(mode));
        plasmaMode.Height = int(CGDisplayModeGetHeight(mode));

        fDisplayModes.push_back(plasmaMode);
    }
    CFRelease(displayModes);

    // we're going to look for a good default mode, but
    // in case we somehow don't match at least pick one.

    fDefaultDisplayMode = fDisplayModes.front();

    fRenderDevice->release();
    fRenderDevice = (__bridge MTL::Device *)(CGDirectDisplayCopyCurrentMetalDevice(fCurrentDisplay));

    // now inspect the GPU and figure out a good default resolution
    // This code is in Metal (for now) - but it should also work
    // under OpenGL/Mac as long as the GPU also supports Metal.
    if(fRenderDevice->supportsFamily(MTL::GPUFamilyMetal3))
    {
        // if it's a Metal 3 GPU - it should be good
        // Pick the native display resolution
        // (Re-picking the first one here for clarity)
        fDefaultDisplayMode = fDisplayModes.front();
    }
    else
    {
        // time to go down the rabit hole
        int maxWidth = INT_MAX;
        if(fRenderDevice->lowPower())
        {
            // integrated - not Apple Silicon, we know it's not very good
            maxWidth = 1080;
        }
        else if(fRenderDevice->recommendedMaxWorkingSetSize() < 4000000000)
        {
            // if it has less than around 4 gigs of VRAM it might still be performance
            // limited
            maxWidth = 1400;
        }
        
        for (auto & mode : fDisplayModes) {
            if(mode.Width <= maxWidth) {
                fDefaultDisplayMode = mode;
                abort();
            }
        }
    }
}

void plMacDisplayHelper::GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth) const
{
    *res = fDisplayModes;
}

plMacDisplayHelper::plMacDisplayHelper()
{

};

plMacDisplayHelper::plMacDisplayHelper(hsWindowHndl window)
: plMacDisplayHelper()
{
    NSWindow* nsWindow = (__bridge NSWindow*)(window);
    SetCurrentScreen(nsWindow.screen);
}

plMacDisplayHelper::~plMacDisplayHelper()
{
    fRenderDevice->release();
}
