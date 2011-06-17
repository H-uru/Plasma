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
// plInputDevice.h

#ifndef PL_INPUT_DEVICE_H
#define PL_INPUT_DEVICE_H

#include "HeadSpin.h"
#include "hsWindows.h"
//#include "pnInputCore/plControlDefinition.h"
#include "pnInputCore/plOSMsg.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
class plMessage;
enum plKeyDef;
struct plMouseInfo;
class plPipeline;

class plInputDevice 
{
public:
    enum Flags
    {
        kDisabled    = 0x1
    };
protected:
    UInt32 fFlags;
public:
    
    plInputDevice() {;}
    virtual ~plInputDevice() {;}

    virtual const char* GetInputName() = 0;

    UInt32 GetFlags() { return fFlags; }
    void SetFlags(UInt32 f) { fFlags = f; }
    virtual void HandleKeyEvent(plOSMsg message, plKeyDef key, bool bKeyDown, hsBool bKeyRepeat) {;}
    virtual void HandleMouseEvent(plOSMsg message, plMouseState state)  {;}
    virtual void HandleWindowActivate(bool bActive, HWND hWnd) {;}
    virtual hsBool MsgReceive(plMessage* msg) {return false;}
    virtual void Shutdown() {;}


};

class plKeyEventMsg;

class plKeyboardDevice : public plInputDevice
{
    hsBool  fAltKeyDown;
    hsBool  fShiftKeyDown;
    hsBool  fCtrlKeyDown;
    hsBool  fCapsLockKeyDown;
    int     fControlMode;
    hsBool  fCapsLockLock;

    static bool     fKeyboardState[256]; // virtual key code is the index, bool is whether it is down or not
    static hsBool   fIgnoreCapsLock; // set if we want it to ignore this key when translating characters (i.e. for chatting)
    static hsBool   fKeyIsDeadKey; // the key we just got was a dead key, store the value if you're a text input object

    static plKeyboardDevice* fInstance;
    void InitKeyboardMaps();
    void InitKeyboardState();

    void ReleaseAllKeys();

public:
    enum
    {
        CONSOLE_MODE = 0,
        CONSOLE_FULL,
        STANDARD_MODE,
    };
    plKeyboardDevice();
    ~plKeyboardDevice();

    void SetControlMode(int i) { fControlMode = i; }

    const char* GetInputName() { return "keyboard"; }
    void HandleKeyEvent(plOSMsg message, plKeyDef key, bool bKeyDown, hsBool bKeyRepeat);
    virtual void HandleWindowActivate(bool bActive, HWND hWnd);
    virtual hsBool IsCapsLockKeyOn();
    virtual void Shutdown();

    static hsBool   IgnoreCapsLock() { return fIgnoreCapsLock; }
    static void     IgnoreCapsLock(hsBool ignore) { fIgnoreCapsLock = ignore; }

    static hsBool   KeyIsDeadKey() { return fKeyIsDeadKey; }
    
    static plKeyboardDevice* GetInstance() { return fInstance; }

    static wchar_t KeyEventToChar( plKeyEventMsg *msg );
};

class plPlate;

#define CURSOR_UP                   "cursor_up.png"
#define CURSOR_UPWARD               "cursor_upward.png"
#define CURSOR_DOWN                 "cursor_down.png"
#define CURSOR_RIGHT                "cursor_right.png"
#define CURSOR_LEFT                 "cursor_left.png"
#define CURSOR_OPEN                 "cursor_open.png"
#define CURSOR_GRAB                 "cursor_grab.png"
#define CURSOR_CLICKED              "cursor_clicked.png"
#define CURSOR_POISED               "cursor_poised.png"
#define CURSOR_4WAY_OPEN            "cursor_4way_open.png"
#define CURSOR_4WAY_CLOSED          "cursor_4way_closed.png"
#define CURSOR_UPDOWN_OPEN          "cursor_updown_open.png"
#define CURSOR_UPDOWN_CLOSED        "cursor_updown_closed.png"
#define CURSOR_LEFTRIGHT_OPEN       "cursor_leftright_open.png"
#define CURSOR_LEFTRIGHT_CLOSED     "cursor_leftright_closed.png"
#define CURSOR_OFFER_BOOK           "cursor_book.png"
#define CURSOR_OFFER_BOOK_HI        "cursor_book_poised.png"
#define CURSOR_OFFER_BOOK_CLICKED   "cursor_book_clicked.png"
#define CURSOR_CLICK_DISABLED       "cursor_disabled.png"
#define CURSOR_HAND                 "cursor_up.png"
#define CURSOR_ARROW                "cursor_up.png"

class plInputEventMsg;

class plMouseDevice : public plInputDevice
{
public:
    plMouseDevice();
    ~plMouseDevice();

    const char* GetInputName() { return "mouse"; }
    void HandleWindowActivate(bool bActive, HWND hWnd);

    hsBool  HasControlFlag(int f) const { return fControlFlags.IsBitSet(f); }
    void    SetControlFlag(int f) 
    { 
        fControlFlags.SetBit(f); 
    }
    void    ClearControlFlag(int which) { fControlFlags.ClearBit( which ); }
    void    SetCursorX(hsScalar x);
    void    SetCursorY(hsScalar y);
    hsScalar GetCursorX() { return fXPos; }
    hsScalar GetCursorY() { return fYPos; }
    UInt32  GetButtonState() { return fButtonState; }
    hsScalar GetCursorOpacity() { return fOpacity; }
    void SetDisplayResolution(hsScalar Width, hsScalar Height);
    
    virtual hsBool MsgReceive(plMessage* msg);
    
    static plMouseDevice* Instance() { return plMouseDevice::fInstance; }
    
    static void SetMsgAlways(bool b) { plMouseDevice::bMsgAlways = b; }
    static void ShowCursor(hsBool override = false);
    static void NewCursor(char* cursor);
    static void HideCursor(hsBool override = false);
    static bool GetHideCursor() { return plMouseDevice::bCursorHidden; }
    static void SetCursorOpacity( hsScalar opacity = 1.f );
    static bool GetInverted() { return plMouseDevice::bInverted; }
    static void SetInverted(bool inverted) { plMouseDevice::bInverted = inverted; }
    static void AddNameToCursor(const char* name);
    static void AddIDNumToCursor(UInt32 idNum);
    static void AddCCRToCursor();
    
protected:
    plInputEventMsg*    fXMsg;
    plInputEventMsg*    fYMsg;
    plInputEventMsg*    fB2Msg;

    // mouse button event queues (only hold 2)
    plInputEventMsg*    fLeftBMsg[2];
    plInputEventMsg*    fRightBMsg[2];
    plInputEventMsg*    fMiddleBMsg[2];

    hsScalar fXPos;
    hsScalar fYPos;
    int      fWXPos; // the windows coordinates of the cursor
    int      fWYPos;
    UInt32   fButtonState;
    hsScalar fOpacity;
    hsBitVector     fControlFlags;
    
    
    plPlate *fCursor;
    char*    fCursorID;

    static plMouseDevice* fInstance;
    static plMouseInfo  fDefaultMouseControlMap[];
    void    CreateCursor( char* cursor );
    void IUpdateCursorSize();
    static bool bMsgAlways;
    static bool bCursorHidden;
    static bool bCursorOverride;
    static bool bInverted;
    static hsScalar fWidth, fHeight;
};



#endif // PL_INPUT_DEVICE_H
