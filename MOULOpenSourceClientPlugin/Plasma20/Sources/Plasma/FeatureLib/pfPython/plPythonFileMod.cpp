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
// plPythonFileMod   - the 'special' Python File modifier.
//
// This modifier will handle the interface to python code that has been file-ized.
//
//////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStream.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../plResMgr/plKeyFinder.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnKeyedObject/plUoid.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plModifier/plLogicModifier.h"
#include "../pfMessage/pfGUINotifyMsg.h"
#include "../plMessage/plRoomLoadNotifyMsg.h"
#include "../pfMessage/plClothingMsg.h"
#include "../pfMessage/pfKIMsg.h"
#include "../plMessage/plMemberUpdateMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnMessage/plRemoteAvatarInfoMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../pnMessage/plSDLNotificationMsg.h"
#include "../plMessage/plNetOwnershipMsg.h"
#include "../plSDL/plSDL.h"
#include "../plVault/plVault.h"
#include "../plMessage/plCCRMsg.h"
#include "../plMessage/plVaultNotifyMsg.h"
#include "../plInputCore/plInputInterfaceMgr.h"
#include "../plInputCore/plInputDevice.h"
#include "../pfMessage/pfMarkerMsg.h"
#include "../pfMessage/pfBackdoorMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../pfMessage/pfMovieEventMsg.h"
#include "../plMessage/plClimbEventMsg.h"
#include "../plMessage/plCaptureRenderMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plMessage/plAccountUpdateMsg.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "../pfGameMgr/pfGameMgr.h"
#include "../plMessage/plAIMsg.h"
#include "../plAvatar/plAvBrainCritter.h"

#include "plProfile.h"

#include "plPythonFileMod.h"
#include "cyPythonInterface.h"
#include "pyKey.h"
#include "cyDraw.h"
#include "cyPhysics.h"
#include "pySceneObject.h"
#include "cyMisc.h"
#include "cyCamera.h"
#include "pyNotify.h"
#include "cyAvatar.h"
#include "pyGeometry3.h"
#include "pyVault.h"
#include "pyVaultNode.h"
#include "pyVaultNodeRef.h"
#include "pyVaultAgeLinkNode.h"
#include "pyPlayer.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"
#include "pyImage.h"
#include "pyCritterBrain.h"

// GUI Control:
#include "pyGUIDialog.h"
#include "pyGUIControlButton.h"
#include "pyGUIControlCheckBox.h"
#include "pyGUIControlEditBox.h"
#include "pyGUIControlListBox.h"
#include "pyGUIControlRadioGroup.h"
#include "pyGUIControlTextBox.h"
#include "pyGUIControlValue.h"
#include "pyGUIControlDynamicText.h"
#include "pyGUIControlMultiLineEdit.h"
#include "pyGUIPopUpMenu.h"
#include "pyGUIControlClickMap.h"

// Game manager
#include "Games/pyGameMgrMsg.h"
#include "Games/pyGameCliMsg.h"

#include <locale>

#include "plPythonSDLModifier.h"

#include "../plMessage/plTimerCallbackMsg.h"

plProfile_CreateTimer("Update", "Python", PythonUpdate);

/////////////////////////////////////////////////////////////////////////////
//
// fFunctionNames    - the actual names of the functions for On[event] types
//
char*	plPythonFileMod::fFunctionNames[] = 
{
	{ "OnFirstUpdate" },		// kfunc_FirstUpdate
	{ "OnUpdate" },				// kfunc_Update
	{ "OnNotify" },				// kfunc_Notify
	{ "OnTimer" },				// kfunc_AtTimer
	{ "OnControlKeyEvent" },	// kfunc_OnKeyEvent
	{ "Load" },					// kfunc_Load
	{ "Save" },					// kfunc_Save
	{ "OnGUINotify" },			// kfunc_GUINotify
	{ "OnPageLoad"	},			// kfunc_PageLoad
	{ "OnClothingUpdate" },		// kfunc_ClothingUpdate
	{ "OnKIMsg" },				// kfunc_KIMsg,
	{ "OnMemberUpdate" },		// kfunc_MemberUpdate,
	{ "OnRemoteAvatarInfo" },	// kfunc_RemoteAvatarInfo,
	{ "OnRTChat" },				// kfunc_RTChat,
	{ "OnVaultEvent" },			// kfunc_VaultEvent,
	{ "AvatarPage" },			// kfunc_AvatarPage,
	{ "OnSDLNotify" },			// kfunc_SDLNotify
	{ "OnOwnershipChanged" },	// kfunc_OwnershipNotify
	{ "OnAgeVaultEvent" },		// kfunc_AgeVaultEvent
	{ "OnInit" },				// kfunc_Init,
	{ "OnCCRMsg" },				// kfunc_OnCCRMsg,
	{ "OnServerInitComplete" }, // kfunc_OnServerInitComplete
	{ "OnVaultNotify" },		// kfunc_OnVaultNotify
	{ "OnDefaultKeyCaught" },	// kfunc_OnDefaultKeyCaught
	{ "OnMarkerMsg" },			// kfunc_OnMarkerMsg,
	{ "OnBackdoorMsg" },		// kfunc_OnBackdoorMsg,
	{ "OnBehaviorNotify" },		// kfunc_OnBehaviorNotify,
	{ "OnLOSNotify" },			// kfunc_OnLOSNotify,
	{ "BeginAgeUnLoad" },		// kfunc_OnBeginAgeLoad,
	{ "OnMovieEvent" },			// kfunc_OnMovieEvent,
	{ "OnScreenCaptureDone" },	// kfunc_OnScreenCaptureDone,
	{ "OnClimbingBlockerEvent"},// kFunc_OnClimbingBlockerEvent,
	{ "OnAvatarSpawn"},			// kFunc_OnAvatarSpawn
	{ "OnAccountUpdate"},		// kFunc_OnAccountUpdate
	{ "gotPublicAgeList"},		// kfunc_gotPublicAgeList
	{ "OnGameMgrMsg" },			// kfunc_OnGameMgrMsg
	{ "OnGameCliMsg" },			// kfunc_OnGameCliMsg
	{ "OnAIMsg" },				// kfunc_OnAIMsg
	{ nil }
};

//// Callback From the Vault Events //////////////////////////////////////////////
class PythonVaultCallback : public VaultCallback
{
protected:
	plPythonFileMod* fPyFileMod;
	int		fFunctionIdx;

public:
	PythonVaultCallback( plPythonFileMod *pymod, int fidx )
	{
		fPyFileMod = pymod;
		fFunctionIdx = fidx;
	}

	void AddedChildNode ( RelVaultNode * parentNode, RelVaultNode * childNode )
	{
		// is there an 'OnVaultEvent' defined?
		if ( fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx] != nil )
		{
			PyObject* ptuple = PyTuple_New(1);
			PyTuple_SetItem(ptuple, 0, pyVaultNodeRef::New(parentNode, childNode));
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
						fPyFileMod->fFunctionNames[fFunctionIdx],
						"lO",pyVault::kVaultNodeRefAdded,ptuple);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFileMod->fPyFunctionInstances[fFunctionIdx] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				fPyFileMod->ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(ptuple);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			fPyFileMod->DisplayPythonOutput();
		}
	}

	void RemovingChildNode ( RelVaultNode * parentNode, RelVaultNode * childNode )
	{
		// is there an 'OnVaultEvent' defined?
		if ( fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx] != nil )
		{
			PyObject* ptuple = PyTuple_New(1);
			PyTuple_SetItem(ptuple, 0, pyVaultNodeRef::New(parentNode, childNode));
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
						fPyFileMod->fFunctionNames[fFunctionIdx],
						"lO",pyVault::kVaultRemovingNodeRef,ptuple);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFileMod->fPyFunctionInstances[fFunctionIdx] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				fPyFileMod->ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(ptuple);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			fPyFileMod->DisplayPythonOutput();
		}
	}

	void ChangedNode ( RelVaultNode * changedNode )
	{
		// is there an 'OnVaultEvent' defined?
		if ( fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx] != nil )
		{
			PyObject* ptuple = PyTuple_New(1);
			PyTuple_SetItem(ptuple, 0, pyVaultNode::New(changedNode));
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
						fPyFileMod->fFunctionNames[fFunctionIdx],
						"lO",pyVault::kVaultNodeSaved,ptuple);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFileMod->fPyFunctionInstances[fFunctionIdx] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				fPyFileMod->ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(ptuple);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			fPyFileMod->DisplayPythonOutput();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
//
//  Class	   : pfPythonKeyCatcher
//  PARAMETERS : none
//
//  PURPOSE    : Small wrapper class to catch discarded key events and pass
//				 them to a plPythonFileMod
//

class pfPythonKeyCatcher : public plDefaultKeyCatcher
{
	plPythonFileMod *fMod;

	public:
		pfPythonKeyCatcher( plPythonFileMod *mod ) : fMod( mod ) {}
		
		virtual void	HandleKeyEvent( plKeyEventMsg *event )
		{
			fMod->HandleDiscardedKey( event );
		}
};

hsBool plPythonFileMod::fAtConvertTime = false;

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : plPythonFileMod and ~plPythonFileMod
//  PARAMETERS : none
//
//  PURPOSE    : Constructor and destructor
//
plPythonFileMod::plPythonFileMod()
{
	fPythonFile = nil;
	fModuleName = nil;
	fModule = nil;
	fLocalNotify= true;
	fIsFirstTimeEval = true;
	fVaultCallback = nil;
	fSDLMod = nil;
	fSelfKey = nil;
	fInstance = nil;
	fKeyCatcher = nil;
	fPipe = nil;
	fAmIAttachedToClone = false;

	// assume that all the functions are not available
	// ...if the functions are defined in the module, then we'll call 'em
	int i;
	for (i=0 ; i<kfunc_lastone; i++)
		fPyFunctionInstances[i] = nil;
}

