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
NetVaultNode::Blob::Blob(const Blob& rhs)
{
    buffer = new uint8_t[rhs.size];
    size = rhs.size;
    memcpy(buffer, rhs.buffer, rhs.size);
}

//============================================================================
NetVaultNode::Blob::Blob(Blob&& rhs)
{
    size = rhs.size;
    buffer = rhs.buffer;
    rhs.size = 0;
    rhs.buffer = nullptr;
}

//============================================================================
void NetVaultNode::Blob::operator=(const Blob& rhs)
{
    if (size != rhs.size) {
        delete[] buffer;
        buffer = new uint8_t[rhs.size];
        size = rhs.size;
    }
    memcpy(buffer, rhs.buffer, rhs.size);
}

//============================================================================
void NetVaultNode::Blob::operator=(Blob&& rhs)
{
    delete[] buffer;
    size = rhs.size;
    buffer = rhs.buffer;
    rhs.size = 0;
    rhs.buffer = nullptr;
}

//============================================================================
bool NetVaultNode::Blob::operator==(const Blob& rhs) const
{
    if (size == rhs.size)
        return memcmp(buffer, rhs.buffer, size) == 0;
    return false;
}

//============================================================================
void NetVaultNode::Clear()
{
    // Sneaky -- we're just going to set the fields to empty.
    // If a field is neither used nor dirty, it doesn't matter what value it actually has.
    fUsedFields = 0;
    fDirtyFields = 0;
    fRevision = kNilUuid;
}

//============================================================================
template<typename T>
static void IZero(T& dest)
{
    dest = 0;
}

template<>
static void IZero<plString>(plString& dest)
{
    dest = "";
}

template<>
static void IZero<plUUID>(plUUID& dest)
{
    dest = kNilUuid;
}

template<>
static void IZero<NetVaultNode::Blob>(NetVaultNode::Blob& blob)
{
    delete[] blob.buffer;
    blob.buffer = nullptr;
    blob.size = 0;
}

