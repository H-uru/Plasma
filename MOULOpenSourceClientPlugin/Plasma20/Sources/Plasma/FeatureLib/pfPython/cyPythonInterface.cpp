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
#include "cyPythonInterface.h"

#include "compile.h"
#include "marshal.h"
#include "eval.h"

#include "pyEnum.h"

#include "pyKey.h"
#include "cyDraw.h"
#include "cyPhysics.h"
#include "pySceneObject.h"
#include "cyMisc.h"
#include "cyCamera.h"
#include "pyNotify.h"
#include "cyAvatar.h"
#include "pyGeometry3.h"
#include "pyMatrix44.h"
#include "pyColor.h"
#include "pyDynamicText.h"
#include "cyAnimation.h"
#include "pyPlayer.h"
#include "pyImage.h"
#include "pyDniCoordinates.h"
#include "cyInputInterface.h"
#include "pySDL.h"
#include "cyAccountManagement.h"

// GUIDialog and its controls
#include "pyGUIDialog.h"
#include "pyGUIControlButton.h"
#include "pyGUIControlDragBar.h"
#include "pyGUIControlCheckBox.h"
#include "pyGUIControlListBox.h"
#include "pyGUIControlEditBox.h"
#include "pyGUIControlMultiLineEdit.h"
#include "pyGUIControlRadioGroup.h"
#include "pyGUIControlTextBox.h"
#include "pyGUIControlValue.h"
#include "pyGUIControlDynamicText.h"
#include "pyGUIControlClickMap.h"
#include "pyGUIControlDraggable.h"
#include "pyGUIPopUpMenu.h"
#include "pyGUISkin.h"

#include "plPythonSDLModifier.h"

// For printing to the log
#include "../plStatusLog/plStatusLog.h"
#include "../plNetGameLib/plNetGameLib.h"

// vault
#include "pyVaultNode.h"
#include "pyVaultFolderNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultImageNode.h"
#include "pyVaultTextNoteNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeInfoListNode.h"
#include "pyVaultSDLNode.h"
#include "pyVaultNodeRef.h"
#include "pyVaultMarkerGameNode.h"
#include "pyVaultSystemNode.h"

// player vault
#include "pyVault.h"
// age vault
#include "pyAgeVault.h"

// net linking mgr
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"

// dni info source
#include "pyDniInfoSource.h"

// audio setting stuff
#include "pyAudioControl.h"

//CCR stufff
#include "pyCCRMgr.h"

// spawn point def
#include "pySpawnPointInfo.h"

#include "pyMarkerMgr.h"
#include "pyStatusLog.h"

// Guess what this is for :P
#include "pyJournalBook.h"

#include "pyKeyMap.h"
#include "pyStream.h"

#include "pyMoviePlayer.h"
#include "pyDrawControl.h"

#include "pyWaveSet.h"
#include "pySwimCurrentInterface.h"

#include "pyCluster.h"
#include "pyGrassShader.h"

#include "pyScoreMgr.h"
#include "pyGameScore.h"

#include "pyCritterBrain.h"

// Game manager stuff
#include "Games/pyGameMgrMsg.h"
#include "Games/pyGameCliMsg.h"
#include "Games/pyGameCli.h"
#include "Games/TicTacToe/pyTTTMsg.h"
#include "Games/TicTacToe/pyTTTGame.h"
#include "Games/Heek/pyHeekMsg.h"
#include "Games/Heek/pyHeekGame.h"
#include "Games/Marker/pyMarkerMsg.h"
#include "Games/Marker/pyMarkerGame.h"
#include "Games/BlueSpiral/pyBlueSpiralMsg.h"
#include "Games/BlueSpiral/pyBlueSpiralGame.h"
#include "Games/ClimbingWall/pyClimbingWallMsg.h"
#include "Games/ClimbingWall/pyClimbingWallGame.h"
#include "Games/VarSync/pyVarSyncMsg.h"
#include "Games/VarSync/pyVarSyncGame.h"

Int32 PythonInterface::initialized = 0;					// only need to initialize all of Python once
hsBool PythonInterface::FirstTimeInit = true;			// start with "this is the first time"
hsBool PythonInterface::IsInShutdown = false;			// whether we are _really_ in shutdown mode

PyMethodDef* PythonInterface::plasmaMethods = nil;		// the Plasma module's methods
PyObject* PythonInterface::plasmaMod = nil;				// pointer to the Plasma module
PyObject* PythonInterface::plasmaConstantsMod = nil;	// pointer to the PlasmaConstants module
PyObject* PythonInterface::plasmaNetConstantsMod = nil;	// pointer to the PlasmaNetConstants module
PyObject* PythonInterface::plasmaVaultConstantsMod = nil; // pointer to the PlasmaVaultConstants module
PyMethodDef* PythonInterface::plasmaGameMethods = nil;	// the PlasmaGame module's methods
PyObject* PythonInterface::plasmaGameMod = nil;			// python object that holds the PlasmaGame module
PyObject* PythonInterface::plasmaGameConstantsMod = nil; // python object that holds the PlasmaGameConstants module
PyObject* PythonInterface::stdOut = nil;				// python object of the stdout file
PyObject* PythonInterface::stdErr = nil;				// python object of the err file

hsBool PythonInterface::debug_initialized = false;		// has the debug been initialized yet?
PyObject* PythonInterface::dbgMod = nil;				// display module for stdout and stderr
PyObject* PythonInterface::dbgOut = nil;
PyObject* PythonInterface::dbgSlice = nil;				// time slice function for the debug window
plStatusLog* PythonInterface::dbgLog = nil;				// output logfile

#ifndef PLASMA_EXTERNAL_RELEASE
bool PythonInterface::usePythonDebugger = false;
plCyDebServer PythonInterface::debugServer;
bool PythonInterface::requestedExit = false;
#endif

// stupid Windows.h  and who started including that!
#undef DrawText

#ifndef PLASMA_EXTERNAL_RELEASE
// Special includes for debugging
#include <frameobject.h>

/////////////////////////////////////////////////////////////////////////////
// Our debugger callback class
class DebuggerCallback: public plCyDebServer::IDebServerCallback
{
private:
	plCyDebServer& fServer;

	PyFrameObject* fFrame;
	PyObject* fExceptionInfo;

	std::string IParseCurrentException(); // returns the current exception as a string representation, and clears it

public:
	DebuggerCallback(plCyDebServer& server): fServer(server) {}

	virtual bool MsgReceive(const plCyDebMessage& msg);
	virtual std::string AdjustFilename(const std::string& filename);
	virtual bool CheckBreakpointCondition(const std::string& condition, std::string& error);

