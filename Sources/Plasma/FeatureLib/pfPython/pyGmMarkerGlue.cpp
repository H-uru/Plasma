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

#include "pyGmMarker.h"

#include <string_theory/string>

#include "plPythonConvert.h"
#include "pyEnum.h"
#include "pyGameCli.h"
#include "pyGlueHelpers.h"

// ===========================================================================

PYTHON_CLASS_DEFINITION(ptGmMarker, pyGmMarker);

PYTHON_DEFAULT_NEW_DEFINITION(ptGmMarker, pyGmMarker)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGmMarker)

PYTHON_NO_INIT_DEFINITION(ptGmMarker)

PYTHON_METHOD_DEFINITION_NOARGS(ptGmMarker, startGame)
{
    self->fThis->StartGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmMarker, pauseGame)
{
    self->fThis->PauseGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmMarker, resetGame)
{
    self->fThis->ResetGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmMarker, changeGameName, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name)) {
        PyErr_SetString(PyExc_TypeError, "changeGameName expects a string");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->ChangeGameName(name);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmMarker, changeTimeLimit, args)
{
    uint32_t timeLimit;
    if (!PyArg_ParseTuple(args, "i", &timeLimit)) {
        PyErr_SetString(PyExc_TypeError, "changeTimeLimit expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->ChangeTimeLimit(timeLimit);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGmMarker, deleteGame)
{
    self->fThis->DeleteGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_WKEY(ptGmMarker, addMarker, args, kwds)
{
    const char* kwdlist[] = { "x", "y", "z", "name", "age", nullptr };
    double x, y, z;
    ST::string name, age;
    int result = PyArg_ParseTupleAndKeywords(
        args, kwds,
        "dddO&O&", const_cast<char**>(kwdlist),
        &x, &y, &z,
        PyUnicode_STStringConverter, &name,
        PyUnicode_STStringConverter, &age
    );
    if (result == 0) {
        PyErr_SetString(PyExc_TypeError, "addMarker expects a double, double, double, string, and string");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->AddMarker(x, y, z, name, age);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmMarker, deleteMarker, args)
{
    uint32_t markerID;
    if (!PyArg_ParseTuple(args, "i", &markerID)) {
        PyErr_SetString(PyExc_TypeError, "deleteMarker expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->DeleteMarker(markerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmMarker, changeMarkerName, args)
{
    uint32_t markerID;
    ST::string markerName;
    if (!PyArg_ParseTuple(args, "iO&", &markerID, PyUnicode_STStringConverter, &markerName)) {
        PyErr_SetString(PyExc_TypeError, "changeMarkerName expects an unsigned int and string");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->ChangeMarkerName(markerID, markerName);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmMarker, captureMarker, args)
{
    uint32_t markerID;
    if (!PyArg_ParseTuple(args, "i", &markerID)) {
        PyErr_SetString(PyExc_TypeError, "captureMarker expects an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->CaptureMarker(markerID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC(ptGmMarker, create, args)
{
    PyObject* handler;
    EMarkerGameType gameType;
    ST::string templateID;
    if (!PyArg_ParseTuple(args, "ObO&", &handler, &gameType, PyUnicode_STStringConverter, &templateID)) {
        PyErr_SetString(PyExc_TypeError, "ptGmMarker.create() expects an object, an int, and a string");
        PYTHON_RETURN_ERROR;
    }

    pyGmMarker::Create(handler, gameType, std::move(templateID));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC_NOARGS(ptGmMarker, isSupported)
{
    return plPython::ConvertFrom(self->fThis->IsSupported());
}

PYTHON_START_METHODS_TABLE(ptGmMarker)
    PYTHON_METHOD_NOARGS(ptGmMarker, startGame, "Type: () -> None\nRequest for the server to start the marker game."),
    PYTHON_METHOD_NOARGS(ptGmMarker, pauseGame, "Type: () -> None\nRequest for the server to pause the marker game."),
    PYTHON_METHOD_NOARGS(ptGmMarker, resetGame, "Type: () -> None\nRequest for the server to clear all markers to the uncaptured state."),
    PYTHON_METHOD(ptGmMarker, changeGameName, "Type: (name: str) -> None\nRequest for the server to change the internal marker game name."),
    PYTHON_METHOD(ptGmMarker, changeTimeLimit, "Type: (timeLimit: int) -> None\nRequest for the server to change the marker game's time limit."),
    PYTHON_METHOD_NOARGS(ptGmMarker, deleteGame, "Type: () -> None\n"
                                                 "Request for the server to delete all data associated with this game, including "
                                                 "the marker definitions and game name."),
    PYTHON_METHOD_WKEY(ptGmMarker, addMarker, "Type: (x: float, y: float, z: float, name: str, age: str) -> None\nRequest for the server to add a new marker to the game."),
    PYTHON_METHOD(ptGmMarker, deleteMarker, "Type: (markerID: int) -> None\nRequest for the server to delete a specific marker from the game."),
    PYTHON_METHOD(ptGmMarker, changeMarkerName, "Type: (markerID: int) -> None\nRequest for the server to change the name of a specific marker from the game."),
    PYTHON_METHOD(ptGmMarker, captureMarker, "Type: (markerId: int) -> None\nRequest for the server to register a capture of the specified marker for our team."),
    PYTHON_METHOD_STATIC(ptGmMarker, create, "Type: (handler: Any, gameType: int, templateId: Optional[str]) -> None\nInitialize a new marker game client with the server."),
    PYTHON_METHOD_STATIC_NOARGS(ptGmMarker, isSupported, "Type: () -> bool\nChecks for the presence of a server-side marker game manager."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGmMarker, pyGameCli, "Legacy marker game client.");

// required functions for PyObject interoperability
PYTHON_CLASS_GMCLI_NEW_IMPL(ptGmMarker, pyGmMarker, pfGmMarker)
PYTHON_CLASS_CHECK_IMPL(ptGmMarker, pyGmMarker)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGmMarker, pyGmMarker)

// ===========================================================================

void pyGmMarker::AddPlasmaGameClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGmMarker);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyGmMarker::AddPlasmaGameConstantsClasses(PyObject* m)
{
    PYTHON_ENUM_START(m, PtMarkerGameType)
    PYTHON_ENUM_ELEMENT(PtMarkerGameType, kMarkerGameQuest, EMarkerGameType::kMarkerGameQuest)
    PYTHON_ENUM_ELEMENT(PtMarkerGameType, kMarkerGameCGZ, EMarkerGameType::kMarkerGameCGZ)
    PYTHON_ENUM_ELEMENT(PtMarkerGameType, kMarkerGameCapture, EMarkerGameType::kMarkerGameCapture)
    PYTHON_ENUM_ELEMENT(PtMarkerGameType, kMarkerGameCaptureAndHold, EMarkerGameType::kMarkerGameCaptureAndHold)
    PYTHON_ENUM_END(m, PtMarkerGameType)
}
