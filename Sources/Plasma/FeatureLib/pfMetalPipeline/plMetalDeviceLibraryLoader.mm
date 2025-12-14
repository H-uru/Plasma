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

#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include "plMetalDevice.h"

void plMetalDevice::LoadLibrary()
{
    /*
     On iOS we're fine loading the Metal 2.1 library. Metal 2.1 includes
     all the Apple Silicon features on iOS. We need Metal 2.3 to get those
     features on macOS.
     */

    NS::Error* error;
#ifdef METAL_3_SDK
    if (@available(macOS 12, iOS 15, *)) {
        if (fMetalDevice->supportsFamily(MTL::GPUFamilyMetal3)) {
            NSURL* shaderURL = [NSBundle.mainBundle URLForResource:@"pfMetalPipelineShadersMSL30" withExtension:@"metallib"];
            fShaderLibrary = fMetalDevice->newLibrary(static_cast<NS::URL*>(shaderURL), &error);
            return;
        }
    }
#endif
#ifdef TARGET_OS_OSX
    if (@available(macOS 11, iOS 14, *)) {
        NSURL* shaderURL = [NSBundle.mainBundle URLForResource:@"pfMetalPipelineShadersMSL23" withExtension:@"metallib"];
        fShaderLibrary = fMetalDevice->newLibrary(static_cast<NS::URL*>(shaderURL), &error);
    } else
#endif
    {
        NSURL* shaderURL = [NSBundle.mainBundle URLForResource:@"pfMetalPipelineShadersMSL21" withExtension:@"metallib"];
        fShaderLibrary = fMetalDevice->newLibrary(static_cast<NS::URL*>(shaderURL), &error);
    }
    hsAssert(error == nil,  "Unexpected error loading Metal shader library");
}
