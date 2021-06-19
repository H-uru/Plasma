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

#include "plNetMessage.h"

#include "plCreatableIndex.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plNetCommonMessage.h"
#include "plNetMsgVersion.h"

#include "pnFactory/plFactory.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plMessage.h"
#include "pnNetCommon/plGenericVar.h"
#include "pnNetCommon/plNetSharedState.h"
#include "pnNetCommon/pnNetCommon.h"

#include "plNetCommon/plNetCommon.h"
#include "plSDL/plSDL.h"

#include <algorithm>

//
// static
//
// see plNetMsgVersion.h
const uint8_t plNetMessage::kVerMajor = PLASMA2_NETMSG_MAJOR_VERSION;
const uint8_t plNetMessage::kVerMinor = PLASMA2_NETMSG_MINOR_VERSION;



////////////////////////////////////////////////////////
// plNetMessage
////////////////////////////////////////////////////////

void plNetMessage::Read(hsStream* s, hsResMgr* mgr)
{
    IPeekBuffer(s);
}

void plNetMessage::Write(hsStream* s, hsResMgr* mgr)
{
    IPokeBuffer(s);
}


void plNetMessage::InitReplyFieldsFrom(plNetMessage * msg)
{
    bool hasContext = msg->GetHasContext();
    SetHasContext(hasContext);
    if (hasContext)
        SetContext(msg->GetContext());
    
    bool hasTransactionID = msg->GetHasTransactionID();
    SetHasTransactionID(hasTransactionID);
    if (hasTransactionID)
        SetTransactionID(msg->GetTransactionID());
    
    bool hasPlayerID = msg->GetHasPlayerID();
    if ( hasPlayerID )
        SetPlayerID( msg->GetPlayerID() );
    
    bool hasAcctUUID = msg->GetHasAcctUUID();
    if ( hasAcctUUID )
        SetAcctUUID( msg->GetAcctUUID() );

    bool hasTimeSent = msg->GetHasTimeSent();
    if ( hasTimeSent )
        SetTimeSent( msg->GetTimeSent() );

#if 0   // I don't think the version should be copied
    if (msg->IsBitSet(kHasVersion))
        SetVersion();
#endif
}

//
// STATIC
// create and READ from lowlevel net buffer
//
plNetMessage* plNetMessage::CreateAndRead(const plNetCommonMessage* msg)
{
    // create
    plNetMessage* pHdr = Create(msg);
    if (!pHdr)
        return nullptr;
    
    // read
    pHdr->PeekBuffer(msg->GetData(), msg->GetLen(), 0, false);
    return pHdr;
}

//
// STATIC
// create from lowlevel net buffer
//
plNetMessage* plNetMessage::Create(const plNetCommonMessage* msg)
{
    if (msg)
    {
        hsReadOnlyStream readStream;
        ClassIndexType classIndex;
        readStream.Init(sizeof(classIndex), msg->GetData());
        readStream.ReadLE16(&classIndex);
        if (!plFactory::IsValidClassIndex(classIndex))
            return nullptr;
        plNetMessage* pnm = plNetMessage::ConvertNoRef(plFactory::Create(classIndex));
        if (pnm)
            pnm->SetNetCoreMsg(msg);
        else
        {
            char str[256];
            sprintf(str, "Factory create failed, class index=%d, garbage msg?", classIndex);
            hsAssert(false, str);
        }
        return pnm;
    }
    return nullptr;
}

int plNetMessage::PokeBuffer(char* bufIn, int bufLen, uint32_t peekOptions)
{
    fPeekStatus = 0;
    
    if (!bufIn)
        return 0;   
    
    if (! (peekOptions & kDontClearBuffer))
        memset(bufIn, 0, bufLen);
    
    ValidatePoke();
    hsWriteOnlyStream writeStream;
    writeStream.Init(bufLen, bufIn);
    int ret;
    if (peekOptions & kBaseClassOnly)
    {
        ret=plNetMessage::IPokeBuffer(&writeStream, peekOptions);
    }
    else
    {
        ret=IPokeBuffer(&writeStream, peekOptions);
    }
    return ret;
}

