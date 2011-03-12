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
#ifndef _pyGUIControlListBox_h_
#define _pyGUIControlListBox_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlListBox   - a wrapper class to provide interface to modifier
//						attached to a GUIControl (such as Button, ListBox, etc.)
//
//////////////////////////////////////////////////////////////////////

#include "hsTemplates.h"

#include "hsStlUtils.h"
#include "pyKey.h"
#include "pyGUIControl.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pyColor;
class pyImage;

class pfGUIListTreeRoot;
class pyGUIControlListBox : public pyGUIControl
{
private:
	hsTArray<pfGUIListTreeRoot *>	fBuildRoots;

protected:
	pyGUIControlListBox(): pyGUIControl() {} // for python glue, do NOT call
	pyGUIControlListBox(pyKey& gckey);
	pyGUIControlListBox(plKey objkey);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGUIControlListBox);
	static PyObject *New(pyKey& gckey);
	static PyObject *New(plKey objkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlListBox object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlListBox); // converts a PyObject to a pyGUIControlListBox (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	static hsBool IsGUIControlListBox(pyKey& gckey);

	// special case control for the listbox
	// ...this allows the listbox to be used without being selectable
	virtual void	Clickable( void );
	virtual void	Unclickable( void );
	virtual Int32	GetSelection( void );
	virtual void	SetSelection( Int32 item );
	virtual void	Refresh( void );
	virtual void	SetElement( UInt16 idx, const char* text );
	virtual void	SetElementW( UInt16 idx, std::wstring text );
	virtual void	RemoveElement( UInt16 index );
	virtual void	ClearAllElements( void );
	virtual UInt16	GetNumElements( void );
	virtual std::string GetElement( UInt16 idx );
	virtual std::wstring GetElementW( UInt16 idx );
	virtual Int16	AddString( const char *string );
	virtual Int16	AddStringW( std::wstring string );
	virtual Int16	AddImage( pyImage& image, hsBool respectAlpha );
	virtual Int16	AddImageInBox( pyImage& image, UInt32 x, UInt32 y, UInt32 width, UInt32 height, hsBool respectAlpha );
	virtual Int16	AddImageAndSwatchesInBox( pyImage& image, UInt32 x, UInt32 y, UInt32 width, UInt32 height, hsBool respectAlpha,
														pyColor &primary, pyColor &secondary );
	virtual void	SetSwatchSize( UInt32 size );
	virtual void	SetSwatchEdgeOffset( UInt32 size );
	virtual void	SetStringJustify( UInt16 idx, UInt32 justify);
	virtual Int16	FindString( const char *toCompareTo );
	virtual Int16	FindStringW( std::wstring toCompareTo );
	virtual Int16	AddTextWColor( const char *str, pyColor& textcolor, UInt32 inheritalpha);
	virtual Int16	AddTextWColorW( std::wstring str, pyColor& textcolor, UInt32 inheritalpha);
	virtual Int16	AddTextWColorWSize( const char *str, pyColor& textcolor, UInt32 inheritalpha, Int32 fontsize);
	virtual Int16	AddTextWColorWSizeW( std::wstring str, pyColor& textcolor, UInt32 inheritalpha, Int32 fontsize);
	virtual void	Add2TextWColor( const char *str1, pyColor& textcolor1,const char *str2, pyColor& textcolor2, UInt32 inheritalpha);
	virtual void	Add2TextWColorW( std::wstring str1, pyColor& textcolor1, std::wstring str2, pyColor& textcolor2, UInt32 inheritalpha);
	virtual Int16	AddStringInBox( const char *string, UInt32 min_width, UInt32 min_height );
	virtual Int16	AddStringInBoxW( std::wstring string, UInt32 min_width, UInt32 min_height );
	virtual	void	ScrollToBegin( void );
	virtual	void	ScrollToEnd( void );
	virtual void	SetScrollPos( Int32 pos );
	virtual Int32	GetScrollPos( void );
	virtual Int32	GetScrollRange( void );

	virtual	void	LockList( void );
	virtual	void	UnlockList( void );

	// To create tree branches, call AddBranch() with a name, then add elements as usual, including new sub-branches
	// via additional AddBranch() calls. Call CloseBranch() to stop writing elements to that branch.
	void			AddBranch( const char *name, hsBool initiallyOpen );
	void			AddBranchW( std::wstring name, hsBool initiallyOpen );
	void			CloseBranch( void );

	void			RemoveSelection( Int32 item );
	void			AddSelection( Int32 item );
	PyObject*		GetSelectionList();
	PyObject*		GetBranchList();
	
	void			AllowNoSelect();
	void			DisallowNoSelect();

};

#endif // _pyGUIControlListBox_h_
