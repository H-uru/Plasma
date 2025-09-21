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

#include "pyVaultPlayerNode.h"

#include <string_theory/string>

#include "plVault/plVault.h"

#include "pyAgeInfoStruct.h"
#include "pyGlueHelpers.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultPlayerNode, pyVaultPlayerNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultPlayerNode, pyVaultPlayerNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultPlayerNode)

PYTHON_INIT_DEFINITION(ptVaultPlayerNode, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setPlayerName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "setPlayerName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetPlayerName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getPlayerName)
{
    return PyUnicode_FromSTString(self->fThis->GetPlayerName());
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerNode, setAvatarShapeName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "setAvatarShapeName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetAvatarShapeName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultPlayerNode, getAvatarShapeName)
{
    return PyUnicode_FromSTString(self->fThis->GetAvatarShapeName());
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


PYTHON_START_METHODS_TABLE(ptVaultPlayerNode)
    PYTHON_METHOD(ptVaultPlayerNode, setPlayerName, "Params: name\nSets the player's name"),
    PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getPlayerName, "Returns the player's name"),
    PYTHON_METHOD(ptVaultPlayerNode, setAvatarShapeName, "Params: name\nSets the avatar's 'shape'"),
    PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getAvatarShapeName, "Returns the avatar's 'shape'"),
    PYTHON_METHOD(ptVaultPlayerNode, setDisabled, "Params: state\nDisables/enables the avatar"),
    PYTHON_METHOD(ptVaultPlayerNode, isDisabled, "Is the avatar disabled?"),
    PYTHON_METHOD(ptVaultPlayerNode, setOnlineTime, "Params: time\nSets the avatar's online time"),
    PYTHON_METHOD_NOARGS(ptVaultPlayerNode, getOnlineTime, "Returns the avatar's online time"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultPlayerNode, pyVaultNode, "Plasma vault player node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultPlayerNode, pyVaultPlayerNode)

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
