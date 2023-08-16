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

#include <Python.h>
#include "pyKey.h"

#include "plPythonSDLModifier.h"
#include "cyPythonInterface.h"

#include "plPythonFileMod.h"
#include "pyObjectRef.h"
#include "cyMisc.h"

#include <optional>

#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSceneObject.h"
#include "plResMgr/plKeyFinder.h"
#include "plAgeDescription/plAgeDescription.h"

#include "plSDL/plSDL.h"

plStateDataRecord * GetAgeSDL()
{
    const plPythonSDLModifier * mod = plPythonSDLModifier::FindAgeSDL();
    if ( mod )
        return mod->GetStateCache();
    return nullptr;
}


#define PyLoggedAssert(cond, text)                              \
    if (!cond) PythonInterface::WriteToLog(ST_LITERAL(text));   \
    hsAssert(cond, text);

plPythonSDLModifier::plPythonSDLModifier(plPythonFileMod* owner) : fOwner(owner)
{
    plStateDescriptor* desc = plSDLMgr::GetInstance()->FindDescriptor(fOwner->fPythonFile, plSDL::kLatestVersion);
    if (desc) {
        // Create a Python SDL record with no values set
        for (int i = 0; i < desc->GetNumVars(); i++) {
            plVarDescriptor* var = desc->GetVar(i);

            ST::string name = var->GetName();
            int count = var->GetCount();

            fMap[name] = SDLObj(nullptr, count, false);
        }
    }
}

plPythonSDLModifier::~plPythonSDLModifier()
{
    for (auto it : fMap) {
        Py_XDECREF(it.second.obj);
    }
}

PyObject* plPythonSDLModifier::GetItem(const ST::string& key)
{
    auto it = fMap.find(key);

    if (it == fMap.end()) {
        ST::string errmsg = ST::format("SDL key {} not found", key);
        PyErr_SetString(PyExc_KeyError, errmsg.c_str());
        PYTHON_RETURN_ERROR;
    }

    PyObject* val = it->second.obj;
    if (val)
        Py_INCREF(val);
    else
        val = PyTuple_New(0);
    return val;
}

void plPythonSDLModifier::ISetItem(const ST::string& key, PyObject* value)
{
    if (!value || !PyTuple_Check(value)) {
        PyLoggedAssert(0, "[SDL] Trying to set a tuple value to something that isn't a tuple");
        return;
    }

    auto it = fMap.find(key);
    if (it == fMap.end()) {
        PyLoggedAssert(0, "[SDL] Tried to set a nonexistent SDL value");
        return;
    }

    SDLObj& oldObj = it->second;
    if (oldObj.size != 0 && PyTuple_Size(value) != oldObj.size) {
        PyLoggedAssert(0, "[SDL] Wrong size for a fixed size SDL value");
        return;
    }

    Py_XDECREF(oldObj.obj);
    Py_INCREF(value);
    oldObj.obj = value;
}

void plPythonSDLModifier::SendToClients(const ST::string& key)
{
    auto it = fMap.find(key);
    if (it != fMap.end())
        it->second.sendToClients = true;
}

void plPythonSDLModifier::SetNotify(pyKey& selfkey, const ST::string& key, float tolerance)
{
    AddNotifyForVar(selfkey.getKey(), key, tolerance);
}

void plPythonSDLModifier::SetItem(const ST::string& key, PyObject* value)
{
    ISetItem(key, value);
    IDirtySynchState(key);
}

template<>
void plPythonSDLModifier::SetItem(const ST::string& key, int index, bool value)
{
    pyObjectRef pyValue = PyBool_FromLong(value ? 1 : 0);
    SetItemIdx(key, index, pyValue.Get(), true);
}

template<>
void plPythonSDLModifier::SetItem(const ST::string& key, int index, float value)
{
    pyObjectRef pyValue = PyFloat_FromDouble(value);
    SetItemIdx(key, index, pyValue.Get(), true);
}

template<>
void plPythonSDLModifier::SetItem(const ST::string& key, int index, int value)
{
    pyObjectRef pyValue = PyLong_FromLong(value);
    SetItemIdx(key, index, pyValue.Get(), true);
}

void plPythonSDLModifier::SetDefault(const ST::string& key, PyObject* value)
{
    ISetItem(key, value);
}

