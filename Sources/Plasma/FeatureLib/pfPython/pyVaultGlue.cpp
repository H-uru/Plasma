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

#include "pyVault.h"

#include <string_theory/string>

#include "plMessage/plVaultNotifyMsg.h"
#include "plVault/plVault.h"

#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"
#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pySDL.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultNode.h"

#ifndef BUILDING_PYPLASMA

// glue functions
PYTHON_CLASS_DEFINITION(ptVault, pyVault);

PYTHON_DEFAULT_NEW_DEFINITION(ptVault, pyVault)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVault)

PYTHON_INIT_DEFINITION(ptVault, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getPlayerInfo)
{
    return self->fThis->GetPlayerInfo();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getKIUsage)
{
    return self->fThis->GetKIUsage();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getInbox)
{
    return self->fThis->GetInbox();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAvatarOutfitFolder)
{
    return self->fThis->GetAvatarOutfitFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAvatarClosetFolder)
{
    return self->fThis->GetAvatarClosetFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getChronicleFolder)
{
    return self->fThis->GetChronicleFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAgeJournalsFolder)
{
    return self->fThis->GetAgeJournalsFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAllPlayersFolder)
{
    return self->fThis->GetAllPlayersFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getIgnoreListFolder)
{
    return self->fThis->GetIgnoreListFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getBuddyListFolder)
{
    return self->fThis->GetBuddyListFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getPeopleIKnowAboutFolder)
{
    return self->fThis->GetPeopleIKnowAboutFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAgesICanVisitFolder)
{
    return self->fThis->GetAgesICanVisitFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getAgesIOwnFolder)
{
    return self->fThis->GetAgesIOwnFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getInviteFolder)
{
    return self->fThis->GetInviteFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getLinkToMyNeighborhood)
{
    return self->fThis->GetLinkToMyNeighborhood();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getLinkToCity)
{
    return self->fThis->GetLinkToCity();
}

PYTHON_METHOD_DEFINITION(ptVault, getOwnedAgeLink, args)
{
    PyObject* ageInfoObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "getOwnedAgeLink expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeInfoStruct::Check(ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "getOwnedAgeLink expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
    return self->fThis->GetOwnedAgeLink(*ageInfo);
}

PYTHON_METHOD_DEFINITION(ptVault, getVisitAgeLink, args)
{
    PyObject* ageInfoObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "getVisitAgeLink expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeInfoStruct::Check(ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "getVisitAgeLink expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
    return self->fThis->GetVisitAgeLink(*ageInfo);
}

PYTHON_METHOD_DEFINITION(ptVault, findChronicleEntry, args)
{
    ST::string entryName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &entryName))
    {
        PyErr_SetString(PyExc_TypeError, "findChronicleEntry expects a string");
        PYTHON_RETURN_ERROR;
    }
    return self->fThis->FindChronicleEntry(entryName);
}

PYTHON_METHOD_DEFINITION(ptVault, addChronicleEntry, args)
{
    ST::string entryName;
    unsigned long entryType;
    ST::string entryValue;
    if (!PyArg_ParseTuple(args, "O&lO&", PyUnicode_STStringConverter, &entryName, &entryType, PyUnicode_STStringConverter, &entryValue))
    {
        PyErr_SetString(PyExc_TypeError, "addChronicleEntry expects a string, an unsigned long, and a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddChronicleEntry(entryName, entryType, entryValue);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getGlobalInbox)
{
    return self->fThis->GetGlobalInbox();
}

PYTHON_METHOD_DEFINITION(ptVault, findNode, args)
{
    PyObject* templateNodeObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &templateNodeObj))
    {
        PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
        PYTHON_RETURN_ERROR;
    }
    if (!pyVaultNode::Check(templateNodeObj))
    {
        PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
        PYTHON_RETURN_ERROR;
    }
    pyVaultNode* templateNode = pyVaultNode::ConvertFrom(templateNodeObj);
    return self->fThis->FindNode(templateNode);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, getPsnlAgeSDL)
{
    return self->fThis->GetPsnlAgeSDL();
}

PYTHON_METHOD_DEFINITION(ptVault, updatePsnlAgeSDL, args)
{
    PyObject* pyrecObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &pyrecObj))
    {
        PyErr_SetString(PyExc_TypeError, "updatePsnlAgeSDL expects a ptSDLStateDataRecord");
        PYTHON_RETURN_ERROR;
    }
    if (!pySDLStateDataRecord::Check(pyrecObj))
    {
        PyErr_SetString(PyExc_TypeError, "updatePsnlAgeSDL expects a ptSDLStateDataRecord");
        PYTHON_RETURN_ERROR;
    }
    pySDLStateDataRecord* pyrec = pySDLStateDataRecord::ConvertFrom(pyrecObj);
    self->fThis->UpdatePsnlAgeSDL(*pyrec);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, inMyPersonalAge)
{
    PYTHON_RETURN_BOOL(self->fThis->InMyPersonalAge());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, inMyNeighborhoodAge)
{
    PYTHON_RETURN_BOOL(self->fThis->InMyNeighborhoodAge());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, amOwnerOfCurrentAge)
{
    PYTHON_RETURN_BOOL(self->fThis->AmOwnerOfCurrentAge());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVault, amCzarOfCurrentAge)
{
    PYTHON_RETURN_BOOL(self->fThis->AmCzarOfCurrentAge());
}

