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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPCOMMON_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpCommon.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPCOMMON_H

#include "pnNetBase/pnNbConst.h"
#include "pnUUID/pnUUID.h"

#include "hsRefCnt.h"


/*****************************************************************************
*
*   Client message field types
*
***/

#ifdef USES_NETCLI

#include "pnEncryption/plChecksum.h"

const NetMsgField kNetMsgFieldAccountName   = NET_MSG_FIELD_STRING(kMaxAccountNameLength);
const NetMsgField kNetMsgFieldPlayerName    = NET_MSG_FIELD_STRING(kMaxPlayerNameLength);
const NetMsgField kNetMsgFieldShaDigest     = NET_MSG_FIELD_RAW_DATA(sizeof(ShaDigest));
const NetMsgField kNetMsgFieldUuid          = NET_MSG_FIELD_DATA(sizeof(plUUID));
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

#pragma pack(push,1)
struct SrvPlayerInfo {
    unsigned    playerInt;
    char16_t    playerName[kMaxPlayerNameLength];
    char16_t    avatarShape[kMaxVaultNodeStringLength];
    unsigned    explorer;
};
#pragma pack(pop)


/*****************************************************************************
*
*   NetAgeInfo
*
***/

struct NetAgeInfo {
    plUUID        ageInstId;
    char16_t      ageFilename[kMaxAgeNameLength];
    char16_t      ageInstName[kMaxAgeNameLength];
    char16_t      ageUserName[kMaxAgeNameLength];
    char16_t      ageDesc[1024];
    uint32_t      ageSequenceNumber;
    uint32_t      ageLanguage;
    uint32_t      population;         // only used with GetPublicAgeList query results
    uint32_t      currentPopulation;  // only used with GetPublicAgeList query results
};

/*****************************************************************************
*
*   NetGameScore
*
***/

struct NetGameScore {
    unsigned    scoreId;
    unsigned    ownerId;
    uint32_t    createdTime;
    ST::string  gameName;
    unsigned    gameType;
    int         value;

    unsigned Read (const uint8_t inbuffer[], unsigned bufsz, uint8_t** end = nullptr); // returns number of bytes read
    unsigned Write (std::vector<uint8_t> * buffer) const;                             // returns number of bytes written

    void CopyFrom (const NetGameScore & score);
};

/*****************************************************************************
*
*   NetGameRank
*
***/

struct NetGameRank {
    unsigned    rank;
    int         score;
    char16_t    name[kMaxPlayerNameLength];

    unsigned Read (const uint8_t inbuffer[], unsigned bufsz, uint8_t** end = nullptr); // returns number of bytes read
    unsigned Write (std::vector<uint8_t> * buffer) const;                             // returns number of bytes written

    void CopyFrom (const NetGameRank & fromRank);
};

/*****************************************************************************
*
*   Server vault structures
*
***/

class NetVaultNode : public hsRefCnt
{
protected:
    enum NodeFields : uint32_t
    {
        kNodeId = (1u << 0),
        kCreateTime = (1u << 1),
        kModifyTime = (1u << 2),
        kCreateAgeName = (1u << 3),
        kCreateAgeUuid = (1u << 4),
        kCreatorAcct = (1u << 5),
        kCreatorId = (1u << 6),
        kNodeType = (1u << 7),
        kInt32_1 = (1u << 8),
        kInt32_2 = (1u << 9),
        kInt32_3 = (1u << 10),
        kInt32_4 = (1u << 11),
        kUInt32_1 = (1u << 12),
        kUInt32_2 = (1u << 13),
        kUInt32_3 = (1u << 14),
        kUInt32_4 = (1u << 15),
        kUuid_1 = (1u << 16),
        kUuid_2 = (1u << 17),
        kUuid_3 = (1u << 18),
        kUuid_4 = (1u << 19),
        kString64_1 = (1u << 20),
        kString64_2 = (1u << 21),
        kString64_3 = (1u << 22),
        kString64_4 = (1u << 23),
        kString64_5 = (1u << 24),
        kString64_6 = (1u << 25),
        kIString64_1 = (1u << 26),
        kIString64_2 = (1u << 27),
        kText_1 = (1u << 28),
        kText_2 = (1u << 29),
        kBlob_1 = (1u << 30),
        kBlob_2 = (1u << 31),

        kValidFields = (kNodeId | kCreateTime | kModifyTime | kCreateAgeName | kCreateAgeUuid |
                        kCreatorAcct | kCreatorId | kNodeType | kInt32_1 | kInt32_2 | kInt32_3 |
                        kInt32_4 | kUInt32_1 | kUInt32_2 | kUInt32_3 | kUInt32_3 | kUInt32_4 |
                        kUuid_1 | kUuid_2 | kUuid_3 | kUuid_4 | kString64_1 | kString64_2 |
                        kString64_3 | kString64_4 | kString64_5 | kString64_6 | kIString64_1 |
                        kIString64_2 | kText_1 | kText_2 | kBlob_1 | kBlob_2)
    };

private:
    uint64_t fUsedFields;
    uint64_t fDirtyFields;
    plUUID fRevision;

