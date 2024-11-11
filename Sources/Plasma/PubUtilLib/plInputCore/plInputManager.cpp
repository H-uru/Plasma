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
// plInputManager.cpp
#include "HeadSpin.h"
#include "hsWindows.h"
#ifdef WIN32
#   include <Windowsx.h>
#endif

#include "plInputManager.h"
#include "plPipeline.h"
#include "plInputDevice.h"
#include "plMessage/plInputEventMsg.h"
#include "plInputInterfaceMgr.h"
#include "hsStream.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plgDispatch.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "pnMessage/plPlayerPageMsg.h"

uint8_t plInputManager::bRecenterMouse = 0;
hsWindowHndl plInputManager::fhWnd = nullptr;
plInputManager* plInputManager::fInstance = nullptr;

plInputManager::plInputManager(hsWindowHndl hWnd) :
    fInterfaceMgr(nullptr),
    localeC("C")
{
    fhWnd = hWnd;
    fInstance = this;
    fActive = false;
    fFirstActivated = false;
    fMouseScale = 1.f;
}

plInputManager::plInputManager() :
    fInterfaceMgr(nullptr)
{
    fInstance = this;
    fActive = false;
    fFirstActivated = false;
    fMouseScale = 1.f;
}

plInputManager::~plInputManager()
{
    fInterfaceMgr->Shutdown();
    fInterfaceMgr = nullptr;

    for (plInputDevice* inputDevice : fInputDevices)
    {
        inputDevice->Shutdown();
        delete inputDevice;
    }
}

//static
void plInputManager::SetRecenterMouse(bool b)
{
    if (b)
        bRecenterMouse++;
    else if (bRecenterMouse > 0)
        bRecenterMouse--;
}


void plInputManager::RecenterCursor()
{
#ifdef HS_BUILD_FOR_WIN32
    RECT rect;
    GetClientRect(fhWnd, &rect);
    POINT pt;
//  pt.y = ( (rect.bottom - rect.top) / 2 ) / fInstance->fMouseScale;
//  pt.x = ( (rect.right - rect.left) / 2 ) / fInstance->fMouseScale;
    ClientToScreen(fhWnd, &pt);
    SetCursorPos( pt.x, pt.y );
#endif
}

void plInputManager::CreateInterfaceMod(plPipeline* p)
{
    fInterfaceMgr = new plInputInterfaceMgr();
    fInterfaceMgr->Init();
}

bool plInputManager::MsgReceive(plMessage* msg)
{
    for (plInputDevice* inputDevice : fInputDevices)
        if (inputDevice->MsgReceive(msg))
            return true;

    return hsKeyedObject::MsgReceive(msg);
}

void plInputManager::Update()
{
}

void plInputManager::SetMouseScale(float s)
{
/*  RECT    rect;
    POINT   currPos;


    // Gotta make sure to move the mouse to the correct new position for the scale
    GetClientRect( fhWnd, &rect );
    GetCursorPos( &currPos );
    ScreenToClient( fhWnd, &currPos );

    float x = (float)currPos.x / rect.right;
    float y = (float)currPos.y / rect.bottom;

    x *= fMouseScale; y *= fMouseScale;

    fMouseScale = s;

    // Refreshes all of the input devices so that they can reset mouse limits, etc
    RECT rect2 = rect;
    rect2.right /= fMouseScale;
    rect2.bottom /= fMouseScale;
    ::MapWindowPoints(fhWnd, nullptr, (POINT *)&rect2, 2);
    BOOL ret = ::ClipCursor( &rect );

    // Now move the cursor to the right spot

    currPos.x = ( x / fMouseScale ) * rect.right;
    currPos.y = ( y / fMouseScale ) * rect.bottom;

    ClientToScreen( fhWnd, &currPos );
    SetCursorPos( currPos.x, currPos.y );
*/
}

// Sometimes the keyboard driver "helps" us translating a key involved in a key
// combo. For example pressing shif-numpad8 will actually generate a KEY_UP event,
// the same as the up arrow. This function undoes that translation.
plKeyDef plInputManager::UntranslateKey(plKeyDef key, bool extended)
{
    if (!extended)
    {
        if (key == KEY_UP)
            return KEY_NUMPAD8;
        if (key == KEY_DOWN)
            return KEY_NUMPAD2;
        if (key == KEY_LEFT)
            return KEY_NUMPAD4;
        if (key == KEY_RIGHT)
            return KEY_NUMPAD6;
    }

    return key;
}

