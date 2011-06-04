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
#ifndef _pyGUIDialog_h_
#define _pyGUIDialog_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIDialog   - a wrapper class to provide interface to modifier
//                   attached to a GUIDialog
//
//////////////////////////////////////////////////////////////////////

#include "hsStlUtils.h"
#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pyColor;

class pyGUIDialog
{
private:
	plKey					fGCkey;
	
protected:
	pyGUIDialog(pyKey& gckey);
	pyGUIDialog(plKey objkey);
	pyGUIDialog();

public:

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIDialog);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIDialog object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIDialog); // converts a PyObject to a pyGUIDialog (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);

	static hsBool IsGUIDialog(pyKey& gckey);

	void setKey(plKey key) {fGCkey = key;} // used by python glue, do NOT call

	enum			// these enums are used in Python so they have to match PlasmaTypes.py
	{
		kDialog=1,
		kButton=2,
		kDraggable=3,
		kListBox=4,
		kTextBox=5,
		kEditBox=6,
		kUpDownPair=7,
		kKnob=8,
		kDragBar=9,
		kCheckBox=10,
		kRadioGroup=11,
		kDynamicText=12,
		kMultiLineEdit=13,
		kPopUpMenu=14,
		kClickMap=15,
	};
	static UInt32 WhatControlType(pyKey& gckey);
	static void GUICursorOff();
	static void GUICursorOn();
	static void GUICursorDimmed();

	// override the equals to operator
	hsBool operator==(const pyGUIDialog &gdobj) const;
	hsBool operator!=(const pyGUIDialog &gdobj) const { return !(gdobj == *this);	}

	// getter and setters
	virtual plKey getObjKey();
	virtual PyObject* getObjPyKey(); // returns pyKey

	// interface functions
	virtual UInt32	GetTagID();

	virtual void	SetEnabled( hsBool e );
	virtual void	Enable() { SetEnabled(true); }
	virtual void	Disable() { SetEnabled(false); }
	virtual hsBool		IsEnabled( void );
	virtual const char	*GetName( void );
	virtual UInt32		GetVersion(void);

	virtual UInt32		GetNumControls( void );
	virtual PyObject*	GetControl( UInt32 idx ); // returns pyKey
	virtual void		SetFocus( pyKey& gcKey );
	virtual void		NoFocus( );
	virtual void		Show( void );
	virtual void		ShowNoReset( void );
	virtual void		Hide( void );
	virtual PyObject*	GetControlFromTag( UInt32 tagID );  // returns pyKey

	// get color schemes
	virtual PyObject*	GetForeColor(); // returns pyColor
	virtual PyObject*	GetSelColor(); // returns pyColor
	virtual PyObject*	GetBackColor(); // returns pyColor
	virtual PyObject*	GetBackSelColor(); // returns pyColor
	virtual UInt32		GetFontSize();
	// set color scheme
	virtual void		SetForeColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetBackColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetBackSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetFontSize(UInt32 fontsize);

	virtual void		UpdateAllBounds( void );
	virtual	void		RefreshAllControls( void );
};

#endif // _pyGUIDialog_h_
