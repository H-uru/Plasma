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

#include "pyGmBlueSpiral.h"

#include "plPythonConvert.h"
#include "pyGameCli.h"
#include "pyGlueHelpers.h"

// ===========================================================================

PYTHON_CLASS_DEFINITION(ptGmBlueSpiral, pyGmBlueSpiral);

PYTHON_DEFAULT_NEW_DEFINITION(ptGmBlueSpiral, pyGmBlueSpiral)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGmBlueSpiral)

PYTHON_NO_INIT_DEFINITION(ptGmBlueSpiral)

PYTHON_METHOD_DEFINITION_NOARGS(ptGmBlueSpiral, startGame)
{
    self->fThis->StartGame();
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGmBlueSpiral, hitCloth, args)
{
    unsigned char cloth;
    if (!PyArg_ParseTuple(args, "b", &cloth)) {
        PyErr_SetString(PyExc_TypeError, "hitCloth expects an unsigned byte");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->HitCloth(cloth);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC(ptGmBlueSpiral, join, args)
{
    PyObject* handler;
    unsigned int tableID;
    if (!PyArg_ParseTuple(args, "On", &handler, &tableID)) {
        PyErr_SetString(PyExc_TypeError, "ptGmBlueSpiral.join() expects an object and an unsigned int");
        PYTHON_RETURN_ERROR;
    }

    pyGmBlueSpiral::Join(handler, tableID);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_STATIC_NOARGS(ptGmBlueSpiral, isSupported)
{
    return plPython::ConvertFrom(self->fThis->IsSupported());
}

PYTHON_START_METHODS_TABLE(ptGmBlueSpiral)
    PYTHON_METHOD_NOARGS(ptGmBlueSpiral, startGame, "Type: () -> None\nRequest for the server to start the game timer."),
    PYTHON_METHOD(ptGmBlueSpiral, hitCloth, "Type: (cloth: int) -> None\nRequest for the server to hit a specific cloth index and validate the correct sequence of cloth inputs."),
    PYTHON_METHOD_STATIC(ptGmBlueSpiral, join, "Type: (handler: Any, tableID: int) -> None\nJoin a common blue spiral game in the current Age."),
    PYTHON_METHOD_STATIC_NOARGS(ptGmBlueSpiral, isSupported, "Type: () -> bool\nChecks for the presence of a server-side blue spiral game manager."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGmBlueSpiral, pyGameCli, "Legacy blue spiral game client.");

// required functions for PyObject interoperability
PYTHON_CLASS_GMCLI_NEW_IMPL(ptGmBlueSpiral, pyGmBlueSpiral, pfGmBlueSpiral)
PYTHON_CLASS_CHECK_IMPL(ptGmBlueSpiral, pyGmBlueSpiral)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGmBlueSpiral, pyGmBlueSpiral)

// ===========================================================================

void pyGmBlueSpiral::AddPlasmaGameClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGmBlueSpiral);
    PYTHON_CLASS_IMPORT_END(m);
}
