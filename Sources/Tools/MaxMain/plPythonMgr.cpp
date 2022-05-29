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
#include "HeadSpin.h"
#include "plgDispatch.h"
#include "plFileSystem.h"

#include "MaxAPI.h"

#include <Python.h>

#include "plPythonMgr.h"
#include "plMaxCFGFile.h"

#include "MaxComponent/plAutoUIBlock.h"
#include "MaxComponent/plPythonFileComponent.h"
#include "MaxComponent/plResponderComponent.h"

#include "pfPython/cyPythonInterface.h"
#include "pfPython/plPythonCallable.h"

plPythonMgr::plPythonMgr()
{
}

plPythonMgr& plPythonMgr::Instance()
{
    static plPythonMgr theInstance;
    return theInstance;
}

// Python wants char, not const char, LAME!
static char* kGetBlockID = "glue_getBlockID";
static char* kGetClassName = "glue_getClassName";
static char* kGetNumParams = "glue_getNumParams";
static char* kGetParam = "glue_getParam";
static char* kGetVersion = "glue_getVersion";
static char* kIsMultiModifier = "glue_isMultiModifier";
static char* kGetVisInfo = "glue_getVisInfo";

bool ICallVoidFunc(PyObject *dict, const char *funcName, PyObject*& val)
{
    PyObject *func = PyDict_GetItemString(dict, funcName);
    if (func )
    {
        if (PyCallable_Check(func))
        {
            val = plPython::CallObject(func).Release();
            if (val)
            {
                // there might have been some message printed, so get it out to the log file
                PythonInterface::getOutputAndReset();
                return true;
            }

            // There was an error when calling the function
            // get the error message
            PyErr_Print();
            PyErr_Clear();
            PythonInterface::getOutputAndReset();
        }
    }

    return false;
}

bool ICallIntFunc(PyObject *dict, const char *funcName, int& val)
{
    PyObject *obj;
    if (ICallVoidFunc(dict, funcName, obj))
    {
        if (PyLong_Check(obj))
        {
            val = PyLong_AsLong(obj);
            Py_DECREF(obj);
            return true;
        }
    }
    return false;
}

bool ICallStrFunc(PyObject *dict, const char *funcName, char*& val)
{
    PyObject *obj;
    if (ICallVoidFunc(dict, funcName, obj))
    {
        if (PyUnicode_Check(obj))
        {
            val = hsStrcpy(PyUnicode_AsUTF8(obj));
            Py_DECREF(obj);
            return true;
        }
    }

    return false;
}

bool ICallStrFunc(PyObject* dict, const char* funcName, wchar_t*& val)
{
    PyObject* obj;
    if (ICallVoidFunc(dict, funcName, obj)) {
        if (PyUnicode_Check(obj)) {
            Py_ssize_t size;
            wchar_t* pyUtf16Str = PyUnicode_AsWideCharString(obj, &size);
            val = new wchar_t[size + 1];
            wcsncpy(val, pyUtf16Str, size + 1);
            PyMem_Free(pyUtf16Str);
            Py_DECREF(obj);
            return true;
        }

        Py_XDECREF(obj);
    }

    return false;
}

enum ParamTypes
{
                            // These numbers used in the python/plasma/glue.py code
    kTypeUndefined,         //  0
    kTypeBool,              //  1
    kTypeInt,               //  2
    kTypeFloat,             //  3
    kTypeString,            //  4
    kTypeSceneObj,          //  5
    kTypeSceneObjList,      //  6
    kTypeActivatorList,     //  7
    kTypeActivator,         //  8
    kTypeResponder,         //  9
    kTypeResponderList,     // 10
    kTypeDynamicText,       // 11
    kTypeGUIDialog,         // 12
    kTypeExcludeRegion,     // 13 (x-rude-oh legion-oh)
    kTypeAnimation,         // 14 (animation component)
    kTypeAvatarBehavior,    // 15 (avatar behaviors, such as one-shots or multistage behaviors)
    kTypeMaterial,          // 16 (material type)
    kTypeGUIPopUpMenu,      // 17 (GUI pop up menu)
    kTypeGUISkin,           // 18 (Guess)
    kTypeWaterComponent,    // 19
    kTypeDropDownList,      // 20
    kTypeSwimCurrentInterface,  // 21
    kTypeClusterComponent,  // 22
    kTypeMaterialAnimation, // 23
    kTypeGrassComponent, // 24
};

