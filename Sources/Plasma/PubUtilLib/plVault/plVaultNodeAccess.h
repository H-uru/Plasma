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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultNodeAccess.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTNODEACCESS_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultNodeAccess.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTNODEACCESS_H

// 'Old' system is full of compiler warnings at /W4, so just hide them
#pragma warning(push, 0)
#include "hsStlUtils.h"
#pragma warning(pop)


/*****************************************************************************
*
*   VaultNode field access strutures
*
***/

class plAgeInfoStruct;
class plAgeLinkStruct;
struct plSpawnPointInfo;

#ifdef CLIENT
typedef std::vector<plSpawnPointInfo>   plSpawnPointVec;
#endif

//============================================================================
// Volatile Vault Node Fields
//============================================================================
uint64_t GetNodeVolatileFields(NetVaultNode* node);

//============================================================================
// NetVaultNodeAccess
//============================================================================
struct NetVaultNodeAccess {
    NetVaultNode *  base;
    uint64_t &         fieldFlags;
    uint64_t &         dirtyFlags;

    NetVaultNodeAccess (NetVaultNode * node);
    NetVaultNodeAccess (const NetVaultNodeAccess &);                    // not implemented
    const NetVaultNodeAccess & operator= (const NetVaultNodeAccess &);  // not implemented
};


//============================================================================
// VaultPlayerNode
//============================================================================
struct VaultPlayerNode : NetVaultNodeAccess {
    static const uint64_t kPlayerName      = NetVaultNode::kIString64_1;
    static const uint64_t kAvatarShapeName = NetVaultNode::kString64_1;
    static const uint64_t kDisabled        = NetVaultNode::kInt32_1;
    static const uint64_t kExplorer        = NetVaultNode::kInt32_2;       // explorer = 1, visitor = 0
    static const uint64_t kOnlineTime      = NetVaultNode::kUInt32_1;
    static const uint64_t kAccountUuid     = NetVaultNode::kUuid_1;
    static const uint64_t kInviteUuid      = NetVaultNode::kUuid_2;

    // Treat these as read-only or node flag fields will become invalid 
    // Threaded apps: Must be accessed with node->critsect locked   
    wchar_t *&  playerName;
    wchar_t *&  avatarShapeName;
    int &       disabled;
    unsigned &  onlineTime;
    Uuid &      accountUuid;
    Uuid &      inviteUuid;
    int &       explorer;
    
    VaultPlayerNode (NetVaultNode * node);
    VaultPlayerNode (const VaultPlayerNode &);                      // not implemented
    const VaultPlayerNode & operator= (const VaultPlayerNode &);    // not implemented
    
    // Threaded apps: Must be called with node->critsect locked 
    void SetPlayerName (const wchar_t v[]);
    void SetAvatarShapeName (const wchar_t v[]);
    void SetDisabled (int v);
    void SetOnlineTime (unsigned v);
    void SetAccountUuid (const Uuid & v);
    void SetInviteUuid (const Uuid & v);
    void SetExplorer (int v);
};


//============================================================================
// VaultPlayerInfoNode
//============================================================================
struct VaultPlayerInfoNode : NetVaultNodeAccess {
    static const uint64_t kPlayerId        = NetVaultNode::kUInt32_1;
    static const uint64_t kPlayerName      = NetVaultNode::kIString64_1;
    static const uint64_t kAgeInstName     = NetVaultNode::kString64_1;    // name of age player is currently in
    static const uint64_t kAgeInstUuid     = NetVaultNode::kUuid_1;        // guid of age player is currently in
    static const uint64_t kOnline          = NetVaultNode::kInt32_1;       // whether or not player is online
    static const uint64_t kCCRLevel        = NetVaultNode::kInt32_2;
    
    // Treat these as read-only or node flag fields will become invalid 
    // Threaded apps: Must be accessed with node->critsect locked   
    unsigned &  playerId;
    wchar_t *&  playerName;
    wchar_t *&  ageInstName;
    Uuid &      ageInstUuid;
    int &       online;
    int &       ccrLevel;

    VaultPlayerInfoNode (NetVaultNode * node);
    VaultPlayerInfoNode (const VaultPlayerInfoNode &);                      // not implemented
    const VaultPlayerInfoNode & operator= (const VaultPlayerInfoNode &);    // not implemented

