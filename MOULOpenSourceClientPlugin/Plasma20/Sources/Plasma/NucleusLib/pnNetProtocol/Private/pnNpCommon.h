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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPCOMMON_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPCOMMON_H


/*****************************************************************************
*
*   Client message field types
*
***/

#ifdef USES_NETCLI

const NetMsgField kNetMsgFieldAccountName   = NET_MSG_FIELD_STRING(kMaxAccountNameLength);
const NetMsgField kNetMsgFieldPlayerName    = NET_MSG_FIELD_STRING(kMaxPlayerNameLength);
const NetMsgField kNetMsgFieldShaDigest     = NET_MSG_FIELD_RAW_DATA(sizeof(ShaDigest));
const NetMsgField kNetMsgFieldUuid          = NET_MSG_FIELD_DATA(sizeof(Uuid));
const NetMsgField kNetMsgFieldTransId       = NET_MSG_FIELD_DWORD();
const NetMsgField kNetMsgFieldTimeMs        = NET_MSG_FIELD_DWORD();
const NetMsgField kNetMsgFieldENetError     = NET_MSG_FIELD_DWORD();
const NetMsgField kNetMsgFieldEAgeId        = NET_MSG_FIELD_DWORD();
const NetMsgField kNetMsgFieldNetNode       = NET_MSG_FIELD_DWORD();
const NetMsgField kNetMsgFieldBuildId       = NET_MSG_FIELD_DWORD();

#endif


/*****************************************************************************
*
*   Player information structures
*
***/

#include <PshPack1.h>
struct SrvPlayerInfo {
	unsigned	playerInt;
	wchar		playerName[kMaxPlayerNameLength];
	wchar		avatarShape[kMaxVaultNodeStringLength];
	unsigned	explorer;
};
#include <PopPack.h>


/*****************************************************************************
*
*   NetAgeInfo
*
***/

struct NetAgeInfo {
	Uuid		ageInstId;
	wchar		ageFilename[kMaxAgeNameLength];
	wchar		ageInstName[kMaxAgeNameLength];
	wchar		ageUserName[kMaxAgeNameLength];
	wchar		ageDesc[1024];
	dword		ageSequenceNumber;
	dword		ageLanguage;
	dword		population;			// only used with GetPublicAgeList query results
	dword		currentPopulation;	// only used with GetPublicAgeList query results
};

/*****************************************************************************
*
*   NetGameScore
*
***/

struct NetGameScore {
	unsigned	scoreId;
	unsigned	ownerId;
	UInt32		createdTime;
	wchar		gameName[kMaxGameScoreNameLength];
	unsigned	gameType;
	int			value;

	unsigned Read (const byte inbuffer[], unsigned bufsz, byte** end = nil);	// returns number of bytes read
	unsigned Write (ARRAY(byte) * buffer) const;								// returns number of bytes written

	void CopyFrom (const NetGameScore & score);
};

/*****************************************************************************
*
*   NetGameRank
*
***/

struct NetGameRank {
	unsigned	rank;
	int			score;
	wchar		name[kMaxPlayerNameLength];

	unsigned Read (const byte inbuffer[], unsigned bufsz, byte** end = nil);	// returns number of bytes read
	unsigned Write (ARRAY(byte) * buffer) const;								// returns number of bytes written

	void CopyFrom (const NetGameRank & fromRank);
};

/*****************************************************************************
*
*   Server vault structures
*
***/

//============================================================================
// NetVaultNode
//============================================================================
// Threaded apps: App is responsible for locking node->critsect before accessing *any* field in this struct
struct NetVaultNode : AtomicRef {
	enum RwOptions {
		kRwDirtyOnly	= 1<<0,	// READ : No meaning
								// WRITE: Only write fields marked dirty
		kRwUpdateDirty	= 1<<1,	// READ : Set dirty flag on fields read from stream
								// WRITE: Clear dirty flag on fields written to stream
	};
	
	enum CopyOptions {
		kCopySetDirty	= 1<<0,		// set dirty flag on changed dst fields
		kCopyOverwrite	= 1<<1,		// overwrite fields for which dst node already has values
		kCopyClear		= 1<<2,		// clear dst fields for which src node does not have values
	};
	
