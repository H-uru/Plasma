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
#include "plPythonCallable.h"
#include "plPythonConvert.h"
#include "hsResMgr.h"
#include "hsStream.h"

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
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrainCritter.h"
#include "pfMessage/pfGameScoreMsg.h"
#include "plMessage/plSubtitleMsg.h"

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

#include "pyAsyncTask.h"
#include "pyGameScoreMsg.h"

#include "plPythonSDLModifier.h"

#include "plMessage/plTimerCallbackMsg.h"

plProfile_CreateCounterNoReset("AsyncTasks", "Python", PythonAsyncTaskCounter);
plProfile_CreateCounterNoReset("AwaitedTasks", "Python", PythonAwaitedTaskCounter);
plProfile_CreateCounterNoReset("ScriptAwaitables", "Python", PythonScriptAwaitableCounter);
plProfile_CreateTimer("Update", "Python", PythonUpdate);

/////////////////////////////////////////////////////////////////////////////
//
// pfPythonAwaitable - a pending awaitable (read: coroutine) from python scripts
//
struct pfPythonAwaitable
{
    pyObjectRef fAwaitable;
    pyObjectRef fIterable;
    plKey fOwner;

    pfPythonAwaitable(pyObjectRef aws, pyObjectRef it, plKey owner)
        : fAwaitable(std::move(aws)), fIterable(std::move(it)), fOwner(std::move(owner))
    {
        plProfile_Inc(PythonScriptAwaitableCounter);
        fOwner->RefObject();
    }

    pfPythonAwaitable(const pfPythonAwaitable& copy) = delete;

    pfPythonAwaitable(pfPythonAwaitable&& move)
        : fAwaitable(std::move(move.fAwaitable)),
          fIterable(std::move(move.fIterable)),
          fOwner(std::move(move.fOwner))
    {
        // These don't have strong ownership, so we just have to rely on the destruction
        // of `move` to decrement the counter appropriately.
        plProfile_Inc(PythonScriptAwaitableCounter);
        fOwner->RefObject();
    }

    ~pfPythonAwaitable()
    {
        plProfile_Dec(PythonScriptAwaitableCounter);
        fOwner->UnRefObject();
    }
};

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
    "OnSubtitleMsg",        // kfunc_OnSubtitleMsg
    nullptr
};

//// Callback From the Vault Events //////////////////////////////////////////////
class PythonVaultCallback : public VaultCallback
{
protected:
    plPythonFileMod* fPyFileMod;
    plPythonFileMod::func_num fFunctionIdx;

public:
    PythonVaultCallback(plPythonFileMod* pymod, plPythonFileMod::func_num fidx)
        : fPyFileMod(pymod), fFunctionIdx(fidx)
    {
    }

