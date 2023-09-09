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

#include <epoxy/gl.h>
#include <vector>

#include "plGLPipeline.h"
#include "plCGLDevice.h"
#include "plEGLDevice.h"
#include "plWGLDevice.h"

static void fillDeviceRecord(hsG3DDeviceRecord& devRec)
{
    devRec.SetCap(hsG3DDeviceSelector::kCapsMipmap);
    devRec.SetCap(hsG3DDeviceSelector::kCapsPerspective);
    devRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures);
    devRec.SetCap(hsG3DDeviceSelector::kCapsDoesSmallTextures);

    devRec.SetLayersAtOnce(8);

    // Just make a fake mode so the device selector will let it through
    hsG3DDeviceMode devMode;
    devMode.SetWidth(hsG3DDeviceSelector::kDefaultWidth);
    devMode.SetHeight(hsG3DDeviceSelector::kDefaultHeight);
    devMode.SetColorDepth(hsG3DDeviceSelector::kDefaultDepth);
    devRec.GetModes().emplace_back(devMode);
}

void plGLEnumerate::Enumerate(std::vector<hsG3DDeviceRecord>& records)
{
#ifdef USE_EGL
    // The USE_EGL define tells us whether the epoxy library includes support
    // for attempting to use EGL on the current platform, but we still need to
    // check if EGL is actually available at runtime.
    //
    // On Windows, this may be true in cases like the PowerVR SDK or when using
    // ANGLE.
    //
    // On Linux, this should be true with mesa or nvidia drivers.
    if (epoxy_has_egl()) {
        hsG3DDeviceRecord rec;
        fillDeviceRecord(rec);
        if (plEGLDevice::Enumerate(rec))
            records.emplace_back(rec);
    }
#endif

#ifdef HS_BUILD_FOR_WIN32
    {
        hsG3DDeviceRecord rec;
        fillDeviceRecord(rec);
        if (plWGLDevice::Enumerate(rec))
            records.emplace_back(rec);
    }
#endif

#ifdef HS_BUILD_FOR_MACOS
    {
        hsG3DDeviceRecord rec;
        fillDeviceRecord(rec);
        if (plCGLDevice::Enumerate(rec))
            records.emplace_back(rec);
    }
#endif
}
