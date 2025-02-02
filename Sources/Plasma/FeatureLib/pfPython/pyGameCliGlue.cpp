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

#include "pyGameCli.h"

#include "plPythonConvert.h"
#include "pyGlueHelpers.h"

// ===========================================================================

PYTHON_CLASS_DEFINITION(ptGameCli, pyGameCli);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCli, pyGameCli)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCli)

PYTHON_NO_INIT_DEFINITION(ptGameCli)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, leaveGame)
{
    self->fThis->LeaveGame();
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGameCli)
    PYTHON_METHOD_NOARGS(ptGameCli, leaveGame, "Type: () -> None\nExplicitly ask the server to allow us to leave the game."),
PYTHON_END_METHODS_TABLE;

PYTHON_GET_DEFINITION_PROXY(ptGameCli, gameID, GetGameID)
PYTHON_SET_DEFINITION_READONLY(ptGameCli, gameID)

PYTHON_GET_DEFINITION_PROXY(ptGameCli, handler, GetHandler)
PYTHON_SET_DEFINITION_READONLY(ptGameCli, handler)

PYTHON_GET_DEFINITION_PROXY(ptGameCli, isLocallyOwned, IsLocallyOwned)
PYTHON_SET_DEFINITION_READONLY(ptGameCli, isLocallyOwned)

PYTHON_GET_DEFINITION_PROXY(ptGameCli, ownerID, GetOwnerID)
PYTHON_SET_DEFINITION_READONLY(ptGameCli, ownerID)

PYTHON_START_GETSET_TABLE(ptGameCli)
    PYTHON_GETSET(ptGameCli, gameID, "Type: int\nThe ID of the game instance on the server."),
    PYTHON_GETSET(ptGameCli, handler, "Type: Any\nThe game event handler."),
    PYTHON_GETSET(ptGameCli, isLocallyOwned, "Type: bool\nWhether or not we are the owner of this game instance."),
    PYTHON_GETSET(ptGameCli, ownerID, "Type: int\nThe ID of the player who owns this game instance."),
PYTHON_END_GETSET_TABLE;

// Type structure definition
#define ptGameCli_AS_NUMBER       PYTHON_NO_AS_NUMBER
#define ptGameCli_AS_SEQUENCE     PYTHON_NO_AS_SEQUENCE
#define ptGameCli_AS_MAPPING      PYTHON_NO_AS_MAPPING
#define ptGameCli_STR             PYTHON_NO_STR
#define ptGameCli_GETATTRO        PYTHON_NO_GETATTRO
#define ptGameCli_SETATTRO        PYTHON_NO_SETATTRO
#define ptGameCli_RICH_COMPARE    PYTHON_NO_RICH_COMPARE
#define ptGameCli_GETSET          PYTHON_DEFAULT_GETSET(ptGameCli)
#define ptGameCli_BASE            PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptGameCli, "Abstract bass class for legacy game clients.");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGameCli, pyGameCli);

// required functions for PyObject interoperability
PYTHON_CLASS_CHECK_IMPL(ptGameCli, pyGameCli)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCli, pyGameCli)

// ===========================================================================

void pyGameCli::AddPlasmaGameClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameCli);
    PYTHON_CLASS_IMPORT_END(m);
}