    // Threaded apps: Must be called with node->critsect locked 
    void SetPlayerId (unsigned v);
    void SetPlayerName (const wchar_t v[]);
    void SetAgeInstName (const wchar_t v[]);
    void SetAgeInstUuid (const Uuid & v);
    void SetOnline (int v);
    void SetCCRLevel (int v);
};


//============================================================================
// VaultFolderNode
//============================================================================
struct VaultFolderNode : NetVaultNodeAccess {
    static const uint64_t kFolderType      = NetVaultNode::kInt32_1;
    static const uint64_t kFolderName      = NetVaultNode::kString64_1;
    
    // Treat these as read-only or node flag fields will become invalid 
    // Threaded apps: Must be accessed with node->critsect locked   
    int &       folderType;
    wchar_t *&  folderName;
    
    VaultFolderNode (NetVaultNode * node);
    VaultFolderNode (const VaultFolderNode &);                      // not implemented
    const VaultFolderNode & operator= (const VaultFolderNode &);    // not implemented

    // Threaded apps: Must be called with node->critsect locked 
    void SetFolderName (const wchar_t v[]);
    void SetFolderType (int v);
};


//============================================================================
// VaultPlayerInfoListNode
//============================================================================
struct VaultPlayerInfoListNode : VaultFolderNode {

    VaultPlayerInfoListNode (NetVaultNode * node);
    VaultPlayerInfoListNode (const VaultPlayerInfoListNode &);                      // not implemented
    const VaultPlayerInfoListNode & operator= (const VaultPlayerInfoListNode &);    // not implemented
};

//============================================================================
// VaultAgeInfoListNode
//============================================================================
struct VaultAgeInfoListNode : VaultFolderNode {

    VaultAgeInfoListNode (NetVaultNode * node);
    VaultAgeInfoListNode (const VaultAgeInfoListNode &);                    // not implemented
    const VaultAgeInfoListNode & operator= (const VaultAgeInfoListNode &);  // not implemented
};

//============================================================================
// VaultChronicleNode
//============================================================================
struct VaultChronicleNode : NetVaultNodeAccess {
    static const uint64_t kEntryType   = NetVaultNode::kInt32_1;
    static const uint64_t kEntryName   = NetVaultNode::kString64_1;
    static const uint64_t kEntryValue  = NetVaultNode::kText_1;
    
    // Treat these as read-only or node flag fields will become invalid 
    // Threaded apps: Must be accessed with node->critsect locked   
    int &       entryType;
    wchar_t *&  entryName;
    wchar_t *&  entryValue;

    VaultChronicleNode (NetVaultNode * node);
    VaultChronicleNode (const VaultChronicleNode &);                    // not implemented
    const VaultChronicleNode & operator= (const VaultChronicleNode &);  // not implemented

    // Threaded apps: Must be called with node->critsect locked 
    void SetEntryType (int v);
    void SetEntryName (const wchar_t v[]);
    void SetEntryValue (const wchar_t v[]);
};


//============================================================================
// VaultSDLNode
//============================================================================
struct VaultSDLNode : NetVaultNodeAccess {
    static const uint64_t kSDLName     = NetVaultNode::kString64_1;
    static const uint64_t kSDLIdent    = NetVaultNode::kInt32_1;
    static const uint64_t kSDLData     = NetVaultNode::kBlob_1;
    
    int &       sdlIdent;
    wchar_t *&  sdlName;
    uint8_t *&  sdlData;
    unsigned &  sdlDataLen;
    
    VaultSDLNode (NetVaultNode * node);
    VaultSDLNode (const VaultSDLNode &);                    // not implemented
    const VaultSDLNode & operator= (const VaultSDLNode &);  // not implemented
    
    void SetSdlIdent (int v);
    void SetSdlName (const wchar_t v[]);

#ifdef CLIENT
    bool GetStateDataRecord (class plStateDataRecord * out, unsigned readOptions = 0);
    void SetStateDataRecord (const class plStateDataRecord * rec, unsigned writeOptions = 0);
    void InitStateDataRecord (const wchar_t sdlRecName[], unsigned writeOptions = 0);
#endif // def CLIENT
};

//============================================================================
// VaultAgeLinkNode
//============================================================================
struct VaultAgeLinkNode : NetVaultNodeAccess {
    static const uint64_t kUnlocked    = NetVaultNode::kInt32_1;
    static const uint64_t kVolatile    = NetVaultNode::kInt32_2;
    static const uint64_t kSpawnPoints = NetVaultNode::kBlob_1;

    int &       unlocked;
    int &       volat;
    uint8_t *&  spawnPoints;
    unsigned &  spawnPointsLen;
    
