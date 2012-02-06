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

#pragma pack(push,1)
struct SrvPlayerInfo {
    unsigned    playerInt;
    wchar_t       playerName[kMaxPlayerNameLength];
    wchar_t       avatarShape[kMaxVaultNodeStringLength];
    unsigned    explorer;
};
#pragma pack(pop)


/*****************************************************************************
*
*   NetAgeInfo
*
***/

struct NetAgeInfo {
    Uuid        ageInstId;
    wchar_t       ageFilename[kMaxAgeNameLength];
    wchar_t       ageInstName[kMaxAgeNameLength];
    wchar_t       ageUserName[kMaxAgeNameLength];
    wchar_t       ageDesc[1024];
    uint32_t       ageSequenceNumber;
    uint32_t       ageLanguage;
    uint32_t       population;         // only used with GetPublicAgeList query results
    uint32_t       currentPopulation;  // only used with GetPublicAgeList query results
};

/*****************************************************************************
*
*   NetGameScore
*
***/

struct NetGameScore {
    unsigned    scoreId;
    unsigned    ownerId;
    uint32_t      createdTime;
    wchar_t       gameName[kMaxGameScoreNameLength];
    unsigned    gameType;
    int         value;

    unsigned Read (const uint8_t inbuffer[], unsigned bufsz, uint8_t** end = nil);    // returns number of bytes read
    unsigned Write (ARRAY(uint8_t) * buffer) const;                                // returns number of bytes written

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
    wchar_t       name[kMaxPlayerNameLength];

