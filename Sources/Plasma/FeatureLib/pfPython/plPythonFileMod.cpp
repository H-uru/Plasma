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
// plPythonFileMod   - the 'special' Python File modifier.
//
// This modifier will handle the interface to python code that has been file-ized.
//
//////////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <locale>
#include "HeadSpin.h"
#include "plgDispatch.h"
#include "pyGeometry3.h"
#include "pyKey.h"
#include "pyObjectRef.h"
#include "hsResMgr.h"
#include "hsStream.h"
#pragma hdrstop

#include "plPythonFileMod.h"

#include "plResMgr/plKeyFinder.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnKeyedObject/plUoid.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plModifier/plLogicModifier.h"
#include "pfMessage/pfGUINotifyMsg.h"
#include "plMessage/plRoomLoadNotifyMsg.h"
#include "pfMessage/plClothingMsg.h"
#include "pfMessage/pfKIMsg.h"
#include "plMessage/plMemberUpdateMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "pnMessage/plRemoteAvatarInfoMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetTransport/plNetTransportMember.h"
#include "pnMessage/plSDLNotificationMsg.h"
#include "plMessage/plNetOwnershipMsg.h"
#include "plSDL/plSDL.h"
#include "plVault/plVault.h"
#include "plMessage/plCCRMsg.h"
#include "plMessage/plVaultNotifyMsg.h"
#include "plInputCore/plInputInterfaceMgr.h"
#include "plInputCore/plInputDevice.h"
#include "pfMessage/pfMarkerMsg.h"
#include "pfMessage/pfBackdoorMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plRenderMsg.h"
#include "pfMessage/pfMovieEventMsg.h"
#include "plMessage/plClimbEventMsg.h"
#include "plMessage/plCaptureRenderMsg.h"
#include "plGImage/plMipmap.h"
#include "plMessage/plAccountUpdateMsg.h"
#include "plAgeLoader/plAgeLoader.h"
#include "plMessage/plAIMsg.h"
#include "plAvatar/plAvBrainCritter.h"
#include "pfMessage/pfGameScoreMsg.h"

#include "plProfile.h"

#include "cyPythonInterface.h"
#include "cyDraw.h"
#include "cyPhysics.h"
#include "pySceneObject.h"
#include "cyMisc.h"
#include "cyCamera.h"
#include "pyNotify.h"
#include "cyAvatar.h"
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

#include "pyGameScoreMsg.h"

#include "plPythonSDLModifier.h"

#include "plMessage/plTimerCallbackMsg.h"

plProfile_CreateTimer("Update", "Python", PythonUpdate);

/////////////////////////////////////////////////////////////////////////////
//
// fFunctionNames    - the actual names of the functions for On[event] types
//
const char* plPythonFileMod::fFunctionNames[] = 
{
    "OnFirstUpdate",        // kfunc_FirstUpdate
    "OnUpdate",             // kfunc_Update
    "OnNotify",             // kfunc_Notify
    "OnTimer",              // kfunc_AtTimer
    "OnControlKeyEvent",    // kfunc_OnKeyEvent
    "Load",                 // kfunc_Load
    "Save",                 // kfunc_Save
    "OnGUINotify",          // kfunc_GUINotify
    "OnPageLoad",           // kfunc_PageLoad
    "OnClothingUpdate",     // kfunc_ClothingUpdate
    "OnKIMsg",              // kfunc_KIMsg,
    "OnMemberUpdate",       // kfunc_MemberUpdate,
    "OnRemoteAvatarInfo",   // kfunc_RemoteAvatarInfo,
    "OnRTChat",             // kfunc_RTChat,
    "OnVaultEvent",         // kfunc_VaultEvent,
    "AvatarPage",           // kfunc_AvatarPage,
    "OnSDLNotify",          // kfunc_SDLNotify
    "OnOwnershipChanged",   // kfunc_OwnershipNotify
    "OnAgeVaultEvent",      // kfunc_AgeVaultEvent
    "OnInit",               // kfunc_Init,
    "OnCCRMsg",             // kfunc_OnCCRMsg,
    "OnServerInitComplete", // kfunc_OnServerInitComplete
    "OnVaultNotify",        // kfunc_OnVaultNotify
    "OnDefaultKeyCaught",   // kfunc_OnDefaultKeyCaught
    "OnMarkerMsg",          // kfunc_OnMarkerMsg,
    "OnBackdoorMsg",        // kfunc_OnBackdoorMsg,
    "OnBehaviorNotify",     // kfunc_OnBehaviorNotify,
    "OnLOSNotify",          // kfunc_OnLOSNotify,
    "BeginAgeUnLoad",       // kfunc_OnBeginAgeLoad,
    "OnMovieEvent",         // kfunc_OnMovieEvent,
    "OnScreenCaptureDone",  // kfunc_OnScreenCaptureDone,
    "OnClimbingBlockerEvent",// kFunc_OnClimbingBlockerEvent,
    "OnAvatarSpawn",        // kFunc_OnAvatarSpawn
    "OnAccountUpdate",      // kFunc_OnAccountUpdate
    "gotPublicAgeList",     // kfunc_gotPublicAgeList
    "OnAIMsg",              // kfunc_OnAIMsg
    "OnGameScoreMsg",       // kfunc_OnGameScoreMsg
    nullptr
};

//// Callback From the Vault Events //////////////////////////////////////////////
class PythonVaultCallback : public VaultCallback
{
protected:
    plPythonFileMod* fPyFileMod;
    int     fFunctionIdx;

public:
    PythonVaultCallback(plPythonFileMod* pymod, int fidx)
        : fPyFileMod(pymod), fFunctionIdx(fidx)
    {
    }

    void AddedChildNode(RelVaultNode* parentNode, RelVaultNode* childNode)
    {
        // is there an 'OnVaultEvent' defined?
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            pyObjectRef ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple.Get(), 0, pyVaultNodeRef::New(parentNode, childNode));

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
                                                     const_cast<char*>(fPyFileMod->fFunctionNames[fFunctionIdx]),
                                                     _pycs("lO"), pyVault::kVaultNodeRefAdded,
                                                     ptuple.Get());
            if (!retVal)
                fPyFileMod->ReportError();
            plProfile_EndTiming(PythonUpdate);
            fPyFileMod->DisplayPythonOutput();
        }
    }

    void RemovingChildNode(RelVaultNode* parentNode, RelVaultNode* childNode)
    {
        // is there an 'OnVaultEvent' defined?
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            pyObjectRef ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple.Get(), 0, pyVaultNodeRef::New(parentNode, childNode));

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
                                                     const_cast<char*>(fPyFileMod->fFunctionNames[fFunctionIdx]),
                                                     _pycs("lO"), pyVault::kVaultRemovingNodeRef,
                                                     ptuple.Get());
            if (!retVal)
                fPyFileMod->ReportError();
            plProfile_EndTiming(PythonUpdate);
        }
    }

    void ChangedNode(RelVaultNode* changedNode)
    {
        // is there an 'OnVaultEvent' defined?
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            pyObjectRef ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple.Get(), 0, pyVaultNode::New(changedNode));

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFileMod->fPyFunctionInstances[fFunctionIdx],
                                                     const_cast<char*>(fPyFileMod->fFunctionNames[fFunctionIdx]),
                                                     _pycs("lO"), pyVault::kVaultNodeSaved,
                                                     ptuple.Get());
            if (!retVal)
                fPyFileMod->ReportError();
            plProfile_EndTiming(PythonUpdate);
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
//
//  Class      : pfPythonKeyCatcher
//  PARAMETERS : none
//
//  PURPOSE    : Small wrapper class to catch discarded key events and pass
//               them to a plPythonFileMod
//

class pfPythonKeyCatcher : public plDefaultKeyCatcher
{
    plPythonFileMod *fMod;

    public:
        pfPythonKeyCatcher( plPythonFileMod *mod ) : fMod( mod ) {}
        
