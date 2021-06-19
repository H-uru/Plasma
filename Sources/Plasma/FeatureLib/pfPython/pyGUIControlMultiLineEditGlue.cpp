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

#include "pyGUIControlMultiLineEdit.h"
#include "pyEnum.h"
#include "pyColor.h"

#include "pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlMultiLineEdit, pyGUIControlMultiLineEdit);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlMultiLineEdit, pyGUIControlMultiLineEdit)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlMultiLineEdit)

PYTHON_INIT_DEFINITION(ptGUIControlMultiLineEdit, args, keywords)
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

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, clickable, Clickable)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, unclickable, Unclickable)

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setScrollPosition, args)
{
    long topLine;
    if (!PyArg_ParseTuple(args, "l", &topLine))
    {
        PyErr_SetString(PyExc_TypeError, "setScrollPosition expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetScrollPosition(topLine);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getScrollPosition)
{
    return PyLong_FromLong(self->fThis->GetScrollPosition());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, isAtEnd)
{
    PYTHON_RETURN_BOOL(self->fThis->IsAtEnd());
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, moveCursor, args)
{
    long dir;
    if (!PyArg_ParseTuple(args, "l", &dir))
    {
        PyErr_SetString(PyExc_TypeError, "moveCursor expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->MoveCursor(dir);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, clearBuffer, ClearBuffer)

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setString, args)
{
    char* text;
    if (!PyArg_ParseTuple(args, "s", &text))
    {
        PyErr_SetString(PyExc_TypeError, "setString expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetText(text);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setStringW, args)
{
    PyObject* textObj;
    if (!PyArg_ParseTuple(args, "O", &textObj))
    {
        PyErr_SetString(PyExc_TypeError, "setStringW expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(textObj))
    {
        wchar_t* temp = PyUnicode_AsWideCharString(textObj, nullptr);
        self->fThis->SetTextW(temp);
        PyMem_Free(temp);
        PYTHON_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "setStringW expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getString)
{
    return PyUnicode_FromString(self->fThis->GetText());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getStringW)
{
    const wchar_t* text = self->fThis->GetTextW();
    return PyUnicode_FromWideChar(text, wcslen(text));
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setEncodedBuffer, args)
{
    PyObject* bufferObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &bufferObj))
    {
        PyErr_SetString(PyExc_TypeError, "setEncodedBuffer expects a python buffer object");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetEncodedBuffer(bufferObj);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setEncodedBufferW, args)
{
    PyObject* bufferObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &bufferObj))
    {
        PyErr_SetString(PyExc_TypeError, "setEncodedBufferW expects a python buffer object");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetEncodedBufferW(bufferObj);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getEncodedBuffer)
{
    const char* buffer = self->fThis->GetEncodedBuffer();
    PyObject* retVal = PyUnicode_FromString(buffer);
    delete [] buffer;
    return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getEncodedBufferW)
{
    const wchar_t* text = self->fThis->GetEncodedBufferW();
    PyObject* retVal = PyUnicode_FromWideChar(text, wcslen(text));
    delete [] text;
    return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getBufferSize)
{
    return PyLong_FromUnsignedLong(self->fThis->GetBufferSize());
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertChar, args)
{
    char c;
    if (!PyArg_ParseTuple(args, "c", &c))
    {
        PyErr_SetString(PyExc_TypeError, "insertChar expects a single character");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->InsertChar(c);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertCharW, args)
{
    PyObject* textObj;
    if (!PyArg_ParseTuple(args, "O", &textObj))
    {
        PyErr_SetString(PyExc_TypeError, "insertCharW expects a single unicode character");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(textObj))
    {
        Py_ssize_t strLen;
        wchar_t* temp = PyUnicode_AsWideCharString(textObj, &strLen);
        if (strLen != 1)
        {
            PyErr_SetString(PyExc_TypeError, "insertCharW expects a single unicode character");
            PyMem_Free(temp);
            PYTHON_RETURN_ERROR;
        }

        self->fThis->InsertCharW(temp[0]);
        PyMem_Free(temp);
        PYTHON_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "insertCharW expects a single unicode character");
        PYTHON_RETURN_ERROR;
    }
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertString, args)
{
    char* str;
    if (!PyArg_ParseTuple(args, "s", &str))
    {
        PyErr_SetString(PyExc_TypeError, "insertString expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->InsertString(str);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertStringW, args)
{
    PyObject* textObj;
    if (!PyArg_ParseTuple(args, "O", &textObj))
    {
        PyErr_SetString(PyExc_TypeError, "insertStringW expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(textObj))
    {
        wchar_t* temp = PyUnicode_AsWideCharString(textObj, nullptr);
        self->fThis->InsertStringW(temp);
        PyMem_Free(temp);
        PYTHON_RETURN_NONE;
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "insertStringW expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertColor, args)
{
    PyObject* colorObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "insertColor expects a ptColor");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "insertColor expects a ptColor");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color = pyColor::ConvertFrom(colorObj);
    self->fThis->InsertColor(*color);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, insertStyle, args)
{
    char style;
    if (!PyArg_ParseTuple(args, "b", &style))
    {
        PyErr_SetString(PyExc_TypeError, "insertStyle expects a 8-bit integer");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->InsertStyle(style);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, deleteChar, DeleteChar)

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, lock, Lock)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, unlock, Unlock)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, isLocked)
{
    PYTHON_RETURN_BOOL(self->fThis->IsLocked());
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setBufferLimit, args)
{
    long limit;
    if (!PyArg_ParseTuple(args, "l", &limit))
    {
        PyErr_SetString(PyExc_TypeError, "setBufferLimit expects a long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetBufferLimit(limit);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getBufferLimit)
{
    return PyLong_FromLong(self->fThis->GetBufferLimit());
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, enableScrollControl, EnableScrollControl)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, disableScrollControl, DisableScrollControl)

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, deleteLinesFromTop, args)
{
    long lines;
    if (!PyArg_ParseTuple(args, "l", &lines))
    {
        PyErr_SetString(PyExc_TypeError, "deleteLinesFromTop expects an int");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->DeleteLinesFromTop(lines);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlMultiLineEdit, getFontSize)
{
    return PyLong_FromUnsignedLong(self->fThis->GetFontSize());
}

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, setFontSize, args)
{
    unsigned long fontSize;
    if (!PyArg_ParseTuple(args, "l", &fontSize))
    {
        PyErr_SetString(PyExc_TypeError, "setFontSize expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetFontSize(fontSize);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlMultiLineEdit, beginUpdate, BeginUpdate)

PYTHON_METHOD_DEFINITION(ptGUIControlMultiLineEdit, endUpdate, args)
{
    bool redraw = true;
    if (!PyArg_ParseTuple(args, "|b", &redraw)) {
        PyErr_SetString(PyExc_TypeError, "endUpdate expects an optional boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->EndUpdate(redraw);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGUIControlMultiLineEdit)
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, clickable, "Sets this listbox to be clickable by the user."),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, unclickable, "Makes this listbox not clickable by the user.\n"
                "Useful when just displaying a list that is not really selectable."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setScrollPosition, "Params: topLine\nSets the what line is the top line."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getScrollPosition, "Gets what line is the top line."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, isAtEnd, "Returns true if the end of the buffer has been reached."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, moveCursor, "Params: direction\nMove the cursor in the specified direction (see PtGUIMultiLineDirection)"),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, clearBuffer, "Clears all text from the multi-line edit control."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setString, "Params: asciiText\nSets the multi-line edit control string."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setStringW, "Params: unicodeText\nUnicode version of setString."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getString, "Gets the string of the edit control."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getStringW, "Unicode version of getString."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setEncodedBuffer, "Params: bufferObject\nSets the edit control to the encoded buffer in the python buffer object. Do NOT use with a result from getEncodedBufferW."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setEncodedBufferW, "Params: bufferObject\nUnicode version of setEncodedBuffer. Do NOT use with a result from getEncodedBuffer."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getEncodedBuffer, "Returns the encoded buffer in a python buffer object. Do NOT use result with setEncodedBufferW."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getEncodedBufferW, "Unicode version of getEncodedBuffer. Do NOT use result with setEncodedBuffer."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getBufferSize, "Returns the size of the buffer"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertChar, "Params: c\nInserts a character at the current cursor position."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertCharW, "Params: c\nUnicode version of insertChar."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertString, "Params: string\nInserts a string at the current cursor position."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertStringW, "Params: string\nUnicode version of insertString"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertColor, "Params: color\nInserts an encoded color object at the current cursor position.\n"
                "'color' is a ptColor object."),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, insertStyle, "Params: style\nInserts an encoded font style at the current cursor position."),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, deleteChar, "Deletes a character at the current cursor position."),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, lock, "Locks the multi-line edit control so the user cannot make changes."),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, unlock, "Unlocks the multi-line edit control so that the user can make changes."),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, isLocked, "Is the multi-line edit control locked? Returns 1 if true otherwise returns 0"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setBufferLimit, "Params: bufferLimit\nSets the buffer max for the editbox"),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getBufferLimit, "Returns the current buffer limit"),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, enableScrollControl, "Enables the scroll control if there is one"),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, disableScrollControl, "Disables the scroll control if there is one"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, deleteLinesFromTop, "Params: numLines\nDeletes the specified number of lines from the top of the text buffer"),
    PYTHON_METHOD_NOARGS(ptGUIControlMultiLineEdit, getFontSize, "Returns the current default font size"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, setFontSize, "Params: fontSize\nSets the default font size for the edit control"),
    PYTHON_BASIC_METHOD(ptGUIControlMultiLineEdit, beginUpdate, "Signifies that the control will be updated heavily starting now, so suppress all redraws"),
    PYTHON_METHOD(ptGUIControlMultiLineEdit, endUpdate, "Signifies that the massive updates are over. We can now redraw."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlMultiLineEdit, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Multi-line edit class");

// required functions for PyObject interoperability
PyObject *pyGUIControlMultiLineEdit::New(pyKey& gckey)
{
    ptGUIControlMultiLineEdit *newObj = (ptGUIControlMultiLineEdit*)ptGUIControlMultiLineEdit_type.tp_new(&ptGUIControlMultiLineEdit_type, nullptr, nullptr);
    newObj->fThis->fGCkey = gckey.getKey();
    return (PyObject*)newObj;
}

PyObject *pyGUIControlMultiLineEdit::New(plKey objkey)
{
    ptGUIControlMultiLineEdit *newObj = (ptGUIControlMultiLineEdit*)ptGUIControlMultiLineEdit_type.tp_new(&ptGUIControlMultiLineEdit_type, nullptr, nullptr);
    newObj->fThis->fGCkey = std::move(objkey);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlMultiLineEdit, pyGUIControlMultiLineEdit)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlMultiLineEdit, pyGUIControlMultiLineEdit)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlMultiLineEdit::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGUIControlMultiLineEdit);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyGUIControlMultiLineEdit::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(PtGUIMultiLineDirection)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kLineStart,        pfGUIMultiLineEditCtrl::kLineStart)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kLineEnd,          pfGUIMultiLineEditCtrl::kLineEnd)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kBufferStart,      pfGUIMultiLineEditCtrl::kBufferStart)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kBufferEnd,        pfGUIMultiLineEditCtrl::kBufferEnd)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneBack,          pfGUIMultiLineEditCtrl::kOneBack)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneForward,       pfGUIMultiLineEditCtrl::kOneForward)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneWordBack,      pfGUIMultiLineEditCtrl::kOneWordBack)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneWordForward,   pfGUIMultiLineEditCtrl::kOneWordForward)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneLineUp,        pfGUIMultiLineEditCtrl::kOneLineUp)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kOneLineDown,      pfGUIMultiLineEditCtrl::kOneLineDown)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kPageUp,           pfGUIMultiLineEditCtrl::kPageUp)
    PYTHON_ENUM_ELEMENT(PtGUIMultiLineDirection, kPageDown,         pfGUIMultiLineEditCtrl::kPageDown)
    PYTHON_ENUM_END(m, PtGUIMultiLineDirection)
}
