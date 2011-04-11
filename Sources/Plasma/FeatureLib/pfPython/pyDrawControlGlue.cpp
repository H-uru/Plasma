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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "pyDrawControl.h"
#include "pyEnum.h"

#include <python.h>

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

void pyDrawControl::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtSetGamma2);
	PYTHON_GLOBAL_METHOD(methods, PtSetShadowVisDistance);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetShadowVisDistance);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableShadows);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableShadows);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsShadowsEnabled);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtCanShadowCast);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableRenderScene);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableRenderScene);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtSetMouseInverted);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtSetMouseUninverted);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsMouseInverted);
	PYTHON_GLOBAL_METHOD(methods, PtSetClickToTurn);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsClickToTurn);
}

/*void pyDrawControl::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtMovieType);
	PYTHON_ENUM_ELEMENT(PtMovieType, kUnknownTypeMovie,	pyMoviePlayer::kUnknownTypeMovie);
	PYTHON_ENUM_ELEMENT(PtMovieType, kBinkMovie,		pyMoviePlayer::kBinkMovie);
	PYTHON_ENUM_END;
}*/