void plInputManager::HandleKeyEvent(plKeyDef key, bool bKeyDown, bool bKeyRepeat, wchar_t c)
{
    for (plInputDevice* inputDevice : fInputDevices)
    {
        inputDevice->HandleKeyEvent(key, bKeyDown, bKeyRepeat, c);
    }
}

#if HS_BUILD_FOR_WIN32
/** Determines if we need to hackily flush cursor updates
 *  \remarks Normally, we would just call SetCursorPos directly. However, in Windows 10's
 *           2017 Fall Creator's Update, SetCursorPos, GetCursorPos, and WM_MOUSEMOVE are buggy.
 *           Research done by Deledrius matches my independent observations and failed fixes:
 *           https://discourse.libsdl.org/t/win10-fall-creators-update-breaks-mouse-warping/23526
 *           Many thanks to these fine folks who work on libsdl!
 */
static bool INeedsWin10CursorHack()
{
    // According to Chromium, Microsoft will be fixing the cursor bug in the next build
    // of Windows 10, so we only need to test for the dreaded 2017 FCU...
    // Reference: https://bugs.chromium.org/p/chromium/issues/detail?id=781182#c15
    // UPDATE: Bug is still present in the April 2018 Update (build 17134)...
    //         so this bandage will now be applied to anything 2017 FCU and later :(
    const RTL_OSVERSIONINFOEXW& version = hsGetWindowsVersion();
    return version.dwMajorVersion == 10 && version.dwBuildNumber >= 16299;
}