plPythonFileMod::~plPythonFileMod()
{
	if ( !fAtConvertTime )		// if this is just an Add that's during a convert, then don't do anymore
	{
		// remove our reference to the instance (but only if we made one)
		if(fInstance)
		{
			if ( fInstance->ob_refcnt > 1)
				Py_DECREF(fInstance);
			//  then have the glue delete the instance of class
			PyObject* delInst = PythonInterface::GetModuleItem("glue_delInst",fModule);
			if ( delInst!=nil && PyCallable_Check(delInst) )
			{
				PyObject* retVal = PyObject_CallFunction(delInst,nil);
				if ( retVal == nil )
				{
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				// display any output
				DisplayPythonOutput();
			}
		}
		fInstance = nil;
	}

	// If we have a key catcher, get rid of it
	delete fKeyCatcher;
	fKeyCatcher = nil;

	// if we created a Vault callback, undo it and get rid of it
	if (fVaultCallback)
	{
		// Set the callback for the vault thingy
		VaultUnregisterCallback(fVaultCallback);
		delete fVaultCallback;
		fVaultCallback = nil;
	}

	if (fSelfKey)
	{
		Py_DECREF(fSelfKey);
		fSelfKey = nil;
	}

	// get rid of the python code
	if ( fPythonFile )
	{
		delete [] fPythonFile;
		fPythonFile = nil;
	}
	// then get rid of this module
	//  NOTE: fModule shouldn't be made in the plugin, only at runtime
	if ( fModuleName && fModule )
	{
		//_PyModule_Clear(fModule);
		PyObject *m;
		PyObject *modules = PyImport_GetModuleDict();
		if( modules && (m = PyDict_GetItemString(modules, fModuleName)) && PyModule_Check(m))
		{
			hsStatusMessageF("Module %s removed from python dictionary",fModuleName);
			PyDict_DelItemString(modules, fModuleName);
		}
		else
		{
			hsStatusMessageF("Module %s not found in python dictionary. Already removed?",fModuleName);
		}
		// the above code should have unloaded the module from python, so it will delete itself, therefore
		// we need to set our pointer to nil to make sure we don't try to use it
		fModule = nil;
	}
	delete [] fModuleName;
	fModuleName = nil;
}

#include "plPythonPack.h"

bool plPythonFileMod::ILoadPythonCode()
{

#ifndef PLASMA_EXTERNAL_RELEASE
	// get code from file and execute in module
	// see if the file exists first before trying to import it
	char pathandfile[256];
	sprintf(pathandfile, ".\\python\\%s.py",fPythonFile);
	wchar *wPathandfile = hsStringToWString(pathandfile);
	hsBool exists = PathDoesFileExist(wPathandfile);
	delete [] wPathandfile;
	if (exists)
	{
		char fromLoad[256];
		//sprintf(fromLoad,"from %s import *", fPythonFile);
		// ok... we can't really use import because Python remembers too much where global variables came from
		// ...and using execfile make it sure that globals are defined in this module and not in the imported module
		sprintf(fromLoad,"execfile('.\\\\python\\\\%s.py')", fPythonFile);
		if ( PythonInterface::RunString( fromLoad, fModule) )
		{
			// we've loaded the code into our module
		// now attach the glue python code to the end
			if ( !PythonInterface::RunString("execfile('.\\\\python\\\\plasma\\\\glue.py')", fModule) )
			{
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
				return false;
			}
			else
				return true;
		}
		DisplayPythonOutput();
		char errMsg[256];
		sprintf(errMsg,"Python file %s.py had errors!!! Could not load.",fPythonFile);
		PythonInterface::WriteToLog(errMsg);
		hsAssert(0,errMsg);
		return false;
	}
#endif  //PLASMA_EXTERNAL_RELEASE

	// Finally, try and find the file in the Python packfile
	// ... for the external users .pak file is only used
	PyObject* pythonCode = PythonPack::OpenPythonPacked(fPythonFile);
	if (pythonCode && PythonInterface::RunPYC(pythonCode, fModule))
		return true;

	DisplayPythonOutput();
	char errMsg[256];
	sprintf(errMsg,"Python file %s.py was not found.",fPythonFile);
	PythonInterface::WriteToLog(errMsg);
	hsAssert(0,errMsg);
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddTarget
//  PARAMETERS : sobj  - object to add as our target
//
//  PURPOSE    : Get the Key of our target
//
// NOTE: This modifier wasn't intended to have multiple targets
//
void plPythonFileMod::AddTarget(plSceneObject* sobj)
{
	plMultiModifier::AddTarget(sobj);
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeBeginLoadingMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	// initialize the python stuff
	if ( !fAtConvertTime )		// if this is just an Add that's during a convert, then don't do anymore
	{
		// was there a python file module with this?
		if ( fPythonFile )
		{
			// has the module not been initialized yet
			if ( !fModule )
			{
				plKey pkey = sobj->GetKey();
				// nope, must be the first object. Then use it as the basis for the module
				char modulename[256];
				IMakeModuleName(modulename,sobj);
				delete [] fModuleName;
				fModuleName = StrDup(modulename);
				fModule = PythonInterface::CreateModule(modulename);

				// if we can't create the instance then there is nothing to do here
				if (!ILoadPythonCode())
				{
					// things are getting off on a bad foot... just say there wasn't a module...
					fModule = nil;
					return;
				}

			// set the name of the file (in the global dictionary of the module)
				PyObject* dict = PyModule_GetDict(fModule);
				PyObject* pfilename = PyString_FromString(fPythonFile);
				PyDict_SetItemString(dict, "glue_name", pfilename);
			// next we need to:
			//  - create instance of class
				PyObject* getInst = PythonInterface::GetModuleItem("glue_getInst",fModule);
				fInstance = nil;
				if ( getInst!=nil && PyCallable_Check(getInst) )
				{
					fInstance = PyObject_CallFunction(getInst,nil);
					if ( fInstance == nil )
						// if there was an error make sure that the stderr gets flushed so it can be seen
						ReportError();
				}
				// display any output
				DisplayPythonOutput();
				if ( fInstance == nil )		// then there was an error
				{
					// display any output (NOTE: this would be disabled in production)
					char errMsg[256];
					sprintf(errMsg,"Python file %s.py, instance not found.",fPythonFile);
					PythonInterface::WriteToLog(errMsg);
					hsAssert(0, errMsg);
					return;			// if we can't create the instance then there is nothing to do here
				}

				// Add the SDL modifier
				if (plPythonSDLModifier::HasSDL(fPythonFile))
				{
					plSceneObject* sceneObj = plSceneObject::ConvertNoRef(GetTarget(0)->GetKey()->ObjectIsLoaded());
					if (sceneObj)
					{
						hsAssert(!fSDLMod, "Python SDL modifier already created");
						fSDLMod = TRACKED_NEW plPythonSDLModifier(this);
						sceneObj->AddModifier(fSDLMod);
					}
				}

				//  - set the self.key and self.sceneobject in the instance of the class
				// create the pyKey for this modifier
				fSelfKey = pyKey::New(GetKey(),this);
				// set the selfKey as an attribute to their instance
				PyObject_SetAttrString(fInstance, "key", fSelfKey);
				// create the sceneobject
				PyObject* pSobj = pySceneObject::New(pkey, fSelfKey);
				// set the sceneobject as an attribute to their instance
				PyObject_SetAttrString(fInstance, "sceneobject", pSobj);
				Py_DECREF(pSobj);
				// set the isInitialStateLoaded to not loaded... yet
				PyObject* pInitialState = PyInt_FromLong(0);
				PyObject_SetAttrString(fInstance, "isInitialStateLoaded", pInitialState);
				Py_DECREF(pInitialState);
				// Give the SDL mod to Python
				if (fSDLMod)
				{
					PyObject* pSDL = pySDLModifier::New(fSDLMod);
					PyObject_SetAttrString(fInstance, "SDL", pSDL);
					Py_DECREF(pSDL);
				}

				//  - set the parameters
				PyObject* setParams = PythonInterface::GetModuleItem("glue_setParam",fModule);
				PyObject* check_isNamed = PythonInterface::GetModuleItem("glue_isNamedAttribute",fModule);
				if ( setParams!=nil && PyCallable_Check(setParams) )
				{
					// loop throught the parameters and set them by id
					// (will need to create the appropiate Python object for each type)
					int nparam;
					for ( nparam=0; nparam<GetParameterListCount() ; nparam++ )
					{
						plPythonParameter parameter = GetParameterItem(nparam);
						// create the python object that matches the type
						// (NOTE: this relies on function created above to help create Plasma python objects)
						PyObject* value = nil;		// assume that there is no conversion available
						int isNamedAttr = 0;
						PyObject* retvalue;
						switch (parameter.fValueType)
						{
							case plPythonParameter::kInt:
								value = PyInt_FromLong(parameter.datarecord.fIntNumber);
								break;
							case plPythonParameter::kFloat:
								value = PyFloat_FromDouble(parameter.datarecord.fFloatNumber);
								break;
							case plPythonParameter::kBoolean:
								value = PyInt_FromLong(parameter.datarecord.fBool);
								break;
							case plPythonParameter::kString:
							case plPythonParameter::kAnimationName:
								isNamedAttr = 0;
								if ( check_isNamed!=nil && PyCallable_Check(check_isNamed) )
								{
									retvalue = PyObject_CallFunction(check_isNamed,"l", parameter.fID);
									if ( retvalue == nil )
									{
										ReportError();
										DisplayPythonOutput();
									}
									if ( retvalue && PyInt_Check(retvalue) )
										isNamedAttr = PyInt_AsLong(retvalue);
									Py_XDECREF(retvalue);
									// is it a NamedActivator
									if ( isNamedAttr == 1 || isNamedAttr == 2)
									{
										if (plAgeLoader::GetInstance()->IsLoadingAge())
										{
											NamedComponent comp;
											comp.isActivator = (isNamedAttr == 1);
											comp.id = parameter.fID;
											comp.name = TRACKED_NEW char[strlen(parameter.datarecord.fString) + 1];
											strcpy(comp.name, parameter.datarecord.fString);
											
											fNamedCompQueue.Append(comp);
										}
										else
										{
											if (isNamedAttr == 1)
												IFindActivatorAndAdd(parameter.datarecord.fString, parameter.fID);
											else
												IFindResponderAndAdd(parameter.datarecord.fString, parameter.fID);
										}
									}
								}
								// if it wasn't a named string then must be normal string type
								if ( isNamedAttr == 0 )
									if ( parameter.datarecord.fString != nil )
										value = PyString_FromString(parameter.datarecord.fString);
								break;
							case plPythonParameter::kSceneObject:
							case plPythonParameter::kSceneObjectList:
								if ( parameter.fObjectKey != nil )
								{
									// create the sceneobject
									value = pySceneObject::New(parameter.fObjectKey, fSelfKey);
								}
								break;
							case plPythonParameter::kActivatorList:
							case plPythonParameter::kResponderList:
							case plPythonParameter::kDynamicText:
							case plPythonParameter::kGUIDialog:
							case plPythonParameter::kExcludeRegion:
							case plPythonParameter::kAnimation:
							case plPythonParameter::kBehavior:
							case plPythonParameter::kMaterial:
							case plPythonParameter::kGUIPopUpMenu:
							case plPythonParameter::kGUISkin:
							case plPythonParameter::kWaterComponent:
							case plPythonParameter::kSwimCurrentInterface:
							case plPythonParameter::kClusterComponentList:
							case plPythonParameter::kMaterialAnimation:
							case plPythonParameter::kGrassShaderComponent:
								if ( parameter.fObjectKey != nil )
								{
									// create pyKey for the object
									value = pyKey::New(parameter.fObjectKey);
								}
								break;
						}
						// if there is a value that was converted then tell the Python code
						if ( value != nil )
						{
							PyObject* retVal = PyObject_CallFunction(setParams,"lO", parameter.fID, value);
							if ( retVal == nil )
							{
								// if there was an error make sure that the stderr gets flushed so it can be seen
								ReportError();
							}
							Py_XDECREF(retVal);
							Py_DECREF(value);
						}
					}
				}

				// check if we need to register named activators or responders
				if (fNamedCompQueue.Count() > 0)
				{
					plgDispatch::Dispatch()->RegisterForExactType( plAgeLoadedMsg::Index(), GetKey() );
				}

			//  - find functions in class they've defined.
				PythonInterface::CheckInstanceForFunctions(fInstance,fFunctionNames,fPyFunctionInstances);
				// clear any errors created by checking for methods in a class
				PyErr_Clear();		// clear the error
			// register for messages that they have functions defined for
				// register for PageLoaded message if needed
				if ( fPyFunctionInstances[kfunc_PageLoad] != nil )
				{
					// register for plRoomLoadNotifyMsg
					plgDispatch::Dispatch()->RegisterForExactType(plRoomLoadNotifyMsg::Index(), GetKey());
				}

				// register for ClothingUpdate message if needed
				if ( fPyFunctionInstances[kfunc_ClothingUpdate] != nil )
				{
					// register for plRoomLoadNotifyMsg
					plgDispatch::Dispatch()->RegisterForExactType(plClothingUpdateBCMsg::Index(), GetKey());
				}

				// register for pfKIMsg message if needed
				if ( fPyFunctionInstances[kfunc_KIMsg] != nil )
				{
					// register for pfKIMsg
					plgDispatch::Dispatch()->RegisterForExactType(pfKIMsg::Index(), GetKey());
				}

				// register for Member update message if needed
				if ( fPyFunctionInstances[kfunc_MemberUpdate] != nil )
				{
					// register for plMemberUpdateMsg
					plgDispatch::Dispatch()->RegisterForExactType(plMemberUpdateMsg::Index(), GetKey());
				}

				// register for Remote Avatar Info message if needed
				if ( fPyFunctionInstances[kfunc_RemoteAvatarInfo] != nil )
				{
					// register for plRemoteAvatarInfoMsg
					plgDispatch::Dispatch()->RegisterForExactType(plRemoteAvatarInfoMsg::Index(), GetKey());
				}

				// register for CCR message if needed
				if ( fPyFunctionInstances[kfunc_OnCCRMsg] != nil )
				{
					// register for plCCRCommunicationMsg
					plgDispatch::Dispatch()->RegisterForExactType(plCCRCommunicationMsg::Index(), GetKey());
				}

				// register for VaultNotify message if needed
				if ( fPyFunctionInstances[kfunc_OnVaultNotify] != nil )
				{
					// register for plVaultNotifyMsg
					plgDispatch::Dispatch()->RegisterForExactType(plVaultNotifyMsg::Index(), GetKey());
				}

				// register for Owndership change notification message if needed
				if ( fPyFunctionInstances[kfunc_OwnershipNotify] != nil )
				{
					// register for plNetOwnershipMsg
					plgDispatch::Dispatch()->RegisterForExactType(plNetOwnershipMsg::Index(), GetKey());
				}

#ifndef PLASMA_EXTERNAL_RELEASE
				// register for Backdoor message if needed
				if ( fPyFunctionInstances[kfunc_OnBackdoorMsg] != nil )
				{
					// register for pfDebugTriggerMsg
					plgDispatch::Dispatch()->RegisterForExactType(pfBackdoorMsg::Index(), GetKey());
				}
#endif  //PLASMA_EXTERNAL_RELEASE

				// register for VaultCallback events if needed
				if ( fPyFunctionInstances[kfunc_VaultEvent] != nil )
				{
					// create the callback object
					// Set the callback for the vault thingy
					fVaultCallback = TRACKED_NEW PythonVaultCallback( this, kfunc_VaultEvent );
					VaultRegisterCallback(fVaultCallback);
				}

				// register ourselves to be the default key catcher if necessary
				if ( fPyFunctionInstances[kfunc_OnDefaultKeyCaught] != nil )
				{
					// Make us a key catcher
					fKeyCatcher = TRACKED_NEW pfPythonKeyCatcher( this );

					// Tell the input interface manager to use our catcher
					plInputInterfaceMgr::GetInstance()->SetDefaultKeyCatcher( fKeyCatcher );
				}

				// register for Marker messages if needed
				if ( fPyFunctionInstances[kfunc_OnMarkerMsg] != nil )
				{
					plgDispatch::Dispatch()->RegisterForExactType(pfMarkerMsg::Index(), GetKey());
				}

				// if they are going to get LOS hit messages then we need to get the Pipeline pointer
				if ( fPyFunctionInstances[kfunc_OnLOSNotify] != nil )
				{
					plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), GetKey() );
				}
				
				// if this is a climbing-wall function, we need to register for climbing wall messages
				if ( fPyFunctionInstances[kfunc_OnClimbBlockerEvent] != nil)
				{
					plgDispatch::Dispatch()->RegisterForExactType( plClimbEventMsg::Index(), GetKey() );	
				}
				if ( fPyFunctionInstances[kfunc_OnAvatarSpawn] != nil)
				{
					plgDispatch::Dispatch()->RegisterForExactType( plAvatarSpawnNotifyMsg::Index(), GetKey() );	
				}
				if ( fPyFunctionInstances[kfunc_OnAccountUpdate] != nil)
				{
					plgDispatch::Dispatch()->RegisterForExactType( plAccountUpdateMsg::Index(), GetKey() );	
				}
				if ( fPyFunctionInstances[kfunc_gotPublicAgeList] != nil)
				{
					plgDispatch::Dispatch()->RegisterForExactType(plNetCommPublicAgeListMsg::Index(), GetKey());
				}
				if ( fPyFunctionInstances[kfunc_OnGameMgrMsg] != nil)
				{
					pfGameMgr::GetInstance()->AddReceiver(GetKey());
				}
				if ( fPyFunctionInstances[kfunc_OnAIMsg] != nil)
				{
					// the message that is spammed to anyone who will listen
					plgDispatch::Dispatch()->RegisterForExactType(plAIBrainCreatedMsg::Index(), GetKey());
				}

				// As the last thing... call the OnInit function if they have one
				if ( fPyFunctionInstances[kfunc_Init] != nil )
				{
					plProfile_BeginTiming(PythonUpdate);
					// call it
					PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Init],fFunctionNames[kfunc_Init],nil);
					if ( retVal == nil )
					{
#ifndef PLASMA_EXTERNAL_RELEASE
						// for some reason this function didn't, remember that and not call it again
						fPyFunctionInstances[kfunc_Init] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
						// if there was an error make sure that the stderr gets flushed so it can be seen
						ReportError();
					}
					Py_XDECREF(retVal);
					plProfile_EndTiming(PythonUpdate);
					// display any output (NOTE: this would be disabled in production)
					DisplayPythonOutput();
				}

				// display python output
				DisplayPythonOutput();
			}
			else
			{
			// else if module is already created... Then we are just adding an addition object to the already existing SceneObject
				if ( fInstance )		// make sure that we have an instance already also
				{
					PyObject* dict = PyModule_GetDict(fModule);
					// create pyKey for the object
					PyObject* pkeyObj = pyKey::New(sobj->GetKey());
					// need to get the instance, that holds the sceneobject that we are attached to
					PyObject* getInst = PythonInterface::GetModuleItem("glue_getInst",fModule);
					// get the sceneObject that should already be created
					PyObject* pSceneObject = PyObject_GetAttrString(fInstance,"sceneobject");
					// add our new object to the list of objects that are in the _selfObject
					PyObject* retVal = PyObject_CallMethod(pSceneObject,"addKey","O",pkeyObj );
					Py_XDECREF(retVal);
					// GetAttrString put a ref on pSceneObject, but we're done with it now.
					Py_XDECREF(pSceneObject); 
					Py_DECREF(pkeyObj);
				}
			}
		}
	}
}

