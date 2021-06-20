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
#include "pyKey.h"

#include "pyCluster.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptCluster, pyCluster);

PYTHON_DEFAULT_NEW_DEFINITION(ptCluster, pyCluster)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptCluster)

PYTHON_INIT_DEFINITION(ptCluster, args, keywords)
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->SetKey(*key);
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptCluster, setVisible, args)
{
    char visibleFlag;
    if (!PyArg_ParseTuple(args, "b", &visibleFlag))
    {
        PyErr_SetString(PyExc_TypeError, "setVisible expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetVisible(visibleFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptCluster)
    PYTHON_METHOD(ptCluster, setVisible, "Params:visible\nShows or hides the cluster object"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptCluster, "Params:key\nCreates a new ptCluster");

// required functions for PyObject interoperability
PyObject *pyCluster::New(plKey key)
{
    ptCluster *newObj = (ptCluster*)ptCluster_type.tp_new(&ptCluster_type, nullptr, nullptr);
    newObj->fThis->fClusterKey = std::move(key);
    return (PyObject*)newObj;
}

PyObject *pyCluster::New(pyKey& key)
{
    ptCluster *newObj = (ptCluster*)ptCluster_type.tp_new(&ptCluster_type, nullptr, nullptr);
    newObj->fThis->fClusterKey = key.getKey();
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptCluster, pyCluster)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptCluster, pyCluster)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyCluster::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptCluster);
    PYTHON_CLASS_IMPORT_END(m);
}