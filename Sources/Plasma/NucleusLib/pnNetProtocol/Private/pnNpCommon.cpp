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

#include "pnNetProtocol/pnNetProtocol.h"

#include "pnUtils/pnUtStr.h"
#include "pnUUID/pnUUID.h"

#include <string>

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
inline void IReadValue (T * value, uint8_t ** buffer, unsigned * bufsz) {
    ASSERT(*bufsz >= sizeof(T));
    *value = *(T *)*buffer;
    *buffer += sizeof(T);
    *bufsz -= sizeof(T);
}

//============================================================================
template <typename T>
inline void IReadArray (T ** buf, unsigned * elems, uint8_t ** buffer, unsigned * bufsz) {
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
inline void IReadString (T ** buf, uint8_t ** buffer, unsigned * bufsz) {
    unsigned elems;
    IReadArray(buf, &elems, buffer, bufsz);
    // ensure the string is null-terminated
    if (elems)
        (*buf)[elems-1] = 0;
}

//============================================================================
template <typename T>
inline void IWriteValue (const T & value, std::vector<uint8_t> * buffer) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "IWriteValue can only be used on trivially copyable types");
    const size_t endPos = buffer->size();
    buffer->resize(endPos + sizeof(T));
    memcpy(buffer->data() + endPos, &value, sizeof(T));
}

//============================================================================
template <typename T>
inline void IWriteArray (const T buf[], unsigned elems, std::vector<uint8_t> * buffer) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "IWriteArray can only be used on trivially copyable types");
    unsigned bytes = elems * sizeof(T);
    IWriteValue(bytes, buffer);
    const size_t endPos = buffer->size();
    buffer->resize(endPos + bytes);
    memcpy(buffer->data() + endPos, buf, bytes);
}

//============================================================================
template <typename T>
inline void IWriteString (const T str[], std::vector<uint8_t> * buffer) {
    IWriteArray(str, std::char_traits<T>::length(str) + 1, buffer);
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

    char16_t* tempstr = nullptr;

    IReadValue(&scoreId, &buffer, &bufsz);
    IReadValue(&ownerId, &buffer, &bufsz);
    IReadValue(&createdTime, &buffer, &bufsz);
    IReadValue(&gameType, &buffer, &bufsz);
    IReadValue(&value, &buffer, &bufsz);
    IReadString(&tempstr, &buffer, &bufsz);

    gameName = ST::string::from_utf16(tempstr);
    free(tempstr);

    if (end)
        *end = buffer;

    return (unsigned)(buffer - start);
}

//============================================================================
unsigned NetGameScore::Write(std::vector<uint8_t> * buffer) const {

    const size_t pos = buffer->size();

    IWriteValue(scoreId, buffer);
    IWriteValue(ownerId, buffer);
    IWriteValue(createdTime, buffer);
    IWriteValue(gameType, buffer);
    IWriteValue(value, buffer);
    IWriteString(gameName.to_wchar().data(), buffer);

    return buffer->size() - pos;
}

//============================================================================
void NetGameScore::CopyFrom(const NetGameScore & score) {
    scoreId     = score.scoreId;
    ownerId     = score.ownerId;
    createdTime = score.createdTime;
    gameType    = score.gameType;
    value       = score.value;
    gameName    = score.gameName;
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

    char16_t* tempstr = nullptr;

    IReadValue(&rank, &buffer, &bufsz);
    IReadValue(&score, &buffer, &bufsz);
    IReadString(&tempstr, &buffer, &bufsz);

    StrCopy(name, tempstr, std::size(name));
    free(tempstr);

    if (end)
        *end = buffer;

    return (unsigned)(buffer - start);
}

//============================================================================
unsigned NetGameRank::Write(std::vector<uint8_t> * buffer) const {

    const size_t pos = buffer->size();

    IWriteValue(rank, buffer);
    IWriteValue(score, buffer);
    IWriteString(name, buffer);

    return buffer->size() - pos;
}

//============================================================================
void NetGameRank::CopyFrom(const NetGameRank & fromRank) {
    rank        = fromRank.rank;
    score       = fromRank.score;
    StrCopy(name, fromRank.name, std::size(name));
}

