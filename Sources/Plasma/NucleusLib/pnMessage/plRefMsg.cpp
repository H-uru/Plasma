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

#include "plRefMsg.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plIntRefMsg.h"
#include "plObjRefMsg.h"

#include "pnKeyedObject/hsKeyedObject.h"

plRefMsg::plRefMsg()
: fRef(), fOldRef(), fContext()
{
}

plRefMsg::plRefMsg(const plKey &r, uint8_t c)
: plMessage(nullptr, r, nullptr), fRef(), fOldRef(), fContext(c)
{
    if( !fContext )
        fContext = kOnCreate;
}

plRefMsg::~plRefMsg()
{
    // Un ref fref and foldref.
}

plRefMsg& plRefMsg::SetRef(hsKeyedObject* ref)
{
    fRef = ref;  // ref count here! paulg
    return *this;
}

plRefMsg& plRefMsg::SetOldRef(hsKeyedObject* oldRef)
{
    fOldRef = oldRef;
    // Ref here! 
    return *this;
}

void plRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    stream->ReadByte(&fContext);

    plKey key;
    key = mgr->ReadKey(stream);
    fRef = (key ? key->GetObjectPtr() : nullptr);
    key = mgr->ReadKey(stream);
    fOldRef = (key ? key->GetObjectPtr() : nullptr);
}

void plRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    stream->WriteByte(fContext);

    mgr->WriteKey(stream, (fRef ? fRef->GetKey() : nullptr));
    mgr->WriteKey(stream, (fOldRef ? fOldRef->GetKey() : nullptr));
}


void plGenRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Read(stream, mgr);
    stream->ReadByte(&fType);
    fWhich = stream->ReadLE32();
}

void plGenRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Write(stream, mgr);
    stream->WriteByte(fType);
    stream->WriteLE32(fWhich);
}

void plIntRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Read(stream, mgr);
    stream->ReadByte(&fType);
    stream->ReadLE16(&fWhich);
    stream->ReadByte(&fIdx);
}

void plIntRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Write(stream, mgr);
    stream->WriteByte(fType);
    stream->WriteLE16(fWhich);
    stream->WriteByte(fIdx);
}

void plObjRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Read(stream, mgr);
    stream->ReadByte(&fType);
    stream->ReadByte(&fWhich);
}

void plObjRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plRefMsg::Write(stream, mgr);
    stream->WriteByte(fType);
    stream->WriteByte(fWhich);
}