void plPythonSDLModifier::SetItemIdx(const ST::string& key, int idx, PyObject* value, bool sendImmediate)
{
    if (!value) {
        PyLoggedAssert(0, "[SDL] Trying to set a value to nil");
        return;
    }

    auto it = fMap.find(key);
    if (it == fMap.end()) {
        PyLoggedAssert(0, "[SDL] Tried to set a nonexistent SDL value");
        return;
    }

    PyObject* pyTuple = it->second.obj;
    int size = it->second.size;
    if (size != 0 && idx >= size) {
        PyLoggedAssert(0, "[SDL] Trying to resize a SDL value that can't be");
        return;
    }

    if (pyTuple && pyTuple->ob_refcnt != 1) {
        // others already have references to the tuple and expect it to be immutable, must make a copy
        Py_ssize_t n = PyTuple_Size(pyTuple);
        PyObject* newTuple = PyTuple_New(n);
        for (Py_ssize_t j = 0; j < n; j++) {
            PyObject* item = PyTuple_GetItem(pyTuple, j);
            Py_INCREF(item);
            PyTuple_SET_ITEM(newTuple, j, item);
        }
        Py_DECREF(pyTuple);
        pyTuple = newTuple;
        it->second.obj = newTuple;
    } else if (pyTuple) {
        // The item tuple already exists but only we have a reference to it, so we can change it
        // if needed.
        if (PyTuple_Size(pyTuple) <= idx) {
            Py_ssize_t oldsize = PyTuple_Size(pyTuple);
            _PyTuple_Resize(&pyTuple, idx + 1);

            // initialize the tuple elements to None, because Python don't like NULLs
            for (Py_ssize_t j = oldsize; j < (idx + 1); j++) {
                Py_INCREF(Py_None);
                PyTuple_SetItem(pyTuple, j, Py_None);
            }

            // _PyTuple_Resize may have recreated pyTuple
            it->second.obj = pyTuple;
        }
    } else {
        int newSize = (size == 0) ? idx + 1 : size;
        pyTuple = PyTuple_New(newSize);

        // initialize the tuple elements to None, because Python don't like NULLs
        for (int j = 0; j < newSize; j++) {
            Py_INCREF(Py_None);
            PyTuple_SET_ITEM(pyTuple, j, Py_None);
        }
        it->second.obj = pyTuple;
    }

    Py_INCREF(value);  // PyTuple_SetItem doesn't increment the ref count
    PyTuple_SetItem(pyTuple, idx, value);

    IDirtySynchState(key, sendImmediate);
}


const char* plPythonSDLModifier::GetSDLName() const
{
    // NOTE:  Not changing GetSDLName since most implementations of this
    //   virtual function just return a stored string literal.
    return fOwner->fPythonFile.c_str();
}

void plPythonSDLModifier::SetFlags(const ST::string& name, bool sendImmediate, bool skipOwnershipCheck)
{
    auto it = fMap.find(name);
    if (it != fMap.end()) {
        it->second.sendImmediate = sendImmediate;
        it->second.skipLocalCheck = skipOwnershipCheck;
    }
}

void plPythonSDLModifier::SetTagString(const ST::string& name, const ST::string& tag)
{
    auto it = fMap.find(name);
    if (it != fMap.end())
        it->second.hintString = tag;
}

void plPythonSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
    plStateDataRecord::SimpleVarsList vars;
    int num = srcState->GetUsedVars(&vars);

    for (int i = 0; i < num; i++) {
        plSimpleStateVariable* var = vars[i];

        ST::string name = var->GetName();

        // Get the SDL value in Python format
        pyObjectRef pyVar = ISDLVarToPython(var);

        SetItem(name, pyVar.Get());
    }

    // Notify the Python code that we updated the SDL record
    fOwner->ICallScriptMethod(plPythonFileMod::kfunc_Load);
}

void plPythonSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
    for (auto it : fMap)
        IPythonVarToSDL(dstState, it.first);
}

void plPythonSDLModifier::IDirtySynchState(const ST::string& name, bool sendImmediate)
{
    auto it = fMap.find(name);
    if (it != fMap.end()) {
        uint32_t flags = 0;
        if (it->second.sendToClients)
            flags |= kBCastToClients;
        if (it->second.sendImmediate)
            flags |= kSendImmediately;
        if (it->second.skipLocalCheck)
            flags |= kSkipLocalOwnershipCheck;
        if (sendImmediate)
            flags |= kSendImmediately;

        GetTarget()->DirtySynchState(fOwner->fPythonFile, flags);
    }
}