/*****************************************************************************
*
*   NetVaultNode
*
***/

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
inline void IZero(T& dest)
{
    dest = 0;
}

template<>
inline void IZero<ST::string>(ST::string& dest)
{
    dest = ST::string();
}

template<>
inline void IZero<plUUID>(plUUID& dest)
{
    dest = kNilUuid;
}

template<>
inline void IZero<std::vector<uint8_t>>(std::vector<uint8_t>& blob)
{
    blob.clear();
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
#define COMPARE_ISTRING(field) if (k##field == bit && f##field.compare_i(rhs->f##field) != 0) return false;
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
inline bool IRead(const uint8_t*& buf, size_t& bufsz, T& dest)
{
    if (bufsz < sizeof(T)) {
        return false;
    }
    const T* ptr = reinterpret_cast<const T*>(buf);
    dest = *ptr;
    buf += sizeof(T);
    bufsz -= sizeof(T);
    return true;
}

template<>
inline bool IRead<ST::string>(const uint8_t*& buf, size_t& bufsz, ST::string& dest)
{
    uint32_t size;
    if (
        !IRead(buf, bufsz, size) || bufsz < size
        // Ensure even byte count
        || size % sizeof(char16_t) != 0
        // String must contain at least one character (the terminator)
        || size < sizeof(char16_t)
    ) {
        return false;
    }
    uint32_t nChars = (size / sizeof(char16_t)) - 1;

    ST::utf16_buffer str;
    str.allocate(nChars);
    memcpy(str.data(), buf, nChars * sizeof(char16_t));
    dest = ST::string::from_utf16(str);
    buf += size;
    bufsz -= size;
    return true;
}

template<>
inline bool IRead<std::vector<uint8_t>>(const uint8_t*& buf, size_t& bufsz, std::vector<uint8_t>& blob)
{
    uint32_t size;
    if (!IRead(buf, bufsz, size) || bufsz < size) {
        return false;
    }

    blob.resize(size);
    memcpy(blob.data(), buf, size);
    buf += size;
    bufsz -= size;
    return true;
}

bool NetVaultNode::Read(const uint8_t* buf, size_t bufsz)
{
    if (!IRead(buf, bufsz, fUsedFields)) {
        return false;
    }

#define READ(field) if (fUsedFields & k##field) if (!IRead(buf, bufsz, f##field)) {fDirtyFields = 0; return false;}
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
    return bufsz == 0;
}

//============================================================================
template<typename T>
inline void IWrite(std::vector<uint8_t>* buffer, const T& value)
{
    static_assert(std::is_trivially_copyable_v<T>,
                  "IWrite can only be used on trivially copyable types");
    const size_t oldSize = buffer->size();
    buffer->resize(oldSize + sizeof(T));
    memcpy(buffer->data() + oldSize, &value, sizeof(T));
}

template<>
inline void IWrite<ST::string>(std::vector<uint8_t>* buffer, const ST::string& value)
{
    ST::utf16_buffer utf16 = value.to_utf16();
    uint32_t strsz = (utf16.size() + 1) * sizeof(char16_t);
    IWrite(buffer, strsz);

    const size_t oldSize = buffer->size();
    buffer->resize(oldSize + strsz);
    memcpy(buffer->data() + oldSize, utf16.data(), strsz);
}

template<>
inline void IWrite<std::vector<uint8_t>>(std::vector<uint8_t>* buffer,
                                         const std::vector<uint8_t>& blob)
{
    IWrite(buffer, static_cast<uint32_t>(blob.size()));

    if (!blob.empty()) {
        const size_t oldSize = buffer->size();
        buffer->resize(oldSize + blob.size());
        memcpy(buffer->data() + oldSize, blob.data(), blob.size());
    }
}

void NetVaultNode::Write(std::vector<uint8_t>* buf, uint32_t ioFlags)
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
void NetVaultNode::ISetVaultBlob(uint64_t bits, std::vector<uint8_t>& blob,
                                 const uint8_t* buf, size_t size)
{
    blob.resize(size);
    memcpy(blob.data(), buf, size);

    fUsedFields |= bits;
    fDirtyFields |= bits;
}
