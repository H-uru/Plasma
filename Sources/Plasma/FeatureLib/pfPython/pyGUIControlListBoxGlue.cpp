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

#include "pyGUIControlListBox.h"
#include "pyImage.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlListBox, pyGUIControlListBox);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlListBox, pyGUIControlListBox)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlListBox)

PYTHON_INIT_DEFINITION(ptGUIControlListBox, args, keywords)
{
    PyObject *keyObject = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObject))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (!pyKey::Check(keyObject))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }

    pyKey *key = pyKey::ConvertFrom(keyObject);
    self->fThis->setKey(key->getKey());

    PYTHON_RETURN_INIT_OK;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, clickable, Clickable)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, unclickable, Unclickable)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getSelection)
{
    return PyLong_FromLong(self->fThis->GetSelection());
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setSelection, args)
{
    long selectionIndex;
    if (!PyArg_ParseTuple(args, "l", &selectionIndex))
    {
        PyErr_SetString(PyExc_TypeError, "setSelection expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetSelection(selectionIndex);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, refresh, Refresh)

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, removeElement, args)
{
    unsigned short index;
    if (!PyArg_ParseTuple(args, "h", &index))
    {
        PyErr_SetString(PyExc_TypeError, "removeElement expects an unsigned short");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RemoveElement(index);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, clearAllElements, ClearAllElements)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getNumElements)
{
    return PyLong_FromLong(self->fThis->GetNumElements());
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setElement, args)
{
    unsigned short index;
    ST::string text;
    if (!PyArg_ParseTuple(args, "hO&", &index, PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "setElement expects an unsigned short and a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetElement(index, std::move(text));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setElementW, args)
{
    unsigned short index;
    ST::string text;
    if (!PyArg_ParseTuple(args, "hO&", &index, PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "setElementW expects an unsigned short and a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetElement(index, std::move(text));
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, getElement, args)
{
    unsigned short index;
    if (!PyArg_ParseTuple(args, "h", &index))
    {
        PyErr_SetString(PyExc_TypeError, "getElement expects an unsigned short");
        PYTHON_RETURN_ERROR;
    }
    return PyUnicode_FromSTString(self->fThis->GetElement(index));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, getElementW, args)
{
    unsigned short index;
    if (!PyArg_ParseTuple(args, "h", &index))
    {
        PyErr_SetString(PyExc_TypeError, "getElementW expects an unsigned short");
        PYTHON_RETURN_ERROR;
    }
    return PyUnicode_FromSTString(self->fThis->GetElement(index));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setStringJustify, args)
{
    unsigned short index;
    unsigned long justify;
    if (!PyArg_ParseTuple(args, "hl", &index, &justify))
    {
        PyErr_SetString(PyExc_TypeError, "setStringJustify expects an unsigned short and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetStringJustify(index, justify);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addString, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "addString expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->AddString(std::move(text)));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addStringW, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "addStringW expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->AddString(std::move(text)));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, findString, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "findString expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->FindString(std::move(text)));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, findStringW, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "findStringW expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->FindString(std::move(text)));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addImage, args)
{
    PyObject* imageObj = nullptr;
    char respectAlphaFlag;
    if (!PyArg_ParseTuple(args, "Ob", &imageObj, &respectAlphaFlag))
    {
        PyErr_SetString(PyExc_TypeError, "addImage expects a ptImage and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyImage::Check(imageObj))
    {
        PyErr_SetString(PyExc_TypeError, "addImage expects a ptImage and a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyImage* image = pyImage::ConvertFrom(imageObj);
    return PyLong_FromLong(self->fThis->AddImage(*image, respectAlphaFlag != 0));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addImageInBox, args)
{
    PyObject* imageObj = nullptr;
    unsigned long x, y, width, height;
    char respectAlphaFlag;
    if (!PyArg_ParseTuple(args, "Ollllb", &imageObj, &x, &y, &width, &height, &respectAlphaFlag))
    {
        PyErr_SetString(PyExc_TypeError, "addImageInBox expects a ptImage, four unsigned longs, and a boolean");
        PYTHON_RETURN_ERROR;
    }
    if (!pyImage::Check(imageObj))
    {
        PyErr_SetString(PyExc_TypeError, "addImageInBox expects a ptImage, four unsigned longs, and a boolean");
        PYTHON_RETURN_ERROR;
    }
    pyImage* image = pyImage::ConvertFrom(imageObj);
    return PyLong_FromLong(self->fThis->AddImageInBox(*image, x, y, width, height, respectAlphaFlag != 0));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addStringWithColor, args)
{
    ST::string text;
    PyObject* colorObj = nullptr;
    unsigned long inheritAlpha;
    if (!PyArg_ParseTuple(args, "O&Ol", PyUnicode_STStringConverter, &text, &colorObj, &inheritAlpha))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColor expects a string, ptColor, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColor expects a string, ptColor, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color = pyColor::ConvertFrom(colorObj);
    return PyLong_FromLong(self->fThis->AddTextWColorW(std::move(text), *color, inheritAlpha));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addStringWithColorWithSize, args)
{
    ST::string text;
    PyObject* colorObj = nullptr;
    unsigned long inheritAlpha;
    long textSize;
    if (!PyArg_ParseTuple(args, "O&Oll", PyUnicode_STStringConverter, &text, &colorObj, &inheritAlpha, &textSize))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColorWithSize expects a string, ptColor, an unsigned long, and a long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColorWithSize expects a string, ptColor, an unsigned long, and a long");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color = pyColor::ConvertFrom(colorObj);
    return PyLong_FromLong(self->fThis->AddTextWColorWSizeW(std::move(text), *color, inheritAlpha, textSize));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, add2StringsWithColors, args)
{
    ST::string text1;
    PyObject* color1Obj = nullptr;
    ST::string text2;
    PyObject* color2Obj = nullptr;
    unsigned long inheritAlpha;
    if (!PyArg_ParseTuple(args, "O&OO&Ol", PyUnicode_STStringConverter, &text1, &color1Obj, PyUnicode_STStringConverter, &text2, &color2Obj, &inheritAlpha))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColor expects a string, ptColor, string, ptColor, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyColor::Check(color2Obj)) || (!pyColor::Check(color2Obj)))
    {
        PyErr_SetString(PyExc_TypeError, "addStringWithColor expects a string, ptColor, string, ptColor, and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color1 = pyColor::ConvertFrom(color1Obj);
    pyColor* color2 = pyColor::ConvertFrom(color2Obj);
    self->fThis->Add2TextWColorW(std::move(text1), *color1, std::move(text2), *color2, inheritAlpha);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addStringInBox, args)
{
    ST::string text;
    unsigned long minWidth, minHeight;
    if (!PyArg_ParseTuple(args, "O&ll", PyUnicode_STStringConverter, &text, &minWidth, &minHeight))
    {
        PyErr_SetString(PyExc_TypeError, "addStringInBox expects a string and two unsigned longs");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->AddStringInBox(std::move(text), minWidth, minHeight));
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, scrollToBegin, ScrollToBegin)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, scrollToEnd, ScrollToEnd)

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setScrollPos, args)
{
    short pos;
    if (!PyArg_ParseTuple(args, "h", &pos))
    {
        PyErr_SetString(PyExc_TypeError, "setScrollPos expects a short");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetScrollPos(pos);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getScrollPos)
{
    return PyLong_FromLong(self->fThis->GetScrollPos());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getScrollRange)
{
    return PyLong_FromLong(self->fThis->GetScrollRange());
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, lock, LockList)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, unlock, UnlockList)

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addBranch, args)
{
    ST::string name;
    char initiallyOpen;
    if (!PyArg_ParseTuple(args, "O&b", PyUnicode_STStringConverter, &name, &initiallyOpen))
    {
        PyErr_SetString(PyExc_TypeError, "addBranch expects a string and a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddBranch(std::move(name), initiallyOpen != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addBranchW, args)
{
    ST::string name;
    char initiallyOpen;
    if (!PyArg_ParseTuple(args, "O&b", PyUnicode_STStringConverter, &name, &initiallyOpen))
    {
        PyErr_SetString(PyExc_TypeError, "addBranchW expects a string and a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddBranch(std::move(name), initiallyOpen != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, closeBranch, CloseBranch)

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, removeSelection, args)
{
    long itemIdx;
    if (!PyArg_ParseTuple(args, "l", &itemIdx))
    {
        PyErr_SetString(PyExc_TypeError, "removeSelection expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RemoveSelection(itemIdx);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addSelection, args)
{
    long itemIdx;
    if (!PyArg_ParseTuple(args, "l", &itemIdx))
    {
        PyErr_SetString(PyExc_TypeError, "addSelection expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddSelection(itemIdx);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getSelectionList)
{
    return self->fThis->GetSelectionList();
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, addImageAndSwatchesInBox, args)
{
    PyObject* imageObj = nullptr;
    unsigned long x, y, width, height;
    char respectAlpha;
    PyObject* primaryObj = nullptr;
    PyObject* secondaryObj = nullptr;
    if (!PyArg_ParseTuple(args, "OllllbOO", &imageObj, &x, &y, &width, &height, &respectAlpha, &primaryObj, &secondaryObj))
    {
        PyErr_SetString(PyExc_TypeError, "addImageAndSwatchesInBox expects a ptImage, four unsigned longs, a boolean, and two ptColor objects");
        PYTHON_RETURN_ERROR;
    }
    if ((!pyImage::Check(imageObj)) || (!pyColor::Check(primaryObj)) || (!pyColor::Check(secondaryObj)))
    {
        PyErr_SetString(PyExc_TypeError, "addImageAndSwatchesInBox expects a ptImage, four unsigned longs, a boolean, and two ptColor objects");
        PYTHON_RETURN_ERROR;
    }
    pyImage* image = pyImage::ConvertFrom(imageObj);
    pyColor* primary = pyColor::ConvertFrom(primaryObj);
    pyColor* secondary = pyColor::ConvertFrom(secondaryObj);
    return PyLong_FromLong(self->fThis->AddImageAndSwatchesInBox(*image, x, y, width, height, respectAlpha != 0, *primary, *secondary));
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setGlobalSwatchSize, args)
{
    unsigned long swatchSize;
    if (!PyArg_ParseTuple(args, "l", &swatchSize))
    {
        PyErr_SetString(PyExc_TypeError, "setGlobalSwatchSize expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetSwatchSize(swatchSize);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlListBox, setGlobalSwatchEdgeOffset, args)
{
    unsigned long offset;
    if (!PyArg_ParseTuple(args, "l", &offset))
    {
        PyErr_SetString(PyExc_TypeError, "setGlobalSwatchEdgeOffset expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetSwatchEdgeOffset(offset);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, allowNoSelect, AllowNoSelect)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlListBox, disallowNoSelect, DisallowNoSelect)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlListBox, getBranchList)
{
    return self->fThis->GetBranchList();
}

PYTHON_START_METHODS_TABLE(ptGUIControlListBox)
    PYTHON_BASIC_METHOD(ptGUIControlListBox, clickable, "Sets this listbox to be clickable by the user."),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, unclickable, "Makes this listbox not clickable by the user.\n"
                "Useful when just displaying a list that is not really selectable."),
    PYTHON_METHOD_NOARGS(ptGUIControlListBox, getSelection, "Returns the currently selected list item in the listbox."),
    PYTHON_METHOD(ptGUIControlListBox, setSelection, "Params: selectionIndex\nSets the current selection in the listbox."),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, refresh, "Refresh the display of the listbox (after updating contents)."),
    PYTHON_METHOD(ptGUIControlListBox, removeElement, "Params: index\nRemoves element at 'index' in the listbox."),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, clearAllElements, "Removes all the items from the listbox, making it empty."),
    PYTHON_METHOD_NOARGS(ptGUIControlListBox, getNumElements, "Return the number of items in the listbox."),
    PYTHON_METHOD(ptGUIControlListBox, setElement, "Params: index,text\nSet a particular item in the listbox to a string."),
    PYTHON_METHOD(ptGUIControlListBox, setElementW, "Params: index,text\nUnicode version of setElement."),
    PYTHON_METHOD(ptGUIControlListBox, getElement, "Params: index\nGet the string of the item at 'index' in the listbox."),
    PYTHON_METHOD(ptGUIControlListBox, getElementW, "Params: index\nUnicode version of getElement."),
    PYTHON_METHOD(ptGUIControlListBox, setStringJustify, "Params: index,justify\nSets the text justification"),
    PYTHON_METHOD(ptGUIControlListBox, addString, "Params: text\nAppends a list item 'text' to the listbox."),
    PYTHON_METHOD(ptGUIControlListBox, addStringW, "Params: text\nUnicode version of addString."),
    PYTHON_METHOD(ptGUIControlListBox, findString, "Params: text\nFinds and returns the index of the item that matches 'text' in the listbox."),
    PYTHON_METHOD(ptGUIControlListBox, findStringW, "Params: text\nUnicode version of findString."),
    PYTHON_METHOD(ptGUIControlListBox, addImage, "Params: image,respectAlphaFlag\nAppends an image item to the listbox"),
    PYTHON_METHOD(ptGUIControlListBox, addImageInBox, "Params: image,x,y,width,height,respectAlpha\nAppends an image item to the listbox, centering within the box dimension."),
    PYTHON_METHOD(ptGUIControlListBox, addStringWithColor, "Params: text,color,inheritAlpha\nAdds a colored string to the list box"),
    PYTHON_METHOD(ptGUIControlListBox, addStringWithColorWithSize, "Params: text,color,inheritAlpha,fontsize\nAdds a text list item with a color and different font size"),
    PYTHON_METHOD(ptGUIControlListBox, add2StringsWithColors, "Params: text1,color1,text2,color2,respectAlpha\nDoesn't work right - DONT USE"),
    PYTHON_METHOD(ptGUIControlListBox, addStringInBox, "Params: text,min_width,min_height\nAdds a text list item that has a minimum width and height"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, scrollToBegin, "Scrolls the listbox to the beginning of the list"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, scrollToEnd, "Scrolls the listbox to the end of the list"),
    PYTHON_METHOD(ptGUIControlListBox, setScrollPos, "Params: pos\nSets the scroll position of the listbox to 'pos'"),
    PYTHON_METHOD_NOARGS(ptGUIControlListBox, getScrollPos, "Returns the current scroll position in the listbox."),
    PYTHON_METHOD_NOARGS(ptGUIControlListBox, getScrollRange, "Returns the max scroll position"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, lock, "Locks the updates to a listbox, so a number of operations can be performed\n"
                "NOTE: an unlock() call must be made before the next lock() can be."),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, unlock, "Unlocks updates to a listbox and does any saved up changes"),
    PYTHON_METHOD(ptGUIControlListBox, addBranch, "Params: name,initiallyOpen\nUNKNOWN"),
    PYTHON_METHOD(ptGUIControlListBox, addBranchW, "Params: name,initiallyOpen\nUnicode version of addBranch"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, closeBranch, "UNKNOWN"),
    PYTHON_METHOD(ptGUIControlListBox, removeSelection, "Params: item\nRemoves item from selection list"),
    PYTHON_METHOD(ptGUIControlListBox, addSelection, "Params: item\nAdds item to selection list"),
    PYTHON_METHOD(ptGUIControlListBox, getSelectionList, "Returns the current selection list"),
    PYTHON_METHOD(ptGUIControlListBox, addImageAndSwatchesInBox, "Params: image,x,y,width,height,respectAlpha,primary,secondary\nAdd the image and color swatches to the list"),
    PYTHON_METHOD(ptGUIControlListBox, setGlobalSwatchSize, "Params: size\nSets the size of the color swatches"),
    PYTHON_METHOD(ptGUIControlListBox, setGlobalSwatchEdgeOffset, "Params: offset\nSets the edge offset of the color swatches"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, allowNoSelect, "Allows the listbox to have no selection"),
    PYTHON_BASIC_METHOD(ptGUIControlListBox, disallowNoSelect, "The listbox must always have a selection"),
    PYTHON_METHOD_NOARGS(ptGUIControlListBox, getBranchList, "get a list of branches in this list (index,isShowingChildren)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlListBox, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control List Box class");

// required functions for PyObject interoperability
PyObject *pyGUIControlListBox::New(pyKey& gckey)
{
    ptGUIControlListBox *newObj = (ptGUIControlListBox*)ptGUIControlListBox_type.tp_new(&ptGUIControlListBox_type, nullptr, nullptr);
    newObj->fThis->fGCkey = gckey.getKey();
    return (PyObject*)newObj;
}

PyObject *pyGUIControlListBox::New(plKey objkey)
{
    ptGUIControlListBox *newObj = (ptGUIControlListBox*)ptGUIControlListBox_type.tp_new(&ptGUIControlListBox_type, nullptr, nullptr);
    newObj->fThis->fGCkey = std::move(objkey);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlListBox, pyGUIControlListBox)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlListBox, pyGUIControlListBox)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlListBox::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGUIControlListBox);
    PYTHON_CLASS_IMPORT_END(m);
}
