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
#include "plNetMsgHelpers.h"
#include "plNetMessage.h"
#include "plCompression/plZlibCompress.h"
#include "pnNetCommon/plNetServers.h"
#include "pnNetCommon/plNetApp.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plMessage.h"
#include "hsStream.h"
#include <algorithm>


////////////////////////////////////////////////////////////////////
// plNetMsgStreamableHelper

plNetMsgStreamableHelper & plNetMsgStreamableHelper::operator =(hsStreamable * value)
{
    fObject = value;
    return *this;
}

int plNetMsgStreamableHelper::Poke(hsStream* stream, uint32_t peekOptions)
{
    hsAssert(fObject, "plNetMsgStreamableHelper::Poke: fObject not set.");
    fObject->Write(stream);
    return stream->GetPosition();
}

int plNetMsgStreamableHelper::Peek(hsStream* stream, uint32_t peekOptions)
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

int plNetMsgCreatableHelper::Poke(hsStream * s, uint32_t peekOptions)
{
    hsAssert(fCreatable,"plNetMsgCreatableHelper::Poke: fCreatable not set");
    uint16_t classIndex = fCreatable->ClassIndex();
    s->WriteLE(classIndex);
    fCreatable->Write(s,nil);
    return s->GetPosition();
}

int plNetMsgCreatableHelper::Peek(hsStream * s, uint32_t peekOptions)
{
    uint16_t classIndex;
    s->LogSubStreamStart("push me");
    s->LogReadLE(&classIndex,"ClassIdx");
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
plNetMsgStreamHelper::plNetMsgStreamHelper() :  fStreamBuf(nil), fStreamType(-1), fStreamLen(0), 
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

int plNetMsgStreamHelper::Poke(hsStream* stream, uint32_t peekOptions)
{
    if ( !(peekOptions & plNetMessage::kDontCompress) )
        Compress();
    stream->WriteLE(fUncompressedSize);
    stream->WriteLE(fCompressionType);
    stream->WriteLE(fStreamLen);
    stream->Write(fStreamLen, fStreamBuf);
    return stream->GetPosition();
}

int plNetMsgStreamHelper::Peek(hsStream* stream, const uint32_t peekOptions)
{
    stream->LogSubStreamStart("Stream Helper");
    stream->LogReadLE(&fUncompressedSize,"UncompressedSize");
    stream->LogReadLE(&fCompressionType,"CompressionType");
    stream->LogReadLE(&fStreamLen,"StreamLen");

    if (fStreamLen)     // stream data exists
    {
        if (!(peekOptions & plNetMessage::kSkipStream))
        {
                if (!fStreamBuf)
                    IAllocStream(fStreamLen);
                stream->LogRead(fStreamLen, fStreamBuf,"StreamData");
                if ( !(peekOptions & plNetMessage::kDontCompress) )
                    Uncompress();
                fStreamType = *(int16_t*)fStreamBuf;      // grab from start fo stream
        }
        else
        {
            stream->ReadLE(&fStreamType);     // never compressed, set by reading directly from stream
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
        s->ReadLE(&fUncompressedSize);
    if (contentFlags.IsBitSet(kCompressionType))
        s->ReadLE(&fCompressionType);
    if (contentFlags.IsBitSet(kStreamLen))
        s->ReadLE(&fStreamLen);
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

    s->WriteLE(fUncompressedSize);
    s->WriteLE(fCompressionType);
    s->WriteLE(fStreamLen);
    s->Write(fStreamLen,fStreamBuf);
}

void plNetMsgStreamHelper::IAllocStream(uint32_t len)
{
    delete [] fStreamBuf;
    fStreamBuf=nil;
    fStreamLen=len;
    if (len)
        fStreamBuf = new uint8_t[len];
}

void plNetMsgStreamHelper::CopyStream(hsStream* ssStream)
{
    uint32_t len=ssStream->GetEOF();
    IAllocStream(len);
    ssStream->CopyToMem(fStreamBuf);
    fStreamType = *(int16_t*)fStreamBuf;
}

void plNetMsgStreamHelper::CopyStream(int32_t len, const void* buf)
{
    IAllocStream(len);
    memcpy(fStreamBuf, buf, len);
    fStreamType = *(int16_t*)fStreamBuf;
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
    uint8_t* buf = (uint8_t*)GetStreamBuf();    // skip creatable index
    uint32_t bufLen = GetStreamLen();
    uint32_t uncompressedSize = bufLen;
    SetUncompressedSize( uncompressedSize );
    if ( compressor.Compress( &buf, &bufLen, offset) )
    {
        SetCompressionType( plNetMessage::kCompressionZlib );
        SetStreamLen(bufLen);
        SetStreamBuf(buf);
        int32_t diff = uncompressedSize-bufLen;
#if 0
        plNetApp::StaticDebugMsg( "\tCompressed stream: {}->{} bytes, ({} {} bytes, {.1f}%)",
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

    uint32_t origLen = GetStreamLen();
    plZlibCompress compressor;
    uint8_t* buf = (uint8_t*)GetStreamBuf();
    uint32_t bufLen = origLen;
    if ( compressor.Uncompress( &buf, &bufLen, GetUncompressedSize(), offset ) )
    {
        SetCompressionType( plNetMessage::kCompressionNone );
        SetStreamLen(bufLen);
        SetStreamBuf(buf);
        int32_t diff = bufLen-origLen;
#if 0
        plNetApp::StaticDebugMsg( "\tUncompressed stream: {}->{} bytes, ({} {} bytes, {.1f}%)",
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

int plNetMsgObjectHelper::Poke(hsStream* stream, uint32_t peekOptions)
{
    fUoid.Write(stream);
    
    return stream->GetPosition();
}

int plNetMsgObjectHelper::Peek(hsStream* stream, const uint32_t peekOptions)
{
    stream->LogSubStreamStart("push me");
    fUoid.Read(stream);
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}

bool plNetMsgObjectHelper::SetFromKey(const plKey &key)
{
    if (!key || key->GetName().empty())
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

int plNetMsgObjectListHelper::Poke(hsStream* stream, uint32_t peekOptions)
{
    int16_t num = GetNumObjects();
    stream->WriteLE(num);
    int i;
    for( i=0 ;i<num  ;i++  )
    {
        GetObject(i)->Poke(stream, peekOptions);
    } // for         

    return stream->GetPosition();
}

int plNetMsgObjectListHelper::Peek(hsStream* stream, const uint32_t peekOptions)
{
    Reset();

    stream->LogSubStreamStart("push me");
    int16_t num;
    stream->LogReadLE(&num,"ObjectListHelper Num");

    int i;
    for( i=0 ;i<num  ;i++  )
    {
        fObjects.push_back(new plNetMsgObjectHelper);
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

int plNetMsgMemberInfoHelper::Peek(hsStream* s, const uint32_t peekOptions)
{
    s->LogSubStreamStart("push me");
    s->LogReadLE(&fFlags,"MemberInfoHelper Flags");
    fClientGuid.Read( s, nil );
    fAvatarUoid.Read(s);
    s->LogSubStreamEnd();
    return s->GetPosition();
}

int plNetMsgMemberInfoHelper::Poke(hsStream* s, const uint32_t peekOptions)
{
    s->WriteLE(fFlags);
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

int plNetMsgMemberListHelper::Peek(hsStream* stream, const uint32_t peekOptions)
{
    int16_t numMembers;
    stream->LogSubStreamStart("push me");
    stream->LogReadLE(&numMembers,"MemberListHelper NumMembers");
    fMembers.clear();
    int i;
    for(i=0;i<numMembers;i++)
    {
        plNetMsgMemberInfoHelper* addr=new plNetMsgMemberInfoHelper;
        addr->Peek(stream, peekOptions);
        AddMember(addr);
    }
    stream->LogSubStreamEnd();  
    return stream->GetPosition();
}

int plNetMsgMemberListHelper::Poke(hsStream* stream, const uint32_t peekOptions)
{
    int16_t numMembers = (int16_t)GetNumMembers();
    stream->WriteLE(numMembers);

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

int plNetMsgReceiversListHelper::Peek(hsStream* stream, const uint32_t peekOptions)
{
    uint8_t numIDs;
    stream->LogSubStreamStart("push me");
    stream->LogReadLE(&numIDs,"ReceiversListHelper NumIDs");
    
    fPlayerIDList.clear();
    int i;
    for(i=0;i<numIDs;i++)
    {
        uint32_t ID;
        stream->LogReadLE(&ID,"ReceiversListHelper ID");      
        AddReceiverPlayerID(ID);
    }
    stream->LogSubStreamEnd();  
    return stream->GetPosition();
}

int plNetMsgReceiversListHelper::Poke(hsStream* stream, const uint32_t peekOptions)
{
    uint8_t numIDs = (uint8_t)GetNumReceivers();
    stream->WriteLE(numIDs);

    int i;
    for(i=0;i<numIDs;i++)
        stream->WriteLE(GetReceiverPlayerID(i));

    return stream->GetPosition();
}


bool plNetMsgReceiversListHelper::RemoveReceiverPlayerID(uint32_t n)
{
    std::vector<uint32_t>::iterator res = std::find(fPlayerIDList.begin(), fPlayerIDList.end(), n);
    if (res != fPlayerIDList.end())
    {
        fPlayerIDList.erase(res);
        return true;
    }
    return false;
}

