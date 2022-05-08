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
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"
#include <string_theory/string>
#include <string_theory/format>

#include "cyPythonInterface.h"
#include "plPythonPack.h"

#include "plStatusLog/plStatusLog.h"

// ==========================================================================

struct pyModulePackFinder
{
    pyObjectRef fLoader;

    PYTHON_CLASS_NEW_DEFINITION;
};

PYTHON_CLASS_DEFINITION(ptModulePackFinder, pyModulePackFinder)
PYTHON_DEFAULT_NEW_DEFINITION(ptModulePackFinder, pyModulePackFinder)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptModulePackFinder)
PYTHON_NO_INIT_DEFINITION(ptModulePackFinder)

struct pyModulePackSpec
{
    ST::string fName;
    ST::string fParent;
    pyObjectRef fPycCode;
    pyObjectRef fLoader;
    pyObjectRef fTargetModule;
    bool fInitializing{ false };

    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION;
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyModulePackSpec);
};

PYTHON_CLASS_DEFINITION(ptModulePackSpec, pyModulePackSpec)
PYTHON_DEFAULT_NEW_DEFINITION(ptModulePackSpec, pyModulePackSpec)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptModulePackSpec)
PYTHON_NO_INIT_DEFINITION(ptModulePackSpec)

struct pyModulePackLoader
{
    PYTHON_CLASS_NEW_DEFINITION;
};

PYTHON_CLASS_DEFINITION(ptModulePackLoader, pyModulePackLoader)
PYTHON_DEFAULT_NEW_DEFINITION(ptModulePackLoader, pyModulePackLoader)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptModulePackLoader)
PYTHON_NO_INIT_DEFINITION(ptModulePackLoader)

// ==========================================================================

PYTHON_METHOD_DEFINITION(ptModulePackFinder, find_spec, args)
{
    ST::string name;
    PyObject* path = nullptr; // always Py_None IME
    PyObject* target = nullptr;
    if (!PyArg_ParseTuple(args, "O&|OO", PyUnicode_STStringConverter, &name, &path, &target)) {
        PyErr_SetString(PyExc_TypeError, "find_spec expects a string, and optional object, and an optional module object");
        return nullptr;
    }
    if (!(target == nullptr || target == Py_None || PyModule_Check(target))) {
        PyErr_SetString(PyExc_TypeError, "find_spec expects a string, and optional object, and an optional module object");
        return nullptr;
    }

    ptModulePackSpec* spec = nullptr;
    if (PythonPack::IsItPythonPacked(name)) {
        spec = (ptModulePackSpec*)pyModulePackSpec::New();
        spec->fThis->fParent = name.before_last('.');
        spec->fThis->fPycCode = PythonPack::OpenPythonPacked(name);
    } else {
        ST::string package = ST::format("{}.__init__", name);
        if (PythonPack::IsItPythonPacked(package)) {
            spec = (ptModulePackSpec*)pyModulePackSpec::New();
            spec->fThis->fParent = name;
            spec->fThis->fPycCode = PythonPack::OpenPythonPacked(package);
        }
    }

    if (spec) {
        spec->fThis->fName = name;
        spec->fThis->fLoader = self->fThis->fLoader;
        // yes, this happens...
        if (target != Py_None)
            spec->fThis->fTargetModule = target;
        return (PyObject*)spec;
    } else {
        // Not an error condition, technically.
        PYTHON_RETURN_NONE;
    }
}

PYTHON_START_METHODS_TABLE(ptModulePackFinder)
    PYTHON_METHOD(ptModulePackFinder, find_spec, ""),
PYTHON_END_METHODS_TABLE;

PLASMA_DEFAULT_TYPE(ptModulePackFinder, "PEP 451 module finder for plPythonPack");
PYTHON_CLASS_NEW_IMPL(ptModulePackFinder, pyModulePackFinder);

// ==========================================================================

PYTHON_GET_DEFINITION(ptModulePackSpec, name)
{
    return PyUnicode_FromSTString(self->fThis->fName);
}

