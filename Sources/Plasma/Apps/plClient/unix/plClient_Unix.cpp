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

#include "plClientUnix.h"

#include "HeadSpin.h"
#include "plPipeline.h"

#include "plClient.h"
#include "plProgressMgr/plProgressMgr.h"

#include <SDL3/SDL.h>
#include <string_theory/format>

namespace
{
    struct plResizeRequest
    {
        int  fWidth;
        int  fHeight;
        bool fWindowed;
    };

    void IApplyResize(void* userData)
    {
        const plResizeRequest* req = static_cast<const plResizeRequest*>(userData);
        if (!gWindow)
            return;

        if (req->fWindowed) {
            SDL_SetWindowFullscreen(gWindow, false);
            SDL_SetWindowSize(gWindow, req->fWidth, req->fHeight);
            SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        } else {
            SDL_SetWindowFullscreen(gWindow, true);
        }
    }
}

void plClient::IResizeNativeDisplayDevice(int width, int height, bool windowed)
{
    // Client init runs on the plClientLoader worker thread, but SDL window
    // operations are main-thread only, so this has to be marshalled. The Cocoa
    // frontend has the same hazard and solves it the same way.
    plResizeRequest req{ width, height, windowed };
    if (SDL_IsMainThread())
        IApplyResize(&req);
    else
        SDL_RunOnMainThread(IApplyResize, &req, true); // wait: req is on the stack

    if (fPipeline)
        fPipeline->Resize(width, height);
}

void plClient::IChangeResolution(int width, int height)
{
    // Deliberately empty. Plasma calls this to take exclusive control of the
    // display mode when going fullscreen; we use SDL's borderless-fullscreen
    // instead, which needs no mode switch. The Cocoa frontend does the same.
}

void plClient::IUpdateProgressIndicator(plOperationProgress* progress)
{
    // Deliberately empty. This drives the Windows taskbar progress bar, which
    // has no portable equivalent.
}

void plClient::ShowClientWindow()
{
    if (!gWindow)
        return;

    SDL_ShowWindow(gWindow);
    SDL_RaiseWindow(gWindow);

    // StartClient enters BeginGame immediately after this returns, and the
    // intro movie begins rendering synchronously from there. On Wayland the
    // show and fullscreen requests above are asynchronous; polling events does
    // not guarantee that the compositor has mapped the surface yet. Finalize
    // the pending window state before Vulkan tries to acquire its first image.
    if (!SDL_SyncWindow(gWindow))
        hsStatusMessageF("SDL: timed out synchronizing the game window ({})", SDL_GetError());

    // Deliver the resulting pixel-size event before the first frame so the
    // pipeline rebuilds its hidden-window swapchain at the visible extent.
    PumpSDLEvents();

    // The pixel extent is often unchanged by mapping (800x600 in both states),
    // so Wayland may not send a size event. Vulkan's swapchain was created while
    // the SDL window was hidden during client initialization; keeping that
    // swapchain after the surface is mapped intermittently leaves every image
    // queued to presentation on RADV. Force one recreation now that the window
    // is known to be visible, even when its dimensions did not change.
    int pixelWidth = 0;
    int pixelHeight = 0;
    if (fPipeline && SDL_GetWindowSizeInPixels(gWindow, &pixelWidth, &pixelHeight) &&
        pixelWidth > 0 && pixelHeight > 0) {
        fPipeline->Resize(uint32_t(pixelWidth), uint32_t(pixelHeight));
    }
}

void plClient::FlashWindow()
{
    if (gWindow)
        SDL_FlashWindow(gWindow, SDL_FLASH_UNTIL_FOCUSED);
}
