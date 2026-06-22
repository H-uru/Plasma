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

#include "plX11ClientWindow.h"
#include "hsOptionalCall.h"
#include "plProduct.h"
#include "plClient.h"

#include <string_theory/format>
#include <string_theory/stdio>

#include "plPipeline.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "plMessage/plInputEventMsg.h"
#include "pfDisplayHelpers/plX11DisplayHelper.h"

// X11 includes MUST be after everything else to avoid contamination!
#include <X11/Xlib.h>
#include <X11/Xutil.h>

hsOptionalCallDecl("libX11", XCloseDisplay);
hsOptionalCallDecl("libX11", XCreateWindow);
hsOptionalCallDecl("libX11", XDestroyWindow);
hsOptionalCallDecl("libX11", XFilterEvent);
hsOptionalCallDecl("libX11", XFlush);
hsOptionalCallDecl("libX11", XInitThreads);
hsOptionalCallDecl("libX11", XInternAtom);
hsOptionalCallDecl("libX11", XLookupString);
hsOptionalCallDecl("libX11", XMapWindow);
hsOptionalCallDecl("libX11", XNextEvent);
hsOptionalCallDecl("libX11", XOpenDisplay);
hsOptionalCallDecl("libX11", XPending);
hsOptionalCallDecl("libX11", XRaiseWindow);
hsOptionalCallDecl("libX11", XRefreshKeyboardMapping);
hsOptionalCallDecl("libX11", XResizeWindow);
hsOptionalCallDecl("libX11", XSetWMProtocols);
hsOptionalCallDecl("libX11", XStoreName);
hsOptionalCallDecl("libX11", XSync);

#ifdef USE_XFIXES
#   include <X11/extensions/Xfixes.h>
    hsOptionalCallDecl("libXfixes", XFixesHideCursor);
    hsOptionalCallDecl("libXfixes", XFixesShowCursor);
#endif

bool plX11ClientWindow::PreInit()
{
    // First: Check that we were able to dynamically load the X11 functions
    if (!(bool)__XOpenDisplay)
        return false;

    if (!*__XInitThreads())
        return false;

    plDisplayHelper::SetInstance(new plX11DisplayHelper());

    return true;
}

bool plX11ClientWindow::CreateClientWindow()
{
    fXDisplay = *__XOpenDisplay(nullptr);
    if (!fXDisplay) {
        hsStatusMessage("Could not connect to X server");
        return false;
    }

    int screen = DefaultScreen(fXDisplay);
    Window root = DefaultRootWindow(fXDisplay);

    XSetWindowAttributes win_attrs = {};
    win_attrs.background_pixmap = None;
    win_attrs.background_pixel = BlackPixel(fXDisplay, screen);
    win_attrs.event_mask = ExposureMask
                         | EnterWindowMask
                         | LeaveWindowMask
                         | PointerMotionMask
                         | ButtonPressMask
                         | ButtonReleaseMask
                         | KeyPressMask
                         | KeyReleaseMask
                         | StructureNotifyMask;

    fXWindow = *__XCreateWindow(fXDisplay, root, fWindowSize.fX, fWindowSize.fY, fWindowSize.fWidth, fWindowSize.fHeight, 0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask | CWBackPixmap | CWBackPixel, &win_attrs);

    ST::string title = plProduct::LongName();
    __XStoreName(fXDisplay, fXWindow, title.c_str());

    /* Setup window close as an event to be handled by us */
    Atom wm_delete_window = *__XInternAtom(fXDisplay, "WM_DELETE_WINDOW", 0);
    __XSetWMProtocols(fXDisplay, fXWindow, &wm_delete_window, 1);

    return true;
}

void plX11ClientWindow::ShowClientWindow()
{
    __XMapWindow(fXDisplay, fXWindow);
    __XRaiseWindow(fXDisplay, fXWindow);
    __XFlush(fXDisplay);
}

