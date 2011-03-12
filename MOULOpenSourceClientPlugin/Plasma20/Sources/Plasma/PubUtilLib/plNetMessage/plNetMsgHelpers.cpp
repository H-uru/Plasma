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
#include "plNetMsgHelpers.h"
#include "plNetMessage.h"
#include "../plCompression/plZlibCompress.h"
#include "../pnNetCommon/plNetServers.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plMessage.h"
#include "hsStream.h"
#include <algorithm>


////////////////////////////////////////////////////////////////////
// plNetMsgStreamableHelper

plNetMsgStreamableHelper & plNetMsgStreamableHelper::operator =(hsStreamable * value)
{
	fObject = value;
	return *this;
}

int plNetMsgStreamableHelper::Poke(hsStream* stream, UInt32 peekOptions)
{
	hsAssert(fObject, "plNetMsgStreamableHelper::Poke: fObject not set.");
	fObject->Write(stream);
	return stream->GetPosition();
}

int plNetMsgStreamableHelper::Peek(hsStream* stream, UInt32 peekOptions)
{
	hsAssert(fObject, "plNetMsgStreamableHelper::Peek: fObject not set.");
	fObject->Read(stream);
	return stream->GetPosition();
}

////////////////////////////////////////////////////////////////////
// plNetMsgCreatableHelper

plNetMsgCreatableHelper::plNetMsgCreatableHelper(plCreatable * object)
:fCreatable(object)
,fWeCreatedIt(false)
{
}

plNetMsgCreatableHelper::~plNetMsgCreatableHelper()
{
	if (fWeCreatedIt)
		hsRefCnt_SafeUnRef(fCreatable);
}

plNetMsgCreatableHelper & plNetMsgCreatableHelper::operator =(plCreatable * value)
{
	SetObject(value);
	return *this;
}

plNetMsgCreatableHelper::operator plCreatable*()
{
	return GetObject();
}

plNetMsgCreatableHelper::operator const plCreatable*()
{
	return GetObject();
}

void plNetMsgCreatableHelper::SetObject(plCreatable * object)
{
	if (fWeCreatedIt)
		hsRefCnt_SafeUnRef(fCreatable);
	fCreatable = object;
	fWeCreatedIt = false;
}

plCreatable * plNetMsgCreatableHelper::GetObject()
{
	return fCreatable;
}

int plNetMsgCreatableHelper::Poke(hsStream * s, UInt32 peekOptions)
{
	hsAssert(fCreatable,"plNetMsgCreatableHelper::Poke: fCreatable not set");
	UInt16 classIndex = fCreatable->ClassIndex();
	s->WriteSwap(classIndex);
	fCreatable->Write(s,nil);
	return s->GetPosition();
}

int plNetMsgCreatableHelper::Peek(hsStream * s, UInt32 peekOptions)
{
	UInt16 classIndex;
	s->LogSubStreamStart("push me");
	s->LogReadSwap(&classIndex,"ClassIdx");
	SetObject(plFactory::Create(classIndex));
	fWeCreatedIt = true;
	hsAssert(fCreatable,"plNetMsgCreatableHelper::Peek: Failed to create plCreatable. Invalid ClassIndex?");
	fCreatable->Read(s,nil);
	s->LogSubStreamEnd();
	return s->GetPosition();
}


/////////////////////////////////////////////////////////
// NOT A MSG
// PL STREAM MSG - HELPER class
/////////////////////////////////////////////////////////
plNetMsgStreamHelper::plNetMsgStreamHelper() :	fStreamBuf(nil), fStreamType(-1), fStreamLen(0), 
		fCompressionType(plNetMessage::kCompressionNone), fUncompressedSize(0),
		fCompressionThreshold( kDefaultCompressionThreshold )
{
}

void plNetMsgStreamHelper::Clear()
{
	delete [] fStreamBuf;
	fStreamBuf = nil;
	fStreamType = 0xff;
	fStreamLen = 0;
	fCompressionType = plNetMessage::kCompressionNone;
	fCompressionThreshold = kDefaultCompressionThreshold;
}

