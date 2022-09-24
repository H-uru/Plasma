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
#include <memory>
#include <type_traits>
#include <variant>

#include <Python.h>
#include <string_theory/format>

#include "HeadSpin.h"
#include "plProfile.h"

#include "cyPythonInterface.h"
#include "plPythonConvert.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

plProfile_Extern(PythonUpdate);

namespace plPython
{
    namespace _detail
    {
        template <size_t _NArgsfT, typename... Args>
        inline pyObjectRef VectorcallObject(PyObject* callable, PyObject* argsObjs[], Args&&... args)
        {
            // Python's vectorcall protocol has an optimization whereby it can reuse the memory allocated
            // for the arguments array if you let it (ab)-use the first slot.
            {
                argsObjs[0] = nullptr;
                size_t i = 1; // Don't touch the first slot...
                ((argsObjs[i++] = ConvertFrom(std::forward<Args>(args))), ...);
            }

            // Now, we perform the Python call with the resulting C++ compile-time magic.
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef result = _PyObject_Vectorcall(
                callable,
                &argsObjs[1], // This is the optimization mentioned earlier.
                (_NArgsfT - 1) | PY_VECTORCALL_ARGUMENTS_OFFSET, // Indicates we are passing args[1] instead of args[0]
                nullptr
            );
            plProfile_EndTiming(PythonUpdate);

            for (size_t i = 1; i < _NArgsfT; ++i)
                Py_XDECREF(argsObjs[i]);
            return result;
        }
    };

    template<typename... Args>
    inline pyObjectRef CallObject(PyObject* callable, Args&&... args)
    {
        hsAssert(PyCallable_Check(callable), "Trying to call a non-callable, eh?");

        // The point of all this is to use Python's new "vectorcall" calling convention.
        // PSF claims that is is faster -- the most evident improvement is that we don't
        // have to allocate a transitional tuple object to pass the arguments.
        if constexpr (sizeof...(args) == 1) {
            // Use Python's built-in vectorcall optimization for one argument.
            pyObjectRef arg = ConvertFrom(std::forward<Args>(args)...);
            plProfile_BeginTiming(PythonUpdate);
            pyObjectRef result = _PyObject_CallOneArg(callable, arg.Get());
            plProfile_EndTiming(PythonUpdate);
            return result;
        } else if constexpr (sizeof...(args) == 0) {
            plProfile_BeginTiming(PythonUpdate);
#if PY_VERSION_HEX >= 0x03090000
            // Use Python's built-in vectorcall optimization for no argument.
            pyObjectRef result = _PyObject_CallNoArg(callable);
#else
            // This is basically the same idea.
            pyObjectRef result = _PyObject_Vectorcall(
                callable,
                nullptr,
                0,
                nullptr
            );
#endif
            plProfile_EndTiming(PythonUpdate);
            return result;
        } else if constexpr (sizeof...(args) < 64) {
            constexpr size_t nargs = sizeof...(args) + 1;
            PyObject* argsObjs[nargs];
            return _detail::VectorcallObject<nargs>(callable, argsObjs, std::forward<Args>(args)...);
        } else {
            // Yikes, so many arguments we need to make an alloation.
            constexpr size_t nargs = sizeof...(args) + 1;
            auto argsObjs = std::make_unique<PyObject* []>(nargs);
            return _detail::VectorcallObject<nargs>(callable, argsObjs.get(), std::forward<Args>(args)...);
        }
    }
    
    template<typename... Args>
    inline pyObjectRef CallObject(const pyObjectRef& callable, Args&&... args)
    {
        return CallObject(callable.Get(), std::forward<Args>(args)...);
    }

    template<typename... _CBArgsT>
    [[nodiscard]]
    inline std::function<void(_CBArgsT...)> BuildCallback(ST::string parentCall, PyObject* callable)
    {
        hsAssert(PyCallable_Check(callable) != 0, "BuildCallback() expects a Python callable.");

        pyObjectRef cb(callable, pyObjectNewRef);
        return [cb = std::move(cb), parentCall = std::move(parentCall)](_CBArgsT&&... args) -> void {
            pyObjectRef result = plPython::CallObject(cb, std::forward<_CBArgsT>(args)...);
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
