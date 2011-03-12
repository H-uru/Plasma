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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop



namespace pnNpCommon {

// Verify our qword constants were properly inited as such.
COMPILER_ASSERT(NetVaultNode::kBlob_2);



/*****************************************************************************
*
*   Local data
*
***/

const unsigned kNumBlobFields	= 4;


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
template <typename T>
static inline void IReadValue (T * value, byte ** buffer, unsigned * bufsz) {
	ASSERT(*bufsz >= sizeof(T));
	*value = *(T *)*buffer;
	*buffer += sizeof(T);
	*bufsz -= sizeof(T);
}

//============================================================================
template <typename T>
static inline void IReadArray (T ** buf, unsigned * elems, byte ** buffer, unsigned * bufsz) {
	dword bytes;
	IReadValue(&bytes, buffer, bufsz);
	ASSERT(bytes % sizeof(T) == 0);
	*elems = bytes / sizeof(T);
	T * src = (T *)*buffer;
	DEL(*buf);
	*buf = (T *)ALLOC(bytes);
	MemCopy(*buf, src, bytes);
	*buffer += bytes;
	*bufsz -= bytes;
}

//============================================================================
template <typename T>
static inline void IReadString (T ** buf, byte ** buffer, unsigned * bufsz) {
	unsigned elems;
	IReadArray(buf, &elems, buffer, bufsz);
	// ensure the string is null-terminated
	if (elems)
		(*buf)[elems-1] = 0;
}

//============================================================================
template <typename T>
static inline void IWriteValue (const T & value, ARRAY(byte) * buffer) {
	T * ptr = (T *) buffer->New(sizeof(T));
	*ptr = value;
}

//============================================================================
template <typename T>
static inline void IWriteArray (const T buf[], unsigned elems, ARRAY(byte) * buffer) {
	unsigned bytes = elems * sizeof(T);
	IWriteValue(bytes, buffer);
	T * dst = (T *) buffer->New(bytes);
	MemCopy(dst, buf, bytes);
}

//============================================================================
template <typename T>
static inline void IWriteString (const T str[], ARRAY(byte) * buffer) {
	IWriteArray(str, StrLen(str) + 1, buffer);
}

//============================================================================
template <typename T>
static inline bool ICompareValue (const T & lhs, const T & rhs) {
	return lhs == rhs;
}

//============================================================================
template <typename T>
static inline bool ICompareString (const T lhs[], const T rhs[]) {
	if (!lhs && !rhs)
		return true;
	if (!lhs || !rhs)
		return false;
	return 0 == StrCmp(lhs, rhs);
}

//============================================================================
template <typename T>
static inline bool ICompareStringI (const T lhs[], const T rhs[]) {
	if (!lhs && !rhs)
		return true;
	if (!lhs || !rhs)
		return false;
	return 0 == StrCmpI(lhs, rhs);
}

//============================================================================
static inline bool ICompareArray (const byte lhs[], const byte rhs[]) {
	ref(lhs);
	ref(rhs);
	return false;
}

//============================================================================
template <typename T>
static inline void ICopyValue (T * plhs, const T & rhs) {
	*plhs = rhs;
}

//============================================================================
template <typename T>
static inline void ICopyString (T ** plhs, const T rhs[]) {
	FREE(*plhs);
	if (rhs)
		*plhs = StrDup(rhs);
	else
		*plhs = StrDup("");
}

//============================================================================
static inline void ICopyString (wchar ** plhs, const wchar rhs[]) {
	FREE(*plhs);
	if (rhs)
		*plhs = StrDup(rhs);
	else
		*plhs = StrDup(L"");
}

//============================================================================
template <typename T>
static bool IStrSqlEscape (const T src[], T * dst, unsigned dstChars) {

	// count the number of ' chars
	unsigned ticks = 0;
	{
		const T * cur = src;
		while (*cur) {
			if (*cur == L'\'')
				++ticks;
			cur++;
		}
	}
	
	unsigned reqChars = StrLen(src) + ticks + 1;
	
	if (dstChars < reqChars)
		// failure!
		return false;
	
	T * cur = dst;

	// copy src to dst, escaping ' chars
	while (*src) {
		if (*src == L'\'') {
			*cur++ = L'\'';
			*cur++ = *src++;
			continue;
		}
		*cur++ = *src++;
	}
	
	// null-terminate dst string
	*cur = 0;
	
	// success!
	return true;
}


} using namespace pnNpCommon;


/*****************************************************************************
*
*   NetGameScore
*
***/

//============================================================================
unsigned NetGameScore::Read(const byte inbuffer[], unsigned bufsz, byte** end) {

	byte * buffer = const_cast<byte *>(inbuffer);
	byte * start = buffer;

	wchar* tempstr = nil;

	IReadValue(&scoreId, &buffer, &bufsz);
	IReadValue(&ownerId, &buffer, &bufsz);
	IReadValue(&createdTime, &buffer, &bufsz);
	IReadValue(&gameType, &buffer, &bufsz);
	IReadValue(&value, &buffer, &bufsz);
	IReadString(&tempstr, &buffer, &bufsz);

	StrCopy(gameName, tempstr, arrsize(gameName));
	DEL(tempstr);

	if (end)
		*end = buffer;

	return buffer - start;
}

//============================================================================
unsigned NetGameScore::Write(ARRAY(byte) * buffer) const {

	unsigned pos = buffer->Count();

	IWriteValue(scoreId, buffer);
	IWriteValue(ownerId, buffer);
	IWriteValue(createdTime, buffer);
	IWriteValue(gameType, buffer);
	IWriteValue(value, buffer);
	IWriteString(gameName, buffer);

	return buffer->Count() - pos;
}

