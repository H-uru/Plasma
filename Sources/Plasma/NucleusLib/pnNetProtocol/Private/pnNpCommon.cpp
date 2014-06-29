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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.cpp
*
***/

#include "../Pch.h"
#include "pnUUID/pnUUID.h"
#pragma hdrstop



namespace pnNpCommon {

// Verify our uint64_t constants were properly inited as such.
static_assert(NetVaultNode::kBlob_2, "NetVaultNode constants failed to init");



/*****************************************************************************
*
*   Local data
*
***/

const unsigned kNumBlobFields   = 4;


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
template <typename T>
static inline void IReadValue (T * value, uint8_t ** buffer, unsigned * bufsz) {
    ASSERT(*bufsz >= sizeof(T));
    *value = *(T *)*buffer;
    *buffer += sizeof(T);
    *bufsz -= sizeof(T);
}

//============================================================================
template <typename T>
static inline void IReadArray (T ** buf, unsigned * elems, uint8_t ** buffer, unsigned * bufsz) {
    uint32_t bytes;
    IReadValue(&bytes, buffer, bufsz);
    ASSERT(bytes % sizeof(T) == 0);
    *elems = bytes / sizeof(T);
    T * src = (T *)*buffer;
    free(*buf);
    *buf = (T *)malloc(bytes);
    memcpy(*buf, src, bytes);
    *buffer += bytes;
    *bufsz -= bytes;
}

//============================================================================
template <typename T>
static inline void IReadString (T ** buf, uint8_t ** buffer, unsigned * bufsz) {
    unsigned elems;
    IReadArray(buf, &elems, buffer, bufsz);
    // ensure the string is null-terminated
    if (elems)
        (*buf)[elems-1] = 0;
}

//============================================================================
template <typename T>
static inline void IWriteValue (const T & value, ARRAY(uint8_t) * buffer) {
    T * ptr = (T *) buffer->New(sizeof(T));
    *ptr = value;
}

//============================================================================
template <typename T>
static inline void IWriteArray (const T buf[], unsigned elems, ARRAY(uint8_t) * buffer) {
    unsigned bytes = elems * sizeof(T);
    IWriteValue(bytes, buffer);
    T * dst = (T *) buffer->New(bytes);
    memcpy(dst, buf, bytes);
}

//============================================================================
template <typename T>
static inline void IWriteString (const T str[], ARRAY(uint8_t) * buffer) {
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
static inline bool ICompareArray (const uint8_t lhs[], const uint8_t rhs[]) {
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
    free(*plhs);
    if (rhs)
        *plhs = StrDup(rhs);
    else
        *plhs = StrDup("");
}

//============================================================================
static inline void ICopyString (wchar_t ** plhs, const wchar_t rhs[]) {
    free(*plhs);
    if (rhs)
        *plhs = StrDup(rhs);
    else
        *plhs = StrDup(L"");
}


} using namespace pnNpCommon;


/*****************************************************************************
*
*   NetGameScore
*
***/

//============================================================================
unsigned NetGameScore::Read(const uint8_t inbuffer[], unsigned bufsz, uint8_t** end) {

    uint8_t * buffer = const_cast<uint8_t *>(inbuffer);
    uint8_t * start = buffer;

    wchar_t* tempstr = nil;

    IReadValue(&scoreId, &buffer, &bufsz);
    IReadValue(&ownerId, &buffer, &bufsz);
    IReadValue(&createdTime, &buffer, &bufsz);
    IReadValue(&gameType, &buffer, &bufsz);
    IReadValue(&value, &buffer, &bufsz);
    IReadString(&tempstr, &buffer, &bufsz);

    StrCopy(gameName, tempstr, arrsize(gameName));
    delete tempstr;

    if (end)
        *end = buffer;

    return buffer - start;
}

//============================================================================
unsigned NetGameScore::Write(ARRAY(uint8_t) * buffer) const {

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
    scoreId     = score.scoreId;
    ownerId     = score.ownerId;
    createdTime = score.createdTime;
    gameType    = score.gameType;
    value       = score.value;
    StrCopy(gameName, score.gameName, arrsize(gameName));
}

/*****************************************************************************
*
*   NetGameRank
*
***/

//============================================================================
unsigned NetGameRank::Read(const uint8_t inbuffer[], unsigned bufsz, uint8_t** end) {

    uint8_t * buffer = const_cast<uint8_t *>(inbuffer);
    uint8_t * start = buffer;

    wchar_t* tempstr = nil;

    IReadValue(&rank, &buffer, &bufsz);
    IReadValue(&score, &buffer, &bufsz);
    IReadString(&tempstr, &buffer, &bufsz);

    StrCopy(name, tempstr, arrsize(name));
    delete tempstr;

    if (end)
        *end = buffer;

    return buffer - start;
}

//============================================================================
unsigned NetGameRank::Write(ARRAY(uint8_t) * buffer) const {

    unsigned pos = buffer->Count();

    IWriteValue(rank, buffer);
    IWriteValue(score, buffer);
    IWriteString(name, buffer);

    return buffer->Count() - pos;
}

//============================================================================
void NetGameRank::CopyFrom(const NetGameRank & fromRank) {
    rank        = fromRank.rank;
    score       = fromRank.score;
    StrCopy(name, fromRank.name, arrsize(name));
}

/*****************************************************************************
*
*   NetVaultNode
*
***/

//============================================================================
void NetVaultNode::DeallocNodeFields () {
    free(createAgeName);
    free(string64_1);
    free(string64_2);
    free(string64_3);
    free(string64_4);
    free(string64_5);
    free(string64_6);
    free(istring64_1);
    free(istring64_2);
    free(text_1);
    free(text_2);
    free(blob_1);
    free(blob_2);
}

//============================================================================
NetVaultNode::NetVaultNode ()
    : hsAtomicRefCnt(0), fieldFlags(0), dirtyFlags(0)
    , nodeId(0), createTime(0), modifyTime(0)
    , createAgeName(nil), creatorId(0)
    , nodeType(0)
    , int32_1(0), int32_2(0), int32_3(0), int32_4(0)
    , uint32_1(0), uint32_2(0), uint32_3(0), uint32_4(0)
    , string64_1(nil), string64_2(nil), string64_3(nil), string64_4(nil)
    , string64_5(nil), string64_6(nil)
    , istring64_1(nil), istring64_2(nil)
    , text_1(nil), text_2(nil)
    , blob_1(nil), blob_1Length(0), blob_2(nil), blob_2Length(0) { }

//============================================================================
NetVaultNode::~NetVaultNode () {
    DeallocNodeFields();
}

//============================================================================
uint32_t NetVaultNode::Read_LCS (const uint8_t inbuffer[], uint32_t bufsz, unsigned rwOpts) {

    DeallocNodeFields();

    uint8_t * buffer = const_cast<uint8_t *>(inbuffer);
    uint8_t * start = buffer;

    IReadValue(&fieldFlags, &buffer, &bufsz);

    #define READ(flag, func, varptr)            if (flag & fieldFlags) func(varptr, &buffer, &bufsz);
    #define READARR(flag, func, varptr, lenptr) if (flag & fieldFlags) func(varptr, lenptr, &buffer, &bufsz);
        READ(kNodeId,       IReadValue,     &nodeId);
        READ(kCreateTime,   IReadValue,     &createTime);
        READ(kModifyTime,   IReadValue,     &modifyTime);
        READ(kCreateAgeName,IReadString,    &createAgeName);
        READ(kCreateAgeUuid,IReadValue,     &createAgeUuid);
        READ(kCreatorAcct,  IReadValue,     &creatorAcct);
        READ(kCreatorId,    IReadValue,     &creatorId);
        READ(kNodeType,     IReadValue,     &nodeType);
        READ(kInt32_1,      IReadValue,     &int32_1);
        READ(kInt32_2,      IReadValue,     &int32_2);
        READ(kInt32_3,      IReadValue,     &int32_3);
        READ(kInt32_4,      IReadValue,     &int32_4);
        READ(kUInt32_1,     IReadValue,     &uint32_1);
        READ(kUInt32_2,     IReadValue,     &uint32_2);
        READ(kUInt32_3,     IReadValue,     &uint32_3);
        READ(kUInt32_4,     IReadValue,     &uint32_4);
        READ(kUuid_1,       IReadValue,     &uuid_1);
        READ(kUuid_2,       IReadValue,     &uuid_2);
        READ(kUuid_3,       IReadValue,     &uuid_3);
        READ(kUuid_4,       IReadValue,     &uuid_4);
        READ(kString64_1,   IReadString,    &string64_1);
        READ(kString64_2,   IReadString,    &string64_2);
        READ(kString64_3,   IReadString,    &string64_3);
        READ(kString64_4,   IReadString,    &string64_4);
        READ(kString64_5,   IReadString,    &string64_5);
        READ(kString64_6,   IReadString,    &string64_6);
        READ(kIString64_1,  IReadString,    &istring64_1);
        READ(kIString64_2,  IReadString,    &istring64_2);
        READ(kText_1,       IReadString,    &text_1);
        READ(kText_2,       IReadString,    &text_2);
        READARR(kBlob_1,    IReadArray,     &blob_1, &blob_1Length);
        READARR(kBlob_2,    IReadArray,     &blob_2, &blob_2Length);
    #undef READARR
    #undef READ

    if (fieldFlags & ~kAllValidFields)
        FATAL("Invalid field flag(s) encountered");

    if (rwOpts & kRwUpdateDirty)
        dirtyFlags = fieldFlags;
    else
        dirtyFlags = 0;
        
    return buffer - start;
}

//============================================================================
uint32_t NetVaultNode::Write_LCS (ARRAY(uint8_t) * buffer, unsigned rwOpts) {

    unsigned pos = buffer->Count();

    uint64_t flags = fieldFlags;

    if (rwOpts & kRwDirtyOnly)
        flags &= dirtyFlags;

    if (!flags)
        return 0;

    IWriteValue(flags, buffer);

    #define WRITE(flag, func, var)          if (flag & flags) func(var, buffer);
    #define WRITEARR(flag, func, var, len)  if (flag & flags) func(var, len, buffer);
        WRITE(kNodeId,          IWriteValue,    nodeId          );
        WRITE(kCreateTime,      IWriteValue,    createTime      );
        WRITE(kModifyTime,      IWriteValue,    modifyTime      );
        WRITE(kCreateAgeName,   IWriteString,   createAgeName ? createAgeName : L"" );
        WRITE(kCreateAgeUuid,   IWriteValue,    createAgeUuid   );
        WRITE(kCreatorAcct,     IWriteValue,    creatorAcct     );
        WRITE(kCreatorId,       IWriteValue,    creatorId       );
        WRITE(kNodeType,        IWriteValue,    nodeType        );
        WRITE(kInt32_1,         IWriteValue,    int32_1         );
        WRITE(kInt32_2,         IWriteValue,    int32_2         );
        WRITE(kInt32_3,         IWriteValue,    int32_3         );
        WRITE(kInt32_4,         IWriteValue,    int32_4         );
        WRITE(kUInt32_1,        IWriteValue,    uint32_1        );
        WRITE(kUInt32_2,        IWriteValue,    uint32_2        );
        WRITE(kUInt32_3,        IWriteValue,    uint32_3        );
        WRITE(kUInt32_4,        IWriteValue,    uint32_4        );
        WRITE(kUuid_1,          IWriteValue,    uuid_1          );
        WRITE(kUuid_2,          IWriteValue,    uuid_2          );
        WRITE(kUuid_3,          IWriteValue,    uuid_3          );
        WRITE(kUuid_4,          IWriteValue,    uuid_4          );
        WRITE(kString64_1,      IWriteString,   string64_1 ? string64_1 : L""       );
        WRITE(kString64_2,      IWriteString,   string64_2 ? string64_2 : L""       );
        WRITE(kString64_3,      IWriteString,   string64_3 ? string64_3 : L""       );
        WRITE(kString64_4,      IWriteString,   string64_4 ? string64_4 : L""       );
        WRITE(kString64_5,      IWriteString,   string64_5 ? string64_5 : L""       );
        WRITE(kString64_6,      IWriteString,   string64_6 ? string64_6 : L""       );
        WRITE(kIString64_1,     IWriteString,   istring64_1 ? istring64_1 : L""     );
        WRITE(kIString64_2,     IWriteString,   istring64_2 ? istring64_2 : L""     );
        WRITE(kText_1,          IWriteString,   text_1 ? text_1 : L""               );
        WRITE(kText_2,          IWriteString,   text_2 ? text_2 : L""               );
        WRITEARR(kBlob_1,       IWriteArray,    blob_1, blob_1Length);
        WRITEARR(kBlob_2,       IWriteArray,    blob_2, blob_2Length);
    #undef WRITEARR
    #undef WRITE

    if (flags & ~kAllValidFields)
        FATAL("Invalid field flag(s) encountered");

    if (rwOpts & kRwUpdateDirty)
        dirtyFlags = 0;
    // else, preserve existing dirtyFlags value

    return buffer->Count() - pos;
}

//============================================================================
bool NetVaultNode::Matches (const NetVaultNode * other) {
    for (uint64_t bit = 1; bit; bit <<= 1) {
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
            COMPARE(kNodeId,        ICompareValue,      nodeId);
            COMPARE(kCreateTime,    ICompareValue,      createTime);
            COMPARE(kModifyTime,    ICompareValue,      modifyTime);
            COMPARE(kCreateAgeName, ICompareStringI,    createAgeName);
            COMPARE(kCreateAgeUuid, ICompareValue,      createAgeUuid);
            COMPARE(kCreatorAcct,   ICompareValue,      creatorAcct);
            COMPARE(kCreatorId,     ICompareValue,      creatorId);
            COMPARE(kNodeType,      ICompareValue,      nodeType);
            COMPARE(kInt32_1,       ICompareValue,      int32_1);
            COMPARE(kInt32_2,       ICompareValue,      int32_2);
            COMPARE(kInt32_3,       ICompareValue,      int32_3);
            COMPARE(kInt32_4,       ICompareValue,      int32_4);
            COMPARE(kUInt32_1,      ICompareValue,      uint32_1);
            COMPARE(kUInt32_2,      ICompareValue,      uint32_2);
            COMPARE(kUInt32_3,      ICompareValue,      uint32_3);
            COMPARE(kUInt32_4,      ICompareValue,      uint32_4);
            COMPARE(kUuid_1,        ICompareValue,      uuid_1);
            COMPARE(kUuid_2,        ICompareValue,      uuid_2);
            COMPARE(kUuid_3,        ICompareValue,      uuid_3);
            COMPARE(kUuid_4,        ICompareValue,      uuid_4);
            COMPARE(kString64_1,    ICompareString,     string64_1);
            COMPARE(kString64_2,    ICompareString,     string64_2);
            COMPARE(kString64_3,    ICompareString,     string64_3);
            COMPARE(kString64_4,    ICompareString,     string64_4);
            COMPARE(kString64_5,    ICompareString,     string64_5);
            COMPARE(kString64_6,    ICompareString,     string64_6);
            COMPARE(kIString64_1,   ICompareStringI,    istring64_1);
            COMPARE(kIString64_2,   ICompareStringI,    istring64_2);
            COMPARE(kText_1,        ICompareString,     text_1);
            COMPARE(kText_2,        ICompareString,     text_2);
            COMPARE(kBlob_1,        ICompareArray,      blob_1);
            COMPARE(kBlob_2,        ICompareArray,      blob_2);
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

    uint64_t origDirtyFlags = dirtyFlags;

    for (uint64_t bit = 1; bit; bit <<= 1) {
        // we already have a value for this field...
        if (bit & fieldFlags) {
            if (!(copyOpts & kCopyOverwrite))
                continue;   // don't overwrite our field value
        }

        // other does not have a value for this field...
        if (!(bit & other->fieldFlags)) {
            // clear our field?
            if (!(copyOpts & kCopyClear))
                continue;
            // clear our field. 
            if (bit & fieldFlags) {
                #define _ZERO(flag, func, var, z)           case flag: func(bit, &var, z); break
                #define _ZEROSTRING(flag, func, var, z)     case flag: func(bit, &var, z, kMaxVaultNodeStringLength); break
                #define _ZEROCLOB(flag, func, var, z)       case flag: func(bit, &var, z, (unsigned)-1); break
                #define _ZEROARR(flag, func, var, varlen)   case flag: func(bit, &var, &varlen, nil, 0); break
                switch (bit) {
                    _ZERO(kNodeId,              IVaultNodeSetValue,     nodeId,         (unsigned)0);
                    _ZERO(kCreateTime,          IVaultNodeSetValue,     createTime,     (unsigned)0);
                    _ZERO(kModifyTime,          IVaultNodeSetValue,     modifyTime,     (unsigned)0);
                    _ZEROSTRING(kCreateAgeName, IVaultNodeSetString,    createAgeName,  L"");
                    _ZERO(kCreateAgeUuid,       IVaultNodeSetValue,     createAgeUuid,  kNilUuid);
                    _ZERO(kCreatorAcct,         IVaultNodeSetValue,     creatorAcct,    kNilUuid);
                    _ZERO(kCreatorId,           IVaultNodeSetValue,     creatorId,      (unsigned)0);
                    _ZERO(kNodeType,            IVaultNodeSetValue,     nodeType,       (unsigned)0);
                    _ZERO(kInt32_1,             IVaultNodeSetValue,     int32_1,        (signed)0);
                    _ZERO(kInt32_2,             IVaultNodeSetValue,     int32_2,        (signed)0);
                    _ZERO(kInt32_3,             IVaultNodeSetValue,     int32_3,        (signed)0);
                    _ZERO(kInt32_4,             IVaultNodeSetValue,     int32_4,        (signed)0);
                    _ZERO(kUInt32_1,            IVaultNodeSetValue,     uint32_1,       (unsigned)0);
                    _ZERO(kUInt32_2,            IVaultNodeSetValue,     uint32_2,       (unsigned)0);
                    _ZERO(kUInt32_3,            IVaultNodeSetValue,     uint32_3,       (unsigned)0);
                    _ZERO(kUInt32_4,            IVaultNodeSetValue,     uint32_4,       (unsigned)0);
                    _ZERO(kUuid_1,              IVaultNodeSetValue,     uuid_1,         kNilUuid);
                    _ZERO(kUuid_2,              IVaultNodeSetValue,     uuid_2,         kNilUuid);
                    _ZERO(kUuid_3,              IVaultNodeSetValue,     uuid_3,         kNilUuid);
                    _ZERO(kUuid_4,              IVaultNodeSetValue,     uuid_4,         kNilUuid);
                    _ZEROSTRING(kString64_1,    IVaultNodeSetString,    string64_1,     L"");
                    _ZEROSTRING(kString64_2,    IVaultNodeSetString,    string64_2,     L"");
                    _ZEROSTRING(kString64_3,    IVaultNodeSetString,    string64_3,     L"");
                    _ZEROSTRING(kString64_4,    IVaultNodeSetString,    string64_4,     L"");
                    _ZEROSTRING(kString64_5,    IVaultNodeSetString,    string64_5,     L"");
                    _ZEROSTRING(kString64_6,    IVaultNodeSetString,    string64_6,     L"");
                    _ZEROSTRING(kIString64_1,   IVaultNodeSetString,    istring64_1,    L"");
                    _ZEROSTRING(kIString64_2,   IVaultNodeSetString,    istring64_2,    L"");
                    _ZEROCLOB(kText_1,          IVaultNodeSetString,    text_1,         L"");
                    _ZEROCLOB(kText_2,          IVaultNodeSetString,    text_2,         L"");
                    _ZEROARR(kBlob_1,           IVaultNodeSetBlob,      blob_1, blob_1Length);
                    _ZEROARR(kBlob_2,           IVaultNodeSetBlob,      blob_2, blob_2Length);
                    DEFAULT_FATAL(bit);
                }
                #undef _ZEROARR
                #undef _ZEROCLOB
                #undef _ZEROSTRING
                #undef _ZERO
            }
        }
        
        #define COPY(flag, func, var)               case flag: func(bit, &var, other->var); break
        #define COPYSTRING(flag, func, var)         case flag: func(bit, &var, other->var, kMaxVaultNodeStringLength); break
        #define COPYCLOB(flag, func, var)           case flag: func(bit, &var, other->var, (unsigned)-1); break
        #define COPYARR(flag, func, var, varlen)    case flag: func(bit, &var, &varlen, other->var, other->varlen); break
        switch (bit) {
            COPY(kNodeId,               IVaultNodeSetValue,     nodeId          );
            COPY(kCreateTime,           IVaultNodeSetValue,     createTime      );
            COPY(kModifyTime,           IVaultNodeSetValue,     modifyTime      );
            COPYSTRING(kCreateAgeName,  IVaultNodeSetString,    createAgeName   );
            COPY(kCreateAgeUuid,        IVaultNodeSetValue,     createAgeUuid   );
            COPY(kCreatorAcct,          IVaultNodeSetValue,     creatorAcct     );
            COPY(kCreatorId,            IVaultNodeSetValue,     creatorId       );
            COPY(kNodeType,             IVaultNodeSetValue,     nodeType        );
            COPY(kInt32_1,              IVaultNodeSetValue,     int32_1         );
            COPY(kInt32_2,              IVaultNodeSetValue,     int32_2         );
            COPY(kInt32_3,              IVaultNodeSetValue,     int32_3         );
            COPY(kInt32_4,              IVaultNodeSetValue,     int32_4         );
            COPY(kUInt32_1,             IVaultNodeSetValue,     uint32_1        );
            COPY(kUInt32_2,             IVaultNodeSetValue,     uint32_2        );
            COPY(kUInt32_3,             IVaultNodeSetValue,     uint32_3        );
            COPY(kUInt32_4,             IVaultNodeSetValue,     uint32_4        );
            COPY(kUuid_1,               IVaultNodeSetValue,     uuid_1          );
            COPY(kUuid_2,               IVaultNodeSetValue,     uuid_2          );
            COPY(kUuid_3,               IVaultNodeSetValue,     uuid_3          );
            COPY(kUuid_4,               IVaultNodeSetValue,     uuid_4          );
            COPYSTRING(kString64_1,     IVaultNodeSetString,    string64_1      );
            COPYSTRING(kString64_2,     IVaultNodeSetString,    string64_2      );
            COPYSTRING(kString64_3,     IVaultNodeSetString,    string64_3      );
            COPYSTRING(kString64_4,     IVaultNodeSetString,    string64_4      );
            COPYSTRING(kString64_5,     IVaultNodeSetString,    string64_5      );
            COPYSTRING(kString64_6,     IVaultNodeSetString,    string64_6      );
            COPYSTRING(kIString64_1,    IVaultNodeSetString,    istring64_1     );
            COPYSTRING(kIString64_2,    IVaultNodeSetString,    istring64_2     );
            COPYCLOB(kText_1,           IVaultNodeSetString,    text_1          );
            COPYCLOB(kText_2,           IVaultNodeSetString,    text_2          );
            COPYARR(kBlob_1,            IVaultNodeSetBlob,      blob_1, blob_1Length);
            COPYARR(kBlob_2,            IVaultNodeSetBlob,      blob_2, blob_2Length);
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
    IVaultNodeSetValue(kNodeId, &nodeId, v);
}

//============================================================================
void NetVaultNode::SetCreateTime (unsigned v) {
    IVaultNodeSetValue(kCreateTime, &createTime, v);
}

//============================================================================
void NetVaultNode::SetModifyTime (unsigned v) {
    IVaultNodeSetValue(kModifyTime, &modifyTime, v);
}

//============================================================================
void NetVaultNode::SetCreateAgeName (const wchar_t v[]) {
    IVaultNodeSetString(kCreateAgeName, &createAgeName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetCreateAgeUuid (const plUUID& v) {
    IVaultNodeSetValue(kCreateAgeUuid, &createAgeUuid, v);
}

//============================================================================
void NetVaultNode::SetCreatorAcct (const plUUID& v) {
    IVaultNodeSetValue(kCreatorAcct, &creatorAcct, v);
}

//============================================================================
void NetVaultNode::SetCreatorId (unsigned v) {
    IVaultNodeSetValue(kCreatorId, &creatorId, v);
}

//============================================================================
void NetVaultNode::SetNodeType (unsigned v) {
    IVaultNodeSetValue(kNodeType, &nodeType, v);
}

//============================================================================
void NetVaultNode::SetInt32_1 (int v) {
    IVaultNodeSetValue(kInt32_1, &int32_1, v);
}

//============================================================================
void NetVaultNode::SetInt32_2 (int v) {
    IVaultNodeSetValue(kInt32_2, &int32_2, v);
}

//============================================================================
void NetVaultNode::SetInt32_3 (int v) {
    IVaultNodeSetValue(kInt32_3, &int32_3, v);
}

//============================================================================
void NetVaultNode::SetInt32_4 (int v) {
    IVaultNodeSetValue(kInt32_4, &int32_4, v);
}

//============================================================================
void NetVaultNode::SetUInt32_1 (unsigned v) {
    IVaultNodeSetValue(kUInt32_1, &uint32_1, v);
}

//============================================================================
void NetVaultNode::SetUInt32_2 (unsigned v) {
    IVaultNodeSetValue(kUInt32_2, &uint32_2, v);
}

//============================================================================
void NetVaultNode::SetUInt32_3 (unsigned v) {
    IVaultNodeSetValue(kUInt32_3, &uint32_3, v);
}

//============================================================================
void NetVaultNode::SetUInt32_4 (unsigned v) {
    IVaultNodeSetValue(kUInt32_4, &uint32_4, v);
}

//============================================================================
void NetVaultNode::SetUuid_1 (const plUUID& v) {
    IVaultNodeSetValue(kUuid_1, &uuid_1, v);
}

//============================================================================
void NetVaultNode::SetUuid_2 (const plUUID& v) {
    IVaultNodeSetValue(kUuid_2, &uuid_2, v);
}

//============================================================================
void NetVaultNode::SetUuid_3 (const plUUID& v) {
    IVaultNodeSetValue(kUuid_3, &uuid_3, v);
}

//============================================================================
void NetVaultNode::SetUuid_4 (const plUUID& v) {
    IVaultNodeSetValue(kUuid_4, &uuid_4, v);
}

//============================================================================
void NetVaultNode::SetString64_1 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_1, &string64_1, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_2 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_2, &string64_2, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_3 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_3, &string64_3, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_4 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_4, &string64_4, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_5 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_5, &string64_5, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetString64_6 (const wchar_t v[]) {
    IVaultNodeSetString(kString64_6, &string64_6, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetIString64_1 (const wchar_t v[]) {
    IVaultNodeSetString(kIString64_1, &istring64_1, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetIString64_2 (const wchar_t v[]) {
    IVaultNodeSetString(kIString64_2, &istring64_2, v, kMaxVaultNodeStringLength);
}

//============================================================================
void NetVaultNode::SetText_1 (const wchar_t v[]) {
    IVaultNodeSetString(kText_1, &text_1, v, (unsigned)-1);
}

//============================================================================
void NetVaultNode::SetText_2 (const wchar_t v[]) {
    IVaultNodeSetString(kText_2, &text_2, v, (unsigned)-1);
}

//============================================================================
void NetVaultNode::SetBlob_1 (const uint8_t v[], uint32_t len) {
    IVaultNodeSetBlob(kBlob_1, &blob_1, &blob_1Length, v, len);
}

//============================================================================
void NetVaultNode::SetBlob_2 (const uint8_t v[], uint32_t len) {
    IVaultNodeSetBlob(kBlob_2, &blob_2, &blob_2Length, v, len);
}


/*****************************************************************************
*
*   CSrvPackBuffer
*
***/

//============================================================================
CSrvPackBuffer::CSrvPackBuffer (unsigned bytes) {
    m_data = (uint8_t *)malloc(bytes);
    m_pos  = m_data;
    m_end  = m_pos + bytes;
}

//============================================================================
void * CSrvPackBuffer::Alloc (unsigned bytes) {
    ASSERT((signed) bytes >= 0);
    ASSERT(m_pos + bytes <= m_end);

    uint8_t * pos = m_pos;
    m_pos += bytes;
    return pos;
}

//============================================================================
void CSrvPackBuffer::AddData (const void * ptr, unsigned bytes) {
    memcpy(Alloc(bytes), ptr, bytes);
}

//============================================================================
void CSrvPackBuffer::AddString (const wchar_t str[]) {
    AddData(str, StrBytes(str));
}

//============================================================================
void CSrvPackBuffer::AddDWordArray (const uint32_t * arr, unsigned count) {
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
    m_pos = (const uint8_t *) buffer;
    m_end = m_pos + count;
}

//============================================================================
const void * CSrvUnpackBuffer::GetData (unsigned bytes) {
    for (;;) {
        const uint8_t * result = m_pos;
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
const wchar_t * CSrvUnpackBuffer::GetString () {
    
     if (m_end) {  
          const wchar_t * end = (const wchar_t *) (m_end - sizeof(wchar_t) + 1);  
          for (const wchar_t * cur = (const wchar_t *) m_pos; cur < end; ) {  
               if (*cur++)  
                    continue;  
  
               const wchar_t * pos   = (const wchar_t *) m_pos;  
               m_pos               = (const uint8_t *) cur;  
               return pos;  
          }  
     }  
  
     m_end = NULL;  
     return NULL;   
}

//============================================================================
const uint32_t * CSrvUnpackBuffer::GetDWordArray (unsigned count) {
    // Don't let large counts cause pointer wrap
    if (count & 0x00ffffff)
        return (const uint32_t *)GetData(count * sizeof(uint32_t));

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
