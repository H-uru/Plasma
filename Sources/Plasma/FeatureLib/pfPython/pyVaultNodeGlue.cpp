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
#include "pyVaultNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultNode, pyVaultNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultNode, pyVaultNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultNode)

PYTHON_NO_INIT_DEFINITION(ptVaultNode)

PYTHON_RICH_COMPARE_DEFINITION(ptVaultNode, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None))
	{
		bool isEqual = (obj1 == obj2);

		if (compareType == Py_EQ)
		{
			if (isEqual)
				PYTHON_RCOMPARE_TRUE;
			else
				PYTHON_RCOMPARE_FALSE;
		}
		else if (compareType == Py_NE)
		{
			if (isEqual)
				PYTHON_RCOMPARE_FALSE;
			else
				PYTHON_RCOMPARE_TRUE;
		}
		PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptVaultNode object");
		PYTHON_RCOMPARE_ERROR;
	}

	if (!pyVaultNode::Check(obj1) || !pyVaultNode::Check(obj2))
	{
		PyErr_SetString(PyExc_NotImplementedError, "cannot compare ptVaultNode objects to non-ptVaultNode objects");
		PYTHON_RCOMPARE_ERROR;
	}
	pyVaultNode *vnObj1 = pyVaultNode::ConvertFrom(obj1);
	pyVaultNode *vnObj2 = pyVaultNode::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*vnObj1) == (*vnObj2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*vnObj1) != (*vnObj2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptVaultNode object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getType)
{
	return PyLong_FromUnsignedLong(self->fThis->GetType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getOwnerNodeID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetOwnerNodeID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getOwnerNode)
{
	return self->fThis->GetOwnerNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getModifyTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetModifyTime());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreatorNodeID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetCreatorNodeID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreatorNode)
{
	return self->fThis->GetCreatorNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreateTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetCreateTime());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreateAgeTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetCreateAgeTime());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreateAgeName)
{
	return PyString_FromString(self->fThis->GetCreateAgeName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreateAgeGuid)
{
	return PyString_FromString(self->fThis->GetCreateAgeGuid());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getCreateAgeCoords)
{
	return self->fThis->GetCreateAgeCoords();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getChildNodeRefList)
{
	return self->fThis->GetChildNodeRefList();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getChildNodeCount)
{
	return PyInt_FromLong(self->fThis->GetChildNodeCount());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, getClientID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetClientID());
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setID, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "setID expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetID(id);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setType, args)
{
	int nodeType;
	if (!PyArg_ParseTuple(args, "i", &nodeType))
	{
		PyErr_SetString(PyExc_TypeError, "setType expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetType(nodeType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setOwnerNodeID, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "setOwnerNodeID expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetOwnerNodeID(id);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setCreatorNodeID, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "setCreatorNodeID expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetCreatorNodeID(id);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setCreateAgeName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setCreateAgeName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetCreateAgeName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, setCreateAgeGuid, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setCreateAgeGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetCreateAgeGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptVaultNode, removeAllNodes, RemoveAllNodes)

PYTHON_METHOD_DEFINITION(ptVaultNode, hasNode, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "hasNode expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->HasNode(id));
}

PYTHON_METHOD_DEFINITION(ptVaultNode, getNode, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "getNode expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetNode2(id);
}

PYTHON_METHOD_DEFINITION(ptVaultNode, getChildNode, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "l", &id))
	{
		PyErr_SetString(PyExc_TypeError, "getNode expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetChildNode(id);
}


PYTHON_METHOD_DEFINITION(ptVaultNode, findNode, args)
{
	PyObject* nodeObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &nodeObj))
	{
		PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVaultNode::Check(nodeObj))
	{
		PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
		PYTHON_RETURN_ERROR;
	}
	pyVaultNode* node = pyVaultNode::ConvertFrom(nodeObj);
	return self->fThis->FindNode(node);
}

PYTHON_METHOD_DEFINITION(ptVaultNode, addNode, args)
{
	PyObject* nodeObj = NULL;
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "O|Ol", &nodeObj, &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "addNode expects a ptVaultNode, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVaultNode::Check(nodeObj))
	{
		PyErr_SetString(PyExc_TypeError, "addNode expects a ptVaultNode, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyVaultNode* node = pyVaultNode::ConvertFrom(nodeObj);
	return self->fThis->AddNode(node, cb, cbContext);
}

PYTHON_METHOD_DEFINITION(ptVaultNode, linkToNode, args)
{
	int nodeID;
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "i|Ol", &nodeID, &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "linkToNode expects an int, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->LinkToNode(nodeID, cb, cbContext);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, removeNode, args)
{
	PyObject* nodeObj = NULL;
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "O|Ol", &nodeObj, &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "removeNode expects a ptVaultNode, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVaultNode::Check(nodeObj))
	{
		PyErr_SetString(PyExc_TypeError, "removeNode expects a ptVaultNode, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyVaultNode* node = pyVaultNode::ConvertFrom(nodeObj);
	PYTHON_RETURN_BOOL(self->fThis->RemoveNode(*node, cb, cbContext));
}

PYTHON_METHOD_DEFINITION(ptVaultNode, save, args)
{
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "|Ol", &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "save expects an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Save(cb, cbContext);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, saveAll, args)
{
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "|Ol", &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "saveAll expects an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SaveAll(cb, cbContext);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, forceSave)
{
	self->fThis->ForceSave();
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultNode, sendTo, args)
{
	unsigned long destNodeID;
	PyObject* cb = NULL;
	unsigned long cbContext = 0;
	if (!PyArg_ParseTuple(args, "l|Ol", &destNodeID, &cb, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "sendTo expects an unsigned long, and an optional object and unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SendTo(destNodeID, cb, cbContext);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToFolderNode)
{
	return self->fThis->UpcastToFolderNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToPlayerInfoListNode)
{
	return self->fThis->UpcastToPlayerInfoListNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToImageNode)
{
	return self->fThis->UpcastToImageNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToTextNoteNode)
{
	return self->fThis->UpcastToTextNoteNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToAgeLinkNode)
{
	return self->fThis->UpcastToAgeLinkNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToChronicleNode)
{
	return self->fThis->UpcastToChronicleNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToPlayerInfoNode)
{
	return self->fThis->UpcastToPlayerInfoNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToMarkerGameNode)
{
	return self->fThis->UpcastToMarkerGameNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToAgeInfoNode)
{
	return self->fThis->UpcastToAgeInfoNode();
}


PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToAgeInfoListNode)
{
	return self->fThis->UpcastToAgeInfoListNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToSDLNode)
{
	return self->fThis->UpcastToSDLNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToPlayerNode)
{
	return self->fThis->UpcastToPlayerNode();
}

#ifndef BUILDING_PYPLASMA
PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNode, upcastToSystemNode)
{
	return self->fThis->UpcastToSystemNode();
}
#endif

PYTHON_START_METHODS_TABLE(ptVaultNode)
	PYTHON_METHOD_NOARGS(ptVaultNode, getID, "Returns the unique ID of this ptVaultNode."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getType, "Returns the type of ptVaultNode this is.\n"
				"See PlasmaVaultTypes.py"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getOwnerNodeID, "Returns the node ID of the owner of this node"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getOwnerNode, "Returns a ptVaultNode of the owner of this node"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getModifyTime, "Returns the modified time of this node, that is useable by python's time library."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreatorNodeID, "Returns the creator's node ID"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreatorNode, "Returns the creator's node"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreateTime, "Returns the when this node was created, that is useable by python's time library."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreateAgeTime, "Returns the time in the Age that the node was created...(?)"),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreateAgeName, "Returns the name of the Age where this node was created."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreateAgeGuid, "Returns the guid as a string of the Age where this node was created."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getCreateAgeCoords, "Returns the location in the Age where this node was created."),

	PYTHON_METHOD_NOARGS(ptVaultNode, getChildNodeRefList, "Returns a list of ptVaultNodeRef that are the children of this node."),
	PYTHON_METHOD_NOARGS(ptVaultNode, getChildNodeCount, "Returns how many children this node has."),

	PYTHON_METHOD_NOARGS(ptVaultNode, getClientID, "Returns the client's ID."),

	PYTHON_METHOD(ptVaultNode, setID, "Params: id\nSets ID of this ptVaultNode."),
	PYTHON_METHOD(ptVaultNode, setType, "Params: type\nSet the type of ptVaultNode this is."),
	PYTHON_METHOD(ptVaultNode, setOwnerNodeID, "Params: id\nSet node ID of the owner of this node"),
	PYTHON_METHOD(ptVaultNode, setCreatorNodeID, "Params: id\nSet creator's node ID"),
	PYTHON_METHOD(ptVaultNode, setCreateAgeName, "Params: name\nSet name of the Age where this node was created."),
	PYTHON_METHOD(ptVaultNode, setCreateAgeGuid, "Params: guid\nSet guid as a string of the Age where this node was created."),

	PYTHON_BASIC_METHOD(ptVaultNode, removeAllNodes, "Removes all the child nodes on this node."),
	PYTHON_METHOD(ptVaultNode, hasNode, "Params: id\nReturns true if node if a child node"),
	PYTHON_METHOD(ptVaultNode, getNode, "Params: id\nReturns ptVaultNodeRef if is a child node, or None"),
	PYTHON_METHOD(ptVaultNode, findNode, "Params: templateNode\nReturns ptVaultNode if child node found matching template, or None"),

	PYTHON_METHOD(ptVaultNode, addNode, "Params: node,cb=None,cbContext=0\nAdds 'node'(ptVaultNode) as a child to this node."),
	PYTHON_METHOD(ptVaultNode, linkToNode, "Params: nodeID,cb=None,cbContext=0\nAdds a link to the node designated by nodeID"),
	PYTHON_METHOD(ptVaultNode, removeNode, "Params: node,cb=None,cbContext=0\nRemoves the child 'node'(ptVaultNode) from this node."),
	PYTHON_METHOD(ptVaultNode, save, "Params: cb=None,cbContext=0\nSave the changes made to this node."),
	PYTHON_METHOD(ptVaultNode, saveAll, "Params: cb=None,cbContext=0\nSaves this node and all its children nodes."),
	PYTHON_METHOD_NOARGS(ptVaultNode, forceSave, "Force the current node to save immediately"),
	PYTHON_METHOD(ptVaultNode, sendTo, "Params: destID,cb=None,cbContext=0\nSend this node to inbox at 'destID'"),

	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToFolderNode, "Returns this ptVaultNode as ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToPlayerInfoListNode, "Returns this ptVaultNode as ptVaultPlayerInfoListNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToImageNode, "Returns this ptVaultNode as ptVaultImageNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToTextNoteNode, "Returns this ptVaultNode as ptVaultTextNoteNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToAgeLinkNode, "Returns this ptVaultNode as ptVaultAgeLinkNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToChronicleNode, "Returns this ptVaultNode as ptVaultChronicleNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToPlayerInfoNode, "Returns this ptVaultNode as ptVaultPlayerInfoNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToMarkerGameNode, "Returns this ptVaultNode as ptVaultMarkerNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToAgeInfoNode, "Returns this ptVaultNode as ptVaultAgeInfoNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToAgeInfoListNode, "Returns this ptVaultNode as ptVaultAgeInfoListNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToSDLNode, "Returns this ptVaultNode as a ptVaultSDLNode"),
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToPlayerNode, "Returns this ptVaultNode as a ptVaultPlayerNode"),
#ifndef BUILDING_PYPLASMA
	PYTHON_METHOD_NOARGS(ptVaultNode, upcastToSystemNode, "Returns this ptVaultNode as a ptVaultSystemNode"),
#endif
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptVaultNode_COMPARE			PYTHON_NO_COMPARE
#define ptVaultNode_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptVaultNode_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptVaultNode_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptVaultNode_STR				PYTHON_NO_STR
#define ptVaultNode_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptVaultNode)
#define ptVaultNode_GETSET			PYTHON_NO_GETSET
#define ptVaultNode_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptVaultNode, "Vault node class");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVaultNode, pyVaultNode);

// required functions for PyObject interoperability
PyObject *pyVaultNode::New(RelVaultNode* nfsNode)
{
	ptVaultNode *newObj = (ptVaultNode*)ptVaultNode_type.tp_new(&ptVaultNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultNode, pyVaultNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultNode, pyVaultNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultNode);
	PYTHON_CLASS_IMPORT_END(m);
}