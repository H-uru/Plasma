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
#ifndef _pyGUIDialog_h_
#define _pyGUIDialog_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIDialog   - a wrapper class to provide interface to modifier
//                   attached to a GUIDialog
//
//////////////////////////////////////////////////////////////////////

#include "pnKeyedObject/plKey.h"

#include "pyGlueHelpers.h"
#include <vector>

class pyColor;
class pyKey;
namespace ST { class string; }

class pyGUIDialog
{
private:
    plKey                   fGCkey;
    
protected:
    pyGUIDialog(pyKey& gckey);
    pyGUIDialog(plKey objkey);
    pyGUIDialog();

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGUIDialog);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(pyKey& gckey);
    static PyObject *New(plKey objkey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIDialog object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIDialog); // converts a PyObject to a pyGUIDialog (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(PyObject* m);

    static bool IsGUIDialog(const plKey& key);

    void setKey(plKey key) { fGCkey = std::move(key); } // used by python glue, do NOT call

    enum            // these enums are used in Python so they have to match PlasmaTypes.py
    {
        kDialog=1,
        kButton=2,
        kDraggable=3,
        kListBox=4,
        kTextBox=5,
        kEditBox=6,
        kUpDownPair=7,
        kKnob=8,
        kDragBar=9,
        kCheckBox=10,
        kRadioGroup=11,
        kDynamicText=12,
        kMultiLineEdit=13,
        kPopUpMenu=14,
        kClickMap=15,
    };
    static PyObject* ConvertControl(const plKey& key);
    static uint32_t WhatControlType(const plKey& key);
    static void GUICursorOff();
    static void GUICursorOn();
    static void GUICursorDimmed();

    // override the equals to operator
    bool operator==(const pyGUIDialog &gdobj) const;
    bool operator!=(const pyGUIDialog &gdobj) const { return !(gdobj == *this);   }

    // getter and setters
    plKey getObjKey();
    PyObject* getObjPyKey(); // returns pyKey

    // interface functions
    uint32_t GetTagID();

    void SetEnabled(bool e);
    void Enable() { SetEnabled(true); }
    void Disable() { SetEnabled(false); }
    bool IsEnabled();
    ST::string GetName() const;
    uint32_t GetVersion();

    size_t GetNumControls();
    PyObject* GetControl(uint32_t idx); // returns pyKey
    PyObject* GetControlMod(uint32_t idx) const;
    void SetFocus(pyKey& gcKey);
    void NoFocus();
    void Show();
    void ShowNoReset();
    void Hide();
    PyObject* GetControlFromTag(uint32_t tagID); // returns pyKey
    PyObject* GetControlModFromTag(uint32_t tagID) const;

    // get color schemes
    PyObject* GetForeColor(); // returns pyColor
    PyObject* GetSelColor(); // returns pyColor
    PyObject* GetBackColor(); // returns pyColor
    PyObject* GetBackSelColor(); // returns pyColor
    uint32_t GetFontSize();
    // set color scheme
    void SetForeColor(float r, float g, float b, float a);
    void SetSelColor(float r, float g, float b, float a);
    void SetBackColor(float r, float g, float b, float a);
    void SetBackSelColor(float r, float g, float b, float a);
    void SetFontSize(uint32_t fontsize);

    void UpdateAllBounds();
    void RefreshAllControls();
};

#endif // _pyGUIDialog_h_
