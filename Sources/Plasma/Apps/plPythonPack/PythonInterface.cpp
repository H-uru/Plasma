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

#include "compile.h"
#include "eval.h"
#include "marshal.h"
#include "cStringIO.h"
#include "plFileSystem.h"

#include <string_theory/stdio>
#include <string_theory/string>

static PyObject* stdOut;    // python object of the stdout file
static PyObject* stdErr;    // python object of the stderr file

void PythonInterface::initPython(const plFileName& rootDir, FILE* outstream, FILE* errstream)
{
    // if haven't been initialized then do it
    if (Py_IsInitialized() == 0)
    {
        // initialize the Python stuff
        // let Python do some intialization...
        Py_SetProgramName(const_cast<char*>("plasma"));
        Py_NoSiteFlag = 1;
        Py_IgnoreEnvironmentFlag = 1;
        Py_Initialize();

        // create the StringIO for the stdout and stderr file
        PycStringIO = (struct PycStringIO_CAPI*)PyCObject_Import(const_cast<char*>("cStringIO"), const_cast<char*>("cStringIO_CAPI"));
        stdOut = (*PycStringIO->NewOutput)(20000);
        stdErr = (*PycStringIO->NewOutput)(20000);

        // if we need the builtins then find the builtin module
        PyObject* sysmod = PyImport_ImportModule("sys");
        // then add the builtin dictionary to our module's dictionary
        if (sysmod != NULL)
        {
            // get the sys's dictionary to find the stdout and stderr
            PyObject* sys_dict = PyModule_GetDict(sysmod);
            if (sys_dict != nullptr && stdOut != nullptr) {
                PyDict_SetItemString(sys_dict, "stdout", stdOut);
            }
            if (sys_dict != nullptr && stdErr != nullptr) {
                PyDict_SetItemString(sys_dict, "stderr", stdErr);
            }
            // NOTE: we will reset the path to not include paths
            // ...that Python may have found in the registry
            PyObject* path_list = PyList_New(0);
            ST::printf(outstream, "Setting up include dirs:\n");

            ST::printf(outstream, "{}\n", rootDir);
            PyObject* more_path = PyString_FromString(rootDir.AsString().c_str());
            PyList_Append(path_list, more_path);

            // make sure that our plasma libraries are gotten before the system ones
            plFileName temp = plFileName::Join(rootDir, "plasma");
            ST::printf(outstream, "{}\n", temp);
            PyObject* more_path3 = PyString_FromString(temp.AsString().c_str());
            PyList_Append(path_list, more_path3);

            temp = plFileName::Join(rootDir, "system");
            ST::printf(outstream, "{}\n\n", temp);
            PyObject* more_path2 = PyString_FromString(temp.AsString().c_str());
            PyList_Append(path_list, more_path2);

            // set the path to be this one
            PyDict_SetItemString(sys_dict, "path", path_list);

            Py_DECREF(sysmod);
        }
    }
//  initialized++;
}

void PythonInterface::addPythonPath(const plFileName& path, FILE* outstream)
{
    PyObject* sysmod = PyImport_ImportModule("sys");
    if (sysmod != NULL)
    {
        PyObject* sys_dict = PyModule_GetDict(sysmod);
        PyObject* path_list = PyDict_GetItemString(sys_dict, "path");

        ST::printf(outstream, "Adding path {}\n", path);
        PyObject* more_path = PyString_FromString(path.AsString().c_str());
        PyList_Append(path_list, more_path);

        Py_DECREF(sysmod);
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
    PyObject* pycode = Py_CompileString(command, filename.AsString().c_str(), Py_file_input);
    return pycode;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DumpObject
//  PARAMETERS : pyobject       - string of commands to execute in the...
//
//  PURPOSE    : marshals an object into a char string
//
bool PythonInterface::DumpObject(PyObject* pyobj, char** pickle, int32_t* size)
{
    PyObject *s;        // the python string object where the marsalled object wil go
    // convert object to a marshalled string python object
#if (PY_MAJOR_VERSION == 2) && (PY_MINOR_VERSION < 4)
    s = PyMarshal_WriteObjectToString(pyobj);
#else
    s = PyMarshal_WriteObjectToString(pyobj, Py_MARSHAL_VERSION);
#endif
    // did it actually do it?
    if ( s != NULL )
    {
        // yes, then get the size and the string address
        *size = PyString_Size(s);
        *pickle =  PyString_AsString(s);
        return true;
    }
    else  // otherwise, there was an error
    {
        // Yikes! errors!
        PyErr_Print();  // FUTURE: we may have to get the string to display in max...later
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : getOutputAndReset
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the stdout file to be displayed
//
void PythonInterface::getOutputAndReset(ST::string& outmsg)
{
    PyObject* pyStr = (*PycStringIO->cgetvalue)(stdOut);
    outmsg = PyString_AsString(pyStr);

    // reset the file back to zero
    PyObject_CallMethod(stdOut, const_cast<char*>("reset"), const_cast<char*>(""));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : getErrorAndReset
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the stderr file to be displayed
//
void PythonInterface::getErrorAndReset(ST::string& errmsg)
{
    PyObject* pyStr = (*PycStringIO->cgetvalue)(stdErr);
    errmsg = PyString_AsString(pyStr);

    // reset the file back to zero
    PyObject_CallMethod(stdErr, const_cast<char*>("reset"), const_cast<char*>(""));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : CreateModule
//  PARAMETERS : module    - module name to create
//
//  PURPOSE    : create a new module with built-ins
//
PyObject* PythonInterface::CreateModule(const char* module)
{
    PyObject *m, *d;
// first we must get rid of any old modules of the same name, we'll replace it
    PyObject *modules = PyImport_GetModuleDict();
    if ((m = PyDict_GetItemString(modules, module)) != NULL && PyModule_Check(m))
        // clear it
        _PyModule_Clear(m);

// create the module
    m = PyImport_AddModule(module);
    if (m == NULL)
        return nil;
    d = PyModule_GetDict(m);
// add in the built-ins
    // first make sure that we don't already have the builtins
    if (PyDict_GetItemString(d, "__builtins__") == NULL)
    {
        // if we need the builtins then find the builtin module
        PyObject *bimod = PyImport_ImportModule("__builtin__");
        // then add the builtin dicitionary to our module's dictionary
        if (bimod == NULL || PyDict_SetItemString(d, "__builtins__", bimod) != 0)
            return nil;
        Py_DECREF(bimod);
    }
    return m;
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
        if (module == NULL)
            return false;
    }
    // get the dictionaries for this module
    d = PyModule_GetDict(module);
    // run the string
    v = PyEval_EvalCode((PyCodeObject*)code, d, d);
    // check for errors and print them
    if (v == NULL)
    {
        // Yikes! errors!
        PyErr_Print();
        return false;
    }
    Py_DECREF(v);
    if (Py_FlushLine())
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

