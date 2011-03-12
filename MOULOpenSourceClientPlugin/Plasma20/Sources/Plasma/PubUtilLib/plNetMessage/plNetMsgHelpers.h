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
#ifndef PL_NET_MSG_HELPERS_inc
#define PL_NET_MSG_HELPERS_inc

//
// These are not messages per se, but helper classes which are used 
// in to avoid multiple derivation by net messages.
//

#include "hsTypes.h"
#include "hsUtils.h"
#include "hsStream.h"
#include "hsStlUtils.h"
#include "../pnNetCommon/pnNetCommon.h"
#include "../pnFactory/plCreatable.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plNetCommon/plClientGuid.h"
#include <algorithm>

class plKey;
class hsStream;


////////////////////////////////////////////////////////////////////
// plNetMsgStreamableHelper
// Will peek/poke anything derived from hsStreamable

class plNetMsgStreamableHelper : public plCreatable
{
	hsStreamable *	fObject;
public:
	plNetMsgStreamableHelper():fObject(nil){}
	plNetMsgStreamableHelper(hsStreamable * object):fObject(object){}
	plNetMsgStreamableHelper & operator =(hsStreamable * value);
	operator hsStreamable *() const { return fObject;}
	operator const hsStreamable *() const { return fObject;}
	CLASSNAME_REGISTER( plNetMsgStreamableHelper );
	GETINTERFACE_ANY(plNetMsgStreamableHelper, plCreatable);
	void SetObject(hsStreamable * object) { fObject=object;}
	hsStreamable * GetObject() const { return fObject;}
	int Poke(hsStream* stream, UInt32 peekOptions=0);
	int Peek(hsStream* stream, UInt32 peekOptions=0);
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
	plCreatable *	fCreatable;
	bool			fWeCreatedIt;
public:
	plNetMsgCreatableHelper(plCreatable * object = nil);
	~plNetMsgCreatableHelper();
	plNetMsgCreatableHelper & operator =(plCreatable * value);
	operator plCreatable*();
	operator const plCreatable*();
	CLASSNAME_REGISTER( plNetMsgCreatableHelper );
	GETINTERFACE_ANY(plNetMsgCreatableHelper, plCreatable);
	void SetObject(plCreatable * object);
	plCreatable * GetObject();
	int Poke(hsStream* stream, UInt32 peekOptions=0);
	int Peek(hsStream* stream, UInt32 peekOptions=0);
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
	UInt32	fUncompressedSize;
	Int16	fStreamType;	// set to creatable type, not read/written, gleaned from creatable stream
	UInt8*	fStreamBuf;
	UInt32	fStreamLen;
	UInt8	fCompressionType;	// see plNetMessage::CompressionType
	UInt32	fCompressionThreshold;  // NOT WRITTEN

	void IAllocStream(UInt32 len);

public:
	enum { kDefaultCompressionThreshold	= 255 }; // bytes

	plNetMsgStreamHelper();
	virtual ~plNetMsgStreamHelper() { delete [] fStreamBuf; }

	CLASSNAME_REGISTER( plNetMsgStreamHelper );
	GETINTERFACE_ANY(plNetMsgStreamHelper, plCreatable);

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);	
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);	

	// creatable ops
	virtual void Read(hsStream* s, hsResMgr* mgr) { Peek(s); }
	virtual void Write(hsStream* s, hsResMgr* mgr) { Poke(s); }
	
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
	
	void Clear();

	// copiers
	void CopyFrom(const plNetMsgStreamHelper* other);
	void CopyStream(hsStream* ssStream);			// copies to fStream
	void CopyStream(Int32 len, const void* buf);	// copies to fStream

	// setters
	void SetCompressionType(UInt8 t) { fCompressionType=t; }
	void SetStreamLen(UInt32 l) { fStreamLen=l; }
	void SetStreamBuf(UInt8* b) { fStreamBuf=b; }
	void SetUncompressedSize(UInt32 s) { fUncompressedSize=s; }

	// Getters
	UInt8 GetCompressionType() const { return fCompressionType; }
	Int16 GetStreamType() const { return fStreamType; }
	UInt32 GetStreamLen() const { return fStreamLen; }
	UInt8* GetStreamBuf() const { return fStreamBuf; }
	UInt32 GetUncompressedSize() const { return fUncompressedSize; }

	bool	Compress(int offset=2 /* skip 2 bytes as creatable index */ );
	bool	Uncompress(int offset=2 /* skip 2 bytes as creatable index */ );
	bool	IsCompressed() const;
	bool	IsCompressable() const;
	UInt32	GetCompressionThreshold() const { return fCompressionThreshold; }
	void	SetCompressionThreshold( UInt32 v ) { fCompressionThreshold=v; }
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
	plUoid	fUoid;
	// update operator()= fxn when adding new members