bool IGetTupleInt(PyObject *tuple, int pos, int& val)
{
    PyObject *param = PyTuple_GetItem(tuple, pos);
    if (param && PyLong_Check(param))
    {
        val = PyLong_AsLong(param);
        return true;
    }

    return false;
}

bool IGetTupleFloat(PyObject *tuple, int pos, float& val)
{
    PyObject *param = PyTuple_GetItem(tuple, pos);
    if (param && PyFloat_Check(param))
    {
        val = (float)PyFloat_AsDouble(param);
        return true;
    }

    return false;
}

bool IGetTupleString(PyObject* tuple, int pos, const char*& val)
{
    PyObject* param = PyTuple_GetItem(tuple, pos);
    if (param && PyUnicode_Check(param))
    {
        val = PyUnicode_AsUTF8(param);
        return true;
    }

    return false;
}

bool IGetTupleString(PyObject *tuple, int pos, ST::string& val)
{
    PyObject *param = PyTuple_GetItem(tuple, pos);
    if (param && PyUnicode_Check(param))
    {
        Py_ssize_t size;
        const char* str = PyUnicode_AsUTF8AndSize(param, &size);
        val = ST::string::from_utf8(str, size);
        return true;
    }

    return false;
}

void IExtractVisInfo(PyObject* tuple, int* id, std::unordered_set<ST::string>& vec)
{
    PyObject* vid = PyTuple_GetItem(tuple, 0);
    PyObject* vstates = PyTuple_GetItem(tuple, 1);

    if (vid && PyLong_Check(vid))
    {
        *id = PyLong_AsLong(vid);
    }

    if (vstates && PySequence_Check(vstates))
    {
        PyObject* element;
        Py_ssize_t lsize = PySequence_Size(vstates);

        for (Py_ssize_t i = 0; i < lsize; i++)
        {
            element = PySequence_GetItem(vstates, i);
            if (element && PyUnicode_Check(element))
            {
                Py_ssize_t strSz;
                const char* str = PyUnicode_AsUTF8AndSize(element, &strSz);
                vec.emplace(ST::string::from_utf8(str, strSz));
            }
        }
    }
}

