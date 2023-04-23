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
#include <string_theory/string>

#include "pyStream.h"
#include "plFileSystem.h"


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
    plFileName filename;
    ST::string flags;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_PlFileNameDecoder, &filename, PyUnicode_STStringConverter, &flags))
    {
        PyErr_SetString(PyExc_TypeError, "open expects a pathlike object or string, and a string");
        PYTHON_RETURN_ERROR;
    }

    PYTHON_RETURN_BOOL(self->fThis->Open(filename, flags));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptStream, readlines)
{
    std::vector<ST::string> lines = self->fThis->ReadLines();
    PyObject* retVal = PyList_New(lines.size());
    for (int i = 0; i < lines.size(); i++)
        PyList_SetItem(retVal, i, PyUnicode_FromSTString(lines[i]));
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptStream, writelines, args)
{
    PyObject* stringList = nullptr;
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
    std::vector<ST::string> strings;
    Py_ssize_t len = PyList_Size(stringList);
    for (Py_ssize_t i = 0; i < len; i++)
    {
        PyObject* element = PyList_GetItem(stringList, i);
        if (!PyUnicode_Check(element))
        {
            PyErr_SetString(PyExc_TypeError, "writelines expects a list of strings");
            PYTHON_RETURN_ERROR;
        }
        strings.emplace_back(PyUnicode_AsSTString(element));
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