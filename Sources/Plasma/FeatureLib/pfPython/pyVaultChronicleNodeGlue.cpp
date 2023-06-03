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

#include "pyVaultChronicleNode.h"
#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultChronicleNode, pyVaultChronicleNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultChronicleNode, pyVaultChronicleNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultChronicleNode)

PYTHON_INIT_DEFINITION(ptVaultChronicleNode, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "setName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Chronicle_SetName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getName)
{
    return PyUnicode_FromSTString(self->fThis->Chronicle_GetName());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setValue, args)
{
    ST::string val;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &val))
    {
        PyErr_SetString(PyExc_TypeError, "setValue expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Chronicle_SetValue(val);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getValue)
{
    return PyUnicode_FromSTString(self->fThis->Chronicle_GetValue());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setEntryType, args)
{
    unsigned long chronType;
    if (!PyArg_ParseTuple(args, "l", &chronType))
    {
        PyErr_SetString(PyExc_TypeError, "setEntryType expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Chronicle_SetType(chronType);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getEntryType)
{
    return PyLong_FromUnsignedLong(self->fThis->Chronicle_GetType());
}

PYTHON_START_METHODS_TABLE(ptVaultChronicleNode)
    PYTHON_METHOD(ptVaultChronicleNode, setName, "Params: name\nSets the name of the chronicle node."),
    PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getName, "Returns the name of the chronicle node."),
    PYTHON_METHOD(ptVaultChronicleNode, setValue, "Params: value\nSets the chronicle to a value that is a string"),
    PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getValue, "Returns the value as a string of this chronicle node."),
    PYTHON_METHOD(ptVaultChronicleNode, setEntryType, "Params: type\nSets this chronicle node to a user defined type."),
    PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getEntryType, "Returns the user defined type of the chronicle node."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultChronicleNode, pyVaultNode, "Plasma vault chronicle node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultChronicleNode, pyVaultChronicleNode)

PYTHON_CLASS_CHECK_IMPL(ptVaultChronicleNode, pyVaultChronicleNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultChronicleNode, pyVaultChronicleNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultChronicleNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultChronicleNode);
    PYTHON_CLASS_IMPORT_END(m);
}
