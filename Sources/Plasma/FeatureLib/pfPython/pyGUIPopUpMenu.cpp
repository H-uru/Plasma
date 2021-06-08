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

#include "pyColor.h"

#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfGameGUIMgr/pfGUIControlMod.h"
#include "pfGameGUIMgr/pfGUIPopUpMenu.h"

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

#include "pfGameGUIMgr/pfGUIControlHandlers.h"

#define kGetMenuPtr(ret) \
        if (fGCkey == nullptr) \
            return ret; \
        pfGUIPopUpMenu *menu = pfGUIPopUpMenu::ConvertNoRef(fGCkey->ObjectIsLoaded()); \
        if (menu == nullptr) \
            return ret;


pyGUIPopUpMenu::pyGUIPopUpMenu(pyKey& gckey)
{
    fGCkey = gckey.getKey();
    fBuiltMenu = nullptr;
}

pyGUIPopUpMenu::pyGUIPopUpMenu(plKey objkey)
{
    fGCkey = objkey;
    fBuiltMenu = nullptr;
}

pyGUIPopUpMenu::pyGUIPopUpMenu()
{
    fGCkey = nullptr;
    fBuiltMenu = nullptr;
}

pyGUIPopUpMenu::pyGUIPopUpMenu( const char *name, float screenOriginX, float screenOriginY, const plLocation &destLoc/* = plLocation::kGlobalFixedLoc */)
{
    fBuiltMenu = pfGUIPopUpMenu::Build(name, nullptr, screenOriginX, screenOriginY, destLoc);
    if (fBuiltMenu != nullptr)
    {
        fGCkey = fBuiltMenu->GetKey();
        fGCkey->RefObject();
    }
    else
        fGCkey = nullptr;
}

pyGUIPopUpMenu::pyGUIPopUpMenu( const char *name, pyGUIPopUpMenu &parent, float screenOriginX, float screenOriginY )
{
    pfGUIPopUpMenu *parentMenu = pfGUIPopUpMenu::ConvertNoRef( parent.fGCkey->ObjectIsLoaded() );
    
    const plLocation &parentLoc = (parentMenu != nullptr) ? parentMenu->GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc;

    fBuiltMenu = pfGUIPopUpMenu::Build( name, parentMenu, screenOriginX, screenOriginY, parentLoc );
    if (fBuiltMenu != nullptr)
    {
        fGCkey = fBuiltMenu->GetKey();
        fGCkey->RefObject();
    }
    else
        fGCkey = nullptr;
}

pyGUIPopUpMenu::~pyGUIPopUpMenu()
{
    // Optimistic, I know, to assume the destructor will be called, but still...
    if (fBuiltMenu != nullptr)
    {
        fBuiltMenu->GetKey()->UnRefObject();
        fBuiltMenu = nullptr;
    }
}

void pyGUIPopUpMenu::setup(plKey objkey)
{
    // kill any previous menu
    if (fBuiltMenu != nullptr)
    {
        fBuiltMenu->GetKey()->UnRefObject();
        fBuiltMenu = nullptr;
    }

    // setup the new one
    fGCkey = objkey;
}

void pyGUIPopUpMenu::setup(const char *name, float screenOriginX, float screenOriginY, const plLocation &destLoc /* = plLocation::kGlobalFixedLoc */)
{
    // kill any previous menu
    if (fBuiltMenu != nullptr)
    {
        fBuiltMenu->GetKey()->UnRefObject();
        fBuiltMenu = nullptr;
    }

    // setup the new one
    fBuiltMenu = pfGUIPopUpMenu::Build(name, nullptr, screenOriginX, screenOriginY, destLoc);
    if (fBuiltMenu != nullptr)
    {
        fGCkey = fBuiltMenu->GetKey();
        fGCkey->RefObject();
    }
    else
        fGCkey = nullptr;
}

void pyGUIPopUpMenu::setup(const char *name, pyGUIPopUpMenu &parent, float screenOriginX, float screenOriginY)
{
    // kill any previous menu
    if (fBuiltMenu != nullptr)
    {
        fBuiltMenu->GetKey()->UnRefObject();
        fBuiltMenu = nullptr;
    }

    // setup the new one
    pfGUIPopUpMenu *parentMenu = pfGUIPopUpMenu::ConvertNoRef( parent.fGCkey->ObjectIsLoaded() );

    const plLocation &parentLoc = (parentMenu != nullptr) ? parentMenu->GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc;

    fBuiltMenu = pfGUIPopUpMenu::Build( name, parentMenu, screenOriginX, screenOriginY, parentLoc );
    if (fBuiltMenu != nullptr)
    {
        fGCkey = fBuiltMenu->GetKey();
        fGCkey->RefObject();
    }
    else
        fGCkey = nullptr;
}

bool pyGUIPopUpMenu::IsGUIPopUpMenu(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIPopUpMenu::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
        return true;
    return false;
}


