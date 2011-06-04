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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"
#include "pyColor.h"
#include "cyPythonInterface.h"

#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUIPopUpMenu.h"

#include "pyGUIPopUpMenu.h"

// the rest of the controls
#include "pyGUIControlButton.h"
#include "pyGUIControlCheckBox.h"
#include "pyGUIControlEditBox.h"
#include "pyGUIControlListBox.h"
#include "pyGUIControlRadioGroup.h"
#include "pyGUIControlTextBox.h"
#include "pyGUIControlValue.h"
#include "pyGUIControlDynamicText.h"
#include "pyGUIControlMultiLineEdit.h"

#include "../pfGameGUIMgr/pfGUIControlHandlers.h"

#define kGetMenuPtr( ret ) if( fGCkey == nil ) return ret; \
			pfGUIPopUpMenu *menu = pfGUIPopUpMenu::ConvertNoRef( fGCkey->ObjectIsLoaded() ); \
			if( menu == nil ) return ret;


pyGUIPopUpMenu::pyGUIPopUpMenu(pyKey& gckey)
{
	fGCkey = gckey.getKey();
	fBuiltMenu = nil;
}

pyGUIPopUpMenu::pyGUIPopUpMenu(plKey objkey)
{
	fGCkey = objkey;
	fBuiltMenu = nil;
}

pyGUIPopUpMenu::pyGUIPopUpMenu()
{
	fGCkey = nil;
	fBuiltMenu = nil;
}

pyGUIPopUpMenu::pyGUIPopUpMenu( const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc/* = plLocation::kGlobalFixedLoc */)
{
	fBuiltMenu = pfGUIPopUpMenu::Build( name, nil, screenOriginX, screenOriginY, destLoc );
	if( fBuiltMenu != nil )
	{
		fGCkey = fBuiltMenu->GetKey();
		fGCkey->RefObject();
	}
	else
		fGCkey = nil;
}

pyGUIPopUpMenu::pyGUIPopUpMenu( const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY )
{
	pfGUIPopUpMenu *parentMenu = pfGUIPopUpMenu::ConvertNoRef( parent.fGCkey->ObjectIsLoaded() );
	
	const plLocation &parentLoc = ( parentMenu != nil ) ? parentMenu->GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc;

	fBuiltMenu = pfGUIPopUpMenu::Build( name, parentMenu, screenOriginX, screenOriginY, parentLoc );
	if( fBuiltMenu != nil )
	{
		fGCkey = fBuiltMenu->GetKey();
		fGCkey->RefObject();
	}
	else
		fGCkey = nil;
}

pyGUIPopUpMenu::~pyGUIPopUpMenu()
{
	// Optimistic, I know, to assume the destructor will be called, but still...
	if( fBuiltMenu != nil )
	{
		fBuiltMenu->GetKey()->UnRefObject();
		fBuiltMenu = nil;
	}
}

void pyGUIPopUpMenu::setup(plKey objkey)
{
	// kill any previous menu
	if( fBuiltMenu != nil )
	{
		fBuiltMenu->GetKey()->UnRefObject();
		fBuiltMenu = nil;
	}

	// setup the new one
	fGCkey = objkey;
}

void pyGUIPopUpMenu::setup(const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc /* = plLocation::kGlobalFixedLoc */)
{
	// kill any previous menu
	if( fBuiltMenu != nil )
	{
		fBuiltMenu->GetKey()->UnRefObject();
		fBuiltMenu = nil;
	}

	// setup the new one
	fBuiltMenu = pfGUIPopUpMenu::Build( name, nil, screenOriginX, screenOriginY, destLoc );
	if( fBuiltMenu != nil )
	{
		fGCkey = fBuiltMenu->GetKey();
		fGCkey->RefObject();
	}
	else
		fGCkey = nil;
}