    unsigned Read (const uint8_t inbuffer[], unsigned bufsz, uint8_t** end = nil);    // returns number of bytes read
    unsigned Write (ARRAY(uint8_t) * buffer) const;                                // returns number of bytes written

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
        kRwDirtyOnly    = 1<<0, // READ : No meaning
                                // WRITE: Only write fields marked dirty
        kRwUpdateDirty  = 1<<1, // READ : Set dirty flag on fields read from stream
                                // WRITE: Clear dirty flag on fields written to stream
    };
    
    enum CopyOptions {
        kCopySetDirty   = 1<<0,     // set dirty flag on changed dst fields
        kCopyOverwrite  = 1<<1,     // overwrite fields for which dst node already has values
        kCopyClear      = 1<<2,     // clear dst fields for which src node does not have values
    };
    
    // These flag values must not change unless all servers are
    // simultaneously replaced, so basically forget it.
    static const uint64_t kNodeId          = (uint64_t)1<< 0;
    static const uint64_t kCreateTime      = (uint64_t)1<< 1;
    static const uint64_t kModifyTime      = (uint64_t)1<< 2;
    static const uint64_t kCreateAgeName   = (uint64_t)1<< 3;
    static const uint64_t kCreateAgeUuid   = (uint64_t)1<< 4;
    static const uint64_t kCreatorAcct     = (uint64_t)1<< 5;
    static const uint64_t kCreatorId       = (uint64_t)1<< 6;
    static const uint64_t kNodeType        = (uint64_t)1<< 7;
    static const uint64_t kInt32_1         = (uint64_t)1<< 8;
    static const uint64_t kInt32_2         = (uint64_t)1<< 9;
    static const uint64_t kInt32_3         = (uint64_t)1<<10;
    static const uint64_t kInt32_4         = (uint64_t)1<<11;
    static const uint64_t kUInt32_1        = (uint64_t)1<<12;
    static const uint64_t kUInt32_2        = (uint64_t)1<<13;
    static const uint64_t kUInt32_3        = (uint64_t)1<<14;
    static const uint64_t kUInt32_4        = (uint64_t)1<<15;
    static const uint64_t kUuid_1          = (uint64_t)1<<16;
    static const uint64_t kUuid_2          = (uint64_t)1<<17;
    static const uint64_t kUuid_3          = (uint64_t)1<<18;
    static const uint64_t kUuid_4          = (uint64_t)1<<19;
    static const uint64_t kString64_1      = (uint64_t)1<<20;
    static const uint64_t kString64_2      = (uint64_t)1<<21;
    static const uint64_t kString64_3      = (uint64_t)1<<22;
    static const uint64_t kString64_4      = (uint64_t)1<<23;
    static const uint64_t kString64_5      = (uint64_t)1<<24;
    static const uint64_t kString64_6      = (uint64_t)1<<25;
    static const uint64_t kIString64_1     = (uint64_t)1<<26;
    static const uint64_t kIString64_2     = (uint64_t)1<<27;
    // blobs always come last
    static const uint64_t kText_1          = (uint64_t)1<<28;
    static const uint64_t kText_2          = (uint64_t)1<<29;
    static const uint64_t kBlob_1          = (uint64_t)1<<30;
    static const uint64_t kBlob_2          = (uint64_t)1<<31;
    
    CCritSect   critsect;
    
    uint64_t       fieldFlags;
    uint64_t       dirtyFlags;
    
    Uuid        revisionId;

    // Treat these as read-only or node flag fields will become invalid 
    // Threaded apps: Must be accessed with node->critsect locked   
    unsigned    nodeId;
    unsigned    createTime;
    unsigned    modifyTime;
    wchar_t *   createAgeName;
    Uuid        createAgeUuid;
    Uuid        creatorAcct;    // accountId of node creator
    unsigned    creatorId;      // playerId of node creator
    unsigned    nodeType;
    int         int32_1;
    int         int32_2;
    int         int32_3;
    int         int32_4;
    unsigned    uint32_1;
    unsigned    uint32_2;
    unsigned    uint32_3;
    unsigned    uint32_4;
    Uuid        uuid_1;
    Uuid        uuid_2;
    Uuid        uuid_3;
    Uuid        uuid_4;
    wchar_t *   string64_1;
    wchar_t *   string64_2;
    wchar_t *   string64_3;
    wchar_t *   string64_4;
    wchar_t *   string64_5;
    wchar_t *   string64_6;
    wchar_t *   istring64_1;
    wchar_t *   istring64_2;
    wchar_t *   text_1;
    wchar_t *   text_2;
    uint8_t *   blob_1; uint32_t blob_1Length;
    uint8_t *   blob_2; uint32_t blob_2Length;
    
    NetVaultNode ();
    ~NetVaultNode ();

    // Threaded apps: Must be called with node->critsect locked 
    unsigned Read_LCS (const uint8_t buffer[], unsigned bufsz, unsigned rwOpts);   // returns number of bytes read
    unsigned Write_LCS (ARRAY(uint8_t) * buffer, unsigned rwOpts);                 // returns number of bytes written
    
    bool Matches (const NetVaultNode * other);
    void CopyFrom (const NetVaultNode * other, unsigned copyOpts);
    
    // Threaded apps: Must be called with node->critsect locked 
    void SetNodeId (unsigned v);
    void SetCreateTime (unsigned v);
    void SetModifyTime (unsigned v);
    void SetCreateAgeName (const wchar_t v[]);
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
    void SetString64_1 (const wchar_t v[]);
    void SetString64_2 (const wchar_t v[]);
    void SetString64_3 (const wchar_t v[]);
    void SetString64_4 (const wchar_t v[]);
    void SetString64_5 (const wchar_t v[]);
    void SetString64_6 (const wchar_t v[]);
    void SetIString64_1 (const wchar_t v[]);
    void SetIString64_2 (const wchar_t v[]);
    void SetText_1 (const wchar_t v[]);
    void SetText_2 (const wchar_t v[]);
    void SetBlob_1 (const uint8_t v[], unsigned len);
    void SetBlob_2 (const uint8_t v[], unsigned len);
    
    void SetText (uint64_t fieldFlag, const wchar_t v[]);
    void SetBlob (uint64_t fieldFlag, const uint8_t v[], unsigned len);

    // These are only here to aid macro expansions (naming case matches field flags)
    inline unsigned GetNodeId () const { return nodeId; }
    inline unsigned GetCreateTime () const { return createTime; }
    inline unsigned GetModifyTime () const { return modifyTime; }
    inline wchar_t * GetCreateAgeName () const { return createAgeName; }
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
    inline wchar_t * GetString64_1 () const { return string64_1; }
    inline wchar_t * GetString64_2 () const { return string64_2; }
    inline wchar_t * GetString64_3 () const { return string64_3; }
    inline wchar_t * GetString64_4 () const { return string64_4; }
    inline wchar_t * GetString64_5 () const { return string64_5; }
    inline wchar_t * GetString64_6 () const { return string64_6; }
    inline wchar_t * GetIString64_1 () const { return istring64_1; }
    inline wchar_t * GetIString64_2 () const { return istring64_2; }
    // no blob "getters"
};