int plNetMsgStreamHelper::Poke(hsStream* stream, UInt32 peekOptions)
{
	if ( !(peekOptions & plNetMessage::kDontCompress) )
		Compress();
	stream->WriteSwap(fUncompressedSize);
	stream->WriteSwap(fCompressionType);
	stream->WriteSwap(fStreamLen);
	stream->Write(fStreamLen, fStreamBuf);
	return stream->GetPosition();
}

int plNetMsgStreamHelper::Peek(hsStream* stream, const UInt32 peekOptions)
{
	stream->LogSubStreamStart("Stream Helper");
	stream->LogReadSwap(&fUncompressedSize,"UncompressedSize");
	stream->LogReadSwap(&fCompressionType,"CompressionType");
	stream->LogReadSwap(&fStreamLen,"StreamLen");

	if (fStreamLen)		// stream data exists
	{
		if (!(peekOptions & plNetMessage::kSkipStream))
		{
				if (!fStreamBuf)
					IAllocStream(fStreamLen);
				stream->LogRead(fStreamLen, fStreamBuf,"StreamData");
				if ( !(peekOptions & plNetMessage::kDontCompress) )
					Uncompress();
				fStreamType = *(Int16*)fStreamBuf;		// grab from start fo stream
		}
		else
		{
			stream->ReadSwap(&fStreamType);		// never compressed, set by reading directly from stream
			stream->LogSkip(fStreamLen-sizeof(fStreamType),"SkippedStreamHelper");
		}
	}
	stream->LogSubStreamEnd();
	return stream->GetPosition();
}

void plNetMsgStreamHelper::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kUncompressedSize))
		s->ReadSwap(&fUncompressedSize);
	if (contentFlags.IsBitSet(kCompressionType))
		s->ReadSwap(&fCompressionType);
	if (contentFlags.IsBitSet(kStreamLen))
		s->ReadSwap(&fStreamLen);
	if (contentFlags.IsBitSet(kStreamBuf))
	{
		if (!fStreamBuf)
			IAllocStream(fStreamLen);
		s->Read(fStreamLen,fStreamBuf);
	}
}

void plNetMsgStreamHelper::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kUncompressedSize);
	contentFlags.SetBit(kCompressionType);
	contentFlags.SetBit(kStreamLen);
	contentFlags.SetBit(kStreamBuf);
	contentFlags.Write(s);

	s->WriteSwap(fUncompressedSize);
	s->WriteSwap(fCompressionType);
	s->WriteSwap(fStreamLen);
	s->Write(fStreamLen,fStreamBuf);
}

void plNetMsgStreamHelper::IAllocStream(UInt32 len)
{
	delete [] fStreamBuf;
	fStreamBuf=nil;
	fStreamLen=len;
	if (len)
		fStreamBuf = TRACKED_NEW UInt8[len];
}

void plNetMsgStreamHelper::CopyStream(hsStream* ssStream)
{
	UInt32 len=ssStream->GetEOF();
	IAllocStream(len);
	ssStream->CopyToMem(fStreamBuf);
	fStreamType = *(Int16*)fStreamBuf;
}

void plNetMsgStreamHelper::CopyStream(Int32 len, const void* buf)
{
	IAllocStream(len);
	memcpy(fStreamBuf, buf, len);
	fStreamType = *(Int16*)fStreamBuf;
}

void plNetMsgStreamHelper::CopyFrom(const plNetMsgStreamHelper* other)
{
	fUncompressedSize = other->GetUncompressedSize();
	fCompressionType = other->GetCompressionType();
	CopyStream(other->GetStreamLen(), other->GetStreamBuf());
}

bool plNetMsgStreamHelper::Compress(int offset)
{
	if ( !IsCompressable() )
		return true;

	plZlibCompress compressor;
	UInt8* buf = (UInt8*)GetStreamBuf();	// skip creatable index
	UInt32 bufLen = GetStreamLen();
	UInt32 uncompressedSize = bufLen;
	SetUncompressedSize( uncompressedSize );
	if ( compressor.Compress( &buf, &bufLen, offset) )
	{
		SetCompressionType( plNetMessage::kCompressionZlib );
		SetStreamLen(bufLen);
		SetStreamBuf(buf);
		Int32 diff = uncompressedSize-bufLen;
#if 0
		plNetApp::StaticDebugMsg( "\tCompressed stream: %lu->%lu bytes, (%s %d bytes, %.1f%%)",
			uncompressedSize, bufLen, (diff>=0)?"shrunk":"GREW?!?", diff, (diff/(float)uncompressedSize)*100 );
#endif
		return true;
	}
	else
	{
		hsAssert( false, "plNetMsgStreamHelper: Compression failed" );
		SetCompressionType( plNetMessage::kCompressionFailed );
		return false;
	}
}