int plNetMessage::PeekBuffer(const char* bufIn, int bufLen, uint32_t peekOptions, bool forcePeek)
{
    if(!bufLen || bufLen < 1)
        return 0;

    uint32_t partialPeekOptions = (peekOptions & kPartialPeekMask);
    if (!forcePeek && (fPeekStatus & partialPeekOptions) )
        return 0;   // already peeked, fully or partially
    
    if (!bufIn)
        return 0;
    
    // set peek status based on peekOptions
    fPeekStatus = partialPeekOptions ? partialPeekOptions : kFullyPeeked;
    
    hsReadOnlyStream readStream;
    readStream.Init(bufLen, bufIn);
    int ret;
    if (peekOptions & kBaseClassOnly)
    {
        ret=plNetMessage::IPeekBuffer(&readStream, peekOptions);
        plNetMessage::ValidatePeek();
    }
    else
    {
        ret=IPeekBuffer(&readStream, peekOptions);
        ValidatePeek();
    }
    
    return ret;
}

void plNetMessage::IWriteClassIndex(hsStream* stream)
{
    ClassIndexType classIndex=ClassIndex();
    hsAssert(sizeof(classIndex)==sizeof(plNetMessageClassIndex), "somebody changed the size of plCreatable::ClassIndex");
    stream->WriteLE16(classIndex);
}

// put in buffer
int plNetMessage::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    IWriteClassIndex(stream);
    
    stream->WriteLE32(fFlags);
    
    if (IsBitSet(kHasVersion))
    {
        stream->WriteByte(fProtocolVerMajor);
        stream->WriteByte(fProtocolVerMinor);
    }
    if (IsBitSet(kHasTimeSent))
        fTimeSent.Write(stream);
    if (IsBitSet(kHasContext))
        stream->WriteLE32(fContext);
    if (IsBitSet(kHasTransactionID))
        stream->WriteLE32(fTransactionID);
    if (IsBitSet(kHasPlayerID))
        stream->WriteLE32(fPlayerID);
    if (IsBitSet(kHasAcctUUID))
        fAcctUUID.Write(stream);

    return stream->GetPosition();
}

void plNetMessage::IReadClassIndex(hsStream* stream)
{
    ClassIndexType classIndex;
    hsAssert(sizeof(classIndex)==sizeof(plNetMessageClassIndex), "somebody changed the size of plCreatable::ClassIndex");
    stream->ReadLE16(&classIndex);
}

// get out of buffer
int plNetMessage::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    IReadClassIndex(stream);
    
    stream->ReadLE32(&fFlags);
    
    // verify version first
    if (IsBitSet(kHasVersion))
    {
        stream->ReadByte(&fProtocolVerMajor);
        stream->ReadByte(&fProtocolVerMinor);
        
        if (fProtocolVerMajor != kVerMajor || fProtocolVerMinor != kVerMinor)
            return 0;   // this will cause derived classes to stop reading
    }
    
    if (!(IsBitSet(kHasVersion)) && (peekOptions & kWantVersion))
    {
        return 0;
    }
    
    if (IsBitSet(kHasTimeSent))
        fTimeSent.Read(stream);
    if (IsBitSet(kHasContext))
        stream->ReadLE32(&fContext);
    if (IsBitSet(kHasTransactionID))
        stream->ReadLE32(&fTransactionID);
    if (IsBitSet(kHasPlayerID))
        stream->ReadLE32(&fPlayerID);
    if (IsBitSet(kHasAcctUUID))
        fAcctUUID.Read( stream );
    return stream->GetPosition();
}

void plNetMessage::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kNetMsgFlags))
        s->ReadLE32(&fFlags);
    
    if (contentFlags.IsBitSet(kNetMsgVersion))
    {
        if (IsBitSet(kHasVersion))
        {
            s->ReadByte(&fProtocolVerMajor);
            s->ReadByte(&fProtocolVerMinor);
        }
    }
    
    if (contentFlags.IsBitSet(kNetMsgTimeSent))
    {
        if (IsBitSet(kHasTimeSent))
            fTimeSent.Read(s);
    }
    
    if (contentFlags.IsBitSet(kNetMsgContext))
    {
        if (IsBitSet(kHasContext))
            s->ReadLE32(&fContext);
    }
    
    if (contentFlags.IsBitSet(kNetMsgTransactionID))
    {
        if (IsBitSet(kHasTransactionID))
            s->ReadLE32(&fTransactionID);
    }
    
    if (contentFlags.IsBitSet(kNetMsgPlayerID))
    {
        if (IsBitSet(kHasPlayerID))
            s->ReadLE32(&fPlayerID);
    }
}