	virtual void InitializeBreak();
	virtual std::vector<std::string> GenerateCallstack();
	virtual std::vector<std::pair<std::string, std::string> > GenerateGlobalsList();
	virtual std::vector<std::pair<std::string, std::string> > GenerateLocalsList();
	virtual std::string EvaluateVariable(const std::string& varName);
	virtual void SetVariableValue(const std::string& varName, const std::string& newValue);

	void SetFrame(PyFrameObject* frame) {fFrame = frame;}
	void SetExceptionInfo(PyObject* exceptionInfo) {fExceptionInfo = exceptionInfo;}
};

std::string DebuggerCallback::IParseCurrentException()
{
	std::string error = "";

	if (PyErr_Occurred() == NULL)
		return error; // no error occurred

	PyObject* errType = NULL;
	PyObject* errVal = NULL;
	PyObject* errTraceback = NULL;
	PyErr_Fetch(&errType, &errVal, &errTraceback); // clears the error flag
	PyErr_NormalizeException(&errType, &errVal, &errTraceback);

	if (PyErr_GivenExceptionMatches(errType, PyExc_SyntaxError))
	{
		// we know how to parse out information from syntax errors
		PyObject* message;
		char* filename = NULL;
		int lineNumber = 0;
		int offset = 0;
		char* text = NULL;

		if (PyTuple_Check(errVal))
		{
			// nested tuple, parse out the error information
			PyArg_Parse(errVal, "(O(ziiz))", &message, &filename, &lineNumber, &offset, &text);
			error += PyString_AsString(message);
			if (text)
				error += text;
		}
		else
		{
			// probably just the error class, retrieve the message and text directly
			PyObject* v;
			if ((v = PyObject_GetAttrString(errVal, "msg")))
			{
				error += PyString_AsString(v);
				Py_DECREF(v);
			}
			if ((v == PyObject_GetAttrString(errVal, "text")))
			{
				if (v != Py_None)
					error += PyString_AsString(v);
				Py_DECREF(v);
			}
		}
	}
	else if (PyClass_Check(errType))
	{
		// otherwise, just return the type of error that occurred
		PyClassObject* exc = (PyClassObject*)errType;
		PyObject* className = exc->cl_name;
		if (className)
			error += PyString_AsString(className);
	}
	else
		error = "Unknown Error";

	// cleanup
	Py_XDECREF(errType);
	Py_XDECREF(errVal);
	Py_XDECREF(errTraceback);

	return error;
}

bool DebuggerCallback::MsgReceive(const plCyDebMessage& msg)
{
	switch (msg.GetMsgType())
	{
	case plCyDebMessage::kMsgExit:
		PythonInterface::DebuggerRequestedExit(true);
		break;
	}
	return false; // let default handling take over
}

std::string DebuggerCallback::AdjustFilename(const std::string& filename)
{
	// python doesn't deal with paths, so we strip out all path information
	std::string retVal = filename;
	std::string::size_type slashPos = filename.rfind('\\');
	if (slashPos != std::string::npos)
		retVal = filename.substr(slashPos + 1);
	else // no back-slashes, look for forward ones
	{
		slashPos = filename.rfind('/');
		if (slashPos != std::string::npos)
			retVal = filename.substr(slashPos + 1);
	}
	return retVal;
}

bool DebuggerCallback::CheckBreakpointCondition(const std::string& condition, std::string& error)
{
	if (!fFrame)
		return true; // just break, we have no current frame?

	if (condition == "")
		return true; // empty condition, break (python doesn't like empty strings)

	// initialize locals, since InitializeBreak isn't called til we break
	PyFrame_FastToLocals(fFrame);

	// run the string in the current context
	PyObject* result = PyRun_String(const_cast<char*>(condition.c_str()), Py_eval_input, fFrame->f_globals, fFrame->f_locals);
	if (result)
	{
		// is the result true?
		int retVal = PyObject_IsTrue(result);
		Py_DECREF(result);

		return (retVal == 1);
	}

	// error occurred, translate it and return
	error = IParseCurrentException();
	return true;
}

void DebuggerCallback::InitializeBreak()
{
	// do a little initialization of our frame (ensuring we get all local data)
	PyFrame_FastToLocals(fFrame);
}

std::vector<std::string> DebuggerCallback::GenerateCallstack()
{
	std::vector<std::string> retVal;

	// we use the frame stored for us by the trace function
	PyFrameObject* curFrame = fFrame;
	while (curFrame)
	{
		std::string filename = PyString_AsString(curFrame->f_code->co_filename);
		int lineNumber = PyCode_Addr2Line(curFrame->f_code, curFrame->f_lasti); // python uses base-1 numbering, we use base-0, but for display we want base-1
		std::string functionName = PyString_AsString(curFrame->f_code->co_name);

		functionName += "(";
		if (curFrame->f_code->co_argcount)
		{
			// we have arguments!
			int argCount = __min(PyTuple_Size(curFrame->f_code->co_varnames), curFrame->f_code->co_argcount);

			for (int curArg = 0; curArg < argCount; ++curArg)
			{
				PyObject* argName = PyTuple_GetItem(curFrame->f_code->co_varnames, curArg);
				if (argName)
				{
					std::string arg = PyString_AsString(argName);
					if (arg == "self")
						continue; // skip self, for readability

					functionName += arg;

					if (curFrame->f_locals)
					{
						// grab value, if our locals dictionary exists
						PyObject* val = PyDict_GetItemString(curFrame->f_locals, arg.c_str());
						if (val)
						{
							functionName += "=";
							functionName += PyString_AsString(PyObject_Str(val));
						}
					}
				}

				if (curArg < argCount - 1)
					functionName += ", ";
			}
		}
		functionName += ")";

		// add it to the callstack
		retVal.push_back(fServer.ConstructCallstackLine(filename, lineNumber, functionName));

		// and step back one frame
		curFrame = curFrame->f_back;
	}

	return retVal;
}

std::vector<std::pair<std::string, std::string> > DebuggerCallback::GenerateGlobalsList()
{
	std::vector<std::pair<std::string, std::string> > retVal;
	if (fFrame && fFrame->f_globals)
	{
		int pos = 0;
		PyObject* key;
		PyObject* value;
		while (PyDict_Next(fFrame->f_globals, &pos, &key, &value))
		{
			// leave modules out of the globals display
			if (key && value && !PyModule_Check(value))
			{
				// leave out glue functions
				if (PyObject_Compare((PyObject*)&PyCFunction_Type, PyObject_Type(value)) == 0)
					continue;

				std::string keyStr = PyString_AsString(PyObject_Str(key));
				if (keyStr == "__builtins__")
					continue; // skip builtins

				bool addQuotes = (PyString_Check(value) || PyUnicode_Check(value));

				std::string valueStr = "";
				if (addQuotes)
					valueStr += "\"";
				valueStr += PyString_AsString(PyObject_Str(value));
				if (addQuotes)
					valueStr += "\"";

				// add it to the list of pairs
				retVal.push_back(std::pair<std::string, std::string>(keyStr, valueStr));
			}
		}
	}
	return retVal;
}