void pyGUIPopUpMenu::setup(const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY)
{
	// kill any previous menu
	if( fBuiltMenu != nil )
	{
		fBuiltMenu->GetKey()->UnRefObject();
		fBuiltMenu = nil;
	}

	// setup the new one
	pfGUIPopUpMenu *parentMenu = pfGUIPopUpMenu::ConvertNoRef( parent.fGCkey->ObjectIsLoaded() );

	const plLocation &parentLoc = ( parentMenu != nil ) ? parentMenu->GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc;

	fBuiltMenu = pfGUIPopUpMenu::Build( name, parentMenu, screenOriginX, screenOriginY, parentLoc );
	if( fBuiltMenu != nil )
	{
		fGCkey = fBuiltMenu->GetKey();
		fGCkey->RefObject();
	}
	else
		fGCkey = nil;
}

hsBool pyGUIPopUpMenu::IsGUIPopUpMenu(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIPopUpMenu::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
		return true;
	return false;
}


// override the equals to operator
hsBool pyGUIPopUpMenu::operator==(const pyGUIPopUpMenu &gcobj) const
{
	plKey theirs = ((pyGUIPopUpMenu&)gcobj).getObjKey();
	if ( fGCkey == nil && theirs == nil )
		return true;
	else if ( fGCkey != nil && theirs != nil )
		return (fGCkey->GetUoid()==theirs->GetUoid());
	else
		return false;
}


// getter and setters
plKey pyGUIPopUpMenu::getObjKey()
{
	return fGCkey;
}


PyObject* pyGUIPopUpMenu::getObjPyKey()
{
	// create a pyKey object that Python will manage
	return pyKey::New(fGCkey);
}


// interface functions
UInt32	pyGUIPopUpMenu::GetTagID()
{
	kGetMenuPtr( 0 );
	return menu->GetTagID();
}


void pyGUIPopUpMenu::SetEnabled( hsBool e )
{
	kGetMenuPtr( ; );
	menu->SetEnabled(e);
}

hsBool pyGUIPopUpMenu::IsEnabled( void )
{
	kGetMenuPtr( false );
	return menu->IsEnabled();
}

const char* pyGUIPopUpMenu::GetName( void )
{
	kGetMenuPtr( "" );
	return menu->GetName();
}


UInt32 pyGUIPopUpMenu::GetVersion(void)
{
	kGetMenuPtr( 0 );
	return menu->GetVersion();
}


void pyGUIPopUpMenu::Show( void )
{
	kGetMenuPtr( ; );
	( (pfGUIDialogMod *)menu )->Show();
}

void pyGUIPopUpMenu::Hide( void )
{
	kGetMenuPtr( ; );
	menu->Hide();
}

	// get color schemes
PyObject* pyGUIPopUpMenu::GetForeColor()
{
	kGetMenuPtr( nil );

	pfGUIColorScheme* color = menu->GetColorScheme();
	return pyColor::New(color->fForeColor.r,color->fForeColor.g,color->fForeColor.b,color->fForeColor.a);
}

PyObject* pyGUIPopUpMenu::GetSelColor()
{
	kGetMenuPtr( nil );

	pfGUIColorScheme* color = menu->GetColorScheme();
	return pyColor::New(color->fSelForeColor.r,color->fSelForeColor.g,color->fSelForeColor.b,color->fSelForeColor.a);
}

PyObject* pyGUIPopUpMenu::GetBackColor()
{
	kGetMenuPtr( nil );

	pfGUIColorScheme* color = menu->GetColorScheme();
	return pyColor::New(color->fBackColor.r,color->fBackColor.g,color->fBackColor.b,color->fBackColor.a);
}

PyObject* pyGUIPopUpMenu::GetBackSelColor()
{
	kGetMenuPtr( nil );

	pfGUIColorScheme* color = menu->GetColorScheme();
	return pyColor::New(color->fSelBackColor.r,color->fSelBackColor.g,color->fSelBackColor.b,color->fSelBackColor.a);
}

	// set color scheme