        virtual void    HandleKeyEvent( plKeyEventMsg *event )
        {
            fMod->HandleDiscardedKey( event );
        }
};

bool plPythonFileMod::fAtConvertTime = false;

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : plPythonFileMod and ~plPythonFileMod
//  PARAMETERS : none
//
//  PURPOSE    : Constructor and destructor
//
plPythonFileMod::plPythonFileMod()
{
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
    if ( !fAtConvertTime )      // if this is just an Add that's during a convert, then don't do anymore
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

    // then get rid of this module
    //  NOTE: fModule shouldn't be made in the plugin, only at runtime
    if ( !fModuleName.empty() && fModule )
    {
        //_PyModule_Clear(fModule);
        PyObject *m;
        PyObject *modules = PyImport_GetModuleDict();
        if( modules && (m = PyDict_GetItemString(modules, fModuleName.c_str())) && PyModule_Check(m))
        {
            hsStatusMessageF("Module %s removed from python dictionary",fModuleName.c_str());
            PyDict_DelItemString(modules, fModuleName.c_str());
        }
        else
        {
            hsStatusMessageF("Module %s not found in python dictionary. Already removed?",fModuleName.c_str());
        }
        // the above code should have unloaded the module from python, so it will delete itself, therefore
        // we need to set our pointer to nil to make sure we don't try to use it
        fModule = nil;
    }
    fModuleName = ST::null;
}

#include "plPythonPack.h"

