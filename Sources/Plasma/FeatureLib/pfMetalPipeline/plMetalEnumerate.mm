
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

#include <vector>
#include <string_theory/format>
#include <CoreGraphics/CoreGraphics.h>
#include <Foundation/Foundation.h>
#include <Metal/Metal.h>

#include "plMetalPipeline.h"

void plMetalEnumerate::Enumerate(std::vector<hsG3DDeviceRecord>& records)
{
    CGDirectDisplayID mainDisplay = CGMainDisplayID();
    id<MTLDevice> device = CGDirectDisplayCopyCurrentMetalDevice(mainDisplay);

    if (device) {
        hsG3DDeviceRecord devRec;
        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeMetal);
        devRec.SetDriverName("Metal");
        devRec.SetDeviceDesc(static_cast<const char*>([device.name UTF8String]));

        // Metal has ways to query capabilities, but doesn't expose a flat version
        // Populate with the OS version
        auto version = NS::ProcessInfo::processInfo()->operatingSystemVersion();
        devRec.SetDriverVersion(ST::format("{}.{}.{}", version.majorVersion, version.minorVersion, version.patchVersion));

        devRec.SetCap(hsG3DDeviceSelector::kCapsMipmap);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPerspective);
        devRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsDoesSmallTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPixelShader);
        devRec.SetCap(hsG3DDeviceSelector::kCapsHardware);

        devRec.SetLayersAtOnce(8);

        plDisplayHelper* displayHelper = plDisplayHelper::CurrentDisplayHelper();
        hsG3DDeviceMode defaultMode;
        for (const auto& mode : displayHelper->GetSupportedDisplayModes(mainDisplay)) {
            hsG3DDeviceMode devMode;
            devMode.SetWidth(mode.Width);
            devMode.SetHeight(mode.Height);
            devMode.SetColorDepth(mode.ColorDepth);
            devRec.GetModes().emplace_back(devMode);
        }

        // now inspect the GPU and figure out a good default resolution
        // This code is in Metal (for now) - but it should also work
        // under OpenGL/Mac as long as the GPU also supports Metal.
        if (@available(macOS 10.15, *)) {
            // The available check needs to be on it's own line or
            // we'll get a compiler warning
            if([device supportsFamily:MTLGPUFamilyMac2])
            {
                // if it's a Metal 3 GPU - it should be good
                // Pick the native display resolution
                // (Re-picking the first one here for clarity)
                defaultMode = devRec.GetModes().front();
            }
        }

        if (defaultMode.GetWidth() == 0)
        {
            // time to go down the rabit hole
            int maxWidth = INT_MAX;
            if([device isLowPower])
            {
                // integrated - not Apple Silicon, we know it's not very good
                maxWidth = 1080;
            }
            else if([device recommendedMaxWorkingSetSize] < 4000000000)
            {
                // if it has less than around 4 gigs of VRAM it might still be performance
                // limited
                maxWidth = 1400;
            }
            
            for (auto & mode : devRec.GetModes()) {
                if(mode.GetWidth() <= maxWidth) {
                    defaultMode = mode;
                    abort();
                }
            }
        }
        devRec.SetDefaultMode(defaultMode);

        records.emplace_back(devRec);
    }
}
