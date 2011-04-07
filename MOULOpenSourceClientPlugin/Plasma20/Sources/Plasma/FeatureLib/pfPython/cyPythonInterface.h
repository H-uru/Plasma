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
//////////////////////////////////////////////////////////////////////
//
// PythonInterface   - The Python interface to the Python dll
//
// NOTE: Eventually, this will be made into a separate dll, because there should
//       only be one instance of this interface. 
//
#include "hsTypes.h"
#include "hsStlUtils.h"
#include <python.h>

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
#include "../../Apps/CyPythonIDE/plCyDebug/plCyDebServer.h"
#endif

class plStatusLog;
class pyKey;

class  PythonInterface
{
private:
	static Int32 initialized;	// count how many times we initialize
								// and make sure that many finalize on the way out
	static hsBool FirstTimeInit;
	static hsBool IsInShutdown; // whether we are _really_ in shutdown mode

	static PyMethodDef* plasmaMethods;
	static PyObject* plasmaMod;	// python object that holds the Plasma module
	static PyObject* plasmaConstantsMod; // python object that holds the PlasmaConstants module
	static PyObject* plasmaNetConstantsMod; // python object that holds the PlasmaNetConstants module
	static PyObject* plasmaVaultConstantsMod; // python object that holds the PlasmaVaultConstants module
	static PyMethodDef* plasmaGameMethods;
	static PyObject* plasmaGameMod; // python object that holds the PlasmaGame module
	static PyObject* plasmaGameConstantsMod; // python object that holds the PlasmaGameConstants module
	static PyObject* stdOut;	// python object of the stdout file
	static PyObject* stdErr;	// python object of the err file

	static hsBool debug_initialized;	// has the debug been initialized yet?
	static PyObject* dbgMod;	// display module for stdout and stderr
	static PyObject* dbgOut;
	static PyObject* dbgSlice;	// time slice function for the debug window
	static plStatusLog* dbgLog;

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
	static bool usePythonDebugger;
	static bool requestedExit;
	static plCyDebServer debugServer;
#endif

public:

	// set that we are truly shutting down
	static void WeAreInShutdown() { IsInShutdown = true; }

	// Initialize the Python dll
	static void initPython();

	// Initialize the Plasma module
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	static void AddPlasmaClasses();

	// Initialize the PlasmaConstants module
	static void AddPlasmaConstantsClasses();

	// Initialize the PlasmaNetConstants module;
	static void AddPlasmaNetConstantsClasses();

	// Initialize the PlasmaVaultConstants module;
	static void AddPlasmaVaultConstantsClasses();

	// Initialize the PlasmaGame module
	static void AddPlasmaGameMethods(std::vector<PyMethodDef> &methods);
	static void AddPlasmaGameClasses();

	// Initialize the PlasmaGameConstants module
	static void AddPlasmaGameConstantsClasses();

	// Initialize the Python to Plasma 
	static void initDebugInterface();

	// Finalize the Python dll, ie. get ready to shut down
	static void finiPython();

	// give the debug window a time slice
	static void debugTimeSlice();

	// get the stdout PyObject
	static PyObject* GetStdOut();
	static PyObject* GetStdErr();

	// get the Output to the error file to be displayed
	static int getOutputAndReset(std::string* output = nil);

	// Writes 'text' to the Python log
	static void WriteToLog(const char* text);

	// Writes 'text' to stderr specified in the python interface
	static void WriteToStdErr(const char* text);

	// Find module. If it doesn't exist then don't create, return nil.
	static PyObject* FindModule(char* module);

	// create a new module with built-ins
	static PyObject* CreateModule(char* module);

	// checks to see if a specific function is defined in this module
	// get an item (probably a function) from the Plasma module
	static PyObject* GetPlasmaItem(char* item);

	// Determine if the module name is unique
	static hsBool IsModuleNameUnique(char* module);
	// get an item (probably a function) from a specific module
	static PyObject* GetModuleItem(char* item, PyObject* module);

	// check a specific module for the define funcitons
	static void CheckModuleForFunctions(PyObject* module, char** funcNames, PyObject** funcTable);

	//	checks to see if a specific function is defined in this instance of a class
	//  and will fill out the funcTable with object instances of where the funciton is
	//
	static void CheckInstanceForFunctions(PyObject* instance, char** funcNames, PyObject** funcTable);

	//  run a python string in a specific module name
	//  PARAMETERS : command       - string of commands to execute in the...
	//             : filename      - filename to say where to code came from
	static PyObject* CompileString(char *command, char* filename);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : DumpObject
	//  PARAMETERS : pyobject       - string of commands to execute in the...
	//
	//  PURPOSE    : marshals an object into a char string
	//
	static hsBool DumpObject(PyObject* pyobj, char** pickle, Int32* size);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : LoadObject
	//  PARAMETERS : pickle    - the pickled object in char string form
	//             : size      - size of the guts to load into an object
	//
	//  PURPOSE    : Load a python object from a pickled object
	//
	static PyObject* LoadObject(char* pickle, Int32 size);


	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RunStringInteractive
	//  PARAMETERS : command   - string of commands to execute in the...
	//             : module    - module name to run 'command' in
	//
	//  PURPOSE    : run a python string in a specific module name
	//             : Interactive mode (displays results)
	//
	static hsBool RunStringInteractive(char *command, PyObject* module);


	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RunString
	//  PARAMETERS : command   - string of commands to execute in the...
	//             : module    - module name to run 'command' in
	//
	//  PURPOSE    : run a python string in a specific module name
	//
	static hsBool RunString(char *command, PyObject* module);


	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RunPYC
	//  PARAMETERS : code      - compiled code
	//             : module    - module name to run the code in
	//
	//  PURPOSE    : run a compiled python code in a specific module name
	//
	static hsBool RunPYC(PyObject* code, PyObject* module);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetpyKeyFromPython
	//  PARAMETERS : pkey      - python object that is a pyKey (ptKey) class
	//
	//  PURPOSE    : turn a PyObject* into a pyKey*
	//
	static pyKey* GetpyKeyFromPython(PyObject* pkey);

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
	static bool UsePythonDebugger() { return usePythonDebugger; }
	static void UsePythonDebugger(bool use) { usePythonDebugger = use; }

	static plCyDebServer* PythonDebugger() {return &debugServer;}
	static bool DebuggerRequestedExit() {return requestedExit;}
	static void DebuggerRequestedExit(bool reqExit) {requestedExit = reqExit;}
#endif
};