bool plPythonFileMod::ILoadPythonCode()
{
#ifndef PLASMA_EXTERNAL_RELEASE
    // get code from file and execute in module
    // see if the file exists first before trying to import it
    plFileName pyfile = plFileName::Join(".", "python", ST::format("{}.py", fPythonFile));
    if (plFileInfo(pyfile).Exists()) {
        // ok... we can't really use import because Python remembers too much where global variables came from
        // ...and using execfile make it sure that globals are defined in this module and not in the imported module
        ST::string fromLoad = ST::format(R"(execfile('.\\python\\{}.py'))", fPythonFile);
        if (PythonInterface::RunString(fromLoad.c_str(), fModule)) {
            // we've loaded the code into our module
            // now attach the glue python code to the end
            if (!PythonInterface::RunString(R"(execfile('.\\python\\plasma\\glue.py'))", fModule)) {
                // display any output (NOTE: this would be disabled in production)
                DisplayPythonOutput();
                return false;
            } else {
                return true;
            }
        }
        DisplayPythonOutput();

        ST::string errMsg = ST::format("Python file {}.py had errors!!! Could not load.", fPythonFile);
        PythonInterface::WriteToLog(errMsg);
        return false;
    }
#endif  //PLASMA_EXTERNAL_RELEASE

    // Finally, try and find the file in the Python packfile
    // ... for the external users .pak file is only used
    PyObject* pythonCode = PythonPack::OpenPythonPacked(fPythonFile);
    if (pythonCode && PythonInterface::RunPYC(pythonCode, fModule))
        return true;

    DisplayPythonOutput();

    ST::string errMsg = ST::format("Python file {}.py was not found.", fPythonFile);
    PythonInterface::WriteToLog(errMsg);
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

    // This initializes the PFM for gameplay, so do nothing if we're exporting in 3ds Max.
    if (!fAtConvertTime && !fPythonFile.empty()) {
        if (!fModule) {
            const plKey& pkey = sobj->GetKey();

            // nope, must be the first object. Then use it as the basis for the module
            fModuleName = IMakeModuleName(sobj);
            fModule = PythonInterface::CreateModule(fModuleName.c_str());

            // if we can't create the instance then there is nothing to do here
            if (!ILoadPythonCode()) {
                // things are getting off on a bad foot... just say there wasn't a module...
                fModule = nullptr;
                return;
            }

            // set the name of the file (in the global dictionary of the module)
            PyObject* dict = PyModule_GetDict(fModule);
            PyObject* pfilename = PyString_FromSTString(fPythonFile);
            PyDict_SetItemString(dict, "glue_name", pfilename);

            // next we need to:
            //  - create instance of class
            PyObject* getInst = PythonInterface::GetModuleItem("glue_getInst",fModule);
            fInstance = nullptr;
            if (getInst && PyCallable_Check(getInst)) {
                fInstance = PyObject_CallFunction(getInst, nullptr);
                if (!fInstance)
                    ReportError();
            }

            // if we can't create the instance then there is nothing to do here
            DisplayPythonOutput();
            if (!fInstance) {
                ST::string errMsg = ST::format("Python file {}.py, instance not found.", fPythonFile);
                PythonInterface::WriteToLog(errMsg);
                return;
            }

            // Add the SDL modifier
            if (plPythonSDLModifier::HasSDL(fPythonFile)) {
                plSceneObject* sceneObj = plSceneObject::ConvertNoRef(GetTarget(0)->GetKey()->ObjectIsLoaded());
                if (sceneObj) {
                    hsAssert(!fSDLMod, "Python SDL modifier already created");
                    fSDLMod = new plPythonSDLModifier(this);
                    sceneObj->AddModifier(fSDLMod);
                }
            }

            //  - set the self.key and self.sceneobject in the instance of the class
            // set the selfKey as an attribute to their instance
            fSelfKey = pyKey::New(GetKey(), this);
            PyObject_SetAttrString(fInstance, "key", fSelfKey);

            // set the sceneobject as an attribute to their instance
            pyObjectRef pSobj = pySceneObject::New(pkey, fSelfKey);
            PyObject_SetAttrString(fInstance, "sceneobject", pSobj.Get());

            // set the isInitialStateLoaded to not loaded... yet
            pyObjectRef pInitialState = PyInt_FromLong(0);
            PyObject_SetAttrString(fInstance, "isInitialStateLoaded", pInitialState.Get());

            // Give the SDL mod to Python
            if (fSDLMod) {
                pyObjectRef pSDL = pySDLModifier::New(fSDLMod);
                PyObject_SetAttrString(fInstance, "SDL", pSDL.Get());
            }

            //  - set the parameters
            PyObject* setParams = PythonInterface::GetModuleItem("glue_setParam", fModule);
            PyObject* check_isNamed = PythonInterface::GetModuleItem("glue_isNamedAttribute",fModule);
            if (setParams && PyCallable_Check(setParams)) {
                // loop throught the parameters and set them by id
                // (will need to create the appropiate Python object for each type)
                for (int nparam=0; nparam < GetParameterListCount(); nparam++) {
                    plPythonParameter parameter = GetParameterItem(nparam);

                    pyObjectRef value;
                    int isNamedAttr = 0;
                    pyObjectRef retvalue;
                    switch (parameter.fValueType) {
                        case plPythonParameter::kInt:
                            value = PyInt_FromLong(parameter.datarecord.fIntNumber);
                            break;
                        case plPythonParameter::kFloat:
                            value = PyFloat_FromDouble(parameter.datarecord.fFloatNumber);
                            break;
                        case plPythonParameter::kbool:
                            value = PyInt_FromLong(parameter.datarecord.fBool);
                            break;
                        case plPythonParameter::kString:
                        case plPythonParameter::kAnimationName:
                            isNamedAttr = 0;
                            if (check_isNamed && PyCallable_Check(check_isNamed)) {
                                retvalue = PyObject_CallFunction(check_isNamed, _pycs("l"), parameter.fID);
                                if (!retvalue ) {
                                    ReportError();
                                    DisplayPythonOutput();
                                }
                                if (retvalue && PyInt_Check(retvalue.Get()) )
                                    isNamedAttr = PyInt_AsLong(retvalue.Get());
                                // is it a NamedActivator
                                if (isNamedAttr == 1 || isNamedAttr == 2) {
                                    if (plAgeLoader::GetInstance()->IsLoadingAge()) {
                                        NamedComponent comp;
                                        comp.isActivator = (isNamedAttr == 1);
                                        comp.id = parameter.fID;
                                        comp.name = parameter.fString;
                                        fNamedCompQueue.Append(comp);
                                    } else {
                                        if (isNamedAttr == 1)
                                            IFindActivatorAndAdd(parameter.fString, parameter.fID);
                                        else
                                            IFindResponderAndAdd(parameter.fString, parameter.fID);
                                    }
                                }
                            }
                            // if it wasn't a named string then must be normal string type
                            if (isNamedAttr == 0)
                                if (!parameter.fString.empty())
                                    value = PyString_FromSTString(parameter.fString);
                            break;
                        case plPythonParameter::kSceneObject:
                        case plPythonParameter::kSceneObjectList:
                            if (parameter.fObjectKey) {
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
                            if (parameter.fObjectKey) {
                                // create pyKey for the object
                                value = pyKey::New(parameter.fObjectKey);
                            }
                            break;
                    }
                    // if there is a value that was converted then tell the Python code
                    if (value) {
                        pyObjectRef retVal = PyObject_CallFunction(setParams, _pycs("lO"),
                                                                   parameter.fID, value.Get());
                        if (!retVal)
                            ReportError();
                    }
                }
            }

            // check if we need to register named activators or responders
            if (fNamedCompQueue.Count() > 0)
                plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

            //  - find functions in class they've defined.
            PythonInterface::CheckInstanceForFunctions(fInstance, fFunctionNames, fPyFunctionInstances);
            // clear any errors created by checking for methods in a class
            PyErr_Clear();

            // register for PageLoaded message if needed
            if (fPyFunctionInstances[kfunc_PageLoad])
                plgDispatch::Dispatch()->RegisterForExactType(plRoomLoadNotifyMsg::Index(), GetKey());

            // register for ClothingUpdate message if needed
            if (fPyFunctionInstances[kfunc_ClothingUpdate])
                plgDispatch::Dispatch()->RegisterForExactType(plClothingUpdateBCMsg::Index(), GetKey());

            // register for pfKIMsg message if needed
            if (fPyFunctionInstances[kfunc_KIMsg])
                plgDispatch::Dispatch()->RegisterForExactType(pfKIMsg::Index(), GetKey());

            // register for Member update message if needed
            if (fPyFunctionInstances[kfunc_MemberUpdate])
                plgDispatch::Dispatch()->RegisterForExactType(plMemberUpdateMsg::Index(), GetKey());

            // register for Remote Avatar Info message if needed
            if (fPyFunctionInstances[kfunc_RemoteAvatarInfo])
                plgDispatch::Dispatch()->RegisterForExactType(plRemoteAvatarInfoMsg::Index(), GetKey());

            // register for CCR message if needed
            if (fPyFunctionInstances[kfunc_OnCCRMsg])
                plgDispatch::Dispatch()->RegisterForExactType(plCCRCommunicationMsg::Index(), GetKey());

            // register for VaultNotify message if needed
            if (fPyFunctionInstances[kfunc_OnVaultNotify])
                plgDispatch::Dispatch()->RegisterForExactType(plVaultNotifyMsg::Index(), GetKey());

            // register for Owndership change notification message if needed
            if (fPyFunctionInstances[kfunc_OwnershipNotify])
                plgDispatch::Dispatch()->RegisterForExactType(plNetOwnershipMsg::Index(), GetKey());

#ifndef PLASMA_EXTERNAL_RELEASE
            // register for Backdoor message if needed
            if (fPyFunctionInstances[kfunc_OnBackdoorMsg])
                plgDispatch::Dispatch()->RegisterForExactType(pfBackdoorMsg::Index(), GetKey());
#endif  //PLASMA_EXTERNAL_RELEASE

            // register for VaultCallback events if needed
            if (fPyFunctionInstances[kfunc_VaultEvent]) {
                fVaultCallback = new PythonVaultCallback(this, kfunc_VaultEvent);
                VaultRegisterCallback(fVaultCallback);
            }

            // register ourselves to be the default key catcher if necessary
            if (fPyFunctionInstances[kfunc_OnDefaultKeyCaught]) {
                fKeyCatcher = new pfPythonKeyCatcher(this);
                plInputInterfaceMgr::GetInstance()->SetDefaultKeyCatcher(fKeyCatcher);
            }

            // register for Marker messages if needed
            if (fPyFunctionInstances[kfunc_OnMarkerMsg])
                plgDispatch::Dispatch()->RegisterForExactType(pfMarkerMsg::Index(), GetKey());

            // if they are going to get LOS hit messages then we need to get the Pipeline pointer
            if (fPyFunctionInstances[kfunc_OnLOSNotify])
                plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());

            // if this is a climbing-wall function, we need to register for climbing wall messages
            if (fPyFunctionInstances[kfunc_OnClimbBlockerEvent])
                plgDispatch::Dispatch()->RegisterForExactType(plClimbEventMsg::Index(), GetKey());

            if (fPyFunctionInstances[kfunc_OnAvatarSpawn])
                plgDispatch::Dispatch()->RegisterForExactType(plAvatarSpawnNotifyMsg::Index(), GetKey());

            if (fPyFunctionInstances[kfunc_OnAccountUpdate])
                plgDispatch::Dispatch()->RegisterForExactType(plAccountUpdateMsg::Index(), GetKey());

            if (fPyFunctionInstances[kfunc_gotPublicAgeList])
                plgDispatch::Dispatch()->RegisterForExactType(plNetCommPublicAgeListMsg::Index(), GetKey());

            if (fPyFunctionInstances[kfunc_OnAIMsg])
                plgDispatch::Dispatch()->RegisterForExactType(plAIBrainCreatedMsg::Index(), GetKey());

            // As the last thing... call the OnInit function if they have one
            if (fPyFunctionInstances[kfunc_Init]){
                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Init],
                                                         const_cast<char*>(fFunctionNames[kfunc_Init]),
                                                         nullptr);
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
            }

            // Oversight fix... Sometimes PythonFileMods are loaded after the AgeInitialState is received.
            // We should really let the script know about that via OnServerInitComplete anyway because it's
            // not good to make assumptions about game state in workarounds for that method not being called
            plNetClientApp* na = plNetClientApp::GetInstance();
            if (!na->GetFlagsBit(plNetClientApp::kLoadingInitialAgeState) && na->GetFlagsBit(plNetClientApp::kPlayingGame)) {
                plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
                if (fPyFunctionInstances[kfunc_OnServerInitComplete]) {
                    plProfile_BeginTiming(PythonUpdate);
                    pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnServerInitComplete],
                                                             const_cast<char*>(fFunctionNames[kfunc_OnServerInitComplete]),
                                                             nullptr);
                    if (!retVal)
                        ReportError();
                    plProfile_EndTiming(PythonUpdate);
                }
            }

            // display python output
            DisplayPythonOutput();
        } else {
            // else if module is already created... Then we are just adding an additional object
            // to the already existing SceneObject. We need to get the instance and add this new
            // target's key to the pySceneObject.
            if (fInstance) {
                PyObject* dict = PyModule_GetDict(fModule);

                pyObjectRef pkeyObj = pyKey::New(sobj->GetKey());
                pyObjectRef pSceneObject = PyObject_GetAttrString(fInstance, "sceneobject");
                pyObjectRef retVal = PyObject_CallMethod(pSceneObject.Get(), _pycs("addKey"),
                                                         _pycs("O"), pkeyObj.Get());
            }
        }
    }
}

