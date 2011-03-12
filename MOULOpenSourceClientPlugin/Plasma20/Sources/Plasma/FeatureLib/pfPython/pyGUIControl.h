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
#ifndef _pyGUIControl_h_
#define _pyGUIControl_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControl   - a base wrapper class to provide interface to modifier
//                   attached to a GUIControl (base class for the GUI controls)
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "pyGeometry3.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyGUIDialog;
class pyColor;

class pyGUIControl
{
protected:
	plKey					fGCkey;
	
	pyGUIControl(): fGCkey(nil) {} // only used by python glue, do NOT call
	pyGUIControl(pyKey& gckey);
	pyGUIControl(plKey objkey);
	// copy constructor
	pyGUIControl(const pyGUIControl& other);

public:
	pyGUIControl& operator=(const pyGUIControl& other);
	pyGUIControl& Copy(const pyGUIControl& other);

	void setKey(plKey key) {fGCkey = key;} // only used by python glue, do NOT call

	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptGUIControl);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	static PyObject *New(const pyGUIControl& other);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControl object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControl); // converts a PyObject to a pyGUIControl (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// override the equals to operator
	hsBool operator==(const pyGUIControl &gcobj) const;
	hsBool operator!=(const pyGUIControl &gcobj) const { return !(gcobj == *this);	}

	// getter and setters
	virtual plKey getObjKey();
	virtual PyObject* getObjPyKey(); // returns pyKey

	// interface functions
	virtual UInt32	GetTagID();
	virtual void	SetEnabled( hsBool e );
	virtual void	Enable() { SetEnabled(true); }
	virtual void	Disable() { SetEnabled(false); }
	virtual hsBool	IsEnabled( void );
	virtual void	SetFocused( hsBool e );
	virtual void	Focus() { SetFocused(true); }
	virtual void	UnFocus() { SetFocused(false); }
	virtual hsBool	IsFocused( void );
	virtual void	SetVisible( hsBool vis );
	virtual void	Show() { SetVisible(true); }
	virtual void	Hide() { SetVisible(false); }
	virtual hsBool	IsVisible( void );
	virtual hsBool	IsInteresting( void );
	virtual void	SetNotifyOnInteresting( hsBool state );
	virtual void	Refresh( void );
	virtual void	SetObjectCenter( pyPoint3& pt);
	virtual PyObject* GetObjectCenter(); // returns pyPoint3
	virtual PyObject* GetOwnerDlg( void ); // returns pyGUIDialog

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

};

#endif // _pyGUIControl_h_
