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

#include "pyKeyMap.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptKeyMap, pyKeyMap);

PYTHON_DEFAULT_NEW_DEFINITION(ptKeyMap, pyKeyMap)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptKeyMap)

PYTHON_INIT_DEFINITION(ptKeyMap, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptKeyMap, convertVKeyToChar, args)
{
    unsigned long virtualKey, keyFlags;
    if (!PyArg_ParseTuple(args, "ll", &virtualKey, &keyFlags))
    {
        PyErr_SetString(PyExc_TypeError, "convertVKeyToChar expects two unsigned longs");
        PYTHON_RETURN_ERROR;
    }
    return PyUnicode_FromSTString(self->fThis->ConvertVKeyToChar(virtualKey, keyFlags));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, convertCharToVKey, args)
{
    ST::string charString;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &charString))
    {
        PyErr_SetString(PyExc_TypeError, "convertCharToVKey expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->ConvertCharToVKey(charString));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, convertCharToFlags, args)
{
    ST::string charString;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &charString))
    {
        PyErr_SetString(PyExc_TypeError, "convertCharToFlags expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->ConvertCharToFlags(charString));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, convertCharToControlCode, args)
{
    ST::string charString;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &charString))
    {
        PyErr_SetString(PyExc_TypeError, "convertCharToControlCode expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->ConvertCharToControlCode(charString));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, convertControlCodeToString, args)
{
    unsigned long code;
    if (!PyArg_ParseTuple(args, "l", &code))
    {
        PyErr_SetString(PyExc_TypeError, "convertControlCodeToString expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return PyUnicode_FromSTString(self->fThis->ConvertControlCodeToString(code));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, bindKey, args)
{
    ST::string key1;
    ST::string key2;
    ST::string action;
    if (!PyArg_ParseTuple(args, "O&O&O&", PyUnicode_STStringConverter, &key1, PyUnicode_STStringConverter, &key2, PyUnicode_STStringConverter, &action))
    {
        PyErr_SetString(PyExc_TypeError, "bindKey expects three strings");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->BindKey(key1, key2, action);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingKey1, args)
{
    unsigned long code;
    if (!PyArg_ParseTuple(args, "l", &code))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingKey1 expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingKey1(code));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingFlags1, args)
{
    unsigned long code;
    if (!PyArg_ParseTuple(args, "l", &code))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingFlags1 expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingFlags1(code));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingKey2, args)
{
    unsigned long code;
    if (!PyArg_ParseTuple(args, "l", &code))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingKey2 expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingKey2(code));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingFlags2, args)
{
    unsigned long code;
    if (!PyArg_ParseTuple(args, "l", &code))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingFlags2 expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingFlags2(code));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, bindKeyToConsoleCommand, args)
{
    ST::string keyStr1;
    ST::string command;
    if (!PyArg_ParseTuple(args, "O&O&", PyUnicode_STStringConverter, &keyStr1, PyUnicode_STStringConverter, &command))
    {
        PyErr_SetString(PyExc_TypeError, "bindKeyToConsoleCommand expects two strings");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->BindKeyToConsoleCommand(keyStr1, command);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingKeyConsole, args)
{
    ST::string command;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &command))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingKeyConsole expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingKeyConsole(command));
}

PYTHON_METHOD_DEFINITION(ptKeyMap, getBindingFlagsConsole, args)
{
    ST::string command;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &command))
    {
        PyErr_SetString(PyExc_TypeError, "getBindingFlagsConsole expects a string");
        PYTHON_RETURN_ERROR;
    }
    return PyLong_FromLong(self->fThis->GetBindingFlagsConsole(command));
}

PYTHON_BASIC_METHOD_DEFINITION(ptKeyMap, writeKeyMap, WriteKeyMap)

PYTHON_START_METHODS_TABLE(ptKeyMap)
    PYTHON_METHOD(ptKeyMap, convertVKeyToChar, "Params: virtualKey,flags\nConvert virtual key and shift flags to string"),
    PYTHON_METHOD(ptKeyMap, convertCharToVKey, "Params: charString\nConvert char string to virtual key"),
    PYTHON_METHOD(ptKeyMap, convertCharToFlags, "Params: charString\nConvert char string to flags"),
    PYTHON_METHOD(ptKeyMap, convertCharToControlCode, "Params: controlCodeString\nConvert string version of control code to number"),
    PYTHON_METHOD(ptKeyMap, convertControlCodeToString, "Params controlCode\nConvert control code to character string"),
    PYTHON_METHOD(ptKeyMap, bindKey, "Params key1,key2,action\nBind keys to an action"),
    PYTHON_METHOD(ptKeyMap, getBindingKey1, "Params controlCode\nReturns key code for controlCode"),
    PYTHON_METHOD(ptKeyMap, getBindingFlags1, "Params controlCode\nReturns modifier flags for controlCode"),
    PYTHON_METHOD(ptKeyMap, getBindingKey2, "Params controlCode\nReturns key code for controlCode"),
    PYTHON_METHOD(ptKeyMap, getBindingFlags2, "Params controlCode\nReturns modifier flags for controlCode"),
    PYTHON_METHOD(ptKeyMap, bindKeyToConsoleCommand, "Params: keyStr1, command\nBinds key to console command"),
    PYTHON_METHOD(ptKeyMap, getBindingKeyConsole, "Params: command\nReturns key for console command mapping"),
    PYTHON_METHOD(ptKeyMap, getBindingFlagsConsole, "Params: command\nReturns modifier flags for the console command mapping"),
    PYTHON_BASIC_METHOD(ptKeyMap, writeKeyMap, "Forces write of the keymap file"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptKeyMap, "Accessor class to the Key Mapping functions");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptKeyMap, pyKeyMap)

PYTHON_CLASS_CHECK_IMPL(ptKeyMap, pyKeyMap)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptKeyMap, pyKeyMap)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyKeyMap::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptKeyMap);
    PYTHON_CLASS_IMPORT_END(m);
}