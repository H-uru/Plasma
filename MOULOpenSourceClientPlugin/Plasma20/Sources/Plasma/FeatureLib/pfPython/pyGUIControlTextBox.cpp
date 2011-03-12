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

#include "../pfGameGUIMgr/pfGUITextBoxMod.h"
#include "../pfGameGUIMgr/pfGUIListElement.h"

#include "pyGUIControlTextBox.h"
#include "pyColor.h"

pyGUIControlTextBox::pyGUIControlTextBox(pyKey& gckey) : pyGUIControl(gckey)
{
	fOriginalColorScheme = nil;
}

pyGUIControlTextBox::pyGUIControlTextBox(plKey objkey) : pyGUIControl(objkey)
{
	fOriginalColorScheme = nil;
}


hsBool pyGUIControlTextBox::IsGUIControlTextBox(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUITextBoxMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}


std::string pyGUIControlTextBox::GetText()
{
	char *temp = hsWStringToString(GetTextW().c_str());
	std::string retVal = temp;
	delete [] temp;
	return retVal;
}

std::wstring pyGUIControlTextBox::GetTextW()
{
	if (fGCkey)
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			if ( ptbmod->GetText() )
			{
				std::wstring retVal = ptbmod->GetText();
				return retVal;
			}
		}
	}
	// else if there is no string... fake one
	return L"";
}

void pyGUIControlTextBox::SetText( const char *text )
{
	wchar_t *wText = hsStringToWString(text);
	SetTextW(wText);
	delete [] wText;
}

void pyGUIControlTextBox::SetTextW( std::wstring text )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
			ptbmod->SetText(text.c_str());
	}
}

void pyGUIControlTextBox::SetFontSize( UInt8 size )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			pfGUIColorScheme* colorscheme = ptbmod->GetColorScheme();
			colorscheme->fFontSize = size;
			ptbmod->UpdateColorScheme();
		}
	}

}

void pyGUIControlTextBox::SetForeColor( pyColor& color )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			pfGUIColorScheme* colorscheme = ptbmod->GetColorScheme();
			colorscheme->fForeColor = color.getColor();
			ptbmod->UpdateColorScheme();
		}
	}

}

PyObject* pyGUIControlTextBox::GetForeColor()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			pfGUIColorScheme* colorscheme = ptbmod->GetColorScheme();
			return pyColor::New(colorscheme->fForeColor);
		}
	}
	PYTHON_RETURN_NONE;
}

void pyGUIControlTextBox::SetBackColor( pyColor& color )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			pfGUIColorScheme* colorscheme = ptbmod->GetColorScheme();
			colorscheme->fBackColor = color.getColor();
		}
	}

}

void pyGUIControlTextBox::SetJustify( UInt8 justify )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			// reset all the flags for justification first
			ptbmod->ClearFlag(pfGUITextBoxMod::kCenterJustify);
			ptbmod->ClearFlag(pfGUITextBoxMod::kRightJustify);
			// then set the one they want
			if ( justify == pfGUIListText::kCenter)
				ptbmod->SetFlag(pfGUITextBoxMod::kCenterJustify);
			else if ( justify == pfGUIListText::kRightJustify)
				ptbmod->SetFlag(pfGUITextBoxMod::kRightJustify);
		}
	}
}

UInt8 pyGUIControlTextBox::GetJustify()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( ptbmod )
		{
			if ( ptbmod->HasFlag(pfGUITextBoxMod::kCenterJustify) )
				return pfGUIListText::kCenter;
			if ( ptbmod->HasFlag(pfGUITextBoxMod::kRightJustify) )
				return pfGUIListText::kRightJustify;
		}
	}
	return pfGUIListText::kLeftJustify;
}