void plPythonFileMod::RemoveTarget(plSceneObject* so)
{
	// remove sdl modifier
	if (fSDLMod)
	{
		if (GetNumTargets() > 0)
		{
			plSceneObject* sceneObj = plSceneObject::ConvertNoRef(GetTarget(0)->GetKey()->ObjectIsLoaded());
			if (sceneObj && fSDLMod)
				sceneObj->RemoveModifier(fSDLMod);
		}
		delete fSDLMod;
		fSDLMod = nil;
	}

	plMultiModifier::RemoveTarget(so);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : HandleDiscardedKey
//  PARAMETERS : msg - the key event message that was discarded
//
//  PURPOSE    : API for processing discarded keys as the deafult key catcher
//

void	plPythonFileMod::HandleDiscardedKey( plKeyEventMsg *msg )
{
	// So OnDefaultKeyCaught takes two parameters: the key character pressed and a boolean saying up or down
	char keyChar = plKeyboardDevice::KeyEventToChar( msg );

	// if the caps lock is down then reverse upper and lowercase
	if ( msg->GetCapsLockKeyDown() )
	{
		if ( std::islower(keyChar,std::locale()) )
			keyChar = std::toupper(keyChar,std::locale());
		else
			keyChar = std::tolower(keyChar,std::locale());
	}

	if (!fPyFunctionInstances[kfunc_OnDefaultKeyCaught])
		return;

	plProfile_BeginTiming( PythonUpdate );

	PyObject* retVal = PyObject_CallMethod( fPyFunctionInstances[ kfunc_OnDefaultKeyCaught ],
				fFunctionNames[ kfunc_OnDefaultKeyCaught ],
				"ciiiii",
				keyChar, 
				(int)msg->GetKeyDown(),
				(int)msg->GetRepeat(),
				(int)msg->GetShiftKeyDown(),
				(int)msg->GetCtrlKeyDown(),
				(int)msg->GetKeyCode() );
	if( retVal == nil )
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		// for some reason this function didn't, remember that and not call it again
		fPyFunctionInstances[ kfunc_OnDefaultKeyCaught ] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
		// if there was an error make sure that the stderr gets flushed so it can be seen
		ReportError();
	}
	Py_XDECREF(retVal);

	plProfile_EndTiming( PythonUpdate );
	// display any output (NOTE: this would be disabled in production)
	DisplayPythonOutput();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IMakeModuleName
//  PARAMETERS : sobj  - object to add as our target
//
//  PURPOSE    : Get the Key of our target
//
// NOTE: This modifier wasn't intended to have multiple targets
//
void plPythonFileMod::IMakeModuleName(char* modulename,plSceneObject* sobj)
{
	// Forgive my general crapulance...
	// This strips underscores out of module names 
	// so python won't truncate them... -S

	plKey pKey = GetKey();
	plKey sKey = sobj->GetKey();

	const char* pKeyName = pKey->GetName(); 
	const char* pSobjName = sKey->GetName(); 

	UInt16 len = hsStrlen(pKeyName);
	UInt16 slen = hsStrlen(pSobjName);

	hsAssert(len+slen < 256, "Warning: String length exceeds 256 characters.");
	
	int i, k = 0;
	for(i = 0; i < slen; i++)
	{
		if(pSobjName[i] == '_') continue;

		modulename[k++] = pSobjName[i];
	}
	for(i = 0; i < len; i++)
	{
		if(pKeyName[i] == '_') continue;

		modulename[k++] = pKeyName[i];
	}

	modulename[k] = '\0';

	// check to see if we are attaching to a clone?
	plKeyImp* pKeyImp = (plKeyImp*)(sKey);
	if (pKeyImp->GetCloneOwner())
	{
		// we have an owner... so we must be a clone.
		// add the cloneID to the end of the module name
		// and set the fIAmAClone flag
		UInt32 cloneID = pKeyImp->GetUoid().GetCloneID();
		sprintf(modulename,"%s%d",modulename,cloneID);
		fAmIAttachedToClone = true;
	}

	// make sure that the actual modulue will be uniqie
	if ( !PythonInterface::IsModuleNameUnique(modulename) )
	{
		// if not unique then add the sequence number to the end of the modulename
		UInt32 seqID = pKeyImp->GetUoid().GetLocation().GetSequenceNumber();
		sprintf(modulename,"%s%d",modulename,seqID);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ISetKeyValue
//  PARAMETERS : key to responder, parameter id
//
//  PURPOSE    : set the param in the python file
//             : so named stuff works
//
void plPythonFileMod::ISetKeyValue(const plKey& key, Int32 id)
{
	PyObject* setParams = PythonInterface::GetModuleItem("glue_setParam",fModule);
	
	if ( setParams != nil && PyCallable_Check(setParams) )
	{
		if ( key != nil )
		{
			// create pyKey for the object
			PyObject* value = pyKey::New(key);

			if ( value != nil )
			{
				PyObject* retVal = PyObject_CallFunction(setParams,"lO", id, value);
				if ( retVal == nil )
				{
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				Py_DECREF(value);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindResponderAndAdd
//  PARAMETERS : ResponderName  - name of the responder to find
//
//  PURPOSE    : find a responder by name in all age and page locations
//             : and add to the Parameter list
//
void plPythonFileMod::IFindResponderAndAdd(const char *responderName, Int32 id)
{
	if ( responderName != nil )
	{
		std::vector<plKey> keylist;
		const plLocation &loc = GetKey()->GetUoid().GetLocation();
		plKeyFinder::Instance().ReallyStupidResponderSearch(responderName,keylist,loc); // use the really stupid search to find the responder
		// the keylist will be filled with all the keys that correspond to that responder
		int list_size = keylist.size();
		int i;
		for ( i=0 ; i<list_size; i++ )
		{
			plPythonParameter parm(id);
			parm.SetToResponder(keylist[i]);
			AddParameter(parm);
			ISetKeyValue(keylist[i], id);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindActivatorAndAdd
//  PARAMETERS : ResponderName  - name of the responder to find
//
//  PURPOSE    : find a responder by name in all age and page locations
//             : and add to the Parameter list
//
void plPythonFileMod::IFindActivatorAndAdd(const char *activatorName, Int32 id)
{
	if ( activatorName != nil )
	{
		std::vector<plKey> keylist;
		const plLocation &loc = GetKey()->GetUoid().GetLocation();
		plKeyFinder::Instance().ReallyStupidActivatorSearch(activatorName,keylist, loc); // use the really stupid search to find the responder
		// the keylist will be filled with all the keys that correspond to that responder
		int list_size = keylist.size();
		// create the Python object that is the list, starts as empty
		int i;
		for ( i=0 ; i<list_size; i++ )
		{
			plPythonParameter parm(id);
			parm.SetToActivator(keylist[i]);
			AddParameter(parm);
			ISetKeyValue(keylist[i], id);

			// need to add ourselves as a receiver to their list
			// first see if it is an logicMod, then add to their receiver list
			plLogicModifier *logic = plLogicModifier::ConvertNoRef(keylist[i]->ObjectIsLoaded());
			if (logic)
			{
				logic->AddNotifyReceiver(this->GetKey());
			}
			else  // else might be a python file key
			{
				// next check to see if it is another PythonFileMod, and add to their notify list
				plPythonFileMod *pymod = plPythonFileMod::ConvertNoRef(keylist[i]->ObjectIsLoaded());
				if (pymod)
				{
					pymod->AddToNotifyList(this->GetKey());
				}
				else  // else maybe its just not loaded yet
				{
					// setup a ref notify when it does get loaded
					hsgResMgr::ResMgr()->AddViaNotify(keylist[i],
													TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, kAddNotify, 0),
													plRefFlags::kPassiveRef);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IEval
//  PARAMETERS : secs
//               del
//               dirty
//
//  PURPOSE    : This is where the main update work is done
//    Tasks:
//      - Call the Python code's Update function (if there)
//
hsBool plPythonFileMod::IEval(double secs, hsScalar del, UInt32 dirty)
{
	if ( fModule )
	{
		// if this is the first time at the Eval, then run Python init
		if ( fIsFirstTimeEval )
		{
			fIsFirstTimeEval = false;		// no longer the first time
			// now run the __init__ function if there is one.
			// is the Update function defined and working (as far as we know)?
			if ( fPyFunctionInstances[kfunc_FirstUpdate] != nil )
			{
				plProfile_BeginTiming(PythonUpdate);
				// call it
				PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_FirstUpdate],fFunctionNames[kfunc_FirstUpdate],nil);
				if ( retVal == nil )
				{
#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_FirstUpdate] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				plProfile_EndTiming(PythonUpdate);
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
			}
		}

		// is the Update function defined and working (as far as we know)?
		if ( fPyFunctionInstances[kfunc_Update] != nil )
		{
			plProfile_BeginTiming(PythonUpdate);
			// call it
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Update],fFunctionNames[kfunc_Update],"df",secs,del);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_Update] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
		}
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : MsgReceive
//  PARAMETERS : msg   - the message that came to us.
//
//  PURPOSE    : Handle all the different types of messages that we recv
//
hsBool plPythonFileMod::MsgReceive(plMessage* msg)
{
	// is it a ref message
	plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(msg);
	if (genRefMsg)
	{
		// is it a ref for a named activator that we need to add to notify?
		if ((genRefMsg->GetContext() & plRefMsg::kOnCreate) && genRefMsg->fWhich == kAddNotify)
		{
			// which kind of activator is this
			plLogicModifier *logic = plLogicModifier::ConvertNoRef(genRefMsg->GetRef());
			if (logic)
			{
				logic->AddNotifyReceiver(this->GetKey());
			}
			else  // else might be a python file key
			{
				// next check to see if it is another PythonFileMod, and add to their notify list
				plPythonFileMod *pymod = plPythonFileMod::ConvertNoRef(genRefMsg->GetRef());
				if (pymod)
				{
					pymod->AddToNotifyList(this->GetKey());
				}
			}
		}
	}

	plAgeLoadedMsg* ageLoadedMsg = plAgeLoadedMsg::ConvertNoRef(msg);
	if (ageLoadedMsg && ageLoadedMsg->fLoaded)
	{
		for (int i = 0; i < fNamedCompQueue.Count(); ++i)
		{
			NamedComponent comp = fNamedCompQueue[i];
			if (comp.isActivator)
				IFindActivatorAndAdd(comp.name, comp.id);
			else
				IFindResponderAndAdd(comp.name, comp.id);

			delete [] comp.name;
		}

		fNamedCompQueue.Reset();

		plgDispatch::Dispatch()->UnRegisterForExactType( plAgeLoadedMsg::Index(), GetKey() );
	}

	// if this is a render message, then we are just trying to get a pointer to the Pipeline
	plRenderMsg *rMsg = plRenderMsg::ConvertNoRef( msg );
	if( rMsg != nil )
	{
		fPipe = rMsg->Pipeline();
		plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), GetKey() );
		return true;
	}

	// are they looking for an Notify message? should be coming from a proActivator
	if (fPyFunctionInstances[kfunc_Notify] != nil)
	{
		// yes, so was there actually a plActivateMsg?
		plNotifyMsg* pNtfyMsg = plNotifyMsg::ConvertNoRef(msg);
		if (pNtfyMsg)
		{
			// remember if this was a Local Broad cast or not
			fLocalNotify = (pNtfyMsg->HasBCastFlag(plMessage ::plBCastFlags::kNetNonLocal)) ? false : true;

			// create a list for the event records
			PyObject* levents = PyList_New(0); // start with a list of no elements
			// loop thought the event records to get the data and transform into python objects
			Int32 num_records = pNtfyMsg->GetEventCount();
			int j;
			for ( j=0; j<num_records; j++ )
			{
				// get an event record
				proEventData* pED = pNtfyMsg->GetEventRecord(j);
				switch ( pED->fEventType )
				{

					case proEventData::kCollision:
						{
							proCollisionEventData *eventData = (proCollisionEventData *)pED;
							// get data from the collision
							// create list
							PyObject* event = PyList_New(4);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kCollision));
							PyList_SetItem(event, 1, PyInt_FromLong(eventData->fEnter ? 1 : 0));
							PyList_SetItem(event, 2, pySceneObject::New(eventData->fHitter, fSelfKey));
							PyList_SetItem(event, 3, pySceneObject::New(eventData->fHittee, fSelfKey));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;
						
					case proEventData::kSpawned:
						{
							proSpawnedEventData *eventData = (proSpawnedEventData *)pED;
							PyObject* event = PyList_New(3);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kSpawned));
							PyList_SetItem(event, 1, pySceneObject::New(eventData->fSpawner, fSelfKey));
							PyList_SetItem(event, 2, pySceneObject::New(eventData->fSpawnee, fSelfKey));
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kPicked:
						{
							// get data from the picked event
							proPickedEventData *eventData = (proPickedEventData *)pED;
							// create list
							PyObject* event = PyList_New(6);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kPicked));
							PyList_SetItem(event, 1, PyInt_FromLong(eventData->fEnabled ? 1 : 0));
							PyList_SetItem(event, 2, pySceneObject::New(eventData->fPicker, fSelfKey));
							PyList_SetItem(event, 3, pySceneObject::New(eventData->fPicked, fSelfKey));
							PyList_SetItem(event, 4, pyPoint3::New(eventData->fHitPoint));

							// make it in the local space
							hsPoint3 tolocal(0,0,0);
							if(eventData->fPicked)
							{
								plSceneObject* obj = plSceneObject::ConvertNoRef(eventData->fPicked->ObjectIsLoaded());
								if ( obj )
								{
									const plCoordinateInterface* ci = obj->GetCoordinateInterface();
									if ( ci )
										tolocal = (hsMatrix44)ci->GetWorldToLocal() * eventData->fHitPoint;
								}
							}
							PyList_SetItem(event, 5, pyPoint3::New(tolocal));

							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kControlKey:
						{
							proControlKeyEventData *eventData = (proControlKeyEventData *)pED;
							// create event list
							PyObject* event = PyList_New(3);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kControlKey));
							PyList_SetItem(event, 1, PyLong_FromLong(eventData->fControlKey));
							PyList_SetItem(event, 2, PyInt_FromLong(eventData->fDown ? 1 : 0));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kVariable:
						{
							proVariableEventData *eventData = (proVariableEventData *)pED;
							// create event list
							PyObject* event = PyList_New(4);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kVariable));
							PyList_SetItem(event, 1, PyString_FromString(eventData->fName));
							PyList_SetItem(event, 2, PyLong_FromLong(eventData->fDataType));
							
							// depending on the data type create the data
							switch ( eventData->fDataType )
							{
								case proEventData::kNumber:
									PyList_SetItem(event, 3, PyFloat_FromDouble(eventData->fNumber));
									break;
								case proEventData::kKey:
									PyList_SetItem(event, 3, pyKey::New(eventData->fKey));
									break;
							}
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kFacing:
						{
							proFacingEventData *eventData = (proFacingEventData *)pED;
							// create event list
							PyObject* event = PyList_New(5);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kFacing));
							PyList_SetItem(event, 1, PyInt_FromLong(eventData->enabled ? 1 : 0));
							PyList_SetItem(event, 2, pySceneObject::New(eventData->fFacer, fSelfKey));
							PyList_SetItem(event, 3, pySceneObject::New(eventData->fFacee, fSelfKey));
							PyList_SetItem(event, 4, PyFloat_FromDouble(eventData->dot));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kContained:
						{
							proContainedEventData *eventData = (proContainedEventData *)pED;
							// create event list
							PyObject* event = PyList_New(4);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kContained));
							PyList_SetItem(event, 1, PyInt_FromLong(eventData->fEntering ? 1 : 0));
							PyList_SetItem(event, 2, pySceneObject::New(eventData->fContained, fSelfKey));
							PyList_SetItem(event, 3, pySceneObject::New(eventData->fContainer, fSelfKey));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kActivate:
						{
							proActivateEventData *eventData = (proActivateEventData *)pED;
							// create event list
							PyObject* event = PyList_New(3);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kActivate));
							PyList_SetItem(event, 1, PyInt_FromLong(eventData->fActive ? 1 : 0));
							PyList_SetItem(event, 2, PyInt_FromLong(eventData->fActivate ? 1 : 0));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kCallback:
						{
							proCallbackEventData *eventData = (proCallbackEventData *)pED;
							// create event list
							PyObject* event = PyList_New(2);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kCallback));
							PyList_SetItem(event, 1, PyLong_FromLong(eventData->fEventType));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kResponderState:
						{
							proResponderStateEventData *eventData = (proResponderStateEventData *)pED;
							// create event list
							PyObject* event = PyList_New(2);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kResponderState));
							PyList_SetItem(event, 1, PyLong_FromLong(eventData->fState));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;

					case proEventData::kMultiStage:
						{
							proMultiStageEventData *eventData = (proMultiStageEventData *)pED;
							// create event list
							PyObject* event = PyList_New(4);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kMultiStage));
							PyList_SetItem(event, 1, PyLong_FromLong(eventData->fStage));
							PyList_SetItem(event, 2, PyLong_FromLong(eventData->fEvent));
							PyList_SetItem(event, 3, pySceneObject::New(eventData->fAvatar, fSelfKey));
							// add this event record to the main event list (lists within a list)
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;
					case proEventData::kOfferLinkingBook:
						{
							proOfferLinkingBookEventData* eventData = (proOfferLinkingBookEventData*)pED;
							// create event list
							PyObject* event = PyList_New(4);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kOfferLinkingBook));
							PyList_SetItem(event, 1, pySceneObject::New(eventData->offerer, fSelfKey));
							PyList_SetItem(event, 2, PyInt_FromLong(eventData->targetAge));
							PyList_SetItem(event, 3, PyInt_FromLong(eventData->offeree));
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;
					case proEventData::kBook:
						{
							proBookEventData* eventData = (proBookEventData*)pED;
							// create event list
							PyObject* event = PyList_New(3);
							PyList_SetItem(event, 0, PyLong_FromLong((long)proEventData::kBook));
							PyList_SetItem(event, 1, PyLong_FromUnsignedLong(eventData->fEvent));
							PyList_SetItem(event, 2, PyLong_FromUnsignedLong(eventData->fLinkID));
							PyList_Append(levents, event);
							Py_DECREF(event);
						}
						break;
				}
			}

			// Need to determine which of the Activators sent this plNotifyMsg
			// and set the ID appropriately
			Int32 id = -1;	// assume that none was found
			if ( pNtfyMsg->GetSender() != nil )
			{
				// loop throught the parameters and set them by id
				// (will need to create the appropiate Python object for each type)
				int npm;
				for ( npm=0; npm<GetParameterListCount() ; npm++ )
				{
					plPythonParameter parameter = GetParameterItem(npm);
					// is it something that could produce a plNotifiyMsg?
					if ( parameter.fValueType == plPythonParameter::kActivatorList
					   	|| parameter.fValueType == plPythonParameter::kBehavior 
						|| parameter.fValueType == plPythonParameter::kResponderList )
					{
						// is there an actual ObjectKey to look at?
					    if (parameter.fObjectKey != nil )
						{
							// is it the same as the sender of the notify message?
							if ( pNtfyMsg->GetSender()->GetUoid() == parameter.fObjectKey->GetUoid() )
							{
								// match! Then return that as the ID
								id = parameter.fID;
							}
						}
					}
				}
			}



			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Notify],fFunctionNames[kfunc_Notify],
						"flO",pNtfyMsg->fState,id,levents);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_Notify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(levents);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for a key event message?
	if (fPyFunctionInstances[kfunc_OnKeyEvent] != nil)
	{
		// we are looking for collision messages, is it one?
		plControlEventMsg* pEMsg = plControlEventMsg::ConvertNoRef(msg);
		if (pEMsg)
		{
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnKeyEvent],fFunctionNames[kfunc_OnKeyEvent],
						"ll",pEMsg->GetControlCode(),pEMsg->ControlActivated());
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnKeyEvent] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}

	}

	// are they looking for an Timer message?
	if (fPyFunctionInstances[kfunc_AtTimer])
	{
		// yes, so was there actually a plActivateMsg?
		plTimerCallbackMsg* pTimerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
		if (pTimerMsg)
		{
			// yes...
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_AtTimer],fFunctionNames[kfunc_AtTimer],
						"l",pTimerMsg->fID);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_AtTimer] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for an GUINotify message?
	if (fPyFunctionInstances[kfunc_GUINotify])
	{
		// yes, so was there actually a plActivateMsg?
		pfGUINotifyMsg* pGUIMsg = pfGUINotifyMsg::ConvertNoRef(msg);
		if (pGUIMsg)
		{
			// yes...
			// call it ... but first create the control that started this mess
			// create the key
			PyObject* pyControl = nil;
			if ( pGUIMsg->GetControlKey() )		// make sure there is a control key
			{
				// now create the control... but first we need to find out what it is
				PyObject* pyCtrlKey = pyKey::New(pGUIMsg->GetControlKey());
				UInt32 control_type = pyGUIDialog::WhatControlType(*(pyKey::ConvertFrom(pyCtrlKey)));
				Py_DECREF(pyCtrlKey);

				switch (control_type)
				{
					case pyGUIDialog::kDialog:
						pyControl = pyGUIDialog::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kButton:
						pyControl = pyGUIControlButton::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kListBox:
						pyControl = pyGUIControlListBox::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kTextBox:
						pyControl = pyGUIControlTextBox::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kEditBox:
						pyControl = pyGUIControlEditBox::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kUpDownPair:
					case pyGUIDialog::kKnob:
						pyControl = pyGUIControlValue::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kCheckBox:
						pyControl = pyGUIControlCheckBox::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kRadioGroup:
						pyControl = pyGUIControlRadioGroup::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kDynamicText:
						pyControl = pyGUIControlDynamicText::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kMultiLineEdit:
						pyControl = pyGUIControlMultiLineEdit::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kPopUpMenu:
						pyControl = pyGUIPopUpMenu::New(pGUIMsg->GetControlKey());
						break;

					case pyGUIDialog::kClickMap:
						pyControl = pyGUIControlClickMap::New(pGUIMsg->GetControlKey());
						break;

					default:
						// we don't know what it is... just send 'em the pyKey
						pyControl = pyKey::New(pGUIMsg->GetControlKey());
						break;

				}
			}
			// Need to determine which of the GUIDialogs sent this plGUINotifyMsg
			// and set the ID appropriately
			Int32 id = -1;	// assume that none was found
			if ( pGUIMsg->GetSender() != nil )
			{
				// loop throught the parameters and set them by id
				// (will need to create the appropiate Python object for each type)
				int npm;
				for ( npm=0; npm<GetParameterListCount() ; npm++ )
				{
					plPythonParameter parameter = GetParameterItem(npm);
					// is it something that could produce a plNotifiyMsg?
					if ( parameter.fValueType == plPythonParameter::kGUIDialog || parameter.fValueType == plPythonParameter::kGUIPopUpMenu )
					{
						// is there an actual ObjectKey to look at?
					    if (parameter.fObjectKey != nil )
						{
							// is it the same of the sender of the notify message?
							if ( pGUIMsg->GetSender()->GetUoid() == parameter.fObjectKey->GetUoid() )
							{
								// match! then set the ID to what the parameter is, so the python programmer can find it
								id = parameter.fID;
							}
						}
					}
				}
			}

			// make sure that we found a control to go with this
			if ( pyControl == nil )
			{
				// if none then return a Python None object
				Py_INCREF(Py_None);
				pyControl = Py_None;
			}

			// call their OnGUINotify method
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_GUINotify],fFunctionNames[kfunc_GUINotify],
						"lOl",id,pyControl,pGUIMsg->GetEvent() );
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_GUINotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(pyControl);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for an RoomLoadNotify message?
	if (fPyFunctionInstances[kfunc_PageLoad])
	{
		// yes, so was there actually a plRoomLoadNotifyMsg?
		plRoomLoadNotifyMsg* pRLNMsg = plRoomLoadNotifyMsg::ConvertNoRef(msg);
		if (pRLNMsg)
		{
			// yes...
			// call it
			char* roomname = "";
			if ( pRLNMsg->GetRoom() != nil )
				roomname = (char*)pRLNMsg->GetRoom()->GetName();

			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_PageLoad],fFunctionNames[kfunc_PageLoad],
						"ls",pRLNMsg->GetWhatHappen(),roomname);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_PageLoad] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}


	// are they looking for an ClothingUpdate message?
	if (fPyFunctionInstances[kfunc_ClothingUpdate])
	{
		// yes, so was there actually a plClothingUpdateBCMsg?
		plClothingUpdateBCMsg* pCUMsg = plClothingUpdateBCMsg::ConvertNoRef(msg);
		if (pCUMsg)
		{
			// yes...
			// call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_ClothingUpdate],fFunctionNames[kfunc_ClothingUpdate],nil);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_ClothingUpdate] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for an KIMsg message?
	if (fPyFunctionInstances[kfunc_KIMsg])
	{
		// yes, so was there actually a pfKIMsg?
		pfKIMsg* pkimsg = pfKIMsg::ConvertNoRef(msg);
		if (pkimsg && pkimsg->GetCommand() != pfKIMsg::kHACKChatMsg)
		{
			// yes...
			// find the value that would go with a command
			PyObject* value;
			std::wstring str;
			switch (pkimsg->GetCommand())
			{
				case pfKIMsg::kSetChatFadeDelay:
					value = PyFloat_FromDouble(pkimsg->GetDelay());
					break;
				case pfKIMsg::kSetTextChatAdminMode:
					value = PyLong_FromLong(pkimsg->GetFlags()&pfKIMsg::kAdminMsg ? 1 : 0 );
					break;
				case pfKIMsg::kYesNoDialog:
					value = PyTuple_New(2);
					str = pkimsg->GetStringU();
					PyTuple_SetItem(value, 0, PyUnicode_FromWideChar(str.c_str(), str.length()));
					PyTuple_SetItem(value, 1, pyKey::New(pkimsg->GetSender()));
					break;
				case pfKIMsg::kGZInRange:
					value = PyTuple_New(2);
					PyTuple_SetItem(value, 0, PyLong_FromLong(pkimsg->GetIntValue()));
					PyTuple_SetItem(value, 1, pyKey::New(pkimsg->GetSender()));
					break;
				case pfKIMsg::kRateIt:
					value = PyTuple_New(3);
					str = pkimsg->GetStringU();
					PyTuple_SetItem(value,0,PyString_FromString(pkimsg->GetUser()));
					PyTuple_SetItem(value,1,PyUnicode_FromWideChar(str.c_str(), str.length()));
					PyTuple_SetItem(value,2,PyLong_FromLong(pkimsg->GetIntValue()));
					break;
				case pfKIMsg::kRegisterImager:
					value = PyTuple_New(2);
					str = pkimsg->GetStringU();
					PyTuple_SetItem(value, 0, PyUnicode_FromWideChar(str.c_str(), str.length()));
					PyTuple_SetItem(value, 1, pyKey::New(pkimsg->GetSender()));
					break;
				case pfKIMsg::kAddPlayerDevice:
				case pfKIMsg::kRemovePlayerDevice:
					{
						str = pkimsg->GetStringU();
						if ( str.length() > 0 )
							value = PyUnicode_FromWideChar(str.c_str(), str.length());
						else
						{
							Py_INCREF(Py_None);
							value = Py_None;
						}
					}
					break;
				case pfKIMsg::kKIChatStatusMsg:
				case pfKIMsg::kKILocalChatStatusMsg:
				case pfKIMsg::kKILocalChatErrorMsg:
				case pfKIMsg::kKIOKDialog:
				case pfKIMsg::kKIOKDialogNoQuit:
				case pfKIMsg::kGZFlashUpdate:
				case pfKIMsg::kKICreateMarkerNode:
					str = pkimsg->GetStringU();
					value = PyUnicode_FromWideChar(str.c_str(), str.length());
					break;
				case pfKIMsg::kMGStartCGZGame:
				case pfKIMsg::kMGStopCGZGame:
				case pfKIMsg::kFriendInviteSent:
				default:
					value = PyLong_FromLong(pkimsg->GetIntValue());
					break;
			}

			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_KIMsg],fFunctionNames[kfunc_KIMsg],
						"lO",pkimsg->GetCommand(),value);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_KIMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(value);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for an MemberUpdate message?
	if (fPyFunctionInstances[kfunc_MemberUpdate])
	{
		// yes, so was there actually a plMemberUpdateMsg?
		plMemberUpdateMsg* pmumsg = plMemberUpdateMsg::ConvertNoRef(msg);
		if (pmumsg)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_MemberUpdate],fFunctionNames[kfunc_MemberUpdate],nil);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_MemberUpdate] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for a RemoteAvatar Info message?
	if (fPyFunctionInstances[kfunc_RemoteAvatarInfo])
	{
		// yes, so was there actually a plActivateMsg?
		plRemoteAvatarInfoMsg* pramsg = plRemoteAvatarInfoMsg::ConvertNoRef(msg);
		if (pramsg)
		{
			// yes...
			PyObject* player;
			// if there was no avatar key in the message
			if ( pramsg->GetAvatarKey() == nil )
			{
				// then just return a None... same thing as nil.. which I guess means a non-avatar is selected
				player = PyInt_FromLong(0);
			}
			else
			{
				// try to create the pyPlayer for where this message came from
				int mbrIndex = plNetClientMgr::GetInstance()->TransportMgr().FindMember(pramsg->GetAvatarKey());
				if ( mbrIndex != -1 )
				{
					plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMember( mbrIndex );
					player = pyPlayer::New(mbr->GetAvatarKey(), mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
				}
				else
				{
					// else if we could not find the player in our list, then no avatar selected
					player = PyInt_FromLong(0);
				}
			}

			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_RemoteAvatarInfo],fFunctionNames[kfunc_RemoteAvatarInfo],
						"O",player);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_RemoteAvatarInfo] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(player);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}


	// are they looking for a CCR communication message?
	if (fPyFunctionInstances[kfunc_OnCCRMsg])
	{
		// yes, so was there actually a plActivateMsg?
		plCCRCommunicationMsg* ccrmsg = plCCRCommunicationMsg::ConvertNoRef(msg);
		if (ccrmsg)
		{
			const char* textmessage = ccrmsg->GetMessage();
			if ( textmessage == nil)
				textmessage = "";
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnCCRMsg],fFunctionNames[kfunc_OnCCRMsg],
						"lsl",ccrmsg->GetType(),textmessage,ccrmsg->GetCCRPlayerID());
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnCCRMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}


	// are they looking for a VaultNotify message?
	if (fPyFunctionInstances[kfunc_OnVaultNotify])
	{
		// yes, so was there actually a plVaultNotifyMsg?
		if (plVaultNotifyMsg * vaultNotifyMsg = plVaultNotifyMsg::ConvertNoRef(msg))
		{
			if ( hsSucceeded( vaultNotifyMsg->GetResultCode() ) )
			{
				// Create a tuple for second argument according to msg type.
				// Default to an empty tuple.
				PyObject* ptuple = PyTuple_New(0);
				switch ( vaultNotifyMsg->GetType() )
				{
					case plVaultNotifyMsg::kRegisteredOwnedAge:
					case plVaultNotifyMsg::kRegisteredVisitAge:
					case plVaultNotifyMsg::kUnRegisteredOwnedAge:
					case plVaultNotifyMsg::kUnRegisteredVisitAge: {
						if (RelVaultNode * rvn = VaultGetNodeIncRef(vaultNotifyMsg->GetArgs()->GetInt(plNetCommon::VaultTaskArgs::kAgeLinkNode))) {
							Py_DECREF(ptuple);
							ptuple = PyTuple_New(1);
							PyTuple_SetItem(ptuple, 0, pyVaultAgeLinkNode::New(rvn));
							rvn->DecRef();
						}
					}
					break;
					
					case plVaultNotifyMsg::kPublicAgeCreated:
					case plVaultNotifyMsg::kPublicAgeRemoved: {
						if (const char * ageName = vaultNotifyMsg->GetArgs()->GetString(plNetCommon::VaultTaskArgs::kAgeFilename)) {
							Py_DECREF(ptuple);
							ptuple = PyTuple_New(1);
							PyTuple_SetItem(ptuple, 0, PyString_FromString(ageName));
						}
					}
					break;
				}

				plProfile_BeginTiming(PythonUpdate);
				PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnVaultNotify],fFunctionNames[kfunc_OnVaultNotify],
							"lO",vaultNotifyMsg->GetType(),ptuple);
				if ( retVal == nil )
				{
#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_OnVaultNotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				Py_DECREF(ptuple);
				plProfile_EndTiming(PythonUpdate);
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
				// we handled this message (I think)
			}
			return true;
		}
	}

	
	// are they looking for a RealTimeChat message?
	if (fPyFunctionInstances[kfunc_RTChat])
	{
		// yes, so was there actually a pfKIMsg?
		pfKIMsg* pkimsg = pfKIMsg::ConvertNoRef(msg);
		if (pkimsg && pkimsg->GetCommand() == pfKIMsg::kHACKChatMsg)
		{
			// yes...
			// filter ignored player
			if ( !VaultAmIgnoringPlayer( pkimsg->GetPlayerID() ) )
			{
				// create the pyPlayer for where this message came from
				PyObject* player;
				PyObject* ptPlayerClass = PythonInterface::GetPlasmaItem("ptPlayer");
				hsAssert(ptPlayerClass,"Could not create a ptPlayer");
				int mbrIndex = plNetClientMgr::GetInstance()->TransportMgr().FindMember(pkimsg->GetPlayerID());
				if ( mbrIndex != -1 )
				{
					plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMember( mbrIndex );
					player = pyPlayer::New(mbr->GetAvatarKey(), pkimsg->GetUser(), mbr->GetPlayerID(), mbr->GetDistSq());
				}
				else
				{
					// else if we could not find the player in our list, then just return a string of the user's name
					const char * fromName = pkimsg->GetUser();
					if (!fromName)
						fromName = "Anonymous Coward";
					player = pyPlayer::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fromName, pkimsg->GetPlayerID(), 0.0);
				}

				plProfile_BeginTiming(PythonUpdate);
				PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_RTChat],fFunctionNames[kfunc_RTChat],
							"Osl",player,pkimsg->GetString().c_str(),pkimsg->GetFlags());
				if ( retVal == nil )
				{
#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_RTChat] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				Py_DECREF(player);
				plProfile_EndTiming(PythonUpdate);
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
				// we handled this message (I think)
				return true;
			}
		}
	}
	if (plPlayerPageMsg::ConvertNoRef(msg))
	{
		if (fPyFunctionInstances[kfunc_AvatarPage])
		{
			// yes, so was there actually a player page msg
			plPlayerPageMsg* ppMsg = plPlayerPageMsg::ConvertNoRef(msg);
			if (ppMsg)
			{
				PyObject* pSobj = pySceneObject::New(ppMsg->fPlayer, fSelfKey);
				plProfile_BeginTiming(PythonUpdate);
				plSynchEnabler ps(true);	// enable dirty state tracking during shutdown	
	
				PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_AvatarPage],fFunctionNames[kfunc_AvatarPage],
							"Oli",pSobj,!ppMsg->fUnload,ppMsg->fLastOut);
				if ( retVal == nil )
				{
	#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_AvatarPage] = nil;
	#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				Py_DECREF(pSobj);
				plProfile_EndTiming(PythonUpdate);
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
				// we handled this message (I think)
				return true;
			}
		}
	}
	if (plAgeBeginLoadingMsg::ConvertNoRef(msg))
	{
		if (fPyFunctionInstances[kfunc_OnBeginAgeLoad])
		{
			// yes, so was there actually a player page msg
			plAgeBeginLoadingMsg* ppMsg = plAgeBeginLoadingMsg::ConvertNoRef(msg);
			if (ppMsg)
			{
				PyObject* pSobj = pySceneObject::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fSelfKey);
				plProfile_BeginTiming(PythonUpdate);
				plSynchEnabler ps(true);	// enable dirty state tracking during shutdown	
	
				PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBeginAgeLoad],fFunctionNames[kfunc_OnBeginAgeLoad],
							"O",pSobj);
				if ( retVal == nil )
				{
	#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_OnBeginAgeLoad] = nil;
	#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
				}
				Py_XDECREF(retVal);
				Py_DECREF(pSobj);
				plProfile_EndTiming(PythonUpdate);
				// display any output (NOTE: this would be disabled in production)
				DisplayPythonOutput();
				// we handled this message (I think)
				return true;
			}
		}
	}
	if (plInitialAgeStateLoadedMsg::ConvertNoRef(msg))// initial server update complete message
	{
		// make sure there is a valid python instance
		if ( fInstance )
		{
			// set the isInitialStateLoaded to that it is loaded
			PyObject* pInitialState = PyInt_FromLong(1);
			PyObject_SetAttrString(fInstance, "isInitialStateLoaded", pInitialState);
			Py_DECREF(pInitialState);
		}
		if (fPyFunctionInstances[kfunc_OnServerInitComplete])
		{
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnServerInitComplete],fFunctionNames[kfunc_OnServerInitComplete],nil);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
					// for some reason this function didn't, remember that and not call it again
					fPyFunctionInstances[kfunc_OnServerInitComplete] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
					// if there was an error make sure that the stderr gets flushed so it can be seen
					ReportError();
			}
			Py_XDECREF(retVal);

			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}
	// are they looking for an plSDLNotificationMsg message?
	if (fPyFunctionInstances[kfunc_SDLNotify])
	{
		// yes, so was there actually a plSDLNotificationMsg?
		plSDLNotificationMsg* sn = plSDLNotificationMsg::ConvertNoRef(msg);
		if (sn)
		{
			const char* tag = sn->fHintString.c_str();
			if (tag == nil)
				tag = "";
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_SDLNotify],fFunctionNames[kfunc_SDLNotify],
						"ssls",sn->fVar->GetName(),sn->fSDLName.c_str(),sn->fPlayerID,tag);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_SDLNotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}
	// are they looking for an plNetOwnershipMsg message?
	if (fPyFunctionInstances[kfunc_OwnershipNotify])
	{
		// yes, so was there actually a plNetOwnershipMsg?
		plNetOwnershipMsg* nom = plNetOwnershipMsg::ConvertNoRef(msg);
		if (nom)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OwnershipNotify],fFunctionNames[kfunc_OwnershipNotify],
						nil);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OwnershipNotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}
	// are they looking for an pfMarkerMsg message?
	if (fPyFunctionInstances[kfunc_OnMarkerMsg])
	{
		pfMarkerMsg* markermsg = pfMarkerMsg::ConvertNoRef(msg);
		if (markermsg)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			// Default to an empty tuple.
			PyObject* ptuple = PyTuple_New(0);
			switch ( markermsg->fType )
			{
				case pfMarkerMsg::kMarkerCaptured:
					// Sent when we collide with a marker
					Py_DECREF(ptuple);
					ptuple = PyTuple_New(1);
					PyTuple_SetItem(ptuple, 0, PyLong_FromUnsignedLong((long)markermsg->fMarkerID));
					break;
			}

			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnMarkerMsg], fFunctionNames[kfunc_OnMarkerMsg],
						"lO", (UInt32)markermsg->fType, ptuple);
			if (retVal == nil)
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnMarkerMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(ptuple);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

