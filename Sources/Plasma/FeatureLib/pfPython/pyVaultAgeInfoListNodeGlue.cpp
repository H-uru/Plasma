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

#include "pyGlueHelpers.h"

#include "pyVaultAgeInfoListNode.h"
#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultAgeInfoListNode, pyVaultAgeInfoListNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultAgeInfoListNode, pyVaultAgeInfoListNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultAgeInfoListNode)

PYTHON_INIT_DEFINITION(ptVaultAgeInfoListNode, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoListNode, hasAge, args)
{
    unsigned long ageID;
    if (!PyArg_ParseTuple(args, "l", &ageID))
    {
        PyErr_SetString(PyExc_TypeError, "hasAge expects a unsigned long");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_BOOL(self->fThis->HasAge(ageID));
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoListNode, addAge, args)
{
    unsigned long ageID;
    if (!PyArg_ParseTuple(args, "l", &ageID))
    {
        PyErr_SetString(PyExc_TypeError, "addAge expects a unsigned long");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_BOOL(self->fThis->AddAge(ageID));
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoListNode, removeAge, args)
{
    unsigned long ageID;
    if (!PyArg_ParseTuple(args, "l", &ageID))
    {
        PyErr_SetString(PyExc_TypeError, "removeAge expects a unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RemoveAge(ageID);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVaultAgeInfoListNode)
    PYTHON_METHOD(ptVaultAgeInfoListNode, hasAge, "Params: ageID\nReturns whether ageID is in the list of ages"),
    PYTHON_METHOD(ptVaultAgeInfoListNode, addAge, "Params: ageID\nAdds ageID to list of ages"),
    PYTHON_METHOD(ptVaultAgeInfoListNode, removeAge, "Params: ageID\nRemoves ageID from list of ages"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultAgeInfoListNode, pyVaultFolderNode, "Plasma vault age info list node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultAgeInfoListNode, pyVaultAgeInfoListNode);

PYTHON_CLASS_CHECK_IMPL(ptVaultAgeInfoListNode, pyVaultAgeInfoListNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultAgeInfoListNode, pyVaultAgeInfoListNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultAgeInfoListNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultAgeInfoListNode);
    PYTHON_CLASS_IMPORT_END(m);
}
