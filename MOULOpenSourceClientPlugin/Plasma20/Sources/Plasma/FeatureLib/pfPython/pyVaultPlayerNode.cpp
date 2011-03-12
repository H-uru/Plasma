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
// pyVaultPlayerNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultPlayerNode.h"
#ifndef BUILDING_PYPLASMA
#include "pyVault.h"
#endif

#include "../pfPython/pyAgeInfoStruct.h"
#include "../pfPython/pyVaultAgeLinkNode.h"
#include "../pfPython/pyVaultPlayerInfoNode.h"
#include "../pfPython/pyVaultPlayerInfoListNode.h"
#include "../pfPython/pyVaultFolderNode.h"
#include "../pfPython/pyVaultChronicleNode.h"
#include "../pfPython/pyVaultSDLNode.h"
#include "../pfPython/pyAgeLinkStruct.h"

#include "../plVault/plVault.h"

//============================================================================
static PyObject * GetPlayerVaultFolder (unsigned folderType) {
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

pyVaultPlayerNode::pyVaultPlayerNode(RelVaultNode *nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultPlayerNode::pyVaultPlayerNode()
: pyVaultNode(nil)	// may not create player nodes from python
{
}

//==================================================================
// class RelVaultNode : public plVaultNode
//
PyObject *pyVaultPlayerNode::GetInbox()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kInboxFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetAvatarOutfitFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kAvatarOutfitFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetAvatarClosetFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kAvatarClosetFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetChronicleFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kChronicleFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetAgeJournalsFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kAgeJournalsFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetIgnoreListFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kIgnoreListFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetBuddyListFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kBuddyListFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetPeopleIKnowAboutFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kPeopleIKnowAboutFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetAgesICanVisitFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kAgesICanVisitFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetAgesIOwnFolder()
{
	if (PyObject * result = GetPlayerVaultFolder(plVault::kAgesIOwnFolder))
		return result;

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetPlayerInfo()
{
	if (RelVaultNode * rvn = VaultGetPlayerInfoNodeIncRef()) {
		PyObject * result = pyVaultPlayerInfoNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetLinkToMyNeighborhood()
{
	plAgeLinkStruct * link = NEW(plAgeLinkStruct);
	
	if (VaultGetLinkToMyNeighborhood(link)) {
		PyObject * result = pyAgeLinkStruct::New(link);
		return result;
	}

	DEL(link);
	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetLinkToCity()
{
	plAgeLinkStruct * link = NEW(plAgeLinkStruct);
	
	if (VaultGetLinkToCity(link)) {
		PyObject * result = pyAgeLinkStruct::New(link);
		return result;
	}

	DEL(link);
	PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetOwnedAgeLink(const pyAgeInfoStruct *info)
{
	plAgeLinkStruct link;
	if (VaultGetOwnedAgeLink(info->GetAgeInfo(), &link))
		return pyAgeLinkStruct::New(&link);
	
	PYTHON_RETURN_NONE;
}

void pyVaultPlayerNode::RemoveOwnedAgeLink(const char* ageFilename)
{
	plAgeInfoStruct info;
	info.SetAgeFilename(ageFilename);
	VaultUnregisterOwnedAgeAndWait(&info);
}

PyObject *pyVaultPlayerNode::GetVisitAgeLink(const pyAgeInfoStruct *info)
{
	if (RelVaultNode * rvn = VaultGetVisitAgeLinkIncRef(info->GetAgeInfo())) {
		PyObject * result = pyVaultAgeLinkNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	PYTHON_RETURN_NONE;
}

void pyVaultPlayerNode::RemoveVisitAgeLink(const char *guidstr)
{
	Uuid uuid;
	GuidFromString(guidstr, &uuid);
	plAgeInfoStruct info;
	info.SetAgeInstanceGuid(&plUUID(uuid));
	VaultUnregisterOwnedAgeAndWait(&info);
}

PyObject *pyVaultPlayerNode::FindChronicleEntry(const char *entryName)
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, entryName, arrsize(wStr));
	if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(wStr)) {
		PyObject * result = pyVaultChronicleNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	PYTHON_RETURN_NONE;
}

void pyVaultPlayerNode::SetPlayerName(const char *value)
{
	hsAssert(false, "python may not change a player's name this way");
}

std::string pyVaultPlayerNode::GetPlayerName()
{
	if (!fNode)
		return "";

	VaultPlayerNode player(fNode);
	char ansiStr[MAX_PATH];		
	StrToAnsi(ansiStr, player.playerName, arrsize(ansiStr));
	return ansiStr;
}

void pyVaultPlayerNode::SetAvatarShapeName(const char *value)
{
	hsAssert(false, "python may not change a player's avatar this way");
}

std::string pyVaultPlayerNode::GetAvatarShapeName()
{
	if (!fNode)
		return "";

	VaultPlayerNode player(fNode);
	char ansiStr[MAX_PATH];		
	StrToAnsi(ansiStr, player.avatarShapeName, arrsize(ansiStr));
	return ansiStr;
}

void pyVaultPlayerNode::SetDisabled(bool value)
{
	hsAssert(false, "python may not change a player's disable state this way");
}

bool pyVaultPlayerNode::IsDisabled()
{
	if (!fNode)
		return false;

	VaultPlayerNode player(fNode);
	return player.disabled;
}

void pyVaultPlayerNode::SetOnlineTime(UInt32 value)
{
	hsAssert(false, "python may not change a player's online time this way");
}

UInt32 pyVaultPlayerNode::GetOnlineTime()
{
	if (!fNode)
		return 0;

	VaultPlayerNode player(fNode);
	return player.onlineTime;
}

void pyVaultPlayerNode::SetExplorer (bool b) {
	if (!fNode)
		return;

	VaultPlayerNode player(fNode);
	player.SetExplorer(b);
}

hsBool pyVaultPlayerNode::IsExplorer () {
	if (!fNode)
		return false;

	VaultPlayerNode player(fNode);
	return player.explorer;
}
