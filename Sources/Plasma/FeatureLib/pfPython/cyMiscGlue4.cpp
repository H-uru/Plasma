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

#include "cyMisc.h"

#include <string_theory/string_stream>
#include <vector>

#include "plPipeline.h"

#include "pnNetBase/pnNetBase.h"

#include "pyAgeInfoStruct.h"
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pySceneObject.h"

PYTHON_GLOBAL_METHOD_DEFINITION(PtRequestLOSScreen, args, "Params: selfKey,ID,xPos,yPos,distance,what,reportType\nRequest a LOS check from a point on the screen")
{
    PyObject* keyObj = nullptr;
    long id;
    float xPos, yPos, distance;
    int what, reportType;
    if (!PyArg_ParseTuple(args, "Olfffii", &keyObj, &id, &xPos, &yPos, &distance, &what, &reportType))
    {
        PyErr_SetString(PyExc_TypeError, "PtRequestLOSScreen expects a ptKey, a long, three floats, and two ints");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtRequestLOSScreen expects a ptKey, a long, three floats, and two ints");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    PYTHON_RETURN_BOOL(cyMisc::RequestLOSScreen(*key, id, xPos, yPos, distance, what, reportType));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtKillParticles, args, "Params: timeRemaining,pctToKill,particleSystem\nTells particleSystem to kill pctToKill percent of its particles")
{
    float timeRemaining, pctToKill;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "ffO", &timeRemaining, &pctToKill, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtKillParticles expects two floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtKillParticles expects two floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::KillParticles(timeRemaining, pctToKill, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetNumParticles, args, "Params: key\nKey is the key of scene object host to particle system")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetNumParticles expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetNumParticles expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    return PyLong_FromLong(cyMisc::GetNumParticles(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetParticleOffset, args, "Params: x,y,z,particlesys\nSets the particlesys particle system's offset")
{
    float x,y,z;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "fffO", &x, &y, &z, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetParticleOffset expects three floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetParticleOffset expects three floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetParticleOffset(x, y, z, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetLightValue, args, "Params: key,name,r,g,b,a\n Key is the key of scene object host to light. Name is the name of the light to manipulate")
{
    PyObject* keyObj = nullptr;
    ST::string name;
    float r,g,b,a;
    if (!PyArg_ParseTuple(args, "OO&ffff", &keyObj, PyUnicode_STStringConverter, &name, &r, &g, &b, &a))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetLightValue expects a ptKey, a string, and four floats");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetLightValue expects a ptKey, a string, and four floats");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetLightColorValue(*key, name, r, g, b, a);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetLightAnimStart, args, "Params: key,name,start\n Key is the key of scene object host to light, start is a bool. Name is the name of the light to manipulate")
{
    PyObject* keyObj = nullptr;
    ST::string name;
    char start;
    if (!PyArg_ParseTuple(args, "OO&b", &keyObj, PyUnicode_STStringConverter, &name, &start))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetLightAnimStart expects a ptKey, a string, and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetLightAnimStart expects a ptKey, a string, and a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetLightAnimationOn(*key, name, start != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsSinglePlayerMode, "Returns whether the game is in single player mode or not")
{
    PYTHON_RETURN_BOOL(cyMisc::IsSinglePlayerMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsDemoMode, "Returns whether the game is in Demo mode or not")
{
    PYTHON_RETURN_BOOL(cyMisc::IsDemoMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsInternalRelease, "Returns whether the client is an internal build or not")
{
    PYTHON_RETURN_BOOL(cyMisc::IsInternalRelease());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsEnterChatModeKeyBound, "Returns whether the EnterChatMode is bound to a key")
{
    PYTHON_RETURN_BOOL(cyMisc::IsEnterChatModeKeyBound());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShootBulletFromScreen, args, "Params: selfkey, xPos, yPos, radius, range\nShoots a bullet from a position on the screen")
{
    PyObject* keyObj = nullptr;
    float xPos, yPos, radius, range;
    if (!PyArg_ParseTuple(args, "Offff", &keyObj, &xPos, &yPos, &radius, &range))
    {
        PyErr_SetString(PyExc_TypeError, "PtShootBulletFromScreen expects a ptKey and four floats");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtShootBulletFromScreen expects a ptKey and four floats");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::ShootBulletFromScreen(*key, xPos, yPos, radius, range);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShootBulletFromObject, args, "Params: selfkey, gunObj, radius, range\nShoots a bullet from an object")
{
    PyObject* selfKeyObj = nullptr;
    PyObject* gunSceneObj = nullptr;
    float radius, range;
    if (!PyArg_ParseTuple(args, "OOff", &selfKeyObj, &gunSceneObj, &radius, &range))
    {
        PyErr_SetString(PyExc_TypeError, "PtShootBulletFromObject expects a ptKey, a ptSceneobject, and two floats");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(selfKeyObj)) || (!pySceneObject::Check(gunSceneObj)))
    {
        PyErr_SetString(PyExc_TypeError, "PtShootBulletFromObject expects a ptKey, a ptSceneobject, and two floats");
        PYTHON_RETURN_ERROR;
    }
    pyKey* selfKey = pyKey::ConvertFrom(selfKeyObj);
    pySceneObject* gunObj = pySceneObject::ConvertFrom(gunSceneObj);
    cyMisc::ShootBulletFromObject(*selfKey, gunObj, radius, range);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetPublicAgeList, args, "Params: ageName\nGet list of public ages for the given age name.\n"
            "The age list will be delivered asynchronously through the callback method gotPublicAgeList(self,ageList). ageList is a list of tuple(ptAgeInfoStruct,nPlayersInAge)")
{
    ST::string ageName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &ageName)) {
        PyErr_SetString(PyExc_TypeError, "PtGetPublicAgeList expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::GetPublicAgeList(ageName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreatePublicAge, args, "Params: ageInfo\nCreate a public instance of the given age.")
{
    PyObject* ageInfoObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtCreatePublicAge expects a ptAgeInfoStruct object");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeInfoStruct::Check(ageInfoObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtCreatePublicAge expects a ptAgeInfoStruct object");
        PYTHON_RETURN_ERROR;
    }
    pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
    cyMisc::CreatePublicAge(ageInfo);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRemovePublicAge, args, "Params: ageInstanceGuid\nRemove a public instance of the given age.")
{
    ST::string ageInstanceGUID;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &ageInstanceGUID))
    {
        PyErr_SetString(PyExc_TypeError, "PtRemovePublicAge expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::RemovePublicAge(ageInstanceGUID);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetClearColor, args, "Params: red,green,blue\nSet the clear color")
{
    float red, green, blue;
    if (!PyArg_ParseTuple(args, "fff", &red, &green, &blue))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetClearColor expects three floats");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SetClearColor(red, green, blue);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalKILevel, "returns local player's ki level")
{
    return PyLong_FromLong(cyMisc::GetKILevel());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtClearCameraStack, cyMisc::ClearCameraStack, "clears all cameras")

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetCameraNumber, args, "Params: x\nReturns camera x's name from stack")
{
    int x;
    if (!PyArg_ParseTuple(args, "i", &x))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetCameraNumber expects an int");
        PYTHON_RETURN_ERROR;
    }
    return PyUnicode_FromSTString(cyMisc::GetCameraNumber(x));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetNumCameras, "returns camera stack size")
{
    return PyLong_FromLong(cyMisc::GetNumCameras());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRebuildCameraStack, args, "Params: name,ageName\nPush camera with this name on the stack")
{
    ST::string name;
    ST::string ageName;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_STStringConverter, &name, PyUnicode_STStringConverter, &ageName))
    {
        PyErr_SetString(PyExc_TypeError, "PtRebuildCameraStack expects two strings");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::RebuildCameraStack(name, ageName);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtRecenterCamera, cyMisc::RecenterCamera, "re-centers the camera")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtFirstPerson, "is the local avatar in first person mode")
{
    PYTHON_RETURN_BOOL(cyMisc::IsFirstPerson());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeIn, args, "Params: lenTime, holdFlag, noSound=0\nFades screen in for lenTime seconds")
{
    float lenTime;
    char holdFlag, noSound = 0;
    if (!PyArg_ParseTuple(args, "fb|b", &lenTime, &holdFlag, &noSound))
    {
        PyErr_SetString(PyExc_TypeError, "PtFadeIn expects a float, a boolean, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FadeIn(lenTime, holdFlag != 0, noSound != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeOut, args, "Params: lenTime, holdFlag, noSound=0\nFades screen out for lenTime seconds")
{
    float lenTime;
    char holdFlag, noSound = 0;
    if (!PyArg_ParseTuple(args, "fb|b", &lenTime, &holdFlag, &noSound))
    {
        PyErr_SetString(PyExc_TypeError, "PtFadeOut expects a float, a boolean, and an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FadeOut(lenTime, holdFlag != 0, noSound != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetGlobalClickability, args, "Params: enable\nEnable or disable all clickables on the local client")
{
    char enable;
    if (!PyArg_ParseTuple(args, "b", &enable))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetGlobalClickability expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SetClickability(enable != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDebugAssert, args, "Params: cond, msg\nDebug only: Assert if condition is false.")
{
    char cond;
    ST::string msg;
    if (!PyArg_ParseTuple(args, "bO&", &cond, PyUnicode_STStringConverter, &msg))
    {
        PyErr_SetString(PyExc_TypeError, "PtDebugAssert expects a boolean and a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::DebugAssert(cond != 0, msg);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtDebugPrint, args, kwargs, "Params: *msgs, level=3, sep=\" \", end=\"\\n\"\n"
                                     "Prints msgs to the Python log given the message's level, "
                                     "optionally separated and terminated by the given strings")
{
    uint32_t  level = cyMisc::kErrorLevel;
    ST::string sep = ST_LITERAL(" ");
    ST::string end = ST_LITERAL("\n");

    do {
        if (kwargs && PyDict_Check(kwargs)) {
            PyObject* value = PyDict_GetItemString(kwargs, "level");
            if (value) {
                if (PyLong_Check(value))
                    level = PyLong_AsLong(value);
                else
                    break;
            }

            value = PyDict_GetItemString(kwargs, "sep");
            if (value) {
                if (PyUnicode_Check(value))
                    sep = PyUnicode_AsSTString(value);
                else
                    break;
            }

            value = PyDict_GetItemString(kwargs, "end");
            if (value) {
                if (PyUnicode_Check(value))
                    end = PyUnicode_AsSTString(value);
                else
                    break;
            }
        }

        ST::string_stream ss;
        for (size_t i = 0; i < PySequence_Fast_GET_SIZE(args); ++i) {
            PyObject* theMsg = PySequence_Fast_GET_ITEM(args, i);
            if (PyUnicode_Check(theMsg))
                Py_XINCREF(theMsg);
            else
                theMsg = PyObject_Str(theMsg);

            if (i != 0)
                ss << sep;
            if (theMsg) {
                ss << PyUnicode_AsSTString(theMsg);
                Py_DECREF(theMsg);
            } else {
                PyErr_Format(PyExc_RuntimeError, "Failed to `str()` argument index %n", i);
                PYTHON_RETURN_ERROR;
            }
        }
        ss << end;
        cyMisc::DebugPrint(ss.to_string(), level);
        PYTHON_RETURN_NONE;
    } while (false);

    // fell through to the type error case
    PyErr_SetString(PyExc_TypeError, "PtDebugPrint expects a sequence of objects, "
                                     "an integer explicitly keyed `level`, "
                                     "an object explicitly keyed `sep`, "
                                     "and an object explicitly keyed `end`");
    PYTHON_RETURN_ERROR;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetAlarm, args, "Params: secs, cbObject, cbContext\nsecs is the amount of time before your alarm goes off.\n"
            "cbObject is a python object with the method onAlarm(int context)\ncbContext is an integer.")
{
    float secs;
    PyObject* cbObject = nullptr;
    unsigned long cbContext;
    if (!PyArg_ParseTuple(args, "fOl", &secs, &cbObject, &cbContext))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetAlarm expects a float, a object with a onAlarm() method, and an int");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SetAlarm(secs, cbObject, cbContext);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSaveScreenShot, args, "Params: fileName,width=640,height=480,quality=75\nTakes a screenshot with the specified filename, size, and quality")
{
    plFileName fileName;
    int width = 640, height = 480, quality = 75;
    if (!PyArg_ParseTuple(args, "O&|iii", PyUnicode_PlFileNameDecoder, &fileName, &width, &height, &quality))
    {
        PyErr_SetString(PyExc_TypeError, "PtSaveScreenShot expects a string, and three optional integers");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SaveScreenShot(fileName, width, height, quality);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtStartScreenCapture, args, "Params: selfKey,width=800,height=600\nStarts a capture of the screen")
{
    PyObject* keyObj = nullptr;
    unsigned short width = 800, height = 600;
    if (!PyArg_ParseTuple(args, "O|hh", &keyObj, &width, &height))
    {
        PyErr_SetString(PyExc_TypeError, "PtStartScreenCapture expects a ptKey, and two optional unsigned 16-bit ints");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtStartScreenCapture expects a ptKey, and two optional unsigned 16-bit ints");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::StartScreenCaptureWH(*key, width, height);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIGZMarkerMsg, args, "Params: markerNumber,sender\nSame as PtSendKIMessageInt except 'sender' could get a notify message back\n")
{
    long markerNumber;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "lO", &markerNumber, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIGZMarkerMsg expects a long and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIGZMarkerMsg expects a long and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SendKIGZMarkerMsg(markerNumber, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIRegisterImagerMsg, args, "Params: imagerName, sender\nSends a message to the KI to register the specified imager")
{
    ST::string name;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O&O", PyUnicode_STStringConverter, &name, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIRegisterImagerMsg expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIRegisterImagerMsg expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SendKIRegisterImagerMsg(name, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWearMaintainerSuit, args, "Params: key,wearOrNot\nWears or removes the maintainer suit of clothes")
{
    PyObject* keyObj = nullptr;
    char wearOrNot;
    if (!PyArg_ParseTuple(args, "Ob", &keyObj, &wearOrNot))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearMaintainerSuit expects a ptKey and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearMaintainerSuit expects a ptKey and a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::WearMaintainerSuit(*key, wearOrNot != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtWearDefaultClothing, args, kw, "Params: key, broadcast=False\nForces the avatar to wear the default clothing set")
{
    const char* kwlist[] = { "key", "broadcast", nullptr };
    PyObject* keyObj = nullptr;
    bool broadcast = false;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "O|b", const_cast<char**>(kwlist), &keyObj, &broadcast))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothing expects a ptKey and an optional bool");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothing expects a ptKey and an optional bool");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::WearDefaultClothing(*key, broadcast);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeTimeOfDayPercent, "Returns the current age time of day as a percent (0 to 1)")
{
    return PyFloat_FromDouble(cyMisc::GetAgeTimeOfDayPercent());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCheckVisLOS, args, "Params: startPoint,endPoint\nDoes LOS check from start to end")
{
    PyObject* startPointObj = nullptr;
    PyObject* endPointObj = nullptr;
    if (!PyArg_ParseTuple(args, "OO", &startPointObj, &endPointObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtCheckVisLOS expects two ptPoint3 objects");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyPoint3::Check(startPointObj)) || (!pyPoint3::Check(endPointObj)))
    {
        PyErr_SetString(PyExc_TypeError, "PtCheckVisLOS expects two ptPoint3 objects");
        PYTHON_RETURN_ERROR;
    }
    pyPoint3* startPoint = pyPoint3::ConvertFrom(startPointObj);
    pyPoint3* endPoint = pyPoint3::ConvertFrom(endPointObj);
    return cyMisc::CheckVisLOS(*startPoint, *endPoint);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtCheckVisLOSFromCursor, "Does LOS check from where the mouse cursor is, into the screen")
{
    return cyMisc::CheckVisLOSFromCursor();
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEnablePlanarReflections, args, "Params: on\nEnables/disables planar reflections")
{
    char on;
    if (!PyArg_ParseTuple(args, "b", &on))
    {
        PyErr_SetString(PyExc_TypeError, "PtEnablePlanarReflections expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::EnablePlanarReflections(on != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtSupportsPlanarReflections, "Type: () -> bool\nReturns if planar reflections are supported")
{
    return PyBool_FromLong(cyMisc::ArePlanarReflectionsSupported() ? 1 : 0);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetSupportedDisplayModes, "Returns a list of supported resolutions")
{
    std::vector<plDisplayMode> res;
    cyMisc::GetSupportedDisplayModes(&res);
    PyObject *retVal = PyList_New(0);
    for (std::vector<plDisplayMode>::iterator curArg = res.begin(); curArg != res.end(); ++curArg)
    {
        PyObject* tup = PyTuple_New(2);
        PyTuple_SetItem(tup, 0, PyLong_FromLong((long)(*curArg).Width));
        PyTuple_SetItem(tup, 1, PyLong_FromLong((long)(*curArg).Height));

        PyList_Append(retVal, tup);
    }
    return retVal;
}
PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopWidth, "Returns desktop width")
{
    return PyLong_FromLong((long)cyMisc::GetDesktopWidth());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopHeight, "Returns desktop height")
{
    return PyLong_FromLong((long)cyMisc::GetDesktopHeight());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopColorDepth, "Returns desktop ColorDepth")
{
    return PyLong_FromLong((long)cyMisc::GetDesktopColorDepth());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDefaultDisplayParams, "Returns the default resolution and display settings")
{
    PipelineParams *pp = cyMisc::GetDefaultDisplayParams();
    PyObject* tup = PyTuple_New(10);
    PyTuple_SetItem(tup, 0, PyLong_FromLong((long)pp->Width));
    PyTuple_SetItem(tup, 1, PyLong_FromLong((long)pp->Height));
    PyTuple_SetItem(tup, 2, PyLong_FromLong((long)pp->Windowed));
    PyTuple_SetItem(tup, 3, PyLong_FromLong((long)pp->ColorDepth));
    PyTuple_SetItem(tup, 4, PyLong_FromLong((long)pp->AntiAliasingAmount));
    PyTuple_SetItem(tup, 5, PyLong_FromLong((long)pp->AnisotropicLevel));
    PyTuple_SetItem(tup, 6, PyLong_FromLong((long)pp->TextureQuality));
    PyTuple_SetItem(tup, 7, PyLong_FromLong((long)pp->VideoQuality));
    PyTuple_SetItem(tup, 8, PyLong_FromLong((long)pp->Shadows));
    PyTuple_SetItem(tup, 9, PyLong_FromLong((long)pp->PlanarReflections));
    return tup;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetGraphicsOptions, args, "Params: width, height, colordepth, windowed, numAAsamples, numAnisoSamples, VSync\nSet the graphics options")
{
    int width = 800, height = 600, colordepth = 32, windowed = 0, numAAsamples = 0, numAnisoSamples = 0, vsync = 0;
    if (!PyArg_ParseTuple(args, "iiiiiii", &width, &height, &colordepth, &windowed, &numAAsamples, &numAnisoSamples, &vsync))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetGraphicsOptions expects a ints for width, height, colordepth, windowed, numAAsamples, numAnisoSamples");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::SetGraphicsOptions(width, height, colordepth, windowed != 0, numAAsamples, numAnisoSamples, vsync != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetBehaviorNetFlags, args, "Params: behKey, netForce, netProp\nSets net flags on the associated behavior")
{
    PyObject* keyObj = nullptr;
    char netForce;
    char netProp;
    if (!PyArg_ParseTuple(args, "Obb", &keyObj, &netForce, &netProp))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetBehaviorNetFlags expects a ptKey, a boolean and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetBehaviorNetFlags expects a ptKey, a boolean and a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetBehaviorNetFlags(*key, netForce != 0, netProp != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendFriendInvite, args, "Params: emailAddress, toName = \"Friend\"\nSends an email with invite code")
{
    ST::string email;
    ST::string name = ST_LITERAL("Friend");
    if (!PyArg_ParseTuple(args, "O&|O&", PyUnicode_STStringConverter, &email,
                          PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendFriendInvite expects a string and optionally another string");
        PYTHON_RETURN_ERROR;
    }

    if (email.size() >= kMaxEmailAddressLength)
    {
        PyErr_SetString(PyExc_TypeError, "PtSendFriendInvite: Email address too long");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::SendFriendInvite(email, name);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGuidGenerate, "Returns string representation for a new guid")
{
    return cyMisc::PyGuidGenerate();
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetAIAvatarsByModelName, args, "Params: modelName\nReturns a list of tuples representing the matching ai avatars")
{
    ST::string modelName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &modelName))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetAIAvatarsByModelName expects a string");
        PYTHON_RETURN_ERROR;
    }

    return cyMisc::GetAIAvatarsByModelName(modelName);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtForceVaultNodeUpdate, args, "Params: nodeId\nForces a vault node to update")
{
    unsigned nodeId;
    if (!PyArg_ParseTuple(args, "I", &nodeId))
    {
        PyErr_SetString(PyExc_TypeError, "PtForceVaultNodeUpdate expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::ForceVaultNodeUpdate(nodeId);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtVaultDownload, args, "Params: nodeId\nDownloads the vault tree of the given nodeid")
{
    unsigned nodeId;
    if (!PyArg_ParseTuple(args, "I", &nodeId))
    {
        PyErr_SetString(PyExc_TypeError, "PtVaultDownload expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::VaultDownload(nodeId);
    PYTHON_RETURN_NONE;
}


///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods4(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(cyMisc4)
        PYTHON_GLOBAL_METHOD(PtRequestLOSScreen)

        PYTHON_GLOBAL_METHOD(PtKillParticles)
        PYTHON_GLOBAL_METHOD(PtGetNumParticles)
        PYTHON_GLOBAL_METHOD(PtSetParticleOffset)

        PYTHON_GLOBAL_METHOD(PtSetLightValue)
        PYTHON_GLOBAL_METHOD(PtSetLightAnimStart)

        PYTHON_GLOBAL_METHOD_NOARGS(PtIsSinglePlayerMode)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsDemoMode)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsInternalRelease)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsEnterChatModeKeyBound)

        PYTHON_GLOBAL_METHOD(PtShootBulletFromScreen)
        PYTHON_GLOBAL_METHOD(PtShootBulletFromObject)

        PYTHON_GLOBAL_METHOD(PtGetPublicAgeList)
        PYTHON_GLOBAL_METHOD(PtCreatePublicAge)
        PYTHON_GLOBAL_METHOD(PtRemovePublicAge)

        PYTHON_GLOBAL_METHOD(PtSetClearColor)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetLocalKILevel)

        PYTHON_BASIC_GLOBAL_METHOD(PtClearCameraStack)
        PYTHON_GLOBAL_METHOD(PtGetCameraNumber)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetNumCameras)
        PYTHON_GLOBAL_METHOD(PtRebuildCameraStack)
        PYTHON_BASIC_GLOBAL_METHOD(PtRecenterCamera)
        PYTHON_GLOBAL_METHOD_NOARGS(PtFirstPerson)

        PYTHON_GLOBAL_METHOD(PtFadeIn)
        PYTHON_GLOBAL_METHOD(PtFadeOut)

        PYTHON_GLOBAL_METHOD(PtSetGlobalClickability)
        PYTHON_GLOBAL_METHOD(PtDebugAssert)
        PYTHON_GLOBAL_METHOD(PtDebugPrint)
        PYTHON_GLOBAL_METHOD(PtSetAlarm)

        PYTHON_GLOBAL_METHOD(PtSaveScreenShot)
        PYTHON_GLOBAL_METHOD(PtStartScreenCapture)

        PYTHON_GLOBAL_METHOD(PtSendKIGZMarkerMsg)
        PYTHON_GLOBAL_METHOD(PtSendKIRegisterImagerMsg)

        PYTHON_GLOBAL_METHOD(PtWearMaintainerSuit)
        PYTHON_GLOBAL_METHOD(PtWearDefaultClothing)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetAgeTimeOfDayPercent)

        PYTHON_GLOBAL_METHOD(PtCheckVisLOS)
        PYTHON_GLOBAL_METHOD_NOARGS(PtCheckVisLOSFromCursor)

        PYTHON_GLOBAL_METHOD(PtEnablePlanarReflections)
        PYTHON_GLOBAL_METHOD_NOARGS(PtSupportsPlanarReflections)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetSupportedDisplayModes)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDesktopWidth)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDesktopHeight)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDesktopColorDepth)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDefaultDisplayParams)
        PYTHON_GLOBAL_METHOD(PtSetGraphicsOptions)

        PYTHON_GLOBAL_METHOD(PtSetBehaviorNetFlags)
        PYTHON_GLOBAL_METHOD(PtSendFriendInvite)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGuidGenerate)
        PYTHON_GLOBAL_METHOD(PtGetAIAvatarsByModelName)
        PYTHON_GLOBAL_METHOD(PtForceVaultNodeUpdate)
        PYTHON_GLOBAL_METHOD(PtVaultDownload)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, cyMisc4)
}
