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

#include "HeadSpin.h"
#include "plProfile.h"

#include "cyPythonInterface.h"
#include "plPythonConvert.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"

plProfile_Extern(PythonUpdate);

namespace plPython
{
    template<typename... Args>
    inline pyObjectRef CallObject(const pyObjectRef& callable, Args&&... args)
    {
        hsAssert(PyCallable_Check(callable.Get()), "Trying to call a non-callable, eh?");

        pyObjectRef tup = ConvertFrom(ToTuple, std::forward<Args>(args)...);
        plProfile_BeginTiming(PythonUpdate);
        pyObjectRef result = PyObject_CallObject(callable.Get(), tup.Get());
        plProfile_EndTiming(PythonUpdate);
        return result;
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