PYTHON_GET_DEFINITION(ptModulePackSpec, loader)
{
    PyObject* loader = self->fThis->fLoader.Get();
    Py_INCREF(loader);
    return loader;
}

PYTHON_GET_DEFINITION(ptModulePackSpec, parent)
{
    return PyUnicode_FromSTString(self->fThis->fParent);
}

PYTHON_GET_DEFINITION(ptModulePackSpec, origin)
{
    PYTHON_RETURN_NONE;
}

PYTHON_GET_DEFINITION(ptModulePackSpec, cached)
{
    // Don't look at me.
    PYTHON_RETURN_NONE;
}

PYTHON_GET_DEFINITION(ptModulePackSpec, submodule_search_locations)
{
    // Actual system paths to search for submodules on. If we really have a module in python.pak,
    // then we don't want to be traipsing around the system for stray files. That is both a security
    // hole and a bug that prevents package modules from working.
    return PyList_New(0);
}

PYTHON_GET_DEFINITION(ptModulePackSpec, loader_state)
{
    PYTHON_RETURN_NONE;
}

PYTHON_GET_DEFINITION(ptModulePackSpec, has_location)
{
    PYTHON_RETURN_BOOL(false);
}

PYTHON_GET_DEFINITION(ptModulePackSpec, _initializing)
{
    PYTHON_RETURN_BOOL(self->fThis->fInitializing);
}

PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, name)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, loader)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, parent)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, origin)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, cached)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, submodule_search_locations)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, loader_state)
PYTHON_SET_DEFINITION_READONLY(ptModulePackSpec, has_location)
PYTHON_SET_DEFINITION(ptModulePackSpec, _initializing, value)
{
    if (PyBool_Check(value)) {
        self->fThis->fInitializing = PyLong_AsLong(value) != 0;
        PYTHON_RETURN_SET_OK;
    }

    PyErr_SetString(PyExc_TypeError, "_initializing should be a bool");
    PYTHON_RETURN_SET_ERROR;
}

PYTHON_START_GETSET_TABLE(ptModulePackSpec)
    PYTHON_GETSET(ptModulePackSpec, name, ""),
    PYTHON_GETSET(ptModulePackSpec, loader, ""),
    PYTHON_GETSET(ptModulePackSpec, parent, ""),
    PYTHON_GETSET(ptModulePackSpec, origin, ""),
    PYTHON_GETSET(ptModulePackSpec, cached, ""),
    PYTHON_GETSET(ptModulePackSpec, submodule_search_locations, ""),
    PYTHON_GETSET(ptModulePackSpec, loader_state, ""),
    PYTHON_GETSET(ptModulePackSpec, has_location, ""),
    PYTHON_GETSET(ptModulePackSpec, _initializing, ""),
PYTHON_END_GETSET_TABLE;

PYTHON_START_METHODS_TABLE(ptModulePackSpec)
    // no methods
PYTHON_END_METHODS_TABLE;

#define ptModulePackSpec_AS_ASYNC       PYTHON_NO_AS_ASYNC
#define ptModulePackSpec_AS_NUMBER      PYTHON_NO_AS_NUMBER
#define ptModulePackSpec_AS_SEQUENCE    PYTHON_NO_AS_SEQUENCE
#define ptModulePackSpec_AS_MAPPING     PYTHON_NO_AS_MAPPING
#define ptModulePackSpec_STR            PYTHON_NO_STR
#define ptModulePackSpec_GETATTRO       PYTHON_DEFAULT_GETATTRO
#define ptModulePackSpec_SETATTRO       PYTHON_DEFAULT_SETATTRO
#define ptModulePackSpec_RICH_COMPARE   PYTHON_NO_RICH_COMPARE
#define ptModulePackSpec_ITER           PYTHON_NO_ITER
#define ptModulePackSpec_ITERNEXT       PYTHON_NO_ITERNEXT
#define ptModulePackSpec_GETSET         PYTHON_DEFAULT_GETSET(ptModulePackSpec)
#define ptModulePackSpec_BASE           PYTHON_NO_BASE