    uint32_t fNodeId;
    uint32_t fCreateTime;
    uint32_t fModifyTime;
    ST::string fCreateAgeName;
    plUUID   fCreateAgeUuid;
    plUUID   fCreatorAcct;
    uint32_t fCreatorId;
    uint32_t fNodeType;
    int32_t  fInt32_1;
    int32_t  fInt32_2;
    int32_t  fInt32_3;
    int32_t  fInt32_4;
    uint32_t fUInt32_1;
    uint32_t fUInt32_2;
    uint32_t fUInt32_3;
    uint32_t fUInt32_4;
    plUUID   fUuid_1;
    plUUID   fUuid_2;
    plUUID   fUuid_3;
    plUUID   fUuid_4;
    ST::string fString64_1;
    ST::string fString64_2;
    ST::string fString64_3;
    ST::string fString64_4;
    ST::string fString64_5;
    ST::string fString64_6;
    ST::string fIString64_1;
    ST::string fIString64_2;
    ST::string fText_1;
    ST::string fText_2;
    std::vector<uint8_t> fBlob_1;
    std::vector<uint8_t> fBlob_2;

    template<typename T>
    inline void ISetVaultField(uint64_t bits, T& field, T value)
    {
        field = value;
        fUsedFields |= bits;
        fDirtyFields |= bits;
    }

    template<typename T>
    inline void ISetVaultField_NoDirty(uint64_t bits, T& field, T value)
    {
        field = value;
        fUsedFields |= bits;
    }

    void ISetVaultBlob(uint64_t bits, std::vector<uint8_t>& blob,
                       const uint8_t* buf, size_t size);

public:
    enum IOFlags
    {
        /** Only write fields marked dirty */
        kDirtyOnly = (1 << 0),

        /** Clears the dirty flag on fields written to the stream */
        kClearDirty = (1 << 1),

        /** Indicates that we should mark the NodeType fields as dirty before writing */
        kDirtyNodeType = (1 << 2),

        /** Indicates that we should mark the String64_1 field as dirty before writing */
        kDirtyString64_1 = (1 << 3),
    };

public:
    NetVaultNode()
        : fNodeId(0), fCreateTime(0), fModifyTime(0), fCreatorId(0), fNodeType(0), fInt32_1(0),
          fInt32_2(0), fInt32_3(0), fInt32_4(0), fUInt32_1(0), fUInt32_2(0), fUInt32_3(0),
          fUInt32_4(0), fUsedFields(0), fDirtyFields(0)
    { }

    /** Clears this NetVaultNode for subsequent usage */
    void Clear();

    /** Copies data from \a node to this NetVaultNode */
    void CopyFrom(const NetVaultNode* node);

    bool Matches(const NetVaultNode* rhs) const;

    bool Read(const uint8_t* buf, size_t bufsz);
    void Write(std::vector<uint8_t>* buf, uint32_t ioFlags=0);

protected:
    uint64_t GetFieldFlags() const { return fUsedFields; }

public:
    bool IsDirty() const { return fDirtyFields != 0; }
    bool IsUsed() const { return fUsedFields != 0; }

    plUUID GetRevision() const { return fRevision; }
    void GenerateRevision() { fRevision = plUUID::Generate(); }

