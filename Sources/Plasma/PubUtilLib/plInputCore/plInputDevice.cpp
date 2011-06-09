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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
// plInputDevice.cpp
//#include "STRING"

#include "hsConfig.h"
#include "hsWindows.h"

#include "plInputDevice.h"
#include "plInputManager.h"
#include "plAvatarInputInterface.h"
#include "plMessage/plInputEventMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "hsUtils.h"
#include "plgDispatch.h"

#include "plPipeline/plPlates.h"
#include "plPipeline/plDebugText.h"

#include "hsWindows.h"
#include "plPipeline.h"

// base size of the cursor 
#define CURSOR_SIZE_X 0.0675f   
#define CURSOR_SIZE_Y 0.09f

// The resolution that uses the base size of the cursor. 
// All other resolutions will scale the cursor size to keep the same physical size.
#define BASE_WIDTH 1024
#define BASE_HEIGHT 768

plKeyboardDevice* plKeyboardDevice::fInstance = nil;
bool plKeyboardDevice::fKeyboardState[256];
hsBool plKeyboardDevice::fIgnoreCapsLock = false;
hsBool plKeyboardDevice::fKeyIsDeadKey = false;

plKeyboardDevice::plKeyboardDevice() :
fShiftKeyDown(false),
fCapsLockKeyDown(false),
fAltKeyDown(false),
fCtrlKeyDown(false),
fCapsLockLock(false),
fControlMode(STANDARD_MODE)
{
    fInstance = this;
    InitKeyboardState();
}

plKeyboardDevice::~plKeyboardDevice()
{
}

void plKeyboardDevice::InitKeyboardState()
{
    static bool initialized = false;
    if (!initialized)
    {
        for (unsigned int i = 0; i < 256; ++i)
            plKeyboardDevice::fKeyboardState[i] = false;
        initialized = true;
    }
}

void plKeyboardDevice::ReleaseAllKeys()
{
    // send a key-up message for all "normal" keys
    for (unsigned int i = 0; i < 256; ++i)
    {
        if ((i == KEY_SHIFT) || (i == KEY_CTRL) || (i == KEY_CAPSLOCK))
            continue; // these are handled slightly differently

        if (fKeyboardState[i])
        {
            fKeyboardState[i] = false;

            // fake a key-up command
            plKeyEventMsg* pMsg = TRACKED_NEW plKeyEventMsg;
            pMsg->SetKeyCode( (plKeyDef)i );
            pMsg->SetKeyDown( false );
            pMsg->SetShiftKeyDown( fShiftKeyDown );
            pMsg->SetCtrlKeyDown( fCtrlKeyDown );
            pMsg->SetCapsLockKeyDown( fCapsLockLock );
            pMsg->SetRepeat( false );
            plgDispatch::MsgSend( pMsg );
        }
    }

    // send key messages for shift and ctrl if necessary because the keys above need to have
    // the proper states of the shift and ctrl keys sent with their messages, and our internal
    // flags for these keys need to be cleared. We don't send a key-up message for caps lock
    // because it doesn't really operate like every other key on the keyboard
    if (fKeyboardState[KEY_SHIFT])
    {
        fKeyboardState[KEY_SHIFT] = false;
        fShiftKeyDown = false;

        plKeyEventMsg* pMsg = TRACKED_NEW plKeyEventMsg;
        pMsg->SetKeyCode( KEY_SHIFT );
        pMsg->SetKeyDown( false );
        pMsg->SetShiftKeyDown( false );
        pMsg->SetCtrlKeyDown( false );
        pMsg->SetCapsLockKeyDown( fCapsLockLock );
        pMsg->SetRepeat( false );
        plgDispatch::MsgSend( pMsg );
    }
    if (fKeyboardState[KEY_CTRL])
    {
        fKeyboardState[KEY_CTRL] = false;
        fCtrlKeyDown = false;

        plKeyEventMsg* pMsg = TRACKED_NEW plKeyEventMsg;
        pMsg->SetKeyCode( KEY_CTRL );
        pMsg->SetKeyDown( false );
        pMsg->SetShiftKeyDown( false );
        pMsg->SetCtrlKeyDown( false );
        pMsg->SetCapsLockKeyDown( fCapsLockLock );
        pMsg->SetRepeat( false );
        plgDispatch::MsgSend( pMsg );
    }
}

