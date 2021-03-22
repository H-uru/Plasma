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

#ifndef _pyPythonCallable_h_
#define _pyPythonCallable_h_

#include <Python.h>
#include <string_theory/string>

#include "cyPythonInterface.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

namespace plPythonCallable
{
    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, bool value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyBool_FromLong(value ? 1 : 0));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, char value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromFormat("%c", (int)value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, const char* value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromString(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, double value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyFloat_FromDouble(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, float value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyFloat_FromDouble(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, int8_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, int16_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, int32_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, PyObject* value)
    {
        PyTuple_SET_ITEM(tuple, idx, value);
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, pyObjectRef& value)
    {
        PyTuple_SET_ITEM(tuple, idx, value.Release());
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, const ST::string& value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromSTString(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint8_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint16_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint32_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    static inline void IBuildTupleArg(PyObject* tuple, size_t idx, wchar_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromFormat("%c", (int)value));
    }

    template<size_t Size, typename Arg>
    static inline void BuildTupleArgs(PyObject* tuple, Arg&& arg)
    {
        IBuildTupleArg(tuple, (Size - 1), std::forward<Arg>(arg));
    }

    template<size_t Size, typename Arg0, typename... Args>
    static inline void BuildTupleArgs(PyObject* tuple, Arg0&& arg0, Args&&... args)
    {
        IBuildTupleArg(tuple, (Size - (sizeof...(args) + 1)), std::forward<Arg0>(arg0));
        BuildTupleArgs<Size>(tuple, std::forward<Args>(args)...);
    }
};

#endif