    uint32_t GetNodeId() const { return fNodeId; }
    uint32_t GetCreateTime() const { return fCreateTime; }
    uint32_t GetModifyTime() const { return fModifyTime; }
    ST::string GetCreateAgeName() const { return fCreateAgeName; }
    plUUID GetCreateAgeUuid() const { return fCreateAgeUuid; }
    plUUID GetCreatorAcct() const { return fCreatorAcct; }
    uint32_t GetCreatorId() const { return fCreatorId; }
    uint32_t GetNodeType() const { return fNodeType; }
    int32_t GetInt32_1() const { return fInt32_1; }
    int32_t GetInt32_2() const { return fInt32_2; }
    int32_t GetInt32_3() const { return fInt32_3; }
    int32_t GetInt32_4() const { return fInt32_4; }
    uint32_t GetUInt32_1() const { return fUInt32_1; }
    uint32_t GetUInt32_2() const { return fUInt32_2; }
    uint32_t GetUInt32_3() const { return fUInt32_3; }
    uint32_t GetUInt32_4() const { return fUInt32_4; }
    plUUID GetUuid_1() const { return fUuid_1; }
    plUUID GetUuid_2() const { return fUuid_2; }
    plUUID GetUuid_3() const { return fUuid_3; }
    plUUID GetUuid_4() const { return fUuid_4; }
    ST::string GetString64_1() const { return fString64_1; }
    ST::string GetString64_2() const { return fString64_2; }
    ST::string GetString64_3() const { return fString64_3; }
    ST::string GetString64_4() const { return fString64_4; }
    ST::string GetString64_5() const { return fString64_5; }
    ST::string GetString64_6() const { return fString64_6; }
    ST::string GetIString64_1() const { return fIString64_1; }
    ST::string GetIString64_2() const { return fIString64_2; }
    ST::string GetText_1() const { return fText_1; }
    ST::string GetText_2() const { return fText_2; }
    const std::vector<uint8_t>& GetBlob_1() const { return fBlob_1; }
    const std::vector<uint8_t>& GetBlob_2() const { return fBlob_2; }

public:
    void SetNodeId(uint32_t value) { ISetVaultField(kNodeId, fNodeId, value); }
    void SetNodeId_NoDirty(uint32_t value) { ISetVaultField_NoDirty(kNodeId, fNodeId, value); }
    void SetCreateTime(uint32_t value) { ISetVaultField(kCreateTime, fCreateTime, value); }
    void SetModifyTime(uint32_t value) { ISetVaultField(kModifyTime, fModifyTime, value); }
    void SetCreateAgeName(const ST::string& value) { ISetVaultField(kCreateAgeName, fCreateAgeName, value); }
    void SetCreateAgeUuid(const plUUID& value) { ISetVaultField(kCreateAgeUuid, fCreateAgeUuid, value); }
    void SetCreatorAcct(const plUUID& value) { ISetVaultField(kCreatorAcct, fCreatorAcct, value); }
    void SetCreatorId(uint32_t value) { ISetVaultField(kCreatorId, fCreatorId, value); }
    void SetNodeType(uint32_t value) { ISetVaultField(kNodeType, fNodeType, value); }
    void SetInt32_1(int32_t value) { ISetVaultField(kInt32_1, fInt32_1, value); }
    void SetInt32_2(int32_t value) { ISetVaultField(kInt32_2, fInt32_2, value); }
    void SetInt32_3(int32_t value) { ISetVaultField(kInt32_3, fInt32_3, value); }
    void SetInt32_4(int32_t value) { ISetVaultField(kInt32_4, fInt32_4, value); }
    void SetUInt32_1(uint32_t value) { ISetVaultField(kUInt32_1, fUInt32_1, value); }
    void SetUInt32_2(uint32_t value) { ISetVaultField(kUInt32_2, fUInt32_2, value); }
    void SetUInt32_3(uint32_t value) { ISetVaultField(kUInt32_3, fUInt32_3, value); }
    void SetUInt32_4(uint32_t value) { ISetVaultField(kUInt32_4, fUInt32_4, value); }
    void SetUuid_1(const plUUID& value) { ISetVaultField(kUuid_1, fUuid_1, value); }
    void SetUuid_2(const plUUID& value) { ISetVaultField(kUuid_2, fUuid_2, value); }
    void SetUuid_3(const plUUID& value) { ISetVaultField(kUuid_3, fUuid_3, value); }
    void SetUuid_4(const plUUID& value) { ISetVaultField(kUuid_4, fUuid_4, value); }
    void SetString64_1(const ST::string& value) { ISetVaultField(kString64_1, fString64_1, value); }
    void SetString64_2(const ST::string& value) { ISetVaultField(kString64_2, fString64_2, value); }
    void SetString64_3(const ST::string& value) { ISetVaultField(kString64_3, fString64_3, value); }
    void SetString64_4(const ST::string& value) { ISetVaultField(kString64_4, fString64_4, value); }
    void SetString64_5(const ST::string& value) { ISetVaultField(kString64_5, fString64_5, value); }
    void SetString64_6(const ST::string& value) { ISetVaultField(kString64_6, fString64_6, value); }
    void SetIString64_1(const ST::string& value) { ISetVaultField(kIString64_1, fIString64_1, value); }
    void SetIString64_2(const ST::string& value) { ISetVaultField(kIString64_2, fIString64_2, value); }
    void SetText_1(const ST::string& value) { ISetVaultField(kText_1, fText_1, value); }
    void SetText_2(const ST::string& value) { ISetVaultField(kText_2, fText_2, value); }

    void SetBlob_1(const uint8_t* buf, size_t size) { ISetVaultBlob(kBlob_1, fBlob_1, buf, size); }
    void SetBlob_2(const uint8_t* buf, size_t size) { ISetVaultBlob(kBlob_2, fBlob_2, buf, size); }
};

//============================================================================
// NetVaultNodeRef (packed because is sent over wire directly)
//============================================================================
#pragma pack(push,1)
struct NetVaultNodeRef {
    unsigned    parentId;
    unsigned    childId;
    unsigned    ownerId;
    bool        seen;
};
#pragma pack(pop)