hsBool plKeyboardDevice::IsCapsLockKeyOn()
{
    return fCapsLockLock;
}

void plKeyboardDevice::Shutdown()
{
}

void plKeyboardDevice::HandleKeyEvent(plOSMsg message, plKeyDef key, bool bKeyDown, hsBool bKeyRepeat)
{
    // update the internal keyboard state
    unsigned int keyCode = (unsigned int)key;
    if ((key >= 0) && (key < 256))
        fKeyboardState[key] = bKeyDown;

    if (key == KEY_SHIFT)
    {
        fShiftKeyDown = bKeyDown;
//      return;
    }
    if (key == KEY_CTRL)
    {
        fCtrlKeyDown = bKeyDown;
//      return;
    }
    if (key == KEY_CAPSLOCK)
    {
        // Keyboards toggle the light on key-down, so I'm going with that.
        if (bKeyDown && !bKeyRepeat)
        {
            fCapsLockLock = !fCapsLockLock;
            plAvatarInputInterface::GetInstance()->ForceAlwaysRun(fCapsLockLock);
        }
    }

    // send a key event...
    plKeyEventMsg* pMsg = TRACKED_NEW plKeyEventMsg;
    pMsg->SetKeyCode( key );
    pMsg->SetKeyDown( bKeyDown );
    pMsg->SetShiftKeyDown( fShiftKeyDown );
    pMsg->SetCtrlKeyDown( fCtrlKeyDown );
    pMsg->SetCapsLockKeyDown( fCapsLockLock );
    pMsg->SetRepeat(bKeyRepeat);
    plgDispatch::MsgSend( pMsg );
}

void plKeyboardDevice::HandleWindowActivate(bool bActive, HWND hWnd)
{
    if (bActive)
    {
        fCtrlKeyDown = false;
    }
    else
    {
        ReleaseAllKeys(); // send key-up events for everything since we're losing focus
    }

}

//// KeyEventToChar //////////////////////////////////////////////////////////
//  Translate a Plasma key event to an actual char
wchar_t    plKeyboardDevice::KeyEventToChar( plKeyEventMsg *msg )
{
    unsigned int   code = msg->GetKeyCode();
    wchar_t    c = 0;
    unsigned char kbState[256];
    wchar_t buffer[256];
    unsigned int scanCode;
    int     retVal;

    buffer[0] = 0;

    // let windows translate everything for us!
    scanCode = MapVirtualKeyW(code, 0);
    GetKeyboardState(kbState);
    if (fIgnoreCapsLock)
        kbState[KEY_CAPSLOCK] = 0; // clear the caps lock key
    retVal = ToUnicode(code, scanCode, kbState, (wchar_t*)buffer, 256, 0);
    if (retVal == -1)
    {
        // It's a stored dead key.
        c = 0;
        fKeyIsDeadKey = true;
    }
    else if (retVal == 0)
        // Invalid crap
        c = 0;
    else if (retVal == 1)
    {
        // Exactly one good character
        fKeyIsDeadKey = false;
        c = buffer[0];
    }
    else if (retVal >= 2)
    {
        fKeyIsDeadKey = !fKeyIsDeadKey;
        if (!fKeyIsDeadKey)
            c = buffer[0];
    }

    return c;
}


//
//
//
// plMouseDevice
//
//

bool plMouseDevice::bMsgAlways = true;
bool plMouseDevice::bCursorHidden = false;
bool plMouseDevice::bCursorOverride = false;
bool plMouseDevice::bInverted = false;
hsScalar plMouseDevice::fWidth = BASE_WIDTH;
hsScalar plMouseDevice::fHeight = BASE_HEIGHT;
plMouseDevice* plMouseDevice::fInstance = 0;

