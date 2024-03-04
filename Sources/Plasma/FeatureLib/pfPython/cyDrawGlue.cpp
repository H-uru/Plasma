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

#include "cyDraw.h"

#include "pyGlueHelpers.h"
#include "pyKey.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptDraw, cyDraw);

PYTHON_DEFAULT_NEW_DEFINITION(ptDraw, cyDraw)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptDraw)

PYTHON_NO_INIT_DEFINITION(ptDraw)

PYTHON_METHOD_DEFINITION(ptDraw, netForce, args)
{
    char forceFlag;
    if (!PyArg_ParseTuple(args, "b", &forceFlag))
    {
        PyErr_SetString(PyExc_TypeError, "netForce requires a boolean argument");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetNetForce(forceFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDraw, enable, args)
{
    char state = 1;
    if (!PyArg_ParseTuple(args, "|b", &state))
    {
        PyErr_SetString(PyExc_TypeError, "enable expects an optional boolean argument");
        PYTHON_RETURN_ERROR;
    }

    self->fThis->EnableT(state != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDraw, disable, Disable)

PYTHON_START_METHODS_TABLE(ptDraw)
    PYTHON_METHOD(ptDraw, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
                "- This is to be used if your Python program is running on only one client\n"
                "Such as a game master, only running on the client that owns a particular object"),
    PYTHON_METHOD(ptDraw, enable, "Params: state=1\nSets the draw enable for the sceneobject attached"),
    PYTHON_BASIC_METHOD(ptDraw, disable, "Disables the draw on the sceneobject attached\n"
                "In other words, makes it invisible"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptDraw, "Plasma Draw class");

// required functions for PyObject interoperability
PyObject *cyDraw::New(PyObject *sender, PyObject *recvr)
{
    ptDraw *newObj = (ptDraw*)ptDraw_type.tp_new(&ptDraw_type, nullptr, nullptr);
    if (sender != nullptr)
    {
        newObj->fThis->SetSender(pyKey::ConvertFrom(sender)->getKey());
    }
    if (recvr != nullptr)
    {
        newObj->fThis->AddRecvr(pyKey::ConvertFrom(recvr)->getKey());
    }
    newObj->fThis->fNetForce = false;

    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptDraw, cyDraw)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptDraw, cyDraw)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyDraw::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptDraw);
    PYTHON_CLASS_IMPORT_END(m);
}