void plPythonFileMod::RemoveTarget(plSceneObject* so)
{
    // remove sdl modifier
    if (fSDLMod) {
        if (GetNumTargets()) {
            plSceneObject* sceneObj = plSceneObject::ConvertNoRef(GetTarget(0)->GetKey()->ObjectIsLoaded());
            if (sceneObj && fSDLMod)
                sceneObj->RemoveModifier(fSDLMod);
        }
        delete fSDLMod;
        fSDLMod = nullptr;
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

void    plPythonFileMod::HandleDiscardedKey(plKeyEventMsg* msg)
{
    if (!fPyFunctionInstances[kfunc_OnDefaultKeyCaught])
        return;

    plProfile_BeginTiming(PythonUpdate);

    pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[ kfunc_OnDefaultKeyCaught ],
                                             const_cast<char*>(fFunctionNames[kfunc_OnDefaultKeyCaught]),
                                             _pycs("ciiiii"),
                                             msg->GetKeyChar(),
                                            (int)msg->GetKeyDown(),
                                            (int)msg->GetRepeat(),
                                            (int)msg->GetShiftKeyDown(),
                                            (int)msg->GetCtrlKeyDown(),
                                            (int)msg->GetKeyCode());
    if (!retVal)
        ReportError();

    plProfile_EndTiming(PythonUpdate);
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
ST::string plPythonFileMod::IMakeModuleName(plSceneObject* sobj)
{
    // This strips underscores out of module names so python won't truncate them... -S

    plKey pKey = GetKey();
    plKey sKey = sobj->GetKey();

    ST::string soName = sKey->GetName().replace("_", "");
    ST::string pmName = pKey->GetName().replace("_", "");

    ST::string_stream name;
    name << soName << pmName;

    // check to see if we are attaching to a clone?
    plKeyImp* pKeyImp = (plKeyImp*)(sKey);
    if (pKeyImp->GetCloneOwner())
    {
        // we have an owner... so we must be a clone.
        // add the cloneID to the end of the module name
        // and set the fIAmAClone flag
        uint32_t cloneID = pKeyImp->GetUoid().GetCloneID();
        name << cloneID;
        fAmIAttachedToClone = true;
    }

    // make sure that the actual modulue will be uniqie
    if ( !PythonInterface::IsModuleNameUnique(name.to_string()))
    {
        // if not unique then add the sequence number to the end of the modulename
        uint32_t seqID = pKeyImp->GetUoid().GetLocation().GetSequenceNumber();
        name << seqID;
    }

    return name.to_string();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ISetKeyValue
//  PARAMETERS : key to responder, parameter id
//
//  PURPOSE    : set the param in the python file
//             : so named stuff works
//
void plPythonFileMod::ISetKeyValue(const plKey& key, int32_t id)
{
    PyObject* setParams = PythonInterface::GetModuleItem("glue_setParam",fModule);

    if (setParams && PyCallable_Check(setParams)) {
        if (key) {
            // create pyKey for the object
            pyObjectRef value = pyKey::New(key);

            if (value) {
                pyObjectRef retVal = PyObject_CallFunction(setParams, _pycs("lO"), id, value.Get());
                if (!retVal)
                    ReportError();
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
void plPythonFileMod::IFindResponderAndAdd(const ST::string &responderName, int32_t id)
{
    if ( !responderName.empty() )
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
void plPythonFileMod::IFindActivatorAndAdd(const ST::string &activatorName, int32_t id)
{
    if ( !activatorName.empty() )
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
                                                    new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, kAddNotify, 0),
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
bool plPythonFileMod::IEval(double secs, float del, uint32_t dirty)
{
    if (fModule) {
        // if this is the first time at the Eval, then run Python OnFirstUpdate
        if (fIsFirstTimeEval) {
            fIsFirstTimeEval = false;
            if (fPyFunctionInstances[kfunc_FirstUpdate]) {
                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_FirstUpdate],
                                                         (char*)fFunctionNames[kfunc_FirstUpdate],
                                                         nullptr);
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
                DisplayPythonOutput();
            }
        }

        // is the Update function defined and working (as far as we know)?
        if (fPyFunctionInstances[kfunc_Update]) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Update],
                                                     const_cast<char*>(fFunctionNames[kfunc_Update]),
                                                     _pycs("df"), secs, del);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
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
bool plPythonFileMod::MsgReceive(plMessage* msg)
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
    if (fPyFunctionInstances[kfunc_Notify]) {
        // yes, so was there actually a plActivateMsg?
        plNotifyMsg* pNtfyMsg = plNotifyMsg::ConvertNoRef(msg);
        if (pNtfyMsg) {
            // remember if this was a Local Broad cast or not
            fLocalNotify = (pNtfyMsg->HasBCastFlag(plMessage::kNetNonLocal)) ? false : true;

            pyObjectRef levents = PyTuple_New(pNtfyMsg->GetEventCount());
            for (int i = 0; i < pNtfyMsg->GetEventCount(); i++) {
                proEventData* pED = pNtfyMsg->GetEventRecord(i);
                switch (pED->fEventType) {
                    case proEventData::kCollision:
                        {
                            proCollisionEventData* eventData = (proCollisionEventData*)pED;

                            PyObject* event = PyTuple_New(4);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kCollision));
                            PyTuple_SET_ITEM(event, 1, PyInt_FromLong(eventData->fEnter ? 1 : 0));
                            PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fHitter, fSelfKey));
                            PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fHittee, fSelfKey));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;
                        
                    case proEventData::kSpawned:
                        {
                            proSpawnedEventData* eventData = (proSpawnedEventData*)pED;

                            PyObject* event = PyTuple_New(3);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kSpawned));
                            PyTuple_SET_ITEM(event, 1, pySceneObject::New(eventData->fSpawner, fSelfKey));
                            PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fSpawnee, fSelfKey));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kPicked:
                        {
                            proPickedEventData* eventData = (proPickedEventData*)pED;
                            PyObject* event = PyTuple_New(6);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kPicked));
                            PyTuple_SET_ITEM(event, 1, PyInt_FromLong(eventData->fEnabled ? 1 : 0));
                            PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fPicker, fSelfKey));
                            PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fPicked, fSelfKey));
                            PyTuple_SET_ITEM(event, 4, pyPoint3::New(eventData->fHitPoint));

                            // make it in the local space
                            hsPoint3 tolocal{ 0.f, 0.f, 0.f };
                            if (eventData->fPicked){
                                plSceneObject* obj = plSceneObject::ConvertNoRef(eventData->fPicked->ObjectIsLoaded());
                                if (obj) {
                                    const plCoordinateInterface* ci = obj->GetCoordinateInterface();
                                    if (ci)
                                        tolocal = (hsMatrix44)ci->GetWorldToLocal() * eventData->fHitPoint;
                                }
                            }
                            PyTuple_SET_ITEM(event, 5, pyPoint3::New(tolocal));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kControlKey:
                        {
                            proControlKeyEventData* eventData = (proControlKeyEventData*)pED;

                            PyObject* event = PyTuple_New(3);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kControlKey));
                            PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fControlKey));
                            PyTuple_SET_ITEM(event, 2, PyInt_FromLong(eventData->fDown ? 1 : 0));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kVariable:
                        {
                            proVariableEventData* eventData = (proVariableEventData*)pED;
                            // create event list
                            PyObject* event = PyTuple_New(4);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kVariable));
                            PyTuple_SET_ITEM(event, 1, PyString_FromSTString(eventData->fName));
                            PyTuple_SET_ITEM(event, 2, PyLong_FromLong(eventData->fDataType));

                            // depending on the data type create the data
                            switch ( eventData->fDataType ) {
                                case proEventData::kFloat:
                                    PyTuple_SET_ITEM(event, 3, PyFloat_FromDouble(eventData->fNumber.f));
                                    break;
                                case proEventData::kKey:
                                    PyTuple_SET_ITEM(event, 3, pyKey::New(eventData->fKey));
                                    break;
                                case proEventData::kInt:
                                    PyTuple_SET_ITEM(event, 3, PyInt_FromLong(eventData->fNumber.i));
                                    break;
                                default:
                                    Py_INCREF(Py_None);
                                    PyTuple_SET_ITEM(event, 3, Py_None);
                                    break;
                            }
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kFacing:
                        {
                            proFacingEventData* eventData = (proFacingEventData*)pED;
                            PyObject* event = PyTuple_New(5);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kFacing));
                            PyTuple_SET_ITEM(event, 1, PyInt_FromLong(eventData->enabled ? 1 : 0));
                            PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fFacer, fSelfKey));
                            PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fFacee, fSelfKey));
                            PyTuple_SET_ITEM(event, 4, PyFloat_FromDouble(eventData->dot));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kContained:
                        {
                            proContainedEventData* eventData = (proContainedEventData*)pED;

                            PyObject* event = PyTuple_New(4);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kContained));
                            PyTuple_SET_ITEM(event, 1, PyInt_FromLong(eventData->fEntering ? 1 : 0));
                            PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fContained, fSelfKey));
                            PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fContainer, fSelfKey));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kActivate:
                        {
                            proActivateEventData* eventData = (proActivateEventData*)pED;

                            PyObject* event = PyTuple_New(3);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kActivate));
                            PyTuple_SET_ITEM(event, 1, PyInt_FromLong(eventData->fActive ? 1 : 0));
                            PyTuple_SET_ITEM(event, 2, PyInt_FromLong(eventData->fActivate ? 1 : 0));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kCallback:
                        {
                            proCallbackEventData* eventData = (proCallbackEventData*)pED;

                            PyObject* event = PyTuple_New(2);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kCallback));
                            PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fEventType));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kResponderState:
                        {
                            proResponderStateEventData* eventData = (proResponderStateEventData*)pED;

                            PyObject* event = PyTuple_New(2);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kResponderState));
                            PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fState));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;

                    case proEventData::kMultiStage:
                        {
                            proMultiStageEventData* eventData = (proMultiStageEventData*)pED;

                            PyObject* event = PyTuple_New(4);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kMultiStage));
                            PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fStage));
                            PyTuple_SET_ITEM(event, 2, PyLong_FromLong(eventData->fEvent));
                            PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fAvatar, fSelfKey));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;
                    case proEventData::kOfferLinkingBook:
                        {
                            proOfferLinkingBookEventData* eventData = (proOfferLinkingBookEventData*)pED;

                            PyObject* event = PyTuple_New(4);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kOfferLinkingBook));
                            PyTuple_SET_ITEM(event, 1, pySceneObject::New(eventData->offerer, fSelfKey));
                            PyTuple_SET_ITEM(event, 2, PyInt_FromLong(eventData->targetAge));
                            PyTuple_SET_ITEM(event, 3, PyInt_FromLong(eventData->offeree));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;
                    case proEventData::kBook:
                        {
                            proBookEventData* eventData = (proBookEventData*)pED;

                            PyObject* event = PyTuple_New(3);
                            PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kBook));
                            PyTuple_SET_ITEM(event, 1, PyLong_FromUnsignedLong(eventData->fEvent));
                            PyTuple_SET_ITEM(event, 2, PyLong_FromUnsignedLong(eventData->fLinkID));
                            PyTuple_SET_ITEM(levents.Get(), i, event);
                        }
                        break;
                }
            }

            // Need to determine which of the Activators sent this plNotifyMsg
            // and set the ID appropriately
            int32_t id = -1;  // assume that none was found
            if (pNtfyMsg->GetSender()) {
                // loop throught the parameters and set them by id
                // (will need to create the appropiate Python object for each type)
                for (int npm = 0; npm<GetParameterListCount(); npm++) {
                    plPythonParameter parameter = GetParameterItem(npm);
                    // is it something that could produce a plNotifiyMsg?
                    if (parameter.fValueType == plPythonParameter::kActivatorList
                        || parameter.fValueType == plPythonParameter::kBehavior 
                        || parameter.fValueType == plPythonParameter::kResponderList) {
                        // is there an actual ObjectKey to look at?
                        if (parameter.fObjectKey) {
                            // is it the same as the sender of the notify message?
                            if (pNtfyMsg->GetSender()->GetUoid() == parameter.fObjectKey->GetUoid()) {
                                // match! Then return that as the ID
                                id = parameter.fID;
                            }
                        }
                    }
                }
            }

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_Notify],
                                                     const_cast<char*>(fFunctionNames[kfunc_Notify]),
                                                     _pycs("flO"), pNtfyMsg->fState, id, levents.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a key event message?
    if (fPyFunctionInstances[kfunc_OnKeyEvent]) {
        plControlEventMsg* pEMsg = plControlEventMsg::ConvertNoRef(msg);
        if (pEMsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnKeyEvent],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnKeyEvent]),
                                                     _pycs("ll"), pEMsg->GetControlCode(),
                                                     pEMsg->ControlActivated());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for an Timer message?
    if (fPyFunctionInstances[kfunc_AtTimer]) {
        plTimerCallbackMsg* pTimerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
        if (pTimerMsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_AtTimer],
                                                     const_cast<char*>(fFunctionNames[kfunc_AtTimer]),
                                                     _pycs("l"), pTimerMsg->fID);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            return true;
        }
    }

    // are they looking for an GUINotify message?
    if (fPyFunctionInstances[kfunc_GUINotify]) {
        pfGUINotifyMsg* pGUIMsg = pfGUINotifyMsg::ConvertNoRef(msg);
        if (pGUIMsg) {
            pyObjectRef pyControl;
            if (pGUIMsg->GetControlKey()) {
                // now create the control... but first we need to find out what it is
                pyObjectRef pyCtrlKey = pyKey::New(pGUIMsg->GetControlKey());
                uint32_t control_type = pyGUIDialog::WhatControlType(*(pyKey::ConvertFrom(pyCtrlKey.Get())));

                switch (control_type) {
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
            int32_t id = -1;  // assume that none was found
            if (pGUIMsg->GetSender()) {
                // loop throught the parameters and set them by id
                // (will need to create the appropiate Python object for each type)
                for (int npm = 0; npm < GetParameterListCount(); npm++) {
                    plPythonParameter parameter = GetParameterItem(npm);
                    // is it something that could produce a plNotifiyMsg?
                    if (parameter.fValueType == plPythonParameter::kGUIDialog || parameter.fValueType == plPythonParameter::kGUIPopUpMenu) {
                        // is there an actual ObjectKey to look at?
                        if (parameter.fObjectKey) {
                            // is it the same of the sender of the notify message?
                            if (pGUIMsg->GetSender()->GetUoid() == parameter.fObjectKey->GetUoid()) {
                                // match! then set the ID to what the parameter is, so the python programmer can find it
                                id = parameter.fID;
                            }
                        }
                    }
                }
            }

            // make sure that we found a control to go with this
            if (!pyControl)
                pyControl.SetPyNone();

            // call their OnGUINotify method
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_GUINotify],
                                                     const_cast<char*>(fFunctionNames[kfunc_GUINotify]),
                                                     _pycs("lOl"), id, pyControl.Get(), pGUIMsg->GetEvent());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for an RoomLoadNotify message?
    if (fPyFunctionInstances[kfunc_PageLoad]) {
        plRoomLoadNotifyMsg* pRLNMsg = plRoomLoadNotifyMsg::ConvertNoRef(msg);
        if (pRLNMsg) {
            pyObjectRef roomname = PyUnicode_FromSTString(pRLNMsg->GetRoom() ?
                                                        pRLNMsg->GetRoom()->GetName() : ST::null);

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_PageLoad],
                                                     const_cast<char*>(fFunctionNames[kfunc_PageLoad]),
                                                     _pycs("lO"), pRLNMsg->GetWhatHappen(), roomname.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }


    // are they looking for an ClothingUpdate message?
    if (fPyFunctionInstances[kfunc_ClothingUpdate]) {
        plClothingUpdateBCMsg* pCUMsg = plClothingUpdateBCMsg::ConvertNoRef(msg);
        if (pCUMsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_ClothingUpdate],
                                                     const_cast<char*>(fFunctionNames[kfunc_ClothingUpdate]),
                                                     nullptr);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for an KIMsg message?
    if (fPyFunctionInstances[kfunc_KIMsg]) {
        pfKIMsg* pkimsg = pfKIMsg::ConvertNoRef(msg);
        if (pkimsg && pkimsg->GetCommand() != pfKIMsg::kHACKChatMsg) {
            pyObjectRef value;
            switch (pkimsg->GetCommand()) {
                case pfKIMsg::kSetChatFadeDelay:
                    value = PyFloat_FromDouble(pkimsg->GetDelay());
                    break;
                case pfKIMsg::kSetTextChatAdminMode:
                    value = PyLong_FromLong(pkimsg->GetFlags()&pfKIMsg::kAdminMsg ? 1 : 0 );
                    break;
                case pfKIMsg::kYesNoDialog:
                    value = PyTuple_New(2);
                    PyTuple_SET_ITEM(value.Get(), 0, PyUnicode_FromSTString(pkimsg->GetString()));
                    PyTuple_SET_ITEM(value.Get(), 1, pyKey::New(pkimsg->GetSender()));
                    break;
                case pfKIMsg::kGZInRange:
                    value = PyTuple_New(2);
                    PyTuple_SET_ITEM(value.Get(), 0, PyLong_FromLong(pkimsg->GetIntValue()));
                    PyTuple_SET_ITEM(value.Get(), 1, pyKey::New(pkimsg->GetSender()));
                    break;
                case pfKIMsg::kRateIt:
                    value = PyTuple_New(3);
                    PyTuple_SET_ITEM(value.Get(), 0, PyString_FromSTString(pkimsg->GetUser()));
                    PyTuple_SET_ITEM(value.Get(), 1, PyUnicode_FromSTString(pkimsg->GetString()));
                    PyTuple_SET_ITEM(value.Get(), 2, PyLong_FromLong(pkimsg->GetIntValue()));
                    break;
                case pfKIMsg::kRegisterImager:
                    value = PyTuple_New(2);
                    PyTuple_SET_ITEM(value.Get(), 0, PyUnicode_FromSTString(pkimsg->GetString()));
                    PyTuple_SET_ITEM(value.Get(), 1, pyKey::New(pkimsg->GetSender()));
                    break;
                case pfKIMsg::kAddPlayerDevice:
                case pfKIMsg::kRemovePlayerDevice:
                    {
                        ST::string str = pkimsg->GetString();
                        if (str.empty())
                            value.SetPyNone();
                        else
                            value = PyUnicode_FromSTString(str);
                    }
                    break;
                case pfKIMsg::kKIChatStatusMsg:
                case pfKIMsg::kKILocalChatStatusMsg:
                case pfKIMsg::kKILocalChatErrorMsg:
                case pfKIMsg::kKIOKDialog:
                case pfKIMsg::kKIOKDialogNoQuit:
                case pfKIMsg::kGZFlashUpdate:
                case pfKIMsg::kKICreateMarkerNode:
                    value = PyUnicode_FromSTString(pkimsg->GetString());
                    break;
                case pfKIMsg::kMGStartCGZGame:
                case pfKIMsg::kMGStopCGZGame:
                case pfKIMsg::kFriendInviteSent:
                default:
                    value = PyLong_FromLong(pkimsg->GetIntValue());
                    break;
            }

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_KIMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_KIMsg]),
                                                     _pycs("lO"), pkimsg->GetCommand(), value.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for an MemberUpdate message?
    if (fPyFunctionInstances[kfunc_MemberUpdate]) {
        plMemberUpdateMsg* pmumsg = plMemberUpdateMsg::ConvertNoRef(msg);
        if (pmumsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_MemberUpdate],
                                                     const_cast<char*>(fFunctionNames[kfunc_MemberUpdate]),
                                                     nullptr);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a RemoteAvatar Info message?
    if (fPyFunctionInstances[kfunc_RemoteAvatarInfo]) {
        plRemoteAvatarInfoMsg* pramsg = plRemoteAvatarInfoMsg::ConvertNoRef(msg);
        if (pramsg) {
            pyObjectRef player;
            if (pramsg->GetAvatarKey()) {
                // try to create the pyPlayer for where this message came from
                int mbrIndex = plNetClientMgr::GetInstance()->TransportMgr().FindMember(pramsg->GetAvatarKey());
                if (mbrIndex != -1) {
                    plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMember( mbrIndex );
                    player = pyPlayer::New(mbr->GetAvatarKey(), mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
                }
            }
            if (!player)
                player = PyInt_FromLong(0);

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_RemoteAvatarInfo],
                                                     const_cast<char*>(fFunctionNames[kfunc_RemoteAvatarInfo]),
                                                     _pycs("O"), player.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }


    // are they looking for a CCR communication message?
    if (fPyFunctionInstances[kfunc_OnCCRMsg]) {
        plCCRCommunicationMsg* ccrmsg = plCCRCommunicationMsg::ConvertNoRef(msg);
        if (ccrmsg) {
            const char* textmessage = ccrmsg->GetMessage();
            if (!textmessage)
                textmessage = "";

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnCCRMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnCCRMsg]),
                                                     _pycs("lsl"), ccrmsg->GetType(), textmessage,
                                                      ccrmsg->GetCCRPlayerID());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a VaultNotify message?
    if (fPyFunctionInstances[kfunc_OnVaultNotify]) {
        if (plVaultNotifyMsg * vaultNotifyMsg = plVaultNotifyMsg::ConvertNoRef(msg)) {
            if (hsSucceeded(vaultNotifyMsg->GetResultCode())) {
                // Create a tuple for second argument according to msg type.
                // Default to an empty tuple.
                pyObjectRef ptuple;
                switch (vaultNotifyMsg->GetType()) {
                    case plVaultNotifyMsg::kRegisteredOwnedAge:
                    case plVaultNotifyMsg::kRegisteredVisitAge:
                    case plVaultNotifyMsg::kUnRegisteredOwnedAge:
                    case plVaultNotifyMsg::kUnRegisteredVisitAge: {
                        if (hsRef<RelVaultNode> rvn = VaultGetNode(vaultNotifyMsg->GetArgs()->GetInt(plNetCommon::VaultTaskArgs::kAgeLinkNode))) {
                            ptuple = PyTuple_New(1);
                            PyTuple_SET_ITEM(ptuple.Get(), 0, pyVaultAgeLinkNode::New(rvn));
                        }
                    }
                    break;
                    
                    case plVaultNotifyMsg::kPublicAgeCreated:
                    case plVaultNotifyMsg::kPublicAgeRemoved: {
                        ST::string ageName = vaultNotifyMsg->GetArgs()->GetString(plNetCommon::VaultTaskArgs::kAgeFilename);
                        if (!ageName.empty()) {
                            ptuple = PyTuple_New(1);
                            PyTuple_SET_ITEM(ptuple.Get(), 0, PyString_FromSTString(ageName));
                        }
                    }
                    break;
                }

                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnVaultNotify],
                                                         const_cast<char*>(fFunctionNames[kfunc_OnVaultNotify]),
                                                         _pycs("lO"), vaultNotifyMsg->GetType(), ptuple.Get());
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
                DisplayPythonOutput();
            }
            return true;
        }
    }

    // are they looking for a RealTimeChat message?
    if (fPyFunctionInstances[kfunc_RTChat]) {
        pfKIMsg* pkimsg = pfKIMsg::ConvertNoRef(msg);
        if (pkimsg && pkimsg->GetCommand() == pfKIMsg::kHACKChatMsg) {
            if (!VaultAmIgnoringPlayer(pkimsg->GetPlayerID())) {
                pyObjectRef uMessage = PyUnicode_FromSTString(pkimsg->GetString());

                pyObjectRef player;
                PyObject* ptPlayerClass = PythonInterface::GetPlasmaItem("ptPlayer");
                hsAssert(ptPlayerClass, "Could not locate the ptPlayer class.");
                int mbrIndex = plNetClientMgr::GetInstance()->TransportMgr().FindMember(pkimsg->GetPlayerID());
                if (mbrIndex != -1) {
                    plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMember( mbrIndex );
                    player = pyPlayer::New(mbr->GetAvatarKey(), pkimsg->GetUser(), mbr->GetPlayerID(), mbr->GetDistSq());
                } else {
                    // else if we could not find the player in our list, then just return a string of the user's name
                    ST::string fromName = pkimsg->GetUser();
                    if (fromName.empty())
                        fromName = "Anonymous Coward";
                    player = pyPlayer::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fromName, pkimsg->GetPlayerID(), 0.0);
                }

                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_RTChat],
                                                         const_cast<char*>(fFunctionNames[kfunc_RTChat]),
                                                         _pycs("OOl"), player.Get(), uMessage.Get(),
                                                         pkimsg->GetFlags());
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
                DisplayPythonOutput();
                return true;
            }
        }
    }

    if (plPlayerPageMsg::ConvertNoRef(msg)) {
        if (fPyFunctionInstances[kfunc_AvatarPage]) {
            plPlayerPageMsg* ppMsg = plPlayerPageMsg::ConvertNoRef(msg);
            if (ppMsg) {
                pyObjectRef pSobj = pySceneObject::New(ppMsg->fPlayer, fSelfKey);
                plSynchEnabler ps(true);    // enable dirty state tracking during shutdown

                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_AvatarPage],
                                                         const_cast<char*>(fFunctionNames[kfunc_AvatarPage]),
                                                         _pycs("Oli"), pSobj.Get(), !ppMsg->fUnload,
                                                         ppMsg->fLastOut);
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
                DisplayPythonOutput();
                return true;
            }
        }
    }

    if (plAgeBeginLoadingMsg::ConvertNoRef(msg)) {
        if (fPyFunctionInstances[kfunc_OnBeginAgeLoad]) {
            plAgeBeginLoadingMsg* ppMsg = plAgeBeginLoadingMsg::ConvertNoRef(msg);
            if (ppMsg) {
                pyObjectRef pSobj = pySceneObject::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fSelfKey);
                plSynchEnabler ps(true);    // enable dirty state tracking during shutdown  

                plProfile_BeginTiming(PythonUpdate);
                pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBeginAgeLoad],
                                                         const_cast<char*>(fFunctionNames[kfunc_OnBeginAgeLoad]),
                                                         _pycs("O"), pSobj.Get());
                if (!retVal)
                    ReportError();
                plProfile_EndTiming(PythonUpdate);
                DisplayPythonOutput();
                return true;
            }
        }
    }

    // initial server update complete message
    if (plInitialAgeStateLoadedMsg::ConvertNoRef(msg))
    {
        if ( fInstance ) {
            // set the isInitialStateLoaded to that it is loaded
            pyObjectRef pInitialState = PyInt_FromLong(1);
            PyObject_SetAttrString(fInstance, "isInitialStateLoaded", pInitialState.Get());
        }
        if (fPyFunctionInstances[kfunc_OnServerInitComplete]) {
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnServerInitComplete],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnServerInitComplete]),
                                                     nullptr);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }
    // are they looking for a plSDLNotificationMsg message?
    if (fPyFunctionInstances[kfunc_SDLNotify]) {
        plSDLNotificationMsg* sn = plSDLNotificationMsg::ConvertNoRef(msg);
        if (sn) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_SDLNotify],
                                                     const_cast<char*>(fFunctionNames[kfunc_SDLNotify]),
                                                     _pycs("ssls"), sn->fVar->GetName().c_str(), sn->fSDLName.c_str(),
                                                     sn->fPlayerID, sn->fHintString.c_str());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a plNetOwnershipMsg message?
    if (fPyFunctionInstances[kfunc_OwnershipNotify]) {
        plNetOwnershipMsg* nom = plNetOwnershipMsg::ConvertNoRef(msg);
        if (nom) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OwnershipNotify],
                                                     (char*)fFunctionNames[kfunc_OwnershipNotify],
                                                     nullptr);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a pfMarkerMsg message?
    if (fPyFunctionInstances[kfunc_OnMarkerMsg]) {
        pfMarkerMsg* markermsg = pfMarkerMsg::ConvertNoRef(msg);
        if (markermsg) {
            pyObjectRef ptuple;
            switch (markermsg->fType) {
                case pfMarkerMsg::kMarkerCaptured:
                    // Sent when we collide with a marker
                    ptuple = PyTuple_New(1);
                    PyTuple_SET_ITEM(ptuple.Get(), 0, PyLong_FromUnsignedLong(markermsg->fMarkerID));
                    break;

                default:
                    ptuple = PyTuple_New(0);
                    break;
            }

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnMarkerMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnMarkerMsg]),
                                                     _pycs("lO"), (uint32_t)markermsg->fType, ptuple.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    // are they looking for a pfBackdoorMsg message?
    if (fPyFunctionInstances[kfunc_OnBackdoorMsg]) {
        pfBackdoorMsg* dt = pfBackdoorMsg::ConvertNoRef(msg);
        if (dt) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBackdoorMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnBackdoorMsg]),
                                                     _pycs("ss"), dt->GetTarget().c_str(), dt->GetString().c_str());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }
#endif  //PLASMA_EXTERNAL_RELEASE

    // are they looking for a plLOSHitMsg message?
    if (fPyFunctionInstances[kfunc_OnLOSNotify]) {
        plLOSHitMsg *pLOSMsg = plLOSHitMsg::ConvertNoRef( msg );
        if (pLOSMsg) {
            pyObjectRef scobj;
            pyObjectRef hitpoint;
            if (pLOSMsg->fObj && plSceneObject::ConvertNoRef(pLOSMsg->fObj->ObjectIsLoaded())) {
                scobj = pySceneObject::New(pLOSMsg->fObj);
                hitpoint = pyPoint3::New(pLOSMsg->fHitPoint);
            } else {
                scobj.SetPyNone();
                hitpoint.SetPyNone();
            }

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnLOSNotify],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnLOSNotify]),
                                                     _pycs("llOOf"), pLOSMsg->fRequestID, pLOSMsg->fNoHit,
                                                     scobj.Get(), hitpoint.Get(), pLOSMsg->fDistance);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a plAvatarBehaviorNotifyMsg message?
    if (fPyFunctionInstances[kfunc_OnBehaviorNotify]) {
        // yes, so was there actually a plAvatarBehaviorNotifyMsg?
        plAvatarBehaviorNotifyMsg* behNotifymsg = plAvatarBehaviorNotifyMsg::ConvertNoRef(msg);
        if (behNotifymsg) {
            // the parent of the sender should be the avatar that did the behavior
            pyObjectRef pSobj;

            plModifier* avmod = plModifier::ConvertNoRef(behNotifymsg->GetSender()->ObjectIsLoaded());
            if (avmod && avmod->GetNumTargets())
                pSobj = pySceneObject::New(avmod->GetTarget(0)->GetKey(), fSelfKey);
            else
                pSobj.SetPyNone();

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnBehaviorNotify],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnBehaviorNotify]),
                                                     _pycs("lOl"), behNotifymsg->fType, pSobj.Get(),
                                                     behNotifymsg->state);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a pfMovieEventMsg message?
    if (fPyFunctionInstances[kfunc_OnMovieEvent]) {
        pfMovieEventMsg* moviemsg = pfMovieEventMsg::ConvertNoRef(msg);
        if (moviemsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnMovieEvent],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnMovieEvent]),
                                                     _pycs("si"), moviemsg->fMovieName.AsString().c_str(),
                                                     (uint32_t)moviemsg->fReason);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    // are they looking for a plCaptureRenderMsg message?
    if (fPyFunctionInstances[kfunc_OnScreenCaptureDone]) {
        plCaptureRenderMsg *capturemsg = plCaptureRenderMsg::ConvertNoRef(msg);
        if (capturemsg) {
            pyObjectRef pSobj;
            if (capturemsg->GetMipmap())
                pSobj = pyImage::New(capturemsg->GetMipmap());
            else
                pSobj.SetPyNone();

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnScreenCaptureDone],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnScreenCaptureDone]),
                                                     _pycs("O"), pSobj.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    if (fPyFunctionInstances[kfunc_OnClimbBlockerEvent]) {
        plClimbEventMsg* pEvent = plClimbEventMsg::ConvertNoRef(msg);
        if (pEvent) {
            pyObjectRef pSobj = pySceneObject::New(pEvent->GetSender(), fSelfKey);

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnClimbBlockerEvent],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnClimbBlockerEvent]),
                                                     _pycs("O"), pSobj.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    if (fPyFunctionInstances[kfunc_OnAvatarSpawn]) {
        plAvatarSpawnNotifyMsg* pSpawn = plAvatarSpawnNotifyMsg::ConvertNoRef(msg);
        if (pSpawn) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnAvatarSpawn],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnAvatarSpawn]),
                                                     _pycs("l"), 1);
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }
    
    if (fPyFunctionInstances[kfunc_OnAccountUpdate]) {
        plAccountUpdateMsg* pUpdateMsg = plAccountUpdateMsg::ConvertNoRef(msg);
        if (pUpdateMsg) {
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnAccountUpdate],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnAccountUpdate]),
                                                     _pycs("iii"), (int)pUpdateMsg->GetUpdateType(),
                                                     (int)pUpdateMsg->GetResult(),
                                                     (int)pUpdateMsg->GetPlayerInt());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    if (fPyFunctionInstances[kfunc_gotPublicAgeList]) {
        plNetCommPublicAgeListMsg * pPubAgeMsg = plNetCommPublicAgeListMsg::ConvertNoRef(msg);
        if (pPubAgeMsg) {
            // We would prefer to use the immutable tuple here, but sometimes the public age list
            // will only have one age in it. Python will "helpfully" unpack this tuple for us as
            // the method's arguments. For now, we will fall back to a list.
            pyObjectRef pyEL = PyList_New(pPubAgeMsg->ages.Count());
            for (unsigned i = 0; i<pPubAgeMsg->ages.Count(); ++i) {
                plAgeInfoStruct ageInfo;
                ageInfo.CopyFrom(pPubAgeMsg->ages[i]);
                unsigned nPlayers = pPubAgeMsg->ages[i].currentPopulation;
                unsigned nOwners = pPubAgeMsg->ages[i].population;

                PyObject* t = PyTuple_New(3);
                PyTuple_SET_ITEM(t, 0, pyAgeInfoStruct::New(&ageInfo));
                PyTuple_SET_ITEM(t, 1, PyLong_FromUnsignedLong(nPlayers));
                PyTuple_SET_ITEM(t, 2, PyLong_FromUnsignedLong(nOwners));
                PyList_SET_ITEM(pyEL.Get(), i, t);
            }

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_gotPublicAgeList],
                                                     const_cast<char*>(fFunctionNames[kfunc_gotPublicAgeList]),
                                                     _pycs("O"), pyEL.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    if (fPyFunctionInstances[kfunc_OnAIMsg]) {
        plAIMsg* aiMsg = plAIMsg::ConvertNoRef(msg);
        if (aiMsg) {
            // grab the sender (the armature mod that has our brain)
            plArmatureMod* armMod = plArmatureMod::ConvertNoRef(aiMsg->GetSender()->ObjectIsLoaded());
            pyObjectRef brainObj;
            if (armMod) {
                plArmatureBrain* brain = armMod->FindBrainByClass(plAvBrainCritter::Index());
                plAvBrainCritter* critterBrain = plAvBrainCritter::ConvertNoRef(brain);
                if (critterBrain)
                    brainObj = pyCritterBrain::New(critterBrain);
            }
            if (!brainObj)
                brainObj.SetPyNone();

            // set up the msg type and any args, based on the message we got
            int msgType = plAIMsg::kAIMsg_Unknown;
            pyObjectRef args;
            plAIBrainCreatedMsg* brainCreatedMsg = plAIBrainCreatedMsg::ConvertNoRef(aiMsg);
            if (brainCreatedMsg)
                msgType = plAIMsg::kAIMsg_BrainCreated;

            plAIArrivedAtGoalMsg* arrivedMsg = plAIArrivedAtGoalMsg::ConvertNoRef(aiMsg);
            if (arrivedMsg) {
                msgType = plAIMsg::kAIMsg_ArrivedAtGoal;
                args = PyTuple_New(1);
                PyTuple_SetItem(args.Get(), 0, pyPoint3::New(arrivedMsg->Goal()));
            }

            // if no args were set, simply set to none
            if (!args)
                args.SetPyNone();

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnAIMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnAIMsg]),
                                                     _pycs("OisO"), brainObj.Get(), msgType,
                                                     aiMsg->BrainUserString().c_str(), args.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
            DisplayPythonOutput();
            return true;
        }
    }

    if (fPyFunctionInstances[kfunc_OnGameScoreMsg]) {
        pfGameScoreMsg* pScoreMsg = pfGameScoreMsg::ConvertNoRef(msg);
        if (pScoreMsg) {
            pyObjectRef pyMsg = pyGameScoreMsg::CreateFinal(pScoreMsg);

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef retVal = PyObject_CallMethod(fPyFunctionInstances[kfunc_OnGameScoreMsg],
                                                     const_cast<char*>(fFunctionNames[kfunc_OnGameScoreMsg]),
                                                     _pycs("O"), pyMsg.Get());
            if (!retVal)
                ReportError();
            plProfile_EndTiming(PythonUpdate);
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
    ST::string objectName = this->GetKeyName();
    objectName += " - ";

    PythonInterface::WriteToStdErr(objectName.c_str());

    PyErr_Print();      // make sure the error is printed
    PyErr_Clear();      // clear the error
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
void plPythonFileMod::SetSourceFile(const ST::string& filename)
{
    fPythonFile = filename;
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
        plCmdIfaceModMsg* pModMsg = new plCmdIfaceModMsg;
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
    plCmdIfaceModMsg* pModMsg = new plCmdIfaceModMsg;
    pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
    pModMsg->SetSender(GetKey());
    pModMsg->SetCmd(plCmdIfaceModMsg::kRemove);
    plgDispatch::MsgSend(pModMsg);
}

void plPythonFileMod::Read(hsStream* stream, hsResMgr* mgr)
{
    plMultiModifier::Read(stream, mgr);

    // read in the compile python code (pyc)
    fPythonFile = stream->ReadSafeString();

    // then read in the list of receivers that want to be notified
    int nRcvs = stream->ReadLE32();
    fReceivers.Reset();
    int m;
    for( m=0; m<nRcvs; m++ )
    {   
        fReceivers.Append(mgr->ReadKey(stream));
    }

    // then read in the list of parameters
    int nParms = stream->ReadLE32();
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
    stream->WriteLE32(fReceivers.GetCount());
    int m;
    for( m=0; m<fReceivers.GetCount(); m++ )
        mgr->WriteKey(stream, fReceivers[m]);

    // then write out the list of parameters
    stream->WriteLE32(fParameters.GetCount());
    int i;
    for( i=0; i<fParameters.GetCount(); i++ )
        fParameters[i].Write(stream,mgr);
}

//// kGlobalNameKonstant /////////////////////////////////////////////////
//  My continued attempt to spread the CORRECT way to spell konstant. -mcn

ST::string plPythonFileMod::kGlobalNameKonstant(ST_LITERAL("VeryVerySpecialPythonFileMod"));