//============================================================================
void NetGameScore::CopyFrom(const NetGameScore & score) {
	scoreId		= score.scoreId;
	ownerId		= score.ownerId;
	createdTime	= score.createdTime;
	gameType	= score.gameType;
	value		= score.value;
	StrCopy(gameName, score.gameName, arrsize(gameName));
}

/*****************************************************************************
*
*   NetGameRank
*
***/

//============================================================================
unsigned NetGameRank::Read(const byte inbuffer[], unsigned bufsz, byte** end) {

	byte * buffer = const_cast<byte *>(inbuffer);
	byte * start = buffer;

	wchar* tempstr = nil;

	IReadValue(&rank, &buffer, &bufsz);
	IReadValue(&score, &buffer, &bufsz);
	IReadString(&tempstr, &buffer, &bufsz);

	StrCopy(name, tempstr, arrsize(name));
	DEL(tempstr);

	if (end)
		*end = buffer;

	return buffer - start;
}

//============================================================================
unsigned NetGameRank::Write(ARRAY(byte) * buffer) const {

	unsigned pos = buffer->Count();

	IWriteValue(rank, buffer);
	IWriteValue(score, buffer);
	IWriteString(name, buffer);

	return buffer->Count() - pos;
}

//============================================================================
void NetGameRank::CopyFrom(const NetGameRank & fromRank) {
	rank		= fromRank.rank;
	score		= fromRank.score;
	StrCopy(name, fromRank.name, arrsize(name));
}

/*****************************************************************************
*
*   NetVaultNode
*
***/

//============================================================================
static void DeallocNodeFields (NetVaultNode * node) {
	for (qword fieldFlag = 1; fieldFlag; fieldFlag <<= 1) {
		if (fieldFlag > node->fieldFlags)
			break;

		#define DELFIELD(f, v) case (qword)(NetVaultNode::f): DEL(node->v); node->v = nil; break
		switch (fieldFlag & node->fieldFlags) {
			DELFIELD(kCreateAgeName,	createAgeName);
			DELFIELD(kString64_1,		string64_1);
			DELFIELD(kString64_2,		string64_2);
			DELFIELD(kString64_3,		string64_3);
			DELFIELD(kString64_4,		string64_4);
			DELFIELD(kString64_5,		string64_5);
			DELFIELD(kString64_6,		string64_6);
			DELFIELD(kIString64_1,		istring64_1);
			DELFIELD(kIString64_2,		istring64_2);
			DELFIELD(kText_1,			text_1);
			DELFIELD(kText_2,			text_2);
			DELFIELD(kBlob_1,			blob_1);
			DELFIELD(kBlob_2,			blob_2);
			default: break;
		}
	}
}

//============================================================================
NetVaultNode::NetVaultNode () {
	ASSERTMSG(!fieldFlags, "NetVaultNode instances must be allocated with NEWZERO");
}

//============================================================================
NetVaultNode::~NetVaultNode () {
	DeallocNodeFields(this);
}

//============================================================================
unsigned NetVaultNode::Read_LCS (const byte inbuffer[], unsigned bufsz, unsigned rwOpts) {

	DeallocNodeFields(this);

	byte * buffer = const_cast<byte *>(inbuffer);
	byte * start = buffer;

	IReadValue(&fieldFlags, &buffer, &bufsz);
	
	for (qword bit = 1; bit; bit <<= 1) {

		// if we've passed all fields on the node then bail	
		if (bit > fieldFlags)
			break;
			
		// if this field isn't in the set to be read, then continue to next
		if (!(bit & fieldFlags))
			continue;

		#define READ(flag, func, varptr)			case flag: func(varptr, &buffer, &bufsz); break
		#define READARR(flag, func, varptr, lenptr)	case flag: func(varptr, lenptr, &buffer, &bufsz); break
		switch (bit) {
			READ(kNodeId,		IReadValue,		&nodeId);
			READ(kCreateTime,	IReadValue,		&createTime);
			READ(kModifyTime,	IReadValue,		&modifyTime);
			READ(kCreateAgeName,IReadString,	&createAgeName);
			READ(kCreateAgeUuid,IReadValue,		&createAgeUuid);
			READ(kCreatorAcct,	IReadValue,		&creatorAcct);
			READ(kCreatorId,	IReadValue,		&creatorId);
			READ(kNodeType,		IReadValue,		&nodeType);
			READ(kInt32_1,		IReadValue,		&int32_1);
			READ(kInt32_2,		IReadValue,		&int32_2);
			READ(kInt32_3,		IReadValue,		&int32_3);
			READ(kInt32_4,		IReadValue,		&int32_4);
			READ(kUInt32_1,		IReadValue,		&uint32_1);
			READ(kUInt32_2,		IReadValue,		&uint32_2);
			READ(kUInt32_3,		IReadValue,		&uint32_3);
			READ(kUInt32_4,		IReadValue,		&uint32_4);
			READ(kUuid_1,		IReadValue,		&uuid_1);
			READ(kUuid_2,		IReadValue,		&uuid_2);
			READ(kUuid_3,		IReadValue,		&uuid_3);
			READ(kUuid_4,		IReadValue,		&uuid_4);
			READ(kString64_1,	IReadString,	&string64_1);
			READ(kString64_2,	IReadString,	&string64_2);
			READ(kString64_3,	IReadString,	&string64_3);
			READ(kString64_4,	IReadString,	&string64_4);
			READ(kString64_5,	IReadString,	&string64_5);
			READ(kString64_6,	IReadString,	&string64_6);
			READ(kIString64_1,	IReadString,	&istring64_1);
			READ(kIString64_2,	IReadString,	&istring64_2);
			READ(kText_1,		IReadString,	&text_1);
			READ(kText_2,		IReadString,	&text_2);
			READARR(kBlob_1,	IReadArray,		&blob_1, &blob_1Length);
			READARR(kBlob_2,	IReadArray,		&blob_2, &blob_2Length);
			DEFAULT_FATAL(bit);
		}
		#undef READARR
		#undef READ
	}
	
	if (rwOpts & kRwUpdateDirty)
		dirtyFlags = fieldFlags;
	else
		dirtyFlags = 0;
		
	return buffer - start;
}