    void AddedChildNode(const hsRef<RelVaultNode>& parentNode,
                        const hsRef<RelVaultNode>& childNode) override
    {
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            PyObject* ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple, 0, pyVaultNodeRef::New(parentNode, childNode));
            fPyFileMod->ICallScriptMethod(fFunctionIdx, (int)pyVault::kVaultNodeRefAdded, ptuple);
        }
    }

    void RemovingChildNode(const hsRef<RelVaultNode>& parentNode,
                           const hsRef<RelVaultNode>& childNode) override
    {
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            PyObject* ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple, 0, pyVaultNodeRef::New(parentNode, childNode));
            fPyFileMod->ICallScriptMethod(fFunctionIdx, (int)pyVault::kVaultRemovingNodeRef, ptuple);
        }
    }

    void ChangedNode(const hsRef<RelVaultNode>& changedNode) override
    {
        if (fPyFileMod && fPyFileMod->fPyFunctionInstances[fFunctionIdx]) {
            PyObject* ptuple = PyTuple_New(1);
            PyTuple_SetItem(ptuple, 0, pyVaultNode::New(changedNode));
            fPyFileMod->ICallScriptMethod(fFunctionIdx, (int)pyVault::kVaultNodeSaved, ptuple);
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
    plPythonFileMod* fMod;

    public:
        pfPythonKeyCatcher(plPythonFileMod *mod )
            : fMod(mod)
        { }

        void HandleKeyEvent(plKeyEventMsg* msg) override
        {
            fMod->ICallScriptMethod(plPythonFileMod::kfunc_OnDefaultKeyCaught,
                                    msg->GetKeyChar(), msg->GetKeyDown(),
                                    msg->GetRepeat(), msg->GetShiftKeyDown(),
                                    msg->GetCtrlKeyDown(), (int)msg->GetKeyCode());
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
    : fModule(), fLocalNotify(true), fIsFirstTimeEval(true),
      fVaultCallback(), fSDLMod(), fSelfKey(), fInstance(), fKeyCatcher(),
      fPipe(), fAmIAttachedToClone()
{
    // assume that all the functions are not available
    // ...if the functions are defined in the module, then we'll call 'em
    int i;
    for (i=0 ; i<kfunc_lastone; i++)
        fPyFunctionInstances[i] = nullptr;
}

plPythonFileMod::~plPythonFileMod()
{
    if (!fAtConvertTime) {
        for (size_t i = 0; fFunctionNames[i] != nullptr; ++i)
            Py_CLEAR(fPyFunctionInstances[i]);

        // remove our reference to the instance (but only if we made one)
        if (fInstance) {
            if (fInstance->ob_refcnt > 1)
                Py_DECREF(fInstance);

            //  then have the glue delete the instance of class
            PyObject* delInst = PythonInterface::GetModuleItem("glue_delInst", fModule);
            if (delInst && PyCallable_Check(delInst)) {
                pyObjectRef retVal = plPython::CallObject(delInst);
                if (!retVal)
                    ReportError();
                DisplayPythonOutput();
            }
        }
        fInstance = nullptr;
    }

    // If we have a key catcher, get rid of it
    delete fKeyCatcher;

    // if we created a Vault callback, undo it and get rid of it
    if (fVaultCallback) {
        // Set the callback for the vault thingy
        VaultUnregisterCallback(fVaultCallback);
        delete fVaultCallback;
    }

    Py_CLEAR(fSelfKey);

    // then get rid of this module
    // NOTE: fModule shouldn't be made in the plugin, only at runtime
    if (!fModuleName.empty() && fModule) {
        //_PyModule_Clear(fModule);
        PyObject* m;
        PyObject* modules = PyImport_GetModuleDict();
        if (modules && (m = PyDict_GetItemString(modules, fModuleName.c_str())) && PyModule_Check(m)) {
            hsStatusMessageF("Module %s removed from python dictionary", fModuleName.c_str());
            PyDict_DelItemString(modules, fModuleName.c_str());
        } else {
            hsStatusMessageF("Module %s not found in python dictionary. Already removed?",fModuleName.c_str());
        }
    }
}

template<typename T>
T* plPythonFileMod::IScriptWantsMsg(func_num methodId, plMessage* msg) const
{
    if (fPyFunctionInstances[methodId])
        return T::ConvertNoRef(msg);
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ICallSciptMethod and friends
//
//  PURPOSE    : Builds Python argument tuple for method calling.
//

namespace plPython
{
    template<>
    inline PyObject* ConvertFrom(ControlEventCode&& value)
    {
        return PyLong_FromLong((long)value);
    }
};

template<typename... Args>
void plPythonFileMod::ICallScriptMethod(func_num methodId, Args&&... args)
{
    PyObject* callable = fPyFunctionInstances[methodId];
    if (!callable)
        return;

    pyObjectRef retVal = plPython::CallObject(callable, std::forward<Args>(args)...);
    if (!retVal)
        ReportError();
    IHandleAwaitable(retVal.Get());
    DisplayPythonOutput();
}

void plPythonFileMod::IHandleAwaitable(PyObject* aw)
{
    if (aw == nullptr)
        return;

    plProfile_BeginTiming(PythonUpdate);
    pyObjectRef iter = pyAsyncTask::GetAsyncIter(aw);
    plProfile_EndTiming(PythonUpdate);
    if (iter) {
        plProfile_BeginTiming(PythonUpdate);
        auto [complete, result] = pyAsyncTask::Pump(iter.Get());
        plProfile_EndTiming(PythonUpdate);
        if (!complete) {
            Py_INCREF(aw);
            fAwaitables.emplace_back(aw, std::move(iter), GetKey());
        }
    }
}

void plPythonFileMod::IPumpAwaitables()
{
    for (auto it = fAwaitables.cbegin(); it != fAwaitables.cend();) {
        plProfile_BeginTiming(PythonUpdate);
        auto [complete, result] = pyAsyncTask::Pump(it->fIterable.Get());
        plProfile_EndTiming(PythonUpdate);
        if (!result)
            ReportError();
        DisplayPythonOutput();

        if (complete)
            it = fAwaitables.erase(it);
        else
            ++it;
    }
}

#include "plPythonPack.h"

bool plPythonFileMod::ILoadPythonCode()
{
#ifndef PLASMA_EXTERNAL_RELEASE
    // get code from file and execute in module
    // see if the file exists first before trying to import it
    plFileName pyfile = plFileName::Join(".", "python", ST::format("{}.py", fPythonFile));
    plFileName gluefile = plFileName::Join(".", "python", "plasma", "glue.py");
    if (plFileInfo(pyfile).Exists() && plFileInfo(gluefile).Exists()) {
        // ok... we can't really use import because Python remembers too much where global variables came from
        // ...and using execfile make it sure that globals are defined in this module and not in the imported module
        // ...but execfile was removed in Python 3 ^_^
        if (PythonInterface::RunFile(pyfile, fModule)) {
            // we've loaded the code into our module
            // now attach the glue python code to the end
            if (PythonInterface::RunFile(gluefile, fModule))
                return true;
        }

        ST::string errMsg = ST::format("Python file {}.py had errors!!! Could not load.", fPythonFile);
        PythonInterface::WriteToLog(errMsg);
        ReportError();
        return false;
    }
#endif  //PLASMA_EXTERNAL_RELEASE

    // Finally, try and find the file in the Python packfile
    // ... for the external users .pak file is only used
    PyObject* pythonCode = PythonPack::OpenPythonPacked(fPythonFile);
    if (pythonCode && PythonInterface::RunPYC(pythonCode, fModule))
        return true;

    ST::string errMsg = ST::format("Python file {}.py was not found.", fPythonFile);
    PythonInterface::WriteToLog(errMsg);
    if (PyErr_Occurred())
        ReportError();
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
            PyObject* pfilename = PyUnicode_FromSTString(fPythonFile);
            PyDict_SetItemString(dict, "glue_name", pfilename);

            // next we need to:
            //  - create instance of class
            PyObject* getInst = PythonInterface::GetModuleItem("glue_getInst",fModule);
            fInstance = nullptr;
            if (getInst && PyCallable_Check(getInst)) {
                fInstance = plPython::CallObject(getInst).Release();
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
            pyObjectRef pInitialState = PyLong_FromLong(0);
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
                            value = PyLong_FromLong(parameter.datarecord.fIntNumber);
                            break;
                        case plPythonParameter::kFloat:
                            value = PyFloat_FromDouble(parameter.datarecord.fFloatNumber);
                            break;
                        case plPythonParameter::kbool:
                            value = PyLong_FromLong(parameter.datarecord.fBool);
                            break;
                        case plPythonParameter::kString:
                        case plPythonParameter::kAnimationName:
                            isNamedAttr = 0;
                            if (check_isNamed && PyCallable_Check(check_isNamed)) {
                                retvalue = plPython::CallObject(check_isNamed, parameter.fID);
                                if (!retvalue ) {
                                    ReportError();
                                    DisplayPythonOutput();
                                }
                                if (retvalue && PyLong_Check(retvalue.Get()) )
                                    isNamedAttr = PyLong_AsLong(retvalue.Get());
                                // is it a NamedActivator
                                if (isNamedAttr == 1 || isNamedAttr == 2) {
                                    if (plAgeLoader::GetInstance()->IsLoadingAge())
                                        fNamedCompQueue.emplace_back(parameter.fString, parameter.fID, (isNamedAttr == 1));
                                    else if (isNamedAttr == 1)
                                        IFindActivatorAndAdd(parameter.fString, parameter.fID);
                                    else
                                        IFindResponderAndAdd(parameter.fString, parameter.fID);
                                }
                            }
                            // if it wasn't a named string then must be normal string type
                            if (isNamedAttr == 0)
                                if (!parameter.fString.empty())
                                    value = PyUnicode_FromSTString(parameter.fString);
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
                        pyObjectRef retVal = plPython::CallObject(
                            setParams,
                            parameter.fID,
                            std::move(value)
                        );
                        if (!retVal)
                            ReportError();
                    }
                }
            }

            // check if we need to register named activators or responders
            if (!fNamedCompQueue.empty())
                plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

            //  - find functions in class they've defined.
            PythonInterface::CheckInstanceForFunctions(fInstance, fFunctionNames, fPyFunctionInstances);

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

            if (fPyFunctionInstances[kfunc_OnSubtitleMsg])
                plgDispatch::Dispatch()->RegisterForExactType(plSubtitleMsg::Index(), GetKey());

            // As the last thing... call the OnInit function if they have one
            ICallScriptMethod(kfunc_Init);

            // Oversight fix... Sometimes PythonFileMods are loaded after the AgeInitialState is received.
            // We should really let the script know about that via OnServerInitComplete anyway because it's
            // not good to make assumptions about game state in workarounds for that method not being called
            plNetClientApp* na = plNetClientApp::GetInstance();
            if (!na->GetFlagsBit(plNetClientApp::kLoadingInitialAgeState) && na->GetFlagsBit(plNetClientApp::kPlayingGame)) {
                plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
                ICallScriptMethod(kfunc_OnServerInitComplete);
            }

            // display python output
            DisplayPythonOutput();
        } else {
            // else if module is already created... Then we are just adding an additional object
            // to the already existing SceneObject. We need to get the instance and add this new
            // target's key to the pySceneObject.
            if (fInstance) {
                pyObjectRef pSceneObject = PyObject_GetAttrString(fInstance, "sceneobject");
                pySceneObject::ConvertFrom(pSceneObject.Get())->addObjKey(sobj->GetKey());
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
//  Function   : IMakeModuleName
//  PARAMETERS : sobj  - object to add as our target
//
//  PURPOSE    : Get the Key of our target
//
// NOTE: This modifier wasn't intended to have multiple targets
//
ST::string plPythonFileMod::IMakeModuleName(const plSceneObject* sobj)
{
    // This strips underscores out of module names so python won't truncate them... -S

    const plKey& pKey = GetKey();
    const plKey& sKey = sobj->GetKey();

    ST::string soName = sKey->GetName().replace("_", "");
    ST::string pmName = pKey->GetName().replace("_", "");

    ST::string_stream name;
    name << soName << pmName;

    // check to see if we are attaching to a clone?
    const plKeyImp* pKeyImp = (plKeyImp*)(sKey);
    if (pKeyImp->GetCloneOwner()) {
        // we have an owner... so we must be a clone.
        // add the cloneID to the end of the module name
        // and set the fIAmAClone flag
        uint32_t cloneID = pKeyImp->GetUoid().GetCloneID();
        name << cloneID;
        fAmIAttachedToClone = true;
    }

    // make sure that the actual modulue will be uniqie
    if (!PythonInterface::IsModuleNameUnique(name.to_string())) {
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
                pyObjectRef retVal = plPython::CallObject(
                    setParams,
                    id,
                    std::move(value)
                );
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
void plPythonFileMod::IFindResponderAndAdd(const ST::string& responderName, int32_t id)
{
    if (!responderName.empty()) {
        std::vector<plKey> keylist;
        const plLocation& loc = GetKey()->GetUoid().GetLocation();
        plKeyFinder::Instance().ReallyStupidResponderSearch(responderName, keylist, loc);
        for (const auto& key : keylist) {
            plPythonParameter parm(id);
            parm.SetToResponder(key);
            AddParameter(parm);
            ISetKeyValue(key, id);
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
void plPythonFileMod::IFindActivatorAndAdd(const ST::string& activatorName, int32_t id)
{
    if (!activatorName.empty()) {
        std::vector<plKey> keylist;
        const plLocation& loc = GetKey()->GetUoid().GetLocation();
        plKeyFinder::Instance().ReallyStupidActivatorSearch(activatorName,keylist, loc);

        for (const auto& key : keylist) {
            plPythonParameter parm(id);
            parm.SetToActivator(key);
            AddParameter(parm);
            ISetKeyValue(key, id);

            // need to add ourselves as a receiver to their list
            if (plLogicModifier* logic = plLogicModifier::ConvertNoRef(key->ObjectIsLoaded())) {
                logic->AddNotifyReceiver(GetKey());
            } else if (plPythonFileMod* pymod = plPythonFileMod::ConvertNoRef(key->ObjectIsLoaded())) {
                pymod->AddToNotifyList(GetKey());
            } else {
                hsgResMgr::ResMgr()->AddViaNotify(key,
                                                  new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, kAddNotify, 0),
                                                  plRefFlags::kPassiveRef);
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
            ICallScriptMethod(kfunc_FirstUpdate);
        }

        ICallScriptMethod(kfunc_Update, secs, del);
        IPumpAwaitables();
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
    plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (genRefMsg) {
        // is it a ref for a named activator that we need to add to notify?
        if ((genRefMsg->GetContext() & plRefMsg::kOnCreate) && genRefMsg->fWhich == kAddNotify) {
            if (plLogicModifier* logic = plLogicModifier::ConvertNoRef(genRefMsg->GetRef())) {
                logic->AddNotifyReceiver(GetKey());
            } else if (plPythonFileMod* pymod = plPythonFileMod::ConvertNoRef(genRefMsg->GetRef())) {
                pymod->AddToNotifyList(GetKey());
            }
        }
    }

    plAgeLoadedMsg* ageLoadedMsg = plAgeLoadedMsg::ConvertNoRef(msg);
    if (ageLoadedMsg && ageLoadedMsg->fLoaded) {
        for (const auto& comp : fNamedCompQueue) {
            if (comp.isActivator)
                IFindActivatorAndAdd(comp.name, comp.id);
            else
                IFindResponderAndAdd(comp.name, comp.id);
        }
        fNamedCompQueue.clear();
        plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
    }

    // if this is a render message, then we are just trying to get a pointer to the Pipeline
    plRenderMsg* rMsg = plRenderMsg::ConvertNoRef(msg);
    if (rMsg) {
        fPipe = rMsg->Pipeline();
        plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
        return true;
    }

    // are they looking for an Notify message? should be coming from a proActivator
    auto pNtfyMsg = IScriptWantsMsg<plNotifyMsg>(kfunc_Notify, msg);
    if (pNtfyMsg) {
        // Cache the whether or not this is a local notification for calls to PtWasLocallyNotified()
        fLocalNotify = !pNtfyMsg->HasBCastFlag(plMessage::kNetNonLocal);

        PyObject* levents = PyTuple_New(pNtfyMsg->GetEventCount());
        for (size_t i = 0; i < pNtfyMsg->GetEventCount(); i++) {
            proEventData* pED = pNtfyMsg->GetEventRecord(i);
            switch (pED->fEventType) {
                case proEventData::kCollision:
                    {
                        proCollisionEventData* eventData = (proCollisionEventData*)pED;

                        PyObject* event = PyTuple_New(4);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kCollision));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fEnter ? 1 : 0));
                        PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fHitter, fSelfKey));
                        PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fHittee, fSelfKey));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;
                        
                case proEventData::kSpawned:
                    {
                        proSpawnedEventData* eventData = (proSpawnedEventData*)pED;

                        PyObject* event = PyTuple_New(3);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kSpawned));
                        PyTuple_SET_ITEM(event, 1, pySceneObject::New(eventData->fSpawner, fSelfKey));
                        PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fSpawnee, fSelfKey));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kPicked:
                    {
                        proPickedEventData* eventData = (proPickedEventData*)pED;
                        PyObject* event = PyTuple_New(6);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kPicked));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fEnabled ? 1 : 0));
                        PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fPicker, fSelfKey));
                        PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fPicked, fSelfKey));
                        PyTuple_SET_ITEM(event, 4, pyPoint3::New(eventData->fHitPoint));

                        // make it in the local space
                        hsPoint3 tolocal;
                        if (eventData->fPicked){
                            plSceneObject* obj = plSceneObject::ConvertNoRef(eventData->fPicked->ObjectIsLoaded());
                            if (obj) {
                                const plCoordinateInterface* ci = obj->GetCoordinateInterface();
                                if (ci)
                                    tolocal = (hsMatrix44)ci->GetWorldToLocal() * eventData->fHitPoint;
                            }
                        }
                        PyTuple_SET_ITEM(event, 5, pyPoint3::New(tolocal));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kControlKey:
                    {
                        proControlKeyEventData* eventData = (proControlKeyEventData*)pED;

                        PyObject* event = PyTuple_New(3);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kControlKey));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fControlKey));
                        PyTuple_SET_ITEM(event, 2, PyLong_FromLong(eventData->fDown ? 1 : 0));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kVariable:
                    {
                        proVariableEventData* eventData = (proVariableEventData*)pED;
                        // create event list
                        PyObject* event = PyTuple_New(4);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kVariable));
                        PyTuple_SET_ITEM(event, 1, PyUnicode_FromSTString(eventData->fName));
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
                                PyTuple_SET_ITEM(event, 3, PyLong_FromLong(eventData->fNumber.i));
                                break;
                            default:
                                Py_INCREF(Py_None);
                                PyTuple_SET_ITEM(event, 3, Py_None);
                                break;
                        }
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kFacing:
                    {
                        proFacingEventData* eventData = (proFacingEventData*)pED;
                        PyObject* event = PyTuple_New(5);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kFacing));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->enabled ? 1 : 0));
                        PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fFacer, fSelfKey));
                        PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fFacee, fSelfKey));
                        PyTuple_SET_ITEM(event, 4, PyFloat_FromDouble(eventData->dot));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kContained:
                    {
                        proContainedEventData* eventData = (proContainedEventData*)pED;

                        PyObject* event = PyTuple_New(4);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kContained));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fEntering ? 1 : 0));
                        PyTuple_SET_ITEM(event, 2, pySceneObject::New(eventData->fContained, fSelfKey));
                        PyTuple_SET_ITEM(event, 3, pySceneObject::New(eventData->fContainer, fSelfKey));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kActivate:
                    {
                        proActivateEventData* eventData = (proActivateEventData*)pED;

                        PyObject* event = PyTuple_New(3);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kActivate));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fActive ? 1 : 0));
                        PyTuple_SET_ITEM(event, 2, PyLong_FromLong(eventData->fActivate ? 1 : 0));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kCallback:
                    {
                        proCallbackEventData* eventData = (proCallbackEventData*)pED;

                        PyObject* event = PyTuple_New(2);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kCallback));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fEventType));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;

                case proEventData::kResponderState:
                    {
                        proResponderStateEventData* eventData = (proResponderStateEventData*)pED;

                        PyObject* event = PyTuple_New(2);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kResponderState));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromLong(eventData->fState));
                        PyTuple_SET_ITEM(levents, i, event);
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
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;
                case proEventData::kOfferLinkingBook:
                    {
                        proOfferLinkingBookEventData* eventData = (proOfferLinkingBookEventData*)pED;

                        PyObject* event = PyTuple_New(4);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kOfferLinkingBook));
                        PyTuple_SET_ITEM(event, 1, pySceneObject::New(eventData->offerer, fSelfKey));
                        PyTuple_SET_ITEM(event, 2, PyLong_FromLong(eventData->targetAge));
                        PyTuple_SET_ITEM(event, 3, PyLong_FromLong(eventData->offeree));
                        PyTuple_SET_ITEM(levents, i, event);
                    }
                    break;
                case proEventData::kBook:
                    {
                        proBookEventData* eventData = (proBookEventData*)pED;

                        PyObject* event = PyTuple_New(3);
                        PyTuple_SET_ITEM(event, 0, PyLong_FromLong((long)proEventData::kBook));
                        PyTuple_SET_ITEM(event, 1, PyLong_FromUnsignedLong(eventData->fEvent));
                        PyTuple_SET_ITEM(event, 2, PyLong_FromUnsignedLong(eventData->fLinkID));
                        PyTuple_SET_ITEM(levents, i, event);
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

        ICallScriptMethod(kfunc_Notify, pNtfyMsg->fState, id, levents);
        return true;
    }

    // are they looking for a key event message?
    auto pEMsg = IScriptWantsMsg<plControlEventMsg>(kfunc_OnKeyEvent, msg);
    if (pEMsg) {
        ICallScriptMethod(kfunc_OnKeyEvent, pEMsg->GetControlCode(), pEMsg->ControlActivated());
        return true;
    }

    // are they looking for an Timer message?
    auto pTimerMsg = IScriptWantsMsg<plTimerCallbackMsg>(kfunc_AtTimer, msg);
    if (pTimerMsg) {
        ICallScriptMethod(kfunc_AtTimer, pTimerMsg->fID);
        return true;
    }

    // are they looking for an GUINotify message?
    auto pGUIMsg = IScriptWantsMsg<pfGUINotifyMsg>(kfunc_GUINotify, msg);
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
        ICallScriptMethod(kfunc_GUINotify, id, std::move(pyControl), pGUIMsg->GetEvent());
        return true;
    }

    // are they looking for an RoomLoadNotify message?
    auto pRLNMsg = IScriptWantsMsg<plRoomLoadNotifyMsg>(kfunc_PageLoad, msg);
    if (pRLNMsg) {
        ICallScriptMethod(kfunc_PageLoad, pRLNMsg->GetWhatHappen(),
                          pRLNMsg->GetRoom() ? pRLNMsg->GetRoom()->GetName() : ST::string());
        return true;
    }


    // are they looking for an ClothingUpdate message?
    auto pCUMsg = IScriptWantsMsg<plClothingUpdateBCMsg>(kfunc_ClothingUpdate, msg);
    if (pCUMsg) {
        ICallScriptMethod(kfunc_ClothingUpdate);
        return true;
    }

    // are they looking for an KIMsg message?
    auto pkimsg = IScriptWantsMsg<pfKIMsg>(kfunc_KIMsg, msg);
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
                PyTuple_SET_ITEM(value.Get(), 0, PyUnicode_FromSTString(pkimsg->GetUser()));
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

        ICallScriptMethod(kfunc_KIMsg, pkimsg->GetCommand(), std::move(value));
        return true;
    }

    // are they looking for an MemberUpdate message?
    auto pmumsg = IScriptWantsMsg<plMemberUpdateMsg>(kfunc_MemberUpdate, msg);
    if (pmumsg) {
        ICallScriptMethod(kfunc_MemberUpdate);
        return true;
    }

    // are they looking for a RemoteAvatar Info message?
    auto pramsg = IScriptWantsMsg<plRemoteAvatarInfoMsg>(kfunc_RemoteAvatarInfo, msg);
    if (pramsg) {
        pyObjectRef player;
        if (pramsg->GetAvatarKey()) {
            // try to create the pyPlayer for where this message came from
            plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMemberByKey(pramsg->GetAvatarKey());
            if (mbr)
                player = pyPlayer::New(mbr->GetAvatarKey(), mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
        }
        if (!player)
            player = PyLong_FromLong(0);
        ICallScriptMethod(kfunc_RemoteAvatarInfo, std::move(player));
        return true;
    }


    // are they looking for a CCR communication message?
    auto ccrmsg = IScriptWantsMsg<plCCRCommunicationMsg>(kfunc_OnCCRMsg, msg);
    if (ccrmsg) {
        const char* textmessage = ccrmsg->GetMessage();
        if (!textmessage)
            textmessage = "";
        ICallScriptMethod(kfunc_OnCCRMsg, (int)ccrmsg->GetType(), textmessage, ccrmsg->GetCCRPlayerID());
        return true;
    }

    // are they looking for a VaultNotify message?
    auto vaultNotifyMsg = IScriptWantsMsg<plVaultNotifyMsg>(kfunc_OnVaultNotify, msg);
    if (vaultNotifyMsg) {
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
                        PyTuple_SET_ITEM(ptuple.Get(), 0, PyUnicode_FromSTString(ageName));
                    }
                }
                break;

                default:
                    ptuple = PyTuple_New(0);
                    break;
            }

            ICallScriptMethod(kfunc_OnVaultNotify, vaultNotifyMsg->GetType(), std::move(ptuple));
        }
        return true;
    }

    // are they looking for a RealTimeChat message?
    pkimsg = IScriptWantsMsg<pfKIMsg>(kfunc_RTChat, msg);
    if (pkimsg && pkimsg->GetCommand() == pfKIMsg::kHACKChatMsg) {
        if (!VaultAmIgnoringPlayer(pkimsg->GetPlayerID())) {
            PyObject* player;
            plNetTransportMember *mbr = plNetClientMgr::GetInstance()->TransportMgr().GetMemberByID(pkimsg->GetPlayerID());
            if (mbr) {
                player = pyPlayer::New(mbr->GetAvatarKey(), pkimsg->GetUser(), mbr->GetPlayerID(), mbr->GetDistSq());
            } else {
                // else if we could not find the player in our list, then just return a string of the user's name
                ST::string fromName = pkimsg->GetUser();
                if (fromName.empty())
                    fromName = ST_LITERAL("Anonymous Coward");
                player = pyPlayer::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fromName, pkimsg->GetPlayerID(), 0.0);
            }

            ICallScriptMethod(kfunc_RTChat, player, pkimsg->GetString(), pkimsg->GetFlags());
        }
    }

    auto ppMsg = IScriptWantsMsg<plPlayerPageMsg>(kfunc_AvatarPage, msg);
    if (ppMsg) {
        pyObjectRef pSobj = pySceneObject::New(ppMsg->fPlayer, fSelfKey);
        plSynchEnabler ps(true);    // enable dirty state tracking during shutdown
        ICallScriptMethod(kfunc_AvatarPage, std::move(pSobj), !ppMsg->fUnload, ppMsg->fLastOut);
        return true;
    }

    auto pABLMsg = IScriptWantsMsg<plAgeBeginLoadingMsg>(kfunc_OnBeginAgeLoad, msg);
    if (pABLMsg) {
        pyObjectRef pSobj = pySceneObject::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), fSelfKey);
        plSynchEnabler ps(true);    // enable dirty state tracking during shutdowny
        ICallScriptMethod(kfunc_OnBeginAgeLoad, std::move(pSobj));
        return true;
    }

    // initial server update complete message
    if (plInitialAgeStateLoadedMsg::ConvertNoRef(msg)) {
        if (fInstance) {
            // set the isInitialStateLoaded to that it is loaded
            pyObjectRef pInitialState = PyLong_FromLong(1);
            PyObject_SetAttrString(fInstance, "isInitialStateLoaded", pInitialState.Get());
        }

        ICallScriptMethod(kfunc_OnServerInitComplete);
        return true;
    }

    auto sn = IScriptWantsMsg<plSDLNotificationMsg>(kfunc_SDLNotify, msg);
    if (sn) {
        ICallScriptMethod(kfunc_SDLNotify, sn->fVar->GetName().c_str(), sn->fSDLName.c_str(),
                          sn->fPlayerID, sn->fHintString.c_str());
        return true;
    }

    // are they looking for a plNetOwnershipMsg message?
    auto nom = IScriptWantsMsg<plNetOwnershipMsg>(kfunc_OwnershipNotify, msg);
    if (nom) {
        ICallScriptMethod(kfunc_OwnershipNotify);
        return true;
    }

    // are they looking for a pfMarkerMsg message?
    auto markermsg = IScriptWantsMsg<pfMarkerMsg>(kfunc_OnMarkerMsg, msg);
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

        ICallScriptMethod(kfunc_OnMarkerMsg, (int)markermsg->fType, std::move(ptuple));
        return true;
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    // are they looking for a pfBackdoorMsg message?
    auto dt = IScriptWantsMsg<pfBackdoorMsg>(kfunc_OnBackdoorMsg, msg);
    if (dt) {
        ICallScriptMethod(kfunc_OnBackdoorMsg, dt->GetTarget().c_str(), dt->GetString().c_str());
        return true;
    }