bool plNetMsgStreamHelper::Uncompress(int offset)
{
	if ( !IsCompressed() )
		return true;

	UInt32 origLen = GetStreamLen();
	plZlibCompress compressor;
	UInt8* buf = (UInt8*)GetStreamBuf();
	UInt32 bufLen = origLen;
	if ( compressor.Uncompress( &buf, &bufLen, GetUncompressedSize(), offset ) )
	{
		SetCompressionType( plNetMessage::kCompressionNone );
		SetStreamLen(bufLen);
		SetStreamBuf(buf);
		Int32 diff = bufLen-origLen;
#if 0
		plNetApp::StaticDebugMsg( "\tUncompressed stream: %lu->%lu bytes, (%s %d bytes, %.1f%%)",
			origLen, bufLen, (diff>=0)?"grew":"SHRUNK?!?", diff, (diff/(float)bufLen)*100 );
#endif
		return true;
	}
	else
	{
		hsAssert( false, "plNetMsgStreamHelper: Uncompression failed" );
		SetCompressionType( plNetMessage::kCompressionFailed );
		return false;
	}
}

bool plNetMsgStreamHelper::IsCompressed() const
{
	return ( fCompressionType==plNetMessage::kCompressionZlib );
}

bool plNetMsgStreamHelper::IsCompressable() const
{
	return ( fCompressionType==plNetMessage::kCompressionNone
		&& fStreamLen>fCompressionThreshold );
}



////////////////////////////////////////////////////////
// NOT A MSG
// plNetMsgObject - HELPER class
////////////////////////////////////////////////////////

plNetMsgObjectHelper & plNetMsgObjectHelper::operator =(const plNetMsgObjectHelper & other)
{
	fUoid = other.GetUoid();
	return *this;
}

int plNetMsgObjectHelper::Poke(hsStream* stream, UInt32 peekOptions)
{
	fUoid.Write(stream);
	
	return stream->GetPosition();
}

int plNetMsgObjectHelper::Peek(hsStream* stream, const UInt32 peekOptions)
{
	stream->LogSubStreamStart("push me");
	fUoid.Read(stream);
	stream->LogSubStreamEnd();
	return stream->GetPosition();
}

hsBool plNetMsgObjectHelper::SetFromKey(const plKey &key)
{
	if (!key || !key->GetName())
		return false;
	
	fUoid = key->GetUoid();

	return true;
}

void plNetMsgObjectHelper::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kObjHelperUoid))
		fUoid.Read(s);
}

void plNetMsgObjectHelper::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kObjHelperUoid);
	contentFlags.Write(s);

	// kObjHelperUoid
	fUoid.Write(s);
}

////////////////////////////////////////////////////////
// NOT A MSG
// plNetMsgObjectList - HELPER class
////////////////////////////////////////////////////////
plNetMsgObjectListHelper::~plNetMsgObjectListHelper()
{
	Reset();	     
}

void plNetMsgObjectListHelper::Reset()
{
	int i;
	for( i=0 ; i<GetNumObjects() ; i++  )
	{
		delete GetObject(i);
		fObjects[i] = nil;
	} // for	
	fObjects.clear();
}

int plNetMsgObjectListHelper::Poke(hsStream* stream, UInt32 peekOptions)
{
	Int16 num = GetNumObjects();
	stream->WriteSwap(num);
	int i;
	for( i=0 ;i<num  ;i++  )
	{
		GetObject(i)->Poke(stream, peekOptions);
	} // for	     

	return stream->GetPosition();
}

