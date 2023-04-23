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

#include "pyVaultAgeLinkNode.h"
#include "pySpawnPointInfo.h"

#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultAgeLinkNode, pyVaultAgeLinkNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultAgeLinkNode, pyVaultAgeLinkNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultAgeLinkNode)

PYTHON_INIT_DEFINITION(ptVaultAgeLinkNode, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeLinkNode, getAgeInfo)
{
    return self->fThis->GetAgeInfo();
}

PYTHON_METHOD_DEFINITION(ptVaultAgeLinkNode, setLocked, args)
{
    char stateFlag;
    if (!PyArg_ParseTuple(args, "b", &stateFlag))
    {
        PyErr_SetString(PyExc_TypeError, "setLocked expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetLocked(stateFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeLinkNode, getLocked)
{
    PYTHON_RETURN_BOOL(self->fThis->GetLocked());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeLinkNode, setVolatile, args)
{
    char stateFlag;
    if (!PyArg_ParseTuple(args, "b", &stateFlag))
    {
        PyErr_SetString(PyExc_TypeError, "setVolatile expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetVolatile(stateFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeLinkNode, getVolatile)
{
    PYTHON_RETURN_BOOL(self->fThis->GetVolatile());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeLinkNode, addSpawnPoint, args)
{
    PyObject* spawnPtObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &spawnPtObj))
    {
        PyErr_SetString(PyExc_TypeError, "addSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
        PYTHON_RETURN_ERROR;
    }
    if (pySpawnPointInfo::Check(spawnPtObj))
    {
        pySpawnPointInfo* spawnPt = pySpawnPointInfo::ConvertFrom(spawnPtObj);
        self->fThis->AddSpawnPoint(*spawnPt);
        PYTHON_RETURN_NONE;
    }
    else if (pySpawnPointInfoRef::Check(spawnPtObj))
    {
        pySpawnPointInfoRef* spawnPt = pySpawnPointInfoRef::ConvertFrom(spawnPtObj);
        self->fThis->AddSpawnPointRef(*spawnPt);
        PYTHON_RETURN_NONE;
    }
    PyErr_SetString(PyExc_TypeError, "addSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptVaultAgeLinkNode, removeSpawnPoint, args)
{
    PyObject* spawnPtObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &spawnPtObj))
    {
        PyErr_SetString(PyExc_TypeError, "removeSpawnPoint expects a ptSpawnPointInfo, a ptSpawnPointInfoRef, or a string");
        PYTHON_RETURN_ERROR;
    }
    if (pySpawnPointInfo::Check(spawnPtObj))
    {
        pySpawnPointInfo* spawnPt = pySpawnPointInfo::ConvertFrom(spawnPtObj);
        self->fThis->RemoveSpawnPoint(*spawnPt);
        PYTHON_RETURN_NONE;
    }
    else if (pySpawnPointInfoRef::Check(spawnPtObj))
    {
        pySpawnPointInfoRef* spawnPt = pySpawnPointInfoRef::ConvertFrom(spawnPtObj);
        self->fThis->RemoveSpawnPointRef(*spawnPt);
        PYTHON_RETURN_NONE;
    }
    else if (PyUnicode_Check(spawnPtObj))
    {
        self->fThis->RemoveSpawnPointByName(PyUnicode_AsSTString(spawnPtObj));
        PYTHON_RETURN_NONE;
    }
    PyErr_SetString(PyExc_TypeError, "removeSpawnPoint expects a ptSpawnPointInfo, a ptSpawnPointInfoRef, or a string");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptVaultAgeLinkNode, hasSpawnPoint, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "hasSpawnPoint expects a string");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_BOOL(self->fThis->HasSpawnPoint(name));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeLinkNode, getSpawnPoints)
{
    return self->fThis->GetSpawnPoints();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeLinkNode, asAgeLinkStruct)
{
    return self->fThis->AsAgeLinkStruct();
}

PYTHON_START_METHODS_TABLE(ptVaultAgeLinkNode)
    PYTHON_METHOD_NOARGS(ptVaultAgeLinkNode, getAgeInfo, "Returns the ageInfo as a ptAgeInfoStruct"),
    PYTHON_METHOD(ptVaultAgeLinkNode, setLocked, "Params: state\nSets whether the link is locked or not"),
    PYTHON_METHOD_NOARGS(ptVaultAgeLinkNode, getLocked, "Returns whether the link is locked or not"),
    PYTHON_METHOD(ptVaultAgeLinkNode, setVolatile, "Params: state\nSets the state of the volitility of the link"),
    PYTHON_METHOD_NOARGS(ptVaultAgeLinkNode, getVolatile, "Returns whether the link is volatile or not"),
    PYTHON_METHOD(ptVaultAgeLinkNode, addSpawnPoint, "Params: point\nAdds the specified ptSpawnPointInfo or ptSpawnPointInfoRef"),
    PYTHON_METHOD(ptVaultAgeLinkNode, removeSpawnPoint, "Params: point\nRemoves the specified spawn point based on a ptSpawnPointInfo, ptSpawnPointInfoRef, or string"),
    PYTHON_METHOD(ptVaultAgeLinkNode, hasSpawnPoint, "Params: spawnPtName\nReturns true if this link has the specified spawn point"),
    PYTHON_METHOD_NOARGS(ptVaultAgeLinkNode, getSpawnPoints, "Returns a list of ptSpawnPointInfo objects"),
    PYTHON_METHOD_NOARGS(ptVaultAgeLinkNode, asAgeLinkStruct, "Returns this ptVaultAgeLinkNode as a ptAgeLinkStruct"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultAgeLinkNode, pyVaultNode, "Plasma vault age link node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultAgeLinkNode, pyVaultAgeLinkNode)

PYTHON_CLASS_CHECK_IMPL(ptVaultAgeLinkNode, pyVaultAgeLinkNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultAgeLinkNode, pyVaultAgeLinkNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultAgeLinkNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultAgeLinkNode);
    PYTHON_CLASS_IMPORT_END(m);
}