//============================================================================
unsigned NetVaultNode::Write_LCS (ARRAY(byte) * buffer, unsigned rwOpts) {

	unsigned pos = buffer->Count();

	qword flags = fieldFlags;

	if (rwOpts & kRwDirtyOnly)
		flags &= dirtyFlags;

	if (!flags)
		return 0;

	IWriteValue(flags, buffer);
		
	for (qword bit = 1; bit; bit <<= 1) {

		// if we've passed all fields on the node then bail	
		if (bit > flags)
			break;

		// if this field isn't in the set to be written, then continue to next
		if (!(bit & flags))
			continue;
			
		#define WRITE(flag, func, var)			case flag: func(var, buffer); break
		#define WRITEARR(flag, func, var, len)	case flag: func(var, len, buffer); break
		switch (bit) {
			WRITE(kNodeId,			IWriteValue,	nodeId			);
			WRITE(kCreateTime,		IWriteValue,	createTime		);
			WRITE(kModifyTime,		IWriteValue,	modifyTime		);
			WRITE(kCreateAgeName,	IWriteString,	createAgeName ? createAgeName : L""	);
			WRITE(kCreateAgeUuid,	IWriteValue,	createAgeUuid	);
			WRITE(kCreatorAcct,		IWriteValue,	creatorAcct		);
			WRITE(kCreatorId,		IWriteValue,	creatorId		);
			WRITE(kNodeType,		IWriteValue,	nodeType		);
			WRITE(kInt32_1,			IWriteValue,	int32_1			);
			WRITE(kInt32_2,			IWriteValue,	int32_2			);
			WRITE(kInt32_3,			IWriteValue,	int32_3			);
			WRITE(kInt32_4,			IWriteValue,	int32_4			);
			WRITE(kUInt32_1,		IWriteValue,	uint32_1		);
			WRITE(kUInt32_2,		IWriteValue,	uint32_2		);
			WRITE(kUInt32_3,		IWriteValue,	uint32_3		);
			WRITE(kUInt32_4,		IWriteValue,	uint32_4		);
			WRITE(kUuid_1,			IWriteValue,	uuid_1			);
			WRITE(kUuid_2,			IWriteValue,	uuid_2			);
			WRITE(kUuid_3,			IWriteValue,	uuid_3			);
			WRITE(kUuid_4,			IWriteValue,	uuid_4			);
			WRITE(kString64_1,		IWriteString,	string64_1 ? string64_1 : L""		);
			WRITE(kString64_2,		IWriteString,	string64_2 ? string64_2 : L""		);
			WRITE(kString64_3,		IWriteString,	string64_3 ? string64_3 : L""		);
			WRITE(kString64_4,		IWriteString,	string64_4 ? string64_4 : L""		);
			WRITE(kString64_5,		IWriteString,	string64_5 ? string64_5 : L""		);
			WRITE(kString64_6,		IWriteString,	string64_6 ? string64_6 : L""		);
			WRITE(kIString64_1,		IWriteString,	istring64_1 ? istring64_1 : L""		);
			WRITE(kIString64_2,		IWriteString,	istring64_2 ? istring64_2 : L""		);
			WRITE(kText_1,			IWriteString,	text_1 ? text_1 : L""				);
			WRITE(kText_2,			IWriteString,	text_2 ? text_2 : L""				);
			WRITEARR(kBlob_1,		IWriteArray,	blob_1,	blob_1Length);
			WRITEARR(kBlob_2,		IWriteArray,	blob_2,	blob_2Length);
			DEFAULT_FATAL(bit);
		}
		#undef WRITEARR
		#undef WRITE
	}

	if (rwOpts & kRwUpdateDirty)
		dirtyFlags = 0;
	// else, preserve existing dirtyFlags value

	return buffer->Count() - pos;
}