plMouseDevice::plMouseDevice()
{
    fXPos = 0;
    fYPos = 0;
    fCursorID = CURSOR_UP;
    fButtonState = 0;
    fOpacity = 1.f;

    fCursor = nil;
    CreateCursor( fCursorID );
    plMouseDevice::fInstance = this;
    fXMsg = nil;
    fYMsg = nil;
    fB2Msg = nil;

    fLeftBMsg[0] = nil;
    fLeftBMsg[1] = nil;
    fRightBMsg[0] = nil;
    fRightBMsg[1] = nil;
    fMiddleBMsg[0] = nil;
    fMiddleBMsg[1] = nil;
    
}

plMouseDevice::~plMouseDevice()
{
    plPlateManager::Instance().DestroyPlate( fCursor );
    fCursor = nil;
    plMouseDevice::fInstance = nil;
}
void plMouseDevice::SetDisplayResolution(hsScalar Width, hsScalar Height)
{
    fWidth = Width;
    fHeight = Height;
    IUpdateCursorSize();
}

void    plMouseDevice::CreateCursor( int cursor )
{
    if( fCursor == nil )
    {
        plPlateManager::Instance().CreatePlate( &fCursor );
        fCursor->CreateFromResource( MAKEINTRESOURCE( cursor ) );
    }
    else
    {
        fCursor->ReloadFromResource( MAKEINTRESOURCE( cursor ) );
    }
    fCursor->SetPosition( 0, 0, 0 );
    IUpdateCursorSize();

    fCursor->SetVisible( true );
    fCursor->SetOpacity( fOpacity );
}

void plMouseDevice::IUpdateCursorSize()
{
    if(fCursor)
    {
        // set the size of the cursor based on resolution.
        fCursor->SetSize( CURSOR_SIZE_X * BASE_WIDTH / fWidth, CURSOR_SIZE_Y * BASE_HEIGHT / fHeight );
    }
}

void plMouseDevice::AddNameToCursor(const char* name)
{
    if (fInstance && name)
    {
        plDebugText     &txt = plDebugText::Instance();
        txt.DrawString(fInstance->fWXPos + 12 ,fInstance->fWYPos - 7,name);
    }
}
void plMouseDevice::AddCCRToCursor()
{
    if (fInstance)
    {
        plDebugText     &txt = plDebugText::Instance();
        txt.DrawString(fInstance->fWXPos + 12, fInstance->fWYPos - 17, "CCR");
    }
}
void plMouseDevice::AddIDNumToCursor(UInt32 idNum)
{
    if (fInstance && idNum)
    {
        plDebugText     &txt = plDebugText::Instance();
        char str[256];
        sprintf(str, "%d",idNum);
        txt.DrawString(fInstance->fWXPos + 12 ,fInstance->fWYPos + 3,str);
    }
}
        

void plMouseDevice::SetCursorX(hsScalar x)
{
    /// Set the cursor position
    if( fCursor == nil  && !plMouseDevice::bCursorHidden)
        CreateCursor( fCursorID );
    
    if (fCursor)
        fCursor->SetPosition( ( x * 2.0f ) - 1.0f, 
                          ( fYPos * 2.0f ) - 1.0f );

//  plDebugText     &txt = plDebugText::Instance();
//  txt.DrawString(fWXPos + 20,fWYPos - 5,"test");

}
void plMouseDevice::SetCursorY(hsScalar y)
{
    /// Set the cursor position
    if( fCursor == nil && !plMouseDevice::bCursorHidden)
        CreateCursor( fCursorID );
    
    if (fCursor)
        fCursor->SetPosition( ( fXPos * 2.0f ) - 1.0f, 
                          ( y * 2.0f ) - 1.0f );

//  plDebugText     &txt = plDebugText::Instance();
//  txt.DrawString(fWXPos + 20,fWYPos - 10,"test");

}


void plMouseDevice::HideCursor(hsBool override)
{
    if( fInstance->fCursor != nil )
        fInstance->fCursor->SetVisible( false );

    plMouseDevice::bCursorOverride = (override != 0);
    plMouseDevice::bCursorHidden = true;

}

void plMouseDevice::ShowCursor(hsBool override)
{
    if( !plMouseDevice::bCursorHidden )
        return;
    if (plMouseDevice::bCursorOverride && !override)
        return;

    plMouseDevice::bCursorHidden = false;
    plMouseDevice::bCursorOverride = false;
    
    if( fInstance->fCursor == nil )
        fInstance->CreateCursor( fInstance->fCursorID );
    fInstance->fCursor->SetVisible( true );
}