    VaultAgeLinkNode (NetVaultNode * node);
    VaultAgeLinkNode (const VaultAgeLinkNode &);                    // not implemented
    const VaultAgeLinkNode & operator= (const VaultAgeLinkNode &);  // not implemented
    
    void SetUnlocked (int v);
    void SetVolatile (int v);

#ifdef CLIENT
    bool CopyTo (plAgeLinkStruct * link);
    void AddSpawnPoint (const plSpawnPointInfo & point); // will only add if not there already.
    void RemoveSpawnPoint (const char spawnPtName[]);
    bool HasSpawnPoint (const char spawnPtName[]) const;
    bool HasSpawnPoint (const plSpawnPointInfo & point) const;  // compares spawn name only, not title.
    void GetSpawnPoints (plSpawnPointVec * out) const;
    void SetSpawnPoints (const plSpawnPointVec & in);
#endif
};

//============================================================================
// VaultImageNode
//============================================================================
struct VaultImageNode : NetVaultNodeAccess {

    enum ImageTypes { kNone=0, kJPEG=1 };

    static const uint64_t kImageType       = NetVaultNode::kInt32_1;
    static const uint64_t kImageTitle      = NetVaultNode::kString64_1;
    static const uint64_t kImageData       = NetVaultNode::kBlob_1;

    wchar_t *&  title;
    int &       imgType;
    uint8_t *&  imgData;
    unsigned &  imgDataLen;

    VaultImageNode (NetVaultNode * node);
    VaultImageNode (const VaultImageNode &);                    // not implemented
    const VaultImageNode & operator= (const VaultImageNode &);  // not implemented
    
    void SetImageTitle (const wchar_t v[]);
    void SetImageType (int v);
    void SetImageData (const uint8_t buffer[], unsigned bytes);
    
#ifdef CLIENT
    void StuffImage (class plMipmap * src);
    bool ExtractImage (class plMipmap ** dst);
#endif
};

//============================================================================
// VaultCliImageNode
//============================================================================
#ifdef CLIENT
struct VaultCliImageNode : VaultImageNode {
    class plMipmap * fMipmap;

    VaultCliImageNode (NetVaultNode * node);
    VaultCliImageNode (const VaultCliImageNode &);                      // not implemented
    const VaultCliImageNode & operator= (const VaultCliImageNode &);    // not implemented
};
#endif // def CLIENT

//============================================================================
// VaultTextNoteNode
//============================================================================
struct VaultTextNoteNode : NetVaultNodeAccess {

    static const uint64_t kNoteType    = NetVaultNode::kInt32_1;
    static const uint64_t kNoteSubType = NetVaultNode::kInt32_2;
    static const uint64_t kNoteTitle   = NetVaultNode::kString64_1;
    static const uint64_t kNoteText    = NetVaultNode::kText_1;
    
    int &       noteType;
    int &       noteSubType;
    wchar_t *&  noteTitle;
    wchar_t *&  noteText;

    VaultTextNoteNode (NetVaultNode * node);
    VaultTextNoteNode (const VaultTextNoteNode &);                      // not implemented
    const VaultTextNoteNode & operator= (const VaultTextNoteNode &);    // not implemented

    void SetNoteType (int v);
    void SetNoteSubType (int v);
    void SetNoteTitle (const wchar_t v[]);
    void SetNoteText (const wchar_t v[]);

#ifdef CLIENT   
    // for kNoteType_Visit/UnVisit
    void SetVisitInfo (const plAgeInfoStruct & info);
    bool GetVisitInfo (plAgeInfoStruct * info);
#endif
};

//============================================================================
// VaultAgeNode
//============================================================================
struct VaultAgeNode : NetVaultNodeAccess {

    static const uint64_t kAgeInstanceGuid         = NetVaultNode::kUuid_1;
    static const uint64_t kParentAgeInstanceGuid   = NetVaultNode::kUuid_2;
    static const uint64_t kAgeName                 = NetVaultNode::kString64_1;
    
    Uuid &          ageInstUuid;
    Uuid &          parentAgeInstUuid;
    wchar_t *&      ageName;
    
    VaultAgeNode (NetVaultNode * node);
    VaultAgeNode (const VaultAgeNode &);                            // not implemented
    const VaultAgeNode & operator= (const VaultAgeNode &);          // not implemented

