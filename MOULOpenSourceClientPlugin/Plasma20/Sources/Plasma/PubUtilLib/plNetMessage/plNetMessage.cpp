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
#include "hsWindows.h"
#include "hsResMgr.h"
#include "plNetMessage.h"
#include "plNetCommonMessage.h"
#include "plNetMsgVersion.h"
#include "plCreatableIndex.h"

#include "../pnKeyedObject/plKeyImp.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnNetCommon/plNetSharedState.h"
#include "../pnMessage/plMessage.h"
#include "../pnNetCommon/pnNetCommon.h"
#include "../pnNetCommon/plGenericVar.h"
#include "../pnFactory/plFactory.h"

#include "../plVault/plVault.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plSDL/plSDL.h"

#if defined(HS_BUILD_FOR_UNIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <algorithm>

//
// static
//
// see plNetMsgVersion.h
const UInt8 plNetMessage::kVerMajor = PLASMA2_NETMSG_MAJOR_VERSION;
const UInt8 plNetMessage::kVerMinor = PLASMA2_NETMSG_MINOR_VERSION;



////////////////////////////////////////////////////////
// plNetMessage
////////////////////////////////////////////////////////
plNetMessage::plNetMessage() :
fTimeRecvd(0),
fBytesRead(0),
fNetCoreMsg(nil),
fContext(0),
fPeekStatus(0),
fTransactionID(0),
fPlayerID(kInvalidPlayerID),
fFlags(0),
fProtocolVerMajor(0),
fProtocolVerMinor(0)
{
}

plNetMessage::~plNetMessage()
{
	
}

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

#if 0	// I don't think the version should be copied
	if (msg->IsBitSet(kHasVersion))
		SetVersion();
#endif
}

//
// STATIC
// create and READ from lowlevel net buffer
//
plNetMessage* plNetMessage::CreateAndRead(const plNetCommonMessage* msg, plStreamLogger::EventList* el)
{
	// create
	plNetMessage* pHdr = Create(msg);
	if (!pHdr)
		return nil;
	
	// read
	pHdr->PeekBuffer(msg->GetData(), msg->GetLen(), 0, false, el);
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
		readStream.ReadSwap(&classIndex);
		if (!plFactory::IsValidClassIndex(classIndex))
			return nil;
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
	return nil;
}

int plNetMessage::PokeBuffer(char* bufIn, int bufLen, UInt32 peekOptions)
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

int plNetMessage::PeekBuffer(const char* bufIn, int bufLen, UInt32 peekOptions, bool forcePeek, plStreamLogger::EventList* el)
{
	if(!bufLen || bufLen < 1)
		return 0;

	UInt32 partialPeekOptions = (peekOptions & kPartialPeekMask);
	if (!forcePeek && (fPeekStatus & partialPeekOptions) )
		return 0;	// already peeked, fully or partially
	
	if (!bufIn)
		return 0;
	
	// set peek status based on peekOptions
	fPeekStatus = partialPeekOptions ? partialPeekOptions : kFullyPeeked;
	
	hsReadOnlyLoggingStream readStream;
	readStream.LogSetList(el);
	readStream.Init(bufLen, bufIn);
	readStream.LogSubStreamStart("plNetMessage");
	readStream.LogStringString(xtl::format("ClassName: %s",this->ClassName()).c_str());
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
	
	readStream.LogSubStreamEnd();
	return ret;
}

void plNetMessage::IWriteClassIndex(hsStream* stream)
{
	ClassIndexType classIndex=ClassIndex();
	hsAssert(sizeof(classIndex)==sizeof(plNetMessageClassIndex), "somebody changed the size of plCreatable::ClassIndex");
	stream->WriteSwap(classIndex);
}

// put in buffer
int plNetMessage::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	IWriteClassIndex(stream);
	
	stream->WriteSwap32(fFlags);
	
	if (IsBitSet(kHasVersion))
	{
		stream->WriteByte(fProtocolVerMajor);
		stream->WriteByte(fProtocolVerMinor);
	}
	if (IsBitSet(kHasTimeSent))
		fTimeSent.Write(stream);
	if (IsBitSet(kHasContext))
		stream->WriteSwap(fContext);
	if (IsBitSet(kHasTransactionID))
		stream->WriteSwap(fTransactionID);
	if (IsBitSet(kHasPlayerID))
		stream->WriteSwap(fPlayerID);
	if (IsBitSet(kHasAcctUUID))
		fAcctUUID.Write(stream);

	return stream->GetPosition();
}

