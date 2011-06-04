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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsTypes.h"
#include "PythonInterface.h"

#include "compile.h"
#include "eval.h"
#include "marshal.h"
#include "cStringIO.h"

static PyObject* stdFile;	// python object of the stdout and err file

void PythonInterface::initPython(std::string rootDir)
{
	// if haven't been initialized then do it
	if ( Py_IsInitialized() == 0 )
	{
		// initialize the Python stuff
		// let Python do some intialization...
		Py_SetProgramName("plasma");
		Py_Initialize();

		// intialize any of our special plasma python modules
//		initP2PInterface();
		// save object to the Plasma module
//		plasmaMod = PyImport_ImportModule("Plasma");

		// create the StringIO for the stdout and stderr file
		PycStringIO = (struct PycStringIO_CAPI*)PyCObject_Import("cStringIO", "cStringIO_CAPI");
		stdFile = (*PycStringIO->NewOutput)(20000);
		// if we need the builtins then find the builtin module
		PyObject* sysmod = PyImport_ImportModule("sys");
		// then add the builtin dicitionary to our module's dictionary
		if (sysmod != NULL )
		{
			// get the sys's dictionary to find the stdout and stderr
			PyObject* sys_dict = PyModule_GetDict(sysmod);
			if (stdFile != nil)
			{
				PyDict_SetItemString(sys_dict,"stdout", stdFile);
				PyDict_SetItemString(sys_dict,"stderr", stdFile);
			}
			// NOTE: we will reset the path to not include paths
			// ...that Python may have found in the registery
			PyObject* path_list = PyList_New(0);
			printf("Setting up include dirs:\n");
			printf("%s\n",rootDir.c_str());
			PyObject* more_path = PyString_FromString(rootDir.c_str());
			PyList_Append(path_list, more_path);
			// make sure that our plasma libraries are gotten before the system ones
			std::string temp = rootDir + "plasma";
			printf("%s\n",temp.c_str());
			PyObject* more_path3 = PyString_FromString(temp.c_str());
			PyList_Append(path_list, more_path3);
			temp = rootDir + "system";
			printf("%s\n\n",temp.c_str());
			PyObject* more_path2 = PyString_FromString("system");
			PyList_Append(path_list, more_path2);
			// set the path to be this one
			PyDict_SetItemString(sys_dict,"path",path_list);


			Py_DECREF(sysmod);
		}
	}
//	initialized++;
}

void PythonInterface::addPythonPath(std::string path)
{
	PyObject* sysmod = PyImport_ImportModule("sys");
	if (sysmod != NULL)
	{
		PyObject* sys_dict = PyModule_GetDict(sysmod);
		PyObject* path_list = PyDict_GetItemString(sys_dict, "path");

		printf("Adding path %s\n", path.c_str());
		PyObject* more_path = PyString_FromString(path.c_str());
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
PyObject* PythonInterface::CompileString(char *command, char* filename)
{
	PyObject* pycode = Py_CompileString(command, filename, Py_file_input);
	return pycode;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DumpObject
//  PARAMETERS : pyobject       - string of commands to execute in the...
//
//  PURPOSE    : marshals an object into a char string
//
hsBool PythonInterface::DumpObject(PyObject* pyobj, char** pickle, Int32* size)
{
	PyObject *s;		// the python string object where the marsalled object wil go
	// convert object to a marshalled string python object
	s = PyMarshal_WriteObjectToString(pyobj);
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
		PyErr_Print();	// FUTURE: we may have to get the string to display in max...later
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : getOutputAndReset
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the error file to be displayed
//
int PythonInterface::getOutputAndReset(char** line)
{
	PyObject* pyStr = (*PycStringIO->cgetvalue)(stdFile);
	char *str = PyString_AsString( pyStr );
	int size = PyString_Size( pyStr );

	// reset the file back to zero
	PyObject_CallMethod(stdFile,"reset","");
/*
	// check to see if the debug python module is loaded
	if ( dbgOut != nil )
	{
		// then send it the new text
		if ( PyObject_CallFunction(dbgOut,"s",str) == nil )
		{
			// for some reason this function didn't, remember that and not call it again
			dbgOut = nil;
			// if there was an error make sure that the stderr gets flushed so it can be seen
			PyErr_Print();		// make sure the error is printed
			PyErr_Clear();		// clear the error
		}
	}
*/
	if (line)
		*line = str;
	return size;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : CreateModule
//  PARAMETERS : module    - module name to create
//
//  PURPOSE    : create a new module with built-ins
//
PyObject* PythonInterface::CreateModule(char* module)
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
hsBool PythonInterface::RunPYC(PyObject* code, PyObject* module)
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
PyObject* PythonInterface::GetModuleItem(char* item, PyObject* module)
{
	if ( module )
	{
		PyObject* d = PyModule_GetDict(module);
		return PyDict_GetItemString(d, item);
	}
	return nil;
}