bool plPythonMgr::IQueryPythonFile(const ST::string& fileName)
{
    PyObject *module = PyImport_ImportModule(fileName.c_str());
    if (module)
    {
        // attach the glue python code to the end
        if (!PythonInterface::RunString("with open('.\\python\\plasma\\glue.py') as f: glue = f.read()\nexec(glue)", module))
        {
            // display any output (NOTE: this would be disabled in production)
            // get the messages
            PythonInterface::getOutputAndReset();
            return false;           // if we can't create the instance then there is nothing to do here
        }
        // Get the dictionary for this module
        PyObject *dict = PyModule_GetDict(module);
        // set the name of the file for the glue.py code to find
        PyObject* pfilename = PyUnicode_FromStringAndSize(fileName.c_str(), fileName.size());
        PyDict_SetItemString(dict, "glue_name", pfilename);

        // Get the block ID
        int blockID = 0;
        if (!ICallIntFunc(dict, kGetBlockID, blockID))
        {
            Py_DECREF(module);
            return false;
        }

        // Get the class name
        MCHAR* className = nullptr;
        if (!ICallStrFunc(dict, kGetClassName, className))
        {
            Py_DECREF(module);
            return false;
        }

        // Get the number of parameters
        int numParams = 0;
        if (!ICallIntFunc(dict, kGetNumParams, numParams))
        {
            Py_DECREF(module);
            return false;
        }

        // determine if this is a multimodifier
        int isMulti = 0;
        ICallIntFunc(dict, kIsMultiModifier, isMulti);

        // Get the version 
        //======================
        //  Get version must be the last call that needs a pythonfile class instance
        //  ... because it delete the instance afterwards
        //  NOTE: get attribute params doesn't need the pythonfile class instance
        int version = 0;
        if (!ICallIntFunc(dict, kGetVersion, version))
        {
            Py_DECREF(module);
            return false;
        }

        PyObject *getParamFunc = PyDict_GetItemString(dict, kGetParam);
        PyObject *getVisInfoFunc = PyDict_GetItemString(dict, kGetVisInfo);

        if (PyCallable_Check(getParamFunc))
        {
            plAutoUIBlock *autoUI = new plAutoUIBlock(PythonFile::GetClassDesc(), blockID, className, version);
            // test to see if it is a multi-modifier type class
            if (isMulti)
                autoUI->SetMultiModifierFlag(true);

            for (int i = numParams-1; i >= 0; i--)
            {
                PyObject* ret = plPython::CallObject(getParamFunc, i).Release();
                
                int ddlParamID = -1;
                std::unordered_set<ST::string> vec;

                if (PyCallable_Check(getVisInfoFunc))
                {
                    pyObjectRef visinfo = plPython::CallObject(getVisInfoFunc, i);
                    if (visinfo && PyTuple_Check(visinfo.Get()))
                    {
                        IExtractVisInfo(visinfo.Get(), &ddlParamID, vec);
                    }
                }

                if (ret)
                {
                    if (PyTuple_Check(ret))
                    {
                        int paramID = -1;
                        ST::string paramName;
                        int paramType = kTypeUndefined;

                        // Get the param ID, name, and type
                        if (IGetTupleInt(ret, 0, paramID) &&
                            IGetTupleString(ret, 1, paramName) &&
                            IGetTupleInt(ret, 2, paramType))
                        {
                            // Get the type specific params and add the param to the AutoUI block
                            switch (paramType)
                            {
                            case kTypeInt:
                                IAddInt(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeFloat:
                                IAddFloat(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeString:
                                IAddString(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeBool:
                                IAddBool(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeSceneObj:
                                IAddSceneObj(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeSceneObjList:
                                IAddSceneObjList(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeActivator:
                                IAddActivator(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeActivatorList:
                                IAddActivatorList(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeResponder:
                                IAddResponder(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeResponderList:
                                IAddResponderList(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeDynamicText:
                                IAddDynamicText(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeGUIDialog:
                                IAddGUIDialog(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeExcludeRegion:
                                IAddExcludeRegion(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeAnimation:
                                IAddAnimation(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeAvatarBehavior:
                                IAddBehavior(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeMaterial:
                                IAddMaterial(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeGUIPopUpMenu:
                                IAddGUIPopUpMenu(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;

                            case kTypeGUISkin:
                                IAddGUISkin(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeWaterComponent:
                                IAddWaterComponent(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeSwimCurrentInterface:
                                IAddSwimCurrentInterface(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeDropDownList:
                                IAddDropDownList(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeClusterComponent:
                                IAddClusterComponent(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeMaterialAnimation:
                                IAddMaterialAnimation(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            case kTypeGrassComponent:
                                IAddGrassComponent(autoUI, ret, paramName, paramID, ddlParamID, vec);
                                break;
                            }
                        }
                    }

                    Py_DECREF(ret);
                }
            }

            PythonFile::AddAutoUIBlock(autoUI);
        }

        delete [] className;
        Py_DECREF(module);
    }
    else
    {
        // There was an error when importing the module
        // get the error message and put it in the log
        PyErr_Print();
        PyErr_Clear();
        PythonInterface::getOutputAndReset();
    }


    return false;
}

void plPythonMgr::IAddBool(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    int def = 0;
    IGetTupleInt(tuple, 3, def);

    autoUI->AddCheckBox(id, {}, paramName, vid, std::move(vstates), def);
}

void plPythonMgr::IAddInt(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    int def=0, min=0, max=100;

    IGetTupleInt(tuple, 3, def);

    PyObject *range = PyTuple_GetItem(tuple, 4);
    if (range && PyTuple_Check(range))
    {
        IGetTupleInt(range, 0, min);
        IGetTupleInt(range, 1, max);
    }

    autoUI->AddIntSpinner(id, {}, paramName, vid, std::move(vstates), def, min, max);
}

void plPythonMgr::IAddFloat(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    float def=0.f, min=0.f, max=1.f;

    IGetTupleFloat(tuple, 3, def);

    PyObject *range = PyTuple_GetItem(tuple, 4);
    if (range && PyTuple_Check(range))
    {
        IGetTupleFloat(range, 0, min);
        IGetTupleFloat(range, 1, max);
    }

    autoUI->AddFloatSpinner(id, {}, paramName, vid, std::move(vstates), def, min, max);
}

void plPythonMgr::IAddString(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    const char *def = nullptr;
    IGetTupleString(tuple, 3, def);

    autoUI->AddEditBox(id, {}, paramName, vid, std::move(vstates), def);
}

void plPythonMgr::IAddSceneObj(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickNodeButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddSceneObjList(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickNodeList(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddActivator(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickActivatorButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddActivatorList(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickActivatorList(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddDynamicText(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickDynamicTextButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddGUIDialog(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickGUIDialogButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddExcludeRegion(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickExcludeRegionButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddWaterComponent(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickWaterComponentButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddSwimCurrentInterface(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickSwimCurrentInterfaceButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddClusterComponent(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickClusterComponentButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddAnimation(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickAnimationButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddBehavior(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickBehaviorButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddMaterial(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickMaterialButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddGUIPopUpMenu(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickGUIPopUpMenuButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddGUISkin(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickGUISkinButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddResponder(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    std::vector<Class_ID> cids{ RESPONDER_CID };
    autoUI->AddPickComponentButton(id, {}, paramName, vid, std::move(vstates), &cids, true);
}

void plPythonMgr::IAddResponderList(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    std::vector<Class_ID> cids{ RESPONDER_CID };
    
    autoUI->AddPickComponentList(id, {}, paramName, vid, std::move(vstates), &cids);
}

void plPythonMgr::IAddMaterialAnimation(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickMaterialAnimationButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::IAddGrassComponent(plAutoUIBlock *autoUI, PyObject *objTuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    autoUI->AddPickGrassComponentButton(id, {}, paramName, vid, std::move(vstates));
}

void plPythonMgr::LoadPythonFiles()
{
    plFileName clientPath = plMaxConfig::GetClientPath(false, true);
    if (clientPath.IsValid())
    {
        plFileName oldCwd = plFileSystem::GetCWD();
        plFileSystem::SetCWD(clientPath);

        // Get the path to the Python subdirectory of the client
        plFileName pythonPath = plFileName::Join(clientPath, "Python");

        PythonInterface::initPython();

        // Iterate through all the Python files in the folder
        std::vector<plFileName> pys = plFileSystem::ListDir(pythonPath, "*.py");
        for (auto iter = pys.begin(); iter != pys.end(); ++iter)
        {
            // Get the filename without the ".py" (module name)
            ST::string fileName = iter->GetFileNameNoExt();

            IQueryPythonFile(fileName);
        }

        PythonInterface::finiPython();

        plFileSystem::SetCWD(oldCwd);
    }
}

void plPythonMgr::IAddDropDownList(plAutoUIBlock *autoUI, PyObject *tuple, const ST::string& paramName, int id, int vid, std::unordered_set<ST::string> vstates)
{
    PyObject *options = PyTuple_GetItem(tuple, 3);
    if (options && PyTuple_Check(options))
    {
        Py_ssize_t size = PyTuple_Size(options);
        std::vector<ST::string> optionsVec(size);

        for (int i = 0; i < size; i++)
            IGetTupleString(options, i, optionsVec[i]);

        autoUI->AddDropDownList(id, {}, paramName, vid, std::move(vstates), std::move(optionsVec));
    }
}