std::vector<std::pair<std::string, std::string> > DebuggerCallback::GenerateLocalsList()
{
	std::vector<std::pair<std::string, std::string> > retVal;
	if (fFrame && fFrame->f_locals)
	{
		int pos = 0;
		PyObject* key;
		PyObject* value;
		while (PyDict_Next(fFrame->f_locals, &pos, &key, &value))
		{
			// leave modules and instances out of the globals display
			if (key && value && !PyModule_Check(value) && !PyInstance_Check(value))
			{
				// leave out functions, classes, and types
				if (PyObject_Compare((PyObject*)&PyFunction_Type, PyObject_Type(value)) == 0)
					continue;
				if (PyObject_Compare((PyObject*)&PyClass_Type, PyObject_Type(value)) == 0)
					continue;
				if (PyObject_Compare((PyObject*)&PyType_Type, PyObject_Type(value)) == 0)
					continue;

				std::string keyStr = PyString_AsString(PyObject_Str(key));
				if (keyStr == "__builtins__")
					continue; // skip builtins

				bool addQuotes = (PyString_Check(value) || PyUnicode_Check(value));

				std::string valueStr = "";
				if (addQuotes)
					valueStr += "\"";
				valueStr += PyString_AsString(PyObject_Str(value));
				if (addQuotes)
					valueStr += "\"";

				// add it to the list of pairs
				retVal.push_back(std::pair<std::string, std::string>(keyStr, valueStr));
			}
		}
	}
	return retVal;
}

std::string DebuggerCallback::EvaluateVariable(const std::string& varName)
{
	if (fFrame)
	{
		PyObject* evalResult = PyRun_String(const_cast<char*>(varName.c_str()), Py_eval_input, fFrame->f_globals, fFrame->f_locals);
		std::string retVal = "";
		if (evalResult)
		{
			// convert the result to something readable
			PyObject* reprObj = PyObject_Repr(evalResult);
			if (reprObj)
				retVal = PyString_AsString(reprObj);
			else
				retVal = "<REPR FAIL>";
			Py_XDECREF(reprObj);
		}
		else
			retVal = IParseCurrentException();
		Py_XDECREF(evalResult);
		return retVal;
	}
	else
		return "<NO FRAME>";
}

void DebuggerCallback::SetVariableValue(const std::string& varName, const std::string& newValue)
{
	std::string expression = varName + "=" + newValue;
	if (fFrame)
	{
		PyObject* evalResult = PyRun_String(const_cast<char*>(expression.c_str()), Py_single_input, fFrame->f_globals, fFrame->f_locals);
		if (evalResult)
			PyFrame_LocalsToFast(fFrame, 0); // convert the locals that changed (if any) back to "fast" locals
		else
			PyErr_Print();
		Py_XDECREF(evalResult);
	}
}

DebuggerCallback debServerCallback(*PythonInterface::PythonDebugger());

// python trace function, handles most of the work required for debugging
static int PythonTraceCallback(PyObject*, PyFrameObject* frame, int what, PyObject* arg)
{
	// obj (first parameter) is always NULL for is (it's the parameter passed by the set trace function)

	// update the callback class' stored values
	debServerCallback.SetFrame(frame);
	debServerCallback.SetExceptionInfo(NULL);

	// translate the python what value to the debugger what value
	plCyDebServer::TraceWhat debuggerWhat;
	switch (what)
	{
	case PyTrace_LINE:
		debuggerWhat = plCyDebServer::kTraceLine;
		break;

	case PyTrace_CALL:
		debuggerWhat = plCyDebServer::kTraceCall;
		break;

	case PyTrace_RETURN:
		debuggerWhat = plCyDebServer::kTraceReturn;
		break;

	case PyTrace_EXCEPTION:
		debuggerWhat = plCyDebServer::kTraceException;
		debServerCallback.SetExceptionInfo(arg); // save off the exception information
		break;

	default:
		assert(!"Invalid what for python trace function");
		return 0; // pretty much ignore if they pass us a bad value
	}

	std::string filename = PyString_AsString(frame->f_code->co_filename);
	int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti) - 1; // python uses base-1 numbering, we use base-0

	// now handle the trace call
	PythonInterface::PythonDebugger()->Trace(debuggerWhat, filename, line, frame->f_tstate->recursion_depth);

	return 0;
}
#endif // PLASMA_EXTERNAL_RELEASE

/////////////////////////////////////////////////////////////////////////////
// A small class that is bound to python so we can redirect stdout

class pyOutputRedirector
{
private:
	std::string fData;
	static bool fTypeCreated;

protected:
	pyOutputRedirector() {}

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptOutputRedirector);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyOutputRedirector object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyOutputRedirector); // converts a PyObject to a pyOutputRedirector (throws error if not correct type)

	void Write(std::string data) {fData += data;}
	void Write(std::wstring data)
	{
		char* strData = hsWStringToString(data.c_str());
		Write(strData);
		delete [] strData;
	}

	// accessor functions for the PyObject*

	// returns the current data stored
	static std::string GetData(PyObject *redirector)
	{
		if (!pyOutputRedirector::Check(redirector))
			return ""; // it's not a redirector object
		pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(redirector);
		return obj->fData;
	}

	// clears the internal buffer out
	static void ClearData(PyObject *redirector)
	{
		if (!pyOutputRedirector::Check(redirector))
			return; // it's not a redirector object
		pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(redirector);
		obj->fData = "";
	}
};

bool pyOutputRedirector::fTypeCreated = false;

// Now for the glue for the redirector
PYTHON_CLASS_DEFINITION(ptOutputRedirector, pyOutputRedirector);

PYTHON_DEFAULT_NEW_DEFINITION(ptOutputRedirector, pyOutputRedirector)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptOutputRedirector)

PYTHON_INIT_DEFINITION(ptOutputRedirector, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptOutputRedirector, write, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "write expects a string or unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->Write(text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		char* text = PyString_AsString(textObj);
		self->fThis->Write(text);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "write expects a string or unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptOutputRedirector)
	PYTHON_METHOD(ptOutputRedirector, write, "Adds text to the output object"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptOutputRedirector, "A class that is used to redirect stdout and stderr");

// required functions for PyObject interoperability
PyObject *pyOutputRedirector::New()
{
	if (!fTypeCreated)
	{
		if (PyType_Ready(&ptOutputRedirector_type) < 0)
			return NULL;
		fTypeCreated = true;
	}
	ptOutputRedirector *newObj = (ptOutputRedirector*)ptOutputRedirector_type.tp_new(&ptOutputRedirector_type, NULL, NULL);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptOutputRedirector, pyOutputRedirector)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptOutputRedirector, pyOutputRedirector)


