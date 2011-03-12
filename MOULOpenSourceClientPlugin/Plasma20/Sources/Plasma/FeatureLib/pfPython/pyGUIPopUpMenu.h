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
#ifndef _pyGUIPopUpMenu_h_
#define _pyGUIPopUpMenu_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIPopUpMenu   - a wrapper class to provide interface to modifier
//                   attached to a GUI Pop-Up Menu
//
//////////////////////////////////////////////////////////////////////

#include "../pfGameGUIMgr/pfGUIPopUpMenu.h"

#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyColor;

class pyGUIPopUpMenu
{
private:
	plKey					fGCkey;
	
	pfGUIPopUpMenu			*fBuiltMenu;

protected:
	pyGUIPopUpMenu(pyKey& gckey);
	pyGUIPopUpMenu(plKey objkey);
	pyGUIPopUpMenu();
	// For creating new menus on the fly
	pyGUIPopUpMenu( const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc );
	pyGUIPopUpMenu( const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY );

public:
	virtual ~pyGUIPopUpMenu();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIPopUpMenu);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	static PyObject *New(const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc);
	static PyObject *New(const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIPopUpMenu object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIPopUpMenu); // converts a PyObject to a pyGUIPopUpMenu (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// these three are for the python glue only, do NOT call
	void setup(plKey objkey);
	void setup(const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc);
	void setup(const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY);

	static hsBool IsGUIPopUpMenu(pyKey& gckey);

	// override the equals to operator
	hsBool operator==(const pyGUIPopUpMenu &gdobj) const;
	hsBool operator!=(const pyGUIPopUpMenu &gdobj) const { return !(gdobj == *this);	}

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

	virtual void		Show( void );
	virtual void		Hide( void );

	// get color schemes
	virtual PyObject*	GetForeColor(); // returns pyColor
	virtual PyObject*	GetSelColor(); // returns pyColor
	virtual PyObject*	GetBackColor(); // returns pyColor
	virtual PyObject*	GetBackSelColor(); // returns pyColor
	// set color scheme
	virtual void		SetForeColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetBackColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );
	virtual void		SetBackSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a );

	// Menu item functions
	virtual void		AddConsoleCmdItem( const char *name, const char *consoleCmd );
	virtual void		AddConsoleCmdItemW( std::wstring name, const char *consoleCmd );
	virtual void		AddNotifyItem( const char *name );
	virtual void		AddNotifyItemW( std::wstring name );
	virtual void		AddSubMenuItem( const char *name, pyGUIPopUpMenu &subMenu );
	virtual void		AddSubMenuItemW( std::wstring name, pyGUIPopUpMenu &subMenu );
};

#endif // _pyGUIPopUpMenu_h_
