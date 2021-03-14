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
#ifndef PL_NET_MSG_HELPERS_inc
#define PL_NET_MSG_HELPERS_inc

//
// These are not messages per se, but helper classes which are used 
// in to avoid multiple derivation by net messages.
//

#include "HeadSpin.h"

#include <string_theory/string>
#include <vector>

#include "pnFactory/plCreatable.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"

#include "plNetCommon/plClientGuid.h"

class plKey;
class hsStream;
class hsStreamable;


////////////////////////////////////////////////////////////////////
// plNetMsgStreamableHelper
// Will peek/poke anything derived from hsStreamable

class plNetMsgStreamableHelper : public plCreatable
{
    hsStreamable *  fObject;
public:
    plNetMsgStreamableHelper() : fObject() { }
    plNetMsgStreamableHelper(hsStreamable * object):fObject(object){}
    plNetMsgStreamableHelper & operator =(hsStreamable * value);
    operator hsStreamable *() const { return fObject;}
    operator const hsStreamable *() const { return fObject;}
    CLASSNAME_REGISTER( plNetMsgStreamableHelper );
    GETINTERFACE_ANY(plNetMsgStreamableHelper, plCreatable);
    void SetObject(hsStreamable * object) { fObject=object;}
    hsStreamable * GetObject() const { return fObject;}
    int Poke(hsStream* stream, uint32_t peekOptions=0);
    int Peek(hsStream* stream, uint32_t peekOptions=0);
};

////////////////////////////////////////////////////////////////////
// plNetMsgCreatableHelper
// Will peek/poke anything derived from plCreatable.
// Will create the object upon read if it hasn't been set by SetObject().
// When helper goes away, object will be unref-ed if it was created by
// the helper, so if you GetObject() and want to keep it longer than the
// lifetime of the helper, ref it.

class plNetMsgCreatableHelper : public plCreatable
{
    plCreatable *   fCreatable;
    bool            fWeCreatedIt;
public:
    plNetMsgCreatableHelper(plCreatable * object = nullptr);
    ~plNetMsgCreatableHelper();
    plNetMsgCreatableHelper & operator =(plCreatable * value);
    operator plCreatable*();
    operator const plCreatable*();
    CLASSNAME_REGISTER( plNetMsgCreatableHelper );
    GETINTERFACE_ANY(plNetMsgCreatableHelper, plCreatable);
    void SetObject(plCreatable * object);
    plCreatable * GetObject();
    int Poke(hsStream* stream, uint32_t peekOptions=0);
    int Peek(hsStream* stream, uint32_t peekOptions=0);
};

////////////////////////////////////////////////////////////////////
//
// Net msg helper class for a stream buffer of some type (saveState, voice, plMessage...)
//
class plNetMsgStreamHelper : public plCreatable
{
private:
    enum ContentsFlags
    {
        kUncompressedSize,
        kStreamBuf,
        kStreamLen,
        kCompressionType,
    };
protected:
    uint32_t  fUncompressedSize;
    int16_t   fStreamType;    // set to creatable type, not read/written, gleaned from creatable stream
    uint8_t*  fStreamBuf;
    uint32_t  fStreamLen;
    uint8_t   fCompressionType;   // see plNetMessage::CompressionType
    uint32_t  fCompressionThreshold;  // NOT WRITTEN

    void IAllocStream(uint32_t len);

public:
    enum { kDefaultCompressionThreshold = 255 }; // bytes

    plNetMsgStreamHelper();
    virtual ~plNetMsgStreamHelper() { delete [] fStreamBuf; }

    CLASSNAME_REGISTER( plNetMsgStreamHelper );
    GETINTERFACE_ANY(plNetMsgStreamHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);   
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);   

    // creatable ops
    void Read(hsStream* s, hsResMgr* mgr) override { Peek(s); }
    void Write(hsStream* s, hsResMgr* mgr) override { Poke(s); }
    
    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
    
    void Clear();

    // copiers
    void CopyFrom(const plNetMsgStreamHelper* other);
    void CopyStream(hsStream* ssStream);            // copies to fStream
    void CopyStream(int32_t len, const void* buf);    // copies to fStream

    // setters
    void SetCompressionType(uint8_t t) { fCompressionType=t; }
    void SetStreamLen(uint32_t l) { fStreamLen=l; }
    void SetStreamBuf(uint8_t* b) { fStreamBuf=b; }
    void SetUncompressedSize(uint32_t s) { fUncompressedSize=s; }

    // Getters
    uint8_t GetCompressionType() const { return fCompressionType; }
    int16_t GetStreamType() const { return fStreamType; }
    uint32_t GetStreamLen() const { return fStreamLen; }
    uint8_t* GetStreamBuf() const { return fStreamBuf; }
    uint32_t GetUncompressedSize() const { return fUncompressedSize; }

    bool    Compress(int offset=2 /* skip 2 bytes as creatable index */ );
    bool    Uncompress(int offset=2 /* skip 2 bytes as creatable index */ );
    bool    IsCompressed() const;
    bool    IsCompressable() const;
    uint32_t  GetCompressionThreshold() const { return fCompressionThreshold; }
    void    SetCompressionThreshold( uint32_t v ) { fCompressionThreshold=v; }
};

