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

#include "pyGmClimbingWall.h"

#include "plPythonConvert.h"
#include "pyEnum.h"
#include "pyGameCli.h"
#include "pyGlueHelpers.h"

// ===========================================================================

PYTHON_CLASS_DEFINITION(ptGmClimbingWall, pyGmClimbingWall);

PYTHON_DEFAULT_NEW_DEFINITION(ptGmClimbingWall, pyGmClimbingWall)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGmClimbingWall)

PYTHON_NO_INIT_DEFINITION(ptGmClimbingWall)

PYTHON_METHOD_DEFINITION(ptGmClimbingWall, changeNumBlockers, args)
{
    int32_t amountToAdjust;
    if (!PyArg_ParseTuple(args, "i", &amountToAdjust)) {
        PyErr_SetString(PyExc_TypeError, "ptGmClimbingWall.changeNumBlockers() expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->ChangeNumBlockers(amountToAdjust);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmClimbingWall, ready, args)
{
    EClimbingWallReadyType readyType;
    uint8_t teamNumber;
    if (!PyArg_ParseTuple(args, "bb", &readyType, &teamNumber)) {
        PyErr_SetString(PyExc_TypeError, "");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->Ready(readyType, teamNumber);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmClimbingWall, changeBlocker, args)
{
    uint8_t teamNumber, blockerNumber;
    bool added;
    if (!PyArg_ParseTuple(args, "bbb", &teamNumber, &blockerNumber, &added)) {
        PyErr_SetString(PyExc_TypeError, "");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->ChangeBlocker(teamNumber, blockerNumber, added);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmClimbingWall, reset)
{
    self->fThis->Reset();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmClimbingWall, enterPlayer, args)
{
    uint8_t teamNumber;
    if (!PyArg_ParseTuple(args, "b", &teamNumber)) {
        PyErr_SetString(PyExc_TypeError, "");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->EnterPlayer(teamNumber);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmClimbingWall, finishGame)
{
    self->fThis->FinishGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmClimbingWall, panic)
{
    self->fThis->Panic();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC(ptGmClimbingWall, join, args)
{
    PyObject* handler;
    unsigned int tableID;
    if (!PyArg_ParseTuple(args, "Oi", &handler, &tableID)) {
        PyErr_SetString(PyExc_TypeError, "ptGmClimbingWall.join() expects an object and an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    pyGmClimbingWall::Join(handler, tableID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC_NOARGS(ptGmClimbingWall, isSupported)
{
    return plPython::ConvertFrom(self->fThis->IsSupported());
}

PYTHON_START_METHODS_TABLE(ptGmClimbingWall)
    PYTHON_METHOD(ptGmClimbingWall, changeNumBlockers,
        "Type: (amountToAdjust: int) -> None\n"
        "Request for the server to change the maximum number of blockers."
    ),
    PYTHON_METHOD(ptGmClimbingWall, ready,
        "Type: (readyType: int, teamNumber: int) -> None\n"
        "Request for the server to change the ready state for the given team."
    ),
    PYTHON_METHOD(ptGmClimbingWall, changeBlocker,
        "Type: (teamNumber: int, blockerNumber: int, added: bool) -> None\n"
        "Request for the server to change the state of a blocker."
    ),
    PYTHON_METHOD_NOARGS(ptGmClimbingWall, reset,
        "Type: () -> None\n"
        "Request for the server to reset the game to the initial state."
    ),
    PYTHON_METHOD(ptGmClimbingWall, enterPlayer,
        "Type: (teamNumber: int) -> None\n"
        "Request for the server to enter the local player as the "
        "person playing the wall game for the given team."
    ),
    PYTHON_METHOD_NOARGS(ptGmClimbingWall, finishGame,
        "Type: () -> None\n"
        "Request for the server to successfully complete the climbing "
        "wall game for our team."
    ),
    PYTHON_METHOD_NOARGS(ptGmClimbingWall, panic,
        "Type: () -> None\n"
        "Request for the server to forfeit the wall game for our team."
    ),
    PYTHON_METHOD_STATIC(ptGmClimbingWall, join,
        "Type: (handler: Any, tableID: int) -> None\n"
        "Join a common climbing wall game in the current Age."
    ),
    PYTHON_METHOD_STATIC_NOARGS(ptGmClimbingWall, isSupported,
        "Type: () -> bool\n"
        "Checks for the presence of a server-side climbing wall "
        "game manager."
    ),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGmClimbingWall, pyGameCli, "Legacy climbing wall game client.");

// required functions for PyObject interoperability
PYTHON_CLASS_GMCLI_NEW_IMPL(ptGmClimbingWall, pyGmClimbingWall, pfGmClimbingWall)
PYTHON_CLASS_CHECK_IMPL(ptGmClimbingWall, pyGmClimbingWall)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGmClimbingWall, pyGmClimbingWall)

// ===========================================================================

void pyGmClimbingWall::AddPlasmaGameClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGmClimbingWall);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyGmClimbingWall::AddPlasmaGameConstantsClasses(PyObject* m)
{
    PYTHON_ENUM_START(PtClimbingWallReadyType)
    PYTHON_ENUM_ELEMENT(PtClimbingWallReadyType, kClimbingWallReadyNumBlockers, EClimbingWallReadyType::kClimbingWallReadyNumBlockers)
    PYTHON_ENUM_ELEMENT(PtClimbingWallReadyType, kClimbingWallReadyBlockers, EClimbingWallReadyType::kClimbingWallReadyBlockers)
    PYTHON_ENUM_END(m, PtClimbingWallReadyType)
}
