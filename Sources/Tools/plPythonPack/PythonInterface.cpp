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
#include "PythonInterface.h"

#include <cpython/initconfig.h>
#include <marshal.h>
#include <pylifecycle.h>


#include "plFileSystem.h"

#include <string_theory/stdio>
#include <string_theory/string>

static PyObject* stdOut;    // python object of the stdout file
static PyObject* stdErr;    // python object of the stderr file

static inline void IAddWideString(PyWideStringList& list, const plFileName& filename)
{
    PyWideStringList_Append(&list, filename.AsString().to_wchar().data());
}

void PythonInterface::initPython(const plFileName& rootDir, const std::vector<plFileName>& extraDirs,
                                 FILE* outstream, FILE* errstream)
{
    // if haven't been initialized then do it
    if (Py_IsInitialized() == 0) {
        PyPreConfig preConfig;
        PyPreConfig_InitIsolatedConfig(&preConfig);
        PyStatus status = Py_PreInitialize(&preConfig);
        if (PyStatus_Exception(status)) {
            ST::printf(stderr, "Python {} pre-init failed: {}", PY_VERSION, status.err_msg);
            return;
        }

        PyConfig config;
        PyConfig_InitIsolatedConfig(&config);
        config.optimization_level = 2;
        config.write_bytecode = 0;
        config.site_import = 0;
        PyConfig_SetString(&config, &config.program_name, L"plasma");

        // Explicit module search paths so no build-env specific stuff gets in.
        IAddWideString(config.module_search_paths, rootDir);
        IAddWideString(config.module_search_paths, plFileName::Join(rootDir, "plasma"));
        IAddWideString(config.module_search_paths, plFileName::Join(rootDir, "system"));
        for (const auto& dir : extraDirs)
            IAddWideString(config.module_search_paths, plFileName::Join(rootDir, dir));
        config.module_search_paths_set = 1;

        // initialize the Python stuff
        status = Py_InitializeFromConfig(&config);
        if (PyStatus_Exception(status)) {
            ST::printf(stderr, "Python {} init failed: {}", PY_VERSION, status.err_msg);
            PyConfig_Clear(&config);
            return;
        }
        PyConfig_Clear(&config);

        if (stdOut)
            PySys_SetObject("stdout", stdOut);
        if (stdErr)
            PySys_SetObject("stderr", stdErr);
    }
}

void PythonInterface::finiPython()
{
    if (Py_IsInitialized() != 0)
    {
        Py_Finalize();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : CompileString
//  PARAMETERS : command       - string of commands to execute in the...
//             : filename      - filename to say where to code came from
//
//  PURPOSE    : run a python string in a specific module name
//
PyObject* PythonInterface::CompileString(const char *command, const plFileName& filename)
{
    PyObject* filenameObj = PyUnicode_FromStringAndSize(
        filename.AsString().c_str(),
        filename.AsString().size()
    );

    PyCompilerFlags flags{};
    flags.cf_feature_version = PY_MINOR_VERSION;

    // Py_CompileString decodes the filename using the filesystem encoding.
    // This is always UTF-8 on Windows as of Python 3.6 (see PEP 529), but we
    // probably shouldn't rely on that. We always use UTF-8 internally, so
    // pass the filename as a Python string object.
    PyObject* pycode = Py_CompileStringObject(
        command,
        filenameObj,
        Py_file_input,
        &flags,
        -1
    );

    Py_DECREF(filenameObj);
    return pycode;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DumpObject
//  PARAMETERS : pyobject       - string of commands to execute in the...
//
//  PURPOSE    : marshals an object into a char string
//
bool PythonInterface::DumpObject(PyObject* pyobj, char** pickle, Py_ssize_t* size)
{
    PyObject *s;        // the python string object where the marsalled object wil go
    // convert object to a marshalled string python object
    s = PyMarshal_WriteObjectToString(pyobj, Py_MARSHAL_VERSION);
    // did it actually do it?
    if (s != nullptr)
    {
        // yes, then get the size and the string address
        *size = PyBytes_Size(s);
        *pickle = new char[*size];
        memcpy(*pickle, PyBytes_AS_STRING(s), *size);
        Py_DECREF(s);
        return true;
    }
    else  // otherwise, there was an error
    {
        *pickle = nullptr;
        *size = 0;

        // Yikes! errors!
        PyErr_Print();  // FUTURE: we may have to get the string to display in max...later
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunPYC
//  PARAMETERS : code      - compiled code
//             : module    - module name to run the code in
//
//  PURPOSE    : run a compiled python code in a specific module name
//
bool PythonInterface::RunPYC(PyObject* code, PyObject* module)
{
    PyObject *d, *v;
    // make sure that we're given a good module... or at least one with an address
    if ( !module )
    {
        // if no module was given then use just use the main module
        module = PyImport_AddModule("__main__");
        if (module == nullptr)
            return false;
    }
    // get the dictionaries for this module
    d = PyModule_GetDict(module);

    if (!PyDict_GetItemString(d, "__builtins__"))
        PyDict_SetItemString(d, "__builtins__", PyEval_GetBuiltins());

    // run the string
    v = PyEval_EvalCode(code, d, d);
    // check for errors and print them
    if (v == nullptr)
    {
        // Yikes! errors!
        PyErr_Print();
        return false;
    }
    Py_DECREF(v);
    PyErr_Clear();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetModuleItem
//  PARAMETERS : item    - what item in the plasma module to get
//
//  PURPOSE    : get an item (probably a function) from a specific module
//
PyObject* PythonInterface::GetModuleItem(const char* item, PyObject* module)
{
    if (module) {
        PyObject* d = PyModule_GetDict(module);
        return PyDict_GetItemString(d, item);
    }
    return nullptr;
}

