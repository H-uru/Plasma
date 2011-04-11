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
#ifndef _pyGUIControlValue_h_
#define _pyGUIControlValue_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlValue   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlValue (such as GUIKnob, GUIUpDownPair)
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "pyGUIControl.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyGUIControlValue : public pyGUIControl
{
protected:
	pyGUIControlValue(): pyGUIControl() {} // for python glue only, do NOT call
	pyGUIControlValue(pyKey& gckey);
	pyGUIControlValue(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE;
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlValue);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlValue object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlValue); // converts a PyObject to a pyGUIControlValue (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlValue(pyKey& gckey);

	virtual hsScalar	GetValue();
	virtual void		SetValue( hsScalar v );
	virtual hsScalar	GetMin( void );
	virtual hsScalar	GetMax( void );
	virtual hsScalar	GetStep( void );
	virtual void		SetRange( hsScalar min, hsScalar max );
	virtual void		SetStep( hsScalar step );
};

class pyGUIControlKnob : public pyGUIControlValue
{
protected:
	pyGUIControlKnob(): pyGUIControlValue() {} // for python glue only, do NOT call
	pyGUIControlKnob(pyKey& gckey);
	pyGUIControlKnob(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlKnob);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlKnob object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlKnob); // converts a PyObject to a pyGUIControlKnob (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlKnob(pyKey& gckey);
};

class pyGUIControlUpDownPair : public pyGUIControlValue
{
protected:
	pyGUIControlUpDownPair(): pyGUIControlValue() {} // for python glue only, do NOT call
	pyGUIControlUpDownPair(pyKey& gckey);
	pyGUIControlUpDownPair(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlUpDownPair);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlUpDownPair object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlUpDownPair); // converts a PyObject to a pyGUIControlUpDownPair (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlUpDownPair(pyKey& gckey);
};

class pyGUIControlProgress : public pyGUIControlValue
{
protected:
	pyGUIControlProgress(): pyGUIControlValue() {} // for python glue only, do NOT call
	pyGUIControlProgress(pyKey& gckey);
	pyGUIControlProgress(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlProgress);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlProgress object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlProgress); // converts a PyObject to a pyGUIControlProgress (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlProgress(pyKey& gckey);
	
	void AnimateToPercentage(float percent);
};

#endif // _pyGUIControlValue_h_
