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
#ifndef _pyGUIControlDynamicText_h_
#define _pyGUIControlDynamicText_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlDynamicText   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlDynamicText
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "pyGUIControl.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyDynamicText;


class pyGUIControlDynamicText : public pyGUIControl
{
protected:
	pyGUIControlDynamicText(): pyGUIControl() {} // for python glue only, do NOT call
	pyGUIControlDynamicText(pyKey& gckey);
	pyGUIControlDynamicText(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlDynamicText);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlDynamicText object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlDynamicText); // converts a PyObject to a pyGUIControlDynamicText (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlDynamicText(pyKey& gckey);

	//specific interface functions
	virtual UInt32	GetNumMaps();
	virtual PyObject* GetMap(UInt32 i); // returns pyDynamicText
};

#endif // _pyGUIControlButton_h_
