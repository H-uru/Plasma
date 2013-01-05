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
    if (RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef()) {
        if (RelVaultNode * rvnFldr = rvnPlr->GetChildFolderNodeIncRef(folderType, 1)) {
            result = pyVaultFolderNode::New(rvnFldr);
            rvnFldr->DecRef();
        }
        rvnPlr->DecRef();
    }
    
    return result;
}

//============================================================================
static PyObject * GetPlayerInfoList (unsigned folderType) {
    PyObject * result = nil;
    if (RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef()) {
        if (RelVaultNode * rvnFldr = rvnPlr->GetChildPlayerInfoListNodeIncRef(folderType, 1)) {
            result = pyVaultPlayerInfoListNode::New(rvnFldr);
            rvnFldr->DecRef();
        }
        rvnPlr->DecRef();
    }
    
    return result;
}

//============================================================================
static PyObject * GetAgeInfoList (unsigned folderType) {
    PyObject * result = nil;
    if (RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef()) {
        if (RelVaultNode * rvnFldr = rvnPlr->GetChildAgeInfoListNodeIncRef(folderType, 1)) {
            result = pyVaultAgeInfoListNode::New(rvnFldr);
            rvnFldr->DecRef();
        }
        rvnPlr->DecRef();
    }
    
    return result;
}

//////////////////////////////////////////////////
PyObject* pyVault::GetPlayerInfo( void )
{
    PyObject * result = nil;
    if (RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef()) {
        if (RelVaultNode * rvnPlrInfo = rvnPlr->GetChildNodeIncRef(plVault::kNodeType_PlayerInfo, 1)) {
            result = pyVaultPlayerInfoNode::New(rvnPlrInfo);
            rvnPlrInfo->DecRef();
        }
        rvnPlr->DecRef();
    }
    
    // just return an empty node
    if (!result)
        result = pyVaultPlayerInfoNode::New(nil);
        
    return result;
}