#ifndef PLASMA_EXTERNAL_RELEASE
	// are they looking for an pfDebugTriggerMsg message?
	if (fPyFunctionInstances[kfunc_OnBackdoorMsg])
	{
		// yes, so was there actually a plNetOwnershipMsg?
		pfBackdoorMsg* dt = pfBackdoorMsg::ConvertNoRef(msg);
		if (dt)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBackdoorMsg],
						fFunctionNames[kfunc_OnBackdoorMsg],
						"ss",dt->GetTarget(),dt->GetString());
			if ( retVal == nil )
			{
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}
#endif  //PLASMA_EXTERNAL_RELEASE

	// are they looking for a plLOSHitMsg message?
	if (fPyFunctionInstances[kfunc_OnLOSNotify])
	{
		// yes, so was there actually a plLOSHitMsg?
		plLOSHitMsg *pLOSMsg = plLOSHitMsg::ConvertNoRef( msg );
		if (pLOSMsg)
		{
			// yes... then call it (self,ID,noHitFlag,sceneobject,hitPoint,distance)
			plProfile_BeginTiming(PythonUpdate);
			PyObject* scobj;
			PyObject* hitpoint;
			if ( pLOSMsg->fObj && plSceneObject::ConvertNoRef( pLOSMsg->fObj->ObjectIsLoaded()) )
			{
				scobj = pySceneObject::New(pLOSMsg->fObj);
				hitpoint = pyPoint3::New(pLOSMsg->fHitPoint);
			}
			else
			{
				// otherwise return a None object for the avatarKey
				Py_INCREF(Py_None);
				scobj = Py_None;
				Py_INCREF(Py_None);
				hitpoint = Py_None;
			}
					
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnLOSNotify],
						fFunctionNames[kfunc_OnLOSNotify],
						"llOOf",pLOSMsg->fRequestID,pLOSMsg->fNoHit,
						scobj, hitpoint, pLOSMsg->fDistance);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnLOSNotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(scobj);
			Py_DECREF(hitpoint);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for a plAvatarBehaviorNotifyMsg message?
	if (fPyFunctionInstances[kfunc_OnBehaviorNotify])
	{
		// yes, so was there actually a plAvatarBehaviorNotifyMsg?
		plAvatarBehaviorNotifyMsg *behNotifymsg = plAvatarBehaviorNotifyMsg::ConvertNoRef(msg);
		if (behNotifymsg)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			// the parent of the sender should be the avatar that did the behavior
			PyObject* pSobj;
				
			plModifier* avmod = plModifier::ConvertNoRef(behNotifymsg->GetSender()->ObjectIsLoaded());
			if ( avmod && avmod->GetNumTargets() > 0 )
			{
				pSobj = pySceneObject::New(avmod->GetTarget(0)->GetKey(), fSelfKey);
			}
			else
			{
				// otherwise return a None object for the avatarKey
				Py_INCREF(Py_None);
				pSobj = Py_None;
			}
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBehaviorNotify],
						fFunctionNames[kfunc_OnBehaviorNotify],
						"lOl",behNotifymsg->fType,pSobj,behNotifymsg->state);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnBehaviorNotify] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(pSobj);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for a pfMovieEventMsg message?
	if (fPyFunctionInstances[kfunc_OnMovieEvent])
	{
		// yes, so was there actually a pfMovieEventMsg?
		pfMovieEventMsg *moviemsg = pfMovieEventMsg::ConvertNoRef(msg);
		if (moviemsg)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnMovieEvent],
						fFunctionNames[kfunc_OnMovieEvent],
						"si",moviemsg->fMovieName,(UInt32)moviemsg->fReason);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnMovieEvent] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	// are they looking for a plCaptureRenderMsg message?
	if (fPyFunctionInstances[kfunc_OnScreenCaptureDone])
	{
		// yes, so was there actually a pfMovieEventMsg?
		plCaptureRenderMsg *capturemsg = plCaptureRenderMsg::ConvertNoRef(msg);
		if (capturemsg)
		{
			// yes... then call it
			plProfile_BeginTiming(PythonUpdate);
			PyObject* pSobj;
				
			if ( capturemsg->GetMipmap() )
			{
				pSobj = pyImage::New(capturemsg->GetMipmap());
			}
			else
			{
				// otherwise return a None object for the avatarKey
				Py_INCREF(Py_None);
				pSobj = Py_None;
			}
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnScreenCaptureDone],
						fFunctionNames[kfunc_OnScreenCaptureDone],
						"O",pSobj);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnScreenCaptureDone] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(pSobj);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();
			// we handled this message (I think)
			return true;
		}
	}

	if (fPyFunctionInstances[kfunc_OnClimbBlockerEvent])
	{
		plClimbEventMsg* pEvent = plClimbEventMsg::ConvertNoRef(msg);
		if (pEvent)
		{
			PyObject* pSobj = pySceneObject::New(pEvent->GetSender(), fSelfKey);
			
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnClimbBlockerEvent],
						fFunctionNames[kfunc_OnClimbBlockerEvent],
						"O",pSobj);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnClimbBlockerEvent] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			Py_DECREF(pSobj);
			return true;
		}
	}
	if (fPyFunctionInstances[kfunc_OnAvatarSpawn])
	{
		plAvatarSpawnNotifyMsg* pSpawn = plAvatarSpawnNotifyMsg::ConvertNoRef(msg);
		if (pSpawn)
		{
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnAvatarSpawn],
						fFunctionNames[kfunc_OnAvatarSpawn],
						"l",1);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnAvatarSpawn] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			return true;
		}
	}
	
	if (fPyFunctionInstances[kfunc_OnAccountUpdate])
	{
		plAccountUpdateMsg* pUpdateMsg = plAccountUpdateMsg::ConvertNoRef(msg);
		if (pUpdateMsg)
		{
			plProfile_BeginTiming(PythonUpdate);
			PyObject* retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnAccountUpdate], fFunctionNames[kfunc_OnAccountUpdate],
				"iii", (int)pUpdateMsg->GetUpdateType(), (int)pUpdateMsg->GetResult(), (int)pUpdateMsg->GetPlayerInt()
			);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnAccountUpdate] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();

			return true;
		}
	}

	if (fPyFunctionInstances[kfunc_gotPublicAgeList])
	{
		plNetCommPublicAgeListMsg * pPubAgeMsg = plNetCommPublicAgeListMsg::ConvertNoRef(msg);
		if (pPubAgeMsg)
		{
			plProfile_BeginTiming(PythonUpdate);
			PyObject* pyEL = PyList_New(pPubAgeMsg->ages.Count());
			for (unsigned i = 0; i<pPubAgeMsg->ages.Count(); ++i) {
				plAgeInfoStruct ageInfo;
				ageInfo.CopyFrom(pPubAgeMsg->ages[i]);
				unsigned nPlayers = pPubAgeMsg->ages[i].currentPopulation;
				unsigned nOwners = pPubAgeMsg->ages[i].population;
				
				PyObject* t = PyTuple_New(3);
				PyTuple_SetItem(t, 0, pyAgeInfoStruct::New(&ageInfo));
				PyTuple_SetItem(t, 1, PyLong_FromUnsignedLong(nPlayers));
				PyTuple_SetItem(t, 2, PyLong_FromUnsignedLong(nOwners));
				PyList_SetItem(pyEL, i, t); // steals the ref
			}
			
			PyObject* retVal = PyObject_CallMethod(
				fPyFunctionInstances[kfunc_gotPublicAgeList],
				fFunctionNames[kfunc_gotPublicAgeList],
				"O",
				pyEL
			);
			if ( retVal == nil )
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_gotPublicAgeList] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output (NOTE: this would be disabled in production)
			DisplayPythonOutput();

			return true;
		}
	}

	if (fPyFunctionInstances[kfunc_OnGameMgrMsg])
	{
		pfGameMgrMsg* gameMgrMsg = pfGameMgrMsg::ConvertNoRef(msg);
		if (gameMgrMsg)
		{
			plProfile_BeginTiming(PythonUpdate);
			PyObject* pythonMsg = pyGameMgrMsg::New(gameMgrMsg);
			PyObject* retVal = PyObject_CallMethod(
				fPyFunctionInstances[kfunc_OnGameMgrMsg],
				fFunctionNames[kfunc_OnGameMgrMsg],
				"O",
				pythonMsg
			);
			Py_DECREF(pythonMsg);
			if (retVal == nil)
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnGameMgrMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output
			DisplayPythonOutput();

			return true;
		}
	}

	if (fPyFunctionInstances[kfunc_OnGameCliMsg])
	{
		pfGameCliMsg* gameMgrMsg = pfGameCliMsg::ConvertNoRef(msg);
		if (gameMgrMsg)
		{
			plProfile_BeginTiming(PythonUpdate);
			PyObject* pythonMsg = pyGameCliMsg::New(gameMgrMsg);
			PyObject* retVal = PyObject_CallMethod(
				fPyFunctionInstances[kfunc_OnGameCliMsg],
				fFunctionNames[kfunc_OnGameCliMsg],
				"O",
				pythonMsg
			);
			Py_DECREF(pythonMsg);
			if (retVal == nil)
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnGameCliMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);
			plProfile_EndTiming(PythonUpdate);
			// display any output
			DisplayPythonOutput();

			return true;
		}
	}

	if (fPyFunctionInstances[kfunc_OnAIMsg])
	{
		plAIMsg* aiMsg = plAIMsg::ConvertNoRef(msg);
		if (aiMsg)
		{
			plProfile_BeginTiming(PythonUpdate);

			// grab the sender (the armature mod that has our brain)
			plArmatureMod* armMod = plArmatureMod::ConvertNoRef(aiMsg->GetSender()->ObjectIsLoaded());
			PyObject* brainObj = NULL;
			if (armMod)
			{
				plArmatureBrain* brain = armMod->FindBrainByClass(plAvBrainCritter::Index());
				plAvBrainCritter* critterBrain = plAvBrainCritter::ConvertNoRef(brain);
				if (critterBrain)
					brainObj = pyCritterBrain::New(critterBrain);
			}
			if (!brainObj)
			{
				Py_INCREF(Py_None);
				brainObj = Py_None;
			}

			// set up the msg type and any args, based on the message we got
			int msgType = plAIMsg::kAIMsg_Unknown;
			PyObject* args = NULL;
			plAIBrainCreatedMsg* brainCreatedMsg = plAIBrainCreatedMsg::ConvertNoRef(aiMsg);
			if (brainCreatedMsg)
				msgType = plAIMsg::kAIMsg_BrainCreated;

			plAIArrivedAtGoalMsg* arrivedMsg = plAIArrivedAtGoalMsg::ConvertNoRef(aiMsg);
			if (arrivedMsg)
			{
				msgType = plAIMsg::kAIMsg_ArrivedAtGoal;
				args = PyTuple_New(1);
				PyTuple_SetItem(args, 0, pyPoint3::New(arrivedMsg->Goal()));
			}

			// if no args were set, simply set to none
			if (!args)
			{
				Py_INCREF(Py_None);
				args = Py_None;
			}

			// call the function with the above arguments
			PyObject* retVal = PyObject_CallMethod(
				fPyFunctionInstances[kfunc_OnAIMsg],
				fFunctionNames[kfunc_OnAIMsg],
				"OisO",
				brainObj, msgType, aiMsg->BrainUserString().c_str(), args
			);
			Py_DECREF(brainObj);
			Py_DECREF(args);
			if (retVal == nil)
			{
#ifndef PLASMA_EXTERNAL_RELEASE
				// for some reason this function didn't, remember that and not call it again
				fPyFunctionInstances[kfunc_OnAIMsg] = nil;
#endif  //PLASMA_EXTERNAL_RELEASE
				// if there was an error make sure that the stderr gets flushed so it can be seen
				ReportError();
			}
			Py_XDECREF(retVal);

			plProfile_EndTiming(PythonUpdate);
			// display any output
			DisplayPythonOutput();

			return true;
		}
	}

	return plModifier::MsgReceive(msg);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ReportError
