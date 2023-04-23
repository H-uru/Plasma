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

#include <string_theory/string>

#include "pyKey.h"

#include "pfGameGUIMgr/pfGUITextBoxMod.h"
#include "pfGameGUIMgr/pfGUIListElement.h"

#include "pyGUIControlTextBox.h"
#include "pyColor.h"

pyGUIControlTextBox::pyGUIControlTextBox(pyKey& gckey) : pyGUIControl(gckey)
{
    fOriginalColorScheme = nullptr;
}

pyGUIControlTextBox::pyGUIControlTextBox(plKey objkey) : pyGUIControl(std::move(objkey))
{
    fOriginalColorScheme = nullptr;
}


bool pyGUIControlTextBox::IsGUIControlTextBox(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUITextBoxMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}


ST::string pyGUIControlTextBox::GetText() const
{
    if (fGCkey) {
        // get the pointer to the modifier
        pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (ptbmod)
            return ptbmod->GetText();
    }
    // else if there is no string... fake one
    return {};
}

void pyGUIControlTextBox::SetText(ST::string text)
{
    if (fGCkey) {
        // get the pointer to the modifier
        pfGUITextBoxMod* ptbmod = pfGUITextBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (ptbmod)
            ptbmod->SetText(std::move(text));
    }
}

void pyGUIControlTextBox::SetFontSize( uint8_t size )
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

void pyGUIControlTextBox::SetJustify( uint8_t justify )
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

uint8_t pyGUIControlTextBox::GetJustify()
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