PyObject* pyVault::GetAvatarOutfitFolder( void )
{
    PyObject * result = GetFolder(plVault::kAvatarOutfitFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetAvatarClosetFolder( void )
{
    PyObject * result = GetFolder(plVault::kAvatarClosetFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetChronicleFolder( void )
{
    PyObject * result = GetFolder(plVault::kChronicleFolder);

    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetInbox( void )
{
    PyObject * result = GetFolder(plVault::kInboxFolder);

    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetAgeJournalsFolder( void )
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
        RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef();
        if (!rvnPlr)
            break;

        for (;;) {
            RelVaultNode * rvnAgeJrnlz = rvnPlr->GetChildFolderNodeIncRef(plVault::kAgeJournalsFolder, 1);
            if (!rvnAgeJrnlz)
                break;

            // Get child nodes up to two levels deep
            ARRAY(RelVaultNode*) nodeArr;
            rvnAgeJrnlz->GetChildNodesIncRef(2, &nodeArr);
            
            RelVaultNode ** cur = nodeArr.Ptr();
            RelVaultNode ** end = nodeArr.Term();
            for (; cur != end; ++cur) {
                RelVaultNode * rvn = *cur;
                if (rvn->GetNodeType() == plVault::kNodeType_Image)
                    ++pictures;
                else if (rvn->GetNodeType() == plVault::kNodeType_TextNote)
                    ++notes;
                else if (rvn->GetNodeType() == plVault::kNodeType_MarkerGame)
                    ++markerGames;
                rvn->DecRef();
            }
            
            rvnAgeJrnlz->DecRef();
            break;
        }
        rvnPlr->DecRef();
        break;
    }       

    // create the tuple of usage numbers
    PyObject* retVal = PyTuple_New(4);
    PyTuple_SetItem(retVal, 0, PyLong_FromUnsignedLong(pictures));
    PyTuple_SetItem(retVal, 1, PyLong_FromUnsignedLong(notes));
    PyTuple_SetItem(retVal, 2, PyLong_FromUnsignedLong(markerGames));
    return retVal;
}


PyObject* pyVault::GetIgnoreListFolder( void )
{
    PyObject * result = GetPlayerInfoList(plVault::kIgnoreListFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetBuddyListFolder( void )
{
    PyObject * result = GetPlayerInfoList(plVault::kBuddyListFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetPeopleIKnowAboutFolder( void )
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

    if (RelVaultNode * rvn = VaultGetOwnedAgeLinkIncRef(&info)) {
        PyObject * result = pyVaultAgeLinkNode::New(rvn);
        rvn->DecRef();
        return result;
    }

    PYTHON_RETURN_NONE;
}

PyObject* pyVault::GetLinkToCity() const
{
    plAgeInfoStruct info;
    info.SetAgeFilename(kCityAgeFilename);

    if (RelVaultNode * rvn = VaultGetOwnedAgeLinkIncRef(&info)) {
        PyObject * result = pyVaultAgeLinkNode::New(rvn);
        rvn->DecRef();
        return result;
    }

    PYTHON_RETURN_NONE;
}


// Owned ages
PyObject* pyVault::GetOwnedAgeLink( const pyAgeInfoStruct & info )
{
    if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(info.GetAgeInfo())) {
        PyObject * result = pyVaultAgeLinkNode::New(rvnLink);
        rvnLink->DecRef();
        return result;
    }

    // just return a None object
    PYTHON_RETURN_NONE;
}

// Visit ages
PyObject* pyVault::GetVisitAgeLink( const pyAgeInfoStruct & info)
{
    if (RelVaultNode * rvnLink = VaultGetVisitAgeLinkIncRef(info.GetAgeInfo())) {
        PyObject * result = pyVaultAgeLinkNode::New(rvnLink);
        rvnLink->DecRef();
        return result;
    }

    // just return a None object
    PYTHON_RETURN_NONE;
}


///////////////
// Chronicle
PyObject* pyVault::FindChronicleEntry( const char * entryName )
{
    wchar_t wEntryName[kMaxVaultNodeStringLength];
    StrToUnicode(wEntryName, entryName, arrsize(wEntryName));
    
    if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(wEntryName)) {
        PyObject * result = pyVaultChronicleNode::New(rvn);
        rvn->DecRef();
        return result;
    }
    
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


PyObject* pyVault::GetAgesICanVisitFolder(void)
{
    PyObject * result = GetAgeInfoList(plVault::kAgesICanVisitFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyVault::GetAgesIOwnFolder(void)
{
    PyObject * result = GetAgeInfoList(plVault::kAgesIOwnFolder);
    
    // if good then return py object
    if (result)
        return result;

    // otherwise return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyVault::GetInviteFolder(void)
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
    NetVaultNode * templateNode = new NetVaultNode;
    templateNode->IncRef();

    if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {

        templateNode->ClearFieldFlags();
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {

            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_SDL);

            if (RelVaultNode * rvnSdl = rvnInfo->GetChildNodeIncRef(templateNode, 1)) {
                VaultSDLNode sdl(rvnSdl);
                plStateDataRecord * rec = new plStateDataRecord;
                if (sdl.GetStateDataRecord(rec, plSDL::kKeepDirty))
                    result = pySDLStateDataRecord::New(rec);
                else
                    delete rec;
                rvnSdl->DecRef();
            }
            rvnInfo->DecRef();
        }
        rvnFldr->DecRef();
    }

    templateNode->DecRef();

    if (!result)
        PYTHON_RETURN_NONE;
    
    return result;
}

void pyVault::UpdatePsnlAgeSDL( pySDLStateDataRecord & pyrec )
{
    plStateDataRecord * rec = pyrec.GetRec();
    if ( !rec )
        return;

    NetVaultNode * templateNode = new NetVaultNode;
    templateNode->IncRef();

    if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {

        templateNode->ClearFieldFlags();
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {

            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_SDL);

            if (RelVaultNode * rvnSdl = rvnInfo->GetChildNodeIncRef(templateNode, 1)) {
                VaultSDLNode sdl(rvnSdl);
                sdl.SetStateDataRecord(rec, plSDL::kDirtyOnly | plSDL::kTimeStampOnRead);
                rvnSdl->DecRef();
            }
            rvnInfo->DecRef();
        }
        rvnFldr->DecRef();
    }

    templateNode->DecRef();
}

bool pyVault::InMyPersonalAge( void ) const
{
    return VaultAmInMyPersonalAge();
}

bool pyVault::InMyNeighborhoodAge( void ) const
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
    NetVaultNode * templateNode = new NetVaultNode;
    templateNode->IncRef();
    templateNode->SetNodeType(plVault::kNodeType_TextNote);
    VaultTextNoteNode visitAcc(templateNode);
    visitAcc.SetNoteType(plVault::kNoteType_Visit);
    visitAcc.SetVisitInfo(*link.GetAgeLink()->GetAgeInfo());
    VaultCreateNode(templateNode, (FVaultCreateNodeCallback)_InvitePlayerToAge, nil, (void*)playerID);
    templateNode->DecRef();
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

    if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(&info)) {
        if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);
            ageInfo.CopyTo(&info);
            rvnInfo->DecRef();
        }

        rvnLink->DecRef();
    }

    NetVaultNode * templateNode = new NetVaultNode;
    templateNode->IncRef();
    templateNode->SetNodeType(plVault::kNodeType_TextNote);
    VaultTextNoteNode visitAcc(templateNode);
    visitAcc.SetNoteType(plVault::kNoteType_UnVisit);
    visitAcc.SetVisitInfo(info);
    VaultCreateNode(templateNode, (FVaultCreateNodeCallback)_UninvitePlayerToAge, nil, (void*)playerID);
    templateNode->DecRef();
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
        title = plString::Format( "%s'", nc->GetPlayerName().c_str() );
        desc = plString::Format( "%s' %s", nc->GetPlayerName().c_str(), link.GetAgeInfo()->GetAgeInstanceName() );
    }
    else
    {
        title = plString::Format( "%s's", nc->GetPlayerName().c_str() );
        desc = plString::Format( "%s's %s", nc->GetPlayerName().c_str(), link.GetAgeInfo()->GetAgeInstanceName() );
    }

    plUUID guid = plUUID::Generate();
    link.GetAgeInfo()->SetAgeInstanceGuid(&guid);
    link.GetAgeInfo()->SetAgeUserDefinedName( title.c_str() );
    link.GetAgeInfo()->SetAgeDescription( desc.c_str() );

    VaultRegisterOwnedAge(&link);
}

bool pyVault::SetAgePublic( const pyAgeInfoStruct * ageInfo, bool makePublic )
{
    // Note: This doesn't actually block (~Hoikas)
    return VaultSetOwnedAgePublicAndWait(ageInfo->GetAgeInfo(), makePublic);
}


PyObject* pyVault::GetGlobalInbox( void )
{
    PyObject * result = nil;
    if (RelVaultNode * rvnGlobalInbox = VaultGetGlobalInboxIncRef()) {
        result = pyVaultFolderNode::New(rvnGlobalInbox);
        rvnGlobalInbox->DecRef();
        return result;
    }

    PYTHON_RETURN_NONE;
}


/////////////////////////////////////////////////////////////

PyObject* pyVault::FindNode( pyVaultNode* templateNode ) const
{
    // See if we already have a matching node locally
    if (RelVaultNode * rvn = VaultGetNodeIncRef(templateNode->GetNode())) {
        PyObject * result = pyVaultNode::New(rvn);
        rvn->DecRef();
        return result;
    }
    
    // See if a matching node exists on the server
    ARRAY(unsigned) nodeIds;
    VaultFindNodesAndWait(templateNode->GetNode(), &nodeIds);
    
    if (nodeIds.Count()) {
        // Only fetch the first matching node since this function returns a single node
        VaultFetchNodesAndWait(&nodeIds[0], 1);
        // If we fetched it successfully then it'll be in our local node cache now
        if (RelVaultNode * rvn = VaultGetNodeIncRef(nodeIds[0])) {
            PyObject * result = pyVaultNode::New(rvn);
            rvn->DecRef();
            return result;
        }
    }
    
    PYTHON_RETURN_NONE;
}


