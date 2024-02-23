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

#include <string_theory/string>

#include "pyGlueHelpers.h"
#include "pyVaultPlayerInfoNode.h"
#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultPlayerInfoNode, pyVaultPlayerInfoNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultPlayerInfoNode, pyVaultPlayerInfoNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultPlayerInfoNode)

PYTHON_INIT_DEFINITION(ptVaultPlayerInfoNode, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoNode, playerSetID, args)
{
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "l", &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "playerSetID expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Player_SetPlayerID(playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerGetID)
{
    return PyLong_FromUnsignedLong(self->fThis->Player_GetPlayerID());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoNode, playerSetName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "playerSetName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Player_SetPlayerName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerGetName)
{
    return PyUnicode_FromSTString(self->fThis->Player_GetPlayerName());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoNode, playerSetAgeInstanceName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "playerSetAgeInstanceName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Player_SetAgeInstanceName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerGetAgeInstanceName)
{
    return PyUnicode_FromSTString(self->fThis->Player_GetAgeInstanceName());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoNode, playerSetAgeGuid, args)
{
    ST::string guid;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guid))
    {
        PyErr_SetString(PyExc_TypeError, "playerSetAgeGuid expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Player_SetAgeGuid(guid);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerGetAgeGuid)
{
    return PyUnicode_FromSTString(self->fThis->Player_GetAgeGuid().AsString());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoNode, playerSetOnline, args)
{
    char stateFlag;
    if (!PyArg_ParseTuple(args, "b", &stateFlag))
    {
        PyErr_SetString(PyExc_TypeError, "playerSetOnline expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Player_SetOnline(stateFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerIsOnline)
{
    PYTHON_RETURN_BOOL(self->fThis->Player_IsOnline());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerInfoNode, playerGetCCRLevel)
{
    return PyLong_FromLong(self->fThis->Player_GetCCRLevel());
}

PYTHON_START_METHODS_TABLE(ptVaultPlayerInfoNode)
    PYTHON_METHOD(ptVaultPlayerInfoNode, playerSetID, "Params: playerID\nNot sure this should be used. Sets the playerID for this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerGetID, "Returns the player ID for this player info node."),
    PYTHON_METHOD(ptVaultPlayerInfoNode, playerSetName, "Params: name\nNot sure this should be used. Sets the player name of this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerGetName, "Returns the player name of this player info node."),
    PYTHON_METHOD(ptVaultPlayerInfoNode, playerSetAgeInstanceName, "Params: name\nNot sure this should be used. Sets the name of the age where the player is for this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerGetAgeInstanceName, "Returns the name of the Age where the player is for this player info node."),
    PYTHON_METHOD(ptVaultPlayerInfoNode, playerSetAgeGuid, "Params: guidString\nNot sure this should be used. Sets the guid for this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerGetAgeGuid, "Returns the guid as a string of where the player is for this player info node."),
    PYTHON_METHOD(ptVaultPlayerInfoNode, playerSetOnline, "Params: state\nNot sure this should be used. Sets the state of the player online status for this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerIsOnline, "Returns the online status of the player for this player info node."),
    PYTHON_METHOD_NOARGS(ptVaultPlayerInfoNode, playerGetCCRLevel, "Returns the ccr level of the player for this player info node."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultPlayerInfoNode, pyVaultNode, "Plasma vault folder node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultPlayerInfoNode, pyVaultPlayerInfoNode)

PYTHON_CLASS_CHECK_IMPL(ptVaultPlayerInfoNode, pyVaultPlayerInfoNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultPlayerInfoNode, pyVaultPlayerInfoNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultPlayerInfoNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultPlayerInfoNode);
    PYTHON_CLASS_IMPORT_END(m);
}