void plNetMessage::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kNetMsgFlags);
    contentFlags.SetBit(kNetMsgTimeSent);
    contentFlags.SetBit(kNetMsgContext);
    contentFlags.SetBit(kNetMsgTransactionID);
    contentFlags.SetBit(kNetMsgPlayerID);
    contentFlags.SetBit(kNetMsgVersion);
    contentFlags.Write(s);
    
    // kNetMsgFlags
    s->WriteLE32(fFlags);
    
    // version
    if (IsBitSet(kHasVersion))
    {
        s->WriteByte(fProtocolVerMajor);
        s->WriteByte(fProtocolVerMinor);
    }
    
    // kNetMsgTimeSent
    if (IsBitSet(kHasTimeSent))
        fTimeSent.Write(s);
    
    // kNetMsgContext
    if (IsBitSet(kHasContext))
        s->WriteLE32(fContext);
    
    // kNetMsgTransactionID
    if (IsBitSet(kHasTransactionID))
        s->WriteLE32(fTransactionID);
    
    // kNetMsgPlayerID
    if (IsBitSet(kHasPlayerID))
        s->WriteLE32(fPlayerID);
}

// Get the Packed Size
int plNetMessage::GetPackSize() 
{
    hsNullStream nullStream;
    return IPokeBuffer(&nullStream);
}

uint32_t plNetMessage::GetNetCoreMsgLen() const
{
    return fNetCoreMsg ? fNetCoreMsg->GetLen() : 0;
}

void plNetMessage::ValidatePeek() const 
{ 
    
}

void plNetMessage::ValidatePoke() const 
{ 
    
}

ST::string plNetMessage::AsString() const
{
    const char* delim = "";

    ST::string_stream ss;
    if (GetHasPlayerID())
    {
        ss << delim << "p:" << GetPlayerID();
        delim = ",";
    }
    if (GetHasTransactionID())
    {
        ss << delim << "x:" << GetTransactionID();
        delim = ",";
    }
    if (GetHasAcctUUID())
    {
        ss << delim << "a:" << GetAcctUUID()->AsString();
        delim = ",";
    }
    if (IsBitSet(kHasVersion))
    {
        ss << delim << "v:" << (int)fProtocolVerMajor << "." << (int)fProtocolVerMinor;
        delim = ",";
    }

    return ss.to_string();
}


////////////////////////////////////////////////////////
// plNetMsgStream
////////////////////////////////////////////////////////
int plNetMsgStream::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fStreamHelper.Poke(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgStream::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fStreamHelper.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgGameMessage
////////////////////////////////////////////////////////
int plNetMsgGameMessage::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgStream::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        if (fDeliveryTime.AtEpoch())
        {
            stream->WriteBool(false);   // not sending
        }
        else
        {
            stream->WriteBool(true);   // sending
            fDeliveryTime.Write(stream);
        }
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgGameMessage::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgStream::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        if (stream->ReadBool())
            fDeliveryTime.Read(stream);
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

plMessage* plNetMsgGameMessage::GetContainedMsg(hsResMgr* resmgr)
{
    hsReadOnlyStream s(StreamInfo()->GetStreamLen(), StreamInfo()->GetStreamBuf());
    return plMessage::ConvertNoRef((resmgr?resmgr:hsgResMgr::ResMgr())->ReadCreatable(&s));
}

void plNetMsgGameMessage::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::ReadVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kNetGameMsgDeliveryTime))
    {
        if (s->ReadBool())
            fDeliveryTime.Read(s);
    }
    
    if (contentFlags.IsBitSet(kNetGameMsgGameMsg))
    {
        plMessage* gameMsg = plMessage::ConvertNoRef(mgr->ReadCreatableVersion(s));
        
        // write message (and label) to ram stream
        hsRAMStream ramStream;
        mgr->WriteCreatable(&ramStream, gameMsg);
        
        // put stream in net msg wrapper
        StreamInfo()->CopyStream(&ramStream);
        
        hsRefCnt_SafeUnRef(gameMsg);
    }
}