//============================================================================
bool NetVaultNode::Matches (const NetVaultNode * other) {
	for (qword bit = 1; bit; bit <<= 1) {
		// if bit has gone past all set fields on the other node without failing, return true
		if (bit > other->fieldFlags)
			return true;
			
		// if the other node does not have the field, then continue to next field			
		if (!(bit & other->fieldFlags))
			continue;

		// if we don't have the field (but the other node does), then return false
		if (!(bit & fieldFlags))
			return false;
			
		#define COMPARE(flag, func, var) case flag: if (!func(var, other->var)) return false; break
		switch (bit) {
			COMPARE(kNodeId,		ICompareValue,		nodeId);
			COMPARE(kCreateTime,	ICompareValue,		createTime);
			COMPARE(kModifyTime,	ICompareValue,		modifyTime);
			COMPARE(kCreateAgeName,	ICompareStringI,	createAgeName);
			COMPARE(kCreateAgeUuid,	ICompareValue,		createAgeUuid);
			COMPARE(kCreatorAcct,	ICompareValue,		creatorAcct);
			COMPARE(kCreatorId,		ICompareValue,		creatorId);
			COMPARE(kNodeType,		ICompareValue,		nodeType);
			COMPARE(kInt32_1,		ICompareValue,		int32_1);
			COMPARE(kInt32_2,		ICompareValue,		int32_2);
			COMPARE(kInt32_3,		ICompareValue,		int32_3);
			COMPARE(kInt32_4,		ICompareValue,		int32_4);
			COMPARE(kUInt32_1,		ICompareValue,		uint32_1);
			COMPARE(kUInt32_2,		ICompareValue,		uint32_2);
			COMPARE(kUInt32_3,		ICompareValue,		uint32_3);
			COMPARE(kUInt32_4,		ICompareValue,		uint32_4);
			COMPARE(kUuid_1,		ICompareValue,		uuid_1);
			COMPARE(kUuid_2,		ICompareValue,		uuid_2);
			COMPARE(kUuid_3,		ICompareValue,		uuid_3);
			COMPARE(kUuid_4,		ICompareValue,		uuid_4);
			COMPARE(kString64_1,	ICompareString,		string64_1);
			COMPARE(kString64_2,	ICompareString,		string64_2);
			COMPARE(kString64_3,	ICompareString,		string64_3);
			COMPARE(kString64_4,	ICompareString,		string64_4);
			COMPARE(kString64_5,	ICompareString,		string64_5);
			COMPARE(kString64_6,	ICompareString,		string64_6);
			COMPARE(kIString64_1,	ICompareStringI,	istring64_1);
			COMPARE(kIString64_2,	ICompareStringI,	istring64_2);
			COMPARE(kText_1,		ICompareString,		text_1);
			COMPARE(kText_2,		ICompareString,		text_2);
			COMPARE(kBlob_1,		ICompareArray,		blob_1);
			COMPARE(kBlob_2,		ICompareArray,		blob_2);
			DEFAULT_FATAL(bit);
		}
		#undef COMPARE
	}
	return true;
}