void plNetMessage::IReadClassIndex(hsStream* stream)
{
	ClassIndexType classIndex;
	hsAssert(sizeof(classIndex)==sizeof(plNetMessageClassIndex), "somebody changed the size of plCreatable::ClassIndex");
	stream->LogReadSwap(&classIndex,"ClassIndex");
}

// get out of buffer
int plNetMessage::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	IReadClassIndex(stream);
	
	stream->LogReadSwap(&fFlags,"Flags");
	
	// verify version first
	if (IsBitSet(kHasVersion))
	{
		stream->LogReadSwap(&fProtocolVerMajor, "Protocol major version");
		stream->LogReadSwap(&fProtocolVerMinor, "Protocol minor version");
		
		if (fProtocolVerMajor != kVerMajor || fProtocolVerMinor != kVerMinor)
			return 0;	// this will cause derived classes to stop reading
	}
	
	if (!(IsBitSet(kHasVersion)) && (peekOptions & kWantVersion))
	{
		return 0;
	}
	
	if (IsBitSet(kHasTimeSent))
		fTimeSent.Read(stream);
	if (IsBitSet(kHasContext))
		stream->LogReadSwap(&fContext,"Context");
	if (IsBitSet(kHasTransactionID))
		stream->LogReadSwap(&fTransactionID,"TransactionID");
	if (IsBitSet(kHasPlayerID))
		stream->LogReadSwap(&fPlayerID,"PlayerID");
	if (IsBitSet(kHasAcctUUID))
		fAcctUUID.Read( stream );
	return stream->GetPosition();
}

