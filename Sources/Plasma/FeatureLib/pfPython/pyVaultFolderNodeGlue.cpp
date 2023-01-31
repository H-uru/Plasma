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

#include "pyVaultFolderNode.h"

#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultFolderNode, pyVaultFolderNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultFolderNode, pyVaultFolderNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultFolderNode)

PYTHON_INIT_DEFINITION(ptVaultFolderNode, args, keywords)
{
    int n = 0;
    if (!PyArg_ParseTuple(args, "|i", &n))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects an optional int");
        PYTHON_RETURN_INIT_ERROR;
    }
    // we don't really do anything? Not according to the associated constructor. Odd...
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, setFolderType, args)
{
    int folderType;
    if (!PyArg_ParseTuple(args, "i", &folderType))
    {
        PyErr_SetString(PyExc_TypeError, "setFolderType expects an int");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Folder_SetType(folderType);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, getFolderType)
{
    return PyLong_FromLong(self->fThis->Folder_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, setFolderName, args)
{
    PyObject* textObj;
    if (!PyArg_ParseTuple(args, "O", &textObj))
    {
        PyErr_SetString(PyExc_TypeError, "setFolderName expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(textObj))
    {
        wchar_t* name = PyUnicode_AsWideCharString(textObj, nullptr);
        self->fThis->Folder_SetNameW(name);
        PyMem_Free(name);
        PYTHON_RETURN_NONE;
    }
    PyErr_SetString(PyExc_TypeError, "setFolderName expects a unicode string");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, getFolderName)
{
    return PyUnicode_FromSTString(self->fThis->Folder_GetName());
}

PYTHON_START_METHODS_TABLE(ptVaultFolderNode)
    PYTHON_METHOD(ptVaultFolderNode, setFolderType, "Params: type\nSet the folder type"),
    PYTHON_METHOD_NOARGS(ptVaultFolderNode, getFolderType, "Returns the folder type (of the standard folder types)"),
    PYTHON_METHOD(ptVaultFolderNode, setFolderName, "Params: name\nSet the folder name"),
    PYTHON_METHOD_NOARGS(ptVaultFolderNode, getFolderName, "Returns the folder's name"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultFolderNode, pyVaultNode, "Params: n=0\nPlasma vault folder node");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVaultFolderNode, pyVaultFolderNode);

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultFolderNode, pyVaultFolderNode);

PYTHON_CLASS_CHECK_IMPL(ptVaultFolderNode, pyVaultFolderNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultFolderNode, pyVaultFolderNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultFolderNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultFolderNode);
    PYTHON_CLASS_IMPORT_END(m);
}
