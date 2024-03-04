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

#include "pyDrawControl.h"

#include "pyGlueHelpers.h"

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetGamma2, args, "Params: gamma\nSet the gamma with gamma2 rules")
{
    float gamma;
    if (!PyArg_ParseTuple(args, "f", &gamma))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetGamma2 expects a float");
        PYTHON_RETURN_ERROR;
    }
    pyDrawControl::SetGamma2(gamma);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetShadowVisDistance, args, "Params: distance\nSet the maximum shadow visibility distance")
{
    float distance;
    if (!PyArg_ParseTuple(args, "f", &distance))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetShadowVisDistance expects a float");
        PYTHON_RETURN_ERROR;
    }
    pyDrawControl::SetShadowVisDistance(distance);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetShadowVisDistance, "Returns the maximum shadow visibility distance")
{
    return PyFloat_FromDouble(pyDrawControl::GetShadowVisDistance());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableShadows, pyDrawControl::EnableShadows, "Turns shadows on")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableShadows, pyDrawControl::DisableShadows, "Turns shadows off")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsShadowsEnabled, "Returns whether shadows are currently turned on")
{
    PYTHON_RETURN_BOOL(pyDrawControl::IsShadowsEnabled());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtCanShadowCast, "Can we cast shadows?")
{
    PYTHON_RETURN_BOOL(pyDrawControl::CanShadowCast());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableRenderScene, pyDrawControl::DisableRenderScene, "UNKNOWN")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableRenderScene, pyDrawControl::EnableRenderScene, "UNKNOWN")

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtSetMouseInverted, pyDrawControl::SetMouseInverted, "Inverts the mouse")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtSetMouseUninverted, pyDrawControl::SetMouseUninverted, "Uninverts the mouse")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsMouseInverted, "Is the mouse currently inverted?")
{
    PYTHON_RETURN_BOOL(pyDrawControl::IsMouseInverted());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetClickToTurn, args, "Params: state\nTurns on click-to-turn")
{
    char stateFlag;
    if (!PyArg_ParseTuple(args, "b", &stateFlag))
    {
        PyErr_SetString(PyExc_TypeError, "PtSetClickToTurn expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyDrawControl::SetClickToTurn(stateFlag != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsClickToTurn, "Is click-to-turn on?")
{
    PYTHON_RETURN_BOOL(pyDrawControl::IsClickToTurn());
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void pyDrawControl::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(ptDraw)
        PYTHON_GLOBAL_METHOD(PtSetGamma2)
        PYTHON_GLOBAL_METHOD(PtSetShadowVisDistance)
        PYTHON_GLOBAL_METHOD_NOARGS(PtGetShadowVisDistance)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableShadows)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableShadows)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsShadowsEnabled)
        PYTHON_GLOBAL_METHOD_NOARGS(PtCanShadowCast)
        PYTHON_BASIC_GLOBAL_METHOD(PtDisableRenderScene)
        PYTHON_BASIC_GLOBAL_METHOD(PtEnableRenderScene)
        PYTHON_BASIC_GLOBAL_METHOD(PtSetMouseInverted)
        PYTHON_BASIC_GLOBAL_METHOD(PtSetMouseUninverted)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsMouseInverted)
        PYTHON_GLOBAL_METHOD(PtSetClickToTurn)
        PYTHON_GLOBAL_METHOD_NOARGS(PtIsClickToTurn)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, ptDraw)
}
