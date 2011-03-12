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

#include "../pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"

#include "pyGUIControlMultiLineEdit.h"
#include "pyColor.h"

pyGUIControlMultiLineEdit::pyGUIControlMultiLineEdit(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlMultiLineEdit::pyGUIControlMultiLineEdit(plKey objkey) : pyGUIControl(objkey)
{
}

hsBool pyGUIControlMultiLineEdit::IsGUIControlMultiLineEdit(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIMultiLineEditCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}


void pyGUIControlMultiLineEdit::SetScrollPosition( Int32 topLine )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->SetScrollPosition(topLine);
		}
	}
}

void pyGUIControlMultiLineEdit::MoveCursor( Int32 dir)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->MoveCursor((enum pfGUIMultiLineEditCtrl::Direction)dir);
		}
	}
}


void pyGUIControlMultiLineEdit::ClearBuffer( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->ClearBuffer();
		}
	}
}

void pyGUIControlMultiLineEdit::SetText( const char *asciiText )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->SetBuffer(asciiText);
		}
	}
}

void pyGUIControlMultiLineEdit::SetTextW( const wchar_t *asciiText )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->SetBuffer(asciiText);
		}
	}
}

const char* pyGUIControlMultiLineEdit::GetText( void )
{
	// up to the caller to free the string... but when?
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			const char* text = pbmod->GetNonCodedBuffer();
			// convert string to a PyObject (which also copies the string)
			return text;
		}
	}
	// return None on error
	return nil;
}

const wchar_t* pyGUIControlMultiLineEdit::GetTextW( void )
{
	// up to the caller to free the string... but when?
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			const wchar_t* text = pbmod->GetNonCodedBufferW();
			// convert string to a PyObject (which also copies the string)
			return text;
		}
	}
	// return None on error
	return nil;
}

//
// set the encoded buffer - encoded with style and color
//
void pyGUIControlMultiLineEdit::SetEncodedBuffer( PyObject* buffer_object )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			// something to do here... later
			UInt8* daBuffer = nil;
			int length;
			PyObject_AsReadBuffer( buffer_object, (const void**)&daBuffer, &length);
			if ( daBuffer != nil )
			{
				// don't alter the user's buffer... but into a copy of our own
				UInt8* altBuffer = TRACKED_NEW UInt8[length];
// =====> temp>> change 0xFEs back into '\0's
				int i;
				for ( i=0 ; i<length ; i++ )
				{
					if ( daBuffer[i] == 254 )
						altBuffer[i] = 0;		// change into a 0xFE
					else
						altBuffer[i] = daBuffer[i];
				}
// =====> temp>> change 0xFEs back into '\0's
				pbmod->SetBuffer( altBuffer, length );
				delete [] altBuffer;

				pbmod->SetCursorToLoc(0);
				pbmod->SetScrollPosition(0);
			}
		}
	}
}

void pyGUIControlMultiLineEdit::SetEncodedBufferW( PyObject* buffer_object )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			// something to do here... later
			UInt16* daBuffer = nil;
			int length;
			PyObject_AsReadBuffer( buffer_object, (const void**)&daBuffer, &length);
			if ( daBuffer != nil )
			{
				// don't alter the user's buffer... but into a copy of our own
				UInt16* altBuffer = TRACKED_NEW UInt16[length];
				// =====> temp>> change 0xFFFEs back into '\0's
				int i;
				for ( i=0 ; i<length ; i++ )
				{
					if ( daBuffer[i] == 0xFFFE )
						altBuffer[i] = 0;		// change into a 0
					else
						altBuffer[i] = daBuffer[i];
				}
				// =====> temp>> change 0xFFFEs back into '\0's
				pbmod->SetBuffer( altBuffer, length );
				delete [] altBuffer;

				pbmod->SetCursorToLoc(0);
				pbmod->SetScrollPosition(0);
			}
		}
	}
}

