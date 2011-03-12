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
#include "../pfGameGUIMgr/pfGUIDialogMod.h"

#include "pyGUIDialog.h"

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
#include "pyGUIPopUpMenu.h"
#include "pyGUIControlClickMap.h"

// specific value controls
#include "../pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "../pfGameGUIMgr/pfGUIUpDownPairMod.h"

#include "../plInputCore/plInputInterface.h"

pyGUIDialog::pyGUIDialog(pyKey& gckey)
{
	fGCkey = gckey.getKey();
}

pyGUIDialog::pyGUIDialog(plKey objkey)
{
	fGCkey = objkey;
}

pyGUIDialog::pyGUIDialog()
{
	fGCkey = nil;
}

hsBool pyGUIDialog::IsGUIDialog(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIDialogMod::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
		return true;
	return false;
}


UInt32 pyGUIDialog::WhatControlType(pyKey& gckey)
{
	// Do the pop-up menu test first, since it's derived from dialog
	if ( pyGUIPopUpMenu::IsGUIPopUpMenu(gckey) )
		return kPopUpMenu;
	else if ( pyGUIDialog::IsGUIDialog(gckey) )
		return kDialog;
	else if ( pyGUIControlButton::IsGUIControlButton(gckey) )
		return kButton;
	else if ( pyGUIControlCheckBox::IsGUIControlCheckBox(gckey) )
		return kCheckBox;
	else if ( pyGUIControlEditBox::IsGUIControlEditBox(gckey) )
		return kEditBox;
	else if ( pyGUIControlListBox::IsGUIControlListBox(gckey) )
		return kListBox;
	else if ( pyGUIControlRadioGroup::IsGUIControlRadioGroup(gckey) )
		return kRadioGroup;
	else if ( pyGUIControlTextBox::IsGUIControlTextBox(gckey) )
		return kTextBox;
	else if ( pyGUIControlValue::IsGUIControlValue(gckey) )
	{
		// then see what kind of value control it is
		if ( pfGUIKnobCtrl::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
			return kKnob;
		else if ( pfGUIUpDownPairMod::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
			return kUpDownPair;
		else
			return 0;
	}
	else if ( pyGUIControlDynamicText::IsGUIControlDynamicText( gckey ) )
		return kDynamicText;
	else if ( pyGUIControlMultiLineEdit::IsGUIControlMultiLineEdit( gckey ) )
		return kMultiLineEdit;
	else if ( pyGUIControlClickMap::IsGUIControlClickMap( gckey ) )
		return kClickMap;
	else
		return 0;
}


// override the equals to operator
hsBool pyGUIDialog::operator==(const pyGUIDialog &gcobj) const
{
	plKey theirs = ((pyGUIDialog&)gcobj).getObjKey();
	if ( fGCkey == nil && theirs == nil )
		return true;
	else if ( fGCkey != nil && theirs != nil )
		return (fGCkey->GetUoid()==theirs->GetUoid());
	else
		return false;
}


// getter and setters
plKey pyGUIDialog::getObjKey()
{
	return fGCkey;
}


PyObject* pyGUIDialog::getObjPyKey()
{
	// create a pyKey object that Python will manage
	return pyKey::New(fGCkey);
}


// interface functions
UInt32	pyGUIDialog::GetTagID()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			return pdmod->GetTagID();
	}
	return 0;
}


void pyGUIDialog::SetEnabled( hsBool e )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			pdmod->SetEnabled(e);
	}
}

hsBool pyGUIDialog::IsEnabled( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			return pdmod->IsEnabled();
	}
	return false;
}

const char* pyGUIDialog::GetName( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			return pdmod->GetName();
	}
	return "";
}


UInt32 pyGUIDialog::GetVersion(void)
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			return pdmod->GetVersion();
	}
	return 0;
}


UInt32 pyGUIDialog::GetNumControls( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			return pdmod->GetNumControls();
	}
	return 0;
}

PyObject* pyGUIDialog::GetControl( UInt32 idx )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIControlMod* pcontrolmod = pdmod->GetControl(idx);
			if ( pcontrolmod )
				return pyKey::New(pcontrolmod->GetKey());
		}
	}

	// if we got here then there must have been an error
	char errmsg[256];
	sprintf(errmsg,"Index %d not found in GUIDialog %s",idx,GetName());
	PyErr_SetString(PyExc_KeyError, errmsg);
	PYTHON_RETURN_ERROR;
}

void pyGUIDialog::SetFocus( pyKey& gcKey )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIControlMod* pcontrolmod = pfGUIControlMod::ConvertNoRef(gcKey.getKey()->ObjectIsLoaded());
			if ( pcontrolmod )
				pdmod->SetFocus(pcontrolmod);
		}
	}
}

