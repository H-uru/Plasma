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

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plInputEventMsg.h"

plInputEventMsg::plInputEventMsg() :
fEvent(-1)
{
    SetBCastFlag(plMessage::kBCastByType);
}

plInputEventMsg::plInputEventMsg(const plKey &s, 
            const plKey &r, 
            const double* t) :
fEvent(-1)
{
    SetBCastFlag(plMessage::kBCastByType);
}

plInputEventMsg::~plInputEventMsg()
{
}

void plInputEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    
    stream->ReadLE32(&fEvent);
}

void plInputEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    
    stream->WriteLE32(fEvent);
}

enum InputEventMsgFlags
{
    kInputEventMsgEvent,
};

void plInputEventMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kInputEventMsgEvent))
        s->ReadLE32(&fEvent);
}

void plInputEventMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kInputEventMsgEvent);
    contentFlags.Write(s);

    // kInputEventMsgEvent
    s->WriteLE32(fEvent);
}

plControlEventMsg::plControlEventMsg() : 
    fCmd()
{
    fControlPct = 1.0f;
    SetBCastFlag(plMessage::kPropagateToModifiers);
    SetBCastFlag(plMessage::kBCastByType, false);
}

plControlEventMsg::plControlEventMsg(const plKey &s, 
            const plKey &r, 
            const double* t) :
    fCmd()
{
    fControlPct = 1.0f;
    SetBCastFlag(plMessage::kBCastByType, false);
    SetBCastFlag(plMessage::kPropagateToModifiers);
}

void plControlEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    fControlCode = (ControlEventCode)stream->ReadLE32();
    fControlActivated = stream->ReadBOOL();
    stream->ReadLEFloat(&fControlPct);
    fTurnToPt.Read(stream);

    // read cmd/string
    plMsgCStringHelper::Peek(fCmd, stream);
}

void plControlEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLE32((int32_t)fControlCode);
    stream->WriteBOOL(fControlActivated);
    stream->WriteLEFloat(fControlPct);
    fTurnToPt.Write(stream);
    
    // write cmd/string
    plMsgCStringHelper::Poke(fCmd, stream);
}

enum ControlEventMsgFlags
{
    kControlEventMsgCode,
    kControlEventMsgActivated,
    kControlEventMsgPct,
    kControlEventMsgTurnToPt,
    kControlEventMsgCmd,
};

void plControlEventMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plInputEventMsg::ReadVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kControlEventMsgCode))
        fControlCode = (ControlEventCode)s->ReadLE32();

    if (contentFlags.IsBitSet(kControlEventMsgActivated))
        fControlActivated = s->ReadBOOL();

    if (contentFlags.IsBitSet(kControlEventMsgPct))
        s->ReadLEFloat(&fControlPct);

    if (contentFlags.IsBitSet(kControlEventMsgTurnToPt))
        fTurnToPt.Read(s);

    // read cmd/string
    if (contentFlags.IsBitSet(kControlEventMsgCmd))
        plMsgCStringHelper::Peek(fCmd, s);
}

void plControlEventMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plInputEventMsg::WriteVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kControlEventMsgCode);
    contentFlags.SetBit(kControlEventMsgActivated);
    contentFlags.SetBit(kControlEventMsgPct);
    contentFlags.SetBit(kControlEventMsgTurnToPt);
    contentFlags.SetBit(kControlEventMsgCmd);
    contentFlags.Write(s);

    // kControlEventMsgCode,    
    s->WriteLE32((int32_t)fControlCode);

    // kControlEventMsgActivated,
    s->WriteBOOL(fControlActivated);

    // kControlEventMsgPct,
    s->WriteLEFloat(fControlPct);

    // kControlEventMsgTurnToPt,
    fTurnToPt.Write(s);

    // kControlEventMsgCmd,
    plMsgCStringHelper::Poke(fCmd, s);
}

plKeyEventMsg::plKeyEventMsg()
{
}

plKeyEventMsg::plKeyEventMsg(const plKey &s, 
                             const plKey &r, 
                             const double* t)
{
}

plKeyEventMsg::~plKeyEventMsg()
{
}

void plKeyEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    fKeyCode = (plKeyDef)stream->ReadLE32();
    fKeyDown = stream->ReadBOOL();
    fCapsLockKeyDown = stream->ReadBOOL();
    fShiftKeyDown = stream->ReadBOOL();
    fCtrlKeyDown = stream->ReadBOOL();
    fRepeat = stream->ReadBOOL();
}

void plKeyEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLE32((int32_t)fKeyCode);
    stream->WriteBOOL(fKeyDown);
    stream->WriteBOOL(fCapsLockKeyDown);
    stream->WriteBOOL(fShiftKeyDown);
    stream->WriteBOOL(fCtrlKeyDown);
    stream->WriteBOOL(fRepeat);
}

plDebugKeyEventMsg::plDebugKeyEventMsg()
{
}

plDebugKeyEventMsg::plDebugKeyEventMsg(const plKey &s, 
                                       const plKey &r, 
                                       const double* t)
{
}

plDebugKeyEventMsg::~plDebugKeyEventMsg()
{
}

void plDebugKeyEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    fKeyCode = (ControlEventCode)stream->ReadLE32();
    fKeyDown = stream->ReadBOOL();
    fCapsLockKeyDown = stream->ReadBOOL();
    fShiftKeyDown = stream->ReadBOOL();
    fCtrlKeyDown = stream->ReadBOOL();
}

void plDebugKeyEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLE32((int32_t)fKeyCode);
    stream->WriteBOOL(fKeyDown);
    stream->WriteBOOL(fCapsLockKeyDown);
    stream->WriteBOOL(fShiftKeyDown);
    stream->WriteBOOL(fCtrlKeyDown);
}

void plIMouseXEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    stream->ReadLEFloat(&fX);
    stream->ReadLE32(&fWx);
}

void plIMouseXEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLEFloat(fX);
    stream->WriteLE32(fWx);
}

void plIMouseYEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    stream->ReadLEFloat(&fY);
    stream->ReadLE32(&fWy);
}

void plIMouseYEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLEFloat(fY);
    stream->WriteLE32(fWy);
}

void plIMouseBEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    stream->ReadLE16(&fButton);
}

void plIMouseBEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLE16(fButton);
}

plMouseEventMsg::plMouseEventMsg() : fXPos(0.0f),fYPos(0.0f),fDX(0.0f),fDY(0.0f),fButton(0)
{
}

plMouseEventMsg::plMouseEventMsg(const plKey &s, 
                                 const plKey &r, 
                                 const double* t)
{
}

plMouseEventMsg::~plMouseEventMsg()
{
}

void plMouseEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    stream->ReadLEFloat(&fXPos);
    stream->ReadLEFloat(&fYPos);
    stream->ReadLEFloat(&fDX);
    stream->ReadLEFloat(&fDY);
    stream->ReadLE16(&fButton);
    stream->ReadLEFloat(&fWheelDelta);
}

void plMouseEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLEFloat(fXPos);
    stream->WriteLEFloat(fYPos);
    stream->WriteLEFloat(fDX);
    stream->WriteLEFloat(fDY);
    stream->WriteLE16(fButton);
    stream->WriteLEFloat(fWheelDelta);
}

/////////////////////////////////////////////////////////////////////////////

// Mapping of bits to the control events we care about
const ControlEventCode plAvatarInputStateMsg::fCodeMap[] =
{
    B_CONTROL_MOVE_FORWARD,
    B_CONTROL_MOVE_BACKWARD,
    B_CONTROL_ROTATE_LEFT,
    B_CONTROL_ROTATE_RIGHT,
    B_CONTROL_STRAFE_LEFT,
    B_CONTROL_STRAFE_RIGHT,
    B_CONTROL_ALWAYS_RUN,
    B_CONTROL_JUMP,
    B_CONTROL_CONSUMABLE_JUMP,
    B_CONTROL_MODIFIER_FAST,
    B_CONTROL_MODIFIER_STRAFE,
    B_CONTROL_LADDER_INVERTED,
};
const uint8_t plAvatarInputStateMsg::fMapSize = 12;

void plAvatarInputStateMsg::Read(hsStream *s, hsResMgr *mgr)
{
    plMessage::IMsgRead(s, mgr);
    fState = s->ReadLE16();
}

void plAvatarInputStateMsg::Write(hsStream *s, hsResMgr *mgr)
{
    plMessage::IMsgWrite(s, mgr);
    s->WriteLE16(fState);
}

enum AvatarInputStateMsgFlags
{
    kAvatarInputStateMsgState,
};

void plAvatarInputStateMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kAvatarInputStateMsgState))
        fState = s->ReadLE16();
}

void plAvatarInputStateMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kAvatarInputStateMsgState);
    contentFlags.Write(s);

    s->WriteLE16(fState);
}

bool plAvatarInputStateMsg::IsCodeInMap(ControlEventCode code)
{
    int i;
    for (i = 0; i < fMapSize; i++)
    {
        if (fCodeMap[i] == code)
            return true;
    }

    return false;
}