void pyGUIPopUpMenu::SetForeColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	kGetMenuPtr( ; );

	pfGUIColorScheme* color = menu->GetColorScheme();
	if ( r >= 0.0 && r <= 1.0 )
		color->fForeColor.r = r;
	if ( g >= 0.0 && g <= 1.0 )
		color->fForeColor.g = g;
	if ( b >= 0.0 && g <= 1.0 )
		color->fForeColor.b = b;
	if ( a >= 0.0 && g <= 1.0 )
		color->fForeColor.a = a;
}

void pyGUIPopUpMenu::SetSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	kGetMenuPtr( ; );

	pfGUIColorScheme* color = menu->GetColorScheme();
	if ( r >= 0.0 && r <= 1.0 )
		color->fSelForeColor.r = r;
	if ( g >= 0.0 && g <= 1.0 )
		color->fSelForeColor.g = g;
	if ( b >= 0.0 && g <= 1.0 )
		color->fSelForeColor.b = b;
	if ( a >= 0.0 && g <= 1.0 )
		color->fSelForeColor.a = a;
}

void pyGUIPopUpMenu::SetBackColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	kGetMenuPtr( ; );

	pfGUIColorScheme* color = menu->GetColorScheme();
	if ( r >= 0.0 && r <= 1.0 )
		color->fBackColor.r = r;
	if ( g >= 0.0 && g <= 1.0 )
		color->fBackColor.g = g;
	if ( b >= 0.0 && g <= 1.0 )
		color->fBackColor.b = b;
	if ( a >= 0.0 && g <= 1.0 )
		color->fBackColor.a = a;
}

void pyGUIPopUpMenu::SetBackSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	kGetMenuPtr( ; );

	pfGUIColorScheme* color = menu->GetColorScheme();
	if ( r >= 0.0 && r <= 1.0 )
		color->fSelBackColor.r = r;
	if ( g >= 0.0 && g <= 1.0 )
		color->fSelBackColor.g = g;
	if ( b >= 0.0 && g <= 1.0 )
		color->fSelBackColor.b = b;
	if ( a >= 0.0 && g <= 1.0 )
		color->fSelBackColor.a = a;
}

void	pyGUIPopUpMenu::AddConsoleCmdItem( const char *name, const char *consoleCmd )
{
	wchar_t *wName = hsStringToWString(name);
	AddConsoleCmdItemW(wName,consoleCmd);
	delete [] wName;
}

void	pyGUIPopUpMenu::AddConsoleCmdItemW( std::wstring name, const char *consoleCmd )
{
	kGetMenuPtr( ; );
	menu->AddItem( name.c_str(), TRACKED_NEW pfGUIConsoleCmdProc( consoleCmd ), nil );
}

void	pyGUIPopUpMenu::AddNotifyItem( const char *name )
{
	wchar_t *wName = hsStringToWString(name);
	AddNotifyItemW(wName);
	delete [] wName;
}

void	pyGUIPopUpMenu::AddNotifyItemW( std::wstring name )
{
	kGetMenuPtr( ; );
	menu->AddItem( name.c_str(), (pfGUICtrlProcObject *)( menu->GetHandler() ), nil );
}

void	pyGUIPopUpMenu::AddSubMenuItem( const char *name, pyGUIPopUpMenu &subMenu )
{
	wchar_t *wName = hsStringToWString(name);
	AddSubMenuItemW(wName,subMenu);
	delete [] wName;
}

void	pyGUIPopUpMenu::AddSubMenuItemW( std::wstring name, pyGUIPopUpMenu &subMenu )
{
	kGetMenuPtr( ; );

	if( subMenu.fGCkey == nil )
		return;
	pfGUIPopUpMenu *subM = pfGUIPopUpMenu::ConvertNoRef( subMenu.fGCkey->ObjectIsLoaded() );
	if( subM == nil )
		return;

	menu->AddItem( name.c_str(), nil, subM );
}