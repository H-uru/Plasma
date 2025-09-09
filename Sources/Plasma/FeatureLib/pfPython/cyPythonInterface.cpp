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
//////////////////////////////////////////////////////////////////////
//
// PythonInterface   - The Python interface to the Python dll
//
// NOTE: Eventually, this will be made into a separate dll, because there should
//       only be one instance of this interface. 
//

#include <functional>
#include <string_theory/string>
#include <string_theory/string_stream>

#include <Python.h>
#include <marshal.h>
#include "pyGeometry3.h"
#include "pyKey.h"
#include "pyMatrix44.h"
#include "pyObjectRef.h"

// CPython specific init stuff
#include <cpython/initconfig.h>
#include <pylifecycle.h>

#include "plPythonCallable.h"
#include "cyPythonInterface.h"

#include "pyEnum.h"
#include "cyDraw.h"
#include "cyParticleSys.h"
#include "cyPhysics.h"
#include "pySceneObject.h"
#include "cyMisc.h"
#include "cyCamera.h"
#include "pyNotify.h"
#include "cyAvatar.h"
#include "pyColor.h"
#include "pyDynamicText.h"
#include "cyAnimation.h"
#include "pyPlayer.h"
#include "pyImage.h"
#include "pyDniCoordinates.h"
#include "cyInputInterface.h"
#include "pySDL.h"
#include "cyAccountManagement.h"

// GameMgr
#include "pyGameCli.h"
#include "pyGameMgr.h"
#include "pyGmBlueSpiral.h"
#include "pyGmMarker.h"
#include "pyGmVarSync.h"

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
#include "plNetClientComm/plNetClientComm.h"
#include "plStatusLog/plStatusLog.h"

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

// audio setting stuff
#include "pyAudioControl.h"

// spawn point def
#include "pySpawnPointInfo.h"

#include "pyMarkerMgr.h"
#include "pyStatusLog.h"

// Guess what this is for :P
#include "pyJournalBook.h"

#include "pyKeyMap.h"
#include "pyImageLibMod.h"
#include "pyLayer.h"
#include "pyStream.h"

#include "pyMoviePlayer.h"
#include "pyDrawControl.h"

#include "pyWaveSet.h"
#include "pySwimCurrentInterface.h"

#include "pyCluster.h"
#include "pyGrassShader.h"

#include "pyGameScore.h"
#include "pyGameScoreMsg.h"

#include "pyCritterBrain.h"

int32_t PythonInterface::initialized = 0;                 // only need to initialize all of Python once
bool    PythonInterface::FirstTimeInit = true;           // start with "this is the first time"
bool    PythonInterface::IsInShutdown = false;           // whether we are _really_ in shutdown mode

PyObject* PythonInterface::stdOut = nullptr;            // python object of the stdout file
PyObject* PythonInterface::stdErr = nullptr;            // python object of the err file

PyObject* PythonInterface::builtInModuleName = nullptr;

