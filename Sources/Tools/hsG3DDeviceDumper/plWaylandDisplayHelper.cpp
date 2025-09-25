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

#include "plWaylandDisplayHelper.h"

#include <cstring>
#include "plWaylandFunctions.h"

template <typename... Args> void ignore(Args...) {}

void plWaylandDisplayHelper::output_handle_mode(void* data, wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    plWaylandDisplayHelper* self = reinterpret_cast<plWaylandDisplayHelper*>(data);

    if (flags & WL_OUTPUT_MODE_CURRENT) {
        self->fDisplayModes.emplace_back(plDisplayMode {
            width,
            height,
            32
        });
    }
}

void plWaylandDisplayHelper::global_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    plWaylandDisplayHelper* self = reinterpret_cast<plWaylandDisplayHelper*>(data);

    if (strcmp(interface, (*__wl_output_interface).name) == 0) {
        static const wl_output_listener output_listener = {
            &ignore,
            &plWaylandDisplayHelper::output_handle_mode,
            &ignore,
            &ignore,
            &ignore,
            &ignore
        };

        self->fCurrentOutput = reinterpret_cast<wl_output*>(wl_registry_bind(registry, id, &__wl_output_interface, std::min<uint32_t>((*__wl_output_interface).version, version)));
        wl_output_add_listener(self->fCurrentOutput, &output_listener, data);
    }
}

plWaylandDisplayHelper::plWaylandDisplayHelper() : fCurrentDisplay(), fCurrentRegistry(), fCurrentOutput()
{
}

plWaylandDisplayHelper::~plWaylandDisplayHelper()
{
    if (fCurrentRegistry)
        wl_registry_destroy(fCurrentRegistry);

    if (fCurrentDisplay)
        __wl_display_disconnect(fCurrentDisplay);
}

void plWaylandDisplayHelper::SetCurrentScreen(wl_display* display) const
{
    if (fCurrentDisplay == display)
        return;
    else if (fCurrentDisplay) {
        if (fCurrentRegistry)
            wl_registry_destroy(fCurrentRegistry);

        __wl_display_disconnect(fCurrentDisplay);
    }

    fCurrentDisplay = display;
    if (!fCurrentDisplay)
        return;

    fCurrentRegistry = wl_display_get_registry(fCurrentDisplay);
    if (!fCurrentRegistry)
        return;

    fDisplayModes.clear();

    static const wl_registry_listener registry_listener = {
        &plWaylandDisplayHelper::global_handler,
        &ignore
    };

    wl_registry_add_listener(fCurrentRegistry, &registry_listener, const_cast<plWaylandDisplayHelper*>(this));

    __wl_display_roundtrip(fCurrentDisplay);
    __wl_display_roundtrip(fCurrentDisplay);

    if (fCurrentOutput) {
        wl_output_destroy(fCurrentOutput);
        fCurrentOutput = nullptr;
    }

    std::sort(fDisplayModes.begin(), fDisplayModes.end(), std::greater());
    auto last = std::unique(fDisplayModes.begin(), fDisplayModes.end());
    fDisplayModes.erase(last, fDisplayModes.end());
}

std::vector<plDisplayMode> plWaylandDisplayHelper::GetSupportedDisplayModes(hsDisplayHndl display, int ColorDepth) const
{
    // Cache the current display so we can answer repeat requests quickly.
    // SetCurrentScreen will catch redundant sets.
    SetCurrentScreen(reinterpret_cast<wl_display*>(display));
    return fDisplayModes;
}

hsDisplayHndl plWaylandDisplayHelper::DefaultDisplay() const
{
    if (!fCurrentDisplay) {
        wl_display* display = *__wl_display_connect(nullptr);
        SetCurrentScreen(display);
    }

    return reinterpret_cast<hsDisplayHndl>(fCurrentDisplay);
}
