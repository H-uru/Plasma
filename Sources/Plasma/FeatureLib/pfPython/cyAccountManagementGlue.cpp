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

#include "cyAccountManagement.h"

#include <string_theory/string>

#include "plMessage/plAccountUpdateMsg.h"

#include "pyEnum.h"
#include "pyGlueHelpers.h"

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAccountPlayerList, "Returns list of players associated with the current account")
{
    return cyAccountManagement::GetPlayerList();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAccountName, "Returns the account name for the current account")
{
    return PyUnicode_FromSTString(cyAccountManagement::GetAccountName());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreatePlayer, args, "Params: playerName, avatarShape, invitation\nCreates a new player")
{
    ST::string playerName;
    ST::string avatarShape;
    ST::string invitation;
    if (!PyArg_ParseTuple(args, "O&O&O&", PyUnicode_STStringConverter, &playerName,
                          PyUnicode_STStringConverter, &avatarShape,
                          PyUnicode_STStringConverter, &invitation)) {
        PyErr_SetString(PyExc_TypeError, "PtCreatePlayer expects three strings");
        PYTHON_RETURN_ERROR;
    }

    cyAccountManagement::CreatePlayer(playerName, avatarShape, invitation);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDeletePlayer, args, "Params: playerInt\nDeletes a player associated with the current account")
{
    unsigned playerInt = 0;
    if (!PyArg_ParseTuple(args, "I", &playerInt))
    {
        PyErr_SetString(PyExc_TypeError, "PtDeletePlayer expects a unsigned int");
        PYTHON_RETURN_ERROR;
    }

    cyAccountManagement::DeletePlayer(playerInt);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetActivePlayer, args, "Params: playerInt\nSets the active player associated with the current account")
{
    unsigned playerInt = 0;
    if (!PyArg_ParseTuple(args, "I", &playerInt))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetActivePlayer expects a unsigned int");
        PYTHON_RETURN_ERROR;
    }

    cyAccountManagement::SetActivePlayer(playerInt);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsActivePlayerSet, "Returns whether or not an active player is set")
{
    PYTHON_RETURN_BOOL(cyAccountManagement::IsActivePlayerSet());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangePassword, args, "Params: password\nChanges the current account's password")
{
    ST::string password;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &password)) {
        PyErr_SetString(PyExc_TypeError, "PtChangePassword expects a string");
        PYTHON_RETURN_ERROR;
    }

    cyAccountManagement::ChangePassword(password);
    PYTHON_RETURN_NONE;
}

void cyAccountManagement::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(ptAccountManagement)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetAccountPlayerList)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetAccountName)
        PYTHON_GLOBAL_METHOD(PtCreatePlayer)
        PYTHON_GLOBAL_METHOD(PtDeletePlayer)
        PYTHON_GLOBAL_METHOD(PtSetActivePlayer)
        PYTHON_GLOBAL_METHOD(PtIsActivePlayerSet)
        PYTHON_GLOBAL_METHOD(PtChangePassword)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, ptAccountManagement)
}

void cyAccountManagement::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtAccountUpdateType)
    PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kCreatePlayer,     plAccountUpdateMsg::kCreatePlayer)
    PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kDeletePlayer,     plAccountUpdateMsg::kDeletePlayer)
    PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kUpgradePlayer,    plAccountUpdateMsg::kUpgradePlayer)
    PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kActivePlayer,     plAccountUpdateMsg::kActivePlayer)
    PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kChangePassword,   plAccountUpdateMsg::kChangePassword)
    PYTHON_ENUM_END(m, PtAccountUpdateType)
}