	// These flag values must not change unless all servers are
	// simultaneously replaced, so basically forget it.
	static const qword kNodeId			= (qword)1<< 0;
	static const qword kCreateTime		= (qword)1<< 1;
	static const qword kModifyTime		= (qword)1<< 2;
	static const qword kCreateAgeName	= (qword)1<< 3;
	static const qword kCreateAgeUuid	= (qword)1<< 4;
	static const qword kCreatorAcct		= (qword)1<< 5;
	static const qword kCreatorId		= (qword)1<< 6;
	static const qword kNodeType		= (qword)1<< 7;
	static const qword kInt32_1			= (qword)1<< 8;
	static const qword kInt32_2			= (qword)1<< 9;
	static const qword kInt32_3			= (qword)1<<10;
	static const qword kInt32_4			= (qword)1<<11;
	static const qword kUInt32_1		= (qword)1<<12;
	static const qword kUInt32_2		= (qword)1<<13;
	static const qword kUInt32_3		= (qword)1<<14;
	static const qword kUInt32_4		= (qword)1<<15;
	static const qword kUuid_1			= (qword)1<<16;
	static const qword kUuid_2			= (qword)1<<17;
	static const qword kUuid_3			= (qword)1<<18;
	static const qword kUuid_4			= (qword)1<<19;
	static const qword kString64_1		= (qword)1<<20;
	static const qword kString64_2		= (qword)1<<21;
	static const qword kString64_3		= (qword)1<<22;
	static const qword kString64_4		= (qword)1<<23;
	static const qword kString64_5		= (qword)1<<24;
	static const qword kString64_6		= (qword)1<<25;
	static const qword kIString64_1		= (qword)1<<26;
	static const qword kIString64_2		= (qword)1<<27;
	// blobs always come last
	static const qword kText_1			= (qword)1<<28;
	static const qword kText_2			= (qword)1<<29;
	static const qword kBlob_1			= (qword)1<<30;
	static const qword kBlob_2			= (qword)1<<31;
	
	CCritSect	critsect;
	
	qword		fieldFlags;
	qword		dirtyFlags;
	
	Uuid		revisionId;

	// Treat these as read-only or node flag fields will become invalid	
	// Threaded apps: Must be accessed with node->critsect locked	
	unsigned	nodeId;
	unsigned	createTime;
	unsigned	modifyTime;
	wchar *		createAgeName;
	Uuid		createAgeUuid;
	Uuid		creatorAcct;	// accountId of node creator
	unsigned	creatorId;		// playerId of node creator
	unsigned	nodeType;
	int			int32_1;
	int			int32_2;
	int			int32_3;
	int			int32_4;
	unsigned	uint32_1;
	unsigned	uint32_2;
	unsigned	uint32_3;
	unsigned	uint32_4;
	Uuid		uuid_1;
	Uuid		uuid_2;
	Uuid		uuid_3;
	Uuid		uuid_4;
	wchar *		string64_1;
	wchar *		string64_2;
	wchar *		string64_3;
	wchar *		string64_4;
	wchar *		string64_5;
	wchar *		string64_6;
	wchar *		istring64_1;
	wchar *		istring64_2;
	wchar *		text_1;
	wchar *		text_2;
	byte *		blob_1;	unsigned blob_1Length;
	byte *		blob_2;	unsigned blob_2Length;
	
	NetVaultNode ();
	~NetVaultNode ();

	// Threaded apps: Must be called with node->critsect locked	
	unsigned Read_LCS (const byte buffer[], unsigned bufsz, unsigned rwOpts);	// returns number of bytes read
	unsigned Write_LCS (ARRAY(byte) * buffer, unsigned rwOpts);					// returns number of bytes written
	
	bool Matches (const NetVaultNode * other);
	void CopyFrom (const NetVaultNode * other, unsigned copyOpts);
	