#endif  //PLASMA_EXTERNAL_RELEASE

    // are they looking for a plLOSHitMsg message?
    auto pLOSMsg = IScriptWantsMsg<plLOSHitMsg>(kfunc_OnLOSNotify, msg);
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

        ICallScriptMethod(
            kfunc_OnLOSNotify,
            pLOSMsg->fRequestID,
            pLOSMsg->fNoHit,
            std::move(scobj),
            std::move(hitpoint),
            pLOSMsg->fDistance
        );
        return true;
    }

    // are they looking for a plAvatarBehaviorNotifyMsg message?
    auto behNotifymsg = IScriptWantsMsg<plAvatarBehaviorNotifyMsg>(kfunc_OnBehaviorNotify, msg);
    if (behNotifymsg) {
        // the parent of the sender should be the avatar that did the behavior
        pyObjectRef pSobj;

        plModifier* avmod = plModifier::ConvertNoRef(behNotifymsg->GetSender()->ObjectIsLoaded());
        if (avmod && avmod->GetNumTargets())
            pSobj = pySceneObject::New(avmod->GetTarget(0)->GetKey(), fSelfKey);
        else
            pSobj.SetPyNone();

        ICallScriptMethod(
            kfunc_OnBehaviorNotify,
            behNotifymsg->fType,
            std::move(pSobj),
            behNotifymsg->state
        );
        return true;
    }

    // are they looking for a pfMovieEventMsg message?
    auto moviemsg = IScriptWantsMsg<pfMovieEventMsg>(kfunc_OnMovieEvent, msg);
    if (moviemsg) {
        ICallScriptMethod(kfunc_OnMovieEvent, moviemsg->fMovieName.AsString().c_str(),
                          (int)moviemsg->fReason);
        return true;
    }

    // are they looking for a plCaptureRenderMsg message?
    auto capturemsg = IScriptWantsMsg<plCaptureRenderMsg>(kfunc_OnScreenCaptureDone, msg);
    if (capturemsg) {
        pyObjectRef pSobj;
        if (capturemsg->GetMipmap())
            pSobj = pyImage::New(capturemsg->GetMipmap());
        else
            pSobj.SetPyNone();
        ICallScriptMethod(kfunc_OnScreenCaptureDone, std::move(pSobj));
        return true;
    }

    auto pEvent = IScriptWantsMsg<plClimbEventMsg>(kfunc_OnClimbBlockerEvent, msg);
    if (pEvent) {
        pyObjectRef pSobj = pySceneObject::New(pEvent->GetSender(), fSelfKey);
        ICallScriptMethod(kfunc_OnClimbBlockerEvent, std::move(pSobj));
        return true;
    }

    auto pSpawn = IScriptWantsMsg<plAvatarSpawnNotifyMsg>(kfunc_OnAvatarSpawn, msg);
    if (pSpawn) {
        ICallScriptMethod(kfunc_OnAvatarSpawn, true);
        return true;
    }

    auto pUpdateMsg = IScriptWantsMsg<plAccountUpdateMsg>(kfunc_OnAccountUpdate, msg);
    if (pUpdateMsg) {
        ICallScriptMethod(kfunc_OnAccountUpdate, pUpdateMsg->GetUpdateType(), pUpdateMsg->GetResult(),
                          pUpdateMsg->GetPlayerInt());
        return true;
    }

    auto pPubAgeMsg = IScriptWantsMsg<plNetCommPublicAgeListMsg>(kfunc_gotPublicAgeList, msg);
    if (pPubAgeMsg) {
        PyObject* pyEL = PyTuple_New(pPubAgeMsg->ages.size());
        for (size_t i = 0; i < pPubAgeMsg->ages.size(); ++i) {
            plAgeInfoStruct ageInfo;
            ageInfo.CopyFrom(pPubAgeMsg->ages[i]);
            unsigned nPlayers = pPubAgeMsg->ages[i].currentPopulation;
            unsigned nOwners = pPubAgeMsg->ages[i].population;

            PyObject* t = PyTuple_New(3);
            PyTuple_SET_ITEM(t, 0, pyAgeInfoStruct::New(&ageInfo));
            PyTuple_SET_ITEM(t, 1, PyLong_FromUnsignedLong(nPlayers));
            PyTuple_SET_ITEM(t, 2, PyLong_FromUnsignedLong(nOwners));
            PyTuple_SET_ITEM(pyEL, i, t);
        }

        ICallScriptMethod(kfunc_gotPublicAgeList, pyEL);
        return true;
    }

    auto aiMsg = IScriptWantsMsg<plAIMsg>(kfunc_OnAIMsg, msg);
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

        if (plAIBrainCreatedMsg::ConvertNoRef(aiMsg))
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

        ICallScriptMethod(
            kfunc_OnAIMsg,
            std::move(brainObj),
            msgType,
            aiMsg->BrainUserString().c_str(),
            std::move(args)
        );
        return true;
    }

    auto pScoreMsg = IScriptWantsMsg<pfGameScoreMsg>(kfunc_OnGameScoreMsg, msg);
    if (pScoreMsg) {
        pyObjectRef score = pyGameScoreMsg::CreateFinal(pScoreMsg);
        ICallScriptMethod(
            kfunc_OnGameScoreMsg,
            std::move(score)
        );
        return true;
    }

    // are they looking for a subtitle notification message?
    auto pSubMsg = IScriptWantsMsg<plSubtitleMsg>(kfunc_OnSubtitleMsg, msg);
    if (pSubMsg) {
        ICallScriptMethod(kfunc_OnSubtitleMsg, pSubMsg->GetText(), pSubMsg->GetSpeaker());
        return true;
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
void plPythonFileMod::ReportError() const
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
    if (fPyFunctionInstances[kfunc_OnKeyEvent]) {
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

    // read in the name of the python script
    fPythonFile = stream->ReadSafeString();

    // then read in the list of receivers that want to be notified
    uint32_t nRcvs = stream->ReadLE32();
    fReceivers.reserve(nRcvs);
    for (size_t i = 0; i < nRcvs; i++)
        fReceivers.push_back(mgr->ReadKey(stream));

    // then read in the list of parameters
    uint32_t nParms = stream->ReadLE32();
    fParameters.resize(nParms);
    for (size_t i = 0; i < nParms; i++)
        fParameters[i].Read(stream, mgr);
}

void plPythonFileMod::Write(hsStream* stream, hsResMgr* mgr)
{
    plMultiModifier::Write(stream, mgr);

    stream->WriteSafeString(fPythonFile);

    // then write out the list of receivers that want to be notified
    stream->WriteLE32((uint32_t)fReceivers.size());
    for (size_t i = 0; i < fReceivers.size(); i++)
        mgr->WriteKey(stream, fReceivers[i]);

    // then write out the list of parameters
    stream->WriteLE32((uint32_t)fParameters.size());
    for (size_t i = 0; i < fParameters.size(); i++)
        fParameters[i].Write(stream, mgr);
}

//// kGlobalNameKonstant /////////////////////////////////////////////////
//  My continued attempt to spread the CORRECT way to spell konstant. -mcn

ST::string plPythonFileMod::kGlobalNameKonstant(ST_LITERAL("VeryVerySpecialPythonFileMod"));
