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
#ifndef _pyGUIControl_h_
#define _pyGUIControl_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControl   - a base wrapper class to provide interface to modifier
//                   attached to a GUIControl (base class for the GUI controls)
//
//////////////////////////////////////////////////////////////////////

#include "pyGlueHelpers.h"
#include "pnKeyedObject/plKey.h"

class pyGUIDialog;
class pyColor;
class pyPoint3;

class pyGUIControl
{
protected:
    plKey                   fGCkey;
    
    pyGUIControl() = default; // only used by python glue, do NOT call
    pyGUIControl(pyKey& gckey);
    pyGUIControl(plKey objkey);
    // copy constructor
    pyGUIControl(const pyGUIControl& other);

public:
    virtual ~pyGUIControl() { }

    pyGUIControl& operator=(const pyGUIControl& other);
    pyGUIControl& Copy(const pyGUIControl& other);

    void setKey(plKey key) {fGCkey = std::move(key);} // only used by python glue, do NOT call

    // required functions for PyObject interoperability
    PYTHON_EXPOSE_TYPE; // so we can subclass
    PYTHON_CLASS_NEW_FRIEND(ptGUIControl);
    static PyObject *New(pyKey& gckey);
    static PyObject *New(plKey objkey);
    static PyObject *New(const pyGUIControl& other);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControl object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControl); // converts a PyObject to a pyGUIControl (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // override the equals to operator
    bool operator==(const pyGUIControl &gcobj) const;
    bool operator!=(const pyGUIControl &gcobj) const { return !(gcobj == *this);  }

    // getter and setters
    virtual plKey getObjKey();
    virtual PyObject* getObjPyKey(); // returns pyKey

    // interface functions
    virtual uint32_t  GetTagID();
    virtual void    SetEnabled( bool e );
    virtual void    Enable() { SetEnabled(true); }
    virtual void    Disable() { SetEnabled(false); }
    virtual bool    IsEnabled();
    virtual void    SetFocused( bool e );
    virtual void    Focus() { SetFocused(true); }
    virtual void    UnFocus() { SetFocused(false); }
    virtual bool    IsFocused();
    virtual void    SetVisible( bool vis );
    virtual void    Show() { SetVisible(true); }
    virtual void    Hide() { SetVisible(false); }
    virtual bool    IsVisible();
    virtual bool    IsInteresting();
    virtual void    SetNotifyOnInteresting( bool state );
    virtual void    Refresh();
    virtual void    SetObjectCenter( pyPoint3& pt);
    virtual PyObject* GetObjectCenter(); // returns pyPoint3
    virtual PyObject* GetOwnerDlg(); // returns pyGUIDialog

    // get color schemes
    virtual PyObject*   GetForeColor() const; // returns pyColor
    virtual PyObject*   GetSelColor() const; // returns pyColor
    virtual PyObject*   GetBackColor() const; // returns pyColor
    virtual PyObject*   GetBackSelColor() const; // returns pyColor
    virtual uint32_t    GetFontSize() const;
    virtual uint8_t     GetFontFlags() const;
    // set color scheme
    virtual void        SetForeColor( float r, float g, float b, float a );
    virtual void        SetSelColor( float r, float g, float b, float a );
    virtual void        SetBackColor( float r, float g, float b, float a );
    virtual void        SetBackSelColor( float r, float g, float b, float a );
    virtual void        SetFontSize(uint32_t fontsize);
    virtual void        SetFontFlags(uint8_t fontflags);

};

#endif // _pyGUIControl_h_
