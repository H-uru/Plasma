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

#include "cyAvatar.h"

#include <string_theory/string>

#include "plFileSystem.h"

#include "plAvatar/plAvBrainHuman.h"

#include "pyColor.h"
#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pySceneObject.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptAvatar, cyAvatar);

PYTHON_DEFAULT_NEW_DEFINITION(ptAvatar, cyAvatar);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAvatar);

PYTHON_NO_INIT_DEFINITION(ptAvatar);

PYTHON_METHOD_DEFINITION(ptAvatar, netForce, args)
{
    char forceFlag;
    if (!PyArg_ParseTuple(args, "b", &forceFlag))
    {
        PyErr_SetString(PyExc_TypeError, "netForce expects a boolean");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->SetNetForce(forceFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, oneShot, args)
{
    PyObject* keyObj = nullptr;
    float duration;
    char usePhysics;
    ST::string animName;
    char drivable, reversable;
    if (!PyArg_ParseTuple(args, "OfbO&bb", &keyObj, &duration, &usePhysics, PyUnicode_STStringConverter, &animName, &drivable, &reversable))
    {
        PyErr_SetString(PyExc_TypeError, "oneShot expects a ptKey, float, boolean, string, and two booleans");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "oneShot expects a ptKey, float, boolean, string, and two booleans");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->OneShot(*key, duration, usePhysics != 0, animName, drivable != 0, reversable != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, runBehavior, args)
{
    PyObject* keyObj = nullptr;
    char netForce;
    char netProp = 1;
    if (!PyArg_ParseTuple(args, "Ob|b", &keyObj, &netForce, &netProp))
    {
        PyErr_SetString(PyExc_TypeError, "runBehavior expects a ptKey and a boolean and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "runBehavior expects a ptKey and a boolean and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->RunBehavior(*key, netForce != 0, netProp != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, runBehaviorSetNotify, args)
{
    PyObject* behKeyObj = nullptr;
    PyObject* replyKeyObj = nullptr;
    char netForce;
    char netProp = 1;
    if (!PyArg_ParseTuple(args, "OOb|b", &behKeyObj, &replyKeyObj, &netForce, &netProp))
    {
        PyErr_SetString(PyExc_TypeError, "runBehaviorSetNotify expects two ptKeys and a boolean and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(behKeyObj)) || (!pyKey::Check(replyKeyObj)))
    {
        PyErr_SetString(PyExc_TypeError, "runBehaviorSetNotify expects two ptKeys and a boolean and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    pyKey* behKey = pyKey::ConvertFrom(behKeyObj);
    pyKey* replyKey = pyKey::ConvertFrom(replyKeyObj);
    self->fThis->RunBehaviorAndReply(*behKey, *replyKey, netForce != 0, netProp != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, runCoopAnim, args)
{
    PyObject* keyObj;
    ST::string animName1;
    ST::string animName2;
    float range = 6;
    float dist = 3;
    bool move = true;
    if (!PyArg_ParseTuple(args, "OO&O&|ffb", &keyObj, PyUnicode_STStringConverter, &animName1,
                          PyUnicode_STStringConverter, &animName2, &range, &dist, &move) || !pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "runCoopAnim expects a ptkey and two strings and an optional float and boolean");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    PYTHON_RETURN_BOOL(self->fThis->RunCoopAnim(*key, animName1, animName2, range, dist, move));
}

PYTHON_METHOD_DEFINITION(ptAvatar, nextStage, args)
{
    PyObject* keyObj = nullptr;
    float transTime;
    char setTime;
    float newTime;
    char setDirection, isForward, netForce;
    if (!PyArg_ParseTuple(args, "Ofbfbbb", &keyObj, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "nextStage expects a ptkey, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "nextStage expects a ptkey, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->NextStage(*key, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, previousStage, args)
{
    PyObject* keyObj = nullptr;
    float transTime;
    char setTime;
    float newTime;
    char setDirection, isForward, netForce;
    if (!PyArg_ParseTuple(args, "Ofbfbbb", &keyObj, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->PreviousStage(*key, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, gotoStage, args)
{
    PyObject* keyObj = nullptr;
    long stage;
    float transTime;
    char setTime;
    float newTime;
    char setDirection, isForward, netForce;
    if (!PyArg_ParseTuple(args, "Olfbfbbb", &keyObj, &stage, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, long, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, long, float, bool, float, and three booleans");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->GoToStage(*key, stage, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getAvatarClothingGroup)
{
    return PyLong_FromLong(self->fThis->GetAvatarClothingGroup());
}

PYTHON_METHOD_DEFINITION(ptAvatar, getEntireClothingList, args)
{
    long clothingType;
    if (!PyArg_ParseTuple(args, "l", &clothingType))
    {
        PyErr_SetString(PyExc_TypeError, "getEntireClothingList expects a long");
        PYTHON_RETURN_ERROR;
    }

    std::vector<ST::string> clothingList = self->fThis->GetEntireClothingList(clothingType);
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, PyUnicode_FromSTString(clothingList[i]));
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getClosetClothingList, args)
{
    long clothingType;
    if (!PyArg_ParseTuple(args, "l", &clothingType))
    {
        PyErr_SetString(PyExc_TypeError, "getClosetCothingList expects a long");
        PYTHON_RETURN_ERROR;
    }

    std::vector<PyObject*> clothingList = self->fThis->GetClosetClothingList(clothingType);
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
    return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getAvatarClothingList)
{
    std::vector<PyObject*> clothingList = self->fThis->GetAvatarClothingList();
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getMatchingClothingItem, args)
{
    ST::string clothingName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &clothingName))
    {
        PyErr_SetString(PyExc_TypeError, "getMatchingClothingItem expects a string");
        PYTHON_RETURN_ERROR;
    }

    return self->fThis->GetMatchingClothingItem(clothingName);
}

PYTHON_METHOD_DEFINITION(ptAvatar, wearClothingItem, args)
{
    ST::string clothingName;
    char update = 1;
    if (!PyArg_ParseTuple(args, "O&|b", PyUnicode_STStringConverter, &clothingName, &update))
    {
        PyErr_SetString(PyExc_TypeError, "wearClothingItem expects a string and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(self->fThis->WearClothingItemU(clothingName, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, removeClothingItem, args)
{
    ST::string clothingName;
    char update = 1;
    if (!PyArg_ParseTuple(args, "O&|b", PyUnicode_STStringConverter, &clothingName, &update))
    {
        PyErr_SetString(PyExc_TypeError, "removeClothingItem expects a string and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(self->fThis->RemoveClothingItemU(clothingName, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintClothingItem, args)
{
    ST::string clothingName;
    PyObject* tintObj = nullptr;
    char update = 1;
    if (!PyArg_ParseTuple(args, "O&O|b", PyUnicode_STStringConverter, &clothingName, &tintObj, &update))
    {
        PyErr_SetString(PyExc_TypeError, "tintClothingItem expects a string, a ptColor, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(tintObj))
    {
        PyErr_SetString(PyExc_TypeError, "tintClothingItem expects a string, a ptColor, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    pyColor* tint = pyColor::ConvertFrom(tintObj);
    PYTHON_RETURN_BOOL(self->fThis->TintClothingItemU(clothingName, *tint, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintClothingItemLayer, args)
{
    ST::string clothingName;
    PyObject* tintObj = nullptr;
    unsigned char layer;
    char update = 1;
    if (!PyArg_ParseTuple(args, "O&OB|b", PyUnicode_STStringConverter, &clothingName, &tintObj, &layer, &update))
    {
        PyErr_SetString(PyExc_TypeError, "tintClothingItemLayer expects a string, a ptColor, an unsigned 8-bit int, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(tintObj))
    {
        PyErr_SetString(PyExc_TypeError, "tintClothingItemLayer expects a string, a ptColor, an unsigned 8-bit int, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }

    pyColor* tint = pyColor::ConvertFrom(tintObj);
    PYTHON_RETURN_BOOL(self->fThis->TintClothingItemLayerU(clothingName, *tint, layer, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, getTintClothingItem, args)
{
    ST::string clothingName;
    unsigned char layer = 1;
    if (!PyArg_ParseTuple(args, "O&|B", PyUnicode_STStringConverter, &clothingName, &layer))
    {
        PyErr_SetString(PyExc_TypeError, "getTintClothingItem expects a string and an optional unsigned 8-bit int");
        PYTHON_RETURN_NONE;
    }
    
    return self->fThis->GetTintClothingItemL(clothingName, layer);
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintSkin, args)
{
    PyObject* tintObj = nullptr;
    char update = 1;
    if (!PyArg_ParseTuple(args, "O|b", &tintObj, &update))
    {
        PyErr_SetString(PyExc_TypeError, "tintSkin expects a ptColor and an optional boolean");
        PYTHON_RETURN_NONE;
    }
    if (!pyColor::Check(tintObj))
    {
        PyErr_SetString(PyExc_TypeError, "tintSkin expects a ptColor and an optional boolean");
        PYTHON_RETURN_NONE;
    }

    pyColor* tint = pyColor::ConvertFrom(tintObj);
    self->fThis->TintSkinU(*tint, update != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getTintSkin)
{
    return self->fThis->GetTintSkin();
}

PYTHON_METHOD_DEFINITION(ptAvatar, enterSubWorld, args)
{
    PyObject* sceneObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &sceneObj))
    {
        PyErr_SetString(PyExc_TypeError, "enterSubWorld expects a ptSceneObject");
        PYTHON_RETURN_ERROR;
    }
    if (!pySceneObject::Check(sceneObj))
    {
        PyErr_SetString(PyExc_TypeError, "enterSubWorld expects a ptSceneObject");
        PYTHON_RETURN_ERROR;
    }

    pySceneObject* target = pySceneObject::ConvertFrom(sceneObj);
    self->fThis->EnterSubWorld(*target);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptAvatar, exitSubWorld, ExitSubWorld)

PYTHON_METHOD_DEFINITION(ptAvatar, setMorph, args)
{
    ST::string clothingName;
    unsigned char layer;
    float value;
    if (!PyArg_ParseTuple(args, "O&Bf", PyUnicode_STStringConverter, &clothingName, &layer, &value))
    {
        PyErr_SetString(PyExc_TypeError, "setMorph expects a string, unsigned 8-bit int, and a float");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->SetMorph(clothingName, layer, value);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getMorph, args)
{
    ST::string clothingName;
    unsigned char layer;
    if (!PyArg_ParseTuple(args, "O&B", PyUnicode_STStringConverter, &clothingName, &layer))
    {
        PyErr_SetString(PyExc_TypeError, "getMorph expects a string, and an unsignd 8-bit int");
        PYTHON_RETURN_ERROR;
    }

    return PyFloat_FromDouble(self->fThis->GetMorph(clothingName, layer));
}

PYTHON_METHOD_DEFINITION(ptAvatar, setSkinBlend, args)
{
    unsigned char layer;
    float value;
    if (!PyArg_ParseTuple(args, "Bf", &layer, &value))
    {
        PyErr_SetString(PyExc_TypeError, "setSkinBlend expects an unsigned 8-bit int and a float");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->SetSkinBlend(layer, value);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getSkinBlend, args)
{
    unsigned char layer;
    if (!PyArg_ParseTuple(args, "B", &layer))
    {
        PyErr_SetString(PyExc_TypeError, "getSkinBlend expects an unsigned 8-bit int");
        PYTHON_RETURN_ERROR;
    }

    return PyFloat_FromDouble(self->fThis->GetSkinBlend(layer));
}

PYTHON_BASIC_METHOD_DEFINITION(ptAvatar, saveClothing, SaveClothing)

PYTHON_METHOD_DEFINITION(ptAvatar, getUniqueMeshList, args)
{
    long clothingType;
    if (!PyArg_ParseTuple(args, "l", &clothingType))
    {
        PyErr_SetString(PyExc_TypeError, "getUniqueMeshList expects a long");
        PYTHON_RETURN_ERROR;
    }

    std::vector<PyObject*> clothingList = self->fThis->GetUniqueMeshList(clothingType);
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getAllWithSameMesh, args)
{
    ST::string clothingName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &clothingName))
    {
        PyErr_SetString(PyExc_TypeError, "getAllWithSameMesh expects a string");
        PYTHON_RETURN_ERROR;
    }

    std::vector<PyObject*> clothingList = self->fThis->GetAllWithSameMesh(clothingName);
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
    return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getWardrobeClothingList)
{
    std::vector<PyObject*> clothingList = self->fThis->GetWardrobeClothingList();
    PyObject* retVal = PyList_New(clothingList.size());
    for (int i = 0; i < clothingList.size(); i++)
        PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, addWardrobeClothingItem, args)
{
    ST::string clothingName;
    PyObject* tint1Obj = nullptr;
    PyObject* tint2Obj = nullptr;
    if (!PyArg_ParseTuple(args, "O&OO", PyUnicode_STStringConverter, &clothingName, &tint1Obj, &tint2Obj))
    {
        PyErr_SetString(PyExc_TypeError, "addWardrobeClothingItem expects a string and two ptColor objects");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyColor::Check(tint1Obj)) || (!pyColor::Check(tint2Obj)))
    {
        PyErr_SetString(PyExc_TypeError, "addWardrobeClothingItem expects a string and two ptColor objects");
        PYTHON_RETURN_ERROR;
    }

    pyColor* tint1 = pyColor::ConvertFrom(tint1Obj);
    pyColor* tint2 = pyColor::ConvertFrom(tint2Obj);
    self->fThis->AddWardrobeClothingItem(clothingName, *tint1, *tint2);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, setReplyKey, args)
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "setReplyKey expects a ptKey object");
        PYTHON_RETURN_NONE;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "setReplyKey expects a ptKey object");
        PYTHON_RETURN_NONE;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->SetSenderKey(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getCurrentMode)
{
    return PyLong_FromLong(self->fThis->GetCurrentMode());
}

PYTHON_METHOD_DEFINITION(ptAvatar, registerForBehaviorNotify, args)
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerForBehaviorNotify expects a ptKey object");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "registerForBehaviorNotify expects a ptKey object");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->RegisterForBehaviorNotify(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, unRegisterForBehaviorNotify, args)
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "unRegisterForBehaviorNotify expects a ptKey object");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "unRegisterForBehaviorNotify expects a ptKey object");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->UnRegisterForBehaviorNotify(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, playSimpleAnimation, args)
{
    ST::string animName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &animName))
    {
        PyErr_SetString(PyExc_TypeError, "playSimpleAnimation expects a string object");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->PlaySimpleAnimation(animName);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, saveClothingToFile, args)
{
    plFileName filename;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_PlFileNameDecoder, &filename))
    {
        PyErr_SetString(PyExc_TypeError, "saveClothingToFile expects a string object");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(self->fThis->SaveClothingToFile(filename));
}

PYTHON_METHOD_DEFINITION(ptAvatar, loadClothingFromFile, args)
{
    plFileName filename;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_PlFileNameDecoder, &filename))
    {
        PyErr_SetString(PyExc_TypeError, "loadClothingFromFile expects a string object");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(self->fThis->LoadClothingFromFile(filename));
}

PYTHON_METHOD_DEFINITION(ptAvatar, setDontPanicLink, args)
{
    bool value;
    if (!PyArg_ParseTuple(args, "b", &value)) {
        PyErr_SetString(PyExc_TypeError, "setDontPanicLink expects a boolean");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->SetDontPanicLink(value);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, findBone, args)
{
    ST::string boneName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &boneName))
    {
        PyErr_SetString(PyExc_TypeError, "findBone expects a string bone name");
        PYTHON_RETURN_ERROR;
    }

    return self->fThis->FindBone(boneName);
}

PYTHON_START_METHODS_TABLE(ptAvatar)
    PYTHON_METHOD(ptAvatar, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
                "- This is to be used if your Python program is running on only one client\n"
                "Such as a game master, only running on the client that owns a particular object"),
    
    PYTHON_METHOD(ptAvatar, oneShot, "Params: seekKey,duration,usePhysicsFlag,animationName,drivableFlag,reversibleFlag\nPlays a one-shot animation on the avatar"),
    PYTHON_METHOD(ptAvatar, runBehavior, "Params: behaviorKey,netForceFlag\nRuns a behavior on the avatar. Can be a single or multi-stage behavior."),
    PYTHON_METHOD(ptAvatar, runBehaviorSetNotify, "Params: behaviorKey,replyKey,netForceFlag\nSame as runBehavior, except send notifications to specified keyed object"),
    PYTHON_METHOD(ptAvatar, runCoopAnim, "Params: targetKey,activeAvatarAnim,targetAvatarAnim,range=6,dist=3,move=1\nSeek near another avatar and run animations on both."),
    PYTHON_METHOD(ptAvatar, nextStage, "Params: behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to the next stage (Why does Matt like so many parameters?)"),
    PYTHON_METHOD(ptAvatar, previousStage, "Params: behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to the previous stage"),
    PYTHON_METHOD(ptAvatar, gotoStage, "Params: behaviorKey,stage,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to a particular stage"),
    
    PYTHON_METHOD_NOARGS(ptAvatar, getAvatarClothingGroup, "Returns what clothing group the avatar belongs to.\n"
                "It is also a means to determine if avatar is male or female"),
    PYTHON_METHOD(ptAvatar, getEntireClothingList, "Params: clothing_type\nGets the entire list of clothing available. 'clothing_type' not used\n"
                "NOTE: should use getClosetClothingList"),
    PYTHON_METHOD(ptAvatar, getClosetClothingList, "Params: clothing_type\nReturns a list of clothes for the avatar that are in specified clothing group."),
    PYTHON_METHOD_NOARGS(ptAvatar, getAvatarClothingList, "Returns a list of clothes that the avatar is currently wearing."),
    PYTHON_METHOD(ptAvatar, getMatchingClothingItem, "Params: clothingName\nFinds the matching clothing item that goes with 'clothingName'\n"
                "Used to find matching left and right gloves and shoes."),
    PYTHON_METHOD(ptAvatar, wearClothingItem, "Params: clothing_name,update=1\nTells the avatar to wear a particular item of clothing.\n"
                "And optionally hold update until later (for applying tinting before wearing)."),
    PYTHON_METHOD(ptAvatar, removeClothingItem, "Params: clothing_name,update=1\nTells the avatar to remove a particular item of clothing."),
    PYTHON_METHOD(ptAvatar, tintClothingItem, "Params: clothing_name,tint,update=1\nTells the avatar to tint(color) a particular item of clothing that they are already wearing.\n"
                "'tint' is a ptColor object"),
    PYTHON_METHOD(ptAvatar, tintClothingItemLayer, "Params: clothing_name,tint,layer,update=1\nTells the avatar to tint(color) a particular layer of a particular item of clothing."),
    PYTHON_METHOD(ptAvatar, getTintClothingItem, "Params: clothing_name,layer=1\nReturns a ptColor of a particular item of clothing that the avatar is wearing.\n"
                "The color will be a ptColor object."),
    PYTHON_METHOD(ptAvatar, tintSkin, "Params: tint,update=1\nTints all of the skin on the avatar, with the ptColor tint"),
    PYTHON_METHOD_NOARGS(ptAvatar, getTintSkin, "Returns a ptColor of the current skin tint for the avatar"),

    PYTHON_METHOD(ptAvatar, enterSubWorld, "Params: sceneobject\nPlaces the avatar into the subworld of the ptSceneObject specified"),
    PYTHON_BASIC_METHOD(ptAvatar, exitSubWorld, "Exits the avatar from the subWorld where it was"),

    PYTHON_METHOD(ptAvatar, setMorph, "Params: clothing_name,layer,value\nSet the morph value (clipped between -1 and 1)"),
    PYTHON_METHOD(ptAvatar, getMorph, "Params: clothing_name,layer\nGet the current morph value"),
    PYTHON_METHOD(ptAvatar, setSkinBlend, "Params: layer,value\nSet the skin blend (value between 0 and 1)"),
    PYTHON_METHOD(ptAvatar, getSkinBlend, "Params: layer\nGet the current skin blend value"),

    PYTHON_BASIC_METHOD(ptAvatar, saveClothing, "Saves the current clothing options (including morphs) to the vault"),

    PYTHON_METHOD(ptAvatar, getUniqueMeshList, "Params: clothing_type\nReturns a list of unique clothing items of the desired type (different meshes)"),
    PYTHON_METHOD(ptAvatar, getAllWithSameMesh, "Params: clothing_name\nReturns a lilst of all clothing items that use the same mesh as the specified one"),
    PYTHON_METHOD_NOARGS(ptAvatar, getWardrobeClothingList, "Return a list of items that are in the avatars closet"),
    PYTHON_METHOD(ptAvatar, addWardrobeClothingItem, "Params: clothing_name,tint1,tint2\nTo add a clothing item to the avatar's wardrobe (closet)"),

    PYTHON_METHOD(ptAvatar, setReplyKey, "Params: key\nSets the sender's key"),

    PYTHON_METHOD_NOARGS(ptAvatar, getCurrentMode, "Returns current brain mode for avatar"),
    
    PYTHON_METHOD(ptAvatar, registerForBehaviorNotify, "Params: selfKey\nThis will register for behavior notifies from the avatar"),
    PYTHON_METHOD(ptAvatar, unRegisterForBehaviorNotify, "Params: selfKey\nThis will unregister behavior notifications"),

    PYTHON_METHOD(ptAvatar, playSimpleAnimation, "Params: animName\nPlay simple animation on avatar"),

    PYTHON_METHOD(ptAvatar, saveClothingToFile, "Params: filename\nSave avatar clothing to a file"),
    PYTHON_METHOD(ptAvatar, loadClothingFromFile, "Params: filename\nLoad avatar clothing from a file"),

    PYTHON_METHOD(ptAvatar, setDontPanicLink, "Type: (value: bool) -> None\nDisables panic linking to Personal Age (warps the avatar back to the start instead)"),
    PYTHON_METHOD(ptAvatar, findBone, "Type: (bone_name: str) -> ptSceneObject\nFind the ptSceneObject for the requested bone"),
PYTHON_END_METHODS_TABLE;

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetBehaviorLoopCount, args, "Params: behaviorKey,stage,loopCount,netForce\nThis will set the loop count for a particular stage in a multistage behavior")
{
    PyObject* keyObj = nullptr;
    long stage, loopCount;
    char netForce;
    if (!PyArg_ParseTuple(args, "Ollb", &keyObj, &stage, &loopCount, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetBehaviorLoopCount expects a ptKey, two longs, and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetBehaviorLoopCount expects a ptKey, two longs, and a boolean");
        PYTHON_RETURN_ERROR;
    }

    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyAvatar::SetLoopCount(*key, stage, loopCount, netForce != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangeAvatar, args, "Params: gender\nChange the local avatar's gender (or clothing type)")
{
    ST::string gender;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &gender))
    {
        PyErr_SetString(PyExc_TypeError, "PtChangeAvatar expects a string");
        PYTHON_RETURN_ERROR;
    }

    cyAvatar::ChangeAvatar(gender);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangePlayerName, args, "Params: name\nChange the local avatar's name")
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "PtChangePlayerName expects a string");
        PYTHON_RETURN_ERROR;
    }

    cyAvatar::ChangePlayerName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEmoteAvatar, args, "Params: emote\nPlay an emote on the local avatar (netpropagated)")
{
    ST::string emote;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &emote))
    {
        PyErr_SetString(PyExc_TypeError, "PtEmoteAvatar expects a string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(cyAvatar::Emote(emote));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarSitOnGround, "Tells the local avatar to sit on ground and enter sit idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::Sit());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterLookingAtKI, "Tells the local avatar to enter looking at KI idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::EnterKiMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitLookingAtKI, "Tells the local avatar to exit looking at KI idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::ExitKiMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterUsePersBook, "Tells the local avatar to enter using their personal book idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::EnterPBMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitUsePersBook, "Tells the local avatar to exit using their personal book idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::ExitPBMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterAFK, "Tells the local avatar to enter AwayFromKeyboard idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::EnterAFKMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitAFK, "Tells the local avatar to exit AwayFromKeyboard idle loop (netpropagated)")
{
    PYTHON_RETURN_BOOL(cyAvatar::ExitAFKMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAvatarEnterAnimMode, args, "Params: animName\nEnter a custom anim loop (netpropagated)")
{
    ST::string animName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &animName))
    {
        PyErr_SetString(PyExc_TypeError, "PtAvatarEnterAnimMode expects a string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(cyAvatar::EnterAnimMode(animName));
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableMovementKeys, cyAvatar::DisableMovementControls, "Disable avatar movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableMovementKeys, cyAvatar::EnableMovementControls, "Enable avatar movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableMouseMovement, cyAvatar::DisableMouseMovement, "Disable avatar mouse movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableMouseMovement, cyAvatar::EnableMouseMovement, "Enable avatar mouse movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableAvatarJump, cyAvatar::DisableAvatarJump, "Disable the ability of the avatar to jump")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableAvatarJump, cyAvatar::EnableAvatarJump, "Enable the ability of the avatar to jump")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableForwardMovement, cyAvatar::DisableForwardMovement, "Disable the ability of the avatar to move forward")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableForwardMovement, cyAvatar::EnableForwardMovement, "Enable the ability of the avatar to move forward")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtLocalAvatarRunKeyDown, "Returns true if the run key is being held down for the local avatar")
{
    PYTHON_RETURN_BOOL(cyAvatar::LocalAvatarRunKeyDown());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtLocalAvatarIsMoving, "Returns true if the local avatar is moving (a movement key is held down)")
{
    PYTHON_RETURN_BOOL(cyAvatar::LocalAvatarIsMoving());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetMouseTurnSensitivity, args, "Params: sensitivity\nSet the mouse sensitivity")
{
    float sensitivity;
    if (!PyArg_ParseTuple(args, "f", &sensitivity))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetMouseTurnSensitivity expects a floating point value");
        PYTHON_RETURN_ERROR;
    }

    cyAvatar::SetMouseTurnSensitivity(sensitivity);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetMouseTurnSensitivity, "Returns the sensitivity")
{
    return PyFloat_FromDouble(cyAvatar::GetMouseTurnSensitivity());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsCurrentBrainHuman, "Returns whether the local avatar current brain is the human brain")
{
    PYTHON_RETURN_BOOL(cyAvatar::IsCurrentBrainHuman());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtAvatarSpawnNext, cyAvatar::SpawnNext, "Send the avatar to the next spawn point")

// Type structure definition
PLASMA_DEFAULT_TYPE(ptAvatar, "Plasma avatar class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAvatar, cyAvatar)

PYTHON_CLASS_CHECK_IMPL(ptAvatar, cyAvatar)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAvatar, cyAvatar)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyAvatar::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptAvatar);
    PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//
void cyAvatar::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(ptAvatar)
        // static/global functions (to the local avatar)
        PYTHON_GLOBAL_METHOD(PtSetBehaviorLoopCount)
        PYTHON_GLOBAL_METHOD(PtChangeAvatar)
        PYTHON_GLOBAL_METHOD(PtChangePlayerName)
        PYTHON_GLOBAL_METHOD(PtEmoteAvatar)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarSitOnGround)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarEnterLookingAtKI)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarExitLookingAtKI)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarEnterUsePersBook)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarExitUsePersBook)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarEnterAFK)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAvatarExitAFK)
        PYTHON_GLOBAL_METHOD(PtAvatarEnterAnimMode)

        // Suspend avatar input
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableMovementKeys)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableMovementKeys)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableMouseMovement)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableMouseMovement)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableAvatarJump)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableAvatarJump)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableForwardMovement)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableForwardMovement)
        PYTHON_GLOBAL_METHOD_NOARGS(PtLocalAvatarRunKeyDown)
        PYTHON_GLOBAL_METHOD_NOARGS(PtLocalAvatarIsMoving)
        PYTHON_GLOBAL_METHOD(PtSetMouseTurnSensitivity)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetMouseTurnSensitivity)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsCurrentBrainHuman)
        PYTHON_BASIC_GLOBAL_METHOD(PtAvatarSpawnNext)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, ptAvatar)
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaConstantsClasses - the python constants definitions
//
void cyAvatar::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtBrainModes)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kGeneric,     plAvBrainGeneric::kGeneric)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kLadder,      plAvBrainGeneric::kLadder)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kSit,         plAvBrainGeneric::kSit)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kSitOnGround, plAvBrainGeneric::kSitOnGround)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kEmote,       plAvBrainGeneric::kEmote)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kAFK,         plAvBrainGeneric::kAFK)
    PYTHON_ENUM_ELEMENT(PtBrainModes, kNonGeneric,  plAvBrainGeneric::kNonGeneric)
    PYTHON_ENUM_END(m, PtBrainModes)

    PYTHON_ENUM_START(m, PtBehaviorTypes)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeStandingJump,     plHBehavior::kBehaviorTypeStandingJump)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalkingJump,      plHBehavior::kBehaviorTypeWalkingJump)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRunningJump,      plHBehavior::kBehaviorTypeRunningJump)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeAnyJump,          plHBehavior::kBehaviorTypeAnyJump)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRunningImpact,    plHBehavior::kBehaviorTypeRunningImpact)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeGroundImpact,     plHBehavior::kBehaviorTypeGroundImpact)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeAnyImpact,        plHBehavior::kBehaviorTypeAnyImpact)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeIdle,             plHBehavior::kBehaviorTypeIdle)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalk,             plHBehavior::kBehaviorTypeWalk)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRun,              plHBehavior::kBehaviorTypeRun)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalkBack,         plHBehavior::kBehaviorTypeWalkBack)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeTurnLeft,         plHBehavior::kBehaviorTypeTurnLeft)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeTurnRight,        plHBehavior::kBehaviorTypeTurnRight)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeSidestepLeft,     plHBehavior::kBehaviorTypeSidestepLeft)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeSidestepRight,    plHBehavior::kBehaviorTypeSidestepRight)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeFall,             plHBehavior::kBehaviorTypeFall)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeMovingTurnLeft,   plHBehavior::kBehaviorTypeMovingTurnLeft)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeMovingTurnRight,  plHBehavior::kBehaviorTypeMovingTurnRight)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeLinkIn,           plHBehavior::kBehaviorTypeLinkIn)
    PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeLinkOut,          plHBehavior::kBehaviorTypeLinkOut)
    PYTHON_ENUM_END(m, PtBehaviorTypes)
}