template<typename T>
std::optional<T> IConvertPythonNumber(PyObject* o) = delete;

template<>
std::optional<int> IConvertPythonNumber(PyObject* o)
{
    if (PyLong_Check(o))
        return PyLong_AsLong(o);
    if (PyNumber_Check(o)) {
        pyObjectRef pyLong = PyNumber_Long(o);
        return PyLong_AsLong(pyLong.Get());
    }
    return std::nullopt;
}

template<>
std::optional<double> IConvertPythonNumber(PyObject* o)
{
    if (PyFloat_Check(o))
        return PyFloat_AS_DOUBLE(o);
    if (PyNumber_Check(o)) {
        pyObjectRef pyFloat = PyNumber_Float(o);
        // pyFloat might have come from some strange land where they return
        // unexpected things, so don't use the unsafe macro here.
        return PyFloat_AsDouble(pyFloat.Get());
    }
    return std::nullopt;
}

template<>
std::optional<float> IConvertPythonNumber(PyObject* o)
{
    if (PyFloat_Check(o))
        return static_cast<float>(PyFloat_AS_DOUBLE(o));
    if (PyNumber_Check(o)) {
        pyObjectRef pyFloat = PyNumber_Float(o);
        // pyFloat might have come from some strange land where they return
        // unexpected things, so don't use the unsafe macro here.
        return static_cast<float>(PyFloat_AsDouble(pyFloat.Get()));
    }
    return std::nullopt;
}

bool plPythonSDLModifier::IPythonVarIdxToSDL(plSimpleStateVariable* var, int varIdx, int type, PyObject* pyVar, 
                                             const ST::string& hintstring)
{
    switch (type) {
    case plVarDescriptor::kShort:
    case plVarDescriptor::kByte:
    case plVarDescriptor::kBool:
    case plVarDescriptor::kInt:
        if (auto v = IConvertPythonNumber<int>(pyVar)) {
            var->Set(v.value(), varIdx);
            if (!hintstring.empty())
                var->GetNotificationInfo().SetHintString(hintstring);
            return true;
        }
        break;

    case plVarDescriptor::kFloat:
        if (auto v = IConvertPythonNumber<float>(pyVar)) {
            var->Set(v.value(), varIdx);
            if (!hintstring.empty())
                var->GetNotificationInfo().SetHintString(hintstring);
            return true;
        }
        break;

    case plVarDescriptor::kString32:
        if (PyUnicode_Check(pyVar)) {
            const char* v = PyUnicode_AsUTF8(pyVar);
            var->Set(v, varIdx);
            if (!hintstring.empty())
                var->GetNotificationInfo().SetHintString(hintstring);
        }
        break;

    case plVarDescriptor::kKey:
        {
            pyKey* key = PythonInterface::GetpyKeyFromPython(pyVar);
            if (key) {
                var->Set(key->getKey(), varIdx);
                if (!hintstring.empty())
                    var->GetNotificationInfo().SetHintString(hintstring);
            }
        }
        break;

    case plVarDescriptor::kDouble:
        if (auto v = IConvertPythonNumber<double>(pyVar)) {
            var->Set(v.value(), varIdx);
            if (!hintstring.empty())
                var->GetNotificationInfo().SetHintString(hintstring);
            return true;
        }
        break;

    case plVarDescriptor::kAgeTimeOfDay:
        break;

    default:
        hsAssert(0, "Not supported yet");
    }

    return false;
}

void plPythonSDLModifier::IPythonVarToSDL(plStateDataRecord* state, const ST::string& name)
{
    plSimpleStateVariable* var = state->FindVar(name);
    PyObject* pyVar = nullptr;
    auto it = fMap.find(name);
    if (it != fMap.end())
        pyVar = it->second.obj;

    if (!var || !pyVar)
        return;

    if (PyTuple_Check(pyVar)) {
        Py_ssize_t count = PyTuple_Size(pyVar);
        plSimpleVarDescriptor::Type type = var->GetSimpleVarDescriptor()->GetType();

        // Ensure that variable length arrays match.
        if (var->GetSimpleVarDescriptor()->IsVariableLength()) {
            if (var->GetCount() != count)
                var->Alloc(count);
        }

        for (Py_ssize_t i = 0; i < count; i++) {
            PyObject* pyVarItem = PyTuple_GetItem(pyVar, i);
            if (pyVarItem)
                IPythonVarIdxToSDL(var, (int)i, type, pyVarItem,it->second.hintString);
        }
    }
}

