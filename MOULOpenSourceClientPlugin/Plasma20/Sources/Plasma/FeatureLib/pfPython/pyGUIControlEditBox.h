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
#ifndef _pyGUIControlEditBox_h_
#define _pyGUIControlEditBox_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlEditBox   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlEditBox
//
//////////////////////////////////////////////////////////////////////

#include "hsStlUtils.h"
#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyGUIControl.h"

class pyColor;

class pyGUIControlEditBox : public pyGUIControl
{
protected:
	pyGUIControlEditBox(): pyGUIControl() {} // for python glue only, do NOT call
	pyGUIControlEditBox(pyKey& gckey);
	pyGUIControlEditBox(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlEditBox);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlEditBox object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlEditBox); // converts a PyObject to a pyGUIControlEditBox (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlEditBox(pyKey& gckey);

	virtual void	SetBufferSize( UInt32 size );
	virtual std::string GetBuffer( void );
	virtual std::wstring GetBufferW( void );
	virtual void	ClearBuffer( void );
	virtual void	SetText( const char *str );
	virtual void	SetTextW( const wchar_t *str );
	virtual void	SetCursorToHome(void);
	virtual void	SetCursorToEnd(void);
	virtual void	SetColor(pyColor& forecolor, pyColor& backcolor);
	virtual void	SetSelColor(pyColor& forecolor, pyColor& backcolor);

	virtual hsBool	WasEscaped();

	virtual void    SetSpecialCaptureKeyMode(hsBool state);
	virtual UInt32  GetLastKeyCaptured();
	virtual UInt32  GetLastModifiersCaptured();
	virtual void    SetLastKeyCapture(UInt32 key, UInt32 modifiers);

	virtual void	SetChatMode(hsBool state);

};

#endif // _pyGUIControlEditBox_h_
