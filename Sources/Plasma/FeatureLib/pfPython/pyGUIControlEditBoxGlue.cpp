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
#include "pyGUIControlEditBox.h"
#include "pyColor.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlEditBox, pyGUIControlEditBox);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlEditBox, pyGUIControlEditBox)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlEditBox)

PYTHON_INIT_DEFINITION(ptGUIControlEditBox, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setStringSize, args)
{
	unsigned long strLen;
	if (!PyArg_ParseTuple(args, "l", &strLen))
	{
		PyErr_SetString(PyExc_TypeError, "setStringSize expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetBufferSize(strLen);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlEditBox, getString)
{
	return PyString_FromString(self->fThis->GetBuffer().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlEditBox, getStringW)
{
	std::wstring val = self->fThis->GetBufferW();
	return PyUnicode_FromWideChar(val.c_str(), val.length());
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlEditBox, clearString, ClearBuffer)

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setString, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setStringW, args)
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
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->SetTextW(text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		self->fThis->SetText(text);
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "setStringW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlEditBox, home, SetCursorToHome)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlEditBox, end, SetCursorToEnd)

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setColor, args)
{
	PyObject* foreColorObj = NULL;
	PyObject* backColorObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &foreColorObj, &backColorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setColor expects two ptColor objects");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyColor::Check(foreColorObj)) || (!pyColor::Check(backColorObj)))
	{
		PyErr_SetString(PyExc_TypeError, "setColor expects two ptColor objects");
		PYTHON_RETURN_ERROR;
	}
	pyColor* foreColor = pyColor::ConvertFrom(foreColorObj);
	pyColor* backColor = pyColor::ConvertFrom(backColorObj);
	self->fThis->SetColor(*foreColor, *backColor);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setSelectionColor, args)
{
	PyObject* foreColorObj = NULL;
	PyObject* backColorObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &foreColorObj, &backColorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setSelectionColor expects two ptColor objects");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyColor::Check(foreColorObj)) || (!pyColor::Check(backColorObj)))
	{
		PyErr_SetString(PyExc_TypeError, "setSelectionColor expects two ptColor objects");
		PYTHON_RETURN_ERROR;
	}
	pyColor* foreColor = pyColor::ConvertFrom(foreColorObj);
	pyColor* backColor = pyColor::ConvertFrom(backColorObj);
	self->fThis->SetSelColor(*foreColor, *backColor);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlEditBox, wasEscaped)
{
	PYTHON_RETURN_BOOL(self->fThis->WasEscaped());
}

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setSpecialCaptureKeyMode, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setSpecialCaptureKeyMode expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSpecialCaptureKeyMode(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlEditBox, getLastKeyCaptured)
{
	return PyInt_FromLong(self->fThis->GetLastKeyCaptured());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlEditBox, getLastModifiersCaptured)
{
	return PyInt_FromLong(self->fThis->GetLastModifiersCaptured());
}

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setLastKeyCapture, args)
{
	unsigned long key;
	unsigned long modifiers;
	if (!PyArg_ParseTuple(args, "ll", &key, &modifiers))
	{
		PyErr_SetString(PyExc_TypeError, "setLastKeyCapture expects two unsigned longs");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLastKeyCapture(key, modifiers);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlEditBox, setChatMode, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setChatMode expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetChatMode(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGUIControlEditBox)
	PYTHON_METHOD(ptGUIControlEditBox, setStringSize, "Params: size\nSets the maximum size of the string that can be inputted by the user."),
	PYTHON_METHOD_NOARGS(ptGUIControlEditBox, getString, "Returns the sting that the user typed in."),
	PYTHON_METHOD_NOARGS(ptGUIControlEditBox, getStringW, "Unicode version of getString."),
	PYTHON_BASIC_METHOD(ptGUIControlEditBox, clearString, "Clears the editbox."),
	PYTHON_METHOD(ptGUIControlEditBox, setString, "Params: text\nPre-sets the editbox to a atring."),
	PYTHON_METHOD(ptGUIControlEditBox, setStringW, "Params: text\nUnicode version of setString."),
	PYTHON_BASIC_METHOD(ptGUIControlEditBox, home, "Sets the cursor in the editbox to before the first character."),
	PYTHON_BASIC_METHOD(ptGUIControlEditBox, end, "Sets the cursor in the editbox to the after the last character."),
	PYTHON_METHOD(ptGUIControlEditBox, setColor, "Params: foreColor,backColor\nSets the fore and back color of the editbox."),
	PYTHON_METHOD(ptGUIControlEditBox, setSelectionColor, "Params: foreColor,backColor\nSets the selection color of the editbox."),
	PYTHON_METHOD_NOARGS(ptGUIControlEditBox, wasEscaped, "If the editbox was escaped then return 1 else return 0"),
	PYTHON_METHOD(ptGUIControlEditBox, setSpecialCaptureKeyMode, "Params: state\nSet the Capture mode on this control"),
	PYTHON_METHOD_NOARGS(ptGUIControlEditBox, getLastKeyCaptured, "Gets the last capture key"),
	PYTHON_METHOD_NOARGS(ptGUIControlEditBox, getLastModifiersCaptured, "Gets the last modifiers flags captured"),
	PYTHON_METHOD(ptGUIControlEditBox, setLastKeyCapture, "Params: key, modifiers\nSet last key captured"),
	PYTHON_METHOD(ptGUIControlEditBox, setChatMode, "Params: state\nSet the Chat mode on this control"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlEditBox, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Editbox class");

// required functions for PyObject interoperability
PyObject *pyGUIControlEditBox::New(pyKey& gckey)
{
	ptGUIControlEditBox *newObj = (ptGUIControlEditBox*)ptGUIControlEditBox_type.tp_new(&ptGUIControlEditBox_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlEditBox::New(plKey objkey)
{
	ptGUIControlEditBox *newObj = (ptGUIControlEditBox*)ptGUIControlEditBox_type.tp_new(&ptGUIControlEditBox_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlEditBox, pyGUIControlEditBox)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlEditBox, pyGUIControlEditBox)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlEditBox::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlEditBox);
	PYTHON_CLASS_IMPORT_END(m);
}