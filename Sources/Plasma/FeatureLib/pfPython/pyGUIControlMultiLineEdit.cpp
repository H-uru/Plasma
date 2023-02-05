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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include <Python.h>
#include "pyKey.h"

#include "pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"

#include "pyGUIControlMultiLineEdit.h"
#include "pyColor.h"

pyGUIControlMultiLineEdit::pyGUIControlMultiLineEdit(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlMultiLineEdit::pyGUIControlMultiLineEdit(plKey objkey) : pyGUIControl(std::move(objkey))
{
}

bool pyGUIControlMultiLineEdit::IsGUIControlMultiLineEdit(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIMultiLineEditCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}


void pyGUIControlMultiLineEdit::SetScrollPosition( int32_t topLine )
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

int32_t pyGUIControlMultiLineEdit::GetScrollPosition()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            return pbmod->GetScrollPosition();
    }
    return 0;
}

bool pyGUIControlMultiLineEdit::IsAtEnd()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            return pbmod->ShowingEndOfBuffer();
    }
    return false;
}

void pyGUIControlMultiLineEdit::MoveCursor( int32_t dir)
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

int32_t pyGUIControlMultiLineEdit::GetCursor() const
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            return pbmod->GetCursor();
    }

    return -1;
}

void pyGUIControlMultiLineEdit::ClearBuffer()
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

void pyGUIControlMultiLineEdit::SetText( const wchar_t *text )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
        {
            pbmod->SetBuffer(text);
        }
    }
}

const wchar_t* pyGUIControlMultiLineEdit::GetText()
{
    // up to the caller to free the string... but when?
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
        {
            const wchar_t* text = pbmod->GetNonCodedBuffer();
            // convert string to a PyObject (which also copies the string)
            return text;
        }
    }
    // return None on error
    return nullptr;
}

void pyGUIControlMultiLineEdit::SetEncodedBuffer( PyObject* buffer_object )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
        {
            // something to do here... later
            Py_buffer view;
            PyObject_GetBuffer(buffer_object, &view, PyBUF_SIMPLE);
            const wchar_t* daBuffer = (const wchar_t*)view.buf;
            Py_ssize_t length = view.len;
            PyBuffer_Release(&view);

            if (daBuffer != nullptr)
            {
                // don't alter the user's buffer... but into a copy of our own
                wchar_t* altBuffer = new wchar_t[length];
                // =====> temp>> change 0xFFFEs back into '\0's
                for (Py_ssize_t i = 0; i < length; i++)
                {
                    if (daBuffer[i] == L'\xfffe')
                        altBuffer[i] = 0;       // change into a 0
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

const wchar_t* pyGUIControlMultiLineEdit::GetEncodedBuffer()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
        {
            size_t length;
            wchar_t* daBuffer = pbmod->GetCodedBuffer(length);
            if ( daBuffer )
            {
                wchar_t* altBuffer = new wchar_t[length + 1];
                // =====> temp>> to get rid of '\0's (change into 0xFFFEs)
                for (size_t i = 0; i < length; i++)
                {
                    if ( daBuffer[i] == 0 )
                        altBuffer[i] = L'\xfffe';      // change into a 0xFFFE
                    else
                        altBuffer[i] = daBuffer[i];
                }
                // add '\0' top end of string
                altBuffer[length] = 0;
                delete [] daBuffer;
                // =====> temp>> to get rid of '\0's (change into 0xFEs)
                // May have to use a string object instead of a buffer
                // (String makes its own copy of the string)
                return altBuffer;
            }
        }
    }
    // return None on error
    return nullptr;
}

size_t pyGUIControlMultiLineEdit::GetBufferSize() const
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


void pyGUIControlMultiLineEdit::InsertChar( wchar_t c )
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

void pyGUIControlMultiLineEdit::InsertString( const wchar_t *string )
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
            hsColorRGBA col = color.getColor();
            pbmod->InsertColor(col);
        }
    }
}

void pyGUIControlMultiLineEdit::InsertStyle( uint8_t fontStyle )
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

void pyGUIControlMultiLineEdit::InsertLink(int16_t linkId)
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
        {
            pbmod->InsertLink(linkId);
        }
    }
}

void pyGUIControlMultiLineEdit::ClearLink()
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
        {
            pbmod->ClearLink();
        }
    }
}

void pyGUIControlMultiLineEdit::DeleteChar()
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

void pyGUIControlMultiLineEdit::Lock()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->Lock();
    }
}

void pyGUIControlMultiLineEdit::Unlock()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->Unlock();
    }
}

bool pyGUIControlMultiLineEdit::IsLocked()
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

void pyGUIControlMultiLineEdit::Clickable()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->ClearFlag(pfGUIControlMod::kIntangible);
    }
}

void pyGUIControlMultiLineEdit::Unclickable()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetFlag(pfGUIControlMod::kIntangible);
    }
}

void pyGUIControlMultiLineEdit::SetBufferLimit(int32_t limit)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetBufferLimit(limit);
    }
}

int32_t pyGUIControlMultiLineEdit::GetBufferLimit()
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

int16_t pyGUIControlMultiLineEdit::GetCurrentLink() const
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            return pbmod->GetCurrentLink();
    }
    return -1;
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

uint32_t pyGUIControlMultiLineEdit::GetFontSize() const
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

void pyGUIControlMultiLineEdit::SetFontSize( uint32_t fontsize )
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetFontSize((uint8_t)fontsize);
    }
}

void pyGUIControlMultiLineEdit::BeginUpdate()
{
    if (fGCkey) {
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            pbmod->BeginUpdate();
    }
}

void pyGUIControlMultiLineEdit::EndUpdate(bool redraw)
{
    if (fGCkey) {
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            pbmod->EndUpdate(redraw);
    }
}

bool pyGUIControlMultiLineEdit::IsUpdating() const
{
    if (fGCkey) {
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            return pbmod->IsUpdating();
    }

    return false;
}

std::tuple<int, int, int, int> pyGUIControlMultiLineEdit::GetMargins() const
{
    if (fGCkey) {
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            return pbmod->GetMargins();
    }

    return std::make_tuple(0, 0, 0, 0);
}

void pyGUIControlMultiLineEdit::SetMargins(int top, int left, int bottom, int right)
{
    if (fGCkey) {
        pfGUIMultiLineEditCtrl* pbmod = pfGUIMultiLineEditCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pbmod)
            pbmod->SetMargins(top, left, bottom, right);
    }
}
