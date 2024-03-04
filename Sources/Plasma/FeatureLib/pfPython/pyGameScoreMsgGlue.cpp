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

#include "pyGameScoreMsg.h"

#include <string_theory/string>

#include "pyGlueHelpers.h"

// Maybe we need a better exception? Seems to be the best built in one though
#define PFGS_PYERR PyExc_RuntimeError

// =================================================================

PYTHON_CLASS_DEFINITION(ptGameScoreMsg, pyGameScoreMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScoreMsg, pyGameScoreMsg);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScoreMsg);

PYTHON_NO_INIT_DEFINITION(ptGameScoreMsg);

PYTHON_START_METHODS_TABLE(ptGameScoreMsg)
    // We have no methods, but our helpers want a table...
    // Eh. Not the end of the world.
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameScoreMsg, "Game Score operation callback message");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGameScoreMsg, pyGameScoreMsg);

PYTHON_CLASS_CHECK_IMPL(ptGameScoreMsg, pyGameScoreMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScoreMsg, pyGameScoreMsg)

// Module and method definitions
void pyGameScoreMsg::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameScoreMsg);
    PYTHON_CLASS_IMPORT_END(m);
}

// =================================================================

PYTHON_CLASS_DEFINITION(ptGameScoreListMsg, pyGameScoreListMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScoreListMsg, pyGameScoreListMsg);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScoreListMsg);

PYTHON_NO_INIT_DEFINITION(ptGameScoreListMsg);

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreListMsg, getName)
{
    return PyUnicode_FromSTString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreListMsg, getOwnerID)
{
    return PyLong_FromLong(self->fThis->GetOwnerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreListMsg, getScores)
{
    if (self->fThis->IsValid())
    {
        size_t count = self->fThis->GetNumScores();
        PyObject* tup = PyTuple_New(count);
        for (size_t i = 0; i < count; ++i)
            PyTuple_SetItem(tup, i, self->fThis->GetScore(i));
        return tup;
    }

    PyErr_SetString(PFGS_PYERR, self->fThis->GetError().c_str());
    PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptGameScoreListMsg)
    PYTHON_METHOD_NOARGS(ptGameScoreListMsg, getName, "Returns the template score name"),
    PYTHON_METHOD_NOARGS(ptGameScoreListMsg, getOwnerID, "Returns the template score ownerID"),
    PYTHON_METHOD_NOARGS(ptGameScoreListMsg, getScores, "Returns a list of scores found by the server"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameScoreListMsg, pyGameScoreMsg, "Game Score message for scores found on the server");

// PyObject interop
PyObject* pyGameScoreListMsg::New(pfGameScoreListMsg* msg)
{
    ptGameScoreListMsg* newObj = (ptGameScoreListMsg*)ptGameScoreListMsg_type.tp_new(&ptGameScoreListMsg_type, nullptr, nullptr);
    hsRefCnt_SafeUnRef(newObj->fThis->fMsg);
    newObj->fThis->fMsg = msg;
    hsRefCnt_SafeRef(newObj->fThis->fMsg);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameScoreListMsg, pyGameScoreListMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScoreListMsg, pyGameScoreListMsg)

// Module and method definitions
void pyGameScoreListMsg::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameScoreListMsg);
    PYTHON_CLASS_IMPORT_END(m);
}

// =================================================================

PYTHON_CLASS_DEFINITION(ptGameScoreTransferMsg, pyGameScoreTransferMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScoreTransferMsg, pyGameScoreTransferMsg);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScoreTransferMsg);

PYTHON_NO_INIT_DEFINITION(ptGameScoreTransferMsg);

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreTransferMsg, getDestination)
{
    if (self->fThis->IsValid())
    {
        return self->fThis->GetDestinationScore();
    }

    PyErr_SetString(PFGS_PYERR, self->fThis->GetError().c_str());
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreTransferMsg, getSource)
{
    if (self->fThis->IsValid())
    {
        return self->fThis->GetSourceScore();
    }

    PyErr_SetString(PFGS_PYERR, self->fThis->GetError().c_str());
    PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptGameScoreTransferMsg)
    PYTHON_METHOD_NOARGS(ptGameScoreTransferMsg, getDestination, "Returns the score points were transferred to"),
    PYTHON_METHOD_NOARGS(ptGameScoreTransferMsg, getSource, "Returns the score points were transferred from"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameScoreTransferMsg, pyGameScoreMsg, "Game Score message indicating a score point transfer");

// PyObject interop
PyObject* pyGameScoreTransferMsg::New(pfGameScoreTransferMsg* msg)
{
    ptGameScoreTransferMsg* newObj = (ptGameScoreTransferMsg*)ptGameScoreTransferMsg_type.tp_new(&ptGameScoreTransferMsg_type, nullptr, nullptr);
    hsRefCnt_SafeUnRef(newObj->fThis->fMsg);
    newObj->fThis->fMsg = msg;
    hsRefCnt_SafeRef(newObj->fThis->fMsg);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameScoreTransferMsg, pyGameScoreTransferMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScoreTransferMsg, pyGameScoreTransferMsg)

// Module and method definitions
void pyGameScoreTransferMsg::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameScoreTransferMsg);
    PYTHON_CLASS_IMPORT_END(m);
}

// =================================================================

PYTHON_CLASS_DEFINITION(ptGameScoreUpdateMsg, pyGameScoreUpdateMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScoreUpdateMsg, pyGameScoreUpdateMsg);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScoreUpdateMsg);

PYTHON_NO_INIT_DEFINITION(ptGameScoreUpdateMsg);

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScoreUpdateMsg, getScore)
{
    if (self->fThis->IsValid())
    {
        return self->fThis->GetScore();
    }

    PyErr_SetString(PFGS_PYERR, self->fThis->GetError().c_str());
    PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptGameScoreUpdateMsg)
    PYTHON_METHOD_NOARGS(ptGameScoreUpdateMsg, getScore, "Returns the updated game score"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameScoreUpdateMsg, pyGameScoreMsg, "Game Score message for a score update operation");

// PyObject interop
PyObject* pyGameScoreUpdateMsg::New(pfGameScoreUpdateMsg* msg)
{
    ptGameScoreUpdateMsg* newObj = (ptGameScoreUpdateMsg*)ptGameScoreUpdateMsg_type.tp_new(&ptGameScoreUpdateMsg_type, nullptr, nullptr);
    hsRefCnt_SafeUnRef(newObj->fThis->fMsg);
    newObj->fThis->fMsg = msg;
    hsRefCnt_SafeRef(newObj->fThis->fMsg);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameScoreUpdateMsg, pyGameScoreUpdateMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScoreUpdateMsg, pyGameScoreUpdateMsg)

// Module and method definitions
void pyGameScoreUpdateMsg::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameScoreUpdateMsg);
    PYTHON_CLASS_IMPORT_END(m);
}

// =================================================================

#undef PFGS_PYERR
