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
#include "pyStream.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptStream, pyStream);

PYTHON_DEFAULT_NEW_DEFINITION(ptStream, pyStream)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptStream)

PYTHON_INIT_DEFINITION(ptStream, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptStream, open, args)
{
	PyObject* filenameObj;
	PyObject* flagsObj;
	if (!PyArg_ParseTuple(args, "OO", &filenameObj, &flagsObj))
	{
		PyErr_SetString(PyExc_TypeError, "open expects two strings");
		PYTHON_RETURN_ERROR;
	}

	std::wstring filename;
	if (PyUnicode_Check(filenameObj))
	{
		int strLen = PyUnicode_GetSize(filenameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)filenameObj, text, strLen);
		text[strLen] = L'\0';
		filename = text;
		delete [] text;
	}
	else if (PyString_Check(filenameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(filenameObj);
		wchar_t* wText = hsStringToWString(text);
		filename = wText;
		delete [] wText;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "open expects two strings");
		PYTHON_RETURN_ERROR;
	}

	std::wstring flags;
	if (PyUnicode_Check(flagsObj))
	{
		int strLen = PyUnicode_GetSize(flagsObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)flagsObj, text, strLen);
		text[strLen] = L'\0';
		flags = text;
		delete [] text;
	}
	else if (PyString_Check(flagsObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(flagsObj);
		wchar_t* wText = hsStringToWString(text);
		flags = wText;
		delete [] wText;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "open expects two strings");
		PYTHON_RETURN_ERROR;
	}

	PYTHON_RETURN_BOOL(self->fThis->Open(filename.c_str(), flags.c_str()));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptStream, readlines)
{
	std::vector<std::string> lines = self->fThis->ReadLines();
	PyObject* retVal = PyList_New(lines.size());
	for (int i = 0; i < lines.size(); i++)
		PyList_SetItem(retVal, i, PyString_FromString(lines[i].c_str()));
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptStream, writelines, args)
{
	PyObject* stringList = NULL;
	if (!PyArg_ParseTuple(args, "O", &stringList))
	{
		PyErr_SetString(PyExc_TypeError, "writelines expects a list of strings");
		PYTHON_RETURN_ERROR;
	}
	if (!PyList_Check(stringList))
	{
		PyErr_SetString(PyExc_TypeError, "writelines expects a list of strings");
		PYTHON_RETURN_ERROR;
	}
	std::vector<std::string> strings;
	int len = PyList_Size(stringList);
	for (int i = 0; i < len; i++)
	{
		PyObject* element = PyList_GetItem(stringList, i);
		if (!PyString_Check(element))
		{
			PyErr_SetString(PyExc_TypeError, "writelines expects a list of strings");
			PYTHON_RETURN_ERROR;
		}
		strings.push_back(PyString_AsString(element));
	}
	PYTHON_RETURN_BOOL(self->fThis->WriteLines(strings));
}

PYTHON_BASIC_METHOD_DEFINITION(ptStream, close, Close)

PYTHON_METHOD_DEFINITION_NOARGS(ptStream, isOpen)
{
	PYTHON_RETURN_BOOL(self->fThis->IsOpen());
}

PYTHON_START_METHODS_TABLE(ptStream)
	PYTHON_METHOD(ptStream, open, "Params: fileName,flags\nOpen a stream file for reading or writing"),
	PYTHON_METHOD_NOARGS(ptStream, readlines, "Reads a list of strings from the file"),
	PYTHON_METHOD(ptStream, writelines, "Params: lines\nWrite a list of strings to the file"),
	PYTHON_BASIC_METHOD(ptStream, close, "Close the status log file"),
	PYTHON_METHOD_NOARGS(ptStream, isOpen, "Returns whether the stream file is currently opened"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptStream, "A basic stream class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptStream, pyStream)

PYTHON_CLASS_CHECK_IMPL(ptStream, pyStream)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptStream, pyStream)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyStream::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptStream);
	PYTHON_CLASS_IMPORT_END(m);
}