void plMouseDevice::NewCursor(int cursor)
{
    fInstance->fCursorID = cursor;
    fInstance->CreateCursor(cursor);
    fInstance->SetCursorX(fInstance->GetCursorX());
    fInstance->SetCursorY(fInstance->GetCursorY());
    
    if (!plMouseDevice::bCursorHidden)
        fInstance->fCursor->SetVisible( true );
}

void    plMouseDevice::SetCursorOpacity( hsScalar opacity )
{
    fInstance->fOpacity = opacity;
    if( fInstance->fCursor != nil )
        fInstance->fCursor->SetOpacity( opacity );
}

hsBool plMouseDevice::MsgReceive(plMessage* msg)
{   
    plEvalMsg* pEMsg = plEvalMsg::ConvertNoRef(msg);
    if (pEMsg)
    {
        if (fXMsg)
        {
            plgDispatch::MsgSend(fXMsg);
            fXMsg = nil;
        }
        else
        {
            plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
            pMsg->SetXPos( fXPos );
            pMsg->SetYPos( fYPos );
            pMsg->SetDX(0);
            pMsg->SetDY(0);
            plgDispatch::MsgSend(pMsg);
        }

        if (fYMsg)
        {
            plgDispatch::MsgSend(fYMsg);
            fYMsg = nil;
        }
        else
        {
            plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
            pMsg->SetXPos( fXPos );
            pMsg->SetYPos( fYPos );
            pMsg->SetDX(0);
            pMsg->SetDY(0);
            plgDispatch::MsgSend(pMsg);
        }

        if( fB2Msg )
        {
            fB2Msg->Send();
            fB2Msg = nil;
        }

        // look for mouse button events in the queues to be sent now
        // ...Left mouse button
        if ( fLeftBMsg[0] != nil)
        {
            fLeftBMsg[0]->Send();
            // slide queue elements over... get 'em on the next eval
            fLeftBMsg[0] = fLeftBMsg[1];
            fLeftBMsg[1] = nil;
        }
        // ...Right mouse button
        if ( fRightBMsg[0] != nil)
        {
            fRightBMsg[0]->Send();
            // slide queue elements over... get 'em on the next eval
            fRightBMsg[0] = fRightBMsg[1];
            fRightBMsg[1] = nil;
        }
        // ...middle mouse button
        if ( fMiddleBMsg[0] != nil)
        {
            fMiddleBMsg[0]->Send();
            // slide queue elements over... get 'em on the next eval
            fMiddleBMsg[0] = fMiddleBMsg[1];
            fMiddleBMsg[1] = nil;
        }
    
    }
    
    plIMouseXEventMsg* pXMsg = plIMouseXEventMsg::ConvertNoRef(msg);
    if (pXMsg)
    {
        // send a mouse event
        plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
        if (pXMsg->fX == 999)
            pMsg->SetXPos( fXPos + 0.001f );
        else
        if (pXMsg->fX == -999)
            pMsg->SetXPos( fXPos - 0.001f );
        else
            pMsg->SetXPos(pXMsg->fX);
        pMsg->SetYPos( fYPos );
        pMsg->SetDX( ( fXPos - pMsg->GetXPos())  );
        pMsg->SetDY(0);
        
        if (pMsg->GetDX() == 0.0f && !plMouseDevice::bMsgAlways)
        {   
            delete pMsg;
            return true;
        }
        if (fXMsg)
            delete fXMsg;
        fXMsg = pMsg;
        
        if (pXMsg->fX == 999)
            fXPos += 0.01;
        else
        if (pXMsg->fX == -999)
            fXPos -= 0.01;
        else
            fXPos = pXMsg->fX;

        SetCursorX(fXPos);
        fWXPos = pXMsg->fWx;
        return true;
    }

    plIMouseYEventMsg* pYMsg = plIMouseYEventMsg::ConvertNoRef(msg);
    if (pYMsg)
    {
        // send a mouse event
        plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
        pMsg->SetXPos( fXPos );
        if (pYMsg->fY == 999)
            pMsg->SetYPos( fYPos + 0.01f );
        else
        if (pYMsg->fY == -999)
            pMsg->SetYPos( fYPos - 0.01f );
        else
            pMsg->SetYPos(pYMsg->fY);
        pMsg->SetDX(0);
        pMsg->SetDY(fYPos  - pMsg->GetYPos());
        
        if (pMsg->GetDY() == 0.0f && !plMouseDevice::bMsgAlways)
        {   
            delete pMsg;
            return true;
        }
        if (fYMsg)
            delete fYMsg;
        fYMsg = pMsg;
        
        if (pYMsg->fY == 999)
            fYPos += 0.01;
        else
        if (pYMsg->fY == -999)
            fYPos -= 0.01;
        else
            fYPos = pYMsg->fY;

        fWYPos = pYMsg->fWy;
        SetCursorY(fYPos);
        
        return true;
    }
    plIMouseBEventMsg* pBMsg = plIMouseBEventMsg::ConvertNoRef(msg);
    if (pBMsg)
    {
        
        // send a mouse event
        plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
        pMsg->SetXPos( fXPos );
        pMsg->SetYPos( fYPos );
        pMsg->SetDX(0);
        pMsg->SetDY(0);

        bool deleteMe = true;

        // which button is different?
        if (pBMsg->fButton & kLeftButtonDown && !(fButtonState & kLeftButtonDown))
        {
            // left button now down
            fButtonState |= kLeftButtonDown;
            pMsg->SetButton( kLeftButtonDown );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kLeftButtonUp && fButtonState & kLeftButtonDown)
        {
            // left button now up
            fButtonState &= ~kLeftButtonDown;
            pMsg->SetButton( kLeftButtonUp );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kRightButtonDown && !(fButtonState & kRightButtonDown))
        {
            // right button now down
            fButtonState |= kRightButtonDown;
            pMsg->SetButton( kRightButtonDown );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kRightButtonUp && fButtonState & kRightButtonDown)
        {
            // right button now up
            fButtonState &= ~kRightButtonDown;
            pMsg->SetButton( kRightButtonUp );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kMiddleButtonDown && !(fButtonState & kMiddleButtonDown))
        {
            // mouse wheel button now down
            fButtonState |= kMiddleButtonDown;
            pMsg->SetButton( kMiddleButtonDown );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kMiddleButtonUp && fButtonState & kMiddleButtonDown)
        {
            // right button now up
            fButtonState &= ~kMiddleButtonDown;
            pMsg->SetButton( kMiddleButtonUp );
            deleteMe = false;
        }

        if (pBMsg->fButton & kRightButtonDblClk)
        {
            // right button dbl clicked, send TWO messages
            plMouseEventMsg* pMsg2 = TRACKED_NEW plMouseEventMsg;
            pMsg2->SetXPos( fXPos );
            pMsg2->SetYPos( fYPos );
            pMsg2->SetDX(0);
            pMsg2->SetDY(0);
            pMsg2->SetButton( kRightButtonDblClk );

            if( fB2Msg != nil )
                delete fB2Msg;
            fB2Msg = pMsg2;

            pMsg->SetButton( kRightButtonDown );
            deleteMe = false;
        }
        else
        if (pBMsg->fButton & kLeftButtonDblClk)
        {
            // left button dbl clicked, send TWO messages
            plMouseEventMsg* pMsg2 = TRACKED_NEW plMouseEventMsg;
            pMsg2->SetXPos( fXPos );
            pMsg2->SetYPos( fYPos );
            pMsg2->SetDX(0);
            pMsg2->SetDY(0);
            pMsg2->SetButton( kLeftButtonDblClk );

            if( fB2Msg != nil )
                delete fB2Msg;
            fB2Msg = pMsg2;

            pMsg->SetButton( kLeftButtonDown );
            deleteMe = false;
        }

        if( deleteMe )
        {
            // mouse button state not changed
            delete pMsg;
            return true;
        }

        // we are going to save up to two button mouse events per button (left and right)
        // that will be dispatched on the next eval

        // which button is this for?
        if ( pMsg->GetButton() == kLeftButtonDown || pMsg->GetButton() == kLeftButtonUp )
        {
            // see if the queue is just empty
            if ( fLeftBMsg[0] == nil)
            {
                // nothing to think about... goes in first slot
                fLeftBMsg[0] = pMsg;
            }
            else if (fLeftBMsg[1] == nil)
            {
                // nothing to think about... goes in second slot
                fLeftBMsg[1] = pMsg;
            }
            else
            {
                // else queue if full... need to make some decisions
                plMouseEventMsg* lastMsg = plMouseEventMsg::ConvertNoRef(pMsg);
                // ...if this is an up event and [1] is a down then we need to remove both
                //    ...because we can't lose the up event and the down will have no match
                if ( pMsg->GetButton() == kLeftButtonUp && lastMsg && lastMsg->GetButton() == kLeftButtonDown)
                {
                    delete pMsg;
                    delete fLeftBMsg[1];
                    fLeftBMsg[1] = nil;
                }
                // ... otherwise ignore this event
                else
                {
                    delete pMsg;
                }
            }
        }
        else if ( pMsg->GetButton() == kRightButtonDown || pMsg->GetButton() == kRightButtonUp )
        {
            // see if the queue is just empty
            if ( fRightBMsg[0] == nil)
            {
                // nothing to think about... goes in first slot
                fRightBMsg[0] = pMsg;
            }
            else if (fRightBMsg[1] == nil)
            {
                // nothing to think about... goes in second slot
                fRightBMsg[1] = pMsg;
            }
            else
            {
                // else queue if full... need to make some decisions
                plMouseEventMsg* lastMsg = plMouseEventMsg::ConvertNoRef(pMsg);
                // ...if this is an up event and [1] is a down then we need to remove both
                //    ...because we can't lose the up event and the down will have no match
                if ( pMsg->GetButton() == kRightButtonUp && lastMsg && lastMsg->GetButton() == kRightButtonDown)
                {
                    delete pMsg;
                    delete fRightBMsg[1];
                    fRightBMsg[1] = nil;
                }
                // ... otherwise ignore this event
                else
                {
                    delete pMsg;
                }
            }
        }
        else if ( pMsg->GetButton() == kMiddleButtonDown || pMsg->GetButton() == kMiddleButtonUp )
        {
            // see if the queue is just empty
            if ( fMiddleBMsg[0] == nil)
            {
                // nothing to think about... goes in first slot
                fMiddleBMsg[0] = pMsg;
            }
            else if (fMiddleBMsg[1] == nil)
            {
                // nothing to think about... goes in second slot
                fMiddleBMsg[1] = pMsg;
            }
            else
            {
                // else queue if full... need to make some decisions
                plMouseEventMsg* lastMsg = plMouseEventMsg::ConvertNoRef(pMsg);
                // ...if this is an up event and [1] is a down then we need to remove both
                //    ...because we can't lose the up event and the down will have no match
                if ( pMsg->GetButton() == kMiddleButtonUp && lastMsg && lastMsg->GetButton() == kMiddleButtonDown)
                {
                    delete pMsg;
                    delete fMiddleBMsg[1];
                    fMiddleBMsg[1] = nil;
                }
                // ... otherwise ignore this event
                else
                {
                    delete pMsg;
                }
            }
        }
        // we are going to dispatch the mouse button events right away
        // and not wait for the next eval, because we shouldn't miss one of these
        
        return true;
    }
    return false;
}



void plMouseDevice::HandleWindowActivate(bool bActive, HWND hWnd)
{
    if ( bActive )
    {
        RECT rect;
        ::GetClientRect(hWnd,&rect);

//      rect.right /= plInputManager::GetInstance()->GetMouseScale();
//      rect.bottom /= plInputManager::GetInstance()->GetMouseScale();

        ::MapWindowPoints( hWnd, NULL, (POINT *)&rect, 2 );
        ::ClipCursor(&rect);
        ::ShowCursor( FALSE );
        SetCapture(hWnd);

    }
    else
    {
        ReleaseCapture();
        ::ClipCursor(nil);
        ::ShowCursor( TRUE );
    }   
}