//============================================================================
void NetVaultNode::CopyFrom (const NetVaultNode * other, unsigned copyOpts) {
	if (this == other)
		return;
		
	qword origDirtyFlags = dirtyFlags;
		
	for (qword bit = 1; bit; bit <<= 1) {
		// we already have a value for this field...
		if (bit & fieldFlags) {
			if (!(copyOpts & kCopyOverwrite))
				continue;	// don't overwrite our field value
		}
				
		// other does not have a value for this field...
		if (!(bit & other->fieldFlags)) {
			// clear our field?
			if (!(copyOpts & kCopyClear))
				continue;
			// clear our field.	
			if (bit & fieldFlags) {
				#define _ZERO(flag, func, var, z)			case flag: func(bit, this, &var, z); break
				#define _ZEROSTRING(flag, func, var, z)		case flag: func(bit, this, &var, z, kMaxVaultNodeStringLength); break
				#define _ZEROCLOB(flag, func, var, z)		case flag: func(bit, this, &var, z, (unsigned)-1); break
				#define _ZEROARR(flag, func, var, varlen)	case flag: func(bit, this, &var, &varlen, nil, 0); break
				switch (bit) {
					_ZERO(kNodeId,				IVaultNodeSetValue,		nodeId,			(unsigned)0);
					_ZERO(kCreateTime,			IVaultNodeSetValue,		createTime,		(unsigned)0);
					_ZERO(kModifyTime,			IVaultNodeSetValue,		modifyTime,		(unsigned)0);
					_ZEROSTRING(kCreateAgeName,	IVaultNodeSetString,	createAgeName,	L"");
					_ZERO(kCreateAgeUuid,		IVaultNodeSetValue,		createAgeUuid,	kNilGuid);
					_ZERO(kCreatorAcct,			IVaultNodeSetValue,		creatorAcct,	kNilGuid);
					_ZERO(kCreatorId,			IVaultNodeSetValue,		creatorId,		(unsigned)0);
					_ZERO(kNodeType,			IVaultNodeSetValue,		nodeType,		(unsigned)0);
					_ZERO(kInt32_1,				IVaultNodeSetValue,		int32_1,		(signed)0);
					_ZERO(kInt32_2,				IVaultNodeSetValue,		int32_2,		(signed)0);
					_ZERO(kInt32_3,				IVaultNodeSetValue,		int32_3,		(signed)0);
					_ZERO(kInt32_4,				IVaultNodeSetValue,		int32_4,		(signed)0);
					_ZERO(kUInt32_1,			IVaultNodeSetValue,		uint32_1,		(unsigned)0);
					_ZERO(kUInt32_2,			IVaultNodeSetValue,		uint32_2,		(unsigned)0);
					_ZERO(kUInt32_3,			IVaultNodeSetValue,		uint32_3,		(unsigned)0);
					_ZERO(kUInt32_4,			IVaultNodeSetValue,		uint32_4,		(unsigned)0);
					_ZERO(kUuid_1,				IVaultNodeSetValue,		uuid_1,			kNilGuid);
					_ZERO(kUuid_2,				IVaultNodeSetValue,		uuid_2,			kNilGuid);
					_ZERO(kUuid_3,				IVaultNodeSetValue,		uuid_3,			kNilGuid);
					_ZERO(kUuid_4,				IVaultNodeSetValue,		uuid_4,			kNilGuid);
					_ZEROSTRING(kString64_1,	IVaultNodeSetString,	string64_1,		L"");
					_ZEROSTRING(kString64_2,	IVaultNodeSetString,	string64_2,		L"");
					_ZEROSTRING(kString64_3,	IVaultNodeSetString,	string64_3,		L"");
					_ZEROSTRING(kString64_4,	IVaultNodeSetString,	string64_4,		L"");
					_ZEROSTRING(kString64_5,	IVaultNodeSetString,	string64_5,		L"");
					_ZEROSTRING(kString64_6,	IVaultNodeSetString,	string64_6,		L"");
					_ZEROSTRING(kIString64_1,	IVaultNodeSetString,	istring64_1,	L"");
					_ZEROSTRING(kIString64_2,	IVaultNodeSetString,	istring64_2,	L"");
					_ZEROCLOB(kText_1,			IVaultNodeSetString,	text_1,			L"");
					_ZEROCLOB(kText_2,			IVaultNodeSetString,	text_2,			L"");
					_ZEROARR(kBlob_1,			IVaultNodeSetBlob,		blob_1,	blob_1Length);
					_ZEROARR(kBlob_2,			IVaultNodeSetBlob,		blob_2,	blob_2Length);
					DEFAULT_FATAL(bit);
				}
				#undef _ZEROARR
				#undef _ZEROCLOB
				#undef _ZEROSTRING
				#undef _ZERO
			}
		}
		
		#define COPY(flag, func, var)				case flag: func(bit, this, &var, other->var); break
		#define COPYSTRING(flag, func, var)			case flag: func(bit, this, &var, other->var, kMaxVaultNodeStringLength); break
		#define COPYCLOB(flag, func, var)			case flag: func(bit, this, &var, other->var, (unsigned)-1); break
		#define COPYARR(flag, func, var, varlen)	case flag: func(bit, this, &var, &varlen, other->var, other->varlen); break
		switch (bit) {
			COPY(kNodeId,				IVaultNodeSetValue,		nodeId			);
			COPY(kCreateTime,			IVaultNodeSetValue,		createTime		);
			COPY(kModifyTime,			IVaultNodeSetValue,		modifyTime		);
			COPYSTRING(kCreateAgeName,	IVaultNodeSetString,	createAgeName	);
			COPY(kCreateAgeUuid,		IVaultNodeSetValue,		createAgeUuid	);
			COPY(kCreatorAcct,			IVaultNodeSetValue,		creatorAcct		);
			COPY(kCreatorId,			IVaultNodeSetValue,		creatorId		);
			COPY(kNodeType,				IVaultNodeSetValue,		nodeType		);
			COPY(kInt32_1,				IVaultNodeSetValue,		int32_1			);
			COPY(kInt32_2,				IVaultNodeSetValue,		int32_2			);
			COPY(kInt32_3,				IVaultNodeSetValue,		int32_3			);
			COPY(kInt32_4,				IVaultNodeSetValue,		int32_4			);
			COPY(kUInt32_1,				IVaultNodeSetValue,		uint32_1		);
			COPY(kUInt32_2,				IVaultNodeSetValue,		uint32_2		);
			COPY(kUInt32_3,				IVaultNodeSetValue,		uint32_3		);
			COPY(kUInt32_4,				IVaultNodeSetValue,		uint32_4		);
			COPY(kUuid_1,				IVaultNodeSetValue,		uuid_1			);
			COPY(kUuid_2,				IVaultNodeSetValue,		uuid_2			);
			COPY(kUuid_3,				IVaultNodeSetValue,		uuid_3			);
			COPY(kUuid_4,				IVaultNodeSetValue,		uuid_4			);
			COPYSTRING(kString64_1,		IVaultNodeSetString,	string64_1		);
			COPYSTRING(kString64_2,		IVaultNodeSetString,	string64_2		);
			COPYSTRING(kString64_3,		IVaultNodeSetString,	string64_3		);
			COPYSTRING(kString64_4,		IVaultNodeSetString,	string64_4		);
			COPYSTRING(kString64_5,		IVaultNodeSetString,	string64_5		);
			COPYSTRING(kString64_6,		IVaultNodeSetString,	string64_6		);
			COPYSTRING(kIString64_1,	IVaultNodeSetString,	istring64_1		);
			COPYSTRING(kIString64_2,	IVaultNodeSetString,	istring64_2		);
			COPYCLOB(kText_1,			IVaultNodeSetString,	text_1			);
			COPYCLOB(kText_2,			IVaultNodeSetString,	text_2			);
			COPYARR(kBlob_1,			IVaultNodeSetBlob,		blob_1,	blob_1Length);
			COPYARR(kBlob_2,			IVaultNodeSetBlob,		blob_2,	blob_2Length);
			DEFAULT_FATAL(bit);
		}
		#undef COPYARR
		#undef COPYCLOB
		#undef COPYSTRING
		#undef COPY
	}
	
	if (!(copyOpts & kCopySetDirty))
		dirtyFlags = origDirtyFlags;
}

//============================================================================
void NetVaultNode::SetNodeId (unsigned v) {
	IVaultNodeSetValue(kNodeId, this, &nodeId, v);
}

//============================================================================
void NetVaultNode::SetCreateTime (unsigned v) {
	IVaultNodeSetValue(kCreateTime, this, &createTime, v);
}

//============================================================================
void NetVaultNode::SetModifyTime (unsigned v) {
	IVaultNodeSetValue(kModifyTime, this, &modifyTime, v);
}

