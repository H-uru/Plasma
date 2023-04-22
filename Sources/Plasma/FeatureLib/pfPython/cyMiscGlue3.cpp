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
#include <utility>
#include "pyKey.h"

#include "cyMisc.h"
#include "pyGlueHelpers.h"
#include "pySceneObject.h"
#include "pnUtils/pnUtils.h"
#include "pnUUID/pnUUID.h"

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendPetitionToCCR, args, "Params: message,reason=0,title=\"\"\nSends a petition with a message to the CCR group")
{
    ST::string message;
    unsigned char reason = 0;
    ST::string title;
    if (!PyArg_ParseTuple(args, "O&|bO&", PyUnicode_STStringConverter, &message, &reason, PyUnicode_STStringConverter, &title))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendPetitionToCCR expects a string, and an optional unsigned 8-bit int and optional string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SendPetitionToCCR(std::move(message), reason, std::move(title));
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendChatToCCR, args, "Params: message,CCRPlayerID\nSends a chat message to a CCR that has contacted this player")
{
    ST::string message;
    long CCRPlayerID;
    if (!PyArg_ParseTuple(args, "O&l", PyUnicode_STStringConverter, &message, &CCRPlayerID))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendChatToCCR expects a string and a long");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SendChatToCCR(std::move(message), CCRPlayerID);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPythonLoggingLevel, "Returns the current level of python logging")
{
    return PyLong_FromUnsignedLong(cyMisc::GetPythonLoggingLevel());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetPythonLoggingLevel, args, "Params: level\nSets the current level of python logging")
{
    unsigned long level;
    if (!PyArg_ParseTuple(args, "l", &level))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetPythonLoggingLevel expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SetPythonLoggingLevel(level);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtConsole, args, "Params: command\nThis will execute 'command' as if it were typed into the Plasma console.")
{
    ST::string command;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &command))
    {
        PyErr_SetString(PyExc_TypeError, "PtConsole expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::Console(command);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtConsoleNet, args, "Params: command,netForce\nThis will execute 'command' on the console, over the network, on all clients.\n"
            "If 'netForce' is true then force command to be sent over the network.")
{
    ST::string command;
    char netForce;
    if (!PyArg_ParseTuple(args, "O&b", PyUnicode_STStringConverter, &command, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "PtConsoleNet expects a string and a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::ConsoleNet(command, netForce != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAtTimeCallback, args, "Params: selfkey,time,id\nThis will create a timer callback that will call OnTimer when complete\n"
            "- 'selfkey' is the ptKey of the PythonFile component\n"
            "- 'time' is how much time from now (in seconds) to call back\n"
            "- 'id' is an integer id that will be returned in the OnTimer call")
{
    PyObject* keyObj = nullptr;
    float time;
    int id;
    if (!PyArg_ParseTuple(args, "Ofi", &keyObj, &time, &id))
    {
        PyErr_SetString(PyExc_TypeError, "PtAtTimeCallback expects a ptKey, a float, and an int");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtAtTimeCallback expects a ptKey, a float, and an int");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::TimerCallback(*key, time, id);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtClearTimerCallbacks, args, "Params: key\nThis will remove timer callbacks to the specified key")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtClearTimerCallbacks expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtClearTimerCallbacks expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::ClearTimerCallbacks(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindSceneobject, args, "Params: name,ageName\nThis will try to find a sceneobject based on its name and what age its in\n"
            "- it will return a ptSceneObject if found"
            "- if not found then a NameError exception will happen")
{
    ST::string name;
    ST::string ageName;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_STStringConverter, &name, PyUnicode_STStringConverter, &ageName))
    {
        PyErr_SetString(PyExc_TypeError, "PtFindSceneobject expects two strings");
        PYTHON_RETURN_ERROR;
    }
    return cyMisc::FindSceneObject(name, ageName);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindSceneobjects, args, "Params: name\nThis will try to find a any sceneobject containing string in name")
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "PtFindSceneobject expects string");
        PYTHON_RETURN_ERROR;
    }
    return cyMisc::FindSceneObjects(name);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindActivator, args, "Params: name\nThis will try to find an activator based on its name\n"
            "- it will return a ptKey if found"
            "- it will return None if not found")
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name))
    {
        PyErr_SetString(PyExc_TypeError, "PtFindActivator expects a string");
        PYTHON_RETURN_ERROR;
    }

    return cyMisc::FindActivator(name);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWasLocallyNotified, args, "Params: selfKey\nReturns 1 if the last notify was local or 0 if the notify originated on the network")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtWasLocallyNotified expects a ptKey");
        PYTHON_RETURN_NONE;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtWasLocallyNotified expects a ptKey");
        PYTHON_RETURN_NONE;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    PYTHON_RETURN_BOOL(cyMisc::WasLocallyNotified(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAttachObject, args, "Params: child,parent,netForce=false\nAttach child to parent based on ptKey or ptSceneobject\n"
            "- childKey is the ptKey or ptSceneobject of the one being attached\n"
            "- parentKey is the ptKey or ptSceneobject of the one being attached to\n"
            "(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)")
{
    PyObject* childObj = nullptr;
    PyObject* parentObj = nullptr;
    char netForce = 0;
    if (!PyArg_ParseTuple(args, "OO|b", &childObj, &parentObj, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "PtAttachObject expects either two ptKeys or two ptSceneobjects and bool");
        PYTHON_RETURN_ERROR;
    }
    if ((pyKey::Check(childObj)) && (pyKey::Check(parentObj)))
    {
        pyKey* child = pyKey::ConvertFrom(childObj);
        pyKey* parent = pyKey::ConvertFrom(parentObj);
        cyMisc::AttachObject(*child, *parent, netForce);
    }
    else if ((pySceneObject::Check(childObj)) && (pySceneObject::Check(parentObj)))
    {
        pySceneObject* child = pySceneObject::ConvertFrom(childObj);
        pySceneObject* parent = pySceneObject::ConvertFrom(parentObj);
        cyMisc::AttachObjectSO(*child, *parent, netForce);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "PtAttachObject expects either two ptKeys or two ptSceneobjects and bool");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDetachObject, args, "Params: child,parent,netForce=false\nDetach child from parent based on ptKey or ptSceneobject\n"
            "- child is the ptKey or ptSceneobject of the one being detached\n"
            "- parent is the ptKey or ptSceneobject of the one being detached from\n"
            "(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)")
{
    PyObject* childObj = nullptr;
    PyObject* parentObj = nullptr;
    char netForce = 0;
    if (!PyArg_ParseTuple(args, "OO|b", &childObj, &parentObj, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "PtDetachObject expects either two ptKeys or two ptSceneobjects and bool");
        PYTHON_RETURN_ERROR;
    }
    if ((pyKey::Check(childObj)) && (pyKey::Check(parentObj)))
    {
        pyKey* child = pyKey::ConvertFrom(childObj);
        pyKey* parent = pyKey::ConvertFrom(parentObj);
        cyMisc::DetachObject(*child, *parent, netForce);
    }
    else if ((pySceneObject::Check(childObj)) && (pySceneObject::Check(parentObj)))
    {
        pySceneObject* child = pySceneObject::ConvertFrom(childObj);
        pySceneObject* parent = pySceneObject::ConvertFrom(parentObj);
        cyMisc::DetachObjectSO(*child, *parent, netForce);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "PtDetachObject expects either two ptKeys or two ptSceneobjects and bool");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDirtySynchState, args, "Params: selfKey,SDLStateName,flags\nDO NOT USE - handled by ptSDL")
{
    PyObject* keyObj = nullptr;
    ST::string SDLStateName;
    unsigned long flags;
    if (!PyArg_ParseTuple(args, "OO&l", &keyObj, PyUnicode_STStringConverter, &SDLStateName, &flags))
    {
        PyErr_SetString(PyExc_TypeError, "PtDirtySynchState expects a ptKey, a string, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtDirtySynchState expects a ptKey, a string, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetDirtySyncState(*key, SDLStateName, flags);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDirtySynchClients, args, "Params: selfKey,SDLStateName,flags\nDO NOT USE - handled by ptSDL")
{
    PyObject* keyObj = nullptr;
    ST::string SDLStateName;
    unsigned long flags;
    if (!PyArg_ParseTuple(args, "OO&l", &keyObj, PyUnicode_STStringConverter, &SDLStateName, &flags))
    {
        PyErr_SetString(PyExc_TypeError, "PtDirtySynchClients expects a ptKey, a string, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtDirtySynchClients expects a ptKey, a string, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetDirtySyncStateWithClients(*key, SDLStateName, flags);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEnableControlKeyEvents, args, "Params: selfKey\nEnable control key events to call OnControlKeyEvent(controlKey,activateFlag)")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtEnableControlKeyEvents expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtEnableControlKeyEvents expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::EnableControlKeyEvents(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDisableControlKeyEvents, args, "Params: selfKey\nDisable the control key events from calling OnControlKeyEvent")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtDisableControlKeyEvents expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtDisableControlKeyEvents expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::DisableControlKeyEvents(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableAvatarCursorFade, cyMisc::EnableAvatarCursorFade, "Enable the avatar cursor fade")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableAvatarCursorFade, cyMisc::DisableAvatarCursorFade, "Disable the avatar cursor fade")

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeLocalAvatar, args, "Params: fade\nFade (or unfade) the local avatar")
{
    char fade;
    if (!PyArg_ParseTuple(args, "b", &fade))
    {
        PyErr_SetString(PyExc_TypeError, "PtFadeLocalAvatar expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FadeLocalPlayer(fade != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetOfferBookMode, args, "Params: selfkey,ageFilename,ageInstanceName\nPut us into the offer book interface")
{
    PyObject* keyObj = nullptr;
    ST::string ageFilename;
    ST::string ageInstanceName;
    if (!PyArg_ParseTuple(args, "OO&O&", &keyObj, PyUnicode_STStringConverter, &ageFilename, PyUnicode_STStringConverter, &ageInstanceName))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetOfferBookMode expects a ptKey, and two strings");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetOfferBookMode expects a ptKey, and two strings");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::EnableOfferBookMode(*key, ageFilename, ageInstanceName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetShareSpawnPoint, args, "Params: spawnPoint\nThis sets the desired spawn point for the receiver to link to")
{
    ST::string spawnPoint;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &spawnPoint))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetShareSpawnPoint expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SetShareSpawnPoint(spawnPoint);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetShareAgeInstanceGuid, args, "Params: instanceGuid\nThis sets the desired age instance guid for the receiver to link to")
{
    ST::string guidStr;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guidStr))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetShareAgeInstanceGuid expects a string");
        PYTHON_RETURN_ERROR;
    }

    plUUID guid(guidStr);
    if (guid == kNilUuid)
    {
        PyErr_SetString(PyExc_TypeError, "PtSetShareAgeInstanceGuid string parameter is not a guid string");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::SetShareAgeInstanceGuid(guid);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkAccepted, args, "Params: offerer\nTell the offerer that we accepted the link offer")
{
    unsigned long offerer;
    if (!PyArg_ParseTuple(args, "l", &offerer))
    {
        PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkAccepted expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::NotifyOffererPublicLinkAccepted(offerer);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkRejected, args, "Params: offerer\nTell the offerer that we rejected the link offer")
{
    unsigned long offerer;
    if (!PyArg_ParseTuple(args, "l", &offerer))
    {
        PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkRejected expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::NotifyOffererPublicLinkRejected(offerer);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkCompleted, args, "Params: offerer\nTell the offerer that we completed the link")
{
    unsigned long offerer;
    if (!PyArg_ParseTuple(args, "l", &offerer))
    {
        PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkCompleted expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::NotifyOffererPublicLinkCompleted(offerer);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtClearOfferBookMode, cyMisc::DisableOfferBookMode, "Cancel the offer book interface")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalClientID, "Returns our local client ID number")
{
    return PyLong_FromLong(cyMisc::GetLocalClientID());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsCCRAway, "Returns current status of CCR dept")
{
    PYTHON_RETURN_BOOL(cyMisc::IsCCRAwayStatus());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAmCCR, "Returns true if local player is a CCR")
{
    PYTHON_RETURN_BOOL(cyMisc::AmCCR());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtToggleAvatarClickability, args, "Params: on\nTurns on and off our avatar's clickability")
{
    char on;
    if (!PyArg_ParseTuple(args, "b", &on))
    {
        PyErr_SetString(PyExc_TypeError, "PtToggleAvatarClickability expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::ToggleAvatarClickability(on != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtTransferParticlesToObject, args, "Params: objFrom, objTo, num\nTransfers num particles from objFrom to objTo")
{
    PyObject* objFrom = nullptr;
    PyObject* objTo = nullptr;
    int num;
    if (!PyArg_ParseTuple(args, "OOi", &objFrom, &objTo, &num))
    {
        PyErr_SetString(PyExc_TypeError, "PtTransferParticlesToObject expects two ptKeys and an int");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(objFrom)) || (!pyKey::Check(objTo)))
    {
        PyErr_SetString(PyExc_TypeError, "PtTransferParticlesToObject expects two ptKeys and an int");
        PYTHON_RETURN_ERROR;
    }
    pyKey* from = pyKey::ConvertFrom(objFrom);
    pyKey* to = pyKey::ConvertFrom(objTo);
    cyMisc::TransferParticlesToKey(*from, *to, num);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetParticleDissentPoint, args, "Params: x, y, z, particlesys\nSets the dissent point of the particlesys to x,y,z")
{
    float x,y,z;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "fffO", &x, &y, &z, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetParticleDissentPoint expects three floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetParticleDissentPoint expects three floats and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::SetParticleDissentPoint(x, y, z, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetControlEvents, args, "Params: on, key\nRegisters or unregisters for control event messages")
{
    char on;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "bO", &on, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetControlEvents expects a boolean and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetControlEvents expects a boolean and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::RegisterForControlEventMessages(on != 0, *key);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLanguage, "Returns the current language as a PtLanguage enum")
{
    return PyLong_FromLong(cyMisc::GetLanguage());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFakeLinkAvatarToObject, args, "Params: avatar,object\nPseudo-links avatar to object within the same age\n")
{
    PyObject* avatarObj = nullptr;
    PyObject* objectObj = nullptr;
    if (!PyArg_ParseTuple(args, "OO", &avatarObj, &objectObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtFakeLinkAvatarToObject expects two ptKeys");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(avatarObj)) || (!pyKey::Check(objectObj)))
    {
        PyErr_SetString(PyExc_TypeError, "PtFakeLinkAvatarToObject expects two ptKeys");
        PYTHON_RETURN_ERROR;
    }
    pyKey* avatar = pyKey::ConvertFrom(avatarObj);
    pyKey* object = pyKey::ConvertFrom(objectObj);
    cyMisc::FakeLinkToObject(*avatar, *object);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtWearDefaultClothingType, args, kw, "Params: key, type, broadcast=False\nForces the avatar to wear the default clothing of the specified type")
{
    const char* kwlist[] = { "key", "type", "broadcast", nullptr };
    PyObject* keyObj = nullptr;
    unsigned long type;
    bool broadcast = false;
    if (!PyArg_ParseTupleAndKeywords(args, kw, "Ol|b", const_cast<char**>(kwlist), &keyObj, &type, &broadcast))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothingType expects a ptKey, an unsigned long, and an optional bool");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothingType expects a ptKey, an unsigned long, and an optional bool");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::WearDefaultClothingType(*key, type, broadcast);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFileExists, args, "Params: filename\nReturns true if the specified file exists")
{
    plFileName filename;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_PlFileNameDecoder, &filename))
    {
        PyErr_SetString(PyExc_TypeError, "PtFileExists expects a pathlike object or string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(cyMisc::FileExists(filename));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreateDir, args, "Params: directory\nCreates the directory and all parent folders. Returns false on failure")
{
    plFileName directory;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_PlFileNameDecoder, &directory))
    {
        PyErr_SetString(PyExc_TypeError, "PtCreateDir expects a pathlike object or string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(cyMisc::CreateDir(directory));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetUserPath, "Returns the unicode path to the client's root user directory. Do NOT convert to a standard string.")
{
    return PyUnicode_FromSTString(cyMisc::GetUserPath().AsString());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetInitPath, "Returns the unicode path to the client's init directory. Do NOT convert to a standard string.")
{
    return PyUnicode_FromSTString(cyMisc::GetInitPath().AsString());
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods3(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(cyMisc3)
        PYTHON_GLOBAL_METHOD(PtSendPetitionToCCR)
        PYTHON_GLOBAL_METHOD(PtSendChatToCCR)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetPythonLoggingLevel)
        PYTHON_GLOBAL_METHOD(PtSetPythonLoggingLevel)

        PYTHON_GLOBAL_METHOD(PtConsole)
        PYTHON_GLOBAL_METHOD(PtConsoleNet)

        PYTHON_GLOBAL_METHOD(PtAtTimeCallback)
        PYTHON_GLOBAL_METHOD(PtClearTimerCallbacks)

        PYTHON_GLOBAL_METHOD(PtFindSceneobject)
        PYTHON_GLOBAL_METHOD(PtFindSceneobjects)
        PYTHON_GLOBAL_METHOD(PtFindActivator)
        PYTHON_GLOBAL_METHOD(PtWasLocallyNotified)

        PYTHON_GLOBAL_METHOD(PtAttachObject)
        PYTHON_GLOBAL_METHOD(PtDetachObject)

        PYTHON_GLOBAL_METHOD(PtDirtySynchState)
        PYTHON_GLOBAL_METHOD(PtDirtySynchClients)

        PYTHON_GLOBAL_METHOD(PtEnableControlKeyEvents)
        PYTHON_GLOBAL_METHOD(PtDisableControlKeyEvents)

        PYTHON_BASIC_GLOBAL_METHOD(PtEnableAvatarCursorFade)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableAvatarCursorFade)
        PYTHON_GLOBAL_METHOD(PtFadeLocalAvatar)

        PYTHON_GLOBAL_METHOD(PtSetOfferBookMode)
        PYTHON_GLOBAL_METHOD(PtSetShareSpawnPoint)
        PYTHON_GLOBAL_METHOD(PtSetShareAgeInstanceGuid)
        PYTHON_GLOBAL_METHOD(PtNotifyOffererLinkAccepted)
        PYTHON_GLOBAL_METHOD(PtNotifyOffererLinkRejected)
        PYTHON_GLOBAL_METHOD(PtNotifyOffererLinkCompleted)
        PYTHON_BASIC_GLOBAL_METHOD(PtClearOfferBookMode)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetLocalClientID)

        PYTHON_GLOBAL_METHOD_NOARGS(PtIsCCRAway)
        PYTHON_GLOBAL_METHOD_NOARGS(PtAmCCR)

        PYTHON_GLOBAL_METHOD(PtToggleAvatarClickability)

        PYTHON_GLOBAL_METHOD(PtTransferParticlesToObject)
        PYTHON_GLOBAL_METHOD(PtSetParticleDissentPoint)

        PYTHON_GLOBAL_METHOD(PtGetControlEvents)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetLanguage)

        PYTHON_GLOBAL_METHOD(PtFakeLinkAvatarToObject)

        PYTHON_GLOBAL_METHOD(PtWearDefaultClothingType)

        PYTHON_GLOBAL_METHOD(PtFileExists)
        PYTHON_GLOBAL_METHOD(PtCreateDir)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetUserPath)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetInitPath)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, cyMisc3)
}
