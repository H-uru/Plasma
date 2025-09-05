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

#include <string_theory/string>
#include <string_theory/format>

#include "plStatusLog/plStatusLog.h"

#include "cyPythonInterface.h"
#include "plPythonCallable.h"
#include "plPythonPack.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

// ==========================================================================

static PyTypeObject* s_ModuleSpec = nullptr;

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

struct pyModulePackLoader
{
    PYTHON_CLASS_NEW_DEFINITION;
};

PYTHON_CLASS_DEFINITION(ptModulePackLoader, pyModulePackLoader)
PYTHON_DEFAULT_NEW_DEFINITION(ptModulePackLoader, pyModulePackLoader)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptModulePackLoader)
PYTHON_NO_INIT_DEFINITION(ptModulePackLoader)

// ==========================================================================

static pyObjectRef IInitModuleSpec(
    const ST::string& modName,
    const ST::string& packedName,
    pyObjectRef loader,
    bool isPackage
)
{
    // Try to share the same python string object for the module name and
    // the packed name, since they are often the same.
    pyObjectRef modNameObj, packedNameObj;
    if (modName.c_str() == packedName.c_str()) {
        modNameObj = PyUnicode_FromSTString(modName);
        packedNameObj = modNameObj;
    } else {
        modNameObj = PyUnicode_FromSTString(modName);
        packedNameObj = PyUnicode_FromSTString(packedName);
    }

    pyObjectRef spec = plPython::CallObject(
        (PyObject*)s_ModuleSpec,
        std::move(modNameObj), std::move(loader),
        plPython::KwArg("origin", std::move(packedNameObj)),
        plPython::KwArg("is_package", isPackage)
    );
    hsAssert(spec, "Could not create ModuleSpec object");
    return spec;
}

PYTHON_METHOD_DEFINITION(ptModulePackFinder, find_spec, args)
{
    ST::string name;
    PyObject* path = nullptr; // always Py_None IME
    PyObject* target = nullptr;
    if (!PyArg_ParseTuple(args, "O&|OO", PyUnicode_STStringConverter, &name, &path, &target)) {
        PyErr_SetString(PyExc_TypeError, "find_spec expects a string, and optional object, and an optional module object");
        PYTHON_RETURN_ERROR;
    }
    if (!(target == nullptr || target == Py_None || PyModule_Check(target))) {
        PyErr_SetString(PyExc_TypeError, "find_spec expects a string, and optional object, and an optional module object");
        PYTHON_RETURN_ERROR;
    }

    pyObjectRef spec;
    if (PythonPack::IsItPythonPacked(name)) {
        spec = IInitModuleSpec(name, name, self->fThis->fLoader, false);
    } else {
        ST::string package = ST::format("{}.__init__", name);
        if (PythonPack::IsItPythonPacked(package))
            spec = IInitModuleSpec(name, package, self->fThis->fLoader, true);
        else
            spec.SetPyNone();
    }

    return spec.Release();
}

PYTHON_START_METHODS_TABLE(ptModulePackFinder)
    PYTHON_METHOD(ptModulePackFinder, find_spec, ""),
PYTHON_END_METHODS_TABLE;

PLASMA_DEFAULT_TYPE(ptModulePackFinder, "PEP 451 module finder for plPythonPack");
PYTHON_CLASS_NEW_IMPL(ptModulePackFinder, pyModulePackFinder);

// ==========================================================================

PYTHON_METHOD_DEFINITION(ptModulePackLoader, create_module, args)
{
    PyObject* specObj;
    if (!PyArg_ParseTuple(args, "O!", s_ModuleSpec, &specObj)) {
        // PyArg_ParseTuple should already set an appropriate error message
        // due to the usage of the O! code.
        PYTHON_RETURN_ERROR;
    }

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
        PYTHON_RETURN_ERROR;
    }

    pyObjectRef moduleSpecObj = PyObject_GetAttrString(pymodule, "__spec__");
    if (!moduleSpecObj) {
        PyErr_SetString(PyExc_ImportError, "module spec missing");
        PYTHON_RETURN_ERROR;
    }

    pyObjectRef packedNameObj = PyObject_GetAttrString(moduleSpecObj.Get(), "origin");
    if (!(packedNameObj && PyUnicode_Check(packedNameObj.Get()))) {
        PyErr_Format(
            PyExc_ImportError,
            "origin missing or invalid in ModuleSpec for module %s",
            PyModule_GetName(pymodule)
        );
        PYTHON_RETURN_ERROR;
    }

    ST::string packedName = PyUnicode_AsSTString(packedNameObj.Get());
    pyObjectRef pycCode = PythonPack::OpenPythonPacked(packedName);
    if (!(pycCode && PyCode_Check(pycCode.Get()))) {
        PyErr_Format(
            PyExc_ImportError,
            "Could not load packed code for module %s",
            PyModule_GetName(pymodule)
        );
        PYTHON_RETURN_ERROR;
    }

    PyObject* dict = PyModule_GetDict(pymodule);
    if (!PyDict_GetItemString(dict, "__builtins__"))
        PyDict_SetItemString(dict, "__builtins__", PyEval_GetBuiltins());

    pyObjectRef evalResult = PyEval_EvalCode(pycCode.Get(), dict, dict);
    if (!evalResult)
        PYTHON_RETURN_ERROR;

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
    dbgLog->AddLine(plStatusLog::kGreen, "Begin initializing PEP 451 python.pak functionality...");

    pyObjectRef implib = PyImport_ImportModule("_frozen_importlib");
    if (!implib) {
        dbgLog->AddLine(plStatusLog::kRed, "Failed to import _frozen_importlib!");
        return;
    }

    PyObject* moduleSpec = GetModuleItem("ModuleSpec", implib.Get());
    if (moduleSpec && PyType_Check(moduleSpec)) {
        s_ModuleSpec = (PyTypeObject*)moduleSpec;
    } else {
        dbgLog->AddLineF(plStatusLog::kRed, "Failed to get ModuleSpec from _frozen_importlib!");
        return;
    }

    dbgLog->AddLine(plStatusLog::kGreen, "Creating _PlasmaImport module...");

    // note: steals ref
    PyObject* mod = PyImport_AddModule("_PlasmaImport");
    if (!mod) {
        dbgLog->AddLine(plStatusLog::kRed, "Failed to create _PlasmaImport module!");
        return;
    }

    PYTHON_CLASS_IMPORT_START(mod);
    PYTHON_CLASS_IMPORT(mod, ptModulePackFinder);
    PYTHON_CLASS_IMPORT(mod, ptModulePackLoader);
    PYTHON_CLASS_IMPORT_END(mod);

    dbgLog->AddLine(plStatusLog::kGreen, "Installing PEP 451 machinery...");

    pyObjectRef metaFinder = pyModulePackFinder::New();
    ((ptModulePackFinder*)metaFinder.Get())->fThis->fLoader = pyModulePackLoader::New();
    PyObject* finders = PySys_GetObject("meta_path");
    hsAssert(finders, "Failed to access sys.meta_path");
    hsAssert(PyList_Check(finders), "sys.meta_path finders is not a list");
    PyList_Append(finders, metaFinder.Get());
}