//============================================================================
void NetVaultNode::SetCreateAgeName (const wchar v[]) {
	IVaultNodeSetString(kCreateAgeName, this, &createAgeName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetCreateAgeUuid (const Uuid & v) {
	IVaultNodeSetValue(kCreateAgeUuid, this, &createAgeUuid, v);
}

//============================================================================
void NetVaultNode::SetCreatorAcct (const Uuid & v) {
	IVaultNodeSetValue(kCreatorAcct, this, &creatorAcct, v);
}

//============================================================================
void NetVaultNode::SetCreatorId (unsigned v) {
	IVaultNodeSetValue(kCreatorId, this, &creatorId, v);
}

//============================================================================
void NetVaultNode::SetNodeType (unsigned v) {
	IVaultNodeSetValue(kNodeType, this, &nodeType, v);
}

//============================================================================
void NetVaultNode::SetInt32_1 (int v) {
	IVaultNodeSetValue(kInt32_1, this, &int32_1, v);
}

//============================================================================
void NetVaultNode::SetInt32_2 (int v) {
	IVaultNodeSetValue(kInt32_2, this, &int32_2, v);
}

//============================================================================
void NetVaultNode::SetInt32_3 (int v) {
	IVaultNodeSetValue(kInt32_3, this, &int32_3, v);
}

//============================================================================
void NetVaultNode::SetInt32_4 (int v) {
	IVaultNodeSetValue(kInt32_4, this, &int32_4, v);
}

//============================================================================
void NetVaultNode::SetUInt32_1 (unsigned v) {
	IVaultNodeSetValue(kUInt32_1, this, &uint32_1, v);
}

//============================================================================
void NetVaultNode::SetUInt32_2 (unsigned v) {
	IVaultNodeSetValue(kUInt32_2, this, &uint32_2, v);
}

//============================================================================
void NetVaultNode::SetUInt32_3 (unsigned v) {
	IVaultNodeSetValue(kUInt32_3, this, &uint32_3, v);
}

//============================================================================
void NetVaultNode::SetUInt32_4 (unsigned v) {
	IVaultNodeSetValue(kUInt32_4, this, &uint32_4, v);
}

//============================================================================
void NetVaultNode::SetUuid_1 (const Uuid & v) {
	IVaultNodeSetValue(kUuid_1, this, &uuid_1, v);
}

//============================================================================
void NetVaultNode::SetUuid_2 (const Uuid & v) {
	IVaultNodeSetValue(kUuid_2, this, &uuid_2, v);
}

//============================================================================
void NetVaultNode::SetUuid_3 (const Uuid & v) {
	IVaultNodeSetValue(kUuid_3, this, &uuid_3, v);
}

//============================================================================
void NetVaultNode::SetUuid_4 (const Uuid & v) {
	IVaultNodeSetValue(kUuid_4, this, &uuid_4, v);
}

//============================================================================
void NetVaultNode::SetString64_1 (const wchar v[]) {
	IVaultNodeSetString(kString64_1, this, &string64_1, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_2 (const wchar v[]) {
	IVaultNodeSetString(kString64_2, this, &string64_2, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_3 (const wchar v[]) {
	IVaultNodeSetString(kString64_3, this, &string64_3, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_4 (const wchar v[]) {
	IVaultNodeSetString(kString64_4, this, &string64_4, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_5 (const wchar v[]) {
	IVaultNodeSetString(kString64_5, this, &string64_5, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_6 (const wchar v[]) {
	IVaultNodeSetString(kString64_6, this, &string64_6, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetIString64_1 (const wchar v[]) {
	IVaultNodeSetString(kIString64_1, this, &istring64_1, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetIString64_2 (const wchar v[]) {
	IVaultNodeSetString(kIString64_2, this, &istring64_2, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetText_1 (const wchar v[]) {
	IVaultNodeSetString(kText_1, this, &text_1, v, (unsigned)-1);
}

//============================================================================
void NetVaultNode::SetText_2 (const wchar v[]) {
	IVaultNodeSetString(kText_2, this, &text_2, v, (unsigned)-1);
}

//============================================================================
void NetVaultNode::SetBlob_1 (const byte v[], unsigned len) {
	IVaultNodeSetBlob(kBlob_1, this, &blob_1, &blob_1Length, v, len);
}

//============================================================================
void NetVaultNode::SetBlob_2 (const byte v[], unsigned len) {
	IVaultNodeSetBlob(kBlob_2, this, &blob_2, &blob_2Length, v, len);
}

//============================================================================
void NetVaultNode::SetText (qword fieldFlag, const wchar v[]) {
	switch (fieldFlag) {
		case kText_1: SetText_1(v); break;
		case kText_2: SetText_2(v); break;
		DEFAULT_FATAL(fieldFlag);
	}
}

//============================================================================
void NetVaultNode::SetBlob (qword fieldFlag, const byte v[], unsigned len) {
	switch (fieldFlag) {
		case kBlob_1: SetBlob_1(v, len); break;
		case kBlob_2: SetBlob_2(v, len); break;
		DEFAULT_FATAL(fieldFlag);
	}
}


/*****************************************************************************
*
*   NetVaultNodeFieldArray
*
***/

//============================================================================
NetVaultNodeFieldArray::NetVaultNodeFieldArray (NetVaultNode * node)
:	node(node)
{
	node->IncRef("FieldArray");
	fields.Add(Field(&node->nodeId,			L"NodeId"));
	fields.Add(Field(&node->createTime,		L"CreateTime"));
	fields.Add(Field(&node->modifyTime,		L"ModifyTime"));
	fields.Add(Field(&node->createAgeName,	L"CreateAgeName"));
	fields.Add(Field(&node->createAgeUuid,	L"CreateAgeUuid"));
	fields.Add(Field(&node->creatorAcct,	L"CreatorAcctId"));
	fields.Add(Field(&node->creatorId,		L"CreatorId"));
	fields.Add(Field(&node->nodeType,		L"NodeType"));
	fields.Add(Field(&node->int32_1,		L"Int32_1"));
	fields.Add(Field(&node->int32_2,		L"Int32_2"));
	fields.Add(Field(&node->int32_3,		L"Int32_3"));
	fields.Add(Field(&node->int32_4,		L"Int32_4"));
	fields.Add(Field(&node->uint32_1,		L"UInt32_1"));
	fields.Add(Field(&node->uint32_2,		L"UInt32_2"));
	fields.Add(Field(&node->uint32_3,		L"UInt32_3"));
	fields.Add(Field(&node->uint32_4,		L"UInt32_4"));
	fields.Add(Field(&node->uuid_1,			L"Uuid_1"));
	fields.Add(Field(&node->uuid_2,			L"Uuid_2"));
	fields.Add(Field(&node->uuid_3,			L"Uuid_3"));
	fields.Add(Field(&node->uuid_4,			L"Uuid_4"));
	fields.Add(Field(&node->string64_1,		L"String64_1"));
	fields.Add(Field(&node->string64_2,		L"String64_2"));
	fields.Add(Field(&node->string64_3,		L"String64_3"));
	fields.Add(Field(&node->string64_4,		L"String64_4"));
	fields.Add(Field(&node->string64_5,		L"String64_5"));
	fields.Add(Field(&node->string64_6,		L"String64_6"));
	fields.Add(Field(&node->istring64_1,	L"IString64_1"));
	fields.Add(Field(&node->istring64_2,	L"IString64_2"));
	fields.Add(Field(&node->text_1,			L"Text_1"));
	fields.Add(Field(&node->text_2,			L"Text_2"));
	fields.Add(Field(&node->blob_1,			L"Blob_1"));
	fields.Add(Field(&node->blob_2,			L"Blob_2"));
}

//============================================================================
NetVaultNodeFieldArray::~NetVaultNodeFieldArray () {
	fields.Clear();
	node->DecRef("FieldArray");
}

//============================================================================
void * NetVaultNodeFieldArray::GetFieldAddress (qword bit) {
	ASSERT(bit);
	
	unsigned index = 0;
	for (qword b = bit; b > 1; b >>= 1)
		++index;

	// do not return blob fields
	if (index < fields.Count() - kNumBlobFields)
		return fields[index].addr;
	else
		return nil;
}

//============================================================================
const wchar * NetVaultNodeFieldArray::GetFieldName (qword bit) {
	ASSERT(bit);
	
	unsigned index = 0;
	for (qword b = bit; b > 1; b >>= 1)
		++index;

	ASSERT(index < fields.Count());
	return fields[index].name;
}

//============================================================================
void NetVaultNodeFieldArray::GetFieldValueString_LCS (
	qword			bit,
	wchar *			dst,
	unsigned		dstChars
) {
	void * fieldAddr = GetFieldAddress(bit);
	
	switch (bit) {
		case NetVaultNode::kNodeId:
		case NetVaultNode::kCreatorId:
		case NetVaultNode::kCreateTime:
		case NetVaultNode::kModifyTime:
		case NetVaultNode::kNodeType:
		case NetVaultNode::kUInt32_1:
		case NetVaultNode::kUInt32_2:
		case NetVaultNode::kUInt32_3:
		case NetVaultNode::kUInt32_4:
			StrPrintf(dst, dstChars, L"%u", *(unsigned *)fieldAddr);
		break;

		case NetVaultNode::kInt32_1:
		case NetVaultNode::kInt32_2:
		case NetVaultNode::kInt32_3:
		case NetVaultNode::kInt32_4:
			StrPrintf(dst, dstChars, L"%i", *(int *)fieldAddr);
		break;

		case NetVaultNode::kCreateAgeUuid:
		case NetVaultNode::kCreatorAcct:
		case NetVaultNode::kUuid_1:
		case NetVaultNode::kUuid_2:
		case NetVaultNode::kUuid_3:
		case NetVaultNode::kUuid_4: {
			wchar tmp[64];
			GuidToHex(*(Uuid *)fieldAddr, tmp, arrsize(tmp));
			StrPrintf(dst, dstChars, L"hextoraw('%s')", tmp);
		}
		break;

		case NetVaultNode::kCreateAgeName:
		case NetVaultNode::kString64_1:
		case NetVaultNode::kString64_2:
		case NetVaultNode::kString64_3:
		case NetVaultNode::kString64_4:
		case NetVaultNode::kString64_5:
		case NetVaultNode::kString64_6:
		case NetVaultNode::kIString64_1:
		case NetVaultNode::kIString64_2: {
			wchar * tmp = ALLOCA(wchar, dstChars);
			IStrSqlEscape(*(wchar **)fieldAddr, tmp, dstChars);
			StrPrintf(dst, dstChars, L"'%s'", tmp);
		}
		break;

//		FIELD(Text_1);
//		FIELD(Text_2);
//		FIELD(Blob_1);
//		FIELD(Blob_2);
		DEFAULT_FATAL(bit);
	}
}

//============================================================================
void NetVaultNodeFieldArray::BuildWhereClause_LCS (
	EWhereCondition	condition,
	wchar *			dst,
	unsigned		dstChars
) {
	if (!dstChars)
		return;
		
	dst[0] = 0;
	
	static const wchar * s_conditionStrs[] = {
		L" AND ",
		L" OR "
	};

	unsigned fieldCount = 0;
	for (qword bit = 1; bit; bit <<= 1) {
		if (!(bit & node->fieldFlags))
			continue;
			
		if (fieldCount++)
			StrPack(dst, s_conditionStrs[condition], dstChars);
			
		wchar str[256];
		GetFieldValueString_LCS(bit, str, arrsize(str));

		StrPack(dst, GetFieldName(bit), dstChars);
		StrPack(dst, L"=", dstChars);
		StrPack(dst, str, dstChars);
	}	
}

//============================================================================
NetVaultNodeFieldArray::ESqlType NetVaultNodeFieldArray::GetSqlType_LCS (qword bit) {
	switch (bit) {
		case NetVaultNode::kNodeId:
		case NetVaultNode::kCreatorId:
		case NetVaultNode::kCreateTime:
		case NetVaultNode::kModifyTime:
		case NetVaultNode::kNodeType:
		case NetVaultNode::kUInt32_1:
		case NetVaultNode::kUInt32_2:
		case NetVaultNode::kUInt32_3:
		case NetVaultNode::kUInt32_4:
		return kSqlUInt32;

		case NetVaultNode::kInt32_1:
		case NetVaultNode::kInt32_2:
		case NetVaultNode::kInt32_3:
		case NetVaultNode::kInt32_4:
		return kSqlInt32;

		case NetVaultNode::kCreateAgeUuid:
		case NetVaultNode::kCreatorAcct:
		case NetVaultNode::kUuid_1:
		case NetVaultNode::kUuid_2:
		case NetVaultNode::kUuid_3:
		case NetVaultNode::kUuid_4:
		return kSqlUuid;

		case NetVaultNode::kCreateAgeName:
		case NetVaultNode::kString64_1:
		case NetVaultNode::kString64_2:
		case NetVaultNode::kString64_3:
		case NetVaultNode::kString64_4:
		case NetVaultNode::kString64_5:
		case NetVaultNode::kString64_6:
		case NetVaultNode::kIString64_1:
		case NetVaultNode::kIString64_2:
		return kSqlString;

		case NetVaultNode::kText_1:
		case NetVaultNode::kText_2:
		return kSqlCLob;
		
//		case NetVaultNode::kBlob_1:
//		case NetVaultNode::kBlob_1:
//		return kSqlBlob:

		default:
		return kSqlInvalid;
	}
}


/*****************************************************************************
*
*   CSrvPackBuffer
*
***/

//============================================================================
CSrvPackBuffer::CSrvPackBuffer (unsigned bytes) {
	m_data = (byte *)ALLOC(bytes);
	m_pos  = m_data;
	m_end  = m_pos + bytes;
}

//============================================================================
void * CSrvPackBuffer::Alloc (unsigned bytes) {
	ASSERT((signed) bytes >= 0);
	ASSERT(m_pos + bytes <= m_end);

	byte * pos = m_pos;
	m_pos += bytes;
	return pos;
}

//============================================================================
void CSrvPackBuffer::AddData (const void * ptr, unsigned bytes) {
    MemCopy(Alloc(bytes), ptr, bytes);
}

//============================================================================
void CSrvPackBuffer::AddString (const wchar str[]) {
	AddData(str, StrBytes(str));
}

//============================================================================
void CSrvPackBuffer::AddDWordArray (const dword * arr, unsigned count) {
	// Don't let large counts cause pointer wrap
	count &= 0x00ffffff;
	AddData(arr, count * sizeof(arr[0]));
}

//============================================================================
void CSrvPackBuffer::AddDWordArray (const unsigned * arr, unsigned count) {
	COMPILER_ASSERT(sizeof(unsigned) == sizeof(dword));
	// Don't let large counts cause pointer wrap
	count &= 0x00ffffff;
	AddData(arr, count * sizeof(arr[0]));
}

//============================================================================
unsigned CSrvPackBuffer::Size () {
	return m_pos - m_data;
}


/*****************************************************************************
*
*   CSrvUnpackBuffer
*
***/

//============================================================================
CSrvUnpackBuffer::CSrvUnpackBuffer (const void * buffer, unsigned count) {
	m_pos = (const byte *) buffer;
	m_end = m_pos + count;
}

//============================================================================
const void * CSrvUnpackBuffer::GetData (unsigned bytes) {
	for (;;) {
		const byte * result = m_pos;
		m_pos += bytes;
		
		if (m_pos < result)
			break;
		if (m_pos > m_end)
			break;
			
		return result;
	}
	
	m_end = nil;
	return nil;
}

//============================================================================
const wchar * CSrvUnpackBuffer::GetString () {
	
     if (m_end) {  
          const wchar * end = (const wchar *) (m_end - sizeof(wchar) + 1);  
          for (const wchar * cur = (const wchar *) m_pos; cur < end; ) {  
               if (*cur++)  
                    continue;  
  
               const wchar * pos   = (const wchar *) m_pos;  
               m_pos               = (const byte *) cur;  
               return pos;  
          }  
     }  
  
     m_end = NULL;  
     return NULL;   
}

//============================================================================
const dword * CSrvUnpackBuffer::GetDWordArray (unsigned count) {
	// Don't let large counts cause pointer wrap
	if (count & 0x00ffffff)
		return (const dword *)GetData(count * sizeof(dword));

	m_end = nil;
	return nil;
}

//============================================================================
unsigned CSrvUnpackBuffer::BytesLeft () {
	return m_end ? m_end - m_pos : 0;
}

//============================================================================
bool CSrvUnpackBuffer::ParseError () {
	return !m_end;
}
