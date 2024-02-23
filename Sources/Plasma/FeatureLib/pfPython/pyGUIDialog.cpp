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

#include <string_theory/format>

#include "pyGlueHelpers.h"
#include "pyKey.h"

#include "pyColor.h"

#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"

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
#include "pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "pfGameGUIMgr/pfGUIProgressCtrl.h"
#include "pfGameGUIMgr/pfGUIUpDownPairMod.h"

pyGUIDialog::pyGUIDialog(pyKey& gckey)
{
    fGCkey = gckey.getKey();
}

pyGUIDialog::pyGUIDialog(plKey objkey)
{
    fGCkey = std::move(objkey);
}

pyGUIDialog::pyGUIDialog()
{
}

PyObject* pyGUIDialog::ConvertControl(const plKey& key)
{
    if (!key)
        return nullptr;

    switch (WhatControlType(key)) {
    case kPopUpMenu:
        // This isn't really a control, but OnGUINotify may receive this.
        return pyGUIPopUpMenu::New(key);
    case kDialog:
        // This is definitely not a control, but OnGUINotify may receive this.
        return pyGUIDialog::New(key);
    case kButton:
        return pyGUIControlButton::New(key);
    case kCheckBox:
        return pyGUIControlCheckBox::New(key);
    case kEditBox:
        return pyGUIControlEditBox::New(key);
    case kListBox:
        return pyGUIControlListBox::New(key);
    case kRadioGroup:
        return pyGUIControlRadioGroup::New(key);
    case kTextBox:
        return pyGUIControlTextBox::New(key);
    case kKnob:
        return pyGUIControlKnob::New(key);
    case kUpDownPair:
        return pyGUIControlUpDownPair::New(key);
    case kDynamicText:
        return pyGUIControlDynamicText::New(key);
    case kMultiLineEdit:
        return pyGUIControlMultiLineEdit::New(key);
    case kClickMap:
        return pyGUIControlClickMap::New(key);
    case kProgress:
        return pyGUIControlProgress::New(key);
    default:
        return nullptr;
    }
}

bool pyGUIDialog::IsGUIDialog(const plKey& key)
{
    if ( key && pfGUIDialogMod::ConvertNoRef(key->GetObjectPtr()) )
        return true;
    return false;
}


uint32_t pyGUIDialog::WhatControlType(const plKey& key)
{
    // Do the pop-up menu test first, since it's derived from dialog
    if ( pyGUIPopUpMenu::IsGUIPopUpMenu(key) )
        return kPopUpMenu;
    else if ( pyGUIDialog::IsGUIDialog(key) )
        return kDialog;
    else if ( pyGUIControlButton::IsGUIControlButton(key) )
        return kButton;
    else if ( pyGUIControlCheckBox::IsGUIControlCheckBox(key) )
        return kCheckBox;
    else if ( pyGUIControlEditBox::IsGUIControlEditBox(key) )
        return kEditBox;
    else if ( pyGUIControlListBox::IsGUIControlListBox(key) )
        return kListBox;
    else if ( pyGUIControlRadioGroup::IsGUIControlRadioGroup(key) )
        return kRadioGroup;
    else if ( pyGUIControlTextBox::IsGUIControlTextBox(key) )
        return kTextBox;
    else if ( pyGUIControlValue::IsGUIControlValue(key) )
    {
        // then see what kind of value control it is
        if ( pfGUIKnobCtrl::ConvertNoRef(key->GetObjectPtr()) )
            return kKnob;
        else if ( pfGUIUpDownPairMod::ConvertNoRef(key->GetObjectPtr()) )
            return kUpDownPair;
        else if ( pfGUIProgressCtrl::ConvertNoRef(key->GetObjectPtr()) )
            return kProgress;
        else
            return 0;
    }
    else if ( pyGUIControlDynamicText::IsGUIControlDynamicText( key ) )
        return kDynamicText;
    else if ( pyGUIControlMultiLineEdit::IsGUIControlMultiLineEdit( key ) )
        return kMultiLineEdit;
    else if ( pyGUIControlClickMap::IsGUIControlClickMap( key ) )
        return kClickMap;
    else
        return 0;
}


