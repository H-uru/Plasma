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

#include "plMacDisplayHelper.h"
#include "plPipeline.h"

#include <AppKit/AppKit.h>
#include <QuartzCore/QuartzCore.h>

plMacDisplayHelper::plMacDisplayHelper() : fCurrentDisplay(kCGNullDirectDisplay)
{
}

void plMacDisplayHelper::SetCurrentScreen(hsDisplayHndl display) const
{
    NSScreen* nsScreen;
    NSArray* screens = [NSScreen screens];
    for (NSUInteger i = 0; i < screens.count; i++) {
        NSScreen*     screen = [screens objectAtIndex:i];
        NSDictionary* deviceDescription = [screen deviceDescription];
        NSNumber*     screenID = [deviceDescription objectForKey:@"NSScreenNumber"];
        if ([screenID unsignedIntValue] == display) {
            nsScreen = screen;
        }
    }
    SetCurrentScreen(nsScreen);
}

void plMacDisplayHelper::SetCurrentScreen(NSScreen* screen) const
{
    CGDirectDisplayID displayID = (CGDirectDisplayID)[[screen.deviceDescription objectForKey:@"NSScreenNumber"] unsignedIntValue];

    if (fCurrentDisplay == displayID)
        return;

    fCurrentDisplay = displayID;

    // Save the native resolution of the desktop. This is used to prevent windowed
    // mode from being larger than the desktop.
    // Going to be a little cheeky here - macOS has a best layout area for windows
    // depending on dock and menu size so I'll return that instead.
    NSRect windowArea = [screen visibleFrame];
    fDesktopDisplayMode.Width = windowArea.size.width;
    fDesktopDisplayMode.Height = windowArea.size.height;
    fDesktopDisplayMode.ColorDepth = 32;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if ([screen respondsToSelector:@selector(backingScaleFactor)]) {
        // visibleFrame is in points - put into pixels
        fDesktopDisplayMode.Width *= [screen backingScaleFactor];
        fDesktopDisplayMode.Height *= [screen backingScaleFactor];
    }
#endif

    // Calculate the region actually available for full screen
    NSRect currentResolution = [screen frame];
#if MAC_OS_X_VERSION_MAX_ALLOWED > 101200 && defined(HAVE_BUILTIN_AVAILABLE)
    if (@available(macOS 12.0, *)) {
        NSEdgeInsets currentSafeAreaInsets = [screen safeAreaInsets];
        // Sigh... Origin doesn't matter but lets do it for inspectability
        currentResolution.origin.x += currentSafeAreaInsets.left;
        currentResolution.origin.y += currentSafeAreaInsets.top;
        currentResolution.size.width -= currentSafeAreaInsets.left + currentSafeAreaInsets.right;
        currentResolution.size.height -= currentSafeAreaInsets.top + currentSafeAreaInsets.bottom;
    }
#endif

    CGFloat safeAspectRatio = static_cast<CGFloat>(currentResolution.size.width) / static_cast<CGFloat>(currentResolution.size.height);

    fDisplayModes.clear();

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    if (&CGDisplayCopyAllDisplayModes != nullptr) {
        CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(fCurrentDisplay, nullptr);
        for (CFIndex i = 0; i < CFArrayGetCount(displayModes); i++) {
            // Now filter out the ones that are taller than the safe area aspect ratio
            // This could break in interesting ways if Apple ships displays that have unsafe
            // areas along the side - but will prevent us stripping any aspect ratios that don't
            // match the display entirely.
            //
            // I'd prefer not to filter a bunch of stuff out but don't strictly have a choice
            // here because there are a lot of fake resolutions in the list.
            //
            // Apple also doesn't include some traditional resolutions on some of their hardware
            // like 800x600, so don't expect those here. My Macbook Pro gives me back 1080p as the lowest.
            // We could force that with kCGDisplayShowDuplicateLowResolutionModes but the aspect ratio
            // filter would still drop some or all of those.
            //
            // Asked for better guidance here from Apple - FB13375033

            CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);

            CGFloat modeAspectRatio = static_cast<CGFloat>(CGDisplayModeGetWidth(mode)) / static_cast<CGFloat>(CGDisplayModeGetHeight(mode));
            if (modeAspectRatio < safeAspectRatio) {
                continue;
            }

            // aspect ratio is good - add the mode to the list

            // Plasma likes to handle modes from largest to smallest,
            // CG likes to go from smallest to largest. Insert modes
            // at the front.
            fDisplayModes.emplace_back(plDisplayMode { static_cast<int>(CGDisplayModeGetWidth(mode)), static_cast<int>(CGDisplayModeGetHeight(mode)), 32 });
        }
        CFRelease(displayModes);
    }
#   if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    else
#   endif
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    {
        CFArrayRef displayModes = CGDisplayAvailableModes(fCurrentDisplay);
        for (CFIndex i = 0; i < CFArrayGetCount(displayModes); i++) {
            CFDictionaryRef mode = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(displayModes, i));

            int width, height, depth;

            CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(mode, kCGDisplayWidth)), kCFNumberIntType, &width);
            CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(mode, kCGDisplayHeight)), kCFNumberIntType, &height);
            CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel)), kCFNumberIntType, &depth);

            CGFloat modeAspectRatio = static_cast<CGFloat>(width) / static_cast<CGFloat>(height);
            if (modeAspectRatio < safeAspectRatio) {
                continue;
            }

            fDisplayModes.emplace_back(plDisplayMode { static_cast<int>(width), static_cast<int>(height), static_cast<int>(depth) });
        }
    }
#endif

    std::sort(fDisplayModes.begin(), fDisplayModes.end(), std::greater());
    auto last = std::unique(fDisplayModes.begin(), fDisplayModes.end());
    fDisplayModes.erase(last, fDisplayModes.end());
}

std::vector<plDisplayMode> plMacDisplayHelper::GetSupportedDisplayModes(hsDisplayHndl display, int ColorDepth) const
{
    // Cache the current display so we can answer repeat requests quickly.
    // SetCurrentScreen will catch redundant sets.
    SetCurrentScreen(display);
    return fDisplayModes;
}

hsDisplayHndl plMacDisplayHelper::DefaultDisplay() const
{
    return CGMainDisplayID();
}
