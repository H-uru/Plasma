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
//////////////////////////////////////////////////////////////////////
//
// pyVault   - a wrapper class to provide interface to the plVault
//
//////////////////////////////////////////////////////////////////////

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

#include "../pnKeyedObject/plKey.h"
#include "cyPythonInterface.h"

#include "../plVault/plVault.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plNetClientComm/plNetClientComm.h"
#include "../plMessage/plVaultNotifyMsg.h"

#include "../plSDL/plSDL.h"


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
	UInt32 pictures = 0;
	UInt32 notes = 0;
	UInt32 markerGames = 0;

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
				if (rvn->nodeType == plVault::kNodeType_Image)
					++pictures;
				else if (rvn->nodeType == plVault::kNodeType_TextNote)
					++notes;
				else if (rvn->nodeType == plVault::kNodeType_MarkerGame)
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
	wchar wEntryName[kMaxVaultNodeStringLength];
	StrToUnicode(wEntryName, entryName, arrsize(wEntryName));
	
	if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(wEntryName)) {
		PyObject * result = pyVaultChronicleNode::New(rvn);
		rvn->DecRef();
		return result;
	}
	
	// just return a None object
	PYTHON_RETURN_NONE;
}

void pyVault::AddChronicleEntry( const char * name, UInt32 type, const char * value )
{
	wchar * wEntryName = StrDupToUnicode(name);
	wchar * wEntryValue = StrDupToUnicode(value);
	
	VaultAddChronicleEntryAndWait(wEntryName, type, wEntryValue);
	
	FREE(wEntryName);
	FREE(wEntryValue);
}


void pyVault::SendToDevice( pyVaultNode& node, const char * deviceName )
{
	if (!node.GetNode())
		return;

	wchar wDevName[256];
	StrToUnicode(wDevName, deviceName, arrsize(wDevName));		
	VaultPublishNode(node.GetNode()->nodeId, wDevName);
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
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	
	if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {
		
		templateNode->fieldFlags = 0;
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
		VaultAgeInfoNode ageInfo(templateNode);
		wchar str[MAX_PATH];
		StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
		ageInfo.SetAgeFilename(str);

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_SDL);

			if (RelVaultNode * rvnSdl = rvnInfo->GetChildNodeIncRef(templateNode, 1)) {
				VaultSDLNode sdl(rvnSdl);
				plStateDataRecord * rec = NEWZERO(plStateDataRecord);
				if (sdl.GetStateDataRecord(rec, plSDL::kKeepDirty))
					result = pySDLStateDataRecord::New(rec);
				else
					DEL(rec);
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

	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	
	if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {
		
		templateNode->fieldFlags = 0;
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
		VaultAgeInfoNode ageInfo(templateNode);
		wchar str[MAX_PATH];
		StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
		ageInfo.SetAgeFilename(str);

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			
			templateNode->fieldFlags = 0;
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

	Uuid ageInstId = *ageInfo->GetAgeInfo()->GetAgeInstanceGuid();
	return VaultAmOwnerOfAge(ageInstId);
}

bool pyVault::AmAgeCzar( const pyAgeInfoStruct * ageInfo )
{
	if (!ageInfo->GetAgeInfo())
		return false;

	Uuid ageInstId = *ageInfo->GetAgeInfo()->GetAgeInstanceGuid();
	return VaultAmCzarOfAge(ageInstId);
}

void pyVault::RegisterMTStation( const char * stationName, const char * backLinkSpawnPtObjName )
{
	wchar wStationName[256];
	wchar wSpawnPt[256];
	StrToUnicode(wStationName, stationName, arrsize(wStationName));
	StrToUnicode(wSpawnPt, backLinkSpawnPtObjName, arrsize(wSpawnPt));
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
	VaultRegisterVisitAgeAndWait(link.GetAgeLink());
}

void pyVault::UnRegisterVisitAge( const char * guidstr )
{
	Uuid uuid;
	GuidFromString(guidstr, &uuid);
	plAgeInfoStruct info;
	info.SetAgeInstanceGuid(&plUUID(uuid));
	VaultUnregisterVisitAgeAndWait(&info);
}

void pyVault::InvitePlayerToAge( const pyAgeLinkStruct & link, UInt32 playerID )
{
	ENetError error;
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_TextNote);
	VaultTextNoteNode visitAcc(templateNode);
	visitAcc.SetNoteType(plVault::kNoteType_Visit);
	visitAcc.SetVisitInfo(*link.GetAgeLink()->GetAgeInfo());
	if (RelVaultNode * rvn = VaultCreateNodeAndWaitIncRef(templateNode, &error)) {
		VaultSendNode(rvn, playerID);
		rvn->DecRef();
	}
	templateNode->DecRef();
}

void pyVault::UnInvitePlayerToAge( const char * str, UInt32 playerID )
{
	plAgeInfoStruct info;
	info.SetAgeInstanceGuid(&plUUID(str));

	if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(&info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode ageInfo(rvnInfo);
			ageInfo.CopyTo(&info);
			rvnInfo->DecRef();
		}

		rvnLink->DecRef();
	}

	ENetError error;
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_TextNote);
	VaultTextNoteNode visitAcc(templateNode);
	visitAcc.SetNoteType(plVault::kNoteType_UnVisit);
	visitAcc.SetVisitInfo(info);
	if (RelVaultNode * rvn = VaultCreateNodeAndWaitIncRef(templateNode, &error)) {
		VaultSendNode(rvn, playerID);
		rvn->DecRef();
	}
	templateNode->DecRef();
}

void pyVault::OfferLinkToPlayer( const pyAgeLinkStruct & link, UInt32 playerID )
{
	hsAssert(false, "eric, port me");
}

void pyVault::CreateNeighborhood()
{
	plNetClientMgr * nc = plNetClientMgr::GetInstance();

	// Unregister old hood	
	plAgeInfoStruct info;
	info.SetAgeFilename(kNeighborhoodAgeFilename);
	VaultUnregisterOwnedAgeAndWait(&info);

	// Register new hood	
	plAgeLinkStruct link;
	link.GetAgeInfo()->SetAgeFilename(kNeighborhoodAgeFilename);
	link.GetAgeInfo()->SetAgeInstanceName(kNeighborhoodAgeInstanceName);

	std::string title;
	std::string desc;

	unsigned nameLen = StrLen(nc->GetPlayerName());
	if (nc->GetPlayerName()[nameLen - 1] == 's' || nc->GetPlayerName()[nameLen - 1] == 'S')
	{
		xtl::format( title, "%s'", nc->GetPlayerName() );
		xtl::format( desc, "%s' %s", nc->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName() );
	}
	else
	{
		xtl::format( title, "%s's", nc->GetPlayerName() );
		xtl::format( desc, "%s's %s", nc->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName() );
	}

	link.GetAgeInfo()->SetAgeInstanceGuid(&plUUID(GuidGenerate()));
	link.GetAgeInfo()->SetAgeUserDefinedName( title.c_str() );
	link.GetAgeInfo()->SetAgeDescription( desc.c_str() );

	VaultRegisterOwnedAgeAndWait(&link);
}

bool pyVault::SetAgePublic( const pyAgeInfoStruct * ageInfo, bool makePublic )
{
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