//  PARAMETERS : 
//
//  PURPOSE    : Report error to somewhere
//
void plPythonFileMod::ReportError()
{
	char objectName[128];
	StrCopy(objectName, this->GetKeyName(), arrsize(objectName));
	StrPack(objectName, " - ", arrsize(objectName));

	PythonInterface::WriteToStdErr(objectName);

	PyErr_Print();		// make sure the error is printed
	PyErr_Clear();		// clear the error
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DisplayPythonOutput
//  PARAMETERS : 
//
//  PURPOSE    : display any Python stdout or stderr to file and to screen(later)
//
void plPythonFileMod::DisplayPythonOutput()
{
	// get the messages
	PythonInterface::getOutputAndReset();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetSourceFile
//  PARAMETERS : code      - text source code
//             : filename  - where the source code came from (just say the object name)
//
//  PURPOSE    : Sets the source code for this modifier.
//             : Compile it into a Python code object
//             : (This is usually called by the component)
//
void plPythonFileMod::SetSourceFile(const char* filename)
{
	delete [] fPythonFile;
	fPythonFile = hsStrcpy(filename);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : getPythonOutput
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the error file to be displayed
//
int  plPythonFileMod::getPythonOutput(std::string* line)
{
	 return PythonInterface::getOutputAndReset(line);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnableControlKeys
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the error file to be displayed
//
void plPythonFileMod::EnableControlKeyEvents()
{
	// register for keyboard events if needed
	if ( fPyFunctionInstances[kfunc_OnKeyEvent] != nil )
	{
		// register for key events
		plCmdIfaceModMsg* pModMsg = TRACKED_NEW plCmdIfaceModMsg;
		pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pModMsg->SetSender(GetKey());
		pModMsg->SetCmd(plCmdIfaceModMsg::kAdd);
		plgDispatch::MsgSend(pModMsg);
	}
}

	
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DisableControlKeys
//  PARAMETERS : none
//
//  PURPOSE    : get the Output to the error file to be displayed
//
void plPythonFileMod::DisableControlKeyEvents()
{
	// unregister for key events
	plCmdIfaceModMsg* pModMsg = TRACKED_NEW plCmdIfaceModMsg;
	pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
	pModMsg->SetSender(GetKey());
	pModMsg->SetCmd(plCmdIfaceModMsg::kRemove);
	plgDispatch::MsgSend(pModMsg);
}

void plPythonFileMod::Read(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Read(stream, mgr);

	// read in the compile python code (pyc)
	if ( fPythonFile )
	{
		// if we already have some code, get rid of it!
		delete [] fPythonFile;
		fPythonFile = nil;
	}
	fPythonFile = stream->ReadSafeString();

	// then read in the list of receivers that want to be notified
	int nRcvs = stream->ReadSwap32();
	fReceivers.Reset();
	int m;
	for( m=0; m<nRcvs; m++ )
	{	
		fReceivers.Append(mgr->ReadKey(stream));
	}

	// then read in the list of parameters
	int nParms = stream->ReadSwap32();
	fParameters.SetCountAndZero(nParms);
	int i;
	for( i=0; i<nParms; i++ )
	{
		plPythonParameter parm;
		parm.Read(stream,mgr);
		fParameters[i] = parm;
	}	
}

void plPythonFileMod::Write(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Write(stream, mgr);

	stream->WriteSafeString(fPythonFile);

	// then write out the list of receivers that want to be notified
	stream->WriteSwap32(fReceivers.GetCount());
	int m;
	for( m=0; m<fReceivers.GetCount(); m++ )
		mgr->WriteKey(stream, fReceivers[m]);

	// then write out the list of parameters
	stream->WriteSwap32(fParameters.GetCount());
	int i;
	for( i=0; i<fParameters.GetCount(); i++ )
		fParameters[i].Write(stream,mgr);
}

//// kGlobalNameKonstant /////////////////////////////////////////////////
//	My continued attempt to spread the CORRECT way to spell konstant. -mcn

char plPythonFileMod::kGlobalNameKonstant[] = "VeryVerySpecialPythonFileMod";
