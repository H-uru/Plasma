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
#include "HeadSpin.h"
#include "plPythonMgr.h"

#include "../MaxComponent/plAutoUIBlock.h"
//#include "Python.h"
#include "plMaxCFGFile.h"
#include "../plFile/hsFiles.h"

#include "plgDispatch.h"
#include "../pfPython/cyPythonInterface.h"
#include "hsUtils.h"

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

bool ICallVoidFunc(PyObject *dict, char *funcName, PyObject*& val)
{
	PyObject *func = PyDict_GetItemString(dict, (char*)funcName);
	if (func )
	{
		if (PyCallable_Check(func))
		{
			val = PyObject_CallFunction(func, NULL);
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

bool ICallIntFunc(PyObject *dict, char *funcName, int& val)
{
	PyObject *obj;
	if (ICallVoidFunc(dict, funcName, obj))
	{
		if (PyInt_Check(obj))
		{
			val = PyInt_AsLong(obj);
			Py_DECREF(obj);
			return true;
		}
	}
	return false;
}

bool ICallStrFunc(PyObject *dict, char *funcName, char*& val)
{
	PyObject *obj;
	if (ICallVoidFunc(dict, funcName, obj))
	{
		if (PyString_Check(obj))
		{
			val = hsStrcpy(PyString_AsString(obj));
			Py_DECREF(obj);
			return true;
		}
	}

	return false;
}


#include "../MaxComponent/plPythonFileComponent.h"
#include "hsUtils.h"

enum ParamTypes
{
							// These numbers used in the python/plasma/glue.py code
	kTypeUndefined,			//  0
	kTypeBool,				//  1
	kTypeInt,				//  2
	kTypeFloat,				//  3
	kTypeString,			//  4
	kTypeSceneObj,			//  5
	kTypeSceneObjList,		//  6
	kTypeActivatorList,		//  7
	kTypeActivator,			//  8
	kTypeResponder,			//  9
	kTypeResponderList,		// 10
	kTypeDynamicText,		// 11
	kTypeGUIDialog,			// 12
	kTypeExcludeRegion,		// 13 (x-rude-oh legion-oh)
	kTypeAnimation,			// 14 (animation component)
	kTypeAvatarBehavior,	// 15 (avatar behaviors, such as one-shots or multistage behaviors)
	kTypeMaterial,			// 16 (material type)
	kTypeGUIPopUpMenu,		// 17 (GUI pop up menu)
	kTypeGUISkin,			// 18 (Guess)
	kTypeWaterComponent,	// 19
	kTypeDropDownList,		// 20
	kTypeSwimCurrentInterface,	// 21
	kTypeClusterComponent,	// 22
	kTypeMaterialAnimation,	// 23
	kTypeGrassComponent, // 24
};

bool IGetTupleInt(PyObject *tuple, int pos, int& val)
{
	PyObject *param = PyTuple_GetItem(tuple, pos);
	if (param && PyInt_Check(param))
	{
		val = PyInt_AsLong(param);
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

bool IGetTupleString(PyObject *tuple, int pos, char*& val)
{
	PyObject *param = PyTuple_GetItem(tuple, pos);
	if (param && PyString_Check(param))
	{
		val = PyString_AsString(param);
		return true;
	}

	return false;
}

void IExtractVisInfo(PyObject* tuple, int* id, std::vector<std::string>* vec)
{
	PyObject* vid = PyTuple_GetItem(tuple, 0);
	PyObject* vstates = PyTuple_GetItem(tuple, 1);

	if (vid && PyInt_Check(vid))
	{
		*id = PyInt_AsLong(vid);
	}

	if (vstates && PyList_Check(vstates))
	{
		PyObject* element;
		int lsize = PyList_Size(vstates);

		for (int i = 0; i < lsize; i++)
		{
			element = PyList_GetItem(vstates, i);
			if (element && PyString_Check(element))
			{
				std::string str = PyString_AsString(element);
				vec->push_back(str);
			}
		}
	}
}

bool plPythonMgr::IQueryPythonFile(char *fileName)
{
	PyObject *module = PyImport_ImportModule(fileName);
	if (module)
	{
		// attach the glue python code to the end
		if ( !PythonInterface::RunString("execfile('.\\python\\plasma\\glue.py')", module) )
		{
			// display any output (NOTE: this would be disabled in production)
			// get the messages
			PythonInterface::getOutputAndReset();
			return false;			// if we can't create the instance then there is nothing to do here
		}
		// Get the dictionary for this module
		PyObject *dict = PyModule_GetDict(module);
		// set the name of the file for the glue.py code to find
		PyObject* pfilename = PyString_FromString(fileName);
		PyDict_SetItemString(dict, "glue_name", pfilename);

		// Get the block ID
		int blockID = 0;
		if (!ICallIntFunc(dict, kGetBlockID, blockID))
		{
			Py_DECREF(module);
			return false;
		}

		// Get the class name
		char *className = nil;
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
			plAutoUIBlock *autoUI = TRACKED_NEW plAutoUIBlock(PythonFile::GetClassDesc(), blockID, className, version);
			// test to see if it is a multi-modifier type class
			if (isMulti)
				autoUI->SetMultiModifierFlag(true);

			for (int i = numParams-1; i >= 0; i--)
			{
				PyObject *ret = PyObject_CallFunction(getParamFunc, "l", i);
				
				PyObject *visinfo = nil;
				int ddlParamID = -1;
				std::vector<std::string> vec;

				if (PyCallable_Check(getVisInfoFunc))
				{
					visinfo = PyObject_CallFunction(getVisInfoFunc, "l", i);
					if (visinfo && PyTuple_Check(visinfo))
					{
						IExtractVisInfo(visinfo, &ddlParamID, &vec);
					}
				}

				if (ret)
				{
					if (PyTuple_Check(ret))
					{
						int paramID = -1;
						char *paramName = nil;
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
								IAddInt(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeFloat:
								IAddFloat(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeString:
								IAddString(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeBool:
								IAddBool(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeSceneObj:
								IAddSceneObj(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeSceneObjList:
								IAddSceneObjList(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeActivator:
								IAddActivator(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeActivatorList:
								IAddActivatorList(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeResponder:
								IAddResponder(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeResponderList:
								IAddResponderList(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeDynamicText:
								IAddDynamicText(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeGUIDialog:
								IAddGUIDialog(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeExcludeRegion:
								IAddExcludeRegion(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeAnimation:
								IAddAnimation(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeAvatarBehavior:
								IAddBehavior(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeMaterial:
								IAddMaterial(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeGUIPopUpMenu:
								IAddGUIPopUpMenu(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;

							case kTypeGUISkin:
								IAddGUISkin(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeWaterComponent:
								IAddWaterComponent(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeSwimCurrentInterface:
								IAddSwimCurrentInterface(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeDropDownList:
								IAddDropDownList(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeClusterComponent:
								IAddClusterComponent(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeMaterialAnimation:
								IAddMaterialAnimation(autoUI, ret, paramName, paramID, ddlParamID, &vec);
								break;
							case kTypeGrassComponent:
								IAddGrassComponent(autoUI, ret, paramName, paramID, ddlParamID, &vec);
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

void plPythonMgr::IAddBool(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	hsBool def = false;
	IGetTupleInt(tuple, 3, def);

	autoUI->AddCheckBox(id, nil, paramName, vid, vstates, def);
}

void plPythonMgr::IAddInt(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	int def=0, min=0, max=100;

	IGetTupleInt(tuple, 3, def);

	PyObject *range = PyTuple_GetItem(tuple, 4);
	if (range && PyTuple_Check(range))
	{
		IGetTupleInt(range, 0, min);
		IGetTupleInt(range, 1, max);
	}

	autoUI->AddIntSpinner(id, nil, paramName, vid, vstates, def, min, max);
}

void plPythonMgr::IAddFloat(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	float def=0, min=0, max=1;

	IGetTupleFloat(tuple, 3, def);

	PyObject *range = PyTuple_GetItem(tuple, 4);
	if (range && PyTuple_Check(range))
	{
		IGetTupleFloat(range, 0, min);
		IGetTupleFloat(range, 1, max);
	}

	autoUI->AddFloatSpinner(id, nil, paramName, vid, vstates, def, min, max);
}

void plPythonMgr::IAddString(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	char *def = nil;
	IGetTupleString(tuple, 3, def);

	autoUI->AddEditBox(id, nil, paramName, vid, vstates, def);
}

void plPythonMgr::IAddSceneObj(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickNodeButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddSceneObjList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickNodeList(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddActivator(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickActivatorButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddActivatorList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickActivatorList(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddDynamicText(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickDynamicTextButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddGUIDialog(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickGUIDialogButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddExcludeRegion(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickExcludeRegionButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddWaterComponent(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickWaterComponentButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddSwimCurrentInterface(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickSwimCurrentInterfaceButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddClusterComponent(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickClusterComponentButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddAnimation(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickAnimationButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddBehavior(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickBehaviorButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddMaterial(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickMaterialButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddGUIPopUpMenu(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickGUIPopUpMenuButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddGUISkin(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickGUISkinButton(id, nil, paramName, vid, vstates);
}

#include "../MaxComponent/plResponderComponent.h"

void plPythonMgr::IAddResponder(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	std::vector<Class_ID> cids;
	cids.push_back(RESPONDER_CID);
	
	autoUI->AddPickComponentButton(id, nil, paramName, vid, vstates, &cids, true);
}

void plPythonMgr::IAddResponderList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	std::vector<Class_ID> cids;
	cids.push_back(RESPONDER_CID);
	
	autoUI->AddPickComponentList(id, nil, paramName, vid, vstates, &cids);
}

void plPythonMgr::IAddMaterialAnimation(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickMaterialAnimationButton(id, nil, paramName, vid, vstates);
}

void plPythonMgr::IAddGrassComponent(plAutoUIBlock *autoUI, PyObject *objTuple, std::string paramName, int id, int vid, std::vector<std::string>* vstates)
{
	autoUI->AddPickGrassComponentButton(id, nil, paramName.c_str(), vid, vstates);
}

#include <direct.h>

void plPythonMgr::LoadPythonFiles()
{
	const char *clientPath = plMaxConfig::GetClientPath(false, true);
	if (clientPath)
	{
		char oldCwd[MAX_PATH];
		_getcwd(oldCwd, sizeof(oldCwd));
		_chdir(clientPath);

		// Get the path to the Python subdirectory of the client
		char pythonPath[MAX_PATH];
		strcpy(pythonPath, clientPath);
		strcat(pythonPath, "Python");

		PythonInterface::initPython();

		// Iterate through all the Python files in the folder
		hsFolderIterator folder(pythonPath);
		while (folder.NextFileSuffix(".py"))
		{
			// Get the filename without the ".py" (module name)
			const char *fullFileName = folder.GetFileName();
			char fileName[_MAX_FNAME];
			_splitpath(fullFileName, NULL, NULL, fileName, NULL);

			IQueryPythonFile(fileName);
		}

		PythonInterface::finiPython();

		_chdir(oldCwd);
	}
}

void plPythonMgr::IAddDropDownList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates)
{
	PyObject *options = PyTuple_GetItem(tuple, 3);
	if (options && PyTuple_Check(options))
	{
		int size = PyTuple_Size(options);
		char* opt = nil;
		std::vector<std::string> optionsVec;

		for (int i = 0; i < size; i++)
		{
			IGetTupleString(options, i, opt);
			std::string str = opt;
			optionsVec.push_back(str);
		}

		autoUI->AddDropDownList(id, nil, paramName, vid, vstates, &optionsVec);
	}
}