void plNetMsgGameMessage::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::WriteVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kNetGameMsgDeliveryTime);
    contentFlags.SetBit(kNetGameMsgGameMsg);
    contentFlags.Write(s);
    
    // kNetGameMsgDeliveryTime
    if (fDeliveryTime.AtEpoch())
    {
        s->WriteBool(false);    // not sending
    }
    else
    {
        s->WriteBool(true);    // sending
        fDeliveryTime.Write(s);
    }
    
    // kNetGameMsgGameMsg
    plMessage* gameMsg = GetContainedMsg();
    mgr->WriteCreatableVersion(s, gameMsg);
    hsRefCnt_SafeUnRef(gameMsg);
}

ST::string plNetMsgGameMessage::AsString() const
{
    const char* noc = plFactory::GetTheFactory()->GetNameOfClass(StreamInfo()->GetStreamType());
    return ST::format("{} {}", plNetMsgStream::AsString(), noc ? noc : "?");
}

////////////////////////////////////////////////////////
// plNetMsgGameMessageDirected
////////////////////////////////////////////////////////
int plNetMsgGameMessageDirected::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgGameMessage::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fReceivers.Poke(stream, peekOptions);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgGameMessageDirected::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgGameMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fReceivers.Peek(stream, peekOptions);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

void plNetMsgGameMessageDirected::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgGameMessage::ReadVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kRecievers))
        fReceivers.ReadVersion(s,mgr);
}

void plNetMsgGameMessageDirected::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgGameMessage::WriteVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kRecievers);
    contentFlags.Write(s);
    
    fReceivers.WriteVersion(s,mgr);
}


////////////////////////////////////////////////////////
// plNetMsgObject
////////////////////////////////////////////////////////
int plNetMsgObject::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fObjectHelper.Poke(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgObject::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fObjectHelper.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

void plNetMsgObject::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::ReadVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kNetMsgObjectHelper))
        fObjectHelper.ReadVersion(s, mgr);
}

void plNetMsgObject::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::WriteVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kNetMsgObjectHelper);
    contentFlags.Write(s);
    
    // kNetMsgObjectHelper
    fObjectHelper.WriteVersion(s, mgr);
}

ST::string plNetMsgObject::AsString() const
{
    return ST::format("object={}, {}", fObjectHelper.GetUoid(), plNetMessage::AsString());
}

////////////////////////////////////////////////////////
// plNetMsgStreamedObject
////////////////////////////////////////////////////////

int plNetMsgStreamedObject::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgObject::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fStreamHelper.Poke(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgStreamedObject::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgObject::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fStreamHelper.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

void plNetMsgStreamedObject::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgObject::ReadVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kStreamHelper))
        fStreamHelper.ReadVersion(s,mgr);
}

void plNetMsgStreamedObject::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgObject::WriteVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kStreamHelper);
    contentFlags.Write(s);
    
    fStreamHelper.WriteVersion(s,mgr);
}


////////////////////////////////////////////////////////////////////
// debug
ST::string plNetMsgSDLState::AsString() const
{
    ISetDescName();     // set desc name for debug if necessary

    return ST::format("object:{}, initial:{}, {}",
        ObjectInfo()->GetObjectName(), fIsInitialState, plNetMsgStreamedObject::AsString());
}

//
// fill out descName for debugging if needed
// fDescName;       // for debugging output only, not read/written
//
void plNetMsgSDLState::ISetDescName() const
{
    if (fDescName.empty() && StreamInfo()->GetStreamLen() && !StreamInfo()->IsCompressed())
    {
        hsReadOnlyStream stream(StreamInfo()->GetStreamLen(), StreamInfo()->GetStreamBuf());
        /* This code can crash the game server sometimes -eap
        char* descName = nullptr;
        int ver;
        if (plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver))
            fDescName = descName;
        delete [] descName;
        */
    }
}

