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

#include "plInputSDL.h"
#include "plClientUnix.h"

#include "HeadSpin.h"
#include "plPipeline.h"

#include "plClient.h"
#include "plClientLoader.h"

#include "pnInputCore/plControlDefinition.h"

#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plNetClient/plNetClientMgr.h"

#include <string_theory/string>

#include <cwctype>

namespace
{
    /**
     * The scancode of the key currently generating text.
     *
     * plInputManager::HandleKeyEvent is called twice for a typed character --
     * once for the physical key with no character, then again with the
     * translated character. SDL delivers those as two separate events, so the
     * scancode has to be carried across from the key event to the text event.
     */
    SDL_Scancode s_textScancode = SDL_SCANCODE_UNKNOWN;

    /** Whether we have hidden the OS cursor in favor of the engine-drawn one. */
    bool s_osCursorHidden = false;

    plInputManager* IGetInputManager()
    {
        if (!gClient)
            return nullptr;
        return gClient->GetInputManager();
    }

    /** Logical window size, in points. Never returns zero. */
    void IGetWindowSize(int* width, int* height)
    {
        *width = 0;
        *height = 0;
        if (gWindow)
            SDL_GetWindowSize(gWindow, width, height);

        // Guard the divisions below; a zero-sized window is possible while
        // minimized on some compositors.
        if (*width <= 0)
            *width = 1;
        if (*height <= 0)
            *height = 1;
    }

    /**
     * Feeds an absolute cursor position to the engine, then handles mouse
     * recentering.
     *
     * plInputManager::RecenterCursor() is compiled out on non-Windows, so
     * mouselook does not work unless the frontend warps the pointer itself and
     * synthesizes the messages the warp would have produced. This mirrors
     * what the Cocoa frontend does in PLSView.
     */
    void ISendMouseMotion(float x, float y)
    {
        plInputManager* inputMgr = IGetInputManager();
        if (!inputMgr)
            return;

        int width, height;
        IGetWindowSize(&width, &height);
        const float density = gWindow ? SDL_GetWindowPixelDensity(gWindow) : 1.f;

        plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
        plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;

        pXMsg->fX = x / float(width);
        pYMsg->fY = y / float(height);

        // SDL and Plasma both use top-left-origin window coordinates.
        pXMsg->fWx = int(x * density);
        pYMsg->fWy = int(y * density);

        inputMgr->MsgReceive(pXMsg);
        inputMgr->MsgReceive(pYMsg);

        if (plInputManager::RecenterMouse()) {
            float warpX = x;
            float warpY = y;

            if (pXMsg->fX <= 0.1f || pXMsg->fX >= 0.9f) {
                warpX = float(width) * 0.5f;

                // The warp does not generate a motion event we can trust, so
                // tell the engine where the cursor landed by hand.
                pXMsg->fX = 0.5f;
                pXMsg->fWx = int(warpX * density);
                inputMgr->MsgReceive(pXMsg);
            }

            if (pYMsg->fY <= 0.1f || pYMsg->fY >= 0.9f) {
                warpY = float(height) * 0.5f;

                pYMsg->fY = 0.5f;
                pYMsg->fWy = int(warpY * density);
                inputMgr->MsgReceive(pYMsg);
            }

            if (warpX != x || warpY != y)
                SDL_WarpMouseInWindow(gWindow, warpX, warpY);
        }

        delete pXMsg;
        delete pYMsg;
    }

    void ISendMouseButton(const SDL_MouseButtonEvent& button)
    {
        plInputManager* inputMgr = IGetInputManager();
        if (!inputMgr)
            return;

        // Any click dismisses the intro movie, same as Win32 and Cocoa.
        if (gClient)
            gClient->SetQuitIntro(true);

        ISendMouseMotion(button.x, button.y);

        plIMouseBEventMsg* pBMsg = new plIMouseBEventMsg;

        // The double click bit has to ride along with the button DOWN, matching
        // Win32's WM_LBUTTONDBLCLK. plMouseDevice::MsgReceive rewrites any
        // message carrying the bit into a button down, so flagging the release
        // would swallow it -- leaving pfGUIDialogMod::fControlOfInterest latched
        // on one control and every other control unclickable.
        switch (button.button) {
        case SDL_BUTTON_LEFT:
            pBMsg->fButton |= button.down ? kLeftButtonDown : kLeftButtonUp;
            if (button.down && button.clicks == 2)
                pBMsg->fButton |= kLeftButtonDblClk;
            break;
        case SDL_BUTTON_RIGHT:
            pBMsg->fButton |= button.down ? kRightButtonDown : kRightButtonUp;
            if (button.down && button.clicks == 2)
                pBMsg->fButton |= kRightButtonDblClk;
            break;
        case SDL_BUTTON_MIDDLE:
            pBMsg->fButton |= button.down ? kMiddleButtonDown : kMiddleButtonUp;
            break;
        default:
            // Plasma has no bindings for the extra buttons.
            delete pBMsg;
            return;
        }

        inputMgr->MsgReceive(pBMsg);
        delete pBMsg;
    }

    void ISendMouseWheel(const SDL_MouseWheelEvent& wheel)
    {
        if (wheel.y == 0.f)
            return;

        int width, height;
        IGetWindowSize(&width, &height);

        // Unlike the other mouse messages, the wheel goes through the dispatcher.
        // Consumers expect Win32's WHEEL_DELTA units (120 per notch); SDL reports
        // one notch as 1.0.
        plMouseEventMsg* pMsg = new plMouseEventMsg;
        pMsg->SetWheelDelta(wheel.y * 120.f);
        pMsg->SetButton(wheel.y < 0.f ? kWheelNeg : kWheelPos);
        pMsg->SetXPos(wheel.mouse_x / float(width));
        pMsg->SetYPos(wheel.mouse_y / float(height));
        pMsg->Send();
    }

