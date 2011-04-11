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
#ifndef _pyGUIControlDragBar_h_
#define _pyGUIControlDragBar_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlDragBar   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlDragBar
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyGUIControl.h"

class pyGUIControlDragBar : public pyGUIControl
{
protected:
	pyGUIControlDragBar(): pyGUIControl() {} // for python glue only, do NOT call
	pyGUIControlDragBar(pyKey& gckey);
	pyGUIControlDragBar(plKey objkey);

public:
	~pyGUIControlDragBar();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlDragBar);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlDragBar object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlDragBar); // converts a PyObject to a pyGUIControlDragBar (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlDragBar(pyKey& gckey);

	virtual void	Anchor( void );
	virtual void	Unanchor( void );
	virtual hsBool	IsAnchored();

};

#endif // _pyGUIControlDragBar_h_