const char* pyGUIControlMultiLineEdit::GetEncodedBuffer()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			UInt32 length;
			UInt8* daBuffer = pbmod->GetCodedBuffer( length );
			if ( daBuffer )
			{
				UInt8* altBuffer = TRACKED_NEW UInt8[length+1];
// =====> temp>> to get rid of '\0's (change into 0xFEs)
				int i;
				for ( i=0 ; i<length ; i++ )
				{
					if ( daBuffer[i] == 0 )
						altBuffer[i] = 254;		// change into a 0xFE
					else
						altBuffer[i] = daBuffer[i];
				}
				// add '\0' top end of string
				altBuffer[length] = 0;
// =====> temp>> to get rid of '\0's (change into 0xFEs)
				delete [] daBuffer;
				// May have to use a string object instead of a buffer
				// (String makes its own copy of the string)
				return (const char*)altBuffer;
			}
		}
	}
	// return None on error
	return nil;
}

const wchar_t* pyGUIControlMultiLineEdit::GetEncodedBufferW()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			UInt32 length;
			UInt16* daBuffer = pbmod->GetCodedBufferW( length );
			if ( daBuffer )
			{
				UInt16* altBuffer = TRACKED_NEW UInt16[length+1];
				// =====> temp>> to get rid of '\0's (change into 0xFFFEs)
				int i;
				for ( i=0 ; i<length ; i++ )
				{
					if ( daBuffer[i] == 0 )
						altBuffer[i] = 0xFFFE;		// change into a 0xFFFE
					else
						altBuffer[i] = daBuffer[i];
				}
				// add '\0' top end of string
				altBuffer[length] = 0;
				delete [] daBuffer;
				// =====> temp>> to get rid of '\0's (change into 0xFEs)
				// May have to use a string object instead of a buffer
				// (String makes its own copy of the string)
				return (const wchar_t*)altBuffer;
			}
		}
	}
	// return None on error
	return nil;
}

UInt32	pyGUIControlMultiLineEdit::GetBufferSize()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			return pbmod->GetBufferSize();
		}
	}
	return 0;
}



void pyGUIControlMultiLineEdit::InsertChar( char c )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertChar(c);
		}
	}
}

void pyGUIControlMultiLineEdit::InsertCharW( wchar_t c )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertChar(c);
		}
	}
}

void pyGUIControlMultiLineEdit::InsertString( const char *string )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertString(string);
		}
	}
}

void pyGUIControlMultiLineEdit::InsertStringW( const wchar_t *string )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertString(string);
		}
	}
}

void pyGUIControlMultiLineEdit::InsertColor( pyColor& color )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertColor(color.getColor());
		}
	}
}

void pyGUIControlMultiLineEdit::InsertStyle( UInt8 fontStyle )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->InsertStyle(fontStyle);
		}
	}
}

void pyGUIControlMultiLineEdit::DeleteChar( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
		{
			pbmod->DeleteChar();
		}
	}
}

void pyGUIControlMultiLineEdit::Lock( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->Lock();
	}
}

void pyGUIControlMultiLineEdit::Unlock( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->Unlock();
	}
}

hsBool pyGUIControlMultiLineEdit::IsLocked( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsLocked();
	}
	// don't know... must false then
	return false;
}

void pyGUIControlMultiLineEdit::Clickable( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->ClearFlag(pfGUIControlMod::kIntangible);
	}
}

void pyGUIControlMultiLineEdit::Unclickable( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetFlag(pfGUIControlMod::kIntangible);
	}
}

void pyGUIControlMultiLineEdit::SetBufferLimit(Int32 limit)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetBufferLimit(limit);
	}
}

Int32 pyGUIControlMultiLineEdit::GetBufferLimit()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->GetBufferLimit();
	}
	return 0;
}

void pyGUIControlMultiLineEdit::EnableScrollControl()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetScrollEnable(true);
	}
}

void pyGUIControlMultiLineEdit::DisableScrollControl()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetScrollEnable(false);
	}
}

void pyGUIControlMultiLineEdit::DeleteLinesFromTop( int lines )
{
	if (fGCkey)
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->DeleteLinesFromTop(lines);
	}
}

UInt32 pyGUIControlMultiLineEdit::GetFontSize()
{
	if (fGCkey)
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->GetFontSize();
	}
	return 12; // something relatively sane
}

void pyGUIControlMultiLineEdit::SetFontSize( UInt32 fontsize )
{
	if (fGCkey)
	{
		// get the pointer to the modifier
		pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetFontSize((UInt8)fontsize);
	}
}