void plNetMessage::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);
	
	if (contentFlags.IsBitSet(kNetMsgFlags))
		s->LogReadSwap(&fFlags,"Flags");
	
	if (contentFlags.IsBitSet(kNetMsgVersion))
	{
		if (IsBitSet(kHasVersion))
		{
			s->LogReadSwap(&fProtocolVerMajor, "Protocol major version");
			s->LogReadSwap(&fProtocolVerMinor, "Protocol minor version");
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
			s->LogReadSwap(&fContext,"Context");
	}
	
	if (contentFlags.IsBitSet(kNetMsgTransactionID))
	{
		if (IsBitSet(kHasTransactionID))
			s->LogReadSwap(&fTransactionID,"TransactionID");
	}
	
	if (contentFlags.IsBitSet(kNetMsgPlayerID))
	{
		if (IsBitSet(kHasPlayerID))
			s->LogReadSwap(&fPlayerID,"PlayerID");
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
	s->WriteSwap32(fFlags);
	
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
		s->WriteSwap(fContext);
	
	// kNetMsgTransactionID
	if (IsBitSet(kHasTransactionID))
		s->WriteSwap(fTransactionID);
	
	// kNetMsgPlayerID
	if (IsBitSet(kHasPlayerID))
		s->WriteSwap(fPlayerID);
}

// Get the Packed Size
int plNetMessage::GetPackSize() 
{
	hsNullStream nullStream;
	return IPokeBuffer(&nullStream);
}

UInt32 plNetMessage::GetNetCoreMsgLen() const
{
	return fNetCoreMsg ? fNetCoreMsg->GetLen() : 0;
}

void plNetMessage::ValidatePeek() const 
{ 
	
}

void plNetMessage::ValidatePoke() const 
{ 
	
}


////////////////////////////////////////////////////////
// plNetMsgStream
////////////////////////////////////////////////////////
int plNetMsgStream::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("StreamHelper");
		fStreamHelper.Poke(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgStream::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("MsgStreamStream");
		fStreamHelper.Peek(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgGameMessage
////////////////////////////////////////////////////////
int plNetMsgGameMessage::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgStream::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		if (fDeliveryTime.AtEpoch())
		{
			stream->WriteByte(0);	// not sending
		}
		else
		{
			stream->WriteByte(1);	// sending
			fDeliveryTime.Write(stream);
		}
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgGameMessage::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgStream::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		if (stream->ReadByte())
		{
			stream->LogSubStreamPushDesc("GameMessage DeliveryTime");
			fDeliveryTime.Read(stream);
		}
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
		if (s->ReadByte())
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
		s->WriteByte(0);	// not sending
	}
	else
	{
		s->WriteByte(1);	// sending
		fDeliveryTime.Write(s);
	}
	
	// kNetGameMsgGameMsg
	plMessage* gameMsg = GetContainedMsg();
	mgr->WriteCreatableVersion(s, gameMsg);
	hsRefCnt_SafeUnRef(gameMsg);
}

////////////////////////////////////////////////////////
// plNetMsgGameMessageDirected
////////////////////////////////////////////////////////
int plNetMsgGameMessageDirected::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgGameMessage::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		fReceivers.Poke(stream, peekOptions);
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgGameMessageDirected::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgGameMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("GameMessageDirected Receivers");
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
int plNetMsgObject::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		fObjectHelper.Poke(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgObject::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("MsgObject");
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

////////////////////////////////////////////////////////
// plNetMsgStreamedObject
////////////////////////////////////////////////////////

int plNetMsgStreamedObject::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgObject::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		fStreamHelper.Poke(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgStreamedObject::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgObject::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("StreamedObject");
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
std::string plNetMsgSDLState::AsStdString() const
{
	std::string s;

	ISetDescName();		// set desc name for debug if necessary

//	xtl::format(s,"object:%s, SDL:%s, initial:%d, %s",
//		ObjectInfo()->GetObjectName(), fDescName.c_str(), fIsInitialState, plNetMsgStreamedObject::AsStdString().c_str() );

	xtl::format(s,"object:%s, initial:%d, %s",
		ObjectInfo()->GetObjectName(), fIsInitialState, plNetMsgStreamedObject::AsStdString().c_str() );

	return s;
}

//
// fill out descName for debugging if needed
// fDescName;		// for debugging output only, not read/written
//
void plNetMsgSDLState::ISetDescName() const
{
	if (fDescName.empty() && StreamInfo()->GetStreamLen() && !StreamInfo()->IsCompressed())
	{
		hsReadOnlyStream stream(StreamInfo()->GetStreamLen(), StreamInfo()->GetStreamBuf());
		/* This code can crash the game server sometimes -eap
		char* descName = nil;
		int ver;
		if (plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver))
			fDescName = descName;
		delete [] descName;
		*/
	}
}

int plNetMsgSDLState::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	ISetDescName();		// stash away the descName before poke/compress
	plNetMsgStreamedObject::IPokeBuffer(stream, peekOptions);
	stream->WriteSwap( fIsInitialState );
	stream->WriteSwap(fPersistOnServer);
	stream->WriteSwap(fIsAvatarState);
	return stream->GetPosition();
}

int plNetMsgSDLState::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	plNetMsgStreamedObject::IPeekBuffer(stream, peekOptions);
	stream->LogReadSwap( &fIsInitialState, "IsInitialAgeState" );
	stream->LogReadSwap(&fPersistOnServer, "SDLState PersistOnServer");
	stream->LogReadSwap(&fIsAvatarState, "SDLState IsAvatarState");
	ISetDescName();		// stash away the descName after peek/uncompress
	return stream->GetPosition();
}

void plNetMsgSDLState::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plNetMsgStreamedObject::ReadVersion(s, mgr);
	
	hsBitVector contentFlags;
	contentFlags.Read(s);
	
	if (contentFlags.IsBitSet(kSDLStateStream))
	{
		UInt32 len;
		s->LogReadSwap(&len,"SDLState StreamLen");
		UInt8* buf = TRACKED_NEW UInt8[len];
		s->LogRead(len, buf,"SDLState StreamData");
		
		StreamInfo()->SetStreamLen(len);
		StreamInfo()->SetStreamBuf(buf);
	}
	if (contentFlags.IsBitSet(kSDLIsInitialState))
		s->LogReadSwap( &fIsInitialState, "IsInitialAgeState" );
	if (contentFlags.IsBitSet(kSDLPersist))
		s->ReadSwap(&fPersistOnServer);
	if (contentFlags.IsBitSet(kSDLAvatarState))
		s->ReadSwap(&fIsAvatarState);
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
	s->WriteSwap32(StreamInfo()->GetStreamLen());
	s->Write(StreamInfo()->GetStreamLen(), StreamInfo()->GetStreamBuf());
	s->WriteSwap( fIsInitialState );
	s->WriteSwap(fPersistOnServer);
	s->WriteSwap(fIsAvatarState);
}

