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
#ifndef _pyDrawControl_h_
#define _pyDrawControl_h_

//////////////////////////////////////////////////////////////////////
//
// pyDrawControl   - a wrapper class all the draw/pipeline control functions
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"


#include <python.h>
#include "pyGlueHelpers.h"

class pyDrawControl
{
protected:
	pyDrawControl() {};

public:
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	//static void AddPlasmaConstantsClasses(PyObject* m);

	// static python functions
	static void SetGamma2(hsScalar gamma);
	static void SetShadowVisDistance(hsScalar distance);
	static hsScalar GetShadowVisDistance();
	static void EnableShadows();
	static void DisableShadows();
	static hsBool IsShadowsEnabled();
	static hsBool CanShadowCast();

	static void DisableRenderScene();
	static void EnableRenderScene();

	static void SetMouseInverted();
	static void SetMouseUninverted();
	static hsBool IsMouseInverted();

	static void SetClickToTurn(hsBool state);
	static hsBool IsClickToTurn();
};

#endif // _pyDrawControl_h_