bool      PythonInterface::debug_initialized = false;   // has the debug been initialized yet?
PyObject* PythonInterface::dbgMod = nullptr;            // display module for stdout and stderr
PyObject* PythonInterface::dbgOut = nullptr;
PyObject* PythonInterface::dbgSlice = nullptr;          // time slice function for the debug window
plStatusLog* PythonInterface::dbgLog = nullptr;         // output logfile

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
bool PythonInterface::usePythonDebugger = false;
plCyDebServer PythonInterface::debugServer;
bool PythonInterface::requestedExit = false;
#endif

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
// Special includes for debugging
#include <string>
#include <vector>
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

    if (PyErr_Occurred() == nullptr)
        return error; // no error occurred

    PyObject* errType = nullptr;
    PyObject* errVal = nullptr;
    PyObject* errTraceback = nullptr;
    PyErr_Fetch(&errType, &errVal, &errTraceback); // clears the error flag
    PyErr_NormalizeException(&errType, &errVal, &errTraceback);

    if (PyErr_GivenExceptionMatches(errType, PyExc_SyntaxError))
    {
        // we know how to parse out information from syntax errors
        PyObject* message;
        char* filename = nullptr;
        int lineNumber = 0;
        int offset = 0;
        char* text = nullptr;

        if (PyTuple_Check(errVal))
        {
            // nested tuple, parse out the error information
            PyArg_Parse(errVal, "(O(ziiz))", &message, &filename, &lineNumber, &offset, &text);
            error += PyUnicode_AsUTF8(message);
            if (text)
                error += text;
        }
        else
        {
            // probably just the error class, retrieve the message and text directly
            PyObject* v;
            if ((v = PyObject_GetAttrString(errVal, "msg")))
            {
                error += PyUnicode_AsUTF8(v);
                Py_DECREF(v);
            }
            if ((v == PyObject_GetAttrString(errVal, "text")))
            {
                if (v != Py_None)
                    error += PyUnicode_AsUTF8(v);
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
            error += PyUnicode_AsUTF8(className);
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
        std::string filename = PyUnicode_AsUTF8(curFrame->f_code->co_filename);
        int lineNumber = PyCode_Addr2Line(curFrame->f_code, curFrame->f_lasti); // python uses base-1 numbering, we use base-0, but for display we want base-1
        std::string functionName = PyUnicode_AsUTF8(curFrame->f_code->co_name);

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
                    std::string arg = PyUnicode_AsUTF8(argName);
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
                            functionName += PyUnicode_AsUTF8(PyObject_Str(val));
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

                std::string keyStr = PyUnicode_AsUTF8(PyObject_Str(key));
                if (keyStr == "builtins")
                    continue; // skip builtins

                bool addQuotes = PyUnicode_Check(value);

                std::string valueStr = "";
                if (addQuotes)
                    valueStr += "\"";
                valueStr += PyUnicode_AsUTF8(PyObject_Str(value));
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

                std::string keyStr = PyUnicode_AsUTF8(PyObject_Str(key));
                if (keyStr == "builtins")
                    continue; // skip builtins

                bool addQuotes = PyUnicode_Check(value);

                std::string valueStr = "";
                if (addQuotes)
                    valueStr += "\"";
                valueStr += PyUnicode_AsUTF8(PyObject_Str(value));
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
                retVal = PyUnicode_AsUTF8(reprObj);
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
    debServerCallback.SetExceptionInfo(nullptr);

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

    std::string filename = PyUnicode_AsUTF8(frame->f_code->co_filename);
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
    ST::string_stream fData;
    static bool fTypeCreated;

protected:
    pyOutputRedirector() {}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptOutputRedirector);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyOutputRedirector object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyOutputRedirector); // converts a PyObject to a pyOutputRedirector (throws error if not correct type)

    void Write(const ST::string& data) {fData << data;}

    void Flush()
    {
        // Currently a no-op - the actual printing happens externally
        // and is not controlled by the Python side.
    }

    // accessor functions for the PyObject*

    // returns the current data stored
    static ST::string GetData(PyObject *redirector)
    {
        if (!pyOutputRedirector::Check(redirector))
            return {}; // it's not a redirector object
        pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(redirector);
        return obj->fData.to_string();
    }

    // clears the internal buffer out
    static void ClearData(PyObject *redirector)
    {
        if (!pyOutputRedirector::Check(redirector))
            return; // it's not a redirector object
        pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(redirector);
        obj->fData.erase(SIZE_MAX);
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
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "write expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Write(text);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptOutputRedirector, flush)
{
    self->fThis->Flush();
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptOutputRedirector)
    PYTHON_METHOD(ptOutputRedirector, write, "Adds text to the output object"),
    PYTHON_METHOD(ptOutputRedirector, flush, "Flushes the output (currently a no-op)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptOutputRedirector, "A class that is used to redirect stdout and stderr");

// required functions for PyObject interoperability
PyObject *pyOutputRedirector::New()
{
    if (!fTypeCreated)
    {
        if (PyType_Ready(&ptOutputRedirector_type) < 0)
            return nullptr;
        fTypeCreated = true;
    }
    ptOutputRedirector *newObj = (ptOutputRedirector*)ptOutputRedirector_type.tp_new(&ptOutputRedirector_type, nullptr, nullptr);
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

    ST::string_stream fData;
    bool        fLog;

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

    void Write(const ST::string& data)
    {
        PyObject* stdOut = PythonInterface::GetStdOut();

        if (stdOut && pyOutputRedirector::Check(stdOut))
        {
            pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(stdOut);
            obj->Write(data);
        }

        if (fLog)
            fData << data;
    }

    void ExceptHook(PyObject* except, PyObject* val, PyObject* tb)
    {
        PyErr_Display(except, val, tb);

        // Send to the log server
        NetCommLogPythonTraceback(fData.to_string());

        if (fLog)
            fData.erase(SIZE_MAX);
    }

    void Flush()
    {
        PyObject* stdOut = PythonInterface::GetStdOut();

        if (stdOut && pyOutputRedirector::Check(stdOut))
        {
            pyOutputRedirector *obj = pyOutputRedirector::ConvertFrom(stdOut);
            obj->Flush();
        }
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
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "write expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Write(text);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptErrorRedirector, excepthook, args)
{
    PyObject *exc, *value, *tb;
    if (!PyArg_ParseTuple(args, "OOO", &exc, &value, &tb))
        PYTHON_RETURN_ERROR;

    self->fThis->ExceptHook(exc, value, tb);

    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptErrorRedirector, flush)
{
    self->fThis->Flush();
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptErrorRedirector)
    PYTHON_METHOD(ptErrorRedirector, write, "Adds text to the output object"),
    PYTHON_METHOD(ptErrorRedirector, excepthook, "Handles exceptions"),
    PYTHON_METHOD(ptErrorRedirector, flush, "Flushes the output (currently a no-op)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptErrorRedirector, "A class that is used to redirect stdout and stderr");

// required functions for PyObject interoperability
PyObject *pyErrorRedirector::New()
{
    if (!fTypeCreated)
    {
        if (PyType_Ready(&ptErrorRedirector_type) < 0)
            return nullptr;
        fTypeCreated = true;
    }
    ptErrorRedirector *newObj = (ptErrorRedirector*)ptErrorRedirector_type.tp_new(&ptErrorRedirector_type, nullptr, nullptr);
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

static void IInitBuiltinModule(const char* modName, const char* docstring, plStatusLog* dbgLog,
                               const std::function<void(PyObject*)>& classFunc=nullptr,
                               const std::function<void(PyObject*)>& methodFunc=nullptr)
{
    // Steals a reference from the modules dict--not a memory leak!
    PyObject* m = PyImport_AddModule(modName);
    if (!m) {
        dbgLog->AddLineF("Could not create the '{}' module.", modName);
        return;
    }

    if (PyModule_SetDocString(m, docstring) != 0) {
        dbgLog->AddLineF("Error setting the docstring for the '{}' module.", modName);
        return;
    }

    if (methodFunc) {
        methodFunc(m);
        if (PyErr_Occurred()) {
            dbgLog->AddLineF("Python error while adding methods to {}:", modName);
            PythonInterface::getOutputAndReset();
            return;
        }
    }

    if (classFunc) {
        classFunc(m);
        if (PyErr_Occurred()) {
            dbgLog->AddLineF("Python error while adding classes to {}:", modName);
            PythonInterface::getOutputAndReset();
        }
    }
}

template<typename _ConfigT, PyStatus(*_FuncT)(const _ConfigT*), void(*_ClearT)(_ConfigT*) = nullptr>
static bool ICheckedInit(_ConfigT& config, plStatusLog* dbgLog, const char* errmsg)
{
    PyStatus status = _FuncT(&config);
    if (PyStatus_Exception(status)) {
        dbgLog->AddLineF(plStatusLog::kRed, "Python {} {}!", PY_VERSION, errmsg);
        if (status.func)
            dbgLog->AddLineF(plStatusLog::kRed, "{}: {}", status.func, status.err_msg);
        else
            dbgLog->AddLine(plStatusLog::kRed, status.err_msg);

        // Dammit GCC (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94554)
        typedef void (*ClearT_Type)(_ConfigT*);
        using ClearT_Null = std::integral_constant<ClearT_Type, nullptr>;
        using ClearT_Param = std::integral_constant<ClearT_Type, _ClearT>;
        if constexpr (!std::is_same_v<ClearT_Null, ClearT_Param>)
            _ClearT(&config);
        return false;
    }
    return true;
}

void PythonInterface::initPython()
{
    // if haven't been initialized then do it
    if (!FirstTimeInit || Py_IsInitialized())
        return;

    if (!dbgLog) {
        dbgLog = plStatusLogMgr::GetInstance().CreateStatusLog(30, "Python.log", 
                                                               plStatusLog::kFilledBackground |
                                                               plStatusLog::kAlignToTop |
                                                               plStatusLog::kTimestamp);
    }

    FirstTimeInit = false;

    // In Python 2, we could rely on a single PEP 302 hook class installed into sys.path_hooks to
    // handle importing modules from python.pak -- this could be initialized after Python. In Python 3,
    // however, the initialization process imports the encodings module and dies if it is not available.
    // This module is written in Python code and found in python.pak, but the python.pak import machinery
    // is not available. If you have Python 3.(whatever) installed locally and are using a DLL, it
    // works. If you violate either of those cases, plClient silently exits (if you're not watching
    // stderr). To fix this, will use the provisional core/main init split introduced in Python 3.8
    // and described in PEPs 432 and 587 to init the "core" (much like _freeze_importlib) and install
    // our PEP 451 import machinery. Then, we'll do the whole main init thingo.
    PyPreConfig preConfig;
    PyPreConfig_InitIsolatedConfig(&preConfig);
    if (!ICheckedInit<PyPreConfig, Py_PreInitialize>(preConfig, dbgLog, "Pre-init failed!"))
        return;

    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);
    config.site_import = 0;
    PyConfig_SetString(&config, &config.program_name, L"plasma");
    config._init_main = 0;

    // Allow importing from the local python directory if and only if this is an internal client.
#ifndef PLASMA_EXTERNAL_RELEASE
    PyWideStringList_Append(&config.module_search_paths, L"./python");
    PyWideStringList_Append(&config.module_search_paths, L"./python/plasma");
    PyWideStringList_Append(&config.module_search_paths, L"./python/system");
    PyWideStringList_Append(&config.module_search_paths, L"./python/system/lib-dynload");
    config.module_search_paths_set = 1;
#endif

    if (!ICheckedInit<PyConfig, Py_InitializeFromConfig, PyConfig_Clear>(config, dbgLog, "Core init failed!"))
        return;

    // Create an interned string for __builtins__ so we don't have to keep converting the string over and over.
    // Python LIKELY already has this string interned.
    builtInModuleName = PyUnicode_InternFromString("__builtins__");

    // We now have enough Python to insert our PEP 451 import machinery.
    initPyPackHook();

    // Now, init the interpreter.
    config._init_main = 1;
    if (!ICheckedInit<PyConfig, Py_InitializeFromConfig, PyConfig_Clear>(config, dbgLog, "Main init failed!"))
        return;

    // Initialize built-in Plasma modules. For some reason, when using the append-inittab thingy,
    // we get complaints about these modules being leaked :(
    // Note: If you add a new built-in module,
    // please add it to the list in Scripts/Python/plasma/generate_stubs.py
    // so that a stub will be generated for the new module.
    IInitBuiltinModule("Plasma", "Plasma 2.0 Game Library", dbgLog, AddPlasmaClasses, AddPlasmaMethods);
    IInitBuiltinModule("PlasmaConstants", "Plasma 2.0 Constants", dbgLog, AddPlasmaConstantsClasses);
    IInitBuiltinModule("PlasmaGame", "Plasma 2.0 GameMgr Library", dbgLog, AddPlasmaGameClasses);
    IInitBuiltinModule("PlasmaGameConstants", "Plasma 2.0 Game Constants", dbgLog, AddPlasmaGameConstantsClasses);
    IInitBuiltinModule("PlasmaNetConstants", "Plasma 2.0 Net Constants", dbgLog, AddPlasmaNetConstantsClasses);
    IInitBuiltinModule("PlasmaVaultConstants", "Plasma 2.0 Vault Constants", dbgLog, AddPlasmaVaultConstantsClasses);

    // Woo, we now have a functional Python 3 interpreter...
    dbgLog->AddLineF("Python {} interpreter is now alive!", PY_VERSION);

#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
    if (usePythonDebugger)
    {
        debugServer.SetCallbackClass(&debServerCallback);
        debugServer.Init();
        PyEval_SetTrace((Py_tracefunc)PythonTraceCallback, nullptr);
    }
#endif

    // create the output redirector for the stdout and stderr file
    stdOut = pyOutputRedirector::New();
    stdErr = pyErrorRedirector::New();

    if (stdOut) {
        if (PySys_SetObject("stdout", stdOut) != 0)
            dbgLog->AddLine(plStatusLog::kRed,  "Could not redirect stdout, Python output may not appear in the log");
    } else {
        dbgLog->AddLine(plStatusLog::kRed, "Could not create python redirector, Python output will not appear in the log");
    }

    if (stdErr) {
        if (PySys_SetObject("stderr", stdErr) == 0) {
            bool dontLog = false;

            // Find the excepthook
            pyObjectRef stdErrExceptHook = PyObject_GetAttrString(stdErr, "excepthook");
            if (stdErrExceptHook) {
                if (!PyCallable_Check(stdErrExceptHook.Get()) || PySys_SetObject("excepthook", stdErrExceptHook.Get()) != 0) {
                    dbgLog->AddLine(plStatusLog::kRed, "Could not redirect excepthook, Python error output will not get to the log server");
                    dontLog = true;
                }
            } else {
                dbgLog->AddLine(plStatusLog::kRed, "Could not find stdErr excepthook, Python error output will not get to the log server");
                dontLog = true;
            }

            if (dontLog && pyErrorRedirector::Check(stdErr)) {
                pyErrorRedirector* redir = pyErrorRedirector::ConvertFrom(stdErr);
                redir->SetLogging(false);
            }
        } else {
            dbgLog->AddLine(plStatusLog::kRed, "Could not redirect stderr, Python error output may not appear in the log or on the log server");
        }
    } else {
        dbgLog->AddLine(plStatusLog::kRed, "Could not create python redirector, Python error output will not appear in the log");
    }

    PyConfig_Clear(&config);
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
        if (dbgMod != nullptr)
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
void PythonInterface::AddPlasmaMethods(PyObject* m)
{
    cyMisc::AddPlasmaMethods(m);
    cyMisc::AddPlasmaMethods2(m);
    cyMisc::AddPlasmaMethods3(m);
    cyMisc::AddPlasmaMethods4(m);
    cyAvatar::AddPlasmaMethods(m);
    cyAccountManagement::AddPlasmaMethods(m);

    pyDrawControl::AddPlasmaMethods(m);
    pyGUIDialog::AddPlasmaMethods(m);
    pyImage::AddPlasmaMethods(m);
    pyJournalBook::AddPlasmaMethods(m);
    pyLayer::AddPlasmaMethods(m);
    pySDLModifier::AddPlasmaMethods(m);
    pySpawnPointInfo::AddPlasmaMethods(m);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaClasses
//  PARAMETERS : none
//
//  PURPOSE    : Add classes to the Plasma module
//
void PythonInterface::AddPlasmaClasses(PyObject* plasmaMod)
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
    pyDynamicText::AddPlasmaClasses(plasmaMod);
    pyImage::AddPlasmaClasses(plasmaMod);
    pyImageLibMod::AddPlasmaClasses(plasmaMod);
    pyJournalBook::AddPlasmaClasses(plasmaMod);
    pyKeyMap::AddPlasmaClasses(plasmaMod);
    pyLayer::AddPlasmaClasses(plasmaMod);
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

    // AI
    pyCritterBrain::AddPlasmaClasses(plasmaMod);

    // Game Scores
    pyGameScore::AddPlasmaClasses(plasmaMod);
    pyGameScoreMsg::AddPlasmaClasses(plasmaMod);
    pyGameScoreListMsg::AddPlasmaClasses(plasmaMod);
    pyGameScoreTransferMsg::AddPlasmaClasses(plasmaMod);
    pyGameScoreUpdateMsg::AddPlasmaClasses(plasmaMod);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaConstants module
//
void PythonInterface::AddPlasmaConstantsClasses(PyObject* plasmaConstantsMod)
{
    cyAvatar::AddPlasmaConstantsClasses(plasmaConstantsMod);
    cyMisc::AddPlasmaConstantsClasses(plasmaConstantsMod);
    cyAccountManagement::AddPlasmaConstantsClasses(plasmaConstantsMod);
    
    //pyDrawControl::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyDynamicText::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyGameScore::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyGUIControlButton::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyGUIControlMultiLineEdit::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyJournalBook::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyMarkerMgr::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyMoviePlayer::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyNotify::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pySDL::AddPlasmaConstantsClasses(plasmaConstantsMod);
    pyStatusLog::AddPlasmaConstantsClasses(plasmaConstantsMod);

    pyAIMsg::AddPlasmaConstantsClasses(plasmaConstantsMod);

    // TODO: put these constants here. remove them from below.
    //pyNetLinkingMgr::AddPlasmaConstantsClasses(plasmaConstantsMod);
    //pyVault::AddPlasmaConstantsClasses(plasmaConstantsMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaGameClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaGame module
//
void PythonInterface::AddPlasmaGameClasses(PyObject* plasmaGameMod)
{
    pyGameCli::AddPlasmaGameClasses(plasmaGameMod);
    pyGmBlueSpiral::AddPlasmaGameClasses(plasmaGameMod);
    pyGmMarker::AddPlasmaGameClasses(plasmaGameMod);
    pyGmVarSync::AddPlasmaGameClasses(plasmaGameMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaGameConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaGameConstants module
//
void PythonInterface::AddPlasmaGameConstantsClasses(PyObject* plasmaGameConstantsMod)
{
    pyGameMgr::AddPlasmaGameConstantsClasses(plasmaGameConstantsMod);
    pyGmMarker::AddPlasmaGameConstantsClasses(plasmaGameConstantsMod);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPlasmaNetConstantsClasses
//  PARAMETERS : none
//
//  PURPOSE    : Initialize the PlasmaNetConstants module
//
void PythonInterface::AddPlasmaNetConstantsClasses(PyObject* plasmaNetConstantsMod)
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
void PythonInterface::AddPlasmaVaultConstantsClasses(PyObject* plasmaVaultConstantsMod)
{
    pyVault::AddPlasmaConstantsClasses(plasmaVaultConstantsMod);
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
#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
        if (usePythonDebugger)
            debugServer.Disconnect();
#endif

        Py_CLEAR(builtInModuleName);

        // let Python clean up after itself
        if (Py_FinalizeEx() != 0)
            dbgLog->AddLine("Hmm... Errors during Python shutdown.");

        // close done the log file, if we created one
        if (dbgLog != nullptr)
        {
            delete dbgLog;
            dbgLog = nullptr;
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
    if (dbgSlice != nullptr)
    {
        // then send it the new text
        pyObjectRef retVal = plPython::CallObject(dbgSlice);
        if (!retVal) {
            // for some reason this function didn't, remember that and not call it again
            dbgSlice = nullptr;
            // if there was an error make sure that the stderr gets flushed so it can be seen
            PyErr_Print();      // make sure the error is printed
            PyErr_Clear();      // clear the error
        }
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
ST::string PythonInterface::getOutputAndReset()
{
    if (stdOut != nullptr)
    {
        ST::string strVal = pyOutputRedirector::GetData(stdOut);
        dbgLog->AddLine(strVal);

        // reset the file back to zero
        pyOutputRedirector::ClearData(stdOut);

        // tell python debugger
#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
        if (UsePythonDebugger())
            PythonInterface::PythonDebugger()->StdOut(strVal);
#endif

        // check to see if the debug python module is loaded
        if (dbgOut != nullptr)
        {
            // then send it the new text
            pyObjectRef retVal = plPython::CallObject(dbgOut, strVal);
            if (!retVal) {
                // for some reason this function didn't, remember that and not call it again
                dbgOut = nullptr;
                // if there was an error make sure that the stderr gets flushed so it can be seen
                PyErr_Print();      // make sure the error is printed
                PyErr_Clear();      // clear the error
            }
        }

        return strVal;
    }
    return {};
}

void PythonInterface::WriteToLog(const ST::string& text)
{
    dbgLog->AddLine(text);
}

void PythonInterface::WriteToStdErr(const ST::string& text)
{
    PyObject* stdErr = PythonInterface::GetStdErr();
    if (stdErr && pyErrorRedirector::Check(stdErr))
    {
        pyErrorRedirector *obj = pyErrorRedirector::ConvertFrom(stdErr);
        obj->Write(text);
    }
}

PyObject* PythonInterface::ImportModule(const char* module) 
{
    PyObject* result = nullptr;
    PyObject* name = PyUnicode_FromString(module);

    if (name != nullptr)
    {
        result = PyImport_Import(name);
        Py_DECREF(name);
    }
    
    return result;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindModule
//  PARAMETERS : module    - module name to find
//
//  PURPOSE    : Find module. If it doesn't exist then don't create, return nil.
//
PyObject* PythonInterface::FindModule(const char* module)
{
    // first we must get rid of any old modules of the same name, we'll replace it
    PyObject *modules = PyImport_GetModuleDict();
    if (PyObject* m = PyDict_GetItemString(modules, module); m && PyModule_Check(m))
        // just return what we found
        return m;

    // couldn't find the module, return None (sorta)
    return nullptr;
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
bool PythonInterface::IsModuleNameUnique(const ST::string& module)
{
    // first we must get rid of any old modules of the same name, we'll replace it
    PyObject *modules = PyImport_GetModuleDict();
    if (PyObject* m = PyDict_GetItemString(modules, module.c_str()); m && PyModule_Check(m))
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
PyObject* PythonInterface::CreateModule(const char* module)
{
    PyObject *m, *d;
    // first we must get rid of any old modules of the same name, we'll replace it
    PyObject *modules = PyImport_GetModuleDict();
    if (m = PyDict_GetItemString(modules, module); m && PyModule_Check(m))
    {
        // clear it
        hsAssert(false, ST::format("ERROR! Creating a python module of the same name - {}", module).c_str());
        ClearModule(m);
    }

    // create the module
    m = PyImport_AddModule(module);
    if (m == nullptr)
        return nullptr;
    d = PyModule_GetDict(m);
    // add in the built-ins
    // first make sure that we don't already have the builtins
    if (PyDict_GetItem(d, builtInModuleName) == nullptr)
    {
        // if we need the builtins then find the builtin module
        PyObject *bimod = PyImport_ImportModule("builtins");
        // then add the builtin dicitionary to our module's dictionary
        if (bimod == nullptr || PyDict_SetItem(d, builtInModuleName, bimod) != 0) {
            getOutputAndReset();
            return nullptr;
        }
        Py_DECREF(bimod);
    }
    return m;
}

void PythonInterface::ClearModule(PyObject* m)
{
    hsAssert(PyModule_Check(m), "PythonInterface::ClearModule() called on a non-module object");
    PyObject* dict = PyModule_GetDict(m);

    // This is basically a reimplementation of the _PyModule_ClearDict function. It's been
    // "private" forever but was finally removed from Python's public API as of 3.13. So,
    // here we are.
    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;

    // First, clear everything that begins with a single underscore.
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (value == Py_None && !PyUnicode_Check(key))
            continue;
        if (!(PyUnicode_READ_CHAR(key, 0) == '_' && PyUnicode_READ_CHAR(key, 1) != '_'))
            continue;
        if (PyDict_SetItem(dict, key, Py_None) != 0)
            PyErr_Print();
    }

    // Finally, clear everything except __builtins__
    pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (value == Py_None || !PyUnicode_Check(key))
            continue;
        if (PyUnicode_Compare(key, builtInModuleName) != 0) {
            if (PyErr_Occurred())
                PyErr_Print();
            continue;
        }
        if (PyDict_SetItem(dict, key, Py_None) != 0)
            PyErr_Print();
    }
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
    if ( module )
    {
        PyObject* d = PyModule_GetDict(module);
        return PyDict_GetItemString(d, item);
    }

    return nullptr;
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
    while (funcNames[i] != nullptr)
    {
        PyObject* func = PyDict_GetItemString(dict, funcNames[i]);
        if (func != nullptr && PyCallable_Check(func) > 0)
        {
            // if it is defined then mark the funcTable
            funcTable[i] = func;
        }
        else    // else we couldn't find the funtion
        {
            funcTable[i] = nullptr;
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
//             : and will fill out the funcTable with pointers to the function objects
//
void PythonInterface::CheckInstanceForFunctions(PyObject* instance, const char** funcNames, PyObject** funcTable)
{
    // start looking for the functions
    for (size_t i = 0; funcNames[i] != nullptr; ++i) {
        // Raises AttributeError if not found and calls to this method assert on !PyErr_Occurred()
        pyObjectRef func = PyObject_GetAttrString(instance, funcNames[i]);
        if (func) {
            if (PyCallable_Check(func.Get()))
                funcTable[i] = func.Release();
        } else {
            PyErr_Clear();
        }
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
bool PythonInterface::DumpObject(PyObject* pyobj, char** pickle, int32_t* size)
{
    PyObject *s;        // the python string object where the marsalled object wil go
    // convert object to a marshalled string python object
    s = PyMarshal_WriteObjectToString(pyobj, Py_MARSHAL_VERSION);

    // did it actually do it?
    if (s != nullptr)
    {
        // yes, then get the size and the string address
        *size = (int32_t)PyBytes_Size(s);
        *pickle = PyBytes_AsString(s);
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
//  Function   : LoadObject
//  PARAMETERS : pickle    - the pickled object in char string form
//             : size      - size of the guts to load into an object
//
//  PURPOSE    : Load a python object from a pickled object
//
PyObject* PythonInterface::LoadObject(char* pickle, int32_t size)
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
bool PythonInterface::RunStringInteractive(const char *command, PyObject* module)
{
    PyObject *d, *v;
    // make sure that we're given a good module... or at least one with an address
    if ( !module )
    {
        // if no module was given then use just use the main module
        module = PyImport_AddModule("__main__");
        if (module == nullptr) {
            PyErr_Print();
            return false;
        }
    }
    // get the dictionaries for this module
    d = PyModule_GetDict(module);
    // run the string
    v = PyRun_String(command, Py_single_input, d, d);
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
//  Function   : RunString
//  PARAMETERS : command   - string of commands to execute in the...
//             : module    - module name to run 'command' in
//
//  PURPOSE    : run a python string in a specific module name
//
bool PythonInterface::RunString(const char* command, PyObject* module)
{
    PyObject *d, *v;
    // make sure that we're given a good module... or at least one with an address
    if (!module) {
        // if no module was given then use just use the main module
        module = PyImport_AddModule("__main__");
        if (!module)
            return false;
    }
    // get the dictionaries for this module
    d = PyModule_GetDict(module);
    // run the string
    v = PyRun_String(command, Py_file_input, d, d);
    // check for errors and print them
    if (!v) {
        // Yikes! errors!
        PyErr_Print();
        return false;
    }
    Py_DECREF(v);
    PyErr_Clear();
    return true;
}

bool PythonInterface::RunFile(const plFileName& filename, PyObject* module)
{
    // make sure that we're given a good module... or at least one with an address
    if (!module) {
        // if no module was given then use just use the main module
        module = PyImport_AddModule("__main__");
        if (!module) {
            getOutputAndReset();
            return false;
        }
    }

    PyObject* moduleDict = PyModule_GetDict(module);
    PyObject* result = PyRun_FileEx(plFileSystem::Open(filename, "r"), filename.AsString().c_str(),
                                    Py_file_input, moduleDict, moduleDict, 1);
    if (!result) {
        getOutputAndReset();
        return false;
    }
    Py_DECREF(result);
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

bool PythonInterface::RunFunctionStringArg(const char* module, const char* name, const ST::string& arg)
{
    pyObjectRef moduleObj = ImportModule(module);
    bool result = false;
    if (moduleObj) {
        pyObjectRef functionObj = PyObject_GetAttrString(moduleObj.Get(), name);
        if (functionObj) {
            pyObjectRef callResult = plPython::CallObject(functionObj, arg);
            if (callResult) {
                result = true;
            }
        }
    }

    if (!result) {
        PyErr_Print();
    }

    return result;
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
        return nullptr;
    return pyKey::ConvertFrom(pkey);
}