// override the equals to operator
bool pyGUIDialog::operator==(const pyGUIDialog &gcobj) const
{
    plKey theirs = ((pyGUIDialog&)gcobj).getObjKey();
    if (fGCkey == nullptr && theirs == nullptr)
        return true;
    else if (fGCkey != nullptr && theirs != nullptr)
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
uint32_t  pyGUIDialog::GetTagID()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            return pdmod->GetTagID();
    }
    return 0;
}


void pyGUIDialog::SetEnabled( bool e )
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            pdmod->SetEnabled(e);
    }
}

bool pyGUIDialog::IsEnabled()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            return pdmod->IsEnabled();
    }
    return false;
}

ST::string pyGUIDialog::GetName() const
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            return pdmod->GetName();
    }
    return {};
}


uint32_t pyGUIDialog::GetVersion()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            return pdmod->GetVersion();
    }
    return 0;
}


size_t pyGUIDialog::GetNumControls()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            return pdmod->GetNumControls();
    }
    return 0;
}

PyObject* pyGUIDialog::GetControl( uint32_t idx )
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
    ST::string errmsg = ST::format("Index {d} not found in GUIDialog {}", idx, GetName());
    PyErr_SetObject(PyExc_KeyError, PyUnicode_FromSTString(errmsg));
    PYTHON_RETURN_ERROR;
}

PyObject* pyGUIDialog::GetControlMod(uint32_t idx) const
{
    if (fGCkey) {
        const pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pdmod) {
            const pfGUIControlMod* pcontrolmod = pdmod->GetControl(idx);
            if (pcontrolmod) {
                PyObject* retVal = ConvertControl(pcontrolmod->GetKey());
                if (retVal)
                    return retVal;
            }
        }
    }

    // if we got here then there must have been an error
    ST::string errmsg = ST::format("Index {d} not found in GUIDialog {}", idx, GetName());
    PyErr_SetObject(PyExc_KeyError, PyUnicode_FromSTString(errmsg));
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

void pyGUIDialog::Show()
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

void pyGUIDialog::ShowNoReset()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            pdmod->ShowNoReset();
    }
}

void pyGUIDialog::Hide()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            pdmod->Hide();
    }
}

PyObject* pyGUIDialog::GetControlFromTag( uint32_t tagID )
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
    ST::string errmsg = ST::format("TagID {d} not found in GUIDialog {}", tagID, GetName());
    PyErr_SetObject(PyExc_KeyError, PyUnicode_FromSTString(errmsg));
    PYTHON_RETURN_ERROR;
}

PyObject* pyGUIDialog::GetControlModFromTag(uint32_t tagID) const
{
    if (fGCkey) {
        const pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (pdmod) {
            const pfGUIControlMod* pcontrolmod = pdmod->GetControlFromTag(tagID);
            if (pcontrolmod) {
                PyObject* retVal = ConvertControl(pcontrolmod->GetKey());
                if (retVal)
                    return retVal;
            }
        }
    }

    // if we got here then there must have been an error
    ST::string errmsg = ST::format("TagID {d} not found in GUIDialog {}", tagID, GetName());
    PyErr_SetObject(PyExc_KeyError, PyUnicode_FromSTString(errmsg));
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

uint32_t pyGUIDialog::GetFontSize()
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
void pyGUIDialog::SetForeColor( float r, float g, float b, float a )
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

void pyGUIDialog::SetSelColor( float r, float g, float b, float a )
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

void pyGUIDialog::SetBackColor( float r, float g, float b, float a )
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

void pyGUIDialog::SetBackSelColor( float r, float g, float b, float a )
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


void pyGUIDialog::SetFontSize(uint32_t fontsize)
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
        {
            pfGUIColorScheme* color = pdmod->GetColorScheme();
            color->fFontSize = (uint8_t)fontsize;
        }
    }
}

void pyGUIDialog::UpdateAllBounds()
{
    if ( fGCkey )
    {
        pfGUIDialogMod* pdmod = pfGUIDialogMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdmod )
            pdmod->UpdateAllBounds();
    }
}

void pyGUIDialog::RefreshAllControls()
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
            pdmod->SetFocus(nullptr);
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