////////////////////////////////////////////////////////
// plNetMsgSDLStateBCast
////////////////////////////////////////////////////////

int plNetMsgSDLStateBCast::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes = plNetMsgSDLState::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgSDLStateBCast::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
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
plNetMsgRoomsList::~plNetMsgRoomsList()
{
	int i;
	for(i=0;i<GetNumRooms();i++)
		delete [] fRoomNames[i];
}

int plNetMsgRoomsList::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		int i, numRooms=fRooms.size();
		stream->WriteSwap(numRooms);
		for(i=0;i<numRooms;i++)
		{
			fRooms[i].Write(stream);
			
			// write room name for debugging
			plMsgCStringHelper::Poke(fRoomNames[i],stream,peekOptions);
		}
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgRoomsList::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		int i, numRooms;
		stream->LogReadSwap(&numRooms,"RoomList NumRooms");
		fRooms.resize(numRooms);
		int oldSize = fRoomNames.size();
		fRoomNames.resize(numRooms);
		for(i=0;i<numRooms;i++)
		{
			plLocation loc;
			loc.Read(stream);
			fRooms[i]=loc;
			// read room name for debugging
			delete [] fRoomNames[i];
			stream->LogSubStreamPushDesc("RoomList");
			plMsgCStringHelper::Peek(fRoomNames[i],stream,peekOptions);
		}
		bytes=stream->GetPosition();
	}
	
	return bytes;
}

void plNetMsgRoomsList::AddRoom(plKey rmKey)
{
	fRooms.push_back(rmKey->GetUoid().GetLocation());
	fRoomNames.push_back(hsStrcpy(rmKey->GetName()));
}

void plNetMsgRoomsList::AddRoomLocation(plLocation loc, const char* rmName)
{
	fRooms.push_back(loc);
	fRoomNames.push_back(rmName ? hsStrcpy(rmName) : nil);
}

int plNetMsgRoomsList::FindRoomLocation(plLocation loc)
{
	std::vector<plLocation>::iterator result = std::find(fRooms.begin(), fRooms.end(), loc);
	return result==fRooms.end() ? -1 : result-fRooms.begin();	
}

////////////////////////////////////////////////////////
// plNetMsgPagingRoom
////////////////////////////////////////////////////////
int plNetMsgPagingRoom::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgRoomsList::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->WriteSwap(fPageFlags);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgPagingRoom::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgRoomsList::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogReadSwap(&fPageFlags,"PageFlags");
		bytes=stream->GetPosition();
	}
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgGroupOwner
////////////////////////////////////////////////////////
int plNetMsgGroupOwner::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		int i, numGroups=fGroups.size();
		stream->WriteSwap(numGroups);
		for(i=0;i<numGroups;i++)
			fGroups[i].Write(stream);
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgGroupOwner::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		int i, num;
		stream->LogReadSwap(&num,"GroupOwnerNum");
		fGroups.resize(num);
		for(i=0;i<num;i++)
		{
			GroupInfo gr;
			gr.Read(stream);
			fGroups[i]=gr;
		}
		
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

int plNetMsgSharedState::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgStreamedObject::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->WriteSwap(fLockRequest);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgSharedState::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgStreamedObject::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogReadSwap(&fLockRequest,"SharedState LockRequest");
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
		s->ReadSwap(&fLockRequest);
}

void plNetMsgSharedState::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plNetMsgStreamedObject::WriteVersion(s,mgr);
	
	hsBitVector contentFlags;
	contentFlags.SetBit(kLockRequest);
	contentFlags.Write(s);
	
	s->WriteSwap(fLockRequest);
}