//============================================================================
inline void IVaultNodeSetString (
    uint64_t        bit,
    NetVaultNode *  node,
    char **         pdst,
    const char      src[],
    unsigned        chars
) {
    free(*pdst);
    if (src && src[0])
        *pdst = StrDupLen(src, chars);
    else
        *pdst = StrDupLen("", chars);
    node->fieldFlags |= bit;
    node->dirtyFlags |= bit;
}

//============================================================================
inline void IVaultNodeSetString (
    uint64_t        bit,
    NetVaultNode *  node,
    wchar_t **      pdst,
    const wchar_t   src[],
    unsigned        chars
) {
    free(*pdst);
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
    uint64_t           bit,
    NetVaultNode *  node,
    T *             pdst,
    const T &       src
) {
    *pdst = src;
    node->fieldFlags |= bit;
    node->dirtyFlags |= bit;
}

//============================================================================
inline void IVaultNodeSetBlob (
    uint64_t        bit,
    NetVaultNode *  node,
    uint8_t **      pdst,
    unsigned *      pdstLen,
    const uint8_t   src[],
    unsigned        srcLen
) {
    free(*pdst);
    if (src) {
        *pdst = (uint8_t*)malloc(srcLen);
        memcpy(*pdst, src, srcLen);
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
        void *          addr;
        const wchar_t *   name;
        Field (void * addr, const wchar_t name[])
        : addr(addr), name(name)
        { }
    };
    ARRAY(Field)    fields;
    NetVaultNode *  node;

    NetVaultNodeFieldArray (NetVaultNode * node);
    ~NetVaultNodeFieldArray ();

    void * GetFieldAddress (uint64_t bit);
    const wchar_t * GetFieldName (uint64_t bit);
    
    // client must lock node's local critical section before calling these.
    void GetFieldValueString_LCS (uint64_t bit, wchar_t * dst, unsigned dstChars);   
    void BuildWhereClause_LCS (EWhereCondition condition, wchar_t * dst, unsigned dstChars);
    ESqlType GetSqlType_LCS (uint64_t bit);
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

//============================================================================
// SrvPackBuffer
//============================================================================

// Allocate a CSrvPackBuffer on the heap with one extra uint32_t to allow for padding
#define SRV_ALLOC_BUFFER(bytes)                                     \
    new(malloc(sizeof(CSrvPackBuffer) + (bytes) + sizeof(uint32_t)))    \
    CSrvPackBuffer(bytes + sizeof(uint32_t))

// Allocate a CSrvPackBuffer on the stack with one extra uint32_t to allow for padding
#define SRV_ALLOCA_BUFFER(bytes)                                        \
    new(_alloca(sizeof(CSrvPackBuffer) + (bytes) + sizeof(uint32_t)))  \
    CSrvPackBuffer(bytes + sizeof(uint32_t))

class CSrvPackBuffer {
public:
    CSrvPackBuffer (unsigned bytes);

    void * Alloc (unsigned bytes);
    void AddData (const void * ptr, unsigned bytes);
    void AddString (const wchar_t str[]);
    void AddDWordArray (const uint32_t * arr, unsigned count);
    // add new "Add..." methods here as needed

    unsigned Size ();
    
private:
    uint8_t * m_pos;
    uint8_t * m_end;
    uint8_t * m_data;
};

class CSrvUnpackBuffer {
public:
    CSrvUnpackBuffer (const void * buffer, unsigned count);

    const void *  GetData (unsigned bytes);
    const wchar_t * GetString ();
    const uint32_t * GetDWordArray (unsigned count);
    
    unsigned BytesLeft ();
    bool ParseError ();

private:
    const uint8_t * m_pos;
    const uint8_t * m_end;
};
