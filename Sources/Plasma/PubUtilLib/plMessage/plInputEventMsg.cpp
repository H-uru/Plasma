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

#include "hsTypes.h"
#include "plInputEventMsg.h"
#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "hsBitVector.h"

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
    
    stream->ReadLE(&fEvent);
}

void plInputEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    
    stream->WriteLE(fEvent);
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
        s->ReadLE(&fEvent);
}

void plInputEventMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kInputEventMsgEvent);
    contentFlags.Write(s);

    // kInputEventMsgEvent
    s->WriteLE(fEvent);
}

plControlEventMsg::plControlEventMsg() : 
    fCmd(nil)
{
    fTurnToPt.Set(0,0,0);
    fControlPct = 1.0f;
    SetBCastFlag(plMessage::kPropagateToModifiers);
    SetBCastFlag(plMessage::kBCastByType, false);
}

plControlEventMsg::plControlEventMsg(const plKey &s, 
            const plKey &r, 
            const double* t) :
    fCmd(nil)
{
    fTurnToPt.Set(0,0,0);
    fControlPct = 1.0f;
    SetBCastFlag(plMessage::kBCastByType, false);
    SetBCastFlag(plMessage::kPropagateToModifiers);
}

plControlEventMsg::~plControlEventMsg()
{   
    delete [] fCmd;
}

void plControlEventMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Read(stream, mgr);
    stream->ReadLE((Int32*)&fControlCode);
    stream->ReadLE(&fControlActivated);
    stream->ReadLE(&fControlPct);
    fTurnToPt.Read(stream);

    // read cmd/string
    plMsgCStringHelper::Peek(fCmd, stream);
}

void plControlEventMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plInputEventMsg::Write(stream, mgr);
    stream->WriteLE((Int32)fControlCode);
    stream->WriteLE(fControlActivated);
    stream->WriteLE(fControlPct);
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
        s->ReadLE((Int32*)&fControlCode);

    if (contentFlags.IsBitSet(kControlEventMsgActivated))
        s->ReadLE(&fControlActivated);

    if (contentFlags.IsBitSet(kControlEventMsgPct))
        s->ReadLE(&fControlPct);

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
    s->WriteLE((Int32)fControlCode);

    // kControlEventMsgActivated,
    s->WriteLE(fControlActivated);

    // kControlEventMsgPct,
    s->WriteLE(fControlPct);

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
const UInt8 plAvatarInputStateMsg::fMapSize = 12;

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

hsBool plAvatarInputStateMsg::IsCodeInMap(ControlEventCode code)
{
    int i;
    for (i = 0; i < fMapSize; i++)
    {
        if (fCodeMap[i] == code)
            return true;
    }

    return false;
}