	// Threaded apps: Must be called with node->critsect locked	
	void SetNodeId (unsigned v);
	void SetCreateTime (unsigned v);
	void SetModifyTime (unsigned v);
	void SetCreateAgeName (const wchar v[]);
	void SetCreateAgeUuid (const Uuid & v);
	void SetCreatorAcct (const Uuid & v);
	void SetCreatorId (unsigned v);
	void SetNodeType (unsigned v);
	void SetInt32_1 (int v);
	void SetInt32_2 (int v);
	void SetInt32_3 (int v);
	void SetInt32_4 (int v);
	void SetUInt32_1 (unsigned v);
	void SetUInt32_2 (unsigned v);
	void SetUInt32_3 (unsigned v);
	void SetUInt32_4 (unsigned v);
	void SetUuid_1 (const Uuid & v);
	void SetUuid_2 (const Uuid & v);
	void SetUuid_3 (const Uuid & v);
	void SetUuid_4 (const Uuid & v);
	void SetString64_1 (const wchar v[]);
	void SetString64_2 (const wchar v[]);
	void SetString64_3 (const wchar v[]);
	void SetString64_4 (const wchar v[]);
	void SetString64_5 (const wchar v[]);
	void SetString64_6 (const wchar v[]);
	void SetIString64_1 (const wchar v[]);
	void SetIString64_2 (const wchar v[]);
	void SetText_1 (const wchar v[]);
	void SetText_2 (const wchar v[]);
	void SetBlob_1 (const byte v[], unsigned len);
	void SetBlob_2 (const byte v[], unsigned len);
	
	void SetText (qword fieldFlag, const wchar v[]);
	void SetBlob (qword fieldFlag, const byte v[], unsigned len);

	// These are only here to aid macro expansions (naming case matches field flags)
	inline unsigned GetNodeId () const { return nodeId; }
	inline unsigned GetCreateTime () const { return createTime; }
	inline unsigned GetModifyTime () const { return modifyTime; }
	inline wchar * GetCreateAgeName () const { return createAgeName; }
	inline Uuid GetCreateAgeUuid () const { return createAgeUuid; }
	inline Uuid GetCreatorAcct () const { return creatorAcct; }
	inline unsigned GetCreatorId () const { return creatorId; }
	inline unsigned GetNodeType () const { return nodeType; }
	inline int GetInt32_1 () const { return int32_1; }
	inline int GetInt32_2 () const { return int32_2; }
	inline int GetInt32_3 () const { return int32_3; }
	inline int GetInt32_4 () const { return int32_4; }
	inline unsigned GetUInt32_1 () const { return uint32_1; }
	inline unsigned GetUInt32_2 () const { return uint32_2; }
	inline unsigned GetUInt32_3 () const { return uint32_3; }
	inline unsigned GetUInt32_4 () const { return uint32_4; }
	inline Uuid GetUuid_1 () const { return uuid_1; }
	inline Uuid GetUuid_2 () const { return uuid_2; }
	inline Uuid GetUuid_3 () const { return uuid_3; }
	inline Uuid GetUuid_4 () const { return uuid_4; }
	inline wchar * GetString64_1 () const { return string64_1; }
	inline wchar * GetString64_2 () const { return string64_2; }
	inline wchar * GetString64_3 () const { return string64_3; }
	inline wchar * GetString64_4 () const { return string64_4; }
	inline wchar * GetString64_5 () const { return string64_5; }
	inline wchar * GetString64_6 () const { return string64_6; }
	inline wchar * GetIString64_1 () const { return istring64_1; }
	inline wchar * GetIString64_2 () const { return istring64_2; }
	// no blob "getters"
};


//============================================================================
inline void IVaultNodeSetString (
	qword			bit,
	NetVaultNode *	node,
	char **			pdst,
	const char		src[],
	unsigned		chars
) {
	FREE(*pdst);
	if (src && src[0])
		*pdst = StrDupLen(src, chars);
	else
		*pdst = StrDupLen("", chars);
	node->fieldFlags |= bit;
	node->dirtyFlags |= bit;
}