/////////////////////////////////////////////////////////////////////////////
// A small class that is bound to python so we can redirect stderr

class pyErrorRedirector
{
private:
	static bool fTypeCreated;

	std::string fData;
	bool		fLog;

protected:
	pyErrorRedirector() : fLog(true) {}

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptErrorRedirector);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyOutputRedirector object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyErrorRedirector); // converts a PyObject to a pyOutputRedirector (throws error if not correct type)

	void SetLogging(bool log)
	{
		fLog = log;
	}

	void Write(std::string data)
	{
		PyObject* stdOut = PythonInterface::GetStdOut();

		if (stdOut && pyOutputRedirector::Check(stdOut))
		{
			pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(stdOut);
			obj->Write(data);
		}

		if (fLog)
			fData += data;
	}

	void Write(std::wstring data)
	{
		char* strData = hsWStringToString(data.c_str());
		Write(strData);
		delete [] strData;
	}

	void ExceptHook(PyObject* except, PyObject* val, PyObject* tb)
	{
		PyErr_Display(except, val, tb);

		// Send to the log server
		wchar* wdata = hsStringToWString(fData.c_str());
		NetCliAuthLogPythonTraceback(wdata);
		delete [] wdata;

		if (fLog)
			fData.clear();
	}
};

bool pyErrorRedirector::fTypeCreated = false;

// Now for the glue for the redirector
PYTHON_CLASS_DEFINITION(ptErrorRedirector, pyErrorRedirector);

PYTHON_DEFAULT_NEW_DEFINITION(ptErrorRedirector, pyErrorRedirector)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptErrorRedirector)

