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

#include <string_theory/string>
#include <utility>
#include <vector>

#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pyPlayer.h"
#include "plPythonConvert.h"


PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtFlashWindow, cyMisc::FlashWindow, "Flashes the client window if it is not focused");

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeName, "Returns filename of the current Age")
{
    return PyUnicode_FromSTString(cyMisc::GetAgeName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeInfo, "Returns ptAgeInfoStruct of the current Age")
{
    return cyMisc::GetAgeInfo();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPrevAgeName, "Returns filename of previous age visited")
{
    return PyUnicode_FromSTString(cyMisc::GetPrevAgeName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPrevAgeInfo, "Returns ptAgeInfoStruct of previous age visited")
{
    return cyMisc::GetPrevAgeInfo();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDniTime, "Returns current D'Ni time")
{
    return PyLong_FromUnsignedLong((unsigned long)cyMisc::GetDniTime());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetServerTime, "Returns the current time on the server (which is GMT)")
{
    return PyLong_FromUnsignedLong((unsigned long)cyMisc::GetServerTime());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGMTtoDniTime, args, "Params: gtime\nConverts GMT time (passed in) to D'Ni time")
{
    unsigned long gtime;
    if (!PyArg_ParseTuple(args, "l", &gtime))
    {
        PyErr_SetString(PyExc_TypeError, "PtGMTtoDniTime expects a long");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromUnsignedLong((unsigned long)cyMisc::ConvertGMTtoDni(gtime));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetClientName, args, "Params: avatarKey=None\nThis will return the name of the client that is owned by the avatar\n"
            "- avatarKey is the ptKey of the avatar to get the client name of.\n"
            "If avatarKey is omitted then the local avatar is used")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "|O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetClientName expects an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (keyObj != nullptr)
    {
        if (!pyKey::Check(keyObj))
        {
            PyErr_SetString(PyExc_TypeError, "PtGetClientName expects a ptKey");
            PYTHON_RETURN_ERROR;
        }
        pyKey* key = pyKey::ConvertFrom(keyObj);
        return PyUnicode_FromSTString(cyMisc::GetClientName(*key));
    }
    else
        return PyUnicode_FromSTString(cyMisc::GetLocalClientName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalAvatar, "This will return a ptSceneobject of the local avatar\n"
            "- if there is no local avatar a NameError exception will happen.")
{
    return cyMisc::GetLocalAvatar();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalPlayer, "Returns a ptPlayer object of the local player")
{
    return cyMisc::GetLocalPlayer();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsSolo, "Type: () -> bool\nReturns whether we are the only player in the Age")
{
    return plPython::ConvertFrom(cyMisc::IsSolo());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPlayerList, "Returns a list of ptPlayer objects of all the remote players")
{
    std::vector<PyObject*> playerList = cyMisc::GetPlayerList();
    PyObject* retVal = PyList_New(playerList.size());
    for (int i = 0; i < playerList.size(); i++)
        PyList_SetItem(retVal, i, playerList[i]); // steals the ref
    return retVal;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPlayerListDistanceSorted, "Returns a list of ptPlayers, sorted by distance")
{
    std::vector<PyObject*> playerList = cyMisc::GetPlayerListDistanceSorted();
    PyObject* retVal = PyList_New(playerList.size());
    for (int i = 0; i < playerList.size(); i++)
        PyList_SetItem(retVal, i, playerList[i]); // steals the ref
    return retVal;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtMaxListenListSize, "Returns the maximum listen number of players")
{
    return PyLong_FromUnsignedLong(cyMisc::GetMaxListenListSize());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtMaxListenDistSq, "Returns the maximum distance (squared) of the listen range")
{
    return PyFloat_FromDouble(cyMisc::GetMaxListenDistSq());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetAvatarKeyFromClientID, args, "Params: clientID\nFrom an integer that is the clientID, find the avatar and return its ptKey")
{
    int clientID;
    if (!PyArg_ParseTuple(args, "i", &clientID))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetAvatarKeyFromClientID expects an integer");
        PYTHON_RETURN_ERROR;
    }
    return cyMisc::GetAvatarKeyFromClientID(clientID);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetClientIDFromAvatarKey, args, "Params: avatarKey\nFrom a ptKey that points at an avatar, return the players clientID (integer)")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetClientIDFromAvatarKey expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetClientIDFromAvatarKey expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey *key = pyKey::ConvertFrom(keyObj);
    return PyLong_FromLong(cyMisc::GetClientIDFromAvatarKey(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetNPCByID, args, "Params: npcID\nThis will return the NPC with a specific ID")
{
    int npcID;
    if (!PyArg_ParseTuple(args, "i", &npcID))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetNPCByID expects an integer");
        PYTHON_RETURN_ERROR;
    }

    return cyMisc::GetNPC(npcID);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetNPCCount, "Returns the number of NPCs in the current age")
{
    return cyMisc::GetNPCCount();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetNumRemotePlayers, "Returns the number of remote players in this Age with you.")
{
    return PyLong_FromLong(cyMisc::GetNumRemotePlayers());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtValidateKey, args, "Params: key\nReturns true(1) if 'key' is valid and loaded,\n"
            "otherwise returns false(0)")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtValidateKey expects an object");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PYTHON_RETURN_BOOL(false);
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    PYTHON_RETURN_BOOL(cyMisc::ValidateKey(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendRTChat, args, "Params: fromPlayer,toPlayerList,message,flags\nSends a realtime chat message to the list of ptPlayers\n"
            "If toPlayerList is an empty list, it is a broadcast message")
{
    PyObject* fromPlayerObj = nullptr;
    PyObject* toPlayerListObj = nullptr;
    ST::string message;
    unsigned int msgFlags = 0;
    const char* err = "PtSendRTChat expects a ptPlayer, a sequence of ptPlayers, a string, and an optional long";

    if (!PyArg_ParseTuple(args, "OOO&|I", &fromPlayerObj, &toPlayerListObj,
                          PyUnicode_STStringConverter, &message, &msgFlags))
    {
        PyErr_SetString(PyExc_TypeError, err);
        PYTHON_RETURN_ERROR;
    }

    if (!pyPlayer::Check(fromPlayerObj))
    {
        PyErr_SetString(PyExc_TypeError, err);
        PYTHON_RETURN_ERROR;
    }
    pyPlayer* sender = pyPlayer::ConvertFrom(fromPlayerObj);

    if (!PySequence_Check(toPlayerListObj))
    {
        PyErr_SetString(PyExc_TypeError, err);
        PYTHON_RETURN_ERROR;
    }
    std::vector<pyPlayer*> toPlayers;
    toPlayers.reserve(PySequence_Size(toPlayerListObj));
    for (Py_ssize_t i = 0; i < PySequence_Size(toPlayerListObj); ++i)
    {
        PyObject* item = PySequence_GetItem(toPlayerListObj, i);
        if (!pyPlayer::Check(item))
        {
            PyErr_SetString(PyExc_TypeError, err);
            PYTHON_RETURN_ERROR;
        }
        toPlayers.push_back(pyPlayer::ConvertFrom(item));
    }

    return PyLong_FromUnsignedLong(cyMisc::SendRTChat(*sender, toPlayers, message, msgFlags));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIMessage, args, "Params: command,value\nSends a command message to the KI frontend.\n"
            "See PlasmaKITypes.py for list of commands")
{
    unsigned long command;
    PyObject* val;
    if (!PyArg_ParseTuple(args, "lO", &command, &val))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIMessage expects a long and either a float or a string");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(val))
    {
        cyMisc::SendKIMessageS(command, PyUnicode_AsSTString(val));
    }
    else if (PyFloat_Check(val))
    {
        float floatValue = (float)PyFloat_AsDouble(val);
        cyMisc::SendKIMessage(command, floatValue);
    }
    else if (PyLong_Check(val))
    {
        // accepting an int if people get lazy
        float floatValue = (float)PyLong_AsLong(val);
        cyMisc::SendKIMessage(command, floatValue);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIMessage expects a long and either a float or a string");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIMessageInt, args, "Params: command,value\nSame as PtSendKIMessage except the value is guaranteed to be a uint32_t\n"
            "(for things like player IDs)")
{
    unsigned long command;
    long val;
    if (!PyArg_ParseTuple(args, "ll", &command, &val))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendKIMessageInt expects two longs");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::SendKIMessageI(command, val);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadAvatarModel, args, "Params: modelName, spawnPoint, userStr = \"\"\nLoads an avatar model at the given spawn point. Assigns the user specified string to it.")
{
    ST::string modelName;
    PyObject* keyObj;
    ST::string userStr;
    if (!PyArg_ParseTuple(args, "O&O|O&", PyUnicode_STStringConverter, &modelName, &keyObj, PyUnicode_STStringConverter, &userStr) ||
        !pyKey::Check(keyObj)) {
        PyErr_SetString(PyExc_TypeError, "PtLoadAvatarModel expects a string, a ptKey, and an optional string");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);

    return cyMisc::LoadAvatarModel(std::move(modelName), *key, userStr);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUnLoadAvatarModel, args, "Params: avatarKey\nForcibly unloads the specified avatar model.\n"
            "Do not use this method unless you require fine-grained control of avatar unloading.")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtUnLoadAvatarModel expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtUnLoadAvatarModel expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::UnLoadAvatarModel(*key);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtForceCursorHidden, cyMisc::ForceCursorHidden, "Forces the cursor to hide, overriding everything.\n"
            "Only call if other methods won't work. The only way to show the cursor after this call is PtForceMouseShown()")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtForceCursorShown, cyMisc::ForceCursorShown, "Forces the cursor to show, overriding everything.\n"
            "Only call if other methods won't work. This is the only way to show the cursor after a call to PtForceMouseHidden()")

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetLocalizedString, args, "Params: name, arguments=None\nReturns the localized string specified by name "
            "(format is Age.Set.Name) and substitutes the arguments in the list of strings passed in as arguments.")
{
    ST::string name;
    PyObject* argObj = nullptr;
    if (!PyArg_ParseTuple(args, "O&|O", PyUnicode_STStringConverter, &name, &argObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a string and a list of strings");
        PYTHON_RETURN_ERROR;
    }
    std::vector<ST::string> argList;

    // convert name from a string
    if (name.empty())
    {
        PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a string and a list of strings");
        PYTHON_RETURN_ERROR;
    }

    if (argObj != nullptr) // NULL is valid... but won't fill the args vector
    {
        // convert args from a list of strings
        if (!PyList_Check(argObj))
        {
            PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a string and a list of strings");
            PYTHON_RETURN_ERROR;
        }

        Py_ssize_t len = PyList_Size(argObj);
        for (Py_ssize_t curItem = 0; curItem < len; curItem++)
        {
            PyObject* item = PyList_GetItem(argObj, curItem);
            ST::string arg = ST_LITERAL("INVALID ARG");
            if (item == Py_None) // none is allowed, but treated as a blank string
                arg = "";
            else if (PyUnicode_Check(item))
                arg = PyUnicode_AsSTString(item);
            // everything else won't throw an error, but will show up as INVALID ARG in the string
            argList.push_back(arg);
        }
    }

    return PyUnicode_FromSTString(cyMisc::GetLocalizedString(name, argList));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDumpLogs, args, "Params: folder\nDumps all current log files to the specified folder (a sub-folder to the log folder)")
{
    ST::string folder;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &folder)) {
        PyErr_SetString(PyExc_TypeError, "PtDumpLogs expects a string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(cyMisc::DumpLogs(folder));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCloneKey, args, "Params: key, loading=False\nCreates clone of key")
{
    PyObject* keyObj = nullptr;
    char loading = 0;
    if (!PyArg_ParseTuple(args, "O|b", &keyObj, &loading) || !pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtCloneKey expects a ptKey and bool");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    return cyMisc::CloneKey(key, loading);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindClones, args, "Params: key\nFinds all clones")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj) || !pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtFindClones expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    return cyMisc::FindClones(key);
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(cyMisc)
        PYTHON_GLOBAL_METHOD_NOARGS(PtFlashWindow)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetAgeName)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetAgeInfo)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetPrevAgeName) 
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetPrevAgeInfo)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetDniTime)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetServerTime)
        PYTHON_GLOBAL_METHOD(PtGMTtoDniTime)

        PYTHON_GLOBAL_METHOD(PtGetClientName)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetLocalAvatar)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetLocalPlayer)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsSolo)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetPlayerList)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetPlayerListDistanceSorted)
        PYTHON_GLOBAL_METHOD_NOARGS(PtMaxListenListSize)
        PYTHON_GLOBAL_METHOD_NOARGS(PtMaxListenDistSq)
        PYTHON_GLOBAL_METHOD(PtGetAvatarKeyFromClientID)
        PYTHON_GLOBAL_METHOD(PtGetClientIDFromAvatarKey)
        PYTHON_GLOBAL_METHOD(PtGetNPCByID)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetNPCCount)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetNumRemotePlayers)

        PYTHON_GLOBAL_METHOD(PtValidateKey)

        PYTHON_GLOBAL_METHOD(PtSendRTChat)
        PYTHON_GLOBAL_METHOD(PtSendKIMessage)
        PYTHON_GLOBAL_METHOD(PtSendKIMessageInt)
    
        PYTHON_GLOBAL_METHOD(PtLoadAvatarModel)
        PYTHON_GLOBAL_METHOD(PtUnLoadAvatarModel)

        PYTHON_BASIC_GLOBAL_METHOD(PtForceCursorHidden)
        PYTHON_BASIC_GLOBAL_METHOD(PtForceCursorShown)

        PYTHON_GLOBAL_METHOD(PtGetLocalizedString)

        PYTHON_GLOBAL_METHOD(PtDumpLogs)
        PYTHON_GLOBAL_METHOD(PtCloneKey)
        PYTHON_GLOBAL_METHOD(PtFindClones)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, cyMisc)
}