PYTHON_METHOD_DEFINITION(ptVault, amAgeOwner, args)
{
    PyObject* ageInfoObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "amAgeOwner expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeInfoStruct::Check(ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "amAgeOwner expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
    PYTHON_RETURN_BOOL(self->fThis->AmAgeOwner(ageInfo));
}

PYTHON_METHOD_DEFINITION(ptVault, amAgeCzar, args)
{
    PyObject* ageInfoObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "amAgeCzar expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeInfoStruct::Check(ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "amAgeCzar expects a ptAgeInfoStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
    PYTHON_RETURN_BOOL(self->fThis->AmAgeCzar(ageInfo));
}

PYTHON_METHOD_DEFINITION(ptVault, registerMTStation, args)
{
    ST::string stationName;
    ST::string mtSpawnPoint;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_STStringConverter, &stationName, PyUnicode_STStringConverter, &mtSpawnPoint))
    {
        PyErr_SetString(PyExc_TypeError, "registerMTStation expects two strings");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RegisterMTStation(stationName, mtSpawnPoint);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, registerOwnedAge, args)
{
    PyObject* ageLinkObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerOwnedAge expects a ptAgeLinkStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerOwnedAge expects a ptAgeLinkStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->RegisterOwnedAge(*ageLink);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, unRegisterOwnedAge, args)
{
    ST::string ageFilename;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &ageFilename))
    {
        PyErr_SetString(PyExc_TypeError, "unRegisterOwnedAge expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->UnRegisterOwnedAge(ageFilename);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, registerVisitAge, args)
{
    PyObject* ageLinkObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerVisitAge expects a ptAgeLinkStruct");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerVisitAge expects a ptAgeLinkStruct");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->RegisterVisitAge(*ageLink);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, unRegisterVisitAge, args)
{
    ST::string guid;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guid))
    {
        PyErr_SetString(PyExc_TypeError, "unRegisterVisitAge expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->UnRegisterVisitAge(guid);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, invitePlayerToAge, args)
{
    PyObject* ageLinkObj = nullptr;
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "Ol", &ageLinkObj, &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "invitePlayerToAge expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "invitePlayerToAge expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->InvitePlayerToAge(*ageLink, playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, unInvitePlayerToAge, args)
{
    ST::string guid;
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "O&l", PyUnicode_STStringConverter, &guid, &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "unInvitePlayerToAge expects a string and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->UnInvitePlayerToAge(guid, playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVault, offerLinkToPlayer, args)
{
    PyObject* ageLinkObj = nullptr;
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "Ol", &ageLinkObj, &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "offerLinkToPlayer expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "offerLinkToPlayer expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->OfferLinkToPlayer(*ageLink, playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptVault, createNeighborhood, CreateNeighborhood)

PYTHON_METHOD_DEFINITION(ptVault, setAgePublic, args)
{
    PyObject* ageInfoObj = nullptr;
    char makePublic;
    if (!PyArg_ParseTuple(args, "Ob", &ageInfoObj, &makePublic))
    {
        PyErr_SetString(PyExc_TypeError, "setAgePublic expects a ptAgeInfoStruct or ptVaultAgeInfoNode and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (pyAgeInfoStruct::Check(ageInfoObj))
    {
        pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
        PYTHON_RETURN_BOOL(self->fThis->SetAgePublic(ageInfo, makePublic != 0));
    }
    else if (pyVaultAgeInfoNode::Check(ageInfoObj))
    {
        pyVaultAgeInfoNode* ageInfoNode = pyVaultAgeInfoNode::ConvertFrom(ageInfoObj);
        PYTHON_RETURN_BOOL(self->fThis->SetAgePublic(ageInfoNode, makePublic != 0));
    }
    PyErr_SetString(PyExc_TypeError, "setAgePublic expects a ptAgeInfoStruct or ptVaultAgeInfoNode and a boolean");
    PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptVault)
    PYTHON_METHOD_NOARGS(ptVault, getPlayerInfo, "Returns a ptVaultNode of type kNodeTypePlayerInfo of the current player"),
    PYTHON_METHOD_NOARGS(ptVault, getKIUsage, "Returns a tuple with usage statistics of the KI (# of pics, # of text notes, # of marker games)"),
    PYTHON_METHOD_NOARGS(ptVault, getInbox, "Returns a ptVaultFolderNode of the current player's inbox folder."),
    PYTHON_METHOD_NOARGS(ptVault, getAvatarOutfitFolder, "Do not use.\n"
                "Returns a ptVaultFolderNode of the avatars outfit."),
    PYTHON_METHOD_NOARGS(ptVault, getAvatarClosetFolder, "Do not use.\n"
                "Returns a ptVaultFolderNode of the avatars outfit in their closet."),
    PYTHON_METHOD_NOARGS(ptVault, getChronicleFolder, "Returns a ptVaultFolderNode of the current player's chronicle folder."),
    PYTHON_METHOD_NOARGS(ptVault, getAgeJournalsFolder, "Returns a ptVaultFolderNode of the current player's age journals folder."),
    PYTHON_METHOD_NOARGS(ptVault, getAllPlayersFolder, "Returns a ptVaultPlayerInfoListNode of the all players folder."),
    PYTHON_METHOD_NOARGS(ptVault, getIgnoreListFolder, "Returns a ptVaultPlayerInfoListNode of the current player's ignore list folder."),
    PYTHON_METHOD_NOARGS(ptVault, getBuddyListFolder, "Returns a ptVaultPlayerInfoListNode of the current player's buddy list folder."),
    PYTHON_METHOD_NOARGS(ptVault, getPeopleIKnowAboutFolder, "Returns a ptVaultPlayerInfoListNode of the current player's people I know about (Recent) list folder."),
    PYTHON_METHOD_NOARGS(ptVault, getAgesICanVisitFolder, "Returns a ptVaultFolderNode of ages I can visit"),
    PYTHON_METHOD_NOARGS(ptVault, getAgesIOwnFolder, "Returns a ptVaultFolderNode of ages that I own"),
    PYTHON_METHOD_NOARGS(ptVault, getInviteFolder, "Returns a ptVaultFolderNode of invites"),
    PYTHON_METHOD_NOARGS(ptVault, getLinkToMyNeighborhood, "Returns a ptVaultAgeLinkNode that will go to my neighborhood"),
    PYTHON_METHOD_NOARGS(ptVault, getLinkToCity, "Returns a ptVaultAgeLinkNode that will go to the city"),
    PYTHON_METHOD(ptVault, getOwnedAgeLink, "Params: ageInfo\nReturns a ptVaultAgeLinkNode to my owned age(ageInfo)"),
    PYTHON_METHOD(ptVault, getVisitAgeLink, "Params: ageInfo\nReturns a ptVaultAgeLinkNode for a visitor to age(ageInfo)"),
    PYTHON_METHOD(ptVault, findChronicleEntry, "Params: entryName\nReturns a ptVaultNode of type kNodeTypeChronicle of the current player's chronicle entry by entryName."),
    PYTHON_METHOD(ptVault, addChronicleEntry, "Params: entryName,type,string\nAdds an entry to the player's chronicle with a value of 'string'."),
    PYTHON_METHOD_NOARGS(ptVault, getGlobalInbox, "Returns a ptVaultFolderNode of the global inbox folder."),
#ifdef GlobalInboxTestCode
    PYTHON_BASIC_METHOD(ptVault, createGlobalInbox, "Creates the global inbox folder."),
#endif
    PYTHON_METHOD(ptVault, findNode, "Params: templateNode\nFind the node matching the template"),
    PYTHON_METHOD_NOARGS(ptVault, getPsnlAgeSDL, "Returns the personal age SDL"),
    PYTHON_METHOD(ptVault, updatePsnlAgeSDL, "Params: pyrec\nUpdates the personal age SDL to the specified data"),
    PYTHON_METHOD_NOARGS(ptVault, inMyPersonalAge, "Are we in the player's personal age?"),
    PYTHON_METHOD_NOARGS(ptVault, inMyNeighborhoodAge, "Are we in the player's neighborhood age?"),
    PYTHON_METHOD_NOARGS(ptVault, amOwnerOfCurrentAge, "Are we the owner of the current age?"),
    PYTHON_METHOD_NOARGS(ptVault, amCzarOfCurrentAge, "Are we the czar (WTH is this?) of the current age?"),
    PYTHON_METHOD(ptVault, amAgeOwner, "Params: ageInfo\nAre we the owner of the specified age?"),
    PYTHON_METHOD(ptVault, amAgeCzar, "Params: ageInfo\nAre we the czar (WTH is this?) of the specified age?"),
    PYTHON_METHOD(ptVault, registerMTStation, "Params: stationName,mtSpawnPoint\nRegisters this player at the specified mass-transit point"),
    PYTHON_METHOD(ptVault, registerOwnedAge, "Params: link\nRegisters the specified age as owned by the player"),
    PYTHON_METHOD(ptVault, unRegisterOwnedAge, "Params: ageFilename\nUnregisters the specified age so it's no longer owned by this player"),
    PYTHON_METHOD(ptVault, registerVisitAge, "Params: link\nRegister this age as visitable by this player"),
    PYTHON_METHOD(ptVault, unRegisterVisitAge, "Params: guid\nUnregisters the specified age so it can no longer be visited by this player"),
    PYTHON_METHOD(ptVault, invitePlayerToAge, "Params: link,playerID\nSends an invitation to visit the age to the specified player"),
    PYTHON_METHOD(ptVault, unInvitePlayerToAge, "Params: guid,playerID\nRevokes the invitation to visit the age"),
    PYTHON_METHOD(ptVault, offerLinkToPlayer, "Params: link,playerID\nOffer a one-time link to the specified player"),
    PYTHON_BASIC_METHOD(ptVault, createNeighborhood, "Creates a new neighborhood"),
    PYTHON_METHOD(ptVault, setAgePublic, "Params: ageInfo,makePublic\nMakes the specified age public or private"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptVault, "Accessor class to the player's vault");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVault, pyVault)

PYTHON_CLASS_CHECK_IMPL(ptVault, pyVault)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVault, pyVault)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVault::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVault);
    PYTHON_CLASS_IMPORT_END(m);
}

#endif // BUILDING_PYPLASMA

void pyVault::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtVaultNodeTypes)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kInvalidNode,             plVault::kNodeType_Invalid)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kVNodeMgrPlayerNode,      plVault::kNodeType_VNodeMgrPlayer)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kVNodeMgrAgeNode,         plVault::kNodeType_VNodeMgrAge)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kFolderNode,              plVault::kNodeType_Folder)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kPlayerInfoNode,          plVault::kNodeType_PlayerInfo)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kImageNode,               plVault::kNodeType_Image)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kTextNoteNode,            plVault::kNodeType_TextNote)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kSDLNode,                 plVault::kNodeType_SDL)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kAgeLinkNode,             plVault::kNodeType_AgeLink)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kChronicleNode,           plVault::kNodeType_Chronicle)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kPlayerInfoListNode,      plVault::kNodeType_PlayerInfoList)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kAgeInfoNode,             plVault::kNodeType_AgeInfo)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kAgeInfoListNode,         plVault::kNodeType_AgeInfoList)
    PYTHON_ENUM_ELEMENT(PtVaultNodeTypes, kMarkerGameNode,          plVault::kNodeType_MarkerGame)
    PYTHON_ENUM_END(m, PtVaultNodeTypes)

    PYTHON_ENUM_START(m, PtVaultStandardNodes)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kUserDefinedNode,             plVault::kUserDefinedNode)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kInboxFolder,                 plVault::kInboxFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kBuddyListFolder,             plVault::kBuddyListFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kIgnoreListFolder,            plVault::kIgnoreListFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kPeopleIKnowAboutFolder,      plVault::kPeopleIKnowAboutFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kChronicleFolder,             plVault::kChronicleFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAvatarOutfitFolder,          plVault::kAvatarOutfitFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgeTypeJournalFolder,        plVault::kAgeTypeJournalFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kSubAgesFolder,               plVault::kSubAgesFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kHoodMembersFolder,           plVault::kHoodMembersFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAllPlayersFolder,            plVault::kAllPlayersFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAllAgeGlobalSDLNodesFolder,  plVault::kAllAgeGlobalSDLNodesFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgeMembersFolder,            plVault::kAgeMembersFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgeJournalsFolder,           plVault::kAgeJournalsFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgeInstanceSDLNode,          plVault::kAgeInstanceSDLNode)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kCanVisitFolder,              plVault::kCanVisitFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgeOwnersFolder,             plVault::kAgeOwnersFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kPlayerInfoNode,              plVault::kPlayerInfoNode)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kPublicAgesFolder,            plVault::kPublicAgesFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgesIOwnFolder,              plVault::kAgesIOwnFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAgesICanVisitFolder,         plVault::kAgesICanVisitFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kAvatarClosetFolder,          plVault::kAvatarClosetFolder)
    PYTHON_ENUM_ELEMENT(PtVaultStandardNodes, kGlobalInboxFolder,           plVault::kGlobalInboxFolder)
    PYTHON_ENUM_END(m, PtVaultStandardNodes)

    PYTHON_ENUM_START(m, PtVaultTextNoteTypes)
    PYTHON_ENUM_ELEMENT(PtVaultTextNoteTypes, kGeneric,     plVault::kNoteType_Generic)
    PYTHON_ENUM_ELEMENT(PtVaultTextNoteTypes, kCCRPetition, plVault::kNoteType_CCRPetition)
    PYTHON_ENUM_END(m, PtVaultTextNoteTypes)
    
    PYTHON_ENUM_START(m, PtVaultTextNoteSubTypes)
    PYTHON_ENUM_ELEMENT(PtVaultTextNoteSubTypes, kGeneric, plVault::kNoteSubType_Generic)
    PYTHON_ENUM_END(m, PtVaultTextNoteSubTypes)

    PYTHON_ENUM_START(m, PtVaultCallbackTypes)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultConnected,          pyVault::kVaultConnected)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultNodeSaved,          pyVault::kVaultNodeSaved)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultNodeRefAdded,       pyVault::kVaultNodeRefAdded)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultRemovingNodeRef,    pyVault::kVaultRemovingNodeRef)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultNodeRefRemoved,     pyVault::kVaultNodeRefRemoved)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultNodeInitialized,    pyVault::kVaultNodeInitialized)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultOperationFailed,    pyVault::kVaultOperationFailed)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultNodeAdded,          pyVault::kVaultNodeAdded)
    PYTHON_ENUM_ELEMENT(PtVaultCallbackTypes, kVaultDisconnected,       pyVault::kVaultDisconnected)
    PYTHON_ENUM_END(m, PtVaultCallbackTypes)

    PYTHON_ENUM_START(m, PtVaultNotifyTypes)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kRegisteredOwnedAge, plVaultNotifyMsg::kRegisteredOwnedAge)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kRegisteredVisitAge, plVaultNotifyMsg::kRegisteredVisitAge)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kUnRegisteredOwnedAge, plVaultNotifyMsg::kUnRegisteredOwnedAge)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kUnRegisteredVisitAge, plVaultNotifyMsg::kUnRegisteredVisitAge)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kPublicAgeCreated, plVaultNotifyMsg::kPublicAgeCreated)
    PYTHON_ENUM_ELEMENT(PtVaultNotifyTypes, kPublicAgeRemoved, plVaultNotifyMsg::kPublicAgeRemoved)
    PYTHON_ENUM_END(m, PtVaultNotifyTypes)
}
