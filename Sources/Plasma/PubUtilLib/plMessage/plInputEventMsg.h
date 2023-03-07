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

#ifndef plInputEventMsg_inc
#define plInputEventMsg_inc

#include "pnMessage/plMessage.h"
#include "pnInputCore/plControlDefinition.h"
#include "hsGeometry3.h"

#include <utility>

class plKeyEventMsg;
class plMouseEventMsg;

class plInputEventMsg : public plMessage
{
public:
    enum
    {
        kConfigure = 0,
    };
    plInputEventMsg();
    plInputEventMsg(const plKey &s,
                      const plKey &r,
                      const double* t);

    ~plInputEventMsg();

    CLASSNAME_REGISTER( plInputEventMsg );
    GETINTERFACE_ANY( plInputEventMsg, plMessage );

    int fEvent;
    
    // IO 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};


class plControlEventMsg : public plInputEventMsg
{
private:
    ST::string          fCmd;
protected:
    
    ControlEventCode    fControlCode;
    bool                fControlActivated;
    hsPoint3            fTurnToPt;
    float              fControlPct;
public:

    plControlEventMsg();
    plControlEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t);

    CLASSNAME_REGISTER( plControlEventMsg );
    GETINTERFACE_ANY( plControlEventMsg, plInputEventMsg );

    void SetCmdString(ST::string cs)        { fCmd = std::move(cs); }
    void SetControlCode(ControlEventCode c) { fControlCode = c; }
    void SetControlActivated(bool b)        { fControlActivated = b; }
    void SetTurnToPt(const hsPoint3& pt)    { fTurnToPt = pt; }
    void SetControlPct(float p)             { fControlPct = p; }

    ControlEventCode    GetControlCode()    const { return fControlCode; }
    bool                ControlActivated()  const { return fControlActivated; }
    hsPoint3            GetTurnToPt()       const { return fTurnToPt; }
    float               GetPct()            const { return fControlPct; }
    ST::string          GetCmdString()      const { return fCmd; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};


class plKeyEventMsg : public plInputEventMsg
{
protected:
    wchar_t         fKeyChar;
    plKeyDef        fKeyCode;
    bool            fKeyDown;
    bool            fCapsLockKeyDown;
    bool            fShiftKeyDown;
    bool            fCtrlKeyDown;
    bool            fRepeat;

public:

    
    plKeyEventMsg();
    plKeyEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t);
    ~plKeyEventMsg();

    CLASSNAME_REGISTER( plKeyEventMsg );
    GETINTERFACE_ANY( plKeyEventMsg, plInputEventMsg );

    void SetKeyChar(wchar_t key) { fKeyChar = key; }
    void SetKeyCode(plKeyDef w) { fKeyCode = w; }
    void SetKeyDown(bool b)   { fKeyDown = b; }
    void SetShiftKeyDown(bool b)  { fShiftKeyDown = b; }
    void SetCtrlKeyDown(bool b)   { fCtrlKeyDown = b; }
    void SetCapsLockKeyDown(bool b)   { fCapsLockKeyDown = b; }
    void SetRepeat(bool b)    { fRepeat = b; }
    
    wchar_t     GetKeyChar() const         { return fKeyChar; }
    plKeyDef    GetKeyCode() const         { return fKeyCode; }
    bool        GetKeyDown() const         { return fKeyDown; }
    bool        GetShiftKeyDown() const    { return fShiftKeyDown; }
    bool        GetCtrlKeyDown() const     { return fCtrlKeyDown; }
    bool        GetCapsLockKeyDown() const { return fCapsLockKeyDown; }
    bool        GetRepeat() const          { return fRepeat; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};


class plDebugKeyEventMsg : public plInputEventMsg
{
protected:
    ControlEventCode    fKeyCode;
    bool                fKeyDown;
    bool                fCapsLockKeyDown;
    bool                fShiftKeyDown;
    bool                fCtrlKeyDown;

public:

    
    plDebugKeyEventMsg();
    plDebugKeyEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t);
    ~plDebugKeyEventMsg();

