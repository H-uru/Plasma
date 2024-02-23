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
#ifndef _pyGUIControlListBox_h_
#define _pyGUIControlListBox_h_

//////////////////////////////////////////////////////////////////////
//
// pyGUIControlListBox   - a wrapper class to provide interface to modifier
//                      attached to a GUIControl (such as Button, ListBox, etc.)
//
//////////////////////////////////////////////////////////////////////

#include <vector>

#include "pyGUIControl.h"
#include "pyGlueDefinitions.h"

class plKey;
class pyColor;
class pyImage;
class pyKey;

class pfGUIListTreeRoot;
namespace ST { class string; }

class pyGUIControlListBox : public pyGUIControl
{
private:
    std::vector<pfGUIListTreeRoot *>   fBuildRoots;

protected:
    pyGUIControlListBox(): pyGUIControl() {} // for python glue, do NOT call
    pyGUIControlListBox(pyKey& gckey);
    pyGUIControlListBox(plKey objkey);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGUIControlListBox);
    static PyObject *New(pyKey& gckey);
    static PyObject *New(plKey objkey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGUIControlListBox object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGUIControlListBox); // converts a PyObject to a pyGUIControlListBox (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    static bool IsGUIControlListBox(const plKey& key);

    // special case control for the listbox
    // ...this allows the listbox to be used without being selectable
    void Clickable();
    void Unclickable();
    int32_t GetSelection();
    void SetSelection(int32_t item);
    void SetElement(uint16_t idx, ST::string text);
    void RemoveElement(uint16_t index);
    void ClearAllElements();
    uint16_t GetNumElements();
    ST::string GetElement(uint16_t idx);
    int16_t AddString(ST::string string);
    int16_t AddImage(pyImage& image, bool respectAlpha);
    int16_t AddImageInBox(pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool respectAlpha);
    int16_t AddImageAndSwatchesInBox(pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool respectAlpha,
                                     pyColor &primary, pyColor &secondary);
    void SetSwatchSize(uint32_t size);
    void SetSwatchEdgeOffset(uint32_t size);
    void SetStringJustify(uint16_t idx, uint32_t justify);
    int16_t FindString(ST::string toCompareTo);
    int16_t AddTextWColorW(ST::string str, pyColor& textcolor, uint32_t inheritalpha);
    int16_t AddTextWColorWSizeW(ST::string str, pyColor& textcolor, uint32_t inheritalpha, int32_t fontsize);
    void Add2TextWColorW(ST::string str1, pyColor& textcolor1, ST::string str2, pyColor& textcolor2, uint32_t inheritalpha);
    int16_t AddStringInBox(ST::string string, uint32_t min_width, uint32_t min_height);
    void ScrollToBegin();
    void ScrollToEnd();
    void SetScrollPos(int32_t pos);
    int32_t GetScrollPos();
    int32_t GetScrollRange();

    void LockList();
    void UnlockList();

    // To create tree branches, call AddBranch() with a name, then add elements as usual, including new sub-branches
    // via additional AddBranch() calls. Call CloseBranch() to stop writing elements to that branch.
    void            AddBranch( ST::string name, bool initiallyOpen );
    void            CloseBranch();

    void            RemoveSelection( int32_t item );
    void            AddSelection( int32_t item );
    PyObject*       GetSelectionList();
    PyObject*       GetBranchList();
    
    void            AllowNoSelect();
    void            DisallowNoSelect();

};

#endif // _pyGUIControlListBox_h_
