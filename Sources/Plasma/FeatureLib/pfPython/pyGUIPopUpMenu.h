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
#ifndef _pyGUIPopUpMenu_h_
#define _pyGUIPopUpMenu_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIPopUpMenu   - a wrapper class to provide interface to modifier
//                   attached to a GUI Pop-Up Menu
//
//////////////////////////////////////////////////////////////////////

#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"

#include "pyGlueDefinitions.h"

class pfGUIPopUpMenu;
class pyColor;
class pyKey;
namespace ST { class string; }

class pyGUIPopUpMenu
{
private:
    plKey                   fGCkey;
    
    pfGUIPopUpMenu          *fBuiltMenu;

protected:
    pyGUIPopUpMenu(pyKey& gckey);
    pyGUIPopUpMenu(plKey objkey);
    pyGUIPopUpMenu();
    // For creating new menus on the fly
    pyGUIPopUpMenu(const ST::string& name, float screenOriginX, float screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc);
    pyGUIPopUpMenu(const ST::string& name, pyGUIPopUpMenu &parent, float screenOriginX, float screenOriginY);

public:
    ~pyGUIPopUpMenu();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGUIPopUpMenu);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(pyKey& gckey);
    static PyObject *New(plKey objkey);
    static PyObject *New(const ST::string& name, float screenOriginX, float screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc);
    static PyObject *New(const ST::string& name, pyGUIPopUpMenu &parent, float screenOriginX, float screenOriginY);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIPopUpMenu object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIPopUpMenu); // converts a PyObject to a pyGUIPopUpMenu (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // these three are for the python glue only, do NOT call
    void setup(plKey objkey);
    void setup(const ST::string& name, float screenOriginX, float screenOriginY, const plLocation &destLoc = plLocation::kGlobalFixedLoc);
    void setup(const ST::string& name, pyGUIPopUpMenu &parent, float screenOriginX, float screenOriginY);

    static bool IsGUIPopUpMenu(const plKey& key);

    // override the equals to operator
    bool operator==(const pyGUIPopUpMenu &gdobj) const;
    bool operator!=(const pyGUIPopUpMenu &gdobj) const { return !(gdobj == *this); }

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

    void Show();
    void Hide();

    // get color schemes
    PyObject* GetForeColor(); // returns pyColor
    PyObject* GetSelColor(); // returns pyColor
    PyObject* GetBackColor(); // returns pyColor
    PyObject* GetBackSelColor(); // returns pyColor
    // set color scheme
    void SetForeColor(float r, float g, float b, float a);
    void SetSelColor(float r, float g, float b, float a);
    void SetBackColor(float r, float g, float b, float a);
    void SetBackSelColor(float r, float g, float b, float a);

    // Menu item functions
    void AddConsoleCmdItem(const ST::string& name, const ST::string& consoleCmd);
    void AddNotifyItem(const ST::string& name);
    void AddSubMenuItem(const ST::string& name, pyGUIPopUpMenu &subMenu);
};

#endif // _pyGUIPopUpMenu_h_