PYTHON_INIT_DEFINITION(ptErrorRedirector, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptErrorRedirector, write, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "write expects a string or unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->Write(text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		char* text = PyString_AsString(textObj);
		self->fThis->Write(text);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "write expects a string or unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptErrorRedirector, excepthook, args)
{
	PyObject *exc, *value, *tb;
	if (!PyArg_ParseTuple(args, "OOO", &exc, &value, &tb))
		PYTHON_RETURN_ERROR;

	self->fThis->ExceptHook(exc, value, tb);

	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptErrorRedirector)
	PYTHON_METHOD(ptErrorRedirector, write, "Adds text to the output object"),
	PYTHON_METHOD(ptErrorRedirector, excepthook, "Handles exceptions"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptErrorRedirector, "A class that is used to redirect stdout and stderr");

// required functions for PyObject interoperability
PyObject *pyErrorRedirector::New()
{
	if (!fTypeCreated)
	{
		if (PyType_Ready(&ptErrorRedirector_type) < 0)
			return NULL;
		fTypeCreated = true;
	}
	ptErrorRedirector *newObj = (ptErrorRedirector*)ptErrorRedirector_type.tp_new(&ptErrorRedirector_type, NULL, NULL);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptErrorRedirector, pyErrorRedirector)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptErrorRedirector, pyErrorRedirector)

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : initPython
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the Python dll
//
void PythonInterface::initPython()
{
	// if haven't been initialized then do it
	if ( FirstTimeInit && Py_IsInitialized() == 0 )
	{
		FirstTimeInit = false;
		// initialize the Python stuff
		// let Python do some initialization...
		Py_SetProgramName("plasma");
		Py_Initialize();

#ifndef PLASMA_EXTERNAL_RELEASE
		if (usePythonDebugger)
		{
			debugServer.SetCallbackClass(&debServerCallback);
			debugServer.Init();
			PyEval_SetTrace((Py_tracefunc)PythonTraceCallback, NULL);
		}
#endif

		if (!dbgLog)
		{
			dbgLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "Python.log", 
				plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kTimestamp );
		}

		// create the output redirector for the stdout and stderr file
		stdOut = pyOutputRedirector::New();
		stdErr = pyErrorRedirector::New();

		// if we need the builtins then find the builtin module
		PyObject* sysmod = PyImport_ImportModule("sys");
		// then add the builtin dictionary to our module's dictionary
		// get the sys's dictionary to find the stdout and stderr
		PyObject* sys_dict = PyModule_GetDict(sysmod);
		Py_INCREF(sys_dict);
		if (stdOut != nil)
		{
			if (PyDict_SetItemString(sys_dict,"stdout", stdOut))
				dbgLog->AddLine("Could not redirect stdout, Python output may not appear in the log\n");
		}
		else
			dbgLog->AddLine("Could not create python redirector, Python output will not appear in the log\n");
		
		if (stdErr != nil)
		{
			if (!PyDict_SetItemString(sys_dict,"stderr", stdErr))
			{
				bool dontLog = false;

				// Find the excepthook
				PyObject* stdErrExceptHook = PyObject_GetAttrString(stdErr, "excepthook");
				if (stdErrExceptHook)
				{
					if (!PyCallable_Check(stdErrExceptHook) || PyDict_SetItemString(sys_dict,"excepthook", stdErrExceptHook))
					{
						dbgLog->AddLine("Could not redirect excepthook, Python error output will not get to the log server\n");
						dontLog = true;
					}
					Py_DECREF(stdErrExceptHook);
				}
				else
				{
					dbgLog->AddLine("Could not find stdErr excepthook, Python error output will not get to the log server\n");
					dontLog = true;
				}

				if (dontLog)
				{
					if (pyErrorRedirector::Check(stdErr))
					{
						pyErrorRedirector* redir = pyErrorRedirector::ConvertFrom(stdErr);
						redir->SetLogging(false);
					}
				}
			}
			else
			{
				dbgLog->AddLine("Could not redirect stderr, Python error output may not appear in the log or on the log server\n");
			}
		}
		else
		{
			dbgLog->AddLine("Could not create python redirector, Python error output will not appear in the log\n");
		}

		// NOTE: we will reset the path to not include paths
		// that Python may have found in the registry
		PyObject* path_list = PyList_New(3);
		if (PyList_SetItem(path_list, 0, PyString_FromString(".\\python")))
		{
			Py_DECREF(sys_dict);
			Py_DECREF(path_list);
			dbgLog->AddLine("Error while creating python path:\n");
			getOutputAndReset();
			return;
		}
		// make sure that our plasma libraries are gotten before the system ones
		if (PyList_SetItem(path_list, 1, PyString_FromString(".\\python\\plasma")))
		{
			Py_DECREF(sys_dict);
			Py_DECREF(path_list);
			dbgLog->AddLine("Error while creating python path:\n");
			getOutputAndReset();
			return;
		}
		if (PyList_SetItem(path_list, 2, PyString_FromString(".\\python\\system")))
		{
			Py_DECREF(sys_dict);
			Py_DECREF(path_list);
			dbgLog->AddLine("Error while creating python path:\n");
			getOutputAndReset();
			return;
		}

		// set the path to be this one
		if (PyDict_SetItemString(sys_dict,"path",path_list))
		{
			Py_DECREF(sys_dict);
			Py_DECREF(path_list);
			dbgLog->AddLine("Error while setting python path:\n");
			getOutputAndReset();
			return;
		}

		Py_DECREF(sys_dict);
		Py_DECREF(path_list);

		std::vector<PyMethodDef> methods; // this is temporary, for easy addition of new methods
		AddPlasmaMethods(methods);

		// now copy the data to our real method definition structure
		plasmaMethods = TRACKED_NEW PyMethodDef[methods.size() + 1];
		for (int curMethod = 0; curMethod < methods.size(); curMethod++)
			plasmaMethods[curMethod] = methods[curMethod];
		PyMethodDef terminator = {NULL};
		plasmaMethods[methods.size()] = terminator; // add the terminator

		// now set up the module with the method data
		plasmaMod = Py_InitModule("Plasma", plasmaMethods);
		if (plasmaMod == NULL)
		{
			dbgLog->AddLine("Could not setup the Plasma module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up Plasma:\n");
			getOutputAndReset();
		}
		Py_INCREF(plasmaMod); // make sure python doesn't get rid of it

		AddPlasmaClasses(); // now add the classes to the module
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to Plasma:\n");
			std::string error;
			getOutputAndReset(&error);
		}

		// initialize the PlasmaConstants module
		PyMethodDef noMethods = {NULL};
		plasmaConstantsMod = Py_InitModule("PlasmaConstants", &noMethods); // it has no methods, just values
		if (plasmaConstantsMod == NULL)
		{
			dbgLog->AddLine("Could not setup the PlasmaConstants module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up PlasmaConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}
		Py_INCREF(plasmaConstantsMod);

		AddPlasmaConstantsClasses();

		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to PlasmaConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}

		// initialize the PlasmaNetConstants module
		plasmaNetConstantsMod = Py_InitModule("PlasmaNetConstants", &noMethods); // it has no methods, just values
		if (plasmaNetConstantsMod == NULL)
		{
			dbgLog->AddLine("Could not setup the PlasmaNetConstants module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up PlasmaNetConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}
		Py_INCREF(plasmaNetConstantsMod);

		AddPlasmaNetConstantsClasses();

		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to PlasmaNetConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}

		// initialize the PlasmaVaultConstants module
		plasmaVaultConstantsMod = Py_InitModule("PlasmaVaultConstants", &noMethods); // it has no methods, just values
		if (plasmaVaultConstantsMod == NULL)
		{
			dbgLog->AddLine("Could not setup the PlasmaVaultConstants module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up PlasmaVaultConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}
		Py_INCREF(plasmaVaultConstantsMod);

		AddPlasmaVaultConstantsClasses();

		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to PlasmaVaultConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}

		// setup the global methods for the PlasmaGame module
		methods.clear();
		AddPlasmaGameMethods(methods);

		// now copy the data to our real method definition structure
		plasmaGameMethods = TRACKED_NEW PyMethodDef[methods.size() + 1];
		for (int curMethod = 0; curMethod < methods.size(); curMethod++)
			plasmaGameMethods[curMethod] = methods[curMethod];
		plasmaGameMethods[methods.size()] = terminator; // add the terminator

		// initialize the PlasmaGame module
		plasmaGameMod = Py_InitModule("PlasmaGame", plasmaGameMethods);
		if (plasmaGameMod == NULL)
		{
			dbgLog->AddLine("Could not setup the PlasmaGame module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up PlasmaGame:\n");
			std::string error;
			getOutputAndReset(&error);
		}
		Py_INCREF(plasmaGameMod);

		AddPlasmaGameClasses();

		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to PlasmaGame:\n");
			std::string error;
			getOutputAndReset(&error);
		}

		// initialize the PlasmaGameConstants module
		plasmaGameConstantsMod = Py_InitModule("PlasmaGameConstants", &noMethods); // it has no methods, just values
		if (plasmaGameConstantsMod == NULL)
		{
			dbgLog->AddLine("Could not setup the PlasmaGameConstants module\n");
			return;
		}
		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while setting up PlasmaGameConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}
		Py_INCREF(plasmaGameConstantsMod);

		AddPlasmaGameConstantsClasses();

		if (PyErr_Occurred())
		{
			dbgLog->AddLine("Python error while adding classes to PlasmaGameConstants:\n");
			std::string error;
			getOutputAndReset(&error);
		}
	}
	initialized++;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : initDebugInterface
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the Python to Plasma 
//
void PythonInterface::initDebugInterface()
{
	if ( !debug_initialized )
	{
		// bring up the debug window
		dbgMod = PyImport_ImportModule("cydebug");
		// was there a debug module?
		if ( dbgMod != nil )
		{
			PyObject *dict;
			// get the dictionary for this module
			dict = PyModule_GetDict(dbgMod);
			dbgOut = PyDict_GetItemString(dict, "writeout");
			dbgSlice = PyDict_GetItemString(dict, "timeslice");
		}
	}
	debug_initialized = true;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaMethods
//  PARAMETERS : none
//
//  PURPOSE    : Add global methods to the Plasma module
//
void PythonInterface::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	cyMisc::AddPlasmaMethods(methods);
	cyAvatar::AddPlasmaMethods(methods);
	cyAccountManagement::AddPlasmaMethods(methods);

	pyDrawControl::AddPlasmaMethods(methods);
	pyGUIDialog::AddPlasmaMethods(methods);
	pyImage::AddPlasmaMethods(methods);
	pyJournalBook::AddPlasmaMethods(methods);
	pySDLModifier::AddPlasmaMethods(methods);
	pySpawnPointInfo::AddPlasmaMethods(methods);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaClasses
//  PARAMETERS : none
//
//  PURPOSE    : Add classes to the Plasma module
//
void PythonInterface::AddPlasmaClasses()
{
	pyKey::AddPlasmaClasses(plasmaMod);
	pySceneObject::AddPlasmaClasses(plasmaMod);

	pyAgeInfoStruct::AddPlasmaClasses(plasmaMod);
	pyAgeInfoStructRef::AddPlasmaClasses(plasmaMod);
	pyAgeLinkStruct::AddPlasmaClasses(plasmaMod);
	pyAgeLinkStructRef::AddPlasmaClasses(plasmaMod);
	pySpawnPointInfo::AddPlasmaClasses(plasmaMod);
	pySpawnPointInfoRef::AddPlasmaClasses(plasmaMod);

	pyColor::AddPlasmaClasses(plasmaMod);
	pyMatrix44::AddPlasmaClasses(plasmaMod);
	pyPoint3::AddPlasmaClasses(plasmaMod);
	pyVector3::AddPlasmaClasses(plasmaMod);

	cyAnimation::AddPlasmaClasses(plasmaMod);
	cyAvatar::AddPlasmaClasses(plasmaMod);
	cyCamera::AddPlasmaClasses(plasmaMod);
	cyDraw::AddPlasmaClasses(plasmaMod);
	cyInputInterface::AddPlasmaClasses(plasmaMod);
	cyParticleSys::AddPlasmaClasses(plasmaMod);
	cyPhysics::AddPlasmaClasses(plasmaMod);

	pyAudioControl::AddPlasmaClasses(plasmaMod);
	pyCluster::AddPlasmaClasses(plasmaMod);
	pyDniCoordinates::AddPlasmaClasses(plasmaMod);
	pyDniInfoSource::AddPlasmaClasses(plasmaMod);
	pyDynamicText::AddPlasmaClasses(plasmaMod);
	pyImage::AddPlasmaClasses(plasmaMod);
	pyJournalBook::AddPlasmaClasses(plasmaMod);
	pyKeyMap::AddPlasmaClasses(plasmaMod);
	pyMarkerMgr::AddPlasmaClasses(plasmaMod);
	pyMoviePlayer::AddPlasmaClasses(plasmaMod);
	pyNetLinkingMgr::AddPlasmaClasses(plasmaMod);
	pyNotify::AddPlasmaClasses(plasmaMod);
	pyPlayer::AddPlasmaClasses(plasmaMod);
	pyStatusLog::AddPlasmaClasses(plasmaMod);
	pyStream::AddPlasmaClasses(plasmaMod);
	pySwimCurrentInterface::AddPlasmaClasses(plasmaMod);
	pyWaveSet::AddPlasmaClasses(plasmaMod);

	// SDL
	pySDLModifier::AddPlasmaClasses(plasmaMod);
	pySDLStateDataRecord::AddPlasmaClasses(plasmaMod);
	pySimpleStateVariable::AddPlasmaClasses(plasmaMod);

	// GUI objects
	pyGUIDialog::AddPlasmaClasses(plasmaMod);
	pyGUISkin::AddPlasmaClasses(plasmaMod);
	pyGUIPopUpMenu::AddPlasmaClasses(plasmaMod);
	// GUI base classes
	pyGUIControl::AddPlasmaClasses(plasmaMod);
	pyGUIControlValue::AddPlasmaClasses(plasmaMod);
	// GUI derived classes
	pyGUIControlButton::AddPlasmaClasses(plasmaMod);
	pyGUIControlCheckBox::AddPlasmaClasses(plasmaMod);
	pyGUIControlClickMap::AddPlasmaClasses(plasmaMod);
	pyGUIControlDragBar::AddPlasmaClasses(plasmaMod);
	pyGUIControlDraggable::AddPlasmaClasses(plasmaMod);
	pyGUIControlDynamicText::AddPlasmaClasses(plasmaMod);
	pyGUIControlEditBox::AddPlasmaClasses(plasmaMod);
	pyGUIControlKnob::AddPlasmaClasses(plasmaMod);
	pyGUIControlListBox::AddPlasmaClasses(plasmaMod);
	pyGUIControlMultiLineEdit::AddPlasmaClasses(plasmaMod);
	pyGUIControlProgress::AddPlasmaClasses(plasmaMod);
	pyGUIControlRadioGroup::AddPlasmaClasses(plasmaMod);
	pyGUIControlTextBox::AddPlasmaClasses(plasmaMod);
	pyGUIControlUpDownPair::AddPlasmaClasses(plasmaMod);

	// Vault objects
	pyAgeVault::AddPlasmaClasses(plasmaMod);
	pyVault::AddPlasmaClasses(plasmaMod);
	// Vault node base classes
	pyVaultNode::AddPlasmaClasses(plasmaMod);
	pyVaultNodeRef::AddPlasmaClasses(plasmaMod);
	pyVaultFolderNode::AddPlasmaClasses(plasmaMod);
	// Vault node derived classes
	pyVaultAgeInfoListNode::AddPlasmaClasses(plasmaMod);
	pyVaultAgeInfoNode::AddPlasmaClasses(plasmaMod);
	pyVaultAgeLinkNode::AddPlasmaClasses(plasmaMod);
	pyVaultChronicleNode::AddPlasmaClasses(plasmaMod);
	pyVaultImageNode::AddPlasmaClasses(plasmaMod);
	pyVaultMarkerGameNode::AddPlasmaClasses(plasmaMod);
	pyVaultPlayerInfoListNode::AddPlasmaClasses(plasmaMod);
	pyVaultPlayerInfoNode::AddPlasmaClasses(plasmaMod);
	pyVaultSDLNode::AddPlasmaClasses(plasmaMod);
	pyVaultSystemNode::AddPlasmaClasses(plasmaMod);
	pyVaultTextNoteNode::AddPlasmaClasses(plasmaMod);
	
	// Shaders
	pyGrassShader::AddPlasmaClasses(plasmaMod);

	// Game Scores
	pyScoreMgr::AddPlasmaClasses(plasmaMod);
	pyGameScore::AddPlasmaClasses(plasmaMod);

	// AI
	pyCritterBrain::AddPlasmaClasses(plasmaMod);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaConstants module
//
void PythonInterface::AddPlasmaConstantsClasses()
{
	pyEnum::AddPlasmaConstantsClasses(plasmaConstantsMod);

	cyAvatar::AddPlasmaConstantsClasses(plasmaConstantsMod);
	cyMisc::AddPlasmaConstantsClasses(plasmaConstantsMod);
	cyAccountManagement::AddPlasmaConstantsClasses(plasmaConstantsMod);
	
	//pyDrawControl::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyDynamicText::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyGUIControlButton::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyGUIControlMultiLineEdit::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyJournalBook::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyMarkerMgr::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyMoviePlayer::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyNotify::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pySDL::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyStatusLog::AddPlasmaConstantsClasses(plasmaConstantsMod);
	pyScoreMgr::AddPlasmaConstantsClasses(plasmaConstantsMod);

	pyAIMsg::AddPlasmaConstantsClasses(plasmaConstantsMod);

	// TODO: put these constants here. remove them from below.
	//pyNetLinkingMgr::AddPlasmaConstantsClasses(plasmaConstantsMod);
	//pyVault::AddPlasmaConstantsClasses(plasmaConstantsMod);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaNetConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaNetConstants module
//
void PythonInterface::AddPlasmaNetConstantsClasses()
{
	pyNetLinkingMgr::AddPlasmaConstantsClasses(plasmaNetConstantsMod);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaVaultConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaVaultConstants module
//
void PythonInterface::AddPlasmaVaultConstantsClasses()
{
	pyVault::AddPlasmaConstantsClasses(plasmaVaultConstantsMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaGameMethods
//  PARAMETERS : none
//
//  PURPOSE    : Add global methods to the PlasmaGame module
//
void PythonInterface::AddPlasmaGameMethods(std::vector<PyMethodDef> &methods)
{
	// General
	pyGameCli::AddPlasmaMethods(methods);

	// TicTacToe game
	pyTTTGame::AddPlasmaMethods(methods);

	// Heek game
	pyHeekGame::AddPlasmaMethods(methods);

	// Marker game
	pyMarkerGame::AddPlasmaMethods(methods);

	// Blue Spiral game
	pyBlueSpiralGame::AddPlasmaMethods(methods);

	// Climbing Wall game
	pyClimbingWallGame::AddPlasmaMethods(methods);

	// Variable Sync game
	pyVarSyncGame::AddPlasmaMethods(methods);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaGameClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaGame module
//
void PythonInterface::AddPlasmaGameClasses()
{
	// General
	pyGameMgrMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameMgrInviteReceivedMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameMgrInviteRevokedMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCliMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCliPlayerJoinedMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCliPlayerLeftMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCliInviteFailedMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCliOwnerChangeMsg::AddPlasmaClasses(plasmaGameMod);
	pyGameCli::AddPlasmaClasses(plasmaGameMod);

	// TicTacToe game
	pyTTTMsg::AddPlasmaClasses(plasmaGameMod);
	pyTTTGameStartedMsg::AddPlasmaClasses(plasmaGameMod);
	pyTTTGameOverMsg::AddPlasmaClasses(plasmaGameMod);
	pyTTTMoveMadeMsg::AddPlasmaClasses(plasmaGameMod);
	pyTTTGame::AddPlasmaClasses(plasmaGameMod);

	// Heek game
	pyHeekMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekPlayGameMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekGoodbyeMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekWelcomeMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekDropMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekSetupMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekLightStateMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekInterfaceStateMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekCountdownStateMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekWinLoseMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekGameWinMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekPointUpdateMsg::AddPlasmaClasses(plasmaGameMod);
	pyHeekGame::AddPlasmaClasses(plasmaGameMod);

	// Marker game
	pyMarkerMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerTemplateCreatedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerTeamAssignedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameTypeMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameStartedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGamePausedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameResetMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameOverMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameNameChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerTimeLimitChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGameDeletedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerMarkerAddedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerMarkerDeletedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerMarkerNameChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerMarkerCapturedMsg::AddPlasmaClasses(plasmaGameMod);
	pyMarkerGame::AddPlasmaClasses(plasmaGameMod);

	// Blue Spiral game
	pyBlueSpiralMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralClothOrderMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralSuccessfulHitMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralGameWonMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralGameOverMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralGameStartedMsg::AddPlasmaClasses(plasmaGameMod);
	pyBlueSpiralGame::AddPlasmaClasses(plasmaGameMod);

	// Climbing Wall game
	pyClimbingWallMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallNumBlockersChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallReadyMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallBlockersChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallPlayerEnteredMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallSuitMachineLockedMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallGameOverMsg::AddPlasmaClasses(plasmaGameMod);
	pyClimbingWallGame::AddPlasmaClasses(plasmaGameMod);

	// Variable Sync game
	pyVarSyncMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncStringVarChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncNumericVarChangedMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncAllVarsSentMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncStringVarCreatedMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncNumericVarCreatedMsg::AddPlasmaClasses(plasmaGameMod);
	pyVarSyncGame::AddPlasmaClasses(plasmaGameMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaGameConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaGameConstants module
//
void PythonInterface::AddPlasmaGameConstantsClasses()
{
	// General
	pyGameMgrMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyGameCliMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyGameCliInviteFailedMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// TicTacToe game
	pyTTTMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyTTTGame::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// Heek game
	pyHeekMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyHeekLightStateMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyHeekCountdownStateMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyHeekGame::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// Marker game
	pyMarkerMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyMarkerGame::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// Blue Spiral game
	pyBlueSpiralMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// Climbing Wall game
	pyClimbingWallMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
	pyClimbingWallGame::AddPlasmaConstantsClasses(plasmaGameConstantsMod);

	// Variable Sync game
	pyVarSyncMsg::AddPlasmaConstantsClasses(plasmaGameConstantsMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : finiPython
//  PARAMETERS : none
//
//  PURPOSE    : Finalize the Python dll, ie. get ready to shut down
//
void PythonInterface::finiPython()
{
	// decrement the number of initializations, on last one do the finalization
	initialized--;
	if ( initialized < 1 && Py_IsInitialized() != 0 && IsInShutdown )
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		if (usePythonDebugger)
			debugServer.Disconnect();
#endif
		// remove debug module if used
		if ( dbgMod )
		{
			Py_DECREF(dbgMod);
			dbgMod = nil;
		}

		if ( stdOut )
		{
			Py_DECREF(stdOut);
			stdOut = nil;
		}

		if ( stdErr )
		{
			Py_DECREF(stdErr);
			stdErr = nil;
		}

		if ( plasmaMod )
		{
			Py_DECREF(plasmaMod);	// get rid of our reference
			plasmaMod = nil;
		}

		if ( plasmaConstantsMod )
		{
			Py_DECREF(plasmaConstantsMod);
			plasmaConstantsMod = nil;
		}

		if ( plasmaNetConstantsMod )
		{
			Py_DECREF(plasmaNetConstantsMod);
			plasmaNetConstantsMod = nil;
		}

		if ( plasmaVaultConstantsMod )
		{
			Py_DECREF(plasmaVaultConstantsMod);
			plasmaVaultConstantsMod = nil;
		}

		if ( plasmaGameMod )
		{
			Py_DECREF(plasmaGameMod);
			plasmaGameMod = nil;
		}

		if ( plasmaGameConstantsMod )
		{
			Py_DECREF(plasmaGameConstantsMod);
			plasmaGameConstantsMod = nil;
		}

		// let Python clean up after itself
		Py_Finalize();

		if (plasmaMethods)
		{
			delete [] plasmaMethods;
			plasmaMethods = nil;
		}

		if (plasmaGameMethods)
		{
			delete [] plasmaGameMethods;
			plasmaGameMethods = nil;
		}

		// close done the log file, if we created one
		if ( dbgLog != nil )
		{
			delete dbgLog;
			dbgLog = nil;
		}

		initialized = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : debugTimeSlice
//  PARAMETERS : none
//
//  PURPOSE    : give the debug window a time slice
//
void PythonInterface::debugTimeSlice()
{
	// check to see if the debug python module is loaded
	if ( dbgSlice != nil )
	{
		// then send it the new text
		PyObject* retVal = PyObject_CallFunction(dbgSlice,nil);
		if ( retVal == nil )
		{
			// for some reason this function didn't, remember that and not call it again
			dbgSlice = nil;
			// if there was an error make sure that the stderr gets flushed so it can be seen
			PyErr_Print();		// make sure the error is printed
			PyErr_Clear();		// clear the error
		}
		Py_XDECREF(retVal);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetStdOut
//  PARAMETERS : none
//
//  PURPOSE    : get the stdOut python object
//
PyObject* PythonInterface::GetStdOut()
{
	return stdOut;
}

PyObject* PythonInterface::GetStdErr()
{
	return stdErr;
}
	
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : getOutputAndReset
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the error file to be displayed
//
int PythonInterface::getOutputAndReset(std::string *output)
{
	if (stdOut != nil)
	{
		std::string strVal = pyOutputRedirector::GetData(stdOut);
		int size = strVal.length();
		dbgLog->AddLine(strVal.c_str());

		// reset the file back to zero
		pyOutputRedirector::ClearData(stdOut);

		// tell python debugger
#ifndef PLASMA_EXTERNAL_RELEASE
		if (UsePythonDebugger())
			PythonInterface::PythonDebugger()->StdOut(strVal);
#endif

		// check to see if the debug python module is loaded
		if ( dbgOut != nil )
		{
			// then send it the new text
			PyObject* retVal = PyObject_CallFunction(dbgOut,"s",strVal.c_str());
			if ( retVal == nil )
			{
				// for some reason this function didn't, remember that and not call it again
				dbgOut = nil;
				// if there was an error make sure that the stderr gets flushed so it can be seen
				PyErr_Print();		// make sure the error is printed
				PyErr_Clear();		// clear the error
			}
			Py_XDECREF(retVal);
		}

		if (output)
			(*output) = strVal;
		return size;
	}
	return 0;
}

void PythonInterface::WriteToLog(const char* text)
{
	dbgLog->AddLine(text);
}

void PythonInterface::WriteToStdErr(const char* text)
{
	PyObject* stdErr = PythonInterface::GetStdErr();
	if (stdErr && pyErrorRedirector::Check(stdErr))
	{
		pyErrorRedirector *obj = pyErrorRedirector::ConvertFrom(stdErr);
		obj->Write(text);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindModule
//  PARAMETERS : module    - module name to find
//
//  PURPOSE    : Find module. If it doesn't exist then don't create, return nil.
//
PyObject* PythonInterface::FindModule(char* module)
{
	PyObject *m;
	// first we must get rid of any old modules of the same name, we'll replace it
	PyObject *modules = PyImport_GetModuleDict();
	if ((m = PyDict_GetItemString(modules, module)) != NULL && PyModule_Check(m))
		// just return what we found
		return m;

	// couldn't find the module, return None (sorta)
	return nil;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsModuleNameUnique
//  PARAMETERS : module    - module name to create
//
//  PURPOSE    : Test to see if the module name is unique
//
//  Returns    : True if unique , otherwise returns False
//
hsBool PythonInterface::IsModuleNameUnique(char* module)
{
	PyObject *m;
	// first we must get rid of any old modules of the same name, we'll replace it
	PyObject *modules = PyImport_GetModuleDict();
	if ((m = PyDict_GetItemString(modules, module)) != NULL && PyModule_Check(m))
	{
		return false;
	}
	return true;
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
	{
		// clear it
		char message[256];
		sprintf(message,"ERROR! Creating a python module of the same name - %s",module);
		hsAssert(false,message);
		_PyModule_Clear(m);
	}

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
//  Function   : GetPlasmaItem
//  PARAMETERS : item    - what item in the plasma module to get
//
//  PURPOSE    : get an item (probably a function) from the Plasma module
//
PyObject* PythonInterface::GetPlasmaItem(char* item)
{
	if ( plasmaMod )
	{
		PyObject* d = PyModule_GetDict(plasmaMod);
		return PyDict_GetItemString(d, item);
	}
	return nil;
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


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : CheckModuleForFunctions
//  PARAMETERS : module    - module to check for
//
//  PURPOSE    : checks to see if a specific function is defined in this module
//
void PythonInterface::CheckModuleForFunctions(PyObject* module, char** funcNames, PyObject** funcTable)
{
	PyObject *dict;
	// get the dictionary for this module
	dict = PyModule_GetDict(module);
	// start looking for the functions
	int i=0;
	while ( funcNames[i] != nil )
	{
		PyObject* func = PyDict_GetItemString(dict, funcNames[i]);
		if ( func != NULL && PyCallable_Check(func)>0 )
		{
			// if it is defined then mark the funcTable
			funcTable[i] = func;
		}
		else	// else we couldn't find the funtion
		{
			funcTable[i] = nil;
		}
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : CheckInstanceForFunctions
//  PARAMETERS : instance    - instance of a class to check
//
//  PURPOSE    : checks to see if a specific function is defined in this instance of a class
//             : and will fill out the funcTable with object instances of where the funciton is
//
void PythonInterface::CheckInstanceForFunctions(PyObject* instance, char** funcNames, PyObject** funcTable)
{
	// start looking for the functions
	int i=0;
	while ( funcNames[i] != nil )
	{
		PyObject* func = PyObject_GetAttrString(instance, funcNames[i]);
		if ( func != NULL )
		{
			if ( PyCallable_Check(func)>0 )
			{
				// if it is defined then mark the funcTable
				funcTable[i] = instance;
			}
			Py_DECREF(func);
		}
		i++;
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
//  Function   : LoadObject
//  PARAMETERS : pickle    - the pickled object in char string form
//             : size      - size of the guts to load into an object
//
//  PURPOSE    : Load a python object from a pickled object
//
PyObject* PythonInterface::LoadObject(char* pickle, Int32 size)
{
	return PyMarshal_ReadObjectFromString(pickle, size);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunStringInteractive
//  PARAMETERS : command   - string of commands to execute in the...
//             : module    - module name to run 'command' in
//
//  PURPOSE    : run a python string in a specific module name
//
//  RETURNS    : pointer to PyObject that is the result of the command
//
hsBool PythonInterface::RunStringInteractive(char *command, PyObject* module)
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
	v = PyRun_String(command, Py_single_input, d, d);
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
//  Function   : RunString
//  PARAMETERS : command   - string of commands to execute in the...
//             : module    - module name to run 'command' in
//
//  PURPOSE    : run a python string in a specific module name
//
hsBool PythonInterface::RunString(char *command, PyObject* module)
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
	v = PyRun_String(command, Py_file_input, d, d);
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
//  Function   : GetpyKeyFromPython
//  PARAMETERS : pkey      - python object that is a pyKey (ptKey) class
//
//  PURPOSE    : turn a PyObject* into a pyKey*
//
pyKey* PythonInterface::GetpyKeyFromPython(PyObject* pkey)
{
	if (!pyKey::Check(pkey))
		return nil;
	return pyKey::ConvertFrom(pkey);
}