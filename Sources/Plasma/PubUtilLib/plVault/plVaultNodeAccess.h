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

#include "hsGeometry3.h"
#include "hsRefCnt.h"

#include "pnNetProtocol/pnNetProtocol.h"

#include <string_theory/string>

/*****************************************************************************
*
*   VaultNode field access strutures
*
***/

class plAgeInfoStruct;
class plAgeLinkStruct;
struct plSpawnPointInfo;

typedef std::vector<plSpawnPointInfo>   plSpawnPointVec;

//============================================================================
// NetVaultNodeAccess
//============================================================================
struct NetVaultNodeAccess {
    hsWeakRef<NetVaultNode> base;

    NetVaultNodeAccess(hsWeakRef<NetVaultNode> node) : base(node) { }

private:
    NetVaultNodeAccess (const NetVaultNodeAccess &) { }
    void operator= (const NetVaultNodeAccess &) { }
};

#define VNODE_ACCESSOR(type, name, basename) \
    type Get##name () const { return base->Get##basename(); } \
    void Set##name (type v) { base->Set##basename(v); }

#define VNODE_BLOB(name, basename) \
    const std::vector<uint8_t>& Get##name() const { return base->Get##basename(); } \
    void Set##name (const uint8_t data[], size_t length) { base->Set##basename(data, length); }

#define VNODE_STRING(name, basename) \
    ST::string Get##name () const { return base->Get##basename(); } \
    void Set##name (const ST::string& v) { base->Set##basename(v); }

//============================================================================
// VaultPlayerNode
//============================================================================
struct VaultPlayerNode : NetVaultNodeAccess {
    VNODE_STRING  (                 PlayerName,         IString64_1);
    VNODE_STRING  (                 AvatarShapeName,    String64_1);
    VNODE_ACCESSOR(int32_t,         Disabled,           Int32_1);
    VNODE_ACCESSOR(int32_t,         Explorer,           Int32_2);   // explorer = 1, visitor = 0
    VNODE_ACCESSOR(uint32_t,        OnlineTime,         UInt32_1);
    VNODE_ACCESSOR(plUUID,          AccountUuid,        Uuid_1);
    VNODE_ACCESSOR(plUUID,          InviteUuid,         Uuid_2);

    VaultPlayerNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};


//============================================================================
// VaultPlayerInfoNode
//============================================================================
struct VaultPlayerInfoNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(uint32_t,        PlayerId,           UInt32_1);
    VNODE_STRING  (                 PlayerName,         IString64_1);
    VNODE_STRING  (                 AgeInstName,        String64_1);    // name of age player is currently in
    VNODE_ACCESSOR(plUUID,          AgeInstUuid,        Uuid_1);        // guid of age player is currently in
    VNODE_ACCESSOR(int32_t,         Online,             Int32_1);       // whether or not player is online
    VNODE_ACCESSOR(int32_t,         CCRLevel,           Int32_2);

    VaultPlayerInfoNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};


//============================================================================
// VaultFolderNode
//============================================================================
struct VaultFolderNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(int32_t,         FolderType,         Int32_1);
    VNODE_STRING  (                 FolderName,         String64_1);

    VaultFolderNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};


//============================================================================
// VaultPlayerInfoListNode
//============================================================================
struct VaultPlayerInfoListNode : VaultFolderNode {
    VaultPlayerInfoListNode(hsWeakRef<NetVaultNode> node) : VaultFolderNode(node) { }
};

//============================================================================
// VaultAgeInfoListNode
//============================================================================
struct VaultAgeInfoListNode : VaultFolderNode {
    VaultAgeInfoListNode(hsWeakRef<NetVaultNode> node) : VaultFolderNode(node) { }
};

//============================================================================
// VaultChronicleNode
//============================================================================
struct VaultChronicleNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(int32_t,         EntryType,          Int32_1);
    VNODE_STRING  (                 EntryName,          String64_1);
    VNODE_STRING  (                 EntryValue,         Text_1);

    VaultChronicleNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};


//============================================================================
// VaultSDLNode
//============================================================================
struct VaultSDLNode : NetVaultNodeAccess {
    VNODE_STRING  (                 SDLName,            String64_1);
    VNODE_ACCESSOR(int32_t,         SDLIdent,           Int32_1);
    VNODE_BLOB    (                 SDLData,            Blob_1);

    VaultSDLNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    bool GetStateDataRecord (class plStateDataRecord * out, unsigned readOptions = 0);
    void SetStateDataRecord (const class plStateDataRecord * rec, unsigned writeOptions = 0);
    void InitStateDataRecord (const ST::string& sdlRecName, unsigned writeOptions = 0);
};

//============================================================================
// VaultAgeLinkNode
//============================================================================
struct VaultAgeLinkNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(int32_t,         Unlocked,           Int32_1);
    VNODE_ACCESSOR(int32_t,         Volatile,           Int32_2);
    VNODE_BLOB    (                 SpawnPoints,        Blob_1);

    VaultAgeLinkNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    bool CopyTo (plAgeLinkStruct * link);
    void AddSpawnPoint (const plSpawnPointInfo & point); // will only add if not there already.
    void RemoveSpawnPoint (const ST::string & spawnPtName);
    bool HasSpawnPoint (const ST::string & spawnPtName) const;
    bool HasSpawnPoint (const plSpawnPointInfo & point) const;  // compares spawn name only, not title.
    void GetSpawnPoints (plSpawnPointVec * out) const;
    void SetSpawnPoints (const plSpawnPointVec & in);
};