void pyGUIDialog::Show( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pdmod->Show();
			pdmod->RefreshAllControls();
		}
	}
}

void pyGUIDialog::ShowNoReset( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			pdmod->ShowNoReset();
	}
}

void pyGUIDialog::Hide( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			pdmod->Hide();
	}
}

PyObject* pyGUIDialog::GetControlFromTag( UInt32 tagID )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIControlMod* pcontrolmod = pdmod->GetControlFromTag(tagID);
			if ( pcontrolmod )
				return pyKey::New(pcontrolmod->GetKey());
		}
	}

	// if we got here then there must have been an error
	char errmsg[256];
	sprintf(errmsg,"TagID %d not found in GUIDialog %s",tagID,GetName());
	PyErr_SetString(PyExc_KeyError, errmsg);
	PYTHON_RETURN_ERROR;
}

	// get color schemes
PyObject* pyGUIDialog::GetForeColor()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fForeColor.r,color->fForeColor.g,color->fForeColor.b,color->fForeColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIDialog::GetSelColor()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fSelForeColor.r,color->fSelForeColor.g,color->fSelForeColor.b,color->fSelForeColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIDialog::GetBackColor()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fBackColor.r,color->fBackColor.g,color->fBackColor.b,color->fBackColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIDialog::GetBackSelColor()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fSelBackColor.r,color->fSelBackColor.g,color->fSelBackColor.b,color->fSelBackColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

UInt32 pyGUIDialog::GetFontSize()
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return color->fFontSize;
		}
	}
	// create a pyColor that will be managed by Python
	return 0;
}


	// set color scheme
void pyGUIDialog::SetForeColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			if ( r >= 0.0 && r <= 1.0 )
				color->fForeColor.r = r;
			if ( g >= 0.0 && g <= 1.0 )
				color->fForeColor.g = g;
			if ( b >= 0.0 && g <= 1.0 )
				color->fForeColor.b = b;
			if ( a >= 0.0 && g <= 1.0 )
				color->fForeColor.a = a;
		}
	}
}

void pyGUIDialog::SetSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			if ( r >= 0.0 && r <= 1.0 )
				color->fSelForeColor.r = r;
			if ( g >= 0.0 && g <= 1.0 )
				color->fSelForeColor.g = g;
			if ( b >= 0.0 && g <= 1.0 )
				color->fSelForeColor.b = b;
			if ( a >= 0.0 && g <= 1.0 )
				color->fSelForeColor.a = a;
		}
	}
}

void pyGUIDialog::SetBackColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			if ( r >= 0.0 && r <= 1.0 )
				color->fBackColor.r = r;
			if ( g >= 0.0 && g <= 1.0 )
				color->fBackColor.g = g;
			if ( b >= 0.0 && g <= 1.0 )
				color->fBackColor.b = b;
			if ( a >= 0.0 && g <= 1.0 )
				color->fBackColor.a = a;
		}
	}
}

void pyGUIDialog::SetBackSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			if ( r >= 0.0 && r <= 1.0 )
				color->fSelBackColor.r = r;
			if ( g >= 0.0 && g <= 1.0 )
				color->fSelBackColor.g = g;
			if ( b >= 0.0 && g <= 1.0 )
				color->fSelBackColor.b = b;
			if ( a >= 0.0 && g <= 1.0 )
				color->fSelBackColor.a = a;
		}
	}
}


void pyGUIDialog::SetFontSize(UInt32 fontsize)
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			color->fFontSize = (UInt8)fontsize;
		}
	}
}

void pyGUIDialog::UpdateAllBounds( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			pdmod->UpdateAllBounds();
	}
}

void pyGUIDialog::RefreshAllControls( void )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
			pdmod->RefreshAllControls();
	}
}

void pyGUIDialog::NoFocus( )
{
	if ( fGCkey )
	{
		pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pdmod->SetFocus(nil);
		}
	}
}

void pyGUIDialog::GUICursorOff()
{
	if ( pfGameGUIMgr::GetInstance() )
		pfGameGUIMgr::GetInstance()->SetCursorOpacity(0.0f);
}

void pyGUIDialog::GUICursorOn()
{
	if ( pfGameGUIMgr::GetInstance() )
		pfGameGUIMgr::GetInstance()->SetCursorOpacity(1.0f);
}

void pyGUIDialog::GUICursorDimmed()
{
	if ( pfGameGUIMgr::GetInstance() )
		pfGameGUIMgr::GetInstance()->SetCursorOpacity(0.4f);
}
