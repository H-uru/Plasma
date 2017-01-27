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
#include "pyGeometry3.h"
#pragma hdrstop

#include "pyVaultMarkerGameNode.h"
#include "plVault/plVault.h"
#include "pnUUID/pnUUID.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultMarkerGameNode, pyVaultMarkerGameNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultMarkerGameNode, pyVaultMarkerGameNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultMarkerGameNode)

PYTHON_INIT_DEFINITION(ptVaultMarkerGameNode, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultMarkerGameNode, getGameName)
{
    return PyUnicode_FromSTString(self->fThis->GetGameName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultMarkerGameNode, getMarkers)
{
    return self->fThis->GetMarkers();
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, setGameName, args)
{
    PyObject* name;
    if (!PyArg_ParseTuple(args, "O", &name) || !PyString_CheckEx(name))
    {
        PyErr_SetString(PyExc_TypeError, "setGameName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetGameName(PyString_AsStringEx(name));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, getReward)
{
    return PyUnicode_FromSTString(self->fThis->GetReward());
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, setReward, args)
{
    PyObject* reward;
    if (!PyArg_ParseTuple(args, "O", &reward) || !PyString_CheckEx(reward)) {
        PyErr_SetString(PyExc_TypeError, "setReward expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetReward(PyString_AsStringEx(reward));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, setMarkers, args)
{
    PyObject* main_seq;
    const char* errmsg = "setMarkers expects a sequence of markers (tuple of int, string, ptPoint3, string)";
    if (!PyArg_ParseTuple(args, "O", &main_seq)) {
        PyErr_SetString(PyExc_TypeError, errmsg);
        PYTHON_RETURN_ERROR;
    }
    if (!PySequence_Check(main_seq)) {
        PyErr_SetString(PyExc_TypeError, errmsg);
        PYTHON_RETURN_ERROR;
    }

    std::vector<VaultMarker> collector;
    collector.reserve(PySequence_Size(main_seq));
    for (Py_ssize_t i = 0; i < PySequence_Size(main_seq); ++i) {
        PyObject* marker_seq = PySequence_GetItem(main_seq, i);
        if (!PySequence_Check(marker_seq) || PySequence_Size(marker_seq) != 4) {
            PyErr_SetString(PyExc_TypeError, errmsg);
            PYTHON_RETURN_ERROR;
        }

        PyObject* id   = PySequence_GetItem(marker_seq, 0);
        PyObject* age  = PySequence_GetItem(marker_seq, 1);
        PyObject* pos  = PySequence_GetItem(marker_seq, 2);
        PyObject* desc = PySequence_GetItem(marker_seq, 3);
        if (!(PyInt_Check(id) && PyString_CheckEx(age) && pyPoint3::Check(pos) && PyString_CheckEx(desc))) {
            PyErr_SetString(PyExc_TypeError, errmsg);
            PYTHON_RETURN_ERROR;
        }

        VaultMarker marker;
        marker.id = PyLong_AsUnsignedLong(id);
        marker.age = PyString_AsStringEx(age);
        marker.pos = pyPoint3::ConvertFrom(pos)->fPoint;
        marker.description = PyString_AsStringEx(desc);
        collector.push_back(marker);
    }

    self->fThis->SetMarkers(collector);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVaultMarkerGameNode)
    PYTHON_METHOD_NOARGS(ptVaultMarkerGameNode, getGameName, "Returns the marker game's name"),
    PYTHON_METHOD_NOARGS(ptVaultMarkerGameNode, getMarkers, "Returns a tuple of markers associated with this game"),
    PYTHON_METHOD_NOARGS(ptVaultMarkerGameNode, getReward, "Returns a string representing the reward for completing this game"),
    PYTHON_METHOD(ptVaultMarkerGameNode, setGameName, "Params: name\nSets marker game's name"),
    PYTHON_METHOD(ptVaultMarkerGameNode, setMarkers, "Params: markers\nSets markers associated with this game"),
    PYTHON_METHOD(ptVaultMarkerGameNode, setReward, "Params: reward\nSets the reward for completing this marker game"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultMarkerGameNode, pyVaultNode, "Params: n=0\nPlasma vault age info node");

// required functions for PyObject interoperability
PyObject *pyVaultMarkerGameNode::New(RelVaultNode* nfsNode)
{
    ptVaultMarkerGameNode *newObj = (ptVaultMarkerGameNode*)ptVaultMarkerGameNode_type.tp_new(&ptVaultMarkerGameNode_type, NULL, NULL);
    newObj->fThis->fNode = nfsNode;
    return (PyObject*)newObj;
}

PyObject *pyVaultMarkerGameNode::New(int n /* =0 */)
{
    ptVaultMarkerGameNode *newObj = (ptVaultMarkerGameNode*)ptVaultMarkerGameNode_type.tp_new(&ptVaultMarkerGameNode_type, NULL, NULL);
    // oddly enough, nothing to do here
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultMarkerGameNode, pyVaultMarkerGameNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultMarkerGameNode, pyVaultMarkerGameNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultMarkerGameNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultMarkerGameNode);
    PYTHON_CLASS_IMPORT_END(m);
}
