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
// pyVaultPlayerNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>
#pragma hdrstop

#include "pyVaultPlayerNode.h"
#include "pyAgeInfoStruct.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultFolderNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultSDLNode.h"
#include "pyAgeLinkStruct.h"
#include "plVault/plVault.h"
#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#endif

//============================================================================
static PyObject * GetPlayerVaultFolder (unsigned folderType) {
    PyObject * result = nil;
    if (RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef()) {
        if (RelVaultNode * rvnFldr = rvnPlr->GetChildFolderNodeIncRef(folderType, 1)) {
            result = pyVaultFolderNode::New(rvnFldr);
            rvnFldr->UnRef();
        }
        rvnPlr->UnRef();
    }
    
    return result;
}

pyVaultPlayerNode::pyVaultPlayerNode(RelVaultNode *nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultPlayerNode::pyVaultPlayerNode()
: pyVaultNode(nil)  // may not create player nodes from python
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
        rvn->UnRef();
        return result;
    }

    PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetLinkToMyNeighborhood()
{
    plAgeLinkStruct link;
    if (VaultGetLinkToMyNeighborhood(&link)) {
        PyObject * result = pyAgeLinkStruct::New(&link);
        return result;
    }

    PYTHON_RETURN_NONE;
}

PyObject *pyVaultPlayerNode::GetLinkToCity()
{
    plAgeLinkStruct link;
    if (VaultGetLinkToCity(&link)) {
        PyObject * result = pyAgeLinkStruct::New(&link);
        return result;
    }

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
        rvn->UnRef();
        return result;
    }

    PYTHON_RETURN_NONE;
}

void pyVaultPlayerNode::RemoveVisitAgeLink(const char *guidstr)
{
    plAgeInfoStruct info;
    plUUID guid(guidstr);
    info.SetAgeInstanceGuid(&guid);
    VaultUnregisterOwnedAgeAndWait(&info);
}

PyObject *pyVaultPlayerNode::FindChronicleEntry(const char *entryName)
{
    wchar_t wStr[MAX_PATH];
    StrToUnicode(wStr, entryName, arrsize(wStr));
    if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(wStr)) {
        PyObject * result = pyVaultChronicleNode::New(rvn);
        rvn->UnRef();
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
    StrToAnsi(ansiStr, player.GetPlayerName(), arrsize(ansiStr));
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
    StrToAnsi(ansiStr, player.GetAvatarShapeName(), arrsize(ansiStr));
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
    return player.GetDisabled();
}

void pyVaultPlayerNode::SetOnlineTime(uint32_t value)
{
    hsAssert(false, "python may not change a player's online time this way");
}

uint32_t pyVaultPlayerNode::GetOnlineTime()
{
    if (!fNode)
        return 0;

    VaultPlayerNode player(fNode);
    return player.GetOnlineTime();
}

void pyVaultPlayerNode::SetExplorer (bool b) {
    if (!fNode)
        return;

    VaultPlayerNode player(fNode);
    player.SetExplorer(b);
}

bool pyVaultPlayerNode::IsExplorer () {
    if (!fNode)
        return false;

    VaultPlayerNode player(fNode);
    return player.GetExplorer();
}