public:

	plNetMsgObjectHelper() {}
	plNetMsgObjectHelper(const plKey key) { SetFromKey(key); }
	virtual ~plNetMsgObjectHelper() { }
	CLASSNAME_REGISTER( plNetMsgObjectHelper );
	GETINTERFACE_ANY(plNetMsgObjectHelper, plCreatable);

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);	
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);

	plNetMsgObjectHelper & operator =(const plNetMsgObjectHelper & other);
	
	// setters
	hsBool SetFromKey(const plKey &key);
	void SetUoid(const plUoid &u) { fUoid=u; }
	
	// getters
	const char* GetObjectName() const { return fUoid.GetObjectName(); }
	UInt32		GetPageID() const { return fUoid.GetLocation().GetSequenceNumber(); }
	const plUoid& GetUoid() const { return fUoid; }
	
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
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

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);	
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);

	void Reset();
	int GetNumObjects() const { return fObjects.size(); }
	plNetMsgObjectHelper* GetObject(int i) { return fObjects[i]; }
	void AddObject(plKey key) { fObjects.push_back(TRACKED_NEW plNetMsgObjectHelper(key)); }
};

//
// Contains a info about a net member.
//
class plNetMsgMemberInfoHelper : public plCreatable
{
protected:
	UInt32 fFlags;
	plUoid fAvatarUoid;
	plClientGuid	fClientGuid;
public:	
	plNetMsgMemberInfoHelper();

	CLASSNAME_REGISTER( plNetMsgMemberInfoHelper );
	GETINTERFACE_ANY( plNetMsgMemberInfoHelper, plCreatable);

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);

	const plClientGuid * GetClientGuid() const { return &fClientGuid; }
	plClientGuid * GetClientGuid() { return &fClientGuid; }

	UInt32 GetFlags() const { return fFlags; }
	plUoid GetAvatarUoid() const { return fAvatarUoid; }

	void SetFlags(UInt32 v) { fFlags=v; }	
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
		UInt32 fID;
		MatchesPlayerID( UInt32 id ): fID( id ){}
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

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);

	int GetNumMembers() const { return fMembers.size(); }
	const plNetMsgMemberInfoHelper* GetMember(int i) const { return fMembers[i]; }
	void AddMember(plNetMsgMemberInfoHelper* a) { fMembers.push_back(a); }
	const MemberInfoHelperVec * GetMembers() const { return &fMembers;}
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
	std::vector<UInt32> fPlayerIDList;
public:
	plNetMsgReceiversListHelper() {}
	virtual ~plNetMsgReceiversListHelper() {}

	CLASSNAME_REGISTER( plNetMsgReceiversListHelper );
	GETINTERFACE_ANY( plNetMsgReceiversListHelper, plCreatable);

	virtual int Poke(hsStream* stream, UInt32 peekOptions=0);
	virtual int Peek(hsStream* stream, UInt32 peekOptions=0);

	void Clear() { fPlayerIDList.clear();	}
	int GetNumReceivers() const { return fPlayerIDList.size(); }
	UInt32 GetReceiverPlayerID(int i) const { return fPlayerIDList[i]; }
	void AddReceiverPlayerID(UInt32 a) { fPlayerIDList.push_back(a); }
	bool RemoveReceiverPlayerID(UInt32 n);	// returns true if found and removed
};


#endif	// PL_NET_MSG__HELPERS_inc