int plNetMsgObjectListHelper::Peek(hsStream* stream, const UInt32 peekOptions)
{
	Reset();

	stream->LogSubStreamStart("push me");
	Int16 num;
	stream->LogReadSwap(&num,"ObjectListHelper Num");

	int i;
	for( i=0 ;i<num  ;i++  )
	{
		fObjects.push_back(TRACKED_NEW plNetMsgObjectHelper);
		GetObject(i)->Peek(stream, peekOptions);
	} // for	     
	stream->LogSubStreamEnd();
	return stream->GetPosition();
}

////////////////////////////////////////////////////////
// NOT A MSG
// plNetMsgMemberInfoHelper - HELPER class
////////////////////////////////////////////////////////
plNetMsgMemberInfoHelper::plNetMsgMemberInfoHelper()
: fFlags(0)
{
}

int plNetMsgMemberInfoHelper::Peek(hsStream* s, const UInt32 peekOptions)
{
	s->LogSubStreamStart("push me");
	s->LogReadSwap(&fFlags,"MemberInfoHelper Flags");
	fClientGuid.Read( s, nil );
	fAvatarUoid.Read(s);
	s->LogSubStreamEnd();
	return s->GetPosition();
}

int plNetMsgMemberInfoHelper::Poke(hsStream* s, const UInt32 peekOptions)
{
	s->WriteSwap(fFlags);
	fClientGuid.Write( s, nil );
	fAvatarUoid.Write(s);
	return s->GetPosition();
}

////////////////////////////////////////////////////////
// NOT A MSG
// plNetMsgMemberListHelper - HELPER class
////////////////////////////////////////////////////////
plNetMsgMemberListHelper::~plNetMsgMemberListHelper()
{
	int i;
	for(i=0;i<GetNumMembers();i++)
		delete fMembers[i];
}

int plNetMsgMemberListHelper::Peek(hsStream* stream, const UInt32 peekOptions)
{
	Int16 numMembers;
	stream->LogSubStreamStart("push me");
	stream->LogReadSwap(&numMembers,"MemberListHelper NumMembers");
	fMembers.clear();
	int i;
	for(i=0;i<numMembers;i++)
	{
		plNetMsgMemberInfoHelper* addr=TRACKED_NEW plNetMsgMemberInfoHelper;
		addr->Peek(stream, peekOptions);
		AddMember(addr);
	}
	stream->LogSubStreamEnd();	
	return stream->GetPosition();
}

int plNetMsgMemberListHelper::Poke(hsStream* stream, const UInt32 peekOptions)
{
	Int16 numMembers = (Int16)GetNumMembers();
	stream->WriteSwap(numMembers);

	int i;
	for(i=0;i<numMembers;i++)
	{
		fMembers[i]->GetClientGuid()->SetClientKey("");
		fMembers[i]->GetClientGuid()->SetAccountUUID(plUUID());
		fMembers[i]->Poke(stream, peekOptions);
	}

	return stream->GetPosition();
}


////////////////////////////////////////////////////////
// NOT A MSG
// plNetMsgReceiversListHelper - HELPER class
////////////////////////////////////////////////////////

int plNetMsgReceiversListHelper::Peek(hsStream* stream, const UInt32 peekOptions)
{
	UInt8 numIDs;
	stream->LogSubStreamStart("push me");
	stream->LogReadSwap(&numIDs,"ReceiversListHelper NumIDs");
	
	fPlayerIDList.clear();
	int i;
	for(i=0;i<numIDs;i++)
	{
		UInt32 ID;
		stream->LogReadSwap(&ID,"ReceiversListHelper ID");		
		AddReceiverPlayerID(ID);
	}
	stream->LogSubStreamEnd();	
	return stream->GetPosition();
}

int plNetMsgReceiversListHelper::Poke(hsStream* stream, const UInt32 peekOptions)
{
	UInt8 numIDs = (UInt8)GetNumReceivers();
	stream->WriteSwap(numIDs);

	int i;
	for(i=0;i<numIDs;i++)
		stream->WriteSwap(GetReceiverPlayerID(i));

	return stream->GetPosition();
}


bool plNetMsgReceiversListHelper::RemoveReceiverPlayerID(UInt32 n)
{
	std::vector<UInt32>::iterator res = std::find(fPlayerIDList.begin(), fPlayerIDList.end(), n);
	if (res != fPlayerIDList.end())
	{
		fPlayerIDList.erase(res);
		return true;
	}
	return false;
}