////////////////////////////////////////////////////////
// plNetMsgGetSharedState
////////////////////////////////////////////////////////
int plNetMsgGetSharedState::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgObject::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		plMsgCArrayHelper::Poke(fSharedStateName,sizeof(fSharedStateName),stream,peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgGetSharedState::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgObject::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("SharedStateName");
		plMsgCArrayHelper::Peek(fSharedStateName,sizeof(fSharedStateName),stream,peekOptions);
		bytes=stream->GetPosition();
	}
	
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgObject
////////////////////////////////////////////////////////
int plNetMsgObjectUpdateFilter::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		fObjectListHelper.Poke(stream, peekOptions);
		stream->WriteSwap(fMaxUpdateFreq);
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgObjectUpdateFilter::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("ObjectUpdateFilter");
		fObjectListHelper.Peek(stream, peekOptions);
		stream->LogReadSwap(&fMaxUpdateFreq,"MsgObjectUpdateFilter MaxUpdateFreq");
		
		bytes=stream->GetPosition();
	}
	
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgMembersList
////////////////////////////////////////////////////////
int plNetMsgMembersList::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		fMemberListHelper.Poke(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgMembersList::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("MembersList");
		fMemberListHelper.Peek(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgMemberUpdate
////////////////////////////////////////////////////////
int plNetMsgMemberUpdate::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPokeBuffer(stream, peekOptions);
	if (bytes)
	{
		// FIX ME to something nice
		fMemberInfo.GetClientGuid()->SetClientKey("");
		fMemberInfo.GetClientGuid()->SetAccountUUID(plUUID());
		fMemberInfo.Poke(stream, peekOptions);
		stream->WriteByte(fAddMember);
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

int plNetMsgMemberUpdate::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMsgServerToClient::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogSubStreamPushDesc("MemberUpdate");
		fMemberInfo.Peek(stream, peekOptions);
		fAddMember = stream->ReadByte();
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

////////////////////////////////////////////////////////
// plNetMsgVoice
////////////////////////////////////////////////////////
int plNetMsgVoice::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	plNetMessage::IPokeBuffer(stream, peekOptions);
	stream->WriteSwap(fFlags);
	stream->WriteSwap(fNumFrames);
	plMsgStdStringHelper::Poke(fVoiceData, stream, peekOptions);
	fReceivers.Poke(stream, peekOptions);
	return stream->GetPosition();
}

int plNetMsgVoice::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogReadSwap(&fFlags,"Voice Flags");
		stream->LogReadSwap(&fNumFrames, "Number of encoded frames");
		stream->LogSubStreamPushDesc("Voice Data");
		plMsgStdStringHelper::Peek(fVoiceData, stream, peekOptions);
		stream->LogSubStreamPushDesc("Voice Receivers");
		fReceivers.Peek(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

void plNetMsgVoice::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plNetMessage::ReadVersion(s,mgr);
	
	UInt16 old = 0;
	hsBitVector contentFlags;
	contentFlags.Read(s);
	
	if (contentFlags.IsBitSet(kDead_FrameSize))
		s->ReadSwap(&old);
	if (contentFlags.IsBitSet(kReceivers))
		fReceivers.ReadVersion(s,mgr);
	if (contentFlags.IsBitSet(kVoiceFlags))
		s->ReadSwap(&fFlags);
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
	s->WriteSwap(fFlags);
	plMsgStdStringHelper::Poke(fVoiceData, s);
}

void plNetMsgVoice::SetVoiceData(char *data, int len)
{  	
	fVoiceData.resize( len );
	memcpy((void *)fVoiceData.data(), data, len );
}

const char *plNetMsgVoice::GetVoiceData() const
{
	return fVoiceData.c_str();
}
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// plNetMsgListenListUpdate
////////////////////////////////////////////////////////
int plNetMsgListenListUpdate::IPokeBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPokeBuffer(stream, peekOptions);
	stream->WriteSwap(fAdding);
	fReceivers.Poke(stream, peekOptions);
	return stream->GetPosition();
}

int plNetMsgListenListUpdate::IPeekBuffer(hsStream* stream, UInt32 peekOptions)
{
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions);
	if (bytes)
	{
		stream->LogReadSwap(&fAdding,"ListenListUpdate Adding");
		stream->LogSubStreamPushDesc("ListenListUpdate Reveivers");
		fReceivers.Peek(stream, peekOptions);
		bytes=stream->GetPosition();
	}
	return bytes;
}

