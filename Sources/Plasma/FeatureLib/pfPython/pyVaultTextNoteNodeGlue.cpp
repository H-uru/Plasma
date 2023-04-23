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

#include <Python.h>
#include <string_theory/string>

#include "pyVaultTextNoteNode.h"
#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultTextNoteNode, pyVaultTextNoteNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultTextNoteNode, pyVaultTextNoteNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultTextNoteNode)

PYTHON_INIT_DEFINITION(ptVaultTextNoteNode, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setTitle, args)
{
    ST::string title;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &title))
    {
        PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Note_SetTitle(title);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getTitle)
{
    return PyUnicode_FromSTString(self->fThis->Note_GetTitle());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setText, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "setText expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Note_SetText(text);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getText)
{
    return PyUnicode_FromSTString(self->fThis->Note_GetText());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setNoteType, args)
{
    long nodeType;
    if (!PyArg_ParseTuple(args, "l", &nodeType))
    {
        PyErr_SetString(PyExc_TypeError, "setNoteType expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Note_SetType(nodeType);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getNoteType)
{
    return PyLong_FromLong(self->fThis->Note_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setNoteSubType, args)
{
    long nodeType;
    if (!PyArg_ParseTuple(args, "l", &nodeType))
    {
        PyErr_SetString(PyExc_TypeError, "setNoteSubType expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Note_SetSubType(nodeType);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultTextNoteNode, getNoteSubType)
{
    return PyLong_FromLong(self->fThis->Note_GetSubType());
}

PYTHON_METHOD_DEFINITION(ptVaultTextNoteNode, setDeviceInbox, args)
{
    ST::string inboxName; 
    PyObject* cb = nullptr;
    unsigned long context = 0;
    if (!PyArg_ParseTuple(args, "O&|Ol", PyUnicode_STStringConverter, &inboxName, &cb, &context))
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
    PYTHON_METHOD(ptVaultTextNoteNode, setTitle, "Params: title\nSets the title of this text note node."),
    PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getTitle, "Returns the title of this text note node."),
    PYTHON_METHOD(ptVaultTextNoteNode, setText, "Params: text\nSets text of the this text note node."),
    PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getText, "Returns the text of this text note node."),
    PYTHON_METHOD(ptVaultTextNoteNode, setNoteType, "Params: type\nSets the type of text note for this text note node."),
    PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getNoteType, "Returns the type of text note for this text note node."),
    PYTHON_METHOD(ptVaultTextNoteNode, setNoteSubType, "Params: subType\nSets the subtype of the this text note node."),
    PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getNoteSubType, "Returns the subtype of this text note node."),
    PYTHON_METHOD(ptVaultTextNoteNode, setDeviceInbox, "Params: inboxName,cb=None,cbContext=0\nSets the device inbox"),
    PYTHON_METHOD_NOARGS(ptVaultTextNoteNode, getDeviceInbox, "Returns a ptVaultFolderNode"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultTextNoteNode, pyVaultNode, "Plasma vault text note node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultTextNoteNode, pyVaultTextNoteNode)

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
