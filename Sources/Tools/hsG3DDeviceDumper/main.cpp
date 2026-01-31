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

#include <cstdlib>
#include <string_theory/stdio>

#include "HeadSpin.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plPipeline/pl3DPipeline.h"

#ifdef HS_BUILD_FOR_MACOS
#   include "pfDisplayHelpers/plMacDisplayHelper.h"
#endif

#ifdef HS_BUILD_FOR_WIN32
#   include "pfDisplayHelpers/plWinDisplayHelper.h"
#endif

#ifdef USE_X11
#   include "pfDisplayHelpers/plX11DisplayHelper.h"
#endif

#ifdef USE_WAYLAND
#   include "pfDisplayHelpers/plWaylandDisplayHelper.h"
#endif

#ifdef PLASMA_PIPELINE_GL
#   include "pfGLPipeline/plGLPipeline.h"
static plGLEnumerate glEnum;
#endif

#ifdef PLASMA_PIPELINE_METAL
#   include "pfMetalPipeline/plMetalPipeline.h"
static plMetalEnumerate mtlEnum;
#endif

int main(int argc, const char* argv[]) {
    hsG3DDeviceSelector devSel;
    plDisplayHelper* helper = nullptr;

#ifdef USE_WAYLAND
#   ifdef USE_X11
    // In a perfect world, this would always be set to tell us if we're running
    // X11 or Wayland, but the world is far from perfect so we'll try this and
    // then guess based on other variables.
    // We only need to do the runtime detection if both are available,
    // otherwise you get what you get
    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if (strcmp(sessionType, "wayland") == 0 || !sessionType && getenv("WAYLAND_DISPLAY"))
        // Fallthrough
#   endif
        helper = new plWaylandDisplayHelper();
#endif

#ifdef USE_X11
    if (!helper && getenv("DISPLAY"))
        helper = new plX11DisplayHelper();
#endif

#ifdef HS_BUILD_FOR_MACOS
    if (!helper)
        helper = new plMacDisplayHelper();
#endif

#ifdef HS_BUILD_FOR_WIN32
    if (!helper)
        helper = new plWinDisplayHelper();
#endif

    plDisplayHelper::SetInstance(helper);

    ST::printf("Checking device records... ");

    devSel.Enumerate(helper->DefaultDisplay());
    devSel.RemoveUnusableDevModes(true);

    ST::printf("{} found.\n\n", devSel.GetDeviceRecords().size());

    for (const hsG3DDeviceRecord& rec : devSel.GetDeviceRecords()) {
        ST::printf("{} ({}) - {}\n", rec.GetDriverName(), rec.GetDriverVersion(), rec.GetDeviceDesc());

        for (const hsG3DDeviceMode& mode : rec.GetModes()) {
            ST::printf("    {}x{} ({}-bit color)\n", mode.GetWidth(), mode.GetHeight(), mode.GetColorDepth());
        }

        ST::printf("\n");
    }

    return 0;
}