int plNetMsgSDLState::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    ISetDescName();     // stash away the descName before poke/compress
    plNetMsgStreamedObject::IPokeBuffer(stream, peekOptions);
    stream->WriteBool(fIsInitialState);
    stream->WriteBool(fPersistOnServer);
    stream->WriteBool(fIsAvatarState);
    return stream->GetPosition();
}

int plNetMsgSDLState::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    plNetMsgStreamedObject::IPeekBuffer(stream, peekOptions);
    fIsInitialState = stream->ReadBool();
    fPersistOnServer = stream->ReadBool();
    fIsAvatarState = stream->ReadBool();
    ISetDescName();     // stash away the descName after peek/uncompress
    return stream->GetPosition();
}

void plNetMsgSDLState::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgStreamedObject::ReadVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kSDLStateStream))
    {
        uint32_t len = s->ReadLE32();
        uint8_t* buf = new uint8_t[len];
        s->Read(len, buf);
        
        StreamInfo()->SetStreamLen(len);
        StreamInfo()->SetStreamBuf(buf);
    }
    if (contentFlags.IsBitSet(kSDLIsInitialState))
        fIsInitialState = s->ReadBool();
    if (contentFlags.IsBitSet(kSDLPersist))
        fPersistOnServer = s->ReadBool();
    if (contentFlags.IsBitSet(kSDLAvatarState))
        fIsAvatarState = s->ReadBool();
}

void plNetMsgSDLState::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgStreamedObject::WriteVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kSDLStateStream);
    contentFlags.SetBit(kSDLIsInitialState);
    contentFlags.SetBit(kSDLPersist);
    contentFlags.SetBit(kSDLAvatarState);
    contentFlags.Write(s);
    
    // kSDLStateStream
    s->WriteLE32(StreamInfo()->GetStreamLen());
    s->Write(StreamInfo()->GetStreamLen(), StreamInfo()->GetStreamBuf());
    s->WriteBool(fIsInitialState);
    s->WriteBool(fPersistOnServer);
    s->WriteBool(fIsAvatarState);
}

////////////////////////////////////////////////////////
// plNetMsgSDLStateBCast
////////////////////////////////////////////////////////

int plNetMsgSDLStateBCast::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes = plNetMsgSDLState::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgSDLStateBCast::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgSDLState::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        bytes=stream->GetPosition();
    }
    return bytes;
}

void plNetMsgSDLStateBCast::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgSDLState::ReadVersion(s, mgr);
}

void plNetMsgSDLStateBCast::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgSDLState::WriteVersion(s, mgr);
}

////////////////////////////////////////////////////////
// plNetMsgRoomsList
////////////////////////////////////////////////////////
int plNetMsgRoomsList::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        size_t numRooms = fRooms.size();
        stream->WriteLE32((uint32_t)numRooms);
        for (size_t i = 0; i < numRooms; i++)
        {
            fRooms[i].Write(stream);
            
            // write room name for debugging
            plMsgCStringHelper::Poke(fRoomNames[i],stream,peekOptions);
        }
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgRoomsList::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        uint32_t numRooms = stream->ReadLE32();
        fRooms.resize(numRooms);
        fRoomNames.resize(numRooms);
        for (uint32_t i = 0; i < numRooms; i++)
        {
            fRooms[i].Read(stream);
            // read room name for debugging
            plMsgCStringHelper::Peek(fRoomNames[i],stream,peekOptions);
        }
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

void plNetMsgRoomsList::AddRoom(const plKey& rmKey)
{
    fRooms.push_back(rmKey->GetUoid().GetLocation());
    fRoomNames.push_back(rmKey->GetName());
}

void plNetMsgRoomsList::AddRoomLocation(const plLocation& loc, const ST::string& rmName)
{
    fRooms.push_back(loc);
    fRoomNames.push_back(rmName);
}

