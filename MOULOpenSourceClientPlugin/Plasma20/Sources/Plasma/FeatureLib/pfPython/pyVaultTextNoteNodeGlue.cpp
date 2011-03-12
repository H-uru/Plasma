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
#include "pyVaultTextNoteNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultTextNoteNode, pyVaultTextNoteNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultTextNoteNode, pyVaultTextNoteNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultTextNoteNode)

PYTHON_INIT_DEFINITION(ptVaultTextNoteNode, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, noteSetTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "noteSetTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, noteGetTitle)
{
	return PyString_FromString(self->fThis->Note_GetTitle().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, noteSetText, args)
{
	char* text;
	if (!PyArg_ParseTuple(args, "s", &text))
	{
		PyErr_SetString(PyExc_TypeError, "noteSetText expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetText(text);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, noteGetText)
{
	return PyString_FromString(self->fThis->Note_GetText().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, noteSetType, args)
{
	long nodeType;
	if (!PyArg_ParseTuple(args, "l", &nodeType))
	{
		PyErr_SetString(PyExc_TypeError, "noteSetType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetType(nodeType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, noteGetType)
{
	return PyLong_FromLong(self->fThis->Note_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, noteSetSubType, args)
{
	long nodeType;
	if (!PyArg_ParseTuple(args, "l", &nodeType))
	{
		PyErr_SetString(PyExc_TypeError, "noteSetSubType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetSubType(nodeType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, noteGetSubType)
{
	return PyLong_FromLong(self->fThis->Note_GetSubType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setTitleW, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "setTitleW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* title = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, title, strLen);
		title[strLen] = L'\0';
		self->fThis->Note_SetTitleW(title);
		delete [] title;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* title = PyString_AsString(textObj);
		self->fThis->Note_SetTitle(title);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setTitleW expects a unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getTitle)
{
	return PyString_FromString(self->fThis->Note_GetTitle().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getTitleW)
{
	std::wstring retVal = self->fThis->Note_GetTitleW();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setText, args)
{
	char* text;
	if (!PyArg_ParseTuple(args, "s", &text))
	{
		PyErr_SetString(PyExc_TypeError, "setText expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetText(text);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setTextW, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "setTextW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->Note_SetTextW(text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		self->fThis->Note_SetText(text);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setTextW expects a unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getText)
{
	return PyString_FromString(self->fThis->Note_GetText().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getTextW)
{
	std::wstring retVal = self->fThis->Note_GetTextW();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setType, args)
{
	long nodeType;
	if (!PyArg_ParseTuple(args, "l", &nodeType))
	{
		PyErr_SetString(PyExc_TypeError, "setType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetType(nodeType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getType)
{
	return PyLong_FromLong(self->fThis->Note_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setSubType, args)
{
	long nodeType;
	if (!PyArg_ParseTuple(args, "l", &nodeType))
	{
		PyErr_SetString(PyExc_TypeError, "setSubType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Note_SetSubType(nodeType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getSubType)
{
	return PyLong_FromLong(self->fThis->Note_GetSubType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setDeviceInbox, args)
{
	char* inboxName; 
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "s|Ol", &inboxName, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "setDeviceInbox expects a string, an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetDeviceInbox(inboxName, cb, context);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getDeviceInbox)
{
	return self->fThis->GetDeviceInbox();
}

PYTHON_START_METHODS_TABLE(ptVaultTextNoteNode)
	// legacy glue
	PYTHON_METHOD(ptVaultTextNoteNode, noteSetTitle, "Params: title\nLEGACY\nSets the title of this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, noteGetTitle, "LEGACY\nReturns the title of this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, noteSetText, "Params: text\nLEGACY\nSets text of the this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, noteGetText, "LEGACY\nReturns the text of this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, noteSetType, "Params: type\nLEGACY\nSets the type of text note for this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, noteGetType, "LEGACY\nReturns the type of text note for this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, noteSetSubType, "Params: subType\nLEGACY\nSets the subtype of the this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, noteGetSubType, "LEGACY\nReturns the subtype of this text note node."),
	// new glue
	PYTHON_METHOD(ptVaultTextNoteNode, setTitle, "Params: title\nSets the title of this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, setTitleW, "Params: title\nUnicode version of setTitle"),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getTitle, "Returns the title of this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getTitleW, "Unicode version of getTitle"),
	PYTHON_METHOD(ptVaultTextNoteNode, setText, "Params: text\nSets text of the this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, setTextW, "Params: text\nUnicode version of setText"),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getText, "Returns the text of this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getTextW, "Unicode version of getText."),
	PYTHON_METHOD(ptVaultTextNoteNode, setType, "Params: type\nSets the type of text note for this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getType, "Returns the type of text note for this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, setSubType, "Params: subType\nSets the subtype of the this text note node."),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getSubType, "Returns the subtype of this text note node."),
	PYTHON_METHOD(ptVaultTextNoteNode, setDeviceInbox, "Params: inboxName,cb=None,cbContext=0\nSets the device inbox"),
	PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getDeviceInbox, "Returns a ptVaultFolderNode"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultTextNoteNode, pyVaultNode, "Plasma vault text note node");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVaultTextNoteNode, pyVaultTextNoteNode)

PyObject *pyVaultTextNoteNode::New(RelVaultNode* nfsNode)
{
	ptVaultTextNoteNode *newObj = (ptVaultTextNoteNode*)ptVaultTextNoteNode_type.tp_new(&ptVaultTextNoteNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultTextNoteNode, pyVaultTextNoteNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultTextNoteNode, pyVaultTextNoteNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultTextNoteNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultTextNoteNode);
	PYTHON_CLASS_IMPORT_END(m);
}