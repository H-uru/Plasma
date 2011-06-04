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

#include "../pfGameGUIMgr/pfGUIEditBoxMod.h"

#include "pyGUIControlEditBox.h"
#include "pyColor.h"

pyGUIControlEditBox::pyGUIControlEditBox(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlEditBox::pyGUIControlEditBox(plKey objkey) : pyGUIControl(objkey)
{
}


hsBool pyGUIControlEditBox::IsGUIControlEditBox(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIEditBoxMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}

void pyGUIControlEditBox::SetBufferSize( UInt32 size )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetBufferSize(size);
	}
}


std::string pyGUIControlEditBox::GetBuffer( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			return pebmod->GetBuffer();
	}
	return "";
}

std::wstring pyGUIControlEditBox::GetBufferW( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			return pebmod->GetBufferW();
	}
	return L"";
}

void pyGUIControlEditBox::ClearBuffer( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->ClearBuffer();
	}
}

void pyGUIControlEditBox::SetText( const char *str )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetText(str);
	}
}

void pyGUIControlEditBox::SetTextW( const wchar_t *str )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetText(str);
	}
}

void pyGUIControlEditBox::SetCursorToHome(void)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetCursorToHome();
	}
}

void pyGUIControlEditBox::SetCursorToEnd(void)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetCursorToEnd();
	}
}


void pyGUIControlEditBox::SetColor(pyColor& forecolor, pyColor& backcolor)
{
/*	if ( fGCkey )
	{
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
		{
			pebmod->GetColorScheme().fForeColor = forecolor.getColor();
			pebmod->GetColorScheme().fBackColor = backcolor.getColor();
		}
	}
*/
}


void pyGUIControlEditBox::SetSelColor(pyColor& forecolor, pyColor& backcolor)
{
/*	if ( fGCkey )
	{
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
		{
			pebmod->GetColorScheme().fSelForeColor = forecolor.getColor();
			pebmod->GetColorScheme().fSelBackColor = backcolor.getColor();
		}
	}
*/
}

hsBool pyGUIControlEditBox::WasEscaped()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			return pebmod->WasEscaped();
	}
	return false;
}

void pyGUIControlEditBox::SetSpecialCaptureKeyMode(hsBool state)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetSpecialCaptureKeyMode(state);
	}
}

UInt32 pyGUIControlEditBox::GetLastKeyCaptured()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			return pebmod->GetLastKeyCaptured();
	}
	return 0;
}

UInt32 pyGUIControlEditBox::GetLastModifiersCaptured()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			return (UInt32)(pebmod->GetLastModifiersCaptured());
	}
	return 0;
}

void pyGUIControlEditBox::SetLastKeyCapture(UInt32 key, UInt32 modifiers)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetLastKeyCapture(key,(UInt8)modifiers);
	}
}

void pyGUIControlEditBox::SetChatMode(hsBool state)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pebmod )
			pebmod->SetChatMode(state);
	}
}
