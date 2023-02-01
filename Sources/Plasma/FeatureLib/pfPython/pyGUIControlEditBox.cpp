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

#include "pfGameGUIMgr/pfGUIEditBoxMod.h"

#include "pyGUIControlEditBox.h"
#include "pyColor.h"

pyGUIControlEditBox::pyGUIControlEditBox(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlEditBox::pyGUIControlEditBox(plKey objkey) : pyGUIControl(std::move(objkey))
{
}


bool pyGUIControlEditBox::IsGUIControlEditBox(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIEditBoxMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}

void pyGUIControlEditBox::SetBufferSize( uint32_t size )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->SetBufferSize(size);
    }
}

std::wstring pyGUIControlEditBox::GetBufferW()
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

void pyGUIControlEditBox::ClearBuffer()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->ClearBuffer();
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

void pyGUIControlEditBox::SetCursorToHome()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->SetCursorToHome();
    }
}

void pyGUIControlEditBox::SetCursorToEnd()
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
/*  if ( fGCkey )
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
/*  if ( fGCkey )
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

bool pyGUIControlEditBox::WasEscaped()
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

void pyGUIControlEditBox::SetSpecialCaptureKeyMode(bool state)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->SetSpecialCaptureKeyMode(state);
    }
}

uint32_t pyGUIControlEditBox::GetLastKeyCaptured()
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

uint32_t pyGUIControlEditBox::GetLastModifiersCaptured()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            return (uint32_t)(pebmod->GetLastModifiersCaptured());
    }
    return 0;
}

void pyGUIControlEditBox::SetLastKeyCapture(uint32_t key, uint32_t modifiers)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->SetLastKeyCapture(key,(uint8_t)modifiers);
    }
}

void pyGUIControlEditBox::SetChatMode(bool state)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIEditBoxMod* pebmod = pfGUIEditBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pebmod )
            pebmod->SetChatMode(state);
    }
}