    void SetAgeInstGuid (const Uuid & v);
    void SetParentAgeInstGuid (const Uuid & v);
    void SetAgeName (const wchar_t v[]);
};

//============================================================================
// VaultAgeInfoNode
//============================================================================
struct VaultAgeInfoNode : NetVaultNodeAccess {

    static const uint64_t kAgeFilename             = NetVaultNode::kString64_2;    // "Garden"
    static const uint64_t kAgeInstanceName         = NetVaultNode::kString64_3;    // "Eder Kemo"
    static const uint64_t kAgeUserDefinedName      = NetVaultNode::kString64_4;    // "Joe's"
    static const uint64_t kAgeInstanceGuid         = NetVaultNode::kUuid_1;        // 6278b081-342a-4229-ac1b-a0b8a2658390
    static const uint64_t kParentAgeInstanceGuid   = NetVaultNode::kUuid_2;        // 9192be7f-89ef-41bc-83db-79afe451e399
    static const uint64_t kAgeDescription          = NetVaultNode::kText_1;        // "Stay out!"
    static const uint64_t kAgeSequenceNumber       = NetVaultNode::kInt32_1;
    static const uint64_t kIsPublic                = NetVaultNode::kInt32_2;
    static const uint64_t kAgeLanguage             = NetVaultNode::kInt32_3;       // The language of the client that made this age
    static const uint64_t kAgeId                   = NetVaultNode::kUInt32_1;
    static const uint64_t kAgeCzarId               = NetVaultNode::kUInt32_2;
    static const uint64_t kAgeInfoFlags            = NetVaultNode::kUInt32_3;
    
    wchar_t *&      ageFilename;
    wchar_t *&      ageInstName;
    wchar_t *&      ageUserDefinedName;
    Uuid &          ageInstUuid;
    Uuid &          parentAgeInstUuid;
    int &           ageSequenceNumber;
    int &           ageIsPublic;
    int &           ageLanguage;
    unsigned &      ageId;
    unsigned &      ageCzarId;
    unsigned &      ageInfoFlags;
    wchar_t *&        ageDescription;

    VaultAgeInfoNode (NetVaultNode * node);
    VaultAgeInfoNode (const VaultAgeInfoNode &);                    // not implemented
    const VaultAgeInfoNode & operator= (const VaultAgeInfoNode &);  // not implemented

    void SetAgeFilename (const wchar_t v[]);
    void SetAgeInstName (const wchar_t v[]);
    void SetAgeUserDefinedName (const wchar_t v[]);
    void SetAgeInstGuid (const Uuid & v);
    void SetParentAgeInstGuid (const Uuid & v);
    void SetAgeSequenceNumber (int v);
    void _SetAgeIsPublic (int v);   // WARNING: Do not call this. The age will not be set public this way. Use NetCliAuthSetAgePublic instead (changes this field's value in the process).
    void SetAgeLanguage (int v);
    void SetAgeId (unsigned v);
    void SetAgeCzarId (unsigned v);
    void SetAgeInfoFlags (unsigned v);
    void SetAgeDescription (const wchar_t v[]);
    
#ifdef CLIENT
    const class plUnifiedTime * GetAgeTime () const;
    void CopyFrom (const plAgeInfoStruct * info);
    void CopyTo (plAgeInfoStruct * info) const;
#endif // def CLIENT
};

//============================================================================
// VaultSystemNode
//============================================================================
struct VaultSystemNode : NetVaultNodeAccess {

    static const uint64_t kCCRStatus   = NetVaultNode::kInt32_1;
    
    int &       ccrStatus;

    VaultSystemNode (NetVaultNode * node);
    VaultSystemNode (const VaultTextNoteNode &);                    // not implemented
    const VaultSystemNode & operator= (const VaultSystemNode &);    // not implemented

    void SetCCRStatus (int v);
};


//============================================================================
// VaultMarkerGameNode
//============================================================================
struct VaultMarkerGameNode : NetVaultNodeAccess {

    static const uint64_t kGameName = NetVaultNode::kText_1;
    static const uint64_t kGameGuid = NetVaultNode::kUuid_1;

    wchar_t *& gameName;
    Uuid &     gameGuid;
    
    VaultMarkerGameNode (NetVaultNode * node);
    VaultMarkerGameNode (const VaultMarkerGameNode &);                      // not implemented
    const VaultMarkerGameNode & operator= (const VaultMarkerGameNode &);    // not implemented
    
    void SetGameName (const wchar_t v[]);
    void SetGameGuid (const Uuid & v);
};