void plInputManager::HandleWin32ControlEvent(UINT message, WPARAM Wparam, LPARAM Lparam, hsWindowHndl hWnd)
{
    if( !fhWnd )
        fhWnd = hWnd;

    bool bExtended;

    switch (message)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            bExtended = Lparam >> 24 & 1;
            bool bRepeat = ((Lparam >> 29) & 0xf) != 0;
            bool down = !(Lparam >> 31);

            HandleKeyEvent( UntranslateKey((plKeyDef)Wparam, bExtended), down, down & bRepeat );
        }
        break;
    case WM_CHAR:
        {
            wchar_t ch = (wchar_t)Wparam;

            // These are handled by KEYUP/KEYDOWN and should not be sent
            // We don't like garbage getting in string fields
            if (ch < 0x80 && std::iscntrl(ch, localeC))
                break;

            BYTE scan = (BYTE)(Lparam >> 16);
            UINT vkey = MapVirtualKey(scan, 1); //MAPVK_VSC_TO_VK

            bExtended = Lparam >> 24 & 1;
            bool bRepeat = ((Lparam >> 29) & 0xf) != 0;
            bool down = !(Lparam >> 31);
 
            HandleKeyEvent( (plKeyDef)vkey, down, bRepeat, ch );
        }
        break;
    case WM_MOUSEWHEEL:
        {
            plMouseEventMsg* pMsg = new plMouseEventMsg;
            int zDelta = GET_WHEEL_DELTA_WPARAM(Wparam);
            pMsg->SetWheelDelta((float)zDelta);
            if (zDelta < 0)
                pMsg->SetButton(kWheelNeg);
            else
                pMsg->SetButton(kWheelPos);

            RECT rect;
            GetClientRect(hWnd, &rect);
            pMsg->SetXPos(LOWORD(Lparam) / (float)rect.right);
            pMsg->SetYPos(HIWORD(Lparam) / (float)rect.bottom);

            pMsg->Send();
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        {
            
            RECT rect;
            GetClientRect(hWnd, &rect);
         
            plIMouseXEventMsg* pXMsg = new plIMouseXEventMsg;
            plIMouseYEventMsg* pYMsg = new plIMouseYEventMsg;
            plIMouseBEventMsg* pBMsg = new plIMouseBEventMsg;

            pXMsg->fWx = GET_X_LPARAM(Lparam);
            pXMsg->fX = (float)GET_X_LPARAM(Lparam) / (float)rect.right;
            pYMsg->fWy = GET_Y_LPARAM(Lparam);
            pYMsg->fY = (float)GET_Y_LPARAM(Lparam) / (float)rect.bottom;

            // Apply mouse scale
//          pXMsg->fX *= fMouseScale;
//          pYMsg->fY *= fMouseScale;
            
            if (Wparam & MK_LBUTTON && message != WM_LBUTTONUP)
            {
                pBMsg->fButton |= kLeftButtonDown;
            }
            else
            {
                pBMsg->fButton |= kLeftButtonUp;
            }
                        
            if (Wparam & MK_RBUTTON && message != WM_RBUTTONUP)
            {
                pBMsg->fButton |= kRightButtonDown;
            }
            else
            {
                pBMsg->fButton |= kRightButtonUp;
            }
            
            if (Wparam & MK_MBUTTON && message != WM_MBUTTONUP)
            {
                pBMsg->fButton |= kMiddleButtonDown;
            }
            else
            {
                pBMsg->fButton |= kMiddleButtonUp;
            }
            
            if( message == WM_LBUTTONDBLCLK )
                pBMsg->fButton |= kLeftButtonDblClk;        // We send the double clicks separately
            if( message == WM_RBUTTONDBLCLK )
                pBMsg->fButton |= kRightButtonDblClk;

            for (plInputDevice* inputDevice : fInputDevices)
            {
                inputDevice->MsgReceive(pXMsg);
                inputDevice->MsgReceive(pYMsg);
                inputDevice->MsgReceive(pBMsg);
            }
            POINT pt;
            
            if (RecenterMouse() && (pXMsg->fX <= 0.1 || pXMsg->fX >= 0.9) )
            {       
                pt.x = (rect.right - rect.left) / 2;
                pt.y = HIWORD(Lparam);
                if (INeedsWin10CursorHack()) {
                    pXMsg->fWx = pt.x;
                    pXMsg->fX = pt.x / (float)rect.right;
                    for (plInputDevice* inputDevice : fInputDevices)
                        inputDevice->MsgReceive(pXMsg);
                }
                ClientToScreen(hWnd, &pt);
                SetCursorPos( pt.x, pt.y );
            }
            else
            if (RecenterMouse() && (pYMsg->fY <= 0.1 || pYMsg->fY >= 0.9) )
            {       
                pt.y = (rect.bottom - rect.top) / 2;
                pt.x = LOWORD(Lparam);
                if (INeedsWin10CursorHack()) {
                    pYMsg->fWy = pt.y;
                    pYMsg->fY = pYMsg->fWy / (float)rect.bottom;
                    for (plInputDevice* inputDevice : fInputDevices)
                        inputDevice->MsgReceive(pYMsg);
                }
                ClientToScreen(hWnd, &pt);
                SetCursorPos( pt.x, pt.y );
            }
            if (RecenterMouse() && Lparam == 0)
            {
                pt.y = (rect.bottom - rect.top) / 2;
                pt.x = (rect.right - rect.left) / 2;
                if (INeedsWin10CursorHack()) {
                    pXMsg->fWx = pt.x;
                    pXMsg->fX = pXMsg->fWx / (float)rect.right;
                    pYMsg->fWy = pt.y;
                    pYMsg->fY = pYMsg->fWy / (float)rect.bottom;

                    for (plInputDevice* inputDevice : fInputDevices) {
                        inputDevice->MsgReceive(pXMsg);
                        inputDevice->MsgReceive(pYMsg);
                    }
                }
                ClientToScreen(hWnd, &pt);
                SetCursorPos( pt.x, pt.y );
            }
            delete(pXMsg);
            delete(pYMsg);
            delete(pBMsg);

        }
        break;
    case WM_ACTIVATE:
        {
            bool activated = ( LOWORD( Wparam ) == WA_INACTIVE ) ? false : true;
            Activate( activated );
        }
        break;
    }

}
#endif

//// Activate ////////////////////////////////////////////////////////////////
//  Handles what happens when the app (window) activates/deactivates

void plInputManager::Activate(bool activating)
{
    for (plInputDevice* inputDevice : fInputDevices)
        inputDevice->HandleWindowActivate(activating, fhWnd);

    fActive = activating;
    fFirstActivated = true;
}

//// AddInputDevice //////////////////////////////////////////////////////////

void plInputManager::AddInputDevice(plInputDevice* pDev)
{
    fInputDevices.emplace_back(pDev);

    if (fFirstActivated)
        pDev->HandleWindowActivate(fActive, fhWnd);
}
