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

#ifndef _plPythonConvert_h_
#define _plPythonConvert_h_

#include <Python.h>
#include <string_theory/string>
#include <type_traits>
#include <tuple>

#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

namespace plPython
{
    template<typename ArgT>
    inline PyObject* ConvertFrom(ArgT&& value) = delete;

    inline PyObject* ConvertFrom(bool value)
    {
        PyObject* result = value ? Py_True : Py_False;
        Py_INCREF(result);
        return result;
    }

    inline PyObject* ConvertFrom(char value)
    {
        return PyUnicode_FromFormat("%c", (int)value);
    }

    inline PyObject* ConvertFrom(const char* value)
    {
        return PyUnicode_FromString(value);
    }

    inline PyObject* ConvertFrom(double value)
    {
        return PyFloat_FromDouble(value);
    }

    inline PyObject* ConvertFrom(float value)
    {
        return PyFloat_FromDouble(value);
    }

    inline PyObject* ConvertFrom(signed char value)
    {
        return PyLong_FromLong(value);
    }

    inline PyObject* ConvertFrom(signed short value)
    {
        return PyLong_FromLong(value);
    }

    inline PyObject* ConvertFrom(signed int value)
    {
        return PyLong_FromLong(value);
    }

    inline PyObject* ConvertFrom(signed long value)
    {
        return PyLong_FromLong(value);
    }

    inline PyObject* ConvertFrom(signed long long value)
    {
        return PyLong_FromLongLong(value);
    }

    inline PyObject* ConvertFrom(ST::string&& value)
    {
        return PyUnicode_FromSTString(value);
    }

    inline PyObject* ConvertFrom(unsigned char value)
    {
        return PyLong_FromUnsignedLong(value);
    }

    inline PyObject* ConvertFrom(unsigned short value)
    {
        return PyLong_FromUnsignedLong(value);
    }

    inline PyObject* ConvertFrom(unsigned int value)
    {
        return PyLong_FromUnsignedLong(value);
    }

    inline PyObject* ConvertFrom(unsigned long value)
    {
        return PyLong_FromUnsignedLong(value);
    }

    inline PyObject* ConvertFrom(unsigned long long value)
    {
        return PyLong_FromUnsignedLongLong(value);
    }

    inline PyObject* ConvertFrom(wchar_t value)
    {
        return PyUnicode_FromFormat("%c", (int)value);
    }

    inline PyObject* ConvertFrom(std::nullptr_t)
    {
        Py_RETURN_NONE;
    }

    /**
     * Returns a stolen reference to match the other overloads.
     */
    inline PyObject* ConvertFrom(PyObject* value)
    {
        return value;
    }

    /**
     * Returns a stolen reference to match the other overloads.
     */
    inline PyObject* ConvertFrom(pyObjectRef&& value)
    {
        return value.Release();
    }

    struct ToTuple_Type {};
    constexpr ToTuple_Type ToTuple;

    template<typename... _TupleArgsT>
    inline PyObject* ConvertFrom(ToTuple_Type, _TupleArgsT&&... args)
    {
        PyObject* tuple = PyTuple_New(sizeof...(args));
        Py_ssize_t i = 0;
        (PyTuple_SET_ITEM(tuple, i++, ConvertFrom(std::forward<_TupleArgsT>(args))), ...);
        return tuple;
    }
};

#endif