void NetVaultNode::CopyFrom(const NetVaultNode* node)
{
    fUsedFields = node->fUsedFields;
    fDirtyFields = node->fDirtyFields;
    fRevision = node->fRevision;

#define COPYORZERO(field) \
    if (fUsedFields & k##field) \
        f##field = node->f##field; \
    else \
        IZero(f##field);

    COPYORZERO(NodeId);
    COPYORZERO(CreateTime);
    COPYORZERO(ModifyTime);
    COPYORZERO(CreateAgeName);
    COPYORZERO(CreateAgeUuid);
    COPYORZERO(CreatorAcct);
    COPYORZERO(CreatorId);
    COPYORZERO(NodeType);
    COPYORZERO(Int32_1);
    COPYORZERO(Int32_2);
    COPYORZERO(Int32_3);
    COPYORZERO(Int32_4);
    COPYORZERO(UInt32_1);
    COPYORZERO(UInt32_2);
    COPYORZERO(UInt32_3);
    COPYORZERO(UInt32_4);
    COPYORZERO(Uuid_1);
    COPYORZERO(Uuid_2);
    COPYORZERO(Uuid_3);
    COPYORZERO(Uuid_4);
    COPYORZERO(String64_1);
    COPYORZERO(String64_2);
    COPYORZERO(String64_3);
    COPYORZERO(String64_4);
    COPYORZERO(String64_5);
    COPYORZERO(String64_6);
    COPYORZERO(IString64_1);
    COPYORZERO(IString64_2);
    COPYORZERO(Text_1);
    COPYORZERO(Text_2);
    COPYORZERO(Blob_1);
    COPYORZERO(Blob_2);

#undef COPYORZERO
}

//============================================================================
bool NetVaultNode::Matches(const NetVaultNode* rhs) const
{
    for (uint64_t bit = 1; bit; bit <<= 1) {
        // If we have tested all fields on the other node, we obviously match.
        if (bit > rhs->fUsedFields)
            return true;

         // If the other node does not have the field, then continue to next field
        if (!(bit & rhs->fUsedFields))
            continue;

        // If we don't have this field, but the other node does, we are obviously not the same.
        if (!(bit & fUsedFields))
            return false;

#define COMPARE(field) if (k##field == bit && f##field != rhs->f##field) return false;
#define COMPARE_ISTRING(field) if (k##field == bit && f##field.CompareI(rhs->f##field) != 0) return false;
    COMPARE(NodeId);
    COMPARE(CreateTime);
    COMPARE(ModifyTime);
    COMPARE_ISTRING(CreateAgeName);
    COMPARE(CreateAgeUuid);
    COMPARE(CreatorAcct);
    COMPARE(CreatorId);
    COMPARE(NodeType);
    COMPARE(Int32_1);
    COMPARE(Int32_2);
    COMPARE(Int32_3);
    COMPARE(Int32_4);
    COMPARE(UInt32_1);
    COMPARE(UInt32_2);
    COMPARE(UInt32_3);
    COMPARE(UInt32_4);
    COMPARE(Uuid_1);
    COMPARE(Uuid_2);
    COMPARE(Uuid_3);
    COMPARE(Uuid_4);
    COMPARE(String64_1);
    COMPARE(String64_2);
    COMPARE(String64_3);
    COMPARE(String64_4);
    COMPARE(String64_5);
    COMPARE(String64_6);
    COMPARE_ISTRING(IString64_1);
    COMPARE_ISTRING(IString64_2);
    COMPARE(Text_1);
    COMPARE(Text_2);
    COMPARE(Blob_1);
    COMPARE(Blob_2);
#undef COMPARE
#undef COMPARE_ISTRING
    }

    // We should never get here, but this silences a warning.
    return true;
}

//============================================================================
template<typename T>
static void IRead(const uint8_t*& buf, T& dest)
{
    const T* ptr = reinterpret_cast<const T*>(buf);
    dest = *ptr;
    buf += sizeof(T);
}

template<>
static void IRead<plString>(const uint8_t*& buf, plString& dest)
{
    uint32_t size = *(reinterpret_cast<const uint32_t*>(buf));
    uint32_t nChars = (size / sizeof(uint16_t)) - 1;
    buf += sizeof(uint32_t);

    plStringBuffer<uint16_t> str;
    uint16_t* theStrBuffer = str.CreateWritableBuffer(nChars);
    memcpy(theStrBuffer, buf, size);
    theStrBuffer[nChars] = 0;
    dest = plString::FromUtf16(str);
    buf += size;
}

template<>
static void IRead<NetVaultNode::Blob>(const uint8_t*& buf, NetVaultNode::Blob& blob)
{
    blob.size = *(reinterpret_cast<const uint32_t*>(buf));
    buf += sizeof(uint32_t);

    delete[] blob.buffer;
    blob.buffer = new uint8_t[blob.size];
    memcpy(blob.buffer, buf, blob.size);
    buf += blob.size;
}

void NetVaultNode::Read(const uint8_t* buf, size_t size)
{
    fUsedFields= *(reinterpret_cast<const uint64_t*>(buf));
    buf += sizeof(uint64_t);

#define READ(field) if (fUsedFields & k##field) IRead(buf, f##field);
    READ(NodeId);
    READ(CreateTime);
    READ(ModifyTime);
    READ(CreateAgeName);
    READ(CreateAgeUuid);
    READ(CreatorAcct);
    READ(CreatorId);
    READ(NodeType);
    READ(Int32_1);
    READ(Int32_2);
    READ(Int32_3);
    READ(Int32_4);
    READ(UInt32_1);
    READ(UInt32_2);
    READ(UInt32_3);
    READ(UInt32_4);
    READ(Uuid_1);
    READ(Uuid_2);
    READ(Uuid_3);
    READ(Uuid_4);
    READ(String64_1);
    READ(String64_2);
    READ(String64_3);
    READ(String64_4);
    READ(String64_5);
    READ(String64_6);
    READ(IString64_1);
    READ(IString64_2);
    READ(Text_1);
    READ(Text_2);
    READ(Blob_1);
    READ(Blob_2);
#undef READ

    fDirtyFields = 0;
}

//============================================================================
template<typename T>
static void IWrite(ARRAY(uint8_t)* buffer, const T& value)
{
    uint8_t* ptr = buffer->New(sizeof(T));
    memcpy(ptr, &value, sizeof(T));
}

template<>
static void IWrite<plString>(ARRAY(uint8_t)* buffer, const plString& value)
{
    plStringBuffer<uint16_t> utf16 = value.ToUtf16();
    uint32_t strsz = (utf16.GetSize() + 1) * 2;
    IWrite(buffer, strsz);

    uint8_t* ptr = buffer->New(strsz);
    memcpy(ptr, utf16.GetData(), strsz);
}

template<>
static void IWrite<NetVaultNode::Blob>(ARRAY(uint8_t)* buffer, const NetVaultNode::Blob& blob)
{
    IWrite(buffer, static_cast<uint32_t>(blob.size));

    if (blob.size > 0) {
        uint8_t* ptr = buffer->New(blob.size);
        memcpy(ptr, blob.buffer, blob.size);
    }
}

void NetVaultNode::Write(ARRAY(uint8_t)* buf, uint32_t ioFlags)
{
    uint64_t flags = fUsedFields;
    if (ioFlags & kDirtyNodeType)
        fDirtyFields |= kNodeType;
    if (ioFlags & kDirtyString64_1)
        fDirtyFields |= kString64_1;
    if (ioFlags & kDirtyOnly)
        flags &= fDirtyFields;
    IWrite(buf, flags);

#define WRITE(field) if (flags & k##field) IWrite(buf, f##field);
    WRITE(NodeId);
    WRITE(CreateTime);
    WRITE(ModifyTime);
    WRITE(CreateAgeName);
    WRITE(CreateAgeUuid);
    WRITE(CreatorAcct);
    WRITE(CreatorId);
    WRITE(NodeType);
    WRITE(Int32_1);
    WRITE(Int32_2);
    WRITE(Int32_3);
    WRITE(Int32_4);
    WRITE(UInt32_1);
    WRITE(UInt32_2);
    WRITE(UInt32_3);
    WRITE(UInt32_4);
    WRITE(Uuid_1);
    WRITE(Uuid_2);
    WRITE(Uuid_3);
    WRITE(Uuid_4);
    WRITE(String64_1);
    WRITE(String64_2);
    WRITE(String64_3);
    WRITE(String64_4);
    WRITE(String64_5);
    WRITE(String64_6);
    WRITE(IString64_1);
    WRITE(IString64_2);
    WRITE(Text_1);
    WRITE(Text_2);
    WRITE(Blob_1);
    WRITE(Blob_2);
#undef WRITE

    if (ioFlags & kClearDirty)
        fDirtyFields = 0;
}

//============================================================================
void NetVaultNode::ISetVaultBlob(uint64_t bits, NetVaultNode::Blob& blob, const uint8_t* buf, size_t size)
{
    delete[] blob.buffer;
    blob.buffer = new uint8_t[size];
    blob.size = size;
    memcpy(blob.buffer, buf, size);

    fUsedFields |= bits;
    fDirtyFields |= bits;
}
