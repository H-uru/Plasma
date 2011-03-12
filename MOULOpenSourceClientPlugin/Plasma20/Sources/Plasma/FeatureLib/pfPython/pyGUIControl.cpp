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

#include "../pfGameGUIMgr/pfGUIControlMod.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"

#include "pyKey.h"
#include "pyGUIControl.h"
#include "pyGUIDialog.h"
#include "pyColor.h"

#include "pyGUIControlCheckBox.h"

pyGUIControl::pyGUIControl(pyKey& gckey)
{
	fGCkey = gckey.getKey();
}

pyGUIControl::pyGUIControl(plKey objkey)
{
	fGCkey = objkey;
}

pyGUIControl& pyGUIControl::operator=(const pyGUIControl& other)
{
	return Copy(other);
}
// copy constructor
pyGUIControl::pyGUIControl(const pyGUIControl& other)
{
	Copy(other);
}

pyGUIControl& pyGUIControl::Copy(const pyGUIControl& other)
{
	fGCkey = other.fGCkey;
	return *this;
}

// override the equals to operator
hsBool pyGUIControl::operator==(const pyGUIControl &gcobj) const
{
	plKey theirs = ((pyGUIControl&)gcobj).getObjKey();
	if ( fGCkey == nil && theirs == nil )
		return true;
	else if ( fGCkey != nil && theirs != nil )
		return (fGCkey->GetUoid()==theirs->GetUoid());
	else
		return false;
}


// getter and setters
plKey pyGUIControl::getObjKey()
{
	return fGCkey;
}


PyObject* pyGUIControl::getObjPyKey()
{
	// create the pyKey that Python will manage
	return pyKey::New(fGCkey);
}


// interface functions
UInt32	pyGUIControl::GetTagID()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->GetTagID();
	}
	return 0;
}


void pyGUIControl::SetEnabled( hsBool e )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->SetEnabled(e);
			if ( e )
				pbmod->ClearFlag(pfGUIControlMod::kIntangible);
			else
				pbmod->SetFlag(pfGUIControlMod::kIntangible);
		}
	}
}

hsBool pyGUIControl::IsEnabled( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsEnabled();
	}
	return false;
}

void pyGUIControl::SetFocused( hsBool e )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetFocused(e);
	}
}

hsBool pyGUIControl::IsFocused( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsFocused();
	}
	return false;
}

void pyGUIControl::SetVisible( hsBool vis )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetVisible(vis);
	}
}

hsBool pyGUIControl::IsVisible( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsVisible();
	}
	return false;
}

hsBool pyGUIControl::IsInteresting( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsInteresting();
	}
	return false;
}

void pyGUIControl::SetNotifyOnInteresting( hsBool state )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetNotifyOnInteresting(state);
	}
}

void pyGUIControl::Refresh( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->Refresh();
	}
}

void pyGUIControl::SetObjectCenter( pyPoint3& pt)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetObjectCenter( pt.fPoint.fX, pt.fPoint.fY );
	}
}


PyObject* pyGUIControl::GetObjectCenter()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pyPoint3::New(pbmod->GetObjectCenter());
	}
	return pyPoint3::New(hsPoint3(0,0,0));
}



PyObject* pyGUIControl::GetOwnerDlg( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			// get the owner dialog modifier pointer
			pfGUIDialogMod* pdialog = pbmod->GetOwnerDlg();
			if ( pdialog )
			{
				// create a pythonized Dialog class object
				return pyGUIDialog::New(pdialog->GetKey());
			}
		}
	}
	PyErr_SetString(PyExc_NameError, "No owner GUIDialog could be found for this control...?");
	PYTHON_RETURN_ERROR;
}

	// get color schemes
PyObject* pyGUIControl::GetForeColor()
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fForeColor.r,color->fForeColor.g,color->fForeColor.b,color->fForeColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIControl::GetSelColor()
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fSelForeColor.r,color->fSelForeColor.g,color->fSelForeColor.b,color->fSelForeColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIControl::GetBackColor()
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fBackColor.r,color->fBackColor.g,color->fBackColor.b,color->fBackColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* pyGUIControl::GetBackSelColor()
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			return pyColor::New(color->fSelBackColor.r,color->fSelBackColor.g,color->fSelBackColor.b,color->fSelBackColor.a);
		}
	}
	PYTHON_RETURN_NONE;
}

UInt32 pyGUIControl::GetFontSize()
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
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
void pyGUIControl::SetForeColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
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

void pyGUIControl::SetSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
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

void pyGUIControl::SetBackColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
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

void pyGUIControl::SetBackSelColor( hsScalar r, hsScalar g, hsScalar b, hsScalar a )
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
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


void pyGUIControl::SetFontSize(UInt32 fontsize)
{
	if ( fGCkey )
	{
		pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pdmod )
		{
			pfGUIColorScheme* color = pdmod->GetColorScheme();
			color->fFontSize = (UInt8)fontsize;
		}
	}
}
