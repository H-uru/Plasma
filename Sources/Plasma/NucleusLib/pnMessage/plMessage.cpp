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
#define PLMESSAGE_PRIVATE
#include "plMessage.h"
#include "hsStream.h"
#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plgDispatch.h"
#include "hsBitVector.h"

#include <algorithm>
#include <iterator>
#include <limits>

plMessage::plMessage()
:   fBCastFlags(kLocalPropagate),
    fTimeStamp(),
    fNetRcvrPlayerIDs(),
    dispatchBreak()
{
}

plMessage::plMessage(const plKey &s, 
            const plKey &r, 
            const double* t)
:   fSender(s),
    fBCastFlags(kLocalPropagate),
    fNetRcvrPlayerIDs(),
    dispatchBreak()
{
    if (r)
        fReceivers.emplace_back(r);
    fTimeStamp = t ? *t : hsTimer::GetSysSeconds();
}

plMessage::~plMessage()
{
    delete fNetRcvrPlayerIDs;
}

size_t          plMessage::GetNumReceivers() const { return fReceivers.size(); }
const plKey&    plMessage::GetReceiver(size_t i) const { return fReceivers[i]; }
plMessage&      plMessage::RemoveReceiver(size_t i) { fReceivers.erase(fReceivers.begin() + i); return *this; }

plMessage&      plMessage::ClearReceivers() { fReceivers.clear(); return *this; }
plMessage&      plMessage::AddReceiver(plKey r) { fReceivers.emplace_back(std::move(r)); return *this; }

plMessage& plMessage::AddReceivers(const std::vector<plKey>& rList)
{
    fReceivers.reserve(fReceivers.size() + rList.size());
    fReceivers.insert(fReceivers.end(), rList.begin(), rList.end());

    return *this;
}

bool plMessage::Send(plKey r, bool async)
{
    if (r)
        AddReceiver(std::move(r));
    return plgDispatch::MsgSend(this,async);
}

bool plMessage::SendAndKeep(plKey r, bool async)
{
    Ref();
    return Send(std::move(r), async);
}

void plMessage::IMsgRead(hsStream* s, hsResMgr* mgr)
{
    plCreatable::Read(s, mgr);

    fSender = mgr->ReadKey(s);
    uint32_t n = s->ReadLE32();
    fReceivers.resize(n);
    for (size_t i = 0; i < fReceivers.size(); i++)
        fReceivers[i] = mgr->ReadKey(s);

    s->ReadLEDouble(&fTimeStamp);
    s->ReadLE32(&fBCastFlags);
}

void plMessage::IMsgWrite(hsStream* s, hsResMgr* mgr)
{
    plCreatable::Write(s, mgr);
    
    mgr->WriteKey(s,fSender);
    s->WriteLE32((uint32_t)fReceivers.size());
    for (const plKey& receiver : fReceivers)
        mgr->WriteKey(s, receiver);

    s->WriteLEDouble(fTimeStamp);
    s->WriteLE32(fBCastFlags);
}

enum MsgFlags
{
    kMsgSender,
    kMsgReceivers,
    kMsgTimeStamp,
    kMsgBCastFlags,
};

void plMessage::IMsgReadVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kMsgSender))
        fSender = mgr->ReadKey(s);

    if (contentFlags.IsBitSet(kMsgReceivers))
    {
        uint32_t n = s->ReadLE32();
        fReceivers.resize(n);
        for (size_t i = 0; i < fReceivers.size(); i++)
            fReceivers[i] = mgr->ReadKey(s);
    }

    if (contentFlags.IsBitSet(kMsgTimeStamp))
        s->ReadLEDouble(&fTimeStamp);

    if (contentFlags.IsBitSet(kMsgBCastFlags))
        fBCastFlags = s->ReadLE32();
}

