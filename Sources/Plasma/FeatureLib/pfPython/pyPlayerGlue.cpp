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
#include "pyKey.h"

#include "pyPlayer.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptPlayer, pyPlayer);

PYTHON_DEFAULT_NEW_DEFINITION(ptPlayer, pyPlayer)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptPlayer)

PYTHON_INIT_DEFINITION(ptPlayer, args, keywords)
{
    // we have two sets of arguments we can use, hence the generic PyObject* pointers
    // argument set 1: pyKey, string, uint32_t, float
    // argument set 2: string, uint32_t
    PyObject* firstObj = nullptr; // can be a pyKey or a string
    PyObject* secondObj = nullptr; // can be a string or a uint32_t
    PyObject* thirdObj = nullptr; // uint32_t
    PyObject* fourthObj = nullptr; // float
    if (!PyArg_ParseTuple(args, "OO|OO", &firstObj, &secondObj, &thirdObj, &fourthObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
        PYTHON_RETURN_INIT_ERROR;
    }

    plKey key;
    ST::string name;
    Py_ssize_t pid = -1;
    float distSeq = -1;

    if (pyKey::Check(firstObj))
    {
        if (!(PyUnicode_Check(secondObj) && PyNumber_Check(thirdObj) && PyFloat_Check(fourthObj)))
        {
            PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
            PYTHON_RETURN_INIT_ERROR;
        }

        key = pyKey::ConvertFrom(firstObj)->getKey();
        name = PyUnicode_AsSTString(secondObj);
        pid = PyNumber_AsSsize_t(thirdObj, nullptr);
        distSeq = (float)PyFloat_AsDouble(fourthObj);
    } else if (PyUnicode_Check(firstObj)) {
        name = PyUnicode_AsSTString(firstObj);
        if (!PyNumber_Check(secondObj) || thirdObj  || fourthObj)
        {
            PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
            PYTHON_RETURN_INIT_ERROR;
        }

        pid = PyNumber_AsSsize_t(secondObj, nullptr);
    } else {
        PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
        PYTHON_RETURN_INIT_ERROR;
    }

    self->fThis->Init(key, name.c_str(), (uint32_t)pid, distSeq);
    PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptPlayer, obj1, obj2, compareType)
{
    if ((obj1 == Py_None) || (obj2 == Py_None) || !pyPlayer::Check(obj1) || !pyPlayer::Check(obj2))
    {
        // if they aren't the same type, they don't match, obviously (we also never equal none)
        if (compareType == Py_EQ)
            PYTHON_RCOMPARE_FALSE;
        else if (compareType == Py_NE)
            PYTHON_RCOMPARE_TRUE;
        else
        {
            PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptPlayer object");
            PYTHON_RCOMPARE_ERROR;
        }
    }
    pyPlayer *player1 = pyPlayer::ConvertFrom(obj1);
    pyPlayer *player2 = pyPlayer::ConvertFrom(obj2);
    if (compareType == Py_EQ)
    {
        if ((*player1) == (*player2))
            PYTHON_RCOMPARE_TRUE;
        PYTHON_RCOMPARE_FALSE;
    }
    else if (compareType == Py_NE)
    {
        if ((*player1) != (*player2))
            PYTHON_RCOMPARE_TRUE;
        PYTHON_RCOMPARE_FALSE;
    }
    PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptPlayer object");
    PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getPlayerName)
{
    return PyUnicode_FromSTString(self->fThis->GetPlayerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getPlayerID)
{
    return PyLong_FromUnsignedLong(self->fThis->GetPlayerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getDistanceSq)
{
    return PyFloat_FromDouble(self->fThis->GetDistSq());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, isCCR)
{
    PYTHON_RETURN_BOOL(self->fThis->IsCCR());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, isServer)
{
    PYTHON_RETURN_BOOL(self->fThis->IsServer());
}

PYTHON_START_METHODS_TABLE(ptPlayer)
    PYTHON_METHOD_NOARGS(ptPlayer, getPlayerName, "Returns the name of the player"),
    PYTHON_METHOD_NOARGS(ptPlayer, getPlayerID, "Returns the unique player ID"),
    PYTHON_METHOD_NOARGS(ptPlayer, getDistanceSq, "Returns the distance to remote player from local player"),
    PYTHON_METHOD_NOARGS(ptPlayer, isCCR, "Is this player a CCR?"),
    PYTHON_METHOD_NOARGS(ptPlayer, isServer, "Is this player a server?"),
PYTHON_END_METHODS_TABLE;

// type structure definition
#define ptPlayer_AS_NUMBER      PYTHON_NO_AS_NUMBER
#define ptPlayer_AS_SEQUENCE    PYTHON_NO_AS_SEQUENCE
#define ptPlayer_AS_MAPPING     PYTHON_NO_AS_MAPPING
#define ptPlayer_STR            PYTHON_NO_STR
#define ptPlayer_GETATTRO       PYTHON_NO_GETATTRO
#define ptPlayer_SETATTRO       PYTHON_NO_SETATTRO
#define ptPlayer_RICH_COMPARE   PYTHON_DEFAULT_RICH_COMPARE(ptPlayer)
#define ptPlayer_GETSET         PYTHON_NO_GETSET
#define ptPlayer_BASE           PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptPlayer, "Params: avkey,name,playerID,distanceSq\nAnd optionally __init__(name,playerID)");

// required functions for PyObject interoperability
PyObject *pyPlayer::New(pyKey& avKey, const ST::string& pname, uint32_t pid, float distsq)
{
    ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, nullptr, nullptr);
    newObj->fThis->Init(avKey.getKey(), pname, pid, distsq);
    return (PyObject*)newObj;
}

PyObject *pyPlayer::New(plKey avKey, const ST::string& pname, uint32_t pid, float distsq)
{
    ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, nullptr, nullptr);
    newObj->fThis->Init(std::move(avKey), pname, pid, distsq);
    return (PyObject*)newObj;
}

PyObject *pyPlayer::New(const ST::string& pname, uint32_t pid)
{
    ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, nullptr, nullptr);
    newObj->fThis->Init(nullptr, pname, pid, -1);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptPlayer, pyPlayer)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptPlayer, pyPlayer)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyPlayer::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptPlayer);
    PYTHON_CLASS_IMPORT_END(m);
}