//
// Contains info about a scene object
//
class plNetMsgObjectHelper : public plCreatable
{
private:
    enum ContentFlags
    {
        kObjHelperUoid,
    };
protected:
    // string names for debug purposes only
    plUoid  fUoid;
    // update operator()= fxn when adding new members
public:

    plNetMsgObjectHelper() {}
    plNetMsgObjectHelper(const plKey key) { SetFromKey(key); }
    virtual ~plNetMsgObjectHelper() { }
    CLASSNAME_REGISTER( plNetMsgObjectHelper );
    GETINTERFACE_ANY(plNetMsgObjectHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);   
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);

    plNetMsgObjectHelper & operator =(const plNetMsgObjectHelper & other);
    
    // setters
    bool SetFromKey(const plKey &key);
    void SetUoid(const plUoid &u) { fUoid=u; }
    
    // getters
    ST::string    GetObjectName() const { return fUoid.GetObjectName(); }
    uint32_t      GetPageID() const { return fUoid.GetLocation().GetSequenceNumber(); }
    const plUoid& GetUoid() const { return fUoid; }
    
    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};

//
// Contains a list of info about scene objects.
//
class plNetMsgObjectListHelper : public plCreatable
{
protected:
    std::vector<plNetMsgObjectHelper*> fObjects;
public:
    plNetMsgObjectListHelper() {}
    virtual ~plNetMsgObjectListHelper();

    CLASSNAME_REGISTER( plNetMsgObjectListHelper );
    GETINTERFACE_ANY(plNetMsgObjectListHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);

    void Reset();
    size_t GetNumObjects() const { return fObjects.size(); }
    plNetMsgObjectHelper* GetObject(size_t i) { return fObjects[i]; }
    void AddObject(plKey key) { fObjects.push_back(new plNetMsgObjectHelper(key)); }
};

//
// Contains a info about a net member.
//
class plNetMsgMemberInfoHelper : public plCreatable
{
protected:
    uint32_t fFlags;
    plUoid fAvatarUoid;
    plClientGuid    fClientGuid;
public: 
    plNetMsgMemberInfoHelper();

    CLASSNAME_REGISTER( plNetMsgMemberInfoHelper );
    GETINTERFACE_ANY( plNetMsgMemberInfoHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);

    const plClientGuid * GetClientGuid() const { return &fClientGuid; }
    plClientGuid * GetClientGuid() { return &fClientGuid; }

    uint32_t GetFlags() const { return fFlags; }
    plUoid GetAvatarUoid() const { return fAvatarUoid; }

    void SetFlags(uint32_t v) { fFlags=v; }   
    void SetAvatarUoid(plUoid u) { fAvatarUoid=u; }
};

//
// Contains a info about a list of net members.
// This is sent from server to client.
//
class plNetMsgMemberListHelper : public plCreatable
{
public:
    typedef std::vector<plNetMsgMemberInfoHelper*> MemberInfoHelperVec;
    struct MatchesPlayerID
    {
        uint32_t fID;
        MatchesPlayerID( uint32_t id ): fID( id ){}
        bool operator()( const plNetMsgMemberInfoHelper * mbr ) const
        {
            return ( mbr && mbr->GetClientGuid()->GetPlayerID()==fID );
        }
    };

protected:
    MemberInfoHelperVec fMembers;

public:
    plNetMsgMemberListHelper() {}
    virtual ~plNetMsgMemberListHelper();

    CLASSNAME_REGISTER( plNetMsgMemberListHelper );
    GETINTERFACE_ANY( plNetMsgMemberListHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);

    size_t GetNumMembers() const { return fMembers.size(); }
    const plNetMsgMemberInfoHelper* GetMember(size_t i) const { return fMembers[i]; }
    void AddMember(plNetMsgMemberInfoHelper* a) { fMembers.push_back(a); }
    const MemberInfoHelperVec* GetMembers() const { return &fMembers;}
};


/////////////////////////////////////////////////////////////////

//
// Contains a list of other players (members).
// This is commonly used to route p2p msgs to groups of players.
// Sent client to server.
//
class plNetMsgReceiversListHelper : public plCreatable
{
protected:
    std::vector<uint32_t> fPlayerIDList;
public:
    plNetMsgReceiversListHelper() {}
    virtual ~plNetMsgReceiversListHelper() {}

    CLASSNAME_REGISTER( plNetMsgReceiversListHelper );
    GETINTERFACE_ANY( plNetMsgReceiversListHelper, plCreatable);

    virtual int Poke(hsStream* stream, uint32_t peekOptions=0);
    virtual int Peek(hsStream* stream, uint32_t peekOptions=0);

    void Clear() { fPlayerIDList.clear();   }
    size_t GetNumReceivers() const { return fPlayerIDList.size(); }
    uint32_t GetReceiverPlayerID(size_t i) const { return fPlayerIDList[i]; }
    void AddReceiverPlayerID(uint32_t a) { fPlayerIDList.push_back(a); }
    bool RemoveReceiverPlayerID(uint32_t n);  // returns true if found and removed
};


#endif  // PL_NET_MSG__HELPERS_inc