void plMessage::IMsgWriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kMsgSender);
    contentFlags.SetBit(kMsgReceivers);
    contentFlags.SetBit(kMsgTimeStamp);
    contentFlags.SetBit(kMsgBCastFlags);
    contentFlags.Write(s);

    // kMsgSender
    mgr->WriteKey(s,fSender);

    // kMsgReceivers
    s->WriteLE32((uint32_t)fReceivers.size());
    for (const plKey& receiver : fReceivers)
        mgr->WriteKey(s, receiver);

    // kMsgTimeStamp
    s->WriteLEDouble(fTimeStamp);

    // kMsgBCastFlags
    s->WriteLE32(fBCastFlags);
}

void plMessage::AddNetReceiver( uint32_t plrID )
{
    if ( !fNetRcvrPlayerIDs )
        fNetRcvrPlayerIDs = new std::vector<uint32_t>;
    fNetRcvrPlayerIDs->push_back( plrID );
}

void plMessage::AddNetReceivers( const std::vector<uint32_t> & plrIDs )
{
    if ( !fNetRcvrPlayerIDs )
        fNetRcvrPlayerIDs = new std::vector<uint32_t>;
    std::copy( plrIDs.begin(), plrIDs.end(), std::back_inserter( *fNetRcvrPlayerIDs ) );
}

/////////////////////////////////////////////////////////////////

// STATIC
int plMsgStringHelper::Poke(const std::string& stringref, hsStream* stream, const uint32_t peekOptions)
{
    hsAssert(
        stringref.length() < std::numeric_limits<plMessage::plStrLen>::max(),
        "buf too big for plMsgStringHelper"
    );
    return Poke(
        stringref.c_str(), (plMessage::plStrLen)stringref.length(),
        stream, peekOptions
    );
}

int plMsgStringHelper::Poke(const char* buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    hsAssert(bufsz < std::numeric_limits<plMessage::plStrLen>::max(), "buf too big for plMsgStringHelper" );
    plMessage::plStrLen strlen = (plMessage::plStrLen)bufsz;
    stream->WriteLE16(strlen);
    if (strlen)
        stream->Write(strlen,buf);
    return stream->GetPosition();
}

int plMsgStringHelper::Poke(const ST::string& stringref, hsStream* stream, const uint32_t peekOptions)
{
    ST::char_buffer buf = stringref.to_latin_1();
    hsAssert(
        buf.size() < std::numeric_limits<plMessage::plStrLen>::max(),
        "buf too big for plMsgStringHelper"
    );
    return Poke(
        buf.data(), (plMessage::plStrLen)buf.size(),
        stream, peekOptions
    );
}

// STATIC
int plMsgStringHelper::Peek(std::string& stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen = stream->ReadLE16();
    hsAssert(strlen <= stream->GetSizeLeft(), "plMsgStringHelper::Peek: overflow peeking string.");
    if (strlen == 0 || strlen > stream->GetSizeLeft()) {
        stringref.clear();
        return stream->GetPosition();
    }

    stringref.erase();
    stringref.resize(strlen);
    if (strlen)
        stream->Read(strlen, stringref.data());
    return stream->GetPosition();
}

int plMsgStringHelper::Peek(ST::string & stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen bufsz = stream->ReadLE16();
    hsAssert(bufsz <= stream->GetSizeLeft(), "plMsgStringHelper::Peek: overflow peeking string.");
    if (bufsz == 0 || bufsz > stream->GetSizeLeft()) {
        stringref.clear();
        return stream->GetPosition();
    }

    ST::char_buffer buf;
    buf.allocate(bufsz);
    stream->Read(bufsz, buf.data());
    stringref = ST::string::from_latin_1(buf);
    return stream->GetPosition();
}


/////////////////////////////////////////////////////////////////

// STATIC
int plMsgCArrayHelper::Poke(const void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    stream->Write(bufsz,buf);
    return stream->GetPosition();
}

// STATIC
int plMsgCArrayHelper::Peek(void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    stream->Read(bufsz, buf);
    return stream->GetPosition();
}

