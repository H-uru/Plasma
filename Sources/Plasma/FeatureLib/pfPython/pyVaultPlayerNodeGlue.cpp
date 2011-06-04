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
#include "pyVaultPlayerNode.h"
#include "pyAgeInfoStruct.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultPlayerNode, pyVaultPlayerNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultPlayerNode, pyVaultPlayerNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultPlayerNode)

PYTHON_INIT_DEFINITION(ptVaultPlayerNode, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getInbox)
{
	return self->fThis->GetInbox();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getPlayerInfo)
{
	return self->fThis->GetPlayerInfo();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAvatarOutfitFolder)
{
	return self->fThis->GetAvatarOutfitFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAvatarClosetFolder)
{
	return self->fThis->GetAvatarClosetFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getChronicleFolder)
{
	return self->fThis->GetChronicleFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAgeJournalsFolder)
{
	return self->fThis->GetAgeJournalsFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getIgnoreListFolder)
{
	return self->fThis->GetIgnoreListFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getBuddyListFolder)
{
	return self->fThis->GetBuddyListFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getPeopleIKnowAboutFolder)
{
	return self->fThis->GetPeopleIKnowAboutFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAgesICanVisitFolder)
{
	return self->fThis->GetAgesICanVisitFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAgesIOwnFolder)
{
	return self->fThis->GetAgesIOwnFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getLinkToMyNeighborhood)
{
	return self->fThis->GetLinkToMyNeighborhood();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getLinkToCity)
{
	return self->fThis->GetLinkToCity();
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, getOwnedAgeLink, args)
{
	PyObject* infoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &infoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getOwnedAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(infoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getOwnedAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* info = pyAgeInfoStruct::ConvertFrom(infoObj);
	return self->fThis->GetOwnedAgeLink(info);
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, removeOwnedAgeLink, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "removeOwnedAgeLink expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RemoveOwnedAgeLink(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, getVisitAgeLink, args)
{
	PyObject* infoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &infoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getVisitAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(infoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getVisitAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* info = pyAgeInfoStruct::ConvertFrom(infoObj);
	return self->fThis->GetVisitAgeLink(info);
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, removeVisitAgeLink, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "removeVisitAgeLink expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RemoveVisitAgeLink(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, findChronicleEntry, args)
{
	char* entryName;
	if (!PyArg_ParseTuple(args, "s", &entryName))
	{
		PyErr_SetString(PyExc_TypeError, "findChronicleEntry expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->FindChronicleEntry(entryName);
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setPlayerName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setPlayerName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetPlayerName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getPlayerName)
{
	return PyString_FromString(self->fThis->GetPlayerName().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setAvatarShapeName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setAvatarShapeName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAvatarShapeName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAvatarShapeName)
{
	return PyString_FromString(self->fThis->GetAvatarShapeName().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setDisabled, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setDisabled expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetDisabled(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, isDisabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsDisabled());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setOnlineTime, args)
{
	unsigned long time;
	if (!PyArg_ParseTuple(args, "l", &time))
	{
		PyErr_SetString(PyExc_TypeError, "setOnlineTime expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetOnlineTime(time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getOnlineTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetOnlineTime());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setExplorer, args)
{
	char explorer;
	if (!PyArg_ParseTuple(args, "b", &explorer))
	{
		PyErr_SetString(PyExc_TypeError, "setExplorer expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetExplorer(explorer != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, isExplorer)
{
	PYTHON_RETURN_BOOL(self->fThis->IsExplorer());
}


PYTHON_BASIC_METHOD_DEFINITION(ptVaultPlayerNode, save, Save)

PYTHON_START_METHODS_TABLE(ptVaultPlayerNode)
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getInbox, "Returns the player's inbox"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getPlayerInfo, "Returns ptVaultPlayerInfoNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAvatarOutfitFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAvatarClosetFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getChronicleFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAgeJournalsFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getIgnoreListFolder, "Returns ptVaultPlayerInfoListNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getBuddyListFolder, "Returns ptVaultPlayerInfoListNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getPeopleIKnowAboutFolder, "Returns ptVaultPlayerInfoListNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAgesICanVisitFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAgesIOwnFolder, "Returns ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getLinkToMyNeighborhood, "Returns ptVaultAgeLinkNode"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getLinkToCity, "Returns ptVaultAgeLinkNode"),
	PYTHON_METHOD(ptVaultPlayerNode, getOwnedAgeLink, "Params: info\nReturns pyVaultAgeLinkNode"),
	PYTHON_METHOD(ptVaultPlayerNode, removeOwnedAgeLink, "Params: guid\nRemoves the specified link"),
	PYTHON_METHOD(ptVaultPlayerNode, getVisitAgeLink, "Params: info\nReturns pyVaultAgeLinkNode"),
	PYTHON_METHOD(ptVaultPlayerNode, removeVisitAgeLink, "Params: guid\nRemoves the specified link"),
	PYTHON_METHOD(ptVaultPlayerNode, findChronicleEntry, "Params: entryName\nReturns ptVaultChronicleNode"),
	PYTHON_METHOD(ptVaultPlayerNode, setPlayerName, "Params: name\nSets the player's name"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getPlayerName, "Returns the player's name"),
	PYTHON_METHOD(ptVaultPlayerNode, setAvatarShapeName, "Params: name\nSets the avatar's 'shape'"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAvatarShapeName, "Returns the avatar's 'shape'"),
	PYTHON_METHOD(ptVaultPlayerNode, setDisabled, "Params: state\nDisables/enables the avatar"),
	PYTHON_METHOD(ptVaultPlayerNode, isDisabled, "Is the avatar disabled?"),
	PYTHON_METHOD(ptVaultPlayerNode, setOnlineTime, "Params: time\nSets the avatar's online time"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getOnlineTime, "Returns the avatar's online time"),
	PYTHON_BASIC_METHOD(ptVaultPlayerNode, save, "Saves the node"),
	PYTHON_METHOD(ptVaultPlayerNode, setExplorer, "Params: boolean\nset true for 'explorer', false for 'visitor'"),
	PYTHON_METHOD_NOARGS(ptVaultPlayerNode, isExplorer, "Returns true for 'explorer', false for 'visitor'."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultPlayerNode, pyVaultNode, "Plasma vault player node");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVaultPlayerNode, pyVaultPlayerNode)

PyObject *pyVaultPlayerNode::New(RelVaultNode* nfsNode)
{
	ptVaultPlayerNode *newObj = (ptVaultPlayerNode*)ptVaultPlayerNode_type.tp_new(&ptVaultPlayerNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultPlayerNode, pyVaultPlayerNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultPlayerNode, pyVaultPlayerNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultPlayerNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultPlayerNode);
	PYTHON_CLASS_IMPORT_END(m);
}