    CLASSNAME_REGISTER( plDebugKeyEventMsg );
    GETINTERFACE_ANY( plDebugKeyEventMsg, plInputEventMsg );

    void SetKeyCode(ControlEventCode w) { fKeyCode = w; }
    void SetKeyDown(bool b)             { fKeyDown = b; }
    void SetShiftKeyDown(bool b)        { fShiftKeyDown = b; }
    void SetCtrlKeyDown(bool b)         { fCtrlKeyDown = b; }
    void SetCapsLockKeyDown(bool b)     { fCapsLockKeyDown = b; }

    ControlEventCode GetKeyCode() const { return fKeyCode; }
    bool  GetKeyDown() const            { return fKeyDown; }
    bool  GetShiftKeyDown() const       { return fShiftKeyDown; }
    bool  GetCtrlKeyDown() const        { return fCtrlKeyDown; }
    bool  GetCapsLockKeyDown() const    { return fCapsLockKeyDown; }


    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plIMouseXEventMsg : public plInputEventMsg
{
public:
    float   fX;
    int     fWx;
    
    plIMouseXEventMsg() : 
    fX(0),fWx(0) {}
    plIMouseXEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t) : 
    fX(0),fWx(0) {}
    ~plIMouseXEventMsg(){}

    CLASSNAME_REGISTER( plIMouseXEventMsg );
    GETINTERFACE_ANY( plIMouseXEventMsg, plInputEventMsg );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plIMouseYEventMsg : public plInputEventMsg
{
public:
    float   fY;
    int     fWy;

    plIMouseYEventMsg() : 
    fY(0),fWy(0) {}
    plIMouseYEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t) : 
    fY(0),fWy(0) {}
    ~plIMouseYEventMsg(){}

    CLASSNAME_REGISTER( plIMouseYEventMsg );
    GETINTERFACE_ANY( plIMouseYEventMsg, plInputEventMsg );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};
class plIMouseBEventMsg : public plInputEventMsg
{
public:
    short   fButton;

    plIMouseBEventMsg() : 
    fButton(0) {}
    plIMouseBEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t) : 
    fButton(0) {}
    ~plIMouseBEventMsg(){}

    CLASSNAME_REGISTER( plIMouseBEventMsg );
    GETINTERFACE_ANY( plIMouseBEventMsg, plInputEventMsg );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plMouseEventMsg : public plInputEventMsg
{
    
protected:

    float fXPos;
    float fYPos;
    float fDX;
    float fDY;
    float fWheelDelta;

    short   fButton;


public:
    plMouseEventMsg();
    plMouseEventMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t);
    ~plMouseEventMsg();

    CLASSNAME_REGISTER( plMouseEventMsg );
    GETINTERFACE_ANY( plMouseEventMsg, plInputEventMsg );
    
    void SetXPos(float Xpos) { fXPos = Xpos; };
    void SetYPos(float Ypos) { fYPos = Ypos; };
    void SetDX(float dX)       { fDX = dX; }
    void SetDY(float dY)       { fDY = dY; }
    void SetButton(short _button) { fButton = _button; }
    void SetWheelDelta(float d) { fWheelDelta = d; }
    
    float GetXPos() { return fXPos; }
    float GetYPos() { return fYPos; }
    float GetDX()   { return fDX; }
    float GetDY()   { return fDY; }
    float GetWheelDelta() { return fWheelDelta; }
    short GetButton() { return fButton; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plAvatarInputStateMsg : public plMessage
{
public:
    uint16_t fState;

    plAvatarInputStateMsg() : plMessage(), fState(0) {}
    ~plAvatarInputStateMsg() {}

    CLASSNAME_REGISTER( plAvatarInputStateMsg );
    GETINTERFACE_ANY( plAvatarInputStateMsg, plMessage );

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;

    // Mapping of bits to the control events we care about
    static const ControlEventCode fCodeMap[];
    static const uint8_t fMapSize;

    static bool IsCodeInMap(ControlEventCode code);
};

#endif // plInputEventMsg_inc