//============================================================================
// VaultImageNode
//============================================================================
struct VaultImageNode : NetVaultNodeAccess {
    enum ImageTypes { kNone=0, kJPEG=1, kPNG=2 };

    VNODE_ACCESSOR(int32_t,         ImageType,          Int32_1);
    VNODE_STRING  (                 ImageTitle,         String64_1);
    VNODE_BLOB    (                 ImageData,          Blob_1);

    VaultImageNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    void StuffImage (class plMipmap * src, int dstType=kJPEG);
    bool ExtractImage (class plMipmap ** dst);
};

//============================================================================
// VaultCliImageNode
//============================================================================
struct VaultCliImageNode : VaultImageNode {
    class plMipmap * fMipmap;

    VaultCliImageNode(hsWeakRef<NetVaultNode> node) : VaultImageNode(node) { }
};

//============================================================================
// VaultTextNoteNode
//============================================================================
struct VaultTextNoteNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(int32_t,         NoteType,           Int32_1);
    VNODE_ACCESSOR(int32_t,         NoteSubType,        Int32_2);
    VNODE_STRING  (                 NoteTitle,          String64_1);
    VNODE_STRING  (                 NoteText,           Text_1);

    VaultTextNoteNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    // for kNoteType_Visit/UnVisit
    void SetVisitInfo (const plAgeInfoStruct & info);
    bool GetVisitInfo (plAgeInfoStruct * info);
};

//============================================================================
// VaultAgeNode
//============================================================================
struct VaultAgeNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(plUUID,          AgeInstanceGuid,        Uuid_1);
    VNODE_ACCESSOR(plUUID,          ParentAgeInstanceGuid,  Uuid_2);
    VNODE_STRING  (                 AgeName,                String64_1);

    VaultAgeNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};

//============================================================================
// VaultAgeInfoNode
//============================================================================
struct VaultAgeInfoNode : NetVaultNodeAccess {
    VNODE_STRING  (                 AgeFilename,            String64_2);    // "Garden"
    VNODE_STRING  (                 AgeInstanceName,        String64_3);    // "Eder Kemo"
    VNODE_STRING  (                 AgeUserDefinedName,     String64_4);    // "Joe's"
    VNODE_ACCESSOR(plUUID,          AgeInstanceGuid,        Uuid_1);        // 6278b081-342a-4229-ac1b-a0b8a2658390
    VNODE_ACCESSOR(plUUID,          ParentAgeInstanceGuid,  Uuid_2);        // 9192be7f-89ef-41bc-83db-79afe451e399
    VNODE_STRING  (                 AgeDescription,         Text_1);        // "Stay out!"
    VNODE_ACCESSOR(int32_t,         AgeSequenceNumber,      Int32_1);
    VNODE_ACCESSOR(int32_t,         AgeLanguage,            Int32_3);       // The language of the client that made this age
    VNODE_ACCESSOR(uint32_t,        AgeId,                  UInt32_1);
    VNODE_ACCESSOR(uint32_t,        AgeCzarId,              UInt32_2);
    VNODE_ACCESSOR(uint32_t,        AgeInfoFlags,           UInt32_3);

    // WARNING: Do not set this. The age will not be set public this way. Use NetCliAuthSetAgePublic instead (changes this field's value in the process).
    VNODE_ACCESSOR(int32_t,         IsPublic,               Int32_2);

    VaultAgeInfoNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    const class plUnifiedTime * GetAgeTime () const;
    void CopyFrom (const plAgeInfoStruct * info);
    void CopyTo (plAgeInfoStruct * info) const;
};

//============================================================================
// VaultSystemNode
//============================================================================
struct VaultSystemNode : NetVaultNodeAccess {
    VNODE_ACCESSOR(int32_t,         CCRStatus,          Int32_1);

    VaultSystemNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }
};


//============================================================================
// VaultMarkerGameNode
//============================================================================
struct VaultMarker {
    uint32_t id;
    ST::string age;
    hsPoint3 pos;
    ST::string description;

    VaultMarker()
        : id()
    { }
    VaultMarker(uint32_t id, ST::string age, const hsPoint3& pos, ST::string description)
        : id(id), age(std::move(age)), pos(pos), description(std::move(description))
    { }
};

struct VaultMarkerGameNode : NetVaultNodeAccess {
    VNODE_STRING  (                 GameName,           Text_1);
    VNODE_STRING  (                 Reward,             Text_2);
    VNODE_ACCESSOR(plUUID,          GameGuid,           Uuid_1);

    VaultMarkerGameNode(hsWeakRef<NetVaultNode> node) : NetVaultNodeAccess(node) { }

    void GetMarkerData(std::vector<VaultMarker>& data) const;
    void SetMarkerData(const std::vector<VaultMarker>& data);
};