int plNetMsgRoomsList::FindRoomLocation(const plLocation& loc)
{
    std::vector<plLocation>::iterator result = std::find(fRooms.begin(), fRooms.end(), loc);
    return result==fRooms.end() ? -1 : result-fRooms.begin();   
}

////////////////////////////////////////////////////////
// plNetMsgPagingRoom
////////////////////////////////////////////////////////
int plNetMsgPagingRoom::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgRoomsList::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        stream->WriteByte(fPageFlags);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgPagingRoom::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgRoomsList::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        stream->ReadByte(&fPageFlags);
        bytes=stream->GetPosition();
    }
    return bytes;
}

ST::string plNetMsgPagingRoom::AsString() const
{
    return ST::format("pageFlags:{02X}, paging {}, requestingState:{}, resetting={}",
        fPageFlags, (fPageFlags & kPagingOut) ? "out" : "in",
        (fPageFlags & kRequestState) ? "yes" : "no", (fPageFlags & kResetList) != 0);
}

////////////////////////////////////////////////////////
// plNetMsgGroupOwner
////////////////////////////////////////////////////////
void plNetMsgGroupOwner::GroupInfo::Read(hsStream* s)
{
    fGroupID.Read(s);
    fOwnIt = s->ReadBool();
}

void plNetMsgGroupOwner::GroupInfo::Write(hsStream* s) const
{
    fGroupID.Write(s);
    s->WriteBool(fOwnIt);
}

int plNetMsgGroupOwner::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        stream->WriteLE32((uint32_t)fGroups.size());
        for (const GroupInfo& group : fGroups)
            group.Write(stream);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgGroupOwner::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        uint32_t num = stream->ReadLE32();
        fGroups.resize(num);
        for (uint32_t i = 0; i < num; i++)
            fGroups[i].Read(stream);

        bytes=stream->GetPosition();
    }
    
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgSharedState
////////////////////////////////////////////////////////

void plNetMsgSharedState::CopySharedState(plNetSharedState* ss)
{
    hsRAMStream stream;
    ss->Write(&stream);
    StreamInfo()->CopyStream(&stream);
}

int plNetMsgSharedState::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgStreamedObject::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        stream->WriteBool(fLockRequest);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgSharedState::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgStreamedObject::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fLockRequest = stream->ReadBool();
        bytes=stream->GetPosition();
    }
    return bytes;
}

void plNetMsgSharedState::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgStreamedObject::ReadVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kLockRequest))
        fLockRequest = s->ReadBool();
}

void plNetMsgSharedState::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgStreamedObject::WriteVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kLockRequest);
    contentFlags.Write(s);
    
    s->WriteBool(fLockRequest);
}

ST::string plNetMsgSharedState::AsString() const
{
    return ST::format("lockReq={}, {}", fLockRequest, plNetMsgStreamedObject::AsString());
}

////////////////////////////////////////////////////////
// plNetMsgGetSharedState
////////////////////////////////////////////////////////
int plNetMsgGetSharedState::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgObject::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        plMsgCArrayHelper::Poke(fSharedStateName,sizeof(fSharedStateName),stream,peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgGetSharedState::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgObject::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        plMsgCArrayHelper::Peek(fSharedStateName,sizeof(fSharedStateName),stream,peekOptions);
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgObject
////////////////////////////////////////////////////////
int plNetMsgObjectUpdateFilter::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fObjectListHelper.Poke(stream, peekOptions);
        stream->WriteLEFloat(fMaxUpdateFreq);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgObjectUpdateFilter::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fObjectListHelper.Peek(stream, peekOptions);
        stream->ReadLEFloat(&fMaxUpdateFreq);
        
        bytes=stream->GetPosition();
    }
    
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgMembersList
////////////////////////////////////////////////////////
int plNetMsgMembersList::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        fMemberListHelper.Poke(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgMembersList::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fMemberListHelper.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgMemberUpdate
////////////////////////////////////////////////////////
int plNetMsgMemberUpdate::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
    if (bytes)
    {
        // FIX ME to something nice
        fMemberInfo.GetClientGuid()->SetClientKey("");
        fMemberInfo.GetClientGuid()->SetAccountUUID(plUUID());
        fMemberInfo.Poke(stream, peekOptions);
        stream->WriteBool(fAddMember);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

int plNetMsgMemberUpdate::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fMemberInfo.Peek(stream, peekOptions);
        fAddMember = stream->ReadBool();
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgVoice
////////////////////////////////////////////////////////
int plNetMsgVoice::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    plNetMessage::IPokeBuffer(stream, peekOptions);
    stream->WriteByte(fFlags);
    stream->WriteByte(fNumFrames);
    plMsgStdStringHelper::Poke(fVoiceData, stream, peekOptions);
    fReceivers.Poke(stream, peekOptions);
    return stream->GetPosition();
}

int plNetMsgVoice::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        stream->ReadByte(&fFlags);
        stream->ReadByte(&fNumFrames);
        plMsgStdStringHelper::Peek(fVoiceData, stream, peekOptions);
        fReceivers.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

void plNetMsgVoice::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::ReadVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kDead_FrameSize))
        (void)s->ReadLE16();
    if (contentFlags.IsBitSet(kReceivers))
        fReceivers.ReadVersion(s,mgr);
    if (contentFlags.IsBitSet(kVoiceFlags))
        s->ReadByte(&fFlags);
    if(contentFlags.IsBitSet(kVoiceData))
        plMsgStdStringHelper::Peek(fVoiceData, s);
}

