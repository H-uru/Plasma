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

#include "pyGUIControl.h"

#include "pfGameGUIMgr/pfGUIControlMod.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"

#include "pyGUIDialog.h"
#include "pyColor.h"
#include "pyGeometry3.h"

#include "pyGUIControlCheckBox.h"

pyGUIControl::pyGUIControl(pyKey& gckey)
{
    fGCkey = gckey.getKey();
}

pyGUIControl::pyGUIControl(plKey objkey)
{
    fGCkey = std::move(objkey);
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
bool pyGUIControl::operator==(const pyGUIControl &gcobj) const
{
    plKey theirs = ((pyGUIControl&)gcobj).getObjKey();
    if (fGCkey == nullptr && theirs == nullptr)
        return true;
    else if (fGCkey != nullptr && theirs != nullptr)
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
uint32_t  pyGUIControl::GetTagID()
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


void pyGUIControl::SetEnabled( bool e )
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

bool pyGUIControl::IsEnabled()
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

void pyGUIControl::SetFocused( bool e )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetFocused(e);
    }
}

bool pyGUIControl::IsFocused()
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

void pyGUIControl::SetVisible( bool vis )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetVisible(vis);
    }
}

bool pyGUIControl::IsVisible()
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

bool pyGUIControl::IsInteresting()
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

void pyGUIControl::SetNotifyOnInteresting( bool state )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIControlMod* pbmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pbmod )
            pbmod->SetNotifyOnInteresting(state);
    }
}

void pyGUIControl::Refresh()
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
    return pyPoint3::New();
}



PyObject* pyGUIControl::GetOwnerDlg()
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
PyObject* pyGUIControl::GetForeColor() const
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

PyObject* pyGUIControl::GetSelColor() const
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

PyObject* pyGUIControl::GetBackColor() const
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

PyObject* pyGUIControl::GetBackSelColor() const
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

uint32_t pyGUIControl::GetFontSize() const
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
void pyGUIControl::SetForeColor( float r, float g, float b, float a )
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

void pyGUIControl::SetSelColor( float r, float g, float b, float a )
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

void pyGUIControl::SetBackColor( float r, float g, float b, float a )
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

void pyGUIControl::SetBackSelColor( float r, float g, float b, float a )
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


void pyGUIControl::SetFontSize(uint32_t fontsize)
{
    if ( fGCkey )
    {
        pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
        {
            pfGUIColorScheme* color = pdmod->GetColorScheme();
            color->fFontSize = (uint8_t)fontsize;
        }
    }
}

void pyGUIControl::SetFontFlags(uint8_t fontFlags)
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pdmod)
        {
            pfGUIColorScheme* colorscheme = pdmod->GetColorScheme();
            colorscheme->fFontFlags = fontFlags;
            pdmod->UpdateColorScheme();
        }
    }
}

uint8_t pyGUIControl::GetFontFlags() const
{
    if (fGCkey)
    {
        // get the pointer to the modifier
        pfGUIControlMod* pdmod = pfGUIControlMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pdmod)
        {
            pfGUIColorScheme* colorscheme = pdmod->GetColorScheme();
            return colorscheme->fFontFlags;
        }
    }
    return 0;
}

