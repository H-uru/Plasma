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
#ifndef _pyGUIControlMultiLineEdit_h_
#define _pyGUIControlMultiLineEdit_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlMultiLineEdit   - a wrapper class to provide interface to modifier
//                   attached to a GUIControlMultiLineEdit
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "pyGUIControl.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyColor;

class pyGUIControlMultiLineEdit : public pyGUIControl
{
protected:
	pyGUIControlMultiLineEdit(): pyGUIControl() {} // used by python glue, do NOT call
	pyGUIControlMultiLineEdit(pyKey& gckey);
	pyGUIControlMultiLineEdit(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlMultiLineEdit);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlMultiLineEdit object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlMultiLineEdit); // converts a PyObject to a pyGUIControlMultiLineEdit (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaConstantsClasses(PyObject *m);

	static hsBool IsGUIControlMultiLineEdit(pyKey& gckey);

	virtual void	Clickable( void );
	virtual void	Unclickable( void );
	virtual void	SetScrollPosition( Int32 topLine );
	virtual void	MoveCursor( Int32 dir );
	virtual void	ClearBuffer( void );
	virtual void	SetText( const char *asciiText );
	virtual void	SetTextW( const wchar_t *asciiText );
	virtual const char* GetText( void );		// returns a python string object
	virtual const wchar_t* GetTextW( void );
	virtual void	SetEncodedBuffer( PyObject* buffer_object );
	virtual void	SetEncodedBufferW( PyObject* buffer_object );
	virtual const char* GetEncodedBuffer();
	virtual const wchar_t* GetEncodedBufferW();
	virtual UInt32	GetBufferSize();
	
	virtual void	SetBufferLimit(Int32 limit);
	virtual Int32	GetBufferLimit();

	virtual void	InsertChar( char c );
	virtual void	InsertCharW( wchar_t c );
	virtual void	InsertString( const char *string );
	virtual void	InsertStringW( const wchar_t *string );
	virtual void	InsertColor( pyColor& color );
	virtual void	InsertStyle( UInt8 fontStyle );
	virtual void	DeleteChar( void );

	virtual void	Lock( void );
	virtual void	Unlock( void );
	virtual hsBool	IsLocked( void );

	virtual void	EnableScrollControl();
	virtual void	DisableScrollControl();

	virtual void	DeleteLinesFromTop( int lines );

	virtual UInt32	GetFontSize();
	virtual void	SetFontSize( UInt32 fontsize );
};

#endif // _pyGUIControlMultiLineEdit_h_