bool plX11ClientWindow::HandleEvent(XEvent* evt)
{
    plClient* client = plClient::GetInstance();

    switch (evt->type) {
    case ClientMessage:
        {
            static Atom atom_wm_protocols = *__XInternAtom(fXDisplay, "WM_PROTOCOLS", 1);
            static Atom atom_wm_delete_window = *__XInternAtom(fXDisplay, "WM_DELETE_WINDOW", 1);

            // Handle closing the window
            if (evt->xclient.message_type == atom_wm_protocols && evt->xclient.data.l[0] == atom_wm_delete_window) {
                client->SetDone(true);
                return true;
            }
        }
        break;
    case ConfigureNotify:
        {
            fWindowSize.Set(evt->xconfigure.x, evt->xconfigure.y, evt->xconfigure.width, evt->xconfigure.height);
            client->GetPipeline()->Resize(fWindowSize.fWidth, fWindowSize.fHeight);
        }
        break;
    case EnterNotify:
        {
            plMouseDevice::ShowCursor();
#ifdef USE_XFIXES
            if ((bool)__XFixesHideCursor && (bool)__XFixesShowCursor) {
                __XFixesHideCursor(fXDisplay, fXWindow);
                __XFlush(fXDisplay);
            }
#endif
        }
        break;
    case LeaveNotify:
        {
            plMouseDevice::HideCursor();
#ifdef USE_XFIXES
            if ((bool)__XFixesHideCursor && (bool)__XFixesShowCursor) {
                __XFixesShowCursor(fXDisplay, fXWindow);
                __XFlush(fXDisplay);
            }
#endif
        }
        break;
    case MotionNotify:
        {
            plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
            plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;

            pXMsg->fWx = evt->xmotion.x;
            pXMsg->fX = (float)evt->xmotion.x / (float)fWindowSize.fWidth;

            pYMsg->fWy = evt->xmotion.y;
            pYMsg->fY = (float)evt->xmotion.y / (float)fWindowSize.fHeight;

            client->GetInputManager()->MsgReceive(pXMsg);
            client->GetInputManager()->MsgReceive(pYMsg);

            delete(pXMsg);
            delete(pYMsg);
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        {
            plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
            plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;
            plIMouseBEventMsg* pBMsg = new plIMouseBEventMsg;

            pXMsg->fWx = evt->xmotion.x;
            pXMsg->fX = (float)evt->xmotion.x / (float)fWindowSize.fWidth;

            pYMsg->fWy = evt->xmotion.y;
            pYMsg->fY = (float)evt->xmotion.y / (float)fWindowSize.fHeight;

            switch (evt->xbutton.button) {
            case Button1:
                pBMsg->fButton |= (evt->xbutton.type == ButtonPress) ? kLeftButtonDown : kLeftButtonUp;
                break;
            case Button2:
                pBMsg->fButton |= (evt->xbutton.type == ButtonPress) ? kMiddleButtonDown : kMiddleButtonUp;
                break;
            case Button3:
                pBMsg->fButton |= (evt->xbutton.type == ButtonPress) ? kRightButtonDown : kRightButtonUp;
                break;
            default:
                break;
            }

            client->SetQuitIntro(true);
            client->GetInputManager()->MsgReceive(pXMsg);
            client->GetInputManager()->MsgReceive(pYMsg);
            client->GetInputManager()->MsgReceive(pBMsg);

            delete(pXMsg);
            delete(pYMsg);
            delete(pBMsg);
        }
        break;
    case MappingNotify:
        {
            __XRefreshKeyboardMapping(&evt->xmapping);
        }
        break;
    case KeyPress:
    case KeyRelease:
        {
            static const unsigned char KEYCODE_LINUX_TO_HID[256] = {
                0, 41, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 43,
                20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 158, 224, 4, 22,
                7, 9, 10, 11, 13, 14, 15, 51, 52, 53, 225, 49, 29, 27, 6, 25,
                5, 17, 16, 54, 55, 56, 229, 85, 226, 44, 57, 58, 59, 60, 61,
                62, 63, 64, 65, 66, 67, 83, 71, 95, 96, 97, 86, 92, 93, 94, 87,
                89, 90, 91, 98, 99, 0, 0, 100, 68, 69, 0, 0, 0, 0, 0, 0, 0, 88,
                228, 84, 154, 230, 0, 74, 82, 75, 80, 79, 77, 81, 78, 73, 76,
                0, 0, 0, 0, 0, 103, 0, 72, 0, 0, 0, 0, 0, 227, 231, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 104, 105, 106, 107, 108, 109,
                110, 111, 112, 113, 114, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0
            };

            bool down = evt->type == KeyPress;

            /* X11 offsets Linux keycodes by 8 */
            uint32_t keycode = evt->xkey.keycode - 8;
            plKeyDef key = KEY_UNMAPPED;
            if (keycode < 256)
                key = (plKeyDef)KEYCODE_LINUX_TO_HID[keycode];

            if (down && key == KEY_Q) { // Quit when Q is hit
                client->SetDone(true);
                return true;
            }

            if (down)
                client->SetQuitIntro(true);

            client->GetInputManager()->HandleKeyEvent(key, down, false, 0);

            if (down) {
                // TODO: Key input
                char buf[6];
                int chars = *__XLookupString(reinterpret_cast<XKeyEvent*>(evt), buf, 6, nullptr, nullptr);

                for (int i = 0; i < chars; i++) {
                    client->GetInputManager()->HandleKeyEvent(key, down, false, buf[i]);
                }
            }
        }
        break;
    default:
        break;
    }

    return false;
}

bool plX11ClientWindow::ProcessEvents()
{
    XEvent ev;
    bool shouldExit = false;

    while (*__XPending(fXDisplay)) {
        __XNextEvent(fXDisplay, &ev);
        if (*__XFilterEvent(&ev, None))
            continue;

        shouldExit |= HandleEvent(&ev);
    }

    return shouldExit;
}

void plX11ClientWindow::DeInit()
{
    if (fXWindow)
        __XDestroyWindow(fXDisplay, fXWindow);
    if (fXDisplay)
        __XCloseDisplay(fXDisplay);
}

void plX11ClientWindow::ResizeClientWindow(uint16_t width, uint16_t height, bool windowed)
{
    __XResizeWindow(fXDisplay, fXWindow, width, height);
    __XSync(fXDisplay, False);
}
