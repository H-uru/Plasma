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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "pyGUIControlTextBox.h"
#include "pyColor.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlTextBox, pyGUIControlTextBox);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlTextBox, pyGUIControlTextBox)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlTextBox)

PYTHON_INIT_DEFINITION(ptGUIControlTextBox, args, keywords)
{
	PyObject *keyObject = NULL;
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

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setString, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setStringW, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "setStringW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* temp = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, temp, strLen);
		temp[strLen] = L'\0';
		self->fThis->SetTextW(temp);
		delete [] temp;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* temp = PyString_AsString(textObj);
		self->fThis->SetText(temp);
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "setStringW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlTextBox, getString)
{
	return PyString_FromString(self->fThis->GetText().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlTextBox, getStringW)
{
	std::wstring retVal = self->fThis->GetTextW();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setFontSize, args)
{
	unsigned char fontSize;
	if (!PyArg_ParseTuple(args, "b", &fontSize))
	{
		PyErr_SetString(PyExc_TypeError, "setFontSize expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetFontSize(fontSize);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setForeColor, args)
{
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setForeColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setForeColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->SetForeColor(*color);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setBackColor, args)
{
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setBackColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setBackColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->SetBackColor(*color);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlTextBox, setStringJustify, args)
{
	unsigned char justify;
	if (!PyArg_ParseTuple(args, "b", &justify))
	{
		PyErr_SetString(PyExc_TypeError, "setStringJustify expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetJustify(justify);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlTextBox, getStringJustify)
{
	return PyInt_FromLong(self->fThis->GetJustify());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlTextBox, getForeColor)
{
	return self->fThis->GetForeColor();
}

PYTHON_START_METHODS_TABLE(ptGUIControlTextBox)
	PYTHON_METHOD(ptGUIControlTextBox, setString, "Params: text\nSets the textbox string to 'text'"),
	PYTHON_METHOD(ptGUIControlTextBox, setStringW, "Params: text\nUnicode version of setString"),
	PYTHON_METHOD_NOARGS(ptGUIControlTextBox, getString, "Returns the string that the TextBox is set to (in case you forgot)"),
	PYTHON_METHOD_NOARGS(ptGUIControlTextBox, getStringW, "Unicode version of getString"),
	PYTHON_METHOD(ptGUIControlTextBox, setFontSize, "Params: size\nDon't use"),
	PYTHON_METHOD(ptGUIControlTextBox, setForeColor, "Params: color\nSets the text forecolor to 'color', which is a ptColor object."),
	PYTHON_METHOD(ptGUIControlTextBox, setBackColor, "Params: color\nSets the text backcolor to 'color', which is a ptColor object."),
	PYTHON_METHOD(ptGUIControlTextBox, setStringJustify, "Params: justify\nSets current justify"),
	PYTHON_METHOD_NOARGS(ptGUIControlTextBox, getStringJustify, "Returns current justify"),
	PYTHON_METHOD_NOARGS(ptGUIControlTextBox, getForeColor, "Returns the current forecolor"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlTextBox, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Textbox class");

// required functions for PyObject interoperability
PyObject *pyGUIControlTextBox::New(pyKey& gckey)
{
	ptGUIControlTextBox *newObj = (ptGUIControlTextBox*)ptGUIControlTextBox_type.tp_new(&ptGUIControlTextBox_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlTextBox::New(plKey objkey)
{
	ptGUIControlTextBox *newObj = (ptGUIControlTextBox*)ptGUIControlTextBox_type.tp_new(&ptGUIControlTextBox_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlTextBox, pyGUIControlTextBox)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlTextBox, pyGUIControlTextBox)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlTextBox::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlTextBox);
	PYTHON_CLASS_IMPORT_END(m);
}