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
#ifndef pyEnum_h
#define pyEnum_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyEnum
//
// PURPOSE: Base class stuff for enumeration support (you don't instance this
//			class
//

#include <python.h>
#include "hsConfig.h"
#include "hsStlUtils.h"
#include "pyGlueHelpers.h"


class pyEnum
{
public:
	static void AddPlasmaConstantsClasses(PyObject *m);
	static void RemovePlasmaConstantsClasses(PyObject *m);
	static void MakeEnum(PyObject *m, const char* name, std::map<std::string, int> values);
};

#endif  // pyEnum_h