PyObject* plPythonSDLModifier::ISDLVarIdxToPython(plSimpleStateVariable* var, int type, int idx)
{
    switch (type) {
    case plVarDescriptor::kShort:
    case plVarDescriptor::kByte:
    case plVarDescriptor::kInt:
        {
            int v;
            var->Get(&v, idx);
            return PyLong_FromLong(v);
        }

    case plVarDescriptor::kFloat:
    case plVarDescriptor::kAgeTimeOfDay:
        {
            float v;
            var->Get(&v, idx);
            return PyFloat_FromDouble(v);
        }

    case plVarDescriptor::kBool:
        {
            bool v;
            var->Get(&v, idx);
            return PyBool_FromLong(v ? 1: 0);
        }

    case plVarDescriptor::kString32:
        {
            char v[256];
            var->Get(v, idx);
            return PyUnicode_FromString(v);
        }

    case plVarDescriptor::kKey:
        {
            plKey v;
            var->Get(&v, idx);
            PyObject* keyObj = pyKey::New(v);
            return keyObj;
        }

    case plVarDescriptor::kDouble:
        {
            double v;
            var->Get(&v, idx);
            return PyFloat_FromDouble(v);
        }

//  case plVarDescriptor::kStateDescriptor:
//  case plVarDescriptor::kCreatable:
//  case plVarDescriptor::kVector3:
//  case plVarDescriptor::kPoint3:
//  case plVarDescriptor::kRGB:
//  case plVarDescriptor::kRGBA:
//  case plVarDescriptor::kQuaternion:
    default:
        hsAssert(0, "Not supported yet");
    }

    PYTHON_RETURN_NONE;
}

PyObject* plPythonSDLModifier::ISDLVarToPython(plSimpleStateVariable* var)
{
    plSimpleVarDescriptor::Type type = var->GetSimpleVarDescriptor()->GetType();
    int count = var->GetCount();

    PyObject* pyTuple = PyTuple_New(count);
    for (int i = 0; i < count; i++) {
        PyObject* varVal = ISDLVarIdxToPython(var, type, i);
        PyTuple_SET_ITEM(pyTuple, i, varVal);
    }

    return pyTuple;
}

bool plPythonSDLModifier::HasSDL(const ST::string& pythonFile)
{
    return (plSDLMgr::GetInstance()->FindDescriptor(pythonFile, plSDL::kLatestVersion) != nullptr);
}

const plSDLModifier* ExternFindAgeSDL()
{
    return plPythonSDLModifier::FindAgeSDL();
}

plPythonSDLModifier* ExternFindAgePySDL()
{
    return plPythonSDLModifier::FindAgeSDL();
}