//============================================================================
inline void IVaultNodeSetString (
	qword			bit,
	NetVaultNode *	node,
	wchar **		pdst,
	const wchar		src[],
	unsigned		chars
) {
	FREE(*pdst);
	if (src && src[0])
		*pdst = StrDupLen(src, chars);
	else
		*pdst = StrDupLen(L"", chars);
	node->fieldFlags |= bit;
	node->dirtyFlags |= bit;
}

//============================================================================
template <typename T>
inline void IVaultNodeSetValue (
	qword			bit,
	NetVaultNode *	node,
	T *				pdst,
	const T &		src
) {
	*pdst = src;
	node->fieldFlags |= bit;
	node->dirtyFlags |= bit;
}

//============================================================================
inline void IVaultNodeSetBlob (
	qword			bit,
	NetVaultNode *	node,
	byte **			pdst,
	unsigned *		pdstLen,
	const byte		src[],
	unsigned		srcLen
) {
	FREE(*pdst);
	if (src) {
		*pdst = (byte*)MEMDUP(src, srcLen);
		*pdstLen = srcLen;
	}
	else {
		*pdst = nil;
		*pdstLen = 0;
	}
	node->fieldFlags |= bit;
	node->dirtyFlags |= bit;
}


//============================================================================
// NetVaultNodeFieldArray
//============================================================================
struct NetVaultNodeFieldArray {
	enum EWhereCondition {
		kAnd,
		kOr
	};
	enum ESqlType {
		kSqlInvalid,
		kSqlInt32,
		kSqlUInt32,
		kSqlUuid,
		kSqlString,
		kSqlCLob,
		KSqlBlob,
	};
	
	struct Field {
		void *			addr;
		const wchar *	name;
		Field (void * addr, const wchar name[])
		: addr(addr), name(name)
		{ }
	};
	ARRAY(Field)	fields;
	NetVaultNode *	node;

	NetVaultNodeFieldArray (NetVaultNode * node);
	~NetVaultNodeFieldArray ();

	void * GetFieldAddress (qword bit);
	const wchar * GetFieldName (qword bit);
	
	// client must lock node's local critical section before calling these.
	void GetFieldValueString_LCS (qword bit, wchar * dst, unsigned dstChars);	
	void BuildWhereClause_LCS (EWhereCondition condition, wchar * dst, unsigned dstChars);
	ESqlType GetSqlType_LCS (qword bit);
};


//============================================================================
// NetVaultNodeRef (packed because is sent over wire directly)
//============================================================================
#include <PshPack1.h>
struct NetVaultNodeRef {
	unsigned	parentId;
	unsigned	childId;
	unsigned	ownerId;
	bool		seen;
};
#include <PopPack.h>

//============================================================================
// SrvPackBuffer
//============================================================================

// Allocate a CSrvPackBuffer on the heap with one extra dword to allow for padding
#define SRV_ALLOC_BUFFER(bytes)										\
	new(ALLOC(sizeof(CSrvPackBuffer) + (bytes) + sizeof(dword)))	\
	CSrvPackBuffer(bytes + sizeof(dword))

// Allocate a CSrvPackBuffer on the stack with one extra dword to allow for padding
#define SRV_ALLOCA_BUFFER(bytes)										\
	new(_alloca(sizeof(CSrvPackBuffer) + (bytes) + sizeof(dword)))	\
	CSrvPackBuffer(bytes + sizeof(dword))

class CSrvPackBuffer {
public:
	CSrvPackBuffer (unsigned bytes);

	void * Alloc (unsigned bytes);
	void AddData (const void * ptr, unsigned bytes);
	void AddString (const wchar str[]);
	void AddDWordArray (const dword * arr, unsigned count);
	void AddDWordArray (const unsigned * arr, unsigned count);
	// add new "Add..." methods here as needed

	unsigned Size ();
	
private:
	byte * m_pos;
	byte * m_end;
	byte * m_data;
};

class CSrvUnpackBuffer {
public:
	CSrvUnpackBuffer (const void * buffer, unsigned count);

	const void *  GetData (unsigned bytes);
	const wchar * GetString ();
	const dword * GetDWordArray (unsigned count);
	
	unsigned BytesLeft ();
	bool ParseError ();

private:
	const byte * m_pos;
	const byte * m_end;
};