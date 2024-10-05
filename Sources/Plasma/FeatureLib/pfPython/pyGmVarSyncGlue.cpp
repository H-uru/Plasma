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

#include "plPythonConvert.h"
#include "pyGameCli.h"
#include "pyGlueHelpers.h"
#include "pyGmVarSync.h"

// ===========================================================================

PYTHON_CLASS_DEFINITION(ptGmVarSync, pyGmVarSync);

PYTHON_DEFAULT_NEW_DEFINITION(ptGmVarSync, pyGmVarSync)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGmVarSync)

PYTHON_NO_INIT_DEFINITION(ptGmVarSync)

PYTHON_METHOD_DEFINITION(ptGmVarSync, setVariable, args)
{
    unsigned int varID;
    PyObject* varValueObj;
    if (!PyArg_ParseTuple(args, "IO", &varID, &varValueObj)) {
        PyErr_SetString(PyExc_TypeError, "setVariable expects an unsigned integer and a number or string");
        PYTHON_RETURN_ERROR;
    }

    if (PyUnicode_Check(varValueObj)) {
        self->fThis->SetVariable(varID, PyUnicode_AsSTString(varValueObj));
        PYTHON_RETURN_NONE;
    } else if (PyNumber_Check(varValueObj)) {
        pyObjectRef varFloatObj = PyNumber_Float(varValueObj);
        self->fThis->SetVariable(varID, PyFloat_AsDouble(varFloatObj.Get()));
        PYTHON_RETURN_NONE;
    }

    PyErr_SetString(PyExc_TypeError, "setVariable expects an unsigned integer and a number or string");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptGmVarSync, createVariable, args)
{
    ST::string varName;
    PyObject* varValueObj;
    if (!PyArg_ParseTuple(args, "O&O", &PyUnicode_STStringConverter, &varName, &varValueObj)) {
        PyErr_SetString(PyExc_TypeError, "createVariable expects a string and a number or string");
        PYTHON_RETURN_ERROR;
    }

    if (PyUnicode_Check(varValueObj)) {
        self->fThis->CreateVariable(varName, PyUnicode_AsSTString(varValueObj));
        PYTHON_RETURN_NONE;
    } else if (PyNumber_Check(varValueObj)) {
        pyObjectRef varFloatObj = PyNumber_Float(varValueObj);
        self->fThis->CreateVariable(varName, PyFloat_AsDouble(varFloatObj.Get()));
        PYTHON_RETURN_NONE;
    }

    PyErr_SetString(PyExc_TypeError, "createVariable expects a string and a number or string");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_STATIC(ptGmVarSync, join, args)
{
    PyObject* handler;
    if (!PyArg_ParseTuple(args, "O", &handler)) {
        PyErr_SetString(PyExc_TypeError, "ptGmVarSync.join() expects an object");
        PYTHON_RETURN_ERROR;
    }

    pyGmVarSync::Join(handler);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC_NOARGS(ptGmVarSync, isSupported)
{
    return plPython::ConvertFrom(self->fThis->IsSupported());
}

PYTHON_START_METHODS_TABLE(ptGmVarSync)
    PYTHON_METHOD_STATIC(ptGmVarSync, join, "Join the common var sync game in the current Age."),
    PYTHON_METHOD_STATIC_NOARGS(ptGmVarSync, isSupported, "Checks for the presence of a server-side var sync game manager."),
    PYTHON_METHOD(ptGmVarSync, setVariable, "Change the value of a variable on the server."),
    PYTHON_METHOD(ptGmVarSync, createVariable, "Create a new variable on the server."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGmVarSync, pyGameCli, "Legacy var sync game client.");

// required functions for PyObject interoperability
PYTHON_CLASS_GMCLI_NEW_IMPL(ptGmVarSync, pyGmVarSync, pfGmVarSync)
PYTHON_CLASS_CHECK_IMPL(ptGmVarSync, pyGmVarSync)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGmVarSync, pyGmVarSync)

// ===========================================================================

void pyGmVarSync::AddPlasmaGameClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGmVarSync);
    PYTHON_CLASS_IMPORT_END(m);
}