    void ISendKey(const SDL_KeyboardEvent& key)
    {
        plInputManager* inputMgr = IGetInputManager();
        if (!inputMgr)
            return;

        const plKeyDef keyDef = plInputSDL::TranslateScancode(key.scancode);

        // Track the scancode even for keys we drop, otherwise a following text
        // event gets attributed to whatever key was pressed before this one.
        s_textScancode = (key.down && keyDef != KEY_UNMAPPED) ? key.scancode
                                                              : SDL_SCANCODE_UNKNOWN;

        if (keyDef == KEY_UNMAPPED)
            return;

        if (key.down && gClient)
            gClient->SetQuitIntro(true);

        inputMgr->HandleKeyEvent(keyDef, key.down, key.repeat);
    }

    void ISendText(const SDL_TextInputEvent& text)
    {
        plInputManager* inputMgr = IGetInputManager();
        if (!inputMgr || !text.text)
            return;

        const plKeyDef keyDef = plInputSDL::TranslateScancode(s_textScancode);

        ST::wchar_buffer chars = ST::string::from_utf8(text.text).to_wchar();
        for (size_t i = 0; i < chars.size(); ++i) {
            if (std::iswcntrl(chars[i]))
                continue;
            inputMgr->HandleKeyEvent(keyDef, true, false, chars[i]);
        }
    }

    void IShowOSCursor(bool show)
    {
        if (show == !s_osCursorHidden)
            return;
        s_osCursorHidden = !show;

        // The engine draws its own cursor as a plPlate, so the two are inverses.
        if (show) {
            SDL_ShowCursor();
            plMouseDevice::HideCursor();
        } else {
            SDL_HideCursor();
            plMouseDevice::ShowCursor();
        }
    }

    void IHandleWindowEvent(const SDL_WindowEvent& window)
    {
        switch (window.type) {
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            if (gClient)
                gClient->WindowActivate(true);
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (gClient)
                gClient->WindowActivate(false);
            break;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            IShowOSCursor(false);
            break;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            IShowOSCursor(true);
            break;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            // data1/data2 are pixels here, not points. The engine only wants the
            // scale factor; plDisplayScaleChangedMsg is the whole DPI contract.
            if (gWindow) {
                plDisplayScaleChangedMsg* msg =
                    new plDisplayScaleChangedMsg(SDL_GetWindowPixelDensity(gWindow));
                msg->Send();
            }
            if (gClient && gClient->GetPipeline())
                gClient->GetPipeline()->Resize(window.data1, window.data2);

            // plClient::IResizeWindow only runs when plDisplayScaleChangedMsg
            // carries a suggested location, and the route that does supply one
            // also re-centers the window -- unusable while the user is dragging
            // a resize. Update the two things it would have set by hand instead.
            if (window.data1 > 0 && window.data2 > 0) {
                if (plMouseDevice::Instance()) {
                    plMouseDevice::Instance()->SetDisplayResolution(float(window.data1),
                                                                    float(window.data2));
                }
                if (pfGameGUIMgr::GetInstance()) {
                    pfGameGUIMgr::GetInstance()->SetAspectRatio(float(window.data1) /
                                                                float(window.data2));
                }
            }
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (gClient) {
                gClient->SetDone(true);
                plNetClientMgr* mgr = plNetClientMgr::GetInstance();
                if (mgr)
                    mgr->QueueDisableNet(false, nullptr);
            }
            break;

        default:
            break;
        }
    }
}

plKeyDef plInputSDL::TranslateScancode(SDL_Scancode scancode)
{
    switch (scancode) {
    // Plasma numbers Return outside the HID range, and has no separate keypad
    // Enter, so both land on KEY_ENTER.
    case SDL_SCANCODE_RETURN:
    case SDL_SCANCODE_KP_ENTER:
        return KEY_ENTER;

    // Plasma only defines the left-hand modifiers.
    case SDL_SCANCODE_RCTRL:
        return KEY_CTRL;
    case SDL_SCANCODE_RSHIFT:
        return KEY_SHIFT;
    case SDL_SCANCODE_RALT:
        return KEY_ALT;

    // SDL_SCANCODE_EXSEL collides with the value Plasma gave KEY_ENTER. Nothing
    // binds it, so drop it rather than let it masquerade as Enter.
    case SDL_SCANCODE_EXSEL:
        return KEY_UNMAPPED;

    default:
        break;
    }

    // plKeyboardDevice tracks a 256-entry key state array and ignores anything
    // outside it (plInputDevice.cpp), so there is no point forwarding the rest.
    if (scancode > 0 && scancode < 256)
        return static_cast<plKeyDef>(scancode);

    return KEY_UNMAPPED;
}

void plInputSDL::HandleEvent(const SDL_Event& evt)
{
    switch (evt.type) {
    case SDL_EVENT_QUIT:
        if (gClient)
            gClient->SetDone(true);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        ISendMouseMotion(evt.motion.x, evt.motion.y);
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        ISendMouseButton(evt.button);
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        ISendMouseWheel(evt.wheel);
        break;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        ISendKey(evt.key);
        break;

    case SDL_EVENT_TEXT_INPUT:
        ISendText(evt.text);
        break;

    default:
        if (evt.type >= SDL_EVENT_WINDOW_FIRST && evt.type <= SDL_EVENT_WINDOW_LAST)
            IHandleWindowEvent(evt.window);
        break;
    }
}
