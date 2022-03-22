
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

#include "HeadSpin.h"

#include <vector>

#include <Foundation/Foundation.h>

#include "plMetalPipeline.h"
#include <sys/utsname.h>

void plMetalEnumerate::Enumerate(std::vector<hsG3DDeviceRecord>& records)
{
    //For now - just use the default device. If there is a high power discrete device - this will spin it up.
    //This will also automatically pin us to an eGPU if present and the user has configured us to use it.
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    
    if (device) {
        hsG3DDeviceRecord devRec;
        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeMetal);
        devRec.SetDriverName("Metal");
        devRec.SetDeviceDesc(device->name()->utf8String());
        //Metal has ways to query capabilities, but doesn't expose a flat version
        //Populate with the OS version
        @autoreleasepool {
            NSProcessInfo *processInfo = [NSProcessInfo processInfo];
            NSOperatingSystemVersion version  = processInfo.operatingSystemVersion;
            NSString *versionString = [NSString stringWithFormat:@"%li.%li.%li", (long)version.majorVersion, (long)version.minorVersion, version.patchVersion];
            devRec.SetDriverVersion([versionString cStringUsingEncoding:NSUTF8StringEncoding]);
        }
        devRec.SetDriverDesc(device->name()->utf8String());
        
        devRec.SetCap(hsG3DDeviceSelector::kCapsMipmap);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPerspective);
        devRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsDoesSmallTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPixelShader);
        devRec.SetCap(hsG3DDeviceSelector::kCapsHardware);

        devRec.SetLayersAtOnce(8);
        
        // Just make a fake mode so the device selector will let it through
        hsG3DDeviceMode devMode;
        devMode.SetWidth(hsG3DDeviceSelector::kDefaultWidth);
        devMode.SetHeight(hsG3DDeviceSelector::kDefaultHeight);
        devMode.SetColorDepth(hsG3DDeviceSelector::kDefaultDepth);
        devRec.GetModes().emplace_back(devMode);
        
        records.emplace_back(devRec);
    }
}