////////////////////////////////////////////////////////////////////
// plNetMsgPlayerPage
////////////////////////////////////////////////////////////////////

int plNetMsgPlayerPage::IPokeBuffer( hsStream* stream, UInt32 peekOptions )
{
	plNetMessage::IPokeBuffer( stream, peekOptions );
	stream->WriteSwap( fUnload );
	fUoid.Write(stream);
	
	return stream->GetPosition();
}

int plNetMsgPlayerPage::IPeekBuffer( hsStream* stream, UInt32 peekOptions )
{
	int bytes = plNetMessage::IPeekBuffer(stream, peekOptions );
	if ( bytes )
	{
		stream->LogReadSwap( &fUnload,"PlayersPage Unload");
		fUoid.Read(stream);
		bytes = stream->GetPosition();
	}
	return bytes;
}


////////////////////////////////////////////////////////////////////
// plNetMsgLoadClone
////////////////////////////////////////////////////////////////////

int plNetMsgLoadClone::IPokeBuffer( hsStream* stream, UInt32 peekOptions )
{
	int bytes = plNetMsgGameMessage::IPokeBuffer( stream, peekOptions );
	if ( bytes )
	{
		fObjectHelper.Poke(stream, peekOptions);
		stream->WriteSwap( fIsPlayer );
		stream->WriteSwap( fIsLoading );
		stream->WriteSwap( fIsInitialState );
		bytes = stream->GetPosition();
	}
	return bytes;	
}

int plNetMsgLoadClone::IPeekBuffer( hsStream* stream, UInt32 peekOptions )
{
	stream->LogSubStreamPushDesc("LoadClone");
	int bytes = plNetMsgGameMessage::IPeekBuffer(stream, peekOptions );
	if ( bytes )
	{
		stream->LogSubStreamPushDesc("MsgObject");
		fObjectHelper.Peek(stream, peekOptions);
		
		stream->LogReadSwap( &fIsPlayer,"LoadClone IsPlayer");
		stream->LogReadSwap( &fIsLoading,"LoadClone IsLoading");
		stream->LogReadSwap( &fIsInitialState, "LoadClone IsInitialState" );
		
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
		s->ReadSwap(&fIsPlayer);
	if (contentFlags.IsBitSet(kIsLoading))
		s->ReadSwap(&fIsLoading);
	if (contentFlags.IsBitSet(kIsInitialState))
		s->ReadSwap(&fIsInitialState);
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
	s->WriteSwap(fIsPlayer);
	s->WriteSwap(fIsLoading);
	s->WriteSwap(fIsInitialState);
}

////////////////////////////////////////////////////////////////////

int plNetMsgInitialAgeStateSent::IPokeBuffer( hsStream* stream, UInt32 peekOptions )
{
	plNetMessage::IPokeBuffer( stream, peekOptions );
	stream->WriteSwap( fNumInitialSDLStates );
	return stream->GetPosition();
}

int plNetMsgInitialAgeStateSent::IPeekBuffer( hsStream* stream, UInt32 peekOptions )
{
	stream->LogSubStreamPushDesc("InitialAgeStateSent");
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions );
	if (bytes)
	{
		stream->LogReadSwap( &fNumInitialSDLStates, "NumInitialSDLStates" );
		bytes=stream->GetPosition();
	}
	return bytes;
}


////////////////////////////////////////////////////////////////////
// plNetMsgRelevanceRegions
////////////////////////////////////////////////////////////////////

int plNetMsgRelevanceRegions::IPokeBuffer( hsStream* stream, UInt32 peekOptions )
{
	plNetMessage::IPokeBuffer( stream, peekOptions );
	fRegionsICareAbout.Write(stream);
	fRegionsImIn.Write(stream);
	
	return stream->GetPosition();
}

int plNetMsgRelevanceRegions::IPeekBuffer( hsStream* stream, UInt32 peekOptions )
{
	stream->LogSubStreamPushDesc("RelevanceRegions");
	int bytes=plNetMessage::IPeekBuffer(stream, peekOptions );
	if (bytes)
	{
		fRegionsICareAbout.Read(stream);
		fRegionsImIn.Read(stream);
		
		bytes=stream->GetPosition();
	}
	return bytes;
}

////////////////////////////////////////////////////////////////////
// End.
