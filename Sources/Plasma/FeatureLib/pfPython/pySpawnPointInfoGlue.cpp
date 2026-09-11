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

#include "pySpawnPointInfo.h"

#include "pyGlueHelpers.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptSpawnPointInfo, pySpawnPointInfo);

PYTHON_DEFAULT_NEW_DEFINITION(ptSpawnPointInfo, pySpawnPointInfo)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSpawnPointInfo)

PYTHON_INIT_DEFINITION(ptSpawnPointInfo, args, keywords)
{
    ST::string title;
    ST::string spawnPt;
    if (!PyArg_ParseTuple(args, "|O&O&", PyUnicode_STStringConverter, &title, PyUnicode_STStringConverter, &spawnPt))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects two optional strings, or no parameters");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (title.empty() && spawnPt.empty())
    {
        // default init
        PYTHON_RETURN_INIT_OK;
    }
    else if (!title.empty() && !spawnPt.empty())
    {
        self->fThis->SetTitle(std::move(title));
        self->fThis->SetName(std::move(spawnPt));
        PYTHON_RETURN_INIT_OK;
    }
    // only one param existed
    PyErr_SetString(PyExc_TypeError, "__init__ expects two optional strings, or no parameters");
    PYTHON_RETURN_INIT_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getTitle)
{
    return PyUnicode_FromSTString(self->fThis->GetTitle());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setTitle, args)
{
    ST::string title;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &title))
    {
        PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetTitle(std::move(title));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getName)
{
    return PyUnicode_FromSTString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "setName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetName(std::move(name));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getCameraStack)
{
    return PyUnicode_FromSTString(self->fThis->GetCameraStack());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setCameraStack, args)
{
    ST::string camStack;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &camStack))
    {
        PyErr_SetString(PyExc_TypeError, "setCameraStack expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetCameraStack(std::move(camStack));
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSpawnPointInfo)
    PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getTitle, "Returns the spawnpoint's title"),
    PYTHON_METHOD(ptSpawnPointInfo, setTitle, "Params: title\nSets the spawnpoint's title"),
    PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getName, "Returns the spawnpoint's name"),
    PYTHON_METHOD(ptSpawnPointInfo, setName, "Params: name\nSets the spawnpoint's name"),
    PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getCameraStack, "Returns the camera stack for this spawnpoint as a string"),
    PYTHON_METHOD(ptSpawnPointInfo, setCameraStack, "Params: stack\nSets the spawnpoint's camera stack (as a string)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptSpawnPointInfo, "Params: title=None,spawnPt=None\nClass to hold spawn point data");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptSpawnPointInfo, pySpawnPointInfo)

PyObject* pySpawnPointInfo::New(plSpawnPointInfo info)
{
    ptSpawnPointInfo *newObj = (ptSpawnPointInfo*)ptSpawnPointInfo_type.tp_new(&ptSpawnPointInfo_type, nullptr, nullptr);
    newObj->fThis->fInfo = std::move(info);
    return (PyObject*)newObj;
}

PyObject* pySpawnPointInfo::New(ST::string title, ST::string spawnPt)
{
    ptSpawnPointInfo *newObj = (ptSpawnPointInfo*)ptSpawnPointInfo_type.tp_new(&ptSpawnPointInfo_type, nullptr, nullptr);
    newObj->fThis->fInfo.SetTitle(std::move(title));
    newObj->fThis->fInfo.SetName(std::move(spawnPt));
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSpawnPointInfo, pySpawnPointInfo)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSpawnPointInfo, pySpawnPointInfo)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySpawnPointInfo::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptSpawnPointInfo);
    PYTHON_CLASS_IMPORT_END(m);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDefaultSpawnPoint, "Returns the default spawnpoint definition (as a ptSpawnPointInfo)")
{
    return pySpawnPointInfo::GetDefaultSpawnPoint();
}

void pySpawnPointInfo::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(ptSpawnPointInfo)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDefaultSpawnPoint)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, ptSpawnPointInfo)
}

// glue functions
PYTHON_CLASS_DEFINITION(ptSpawnPointInfoRef, pySpawnPointInfoRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptSpawnPointInfoRef, pySpawnPointInfoRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSpawnPointInfoRef)

PYTHON_NO_INIT_DEFINITION(ptSpawnPointInfoRef)

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getTitle)
{
    return PyUnicode_FromSTString(self->fThis->GetTitle());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setTitle, args)
{
    ST::string title;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &title))
    {
        PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetTitle(std::move(title));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getName)
{
    return PyUnicode_FromSTString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "setName expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetName(std::move(name));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getCameraStack)
{
    return PyUnicode_FromSTString(self->fThis->GetCameraStack());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setCameraStack, args)
{
    ST::string camStack;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &camStack))
    {
        PyErr_SetString(PyExc_TypeError, "setCameraStack expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetCameraStack(std::move(camStack));
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSpawnPointInfoRef)
    PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getTitle, "Returns the spawnpoint's title"),
    PYTHON_METHOD(ptSpawnPointInfoRef, setTitle, "Params: title\nSets the spawnpoint's title"),
    PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getName, "Returns the spawnpoint's name"),
    PYTHON_METHOD(ptSpawnPointInfoRef, setName, "Params: name\nSets the spawnpoint's name"),
    PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getCameraStack, "Returns the camera stack for this spawnpoint as a string"),
    PYTHON_METHOD(ptSpawnPointInfoRef, setCameraStack, "Params: stack\nSets the spawnpoint's camera stack (as a string)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptSpawnPointInfoRef, "Class to hold spawn point data");

// required functions for PyObject interoperability
PyObject *pySpawnPointInfoRef::New(plSpawnPointInfo& info)
{
    ptSpawnPointInfoRef *newObj = (ptSpawnPointInfoRef*)ptSpawnPointInfoRef_type.tp_new(&ptSpawnPointInfoRef_type, nullptr, nullptr);
    newObj->fThis->fInfo = info;
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSpawnPointInfoRef, pySpawnPointInfoRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSpawnPointInfoRef, pySpawnPointInfoRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySpawnPointInfoRef::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptSpawnPointInfoRef);
    PYTHON_CLASS_IMPORT_END(m);
}