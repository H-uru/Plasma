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
#include "Python.h"

#include <string>

namespace PythonInterface
{
	void initPython(std::string rootDir);
	void finiPython();
	// So the Python packer can add extra paths
	void addPythonPath(std::string dir);

	PyObject* CompileString(char *command, char* filename);
	hsBool DumpObject(PyObject* pyobj, char** pickle, Int32* size);
	int getOutputAndReset(char** line=nil);
	PyObject* CreateModule(char* module);
	hsBool RunPYC(PyObject* code, PyObject* module);
	PyObject* GetModuleItem(char* item, PyObject* module);
}