void plNetMsgVoice::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMessage::WriteVersion(s,mgr);
    
    hsBitVector contentFlags;

    contentFlags.SetBit(kReceivers);
    contentFlags.SetBit(kVoiceFlags);
    contentFlags.SetBit(kVoiceData);
    contentFlags.Write(s);
    
    fReceivers.WriteVersion(s,mgr);
    s->WriteByte(fFlags);
    plMsgStdStringHelper::Poke(fVoiceData, s);
}

void plNetMsgVoice::SetVoiceData(const void* data, size_t len)
{
    fVoiceData.resize(len);
    memcpy((void *)fVoiceData.data(), data, len );
}

const char *plNetMsgVoice::GetVoiceData() const
{
    return fVoiceData.c_str();
}

ST::string plNetMsgVoice::AsString() const
{
    return ST::format("len={}", fVoiceData.size());
}

////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// plNetMsgListenListUpdate
////////////////////////////////////////////////////////
int plNetMsgListenListUpdate::IPokeBuffer(hsStream* stream, uint32_t peekOptions)
{
    plNetMessage::IPokeBuffer(stream, peekOptions);
    stream->WriteBool(fAdding);
    fReceivers.Poke(stream, peekOptions);
    return stream->GetPosition();
}

int plNetMsgListenListUpdate::IPeekBuffer(hsStream* stream, uint32_t peekOptions)
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
    if (bytes)
    {
        fAdding = stream->ReadBool();
        fReceivers.Peek(stream, peekOptions);
        bytes=stream->GetPosition();
    }
    return bytes;
}

////////////////////////////////////////////////////////////////////
// plNetMsgPlayerPage
////////////////////////////////////////////////////////////////////

int plNetMsgPlayerPage::IPokeBuffer( hsStream* stream, uint32_t peekOptions )
{
    plNetMessage::IPokeBuffer( stream, peekOptions );
    stream->WriteBool(fUnload);
    fUoid.Write(stream);
    
    return stream->GetPosition();
}

int plNetMsgPlayerPage::IPeekBuffer( hsStream* stream, uint32_t peekOptions )
{
    int bytes = plNetMessage::IPeekBuffer(stream, peekOptions );
    if ( bytes )
    {
        fUnload = stream->ReadBool();
        fUoid.Read(stream);
        bytes = stream->GetPosition();
    }
    return bytes;
}


////////////////////////////////////////////////////////////////////
// plNetMsgLoadClone
////////////////////////////////////////////////////////////////////

int plNetMsgLoadClone::IPokeBuffer( hsStream* stream, uint32_t peekOptions )
{
    int bytes = plNetMsgGameMessage::IPokeBuffer( stream, peekOptions );
    if ( bytes )
    {
        fObjectHelper.Poke(stream, peekOptions);
        stream->WriteBool(fIsPlayer);
        stream->WriteBool(fIsLoading);
        stream->WriteBool(fIsInitialState);
        bytes = stream->GetPosition();
    }
    return bytes;   
}

