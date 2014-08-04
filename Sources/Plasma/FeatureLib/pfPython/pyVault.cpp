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
//////////////////////////////////////////////////////////////////////
//
// pyVault   - a wrapper class to provide interface to the plVault
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>
#pragma hdrstop

#ifdef BUILDING_PYPLASMA
# error "pyVault is not compatible with pyPlasma.pyd. Use BUILDING_PYPLASMA macro to ifdef out unwanted headers."
#endif

#include "pyVault.h"
#include "pyVaultNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeInfoListNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultFolderNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultTextNoteNode.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"
#include "pySDL.h"

#include "pnKeyedObject/plKey.h"

#include "plVault/plVault.h"
#include "pnNetCommon/plNetApp.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClient/plNetLinkingMgr.h"
#include "plNetClientComm/plNetClientComm.h"
#include "plMessage/plVaultNotifyMsg.h"

#include "plSDL/plSDL.h"

//============================================================================
static PyObject * GetFolder (unsigned folderType) {
    PyObject * result = nil;
    if (hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode()) {
        if (hsRef<RelVaultNode> rvnFldr = rvnPlr->GetChildFolderNode(folderType, 1))
            result = pyVaultFolderNode::New(rvnFldr);
    }
    
    return result;
}

//============================================================================
static PyObject * GetPlayerInfoList (unsigned folderType) {
    PyObject * result = nil;
    if (hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode()) {
        if (hsRef<RelVaultNode> rvnFldr = rvnPlr->GetChildPlayerInfoListNode(folderType, 1))
            result = pyVaultPlayerInfoListNode::New(rvnFldr);
    }
    
    return result;
}

//============================================================================
static PyObject * GetAgeInfoList (unsigned folderType) {
    PyObject * result = nil;
    if (hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode()) {
        if (hsRef<RelVaultNode> rvnFldr = rvnPlr->GetChildAgeInfoListNode(folderType, 1))
            result = pyVaultAgeInfoListNode::New(rvnFldr);
    }
    
    return result;
}

//////////////////////////////////////////////////
PyObject* pyVault::GetPlayerInfo()
{
    PyObject * result = nil;
    if (hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode()) {
        if (hsRef<RelVaultNode> rvnPlrInfo = rvnPlr->GetChildNode(plVault::kNodeType_PlayerInfo, 1))
            result = pyVaultPlayerInfoNode::New(rvnPlrInfo);
    }
    
    // just return an empty node
    if (!result)
        result = pyVaultPlayerInfoNode::New(nil);
        
    return result;
}


