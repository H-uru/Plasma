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
// plInputDevice.h

#ifndef PL_INPUT_DEVICE_H
#define PL_INPUT_DEVICE_H

#include "HeadSpin.h"

//#include "pnInputCore/plControlDefinition.h"
#include "pnInputCore/plKeyDef.h"
#include "hsBitVector.h"

class plMessage;
struct plMouseInfo;
class plPipeline;

namespace ST { class string; }

class plInputDevice 
{
public:
    enum Flags
    {
        kDisabled    = 0x1
    };
protected:
    uint32_t fFlags;
public:
    
    plInputDevice()
        : fFlags()
    { }
    virtual ~plInputDevice() { }

    virtual const char* GetInputName() = 0;

    uint32_t GetFlags() { return fFlags; }
    void SetFlags(uint32_t f) { fFlags = f; }
    virtual void HandleKeyEvent(plKeyDef key, bool bKeyDown, bool bKeyRepeat, wchar_t c = 0) { }
    virtual void HandleWindowActivate(bool bActive, hsWindowHndl hWnd) { }
    virtual bool MsgReceive(plMessage* msg) {return false;}
    virtual void Shutdown() { }


};

class plKeyEventMsg;

class plKeyboardDevice : public plInputDevice
{
    bool    fAltKeyDown;
    bool    fShiftKeyDown;
    bool    fCtrlKeyDown;
    bool    fCapsLockKeyDown;
    int     fControlMode;
    bool    fCapsLockLock;

    static bool     fKeyboardState[256]; // virtual key code is the index, bool is whether it is down or not
    static bool     fIgnoreCapsLock; // set if we want it to ignore this key when translating characters (i.e. for chatting)

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

    const char* GetInputName() override { return "keyboard"; }
    void HandleKeyEvent(plKeyDef key, bool bKeyDown, bool bKeyRepeat, wchar_t c = 0) override;
    void HandleWindowActivate(bool bActive, hsWindowHndl hWnd) override;
    virtual bool IsCapsLockKeyOn();
    void Shutdown() override;

    static bool     IgnoreCapsLock() { return fIgnoreCapsLock; }
    static void     IgnoreCapsLock(bool ignore) { fIgnoreCapsLock = ignore; }
    
    static plKeyboardDevice* GetInstance() { return fInstance; }
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

    const char* GetInputName() override { return "mouse"; }

    bool    HasControlFlag(int f) const { return fControlFlags.IsBitSet(f); }
    void    SetControlFlag(int f) 
    { 
        fControlFlags.SetBit(f); 
    }
    void    ClearControlFlag(int which) { fControlFlags.ClearBit( which ); }
    void    SetCursorX(float x);
    void    SetCursorY(float y);
    float GetCursorX() { return fXPos; }
    float GetCursorY() { return fYPos; }
    uint32_t  GetButtonState() { return fButtonState; }
    float GetCursorOpacity() { return fOpacity; }
    void SetDisplayResolution(float Width, float Height);
    void SetScale(float Scale);
    
    bool MsgReceive(plMessage* msg) override;
    
    static plMouseDevice* Instance() { return plMouseDevice::fInstance; }
    
    static void SetMsgAlways(bool b) { plMouseDevice::bMsgAlways = b; }
    static void ShowCursor(bool override = false);
    static void NewCursor(const char* cursor);
    static void HideCursor(bool override = false);
    static bool GetHideCursor() { return plMouseDevice::bCursorHidden; }
    static void SetCursorOpacity( float opacity = 1.f );
    static bool GetInverted() { return plMouseDevice::bInverted; }
    static void SetInverted(bool inverted) { plMouseDevice::bInverted = inverted; }
    static void AddNameToCursor(const ST::string& name);
    static void AddIDNumToCursor(uint32_t idNum);
    static void AddCCRToCursor();
    
protected:
    plInputEventMsg*    fXMsg;
    plInputEventMsg*    fYMsg;
    plInputEventMsg*    fB2Msg;

    // mouse button event queues (only hold 2)
    plInputEventMsg*    fLeftBMsg[2];
    plInputEventMsg*    fRightBMsg[2];
    plInputEventMsg*    fMiddleBMsg[2];

    float fXPos;
    float fYPos;
    int      fWXPos; // the windows coordinates of the cursor
    int      fWYPos;
    uint32_t   fButtonState;
    float fOpacity;
    hsBitVector     fControlFlags;
    
    
    plPlate *fCursor;
    const char*    fCursorID;

    static plMouseDevice* fInstance;
    static plMouseInfo  fDefaultMouseControlMap[];
    void    CreateCursor(const char* cursor );
    void IUpdateCursorSize();
    static bool bMsgAlways;
    static bool bCursorHidden;
    static bool bCursorOverride;
    static bool bInverted;
    static float fWidth, fHeight;
    static float fScale;
};



#endif // PL_INPUT_DEVICE_H