// override the equals to operator
bool pyGUIPopUpMenu::operator==(const pyGUIPopUpMenu &gcobj) const
{
    plKey theirs = ((pyGUIPopUpMenu&)gcobj).getObjKey();
    if (fGCkey == nullptr && theirs == nullptr)
        return true;
    else if (fGCkey != nullptr && theirs != nullptr)
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
uint32_t  pyGUIPopUpMenu::GetTagID()
{
    kGetMenuPtr( 0 );
    return menu->GetTagID();
}


void pyGUIPopUpMenu::SetEnabled( bool e )
{
    kGetMenuPtr( ; );
    menu->SetEnabled(e);
}

bool pyGUIPopUpMenu::IsEnabled()
{
    kGetMenuPtr( false );
    return menu->IsEnabled();
}

ST::string pyGUIPopUpMenu::GetName() const
{
    kGetMenuPtr( {} );
    return menu->GetName();
}


uint32_t pyGUIPopUpMenu::GetVersion()
{
    kGetMenuPtr( 0 );
    return menu->GetVersion();
}


void pyGUIPopUpMenu::Show()
{
    kGetMenuPtr( ; );
    ( (pfGUIDialogMod *)menu )->Show();
}

void pyGUIPopUpMenu::Hide()
{
    kGetMenuPtr( ; );
    menu->Hide();
}

    // get color schemes
PyObject* pyGUIPopUpMenu::GetForeColor()
{
    kGetMenuPtr(nullptr);

    pfGUIColorScheme* color = menu->GetColorScheme();
    return pyColor::New(color->fForeColor.r,color->fForeColor.g,color->fForeColor.b,color->fForeColor.a);
}

PyObject* pyGUIPopUpMenu::GetSelColor()
{
    kGetMenuPtr(nullptr);

    pfGUIColorScheme* color = menu->GetColorScheme();
    return pyColor::New(color->fSelForeColor.r,color->fSelForeColor.g,color->fSelForeColor.b,color->fSelForeColor.a);
}

PyObject* pyGUIPopUpMenu::GetBackColor()
{
    kGetMenuPtr(nullptr);

    pfGUIColorScheme* color = menu->GetColorScheme();
    return pyColor::New(color->fBackColor.r,color->fBackColor.g,color->fBackColor.b,color->fBackColor.a);
}

PyObject* pyGUIPopUpMenu::GetBackSelColor()
{
    kGetMenuPtr(nullptr);

    pfGUIColorScheme* color = menu->GetColorScheme();
    return pyColor::New(color->fSelBackColor.r,color->fSelBackColor.g,color->fSelBackColor.b,color->fSelBackColor.a);
}

    // set color scheme
void pyGUIPopUpMenu::SetForeColor( float r, float g, float b, float a )
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

void pyGUIPopUpMenu::SetSelColor( float r, float g, float b, float a )
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

void pyGUIPopUpMenu::SetBackColor( float r, float g, float b, float a )
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

void pyGUIPopUpMenu::SetBackSelColor( float r, float g, float b, float a )
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

void    pyGUIPopUpMenu::AddConsoleCmdItem( const char *name, const char *consoleCmd )
{
    wchar_t *wName = hsStringToWString(name);
    AddConsoleCmdItemW(wName,consoleCmd);
    delete [] wName;
}

void    pyGUIPopUpMenu::AddConsoleCmdItemW( std::wstring name, const char *consoleCmd )
{
    kGetMenuPtr( ; );
    menu->AddItem(name.c_str(), new pfGUIConsoleCmdProc(consoleCmd), nullptr);
}

void    pyGUIPopUpMenu::AddNotifyItem( const char *name )
{
    wchar_t *wName = hsStringToWString(name);
    AddNotifyItemW(wName);
    delete [] wName;
}

void    pyGUIPopUpMenu::AddNotifyItemW( std::wstring name )
{
    kGetMenuPtr( ; );
    menu->AddItem(name.c_str(), (pfGUICtrlProcObject *)(menu->GetHandler()), nullptr);
}

void    pyGUIPopUpMenu::AddSubMenuItem( const char *name, pyGUIPopUpMenu &subMenu )
{
    wchar_t *wName = hsStringToWString(name);
    AddSubMenuItemW(wName,subMenu);
    delete [] wName;
}

void    pyGUIPopUpMenu::AddSubMenuItemW( std::wstring name, pyGUIPopUpMenu &subMenu )
{
    kGetMenuPtr( ; );

    if (subMenu.fGCkey == nullptr)
        return;
    pfGUIPopUpMenu *subM = pfGUIPopUpMenu::ConvertNoRef( subMenu.fGCkey->ObjectIsLoaded() );
    if (subM == nullptr)
        return;

    menu->AddItem(name.c_str(), nullptr, subM);
}