plPythonSDLModifier* plPythonSDLModifier::FindAgeSDL()
{
    ST::string ageName = cyMisc::GetAgeName();

    if (ageName.empty())
        return nullptr; // don't have an age, probably because we're running in max?

    // find the Age Global object
    plLocation loc = plKeyFinder::Instance().FindLocation(ageName, plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
    if (loc.IsValid()) {
        plUoid oid(loc,plPythonFileMod::Index(), plPythonFileMod::kGlobalNameKonstant);
        if (oid.IsValid()) {
            plKey key = hsgResMgr::ResMgr()->FindKey(oid);

            plPythonFileMod *pfmod = plPythonFileMod::ConvertNoRef(key ? key->ObjectIsLoaded() : nullptr);
            if (pfmod) {
                plPythonSDLModifier * sdlMod = pfmod->GetSDLMod();
                if (sdlMod)
                    return sdlMod;

                plNetClientApp::StaticErrorMsg("pfmod {} has a nil python SDL modifier for age sdl {}",
                    pfmod->GetKeyName().c_str("?"), ageName);
            } else {
                if (!key)
                    plNetClientApp::StaticErrorMsg("nil key {} for age sdl {}", ageName, oid.StringIze());
                else if (!key->ObjectIsLoaded())
                    plNetClientApp::StaticErrorMsg("key {} not loaded for age sdl {}",
                                                   key->GetName().c_str("?"), ageName);
                else if (!plPythonFileMod::ConvertNoRef(key->ObjectIsLoaded()))
                    plNetClientApp::StaticErrorMsg("key {} is not a python file mod for age sdl {}",
                                                   key->GetName().c_str("?"), ageName);
            }
        } else {
            plNetClientApp::StaticErrorMsg("Invalid plUoid for age sdl {}", ageName);
        }
    } else {
        plNetClientApp::StaticErrorMsg("Invalid plLocation for age sdl {}", ageName);
    }

    // couldn't find one (maybe because we didn't look)
    return nullptr;
}

plKey ExternFindAgeSDLTarget()
{
    return plPythonSDLModifier::FindAgeSDLTarget();
}

plKey plPythonSDLModifier::FindAgeSDLTarget()
{
    // find the Age Global object
    plLocation loc = plKeyFinder::Instance().FindLocation(cyMisc::GetAgeName(),plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
    if (loc.IsValid()) {
        plUoid oid(loc,plPythonFileMod::Index(), plPythonFileMod::kGlobalNameKonstant);
        if (oid.IsValid()) {
            plKey key = hsgResMgr::ResMgr()->FindKey(oid);

            plPythonFileMod* pfmod = plPythonFileMod::ConvertNoRef(key ? key->GetObjectPtr() : nullptr);
            if (pfmod) {
                if (pfmod->GetTarget(0))
                    return pfmod->GetTarget(0)->GetKey();
            }
        }
    }

    // couldn't find one (maybe because we didn't look)
    return nullptr;
}


/////////////////////////////////////////////

pySDLModifier::pySDLModifier(plPythonSDLModifier* sdlMod)
{
    fRecord = sdlMod;
}


PyObject* pySDLModifier::GetAgeSDL()
{
    ST::string ageName = cyMisc::GetAgeName();
    if (ageName.empty())
        PYTHON_RETURN_NONE; // just return none if the age is blank (running in max?)

    const plPythonSDLModifier* ageSDL = plPythonSDLModifier::FindAgeSDL();
    if (ageSDL)
        return pySDLModifier::New((plPythonSDLModifier*)ageSDL);

    // didn't find one, throw an exception for the python programmer to chew on
    ST::string err = ST::format("Age Global SDL for {} does not exist!", ageName);
    plNetClientApp::StaticErrorMsg(err);
    PyErr_SetString(PyExc_KeyError, err.c_str());
    PYTHON_RETURN_ERROR;
}


void pySDLModifier::SetDefault(pySDLModifier& self, const ST::string& key, PyObject* value)
{
    self.fRecord->SetDefault(key, value);
}

void pySDLModifier::SendToClients(pySDLModifier& self, const ST::string& key)
{
    self.fRecord->SendToClients(key);
}

void pySDLModifier::SetNotify(pySDLModifier& self, pyKey& selfkey, const ST::string& key, float tolerance)
{
    self.fRecord->SetNotify(selfkey, key, tolerance);
}

PyObject* pySDLModifier::GetItem(pySDLModifier& self, const ST::string& key)
{
    return self.fRecord->GetItem(key);
}

void pySDLModifier::SetItem(pySDLModifier& self, const ST::string& key, PyObject* value)
{
    self.fRecord->SetItem(key, value);
}

void pySDLModifier::SetItemIdx(pySDLModifier& self, const ST::string& key, int idx, PyObject* value)
{
    self.fRecord->SetItemIdx(key, idx, value);
}

void pySDLModifier::SetItemIdxImmediate(pySDLModifier& self, const ST::string& key, int idx, PyObject* value)
{
    self.fRecord->SetItemIdx(key, idx, value, true);
}

void pySDLModifier::SetFlags(pySDLModifier& self, const ST::string& name, bool sendImmediate, bool skipOwnershipCheck)
{
    self.fRecord->SetFlags(name,sendImmediate,skipOwnershipCheck);
}

void pySDLModifier::SetTagString(pySDLModifier& self, const ST::string& name, const ST::string& tag)
{
    self.fRecord->SetTagString(name,tag);
}