int plNetMsgLoadClone::IPeekBuffer( hsStream* stream, uint32_t peekOptions )
{
    int bytes = plNetMsgGameMessage::IPeekBuffer(stream, peekOptions );
    if ( bytes )
    {
        fObjectHelper.Peek(stream, peekOptions);
        
        fIsPlayer = stream->ReadBool();
        fIsLoading = stream->ReadBool();
        fIsInitialState = stream->ReadBool();
        
        bytes = stream->GetPosition();
    }
    return bytes;
}

void plNetMsgLoadClone::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgGameMessage::ReadVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);
    
    if (contentFlags.IsBitSet(kObjectHelper))
        fObjectHelper.ReadVersion(s,mgr);
    if (contentFlags.IsBitSet(kIsPlayer))
        fIsPlayer = s->ReadBool();
    if (contentFlags.IsBitSet(kIsLoading))
        fIsLoading = s->ReadBool();
    if (contentFlags.IsBitSet(kIsInitialState))
        fIsInitialState = s->ReadBool();
}

void plNetMsgLoadClone::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plNetMsgGameMessage::WriteVersion(s,mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kObjectHelper);
    contentFlags.SetBit(kIsPlayer);
    contentFlags.SetBit(kIsLoading);
    contentFlags.SetBit(kIsInitialState);
    contentFlags.Write(s);
    
    fObjectHelper.WriteVersion(s,mgr);
    s->WriteBool(fIsPlayer);
    s->WriteBool(fIsLoading);
    s->WriteBool(fIsInitialState);
}

ST::string plNetMsgLoadClone::AsString() const
{
    return ST::format("object={} initial={}, {}", fObjectHelper.GetUoid(), fIsInitialState,
        plNetMsgGameMessage::AsString());
}

////////////////////////////////////////////////////////////////////

int plNetMsgInitialAgeStateSent::IPokeBuffer( hsStream* stream, uint32_t peekOptions )
{
    plNetMessage::IPokeBuffer( stream, peekOptions );
    stream->WriteLE32(fNumInitialSDLStates);
    return stream->GetPosition();
}

int plNetMsgInitialAgeStateSent::IPeekBuffer( hsStream* stream, uint32_t peekOptions )
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions );
    if (bytes)
    {
        stream->ReadLE32(&fNumInitialSDLStates);
        bytes=stream->GetPosition();
    }
    return bytes;
}


////////////////////////////////////////////////////////////////////
// plNetMsgRelevanceRegions
////////////////////////////////////////////////////////////////////

int plNetMsgRelevanceRegions::IPokeBuffer( hsStream* stream, uint32_t peekOptions )
{
    plNetMessage::IPokeBuffer( stream, peekOptions );
    fRegionsICareAbout.Write(stream);
    fRegionsImIn.Write(stream);
    
    return stream->GetPosition();
}

int plNetMsgRelevanceRegions::IPeekBuffer( hsStream* stream, uint32_t peekOptions )
{
    int bytes=plNetMessage::IPeekBuffer(stream, peekOptions );
    if (bytes)
    {
        fRegionsICareAbout.Read(stream);
        fRegionsImIn.Read(stream);
        
        bytes=stream->GetPosition();
    }
    return bytes;
}

ST::string plNetMsgRelevanceRegions::AsString() const
{
    ST::string b1, b2;
    for (uint32_t i = 0; i < fRegionsImIn.GetNumBitVectors(); i++)
        b1 += ST::format("{#x} ", fRegionsImIn.GetBitVector(i));
    for (uint32_t i = 0; i < fRegionsICareAbout.GetNumBitVectors(); i++)
        b2 += ST::format("{#x} ", fRegionsICareAbout.GetBitVector(i));
    return ST::format("rgnsImIn:{}, rgnsICareAbout:{}, {}",
        b1, b2, plNetMessage::AsString());
}

////////////////////////////////////////////////////////////////////
// End.