PLASMA_CUSTOM_TYPE(ptModulePackSpec, "PEP 451 module spec for plPythonPack");
PYTHON_CLASS_NEW_IMPL(ptModulePackSpec, pyModulePackSpec);
PYTHON_CLASS_CHECK_IMPL(ptModulePackSpec, pyModulePackSpec);
PYTHON_CLASS_CONVERT_FROM_IMPL(ptModulePackSpec, pyModulePackSpec);

// ==========================================================================

PYTHON_METHOD_DEFINITION(ptModulePackLoader, create_module, args)
{
#ifdef HS_DEBUGGING
    PyObject* specObj;
    if (!PyArg_ParseTuple(args, "O", &specObj) || !pyModulePackSpec::Check(specObj)) {
        PyErr_SetString(PyExc_TypeError, "create_module expects a ptModulePackSpec object");
        return nullptr;
    }
#endif

    // Creating the module here causes very bad things to happen for some reason. PEP 451 indicates
    // that this method is optional, however, it was made required in Python 3.6. Returning None
    // requests Python to create the module for us.
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptModulePackLoader, exec_module, args)
{
    PyObject* pymodule;
    if (!PyArg_ParseTuple(args, "O", &pymodule) || !PyModule_Check(pymodule)) {
        PyErr_SetString(PyExc_TypeError, "exec_module expects a module object");
        return nullptr;
    }

    pyObjectRef moduleSpecObj = PyObject_GetAttrString(pymodule, "__spec__");
    if (!moduleSpecObj) {
        PyErr_SetString(PyExc_ImportError, "module spec missing");
        return nullptr;
    }
    hsAssert(pyModulePackSpec::Check(moduleSpecObj.Get()), "module spec of unexpected type");
    pyModulePackSpec* spec = pyModulePackSpec::ConvertFrom(moduleSpecObj.Get());

    PyObject* dict = PyModule_GetDict(pymodule);
    if (!PyDict_GetItemString(dict, "__builtins__"))
        PyDict_SetItemString(dict, "__builtins__", PyEval_GetBuiltins());

    if (spec->fPycCode) {
        pyObjectRef result = PyEval_EvalCode(spec->fPycCode.Get(), dict, dict);
        if (!result)
            return nullptr;
    }
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptModulePackLoader)
    PYTHON_METHOD(ptModulePackLoader, create_module, ""),
    PYTHON_METHOD(ptModulePackLoader, exec_module, ""),
PYTHON_END_METHODS_TABLE;

PLASMA_DEFAULT_TYPE(ptModulePackLoader, "PEP 451 module loader for plPythonPack");
PYTHON_CLASS_NEW_IMPL(ptModulePackLoader, pyModulePackLoader);

// ==========================================================================

void PythonInterface::initPyPackHook()
{
#ifdef HS_DEBUGGING
    dbgLog->AddLine(plStatusLog::kGreen, "Creating PEP 451 module...");
#endif

    // note: steals ref
    PyObject* mod = PyImport_AddModule("_PlasmaImport");
    if (!mod) {
        dbgLog->AddLine(plStatusLog::kRed, "Failed to create _PlasmaImport module!");
        return;
    }

    PYTHON_CLASS_IMPORT_START(mod);
    PYTHON_CLASS_IMPORT(mod, ptModulePackFinder);
    PYTHON_CLASS_IMPORT(mod, ptModulePackSpec);
    PYTHON_CLASS_IMPORT(mod, ptModulePackLoader);
    PYTHON_CLASS_IMPORT_END(mod);

#ifdef HS_DEBUGGING
    dbgLog->AddLine(plStatusLog::kGreen, "Installing PEP 451 machinery...");
#endif

    pyObjectRef metaFinder = pyModulePackFinder::New();
    ((ptModulePackFinder*)metaFinder.Get())->fThis->fLoader = pyModulePackLoader::New();
    PyObject* finders = PySys_GetObject("meta_path");
    hsAssert(finders, "Failed to access sys.meta_path");
    hsAssert(PyList_Check(finders), "sys.meta_path finders is not a list");
    PyList_Append(finders, metaFinder.Get());
}
