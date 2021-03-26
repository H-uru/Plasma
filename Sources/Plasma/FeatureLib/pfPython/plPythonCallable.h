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

#include <functional>
#include <type_traits>
#include <variant>

#include <Python.h>
#include <string_theory/format>

#include "plProfile.h"

#include "cyPythonInterface.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

plProfile_Extern(PythonUpdate);

namespace plPythonCallable
{
    template<typename ArgT>
    inline void IBuildTupleArg(PyObject* tuple, size_t idx, ArgT value) = delete;

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, bool value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyBool_FromLong(value ? 1 : 0));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, char value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromFormat("%c", (int)value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, const char* value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromString(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, double value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyFloat_FromDouble(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, float value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyFloat_FromDouble(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, int8_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, int16_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, int32_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromLong(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, PyObject* value)
    {
        PyTuple_SET_ITEM(tuple, idx, value);
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, pyObjectRef& value)
    {
        PyTuple_SET_ITEM(tuple, idx, value.Release());
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, const ST::string& value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromSTString(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint8_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint16_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, uint32_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyLong_FromSize_t(value));
    }

    inline void IBuildTupleArg(PyObject* tuple, size_t idx, wchar_t value)
    {
        PyTuple_SET_ITEM(tuple, idx, PyUnicode_FromFormat("%c", (int)value));
    }

    template<size_t Size, typename Arg>
    inline void BuildTupleArgs(PyObject* tuple, Arg&& arg)
    {
        IBuildTupleArg(tuple, (Size - 1), std::forward<Arg>(arg));
    }

    template<size_t Size, typename Arg0, typename... Args>
    inline void BuildTupleArgs(PyObject* tuple, Arg0&& arg0, Args&&... args)
    {
        IBuildTupleArg(tuple, (Size - (sizeof...(args) + 1)), std::forward<Arg0>(arg0));
        BuildTupleArgs<Size>(tuple, std::forward<Args>(args)...);
    }

    template<typename... _CBArgsT>
    [[nodiscard]]
    inline std::function<void(_CBArgsT...)> BuildCallback(ST::string parentCall, PyObject* callable)
    {
        hsAssert(PyCallable_Check(callable) != 0, "BuildCallback() expects a Python callable.");

        pyObjectRef cb(callable, pyObjectNewRef);
        return [cb = std::move(cb), parentCall = std::move(parentCall)](_CBArgsT&&... args) -> void {
            pyObjectRef tuple = PyTuple_New(sizeof...(args));
            BuildTupleArgs<sizeof...(args)>(tuple.Get(), std::forward<_CBArgsT>(args)...);

            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef result = PyObject_CallObject(cb.Get(), tuple.Get());
            plProfile_EndTiming(PythonUpdate);

            if (!result) {
                // Stash the error state so we can get some info about the
                // callback before printing the exception itself.
                PyObject* ptype, * pvalue, * ptraceback;
                PyErr_Fetch(&ptype, &pvalue, &ptraceback);
                pyObjectRef repr = PyObject_Repr(cb.Get());
                PythonInterface::WriteToLog(ST::format("Error executing '{}' callback for '{}'",
                                                       PyUnicode_AsSTString(repr.Get()),
                                                       parentCall));
                PyErr_Restore(ptype, pvalue, ptraceback);
                PyErr_Print();
            }
        };
    }

    template<typename... _CBArgsT>
    inline void BuildCallback(ST::string parentCall, PyObject* callable,
                              std::function<void(_CBArgsT...)>& cb)
    {
        cb = BuildCallback<_CBArgsT...>(std::move(parentCall), callable);
    }

    template<size_t _AlternativeN, typename... _VariantArgsT>
    inline void BuildCallback(ST::string parentCall, PyObject* callable,
                              std::variant<_VariantArgsT...>& cb)
    {
        std::variant_alternative_t<_AlternativeN, std::decay_t<decltype(cb)>> cbFunc;
        BuildCallback(std::move(parentCall), callable, cbFunc);
        cb = std::move(cbFunc);
    }
};

#endif
