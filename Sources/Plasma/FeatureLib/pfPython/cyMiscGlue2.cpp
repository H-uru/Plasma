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
#include <string_view>
#include <vector>

#include "plMessage/plConfirmationMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plNetCommon/plNetCommon.h"
#include "plResMgr/plLocalization.h"

#include "plPythonCallable.h"
#include "plPythonConvert.h"
#include "pyColor.h"
#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pyPlayer.h"

namespace plPython
{
    template<>
    inline PyObject* ConvertFrom(plConfirmationMsg::Result&& value)
    {
        return PyLong_FromSsize_t((Py_ssize_t)value);
    }
};

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtYesNoDialog, args, kwargs,
            "Type: (cb: Union[None, ptKey, Callable], message: str, /, dialogType: int = PtConfirmationType.YesNo) -> None\n"
            "This will display a confirmation dialog to the user with the text `message`. "
            "This dialog _has_ to be answered by the user, "
            "and their answer will be returned in a Notify message or callback given by `cb`.")
{
    const char* keywords[]{ "", "", "dialogType", nullptr };
    constexpr std::string_view kErrorMsg = "PtYesNoDialog expects a ptKey or callable, "
                                           "a string or localization path, and an optional int.";
    PyObject* cbObj;
    ST::string text;
    plConfirmationMsg::Type dialogType = plConfirmationMsg::Type::YesNo;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OO&|I", const_cast<char**>(keywords),
                                     &cbObj,
                                     PyUnicode_STStringConverter, &text,
                                     &dialogType)) {
        PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
        PYTHON_RETURN_ERROR;
    }

    plConfirmationMsg::Callback cb;
    if (pyKey::Check(cbObj)) {
        cb = pyKey::ConvertFrom(cbObj)->getKey();
    } else if (PyCallable_Check(cbObj)) {
        plPython::BuildCallback<1>("PtYesNoDialog", cbObj, cb);
    } else if (cbObj != Py_None) {
        PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
        PYTHON_RETURN_ERROR;
    }

    // We already have the message class definition included, so just send from here.
    auto msg = new plConfirmationMsg(std::move(text), dialogType, std::move(cb));
    msg->Send();

    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtLocalizedYesNoDialog, args, kwargs,
    "Type: (cb: Union[None, Callable, ptKey], path: str, *args, dialogType: int = PtConfirmationType.YesNo) -> None\n"
    "This will display a confirmation dialog to the user with the localized text `path` "
    "with any optional localization `args` applied. This dialog _has_ to be answered by the user, "
    "and their answer will be returned in a Notify message or callback given by `cb`.")
{
    constexpr std::string_view kErrorMsg = "PtLocalizedYesNoDialog expects a ptKey or callable, "
                                           "a string, optional localization arguments, and an "
                                           "optional int.";

    // We cannot use PyArg_ParseTuple or PyArg_ParseTupleAndKeywords due to our usage
    // of *args. While we could accept a single sequence for our localization arguments
    // and get that functionality back, the interface would not be very Pythonic.
    if (!PyTuple_Check(args) || PyTuple_Size(args) < 2) {
        PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
        PYTHON_RETURN_ERROR;
    }

    PyObject* cbObj = PyTuple_GET_ITEM(args, 0);
    plConfirmationMsg::Callback cb;
    if (pyKey::Check(cbObj)) {
        cb = pyKey::ConvertFrom(cbObj)->getKey();
    } else if (PyCallable_Check(cbObj)) {
        plPython::BuildCallback<1>("PtLocalizedYesNoDialog", cbObj, cb);
    } else if (cbObj != Py_None) {
        PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
        PYTHON_RETURN_ERROR;
    }

    PyObject* pathObj = PyTuple_GET_ITEM(args, 1);
    if (!PyUnicode_Check(pathObj)) {
        PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
        PYTHON_RETURN_ERROR;
    }
    ST::string path = PyUnicode_AsSTString(pathObj);

    constexpr Py_ssize_t kLocArgOffset = 2;
    const Py_ssize_t totalArgs = PyTuple_Size(args);
    std::vector<ST::string> locArgs(totalArgs - kLocArgOffset);
    for (Py_ssize_t i = kLocArgOffset; i < totalArgs; ++i) {
        PyObject* arg = PyTuple_GET_ITEM(args, i);
        if (PyUnicode_Check(arg)) {
            locArgs[i - kLocArgOffset] = PyUnicode_AsSTString(arg);
        } else {
            pyObjectRef argStr = PyObject_Str(arg);
            if (!argStr)
                // Don't blow away the internal error state
                PYTHON_RETURN_ERROR;
            locArgs[i - kLocArgOffset] = PyUnicode_AsSTString(argStr.Get());
        }
    }

    plConfirmationMsg::Type dialogType = plConfirmationMsg::Type::YesNo;
    if (kwargs) {
        if (!PyArg_ValidateKeywordArguments(kwargs)) {
            PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
            PYTHON_RETURN_ERROR;
        }

        PyObject* dialogTypeObj = PyDict_GetItemString(kwargs, "dialogType");
        if (dialogTypeObj != nullptr) {
            if (PyLong_Check(dialogTypeObj)) {
                dialogType = (plConfirmationMsg::Type)PyLong_AsLong(dialogTypeObj);
            } else if (PyNumber_Check(dialogTypeObj)) {
                // The weird internal enum type isn't an int but implements the number protocol.
                pyObjectRef dialogTypeLong = PyNumber_Long(dialogTypeObj);
                if (!dialogTypeLong) {
                    PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
                    PYTHON_RETURN_ERROR;
                }
                dialogType = (plConfirmationMsg::Type)PyLong_AsLong(dialogTypeLong.Get());
            } else {
                PyErr_SetString(PyExc_TypeError, kErrorMsg.data());
                PYTHON_RETURN_ERROR;
            }
        }
    }

    auto msg = new plLocalizedConfirmationMsg(std::move(path), std::move(locArgs), dialogType, std::move(cb));
    msg->Send();

    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRateIt, args, "Params: chronicleName,dialogPrompt,onceFlag\nShows a dialog with dialogPrompt and stores user input rating into chronicleName")
{
    ST::string chronicleName;
    ST::string dialogPrompt;
    char onceFlag;
    if (!PyArg_ParseTuple(args, "O&O&b", PyUnicode_STStringConverter, &chronicleName, PyUnicode_STStringConverter, &dialogPrompt, &onceFlag))
    {
        PyErr_SetString(PyExc_TypeError, "PtRateIt expects two strings and a boolean");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::RateIt(chronicleName, dialogPrompt, onceFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtExcludeRegionSet, args, "Params: senderKey,regionKey,state\nThis will set the state of an exclude region\n"
            "- 'senderKey' is a ptKey of the PythonFile component\n"
            "- 'regionKey' is a ptKey of the exclude region\n"
            "- 'state' is either kExRegRelease or kExRegClear")
{
    PyObject* senderObj = nullptr;
    PyObject* regionObj = nullptr;
    unsigned short stateVal;
    if (!PyArg_ParseTuple(args, "OOh", &senderObj, &regionObj, &stateVal))
    {
        PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSet expects two ptKeys and a short");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(senderObj)) || (!pyKey::Check(regionObj)))
    {
        PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSet expects two ptKeys and a short");
        PYTHON_RETURN_ERROR;
    }
    pyKey* sender = pyKey::ConvertFrom(senderObj);
    pyKey* region = pyKey::ConvertFrom(regionObj);
    cyMisc::ExcludeRegionSet(*sender, *region, stateVal);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtExcludeRegionSetNow, args, "Params: senderKey,regionKey,state\nThis will set the state of an exclude region immediately on the server\n"
            "- 'senderKey' is a ptKey of the PythonFile component\n"
            "- 'regionKey' is a ptKey of the exclude region\n"
            "- 'state' is either kExRegRelease or kExRegClear")
{
    PyObject* senderObj = nullptr;
    PyObject* regionObj = nullptr;
    unsigned short stateVal;
    if (!PyArg_ParseTuple(args, "OOh", &senderObj, &regionObj, &stateVal))
    {
        PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSetNow expects two ptKeys and a short");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyKey::Check(senderObj)) || (!pyKey::Check(regionObj)))
    {
        PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSetNow expects two ptKeys and a short");
        PYTHON_RETURN_ERROR;
    }
    pyKey* sender = pyKey::ConvertFrom(senderObj);
    pyKey* region = pyKey::ConvertFrom(regionObj);
    cyMisc::ExcludeRegionSetNow(*sender, *region, stateVal);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAcceptInviteInGame, args, "Params: friendName,inviteKey\nSends a VaultTask to the server to perform the invite")
{
    ST::string friendName;
    ST::string inviteKey;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_STStringConverter, &friendName, PyUnicode_STStringConverter, &inviteKey))
    {
        PyErr_SetString(PyExc_TypeError, "PtAcceptInviteInGame expects two strings");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::AcceptInviteInGame(friendName, inviteKey);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetTime, "Returns the number of seconds since the game was started.")
{
    return PyFloat_FromDouble(cyMisc::GetSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetGameTime, "Returns the system game time (frame based) in seconds.")
{
    return PyFloat_FromDouble(cyMisc::GetSysSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetFrameDeltaTime, "Returns the amount of time that has elapsed since last frame.")
{
    return PyFloat_FromDouble(cyMisc::GetDelSysSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtPageInNode, args, "Params: nodeName, netForce=False, ageName=\"\"\nPages in node, or a list of nodes")
{
    PyObject* nodeNameObj = nullptr;
    ST::string ageName;
    char netForce = 0;
    if (!PyArg_ParseTuple(args, "O|bO&", &nodeNameObj, &netForce, PyUnicode_STStringConverter, &ageName))
    {
        PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
        PYTHON_RETURN_ERROR;
    }
    std::vector<ST::string> nodeNames;
    if (PyUnicode_Check(nodeNameObj))
    {
        nodeNames.emplace_back(PyUnicode_AsSTString(nodeNameObj));
    }
    else if (PyList_Check(nodeNameObj))
    {
        Py_ssize_t num = PyList_Size(nodeNameObj);
        for (Py_ssize_t i = 0; i < num; i++)
        {
            PyObject* listItem = PyList_GetItem(nodeNameObj, i);
            if (!PyUnicode_Check(listItem))
            {
                PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
                PYTHON_RETURN_ERROR;
            }
            nodeNames.emplace_back(PyUnicode_AsSTString(listItem));
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::PageInNodes(nodeNames, ageName, netForce);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtPageOutNode, args, "Params: nodeName, netForce=False\nPages out a node")
{
    ST::string nodeName;
    char netForce = 0;
    if (!PyArg_ParseTuple(args, "O&|b", PyUnicode_STStringConverter, &nodeName, &netForce))
    {
        PyErr_SetString(PyExc_TypeError, "PtPageOutNode expects a string and bool");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::PageOutNode(nodeName, netForce);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLimitAvatarLOD, args, "Params: LODlimit\nSets avatar's LOD limit")
{
    int lodLimit;
    if (!PyArg_ParseTuple(args, "i", &lodLimit))
    {
        PyErr_SetString(PyExc_TypeError, "PtLimitAvatarLOD expects an integer");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::LimitAvatarLOD(lodLimit);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefColor, args, "Params: color\nSets default fog color")
{
    PyObject* colorObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtFogSetDefColor expects a ptColor object");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtFogSetDefColor expects a ptColor object");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color = pyColor::ConvertFrom(colorObj);
    cyMisc::FogSetDefColor(*color);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefLinear, args, "Params: start,end,density\nSet linear fog values")
{
    float start, end, density;
    if (!PyArg_ParseTuple(args, "fff", &start, &end, &density))
    {
        PyErr_SetString(PyExc_TypeError, "PtFogSetDefLinear expects three floats");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FogSetDefLinear(start, end, density);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefExp, args, "Params: end,density\nSet exp fog values")
{
    float end, density;
    if (!PyArg_ParseTuple(args, "ff", &end, &density))
    {
        PyErr_SetString(PyExc_TypeError, "PtFogSetDefExp expects three floats");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FogSetDefExp(end, density);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefExp2, args, "Params: end,density\nSet exp2 fog values")
{
    float end, density;
    if (!PyArg_ParseTuple(args, "ff", &end, &density))
    {
        PyErr_SetString(PyExc_TypeError, "PtFogSetDefExp2 expects three floats");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::FogSetDefExp2(end, density);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadDialog, args, "Params: dialogName,selfKey=None,ageName=\"\"\nLoads a GUI dialog by name and optionally set the Notify proc key\n"
            "If the dialog is already loaded then it won't load it again")
{
    ST::string dialogName;
    PyObject* keyObj = nullptr;
    ST::string ageName = ST_LITERAL("GUI");
    if (!PyArg_ParseTuple(args, "O&|OO&", PyUnicode_STStringConverter, &dialogName, &keyObj, PyUnicode_STStringConverter, &ageName))
    {
        PyErr_SetString(PyExc_TypeError, "PtLoadDialog expects a string, and optionally a ptKey and second string");
        PYTHON_RETURN_ERROR;
    }
    if (keyObj)
    {
        if (!pyKey::Check(keyObj))
        {
            PyErr_SetString(PyExc_TypeError, "PtLoadDialog expects a string, and optionally a ptKey and second string");
            PYTHON_RETURN_ERROR;
        }
        pyKey* key = pyKey::ConvertFrom(keyObj);
        cyMisc::LoadDialogKA(dialogName, *key, ageName);
    }
    else
        cyMisc::LoadDialog(dialogName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUnloadDialog, args, "Params: dialogName\nThis will unload the GUI dialog by name. If not loaded then nothing will happen")
{
    ST::string dialogName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &dialogName))
    {
        PyErr_SetString(PyExc_TypeError, "PtUnloadDialog expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::UnloadDialog(dialogName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsDialogLoaded, args, "Params: dialogName\nTest to see if a GUI dialog is loaded, by name")
{
    ST::string dialogName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &dialogName))
    {
        PyErr_SetString(PyExc_TypeError, "PtIsDialogLoaded expects a string");
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_BOOL(cyMisc::IsDialogLoaded(dialogName));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShowDialog, args, "Params: dialogName\nShow a GUI dialog by name (does not load dialog)")
{
    ST::string dialogName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &dialogName))
    {
        PyErr_SetString(PyExc_TypeError, "PtShowDialog expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::ShowDialog(dialogName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtHideDialog, args, "Params: dialogName\nHide a GUI dialog by name (does not unload dialog)")
{
    ST::string dialogName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &dialogName))
    {
        PyErr_SetString(PyExc_TypeError, "PtHideDialog expects a string");
        PYTHON_RETURN_ERROR;
    }
    cyMisc::HideDialog(dialogName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetDialogFromTagID, args, "Params: tagID\nReturns the dialog associated with the tagID")
{
    unsigned long tagID;
    if (!PyArg_ParseTuple(args, "l", &tagID))
    {
        PyErr_SetString(PyExc_TypeError, "PtGetDialogFromTagID expects a long");
        PYTHON_RETURN_ERROR;
    }
    return cyMisc::GetDialogFromTagID(tagID);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetDialogFromString, args, "Params: dialogName\nGet a ptGUIDialog from its name")
{
    ST::string dialogName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &dialogName))
    {
        PyErr_SetString(PyExc_TypeError, "PtHideDialog expects a string");
        PYTHON_RETURN_ERROR;
    }
    return cyMisc::GetDialogFromString(dialogName);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsGUIModal, "Returns true if the GUI is displaying a modal dialog and blocking input")
{
    PYTHON_RETURN_BOOL(cyMisc::IsGUIModal());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendPrivateChatList, args, "Params: chatList\nLock the local avatar into private vox messaging, and / or add new members to his chat list")
{
    PyObject* chatListObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &chatListObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
        PYTHON_RETURN_ERROR;
    }

    std::vector<pyPlayer*> chatList;
    if (PyList_Check(chatListObj))
    {
        Py_ssize_t listSize = PyList_Size(chatListObj);
        for (Py_ssize_t i = 0; i < listSize; i++)
        {
            PyObject* listItem = PyList_GetItem(chatListObj, i);
            if (!pyPlayer::Check(listItem))
            {
                PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
                PYTHON_RETURN_ERROR;
            }
            chatList.push_back(pyPlayer::ConvertFrom(listItem));
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
        PYTHON_RETURN_ERROR;
    }

    cyMisc::SetPrivateChatList(chatList);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtClearPrivateChatList, args, "Params: memberKey\nRemove the local avatar from private vox messaging, and / or clear members from his chat list")
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtClearPrivateChatList expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "PtClearPrivateChatList expects a ptKey");
        PYTHON_RETURN_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    cyMisc::ClearPrivateChatList(*key);
    PYTHON_RETURN_NONE;
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods2(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(cyMisc2)
        PYTHON_GLOBAL_METHOD(PtYesNoDialog)
        PYTHON_GLOBAL_METHOD(PtLocalizedYesNoDialog)
        PYTHON_GLOBAL_METHOD(PtRateIt)

        PYTHON_GLOBAL_METHOD(PtExcludeRegionSet)
        PYTHON_GLOBAL_METHOD(PtExcludeRegionSetNow)

        PYTHON_GLOBAL_METHOD(PtAcceptInviteInGame)

        PYTHON_GLOBAL_METHOD_NOARGS(PtGetTime)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetGameTime)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetFrameDeltaTime)

        PYTHON_GLOBAL_METHOD(PtPageInNode)
        PYTHON_GLOBAL_METHOD(PtPageOutNode)

        PYTHON_GLOBAL_METHOD(PtLimitAvatarLOD)

        PYTHON_GLOBAL_METHOD(PtFogSetDefColor)
        PYTHON_GLOBAL_METHOD(PtFogSetDefLinear)
        PYTHON_GLOBAL_METHOD(PtFogSetDefExp)
        PYTHON_GLOBAL_METHOD(PtFogSetDefExp2)

        PYTHON_GLOBAL_METHOD(PtLoadDialog)
        PYTHON_GLOBAL_METHOD(PtUnloadDialog)
        PYTHON_GLOBAL_METHOD(PtIsDialogLoaded)
        PYTHON_GLOBAL_METHOD(PtShowDialog)
        PYTHON_GLOBAL_METHOD(PtHideDialog)
        PYTHON_GLOBAL_METHOD(PtGetDialogFromTagID)
        PYTHON_GLOBAL_METHOD(PtGetDialogFromString)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsGUIModal)

        PYTHON_GLOBAL_METHOD(PtSendPrivateChatList)
        PYTHON_GLOBAL_METHOD(PtClearPrivateChatList)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, cyMisc2)
}

void cyMisc::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtConfirmationResult)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, OK, plConfirmationMsg::Result::OK)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, Cancel, plConfirmationMsg::Result::Cancel)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, Yes, plConfirmationMsg::Result::Yes)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, No, plConfirmationMsg::Result::No)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, Quit, plConfirmationMsg::Result::Quit)
    PYTHON_ENUM_ELEMENT(PtConfirmationResult, Logout, plConfirmationMsg::Result::Logout)
    PYTHON_ENUM_END(m, PtConfirmationResult)

    PYTHON_ENUM_START(m, PtConfirmationType)
    PYTHON_ENUM_ELEMENT(PtConfirmationType, OK, plConfirmationMsg::Type::OK)
    PYTHON_ENUM_ELEMENT(PtConfirmationType, ConfirmQuit, plConfirmationMsg::Type::ConfirmQuit)
    PYTHON_ENUM_ELEMENT(PtConfirmationType, ForceQuit, plConfirmationMsg::Type::ForceQuit)
    PYTHON_ENUM_ELEMENT(PtConfirmationType, YesNo, plConfirmationMsg::Type::YesNo)
    PYTHON_ENUM_END(m, PtConfirmationType)

    PYTHON_ENUM_START(m, PtCCRPetitionType)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kGeneralHelp,plNetCommon::PetitionTypes::kGeneralHelp)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kBug,        plNetCommon::PetitionTypes::kBug)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kFeedback,   plNetCommon::PetitionTypes::kFeedback)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kExploit,    plNetCommon::PetitionTypes::kExploit)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kHarass,     plNetCommon::PetitionTypes::kHarass)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kStuck,      plNetCommon::PetitionTypes::kStuck)
    PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kTechnical,  plNetCommon::PetitionTypes::kTechnical)
    PYTHON_ENUM_END(m, PtCCRPetitionType)

    PYTHON_ENUM_START(m, PtLanguage)
    PYTHON_ENUM_ELEMENT(PtLanguage, kEnglish,       plLocalization::kEnglish)
    PYTHON_ENUM_ELEMENT(PtLanguage, kFrench,        plLocalization::kFrench)
    PYTHON_ENUM_ELEMENT(PtLanguage, kGerman,        plLocalization::kGerman)
    PYTHON_ENUM_ELEMENT(PtLanguage, kSpanish,       plLocalization::kSpanish)
    PYTHON_ENUM_ELEMENT(PtLanguage, kItalian,       plLocalization::kItalian)
    PYTHON_ENUM_ELEMENT(PtLanguage, kJapanese,      plLocalization::kJapanese)
    PYTHON_ENUM_ELEMENT(PtLanguage, kDutch,         plLocalization::kDutch)
    PYTHON_ENUM_ELEMENT(PtLanguage, kRussian,       plLocalization::kRussian)
    PYTHON_ENUM_ELEMENT(PtLanguage, kPolish,        plLocalization::kPolish)
    PYTHON_ENUM_ELEMENT(PtLanguage, kCzech,         plLocalization::kCzech)
    PYTHON_ENUM_ELEMENT(PtLanguage, kNumLanguages,  plLocalization::kNumLanguages)
    PYTHON_ENUM_END(m, PtLanguage)

    PYTHON_ENUM_START(m, PtLOSReportType)
    PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportHit,        plLOSRequestMsg::kReportHit)
    PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportMiss,       plLOSRequestMsg::kReportMiss)
    PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportHitOrMiss,  plLOSRequestMsg::kReportHitOrMiss)
    PYTHON_ENUM_END(m, PtLOSReportType)

    PYTHON_ENUM_START(m, PtLOSObjectType)
    PYTHON_ENUM_ELEMENT(PtLOSObjectType, kClickables,       kClickables)
    PYTHON_ENUM_ELEMENT(PtLOSObjectType, kCameraBlockers,   kCameraBlockers)
    PYTHON_ENUM_ELEMENT(PtLOSObjectType, kCustom,           kCustom)
    PYTHON_ENUM_ELEMENT(PtLOSObjectType, kShootable,        kShootable)
    PYTHON_ENUM_END(m, PtLOSObjectType)
}
