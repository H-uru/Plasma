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
#include "pyPloticus.h"

#include "../pfPython/pyGlueHelpers.h"
#include <python.h>

#include <vector>

////////////////////////////////////////////////////////////////////

extern "C" {
// Ploticus C API
int ploticus_init(char *, char *);
int ploticus_arg(char *, char *);
int ploticus_begin();
void ploticus_end();
void ploticus_execline(char *);
int ploticus_execscript(char *, int);
int ploticus_getvar(char *, char *);
void ploticus_setvar(char *, char *);
}

////////////////////////////////////////////////////////////////////

void pyPloticus::Init(char* device, char* outfilename)
{
	ploticus_init( device, outfilename );
}

void pyPloticus::Arg(char* name, char* value)
{
	ploticus_arg( name, value );
}

void pyPloticus::Begin()
{
	ploticus_begin();
}

void pyPloticus::End()
{
	ploticus_end();
}

void pyPloticus::ExecLine(char* line)
{
	ploticus_execline( line );
}

void pyPloticus::ExecScript(char* scriptfile, int prefab)
{
	ploticus_execscript( scriptfile, prefab );
}

void pyPloticus::GetVar(char* name, char* value)
{
	ploticus_getvar( name, value );
}

void pyPloticus::SetVar(char* name, char* value)
{
	ploticus_setvar( name, value );
}

PYTHON_GLOBAL_METHOD_DEFINITION(init, args, "Params: device,outfilename\nUNKNOWN")
{
	char* device;
	char* outfilename;
	if (!PyArg_ParseTuple(args, "ss", &device, &outfilename))
	{
		PyErr_SetString(PyExc_TypeError, "init expects two strings");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::Init(device, outfilename);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(arg, args, "Params: name,value\nUNKNOWN")
{
	char* name;
	char* value;
	if (!PyArg_ParseTuple(args, "ss", &name, &value))
	{
		PyErr_SetString(PyExc_TypeError, "arg expects two strings");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::Arg(name, value);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(begin, pyPloticus::Begin, "UNKNOWN")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(end, pyPloticus::End, "UNKNOWN")

PYTHON_GLOBAL_METHOD_DEFINITION(execLine, args, "Params: line\nUNKNOWN")
{
	char* line;
	if (!PyArg_ParseTuple(args, "s", &line))
	{
		PyErr_SetString(PyExc_TypeError, "execLine expects a string");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::ExecLine(line);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(execScript, args, "Params: file,prefab\nUNKNOWN")
{
	char* file;
	int prefab;
	if (!PyArg_ParseTuple(args, "si", &file, &prefab))
	{
		PyErr_SetString(PyExc_TypeError, "execScript expects a string and an int");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::ExecScript(file, prefab);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(getVar, args, "Params: name,value\nUNKNOWN")
{
	char* name;
	char* value;
	if (!PyArg_ParseTuple(args, "ss", &name, &value))
	{
		PyErr_SetString(PyExc_TypeError, "getVar expects two strings");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::GetVar(name, value);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(setVar, args, "Params: name,value\nUNKNOWN")
{
	char* name;
	char* value;
	if (!PyArg_ParseTuple(args, "ss", &name, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setVar expects two strings");
		PYTHON_RETURN_ERROR;
	}
	pyPloticus::SetVar(name, value);
	PYTHON_RETURN_NONE;
}

void AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, init);
	PYTHON_GLOBAL_METHOD(methods, arg);
	PYTHON_BASIC_GLOBAL_METHOD(methods, begin);
	PYTHON_BASIC_GLOBAL_METHOD(methods, end);
	PYTHON_GLOBAL_METHOD(methods, execLine);
	PYTHON_GLOBAL_METHOD(methods, execScript);
	PYTHON_GLOBAL_METHOD(methods, getVar);
	PYTHON_GLOBAL_METHOD(methods, setVar);
}

////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) void PyInit_pyPloticus(void)
{
	std::vector<PyMethodDef> methods; // this is temporary, for easy addition of new methods
	AddPlasmaMethods(methods);

	// now copy the data to our real method definition structure
	PyMethodDef* plasmaMethods = new PyMethodDef[methods.size() + 1];
	for (int curMethod = 0; curMethod < methods.size(); curMethod++)
		plasmaMethods[curMethod] = methods[curMethod];
	PyMethodDef terminator = {NULL};
	plasmaMethods[methods.size()] = terminator; // add the terminator

	// Init the module
	PyObject *m = Py_InitModule("pyPloticus", plasmaMethods);

	delete [] plasmaMethods; // clean up
}

////////////////////////////////////////////////////////////////////
