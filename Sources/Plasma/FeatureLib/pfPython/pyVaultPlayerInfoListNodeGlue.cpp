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

#include "pyVaultPlayerInfoListNode.h"
#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultPlayerInfoListNode, pyVaultPlayerInfoListNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultPlayerInfoListNode, pyVaultPlayerInfoListNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultPlayerInfoListNode)

PYTHON_INIT_DEFINITION(ptVaultPlayerInfoListNode, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoListNode, hasPlayer, args)
{
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "l", &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "hasPlayer expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_BOOL(self->fThis->HasPlayer(playerID));
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoListNode, addPlayer, args)
{
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "l", &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "addPlayer expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddPlayer(playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoListNode, removePlayer, args)
{
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "l", &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "removePlayer expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RemovePlayer(playerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultPlayerInfoListNode, getPlayer, args)
{
    unsigned long playerID;
    if (!PyArg_ParseTuple(args, "l", &playerID))
    {
        PyErr_SetString(PyExc_TypeError, "getPlayer expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return self->fThis->GetPlayer(playerID);
}

PYTHON_BASIC_METHOD_DEFINITION(ptVaultPlayerInfoListNode, sort, Sort)

PYTHON_START_METHODS_TABLE(ptVaultPlayerInfoListNode)
    PYTHON_METHOD(ptVaultPlayerInfoListNode, hasPlayer, "Params: playerID\nReturns whether the 'playerID' is a member of this player info list node."),
    PYTHON_METHOD(ptVaultPlayerInfoListNode, addPlayer, "Params: playerID\nAdds playerID player to this player info list node."),
    PYTHON_METHOD(ptVaultPlayerInfoListNode, removePlayer, "Params: playerID\nRemoves playerID player from this player info list node."),
    PYTHON_METHOD(ptVaultPlayerInfoListNode, getPlayer, "Params: playerID\nGets the player info node for the specified player."),
    PYTHON_BASIC_METHOD(ptVaultPlayerInfoListNode, sort, "Sorts the player list by some means...?"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultPlayerInfoListNode, pyVaultFolderNode, "Plasma vault player info list node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultPlayerInfoListNode, pyVaultPlayerInfoListNode)

PYTHON_CLASS_CHECK_IMPL(ptVaultPlayerInfoListNode, pyVaultPlayerInfoListNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultPlayerInfoListNode, pyVaultPlayerInfoListNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultPlayerInfoListNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultPlayerInfoListNode);
    PYTHON_CLASS_IMPORT_END(m);
}