PyObject* pyVault::GetAvatarOutfitFolder()
{
    PyObject * result = GetFolder(plVault::kAvatarOutfitFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetAvatarClosetFolder()
{
    PyObject * result = GetFolder(plVault::kAvatarClosetFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetChronicleFolder()
{
    PyObject * result = GetFolder(plVault::kChronicleFolder);

    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetInbox()
{
    PyObject * result = GetFolder(plVault::kInboxFolder);

    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetAgeJournalsFolder()
{
    PyObject * result = GetFolder(plVault::kAgeJournalsFolder);
    
    // just return an empty node
    if (!result)
        result = pyVaultFolderNode::New(nil);
        
    return result;
}

// finds the stats for the players vault
// ...such as how many pictures, notes and markers they have
PyObject* pyVault::GetKIUsage(void)
{
    uint32_t pictures = 0;
    uint32_t notes = 0;
    uint32_t markerGames = 0;

    for (;;) {
        hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode();
        if (!rvnPlr)
            break;

        for (;;) {
            hsRef<RelVaultNode> rvnAgeJrnlz = rvnPlr->GetChildFolderNode(plVault::kAgeJournalsFolder, 1);
            if (!rvnAgeJrnlz)
                break;

            // Get child nodes up to two levels deep
            RelVaultNode::RefList nodeArr;
            rvnAgeJrnlz->GetChildNodes(2, &nodeArr);
            
            for (const hsRef<RelVaultNode> &rvn : nodeArr) {
                if (rvn->GetNodeType() == plVault::kNodeType_Image)
                    ++pictures;
                else if (rvn->GetNodeType() == plVault::kNodeType_TextNote)
                    ++notes;
                else if (rvn->GetNodeType() == plVault::kNodeType_MarkerGame)
                    ++markerGames;
            }
            
            break;
        }
        break;
    }       

    // create the tuple of usage numbers
    PyObject* retVal = PyTuple_New(4);
    PyTuple_SetItem(retVal, 0, PyLong_FromUnsignedLong(pictures));
    PyTuple_SetItem(retVal, 1, PyLong_FromUnsignedLong(notes));
    PyTuple_SetItem(retVal, 2, PyLong_FromUnsignedLong(markerGames));
    return retVal;
}

PyObject* pyVault::GetAllPlayersFolder()
{
#ifndef PLASMA_EXTERNAL_RELEASE
    PyObject* result = GetPlayerInfoList(plVault::kAllPlayersFolder);
    if (result)
        return result;
#endif

    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetIgnoreListFolder()
{
    PyObject * result = GetPlayerInfoList(plVault::kIgnoreListFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetBuddyListFolder()
{
    PyObject * result = GetPlayerInfoList(plVault::kBuddyListFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetPeopleIKnowAboutFolder()
{
    PyObject * result = GetPlayerInfoList(plVault::kPeopleIKnowAboutFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

/////////

PyObject* pyVault::GetLinkToMyNeighborhood() const
{
    plAgeInfoStruct info;
    info.SetAgeFilename(kNeighborhoodAgeFilename);

    if (hsRef<RelVaultNode> rvn = VaultGetOwnedAgeLink(&info))
        return pyVaultAgeLinkNode::New(rvn);

    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetLinkToCity() const
{
    plAgeInfoStruct info;
    info.SetAgeFilename(kCityAgeFilename);

    if (hsRef<RelVaultNode> rvn = VaultGetOwnedAgeLink(&info))
        return pyVaultAgeLinkNode::New(rvn);

    PYTHON_RETURN_NONE;
}


// Owned ages
PyObject* pyVault::GetOwnedAgeLink( const pyAgeInfoStruct & info )
{
    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(info.GetAgeInfo()))
        return pyVaultAgeLinkNode::New(rvnLink);

    // just return a None object
    PYTHON_RETURN_NONE;
}

// Visit ages
PyObject* pyVault::GetVisitAgeLink( const pyAgeInfoStruct & info)
{
    if (hsRef<RelVaultNode> rvnLink = VaultGetVisitAgeLink(info.GetAgeInfo()))
        return pyVaultAgeLinkNode::New(rvnLink);

    // just return a None object
    PYTHON_RETURN_NONE;
}


///////////////
// Chronicle
PyObject* pyVault::FindChronicleEntry( const char * entryName )
{
    wchar_t wEntryName[kMaxVaultNodeStringLength];
    StrToUnicode(wEntryName, entryName, arrsize(wEntryName));
    
    if (hsRef<RelVaultNode> rvn = VaultFindChronicleEntry(wEntryName))
        return pyVaultChronicleNode::New(rvn);
    
    // just return a None object
    PYTHON_RETURN_NONE;
}

void pyVault::AddChronicleEntry( const char * name, uint32_t type, const char * value )
{
    wchar_t * wEntryName = StrDupToUnicode(name);
    wchar_t * wEntryValue = StrDupToUnicode(value);
    
    // FIXME: We should ideally not block, but for now, the Python assumes that when 
    //        we return, the chronicle exists and can be found with findChronicleEntry. 
    //        Maybe we should insert a dummy into the tree? (currently hard)
    VaultAddChronicleEntryAndWait(wEntryName, type, wEntryValue);
    
    free(wEntryName);
    free(wEntryValue);
}


void pyVault::SendToDevice( pyVaultNode& node, const char * deviceName )
{
    if (!node.GetNode())
        return;

    wchar_t wDevName[256];
    StrToUnicode(wDevName, deviceName, arrsize(wDevName));

    // Note: This actually blocks (~Hoikas)
    VaultPublishNode(node.GetNode()->GetNodeId(), wDevName);
}


PyObject* pyVault::GetAgesICanVisitFolder()
{
    PyObject * result = GetAgeInfoList(plVault::kAgesICanVisitFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyVault::GetAgesIOwnFolder()
{
    PyObject * result = GetAgeInfoList(plVault::kAgesIOwnFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyVault::GetInviteFolder()
{
    PyObject * result = GetFolder(plVault::kPlayerInviteFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyVault::GetPsnlAgeSDL() const
{
    PyObject * result = nil;
    hsRef<NetVaultNode> templateNode = new NetVaultNode;

    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {

        templateNode->ClearFieldFlags();
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {

            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_SDL);

            if (hsRef<RelVaultNode> rvnSdl = rvnInfo->GetChildNode(templateNode, 1)) {
                VaultSDLNode sdl(rvnSdl);
                plStateDataRecord * rec = new plStateDataRecord;
                if (sdl.GetStateDataRecord(rec, plSDL::kKeepDirty))
                    result = pySDLStateDataRecord::New(rec);
                else
                    delete rec;
            }
        }
    }

    if (!result)
        PYTHON_RETURN_NONE;
    
    return result;
}

void pyVault::UpdatePsnlAgeSDL( pySDLStateDataRecord & pyrec )
{
    plStateDataRecord * rec = pyrec.GetRec();
    if ( !rec )
        return;

    hsRef<NetVaultNode> templateNode = new NetVaultNode;

    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {

        templateNode->ClearFieldFlags();
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {

            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_SDL);

            if (hsRef<RelVaultNode> rvnSdl = rvnInfo->GetChildNode(templateNode, 1)) {
                VaultSDLNode sdl(rvnSdl);
                sdl.SetStateDataRecord(rec, plSDL::kDirtyOnly | plSDL::kTimeStampOnRead);
            }
        }
    }
}

bool pyVault::InMyPersonalAge() const
{
    return VaultAmInMyPersonalAge();
}

bool pyVault::InMyNeighborhoodAge() const
{
    return VaultAmInMyNeighborhoodAge();
}

bool pyVault::AmOwnerOfCurrentAge() const
{
    return VaultAmOwnerOfCurrentAge();
}

bool pyVault::AmCzarOfCurrentAge() const
{
    return VaultAmCzarOfCurrentAge();
}

bool pyVault::AmAgeOwner( const pyAgeInfoStruct * ageInfo )
{
    if (!ageInfo->GetAgeInfo())
        return false;

    plUUID ageInstId = *ageInfo->GetAgeInfo()->GetAgeInstanceGuid();
    return VaultAmOwnerOfAge(ageInstId);
}

bool pyVault::AmAgeCzar( const pyAgeInfoStruct * ageInfo )
{
    if (!ageInfo->GetAgeInfo())
        return false;

    plUUID ageInstId = *ageInfo->GetAgeInfo()->GetAgeInstanceGuid();
    return VaultAmCzarOfAge(ageInstId);
}

void pyVault::RegisterMTStation( const char * stationName, const char * backLinkSpawnPtObjName )
{
    wchar_t wStationName[256];
    wchar_t wSpawnPt[256];
    StrToUnicode(wStationName, stationName, arrsize(wStationName));
    StrToUnicode(wSpawnPt, backLinkSpawnPtObjName, arrsize(wSpawnPt));

    // Note: This doesn't actually block (~Hoikas)
    VaultRegisterMTStationAndWait( wStationName, wSpawnPt);
}

void pyVault::RegisterOwnedAge( const pyAgeLinkStruct & link )
{
    VaultRegisterOwnedAgeAndWait(link.GetAgeLink());
}

void pyVault::UnRegisterOwnedAge( const char * ageFilename )
{
    plAgeInfoStruct info;
    info.SetAgeFilename(ageFilename);
    VaultUnregisterOwnedAgeAndWait(&info);
}

void pyVault::RegisterVisitAge( const pyAgeLinkStruct & link )
{
    VaultRegisterVisitAge(link.GetAgeLink());
}

void pyVault::UnRegisterVisitAge( const char * guidstr )
{
    plAgeInfoStruct info;
    plUUID guid(guidstr);
    info.SetAgeInstanceGuid(&guid);
    VaultUnregisterVisitAgeAndWait(&info);
}

//============================================================================
void _InvitePlayerToAge(ENetError result, void* state, void* param, RelVaultNode* node)
{
    if (result == kNetSuccess)
        VaultSendNode(node, (uint32_t)((uintptr_t)param));
}

void pyVault::InvitePlayerToAge( const pyAgeLinkStruct & link, uint32_t playerID )
{
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_TextNote);
    VaultTextNoteNode visitAcc(templateNode);
    visitAcc.SetNoteType(plVault::kNoteType_Visit);
    visitAcc.SetVisitInfo(*link.GetAgeLink()->GetAgeInfo());
    VaultCreateNode(templateNode, (FVaultCreateNodeCallback)_InvitePlayerToAge, nil, (void*)playerID);
}

//============================================================================
void _UninvitePlayerToAge(ENetError result, void* state, void* param, RelVaultNode* node)
{
    if (result == kNetSuccess)
        VaultSendNode(node, (uint32_t)((uintptr_t)param));
}

void pyVault::UnInvitePlayerToAge( const char * str, uint32_t playerID )
{
    plAgeInfoStruct info;
    plUUID guid(str);
    info.SetAgeInstanceGuid(&guid);

    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(&info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);
            ageInfo.CopyTo(&info);
        }
    }

    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_TextNote);
    VaultTextNoteNode visitAcc(templateNode);
    visitAcc.SetNoteType(plVault::kNoteType_UnVisit);
    visitAcc.SetVisitInfo(info);
    VaultCreateNode(templateNode, (FVaultCreateNodeCallback)_UninvitePlayerToAge, nil, (void*)playerID);
}

//============================================================================
void pyVault::OfferLinkToPlayer( const pyAgeLinkStruct & link, uint32_t playerID )
{
    hsAssert(false, "eric, port me");
}

//============================================================================
void pyVault::CreateNeighborhood()
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    // Unregister old hood
    // Note: This doesn't actually block (~Hoikas)
    plAgeInfoStruct info;
    info.SetAgeFilename(kNeighborhoodAgeFilename);
    VaultUnregisterOwnedAgeAndWait(&info);

    // Register new hood    
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(kNeighborhoodAgeFilename);
    link.GetAgeInfo()->SetAgeInstanceName(kNeighborhoodAgeInstanceName);

    plString title;
    plString desc;

    unsigned nameLen = nc->GetPlayerName().GetSize();
    if (nc->GetPlayerName().CharAt(nameLen - 1) == 's' || nc->GetPlayerName().CharAt(nameLen - 1) == 'S')
    {
        title = plFormat("{}'", nc->GetPlayerName());
        desc = plFormat("{}' {}", nc->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName());
    }
    else
    {
        title = plFormat("{}'s", nc->GetPlayerName());
        desc = plFormat("{}'s {}", nc->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName());
    }

    plUUID guid = plUUID::Generate();
    link.GetAgeInfo()->SetAgeInstanceGuid(&guid);
    link.GetAgeInfo()->SetAgeUserDefinedName(title);
    link.GetAgeInfo()->SetAgeDescription(desc);

    VaultRegisterOwnedAge(&link);
}

bool pyVault::SetAgePublic( const pyAgeInfoStruct * ageInfo, bool makePublic )
{
    // Note: This doesn't actually block (~Hoikas)
    return VaultSetOwnedAgePublicAndWait(ageInfo->GetAgeInfo(), makePublic);
}


PyObject* pyVault::GetGlobalInbox()
{
    if (hsRef<RelVaultNode> rvnGlobalInbox = VaultGetGlobalInbox())
        return pyVaultFolderNode::New(rvnGlobalInbox);

    PYTHON_RETURN_NONE;
}


/////////////////////////////////////////////////////////////

PyObject* pyVault::FindNode( pyVaultNode* templateNode ) const
{
    // See if we already have a matching node locally
    if (hsRef<RelVaultNode> rvn = VaultGetNode(templateNode->GetNode()))
        return pyVaultNode::New(rvn);
    
    // See if a matching node exists on the server
    ARRAY(unsigned) nodeIds;
    VaultFindNodesAndWait(templateNode->GetNode(), &nodeIds);
    
    if (nodeIds.Count()) {
        // Only fetch the first matching node since this function returns a single node
        VaultFetchNodesAndWait(&nodeIds[0], 1);
        // If we fetched it successfully then it'll be in our local node cache now
        if (hsRef<RelVaultNode> rvn = VaultGetNode(nodeIds[0]))
            return pyVaultNode::New(rvn);
    }
    
    PYTHON_RETURN_NONE;
}
