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

#ifndef _pyGameHandler_h_
#define _pyGameHandler_h_

#include "pyGameCli.h"
#include "pyObjectRef.h"
#include "plPythonCallable.h"

#include <type_traits>

#include <pfGameMgr/pfGameCli.h>

namespace plPython
{
    template<>
    inline PyObject* ConvertFrom(EGameJoinError&& error)
    {
        // One of the values is actually signed.
        return PyLong_FromLong((long)error);
    }
};

template<class _GameHandlerT, class _GlueT>
class pyGameHandler : public _GameHandlerT
{
    static_assert(
        std::is_base_of_v <pfGameHandler, _GameHandlerT>,
        "pyGameHandler _GameHandlerT template argument must be a subclass of pfGameHandler"
    );
    static_assert(
        std::is_base_of_v<pyGameCli, _GlueT>,
        "pyGameHandler _GlueT template argument must be a subclass of pyGameCli"
    );

private:
    pyObjectRef fPyObj;

protected:
    pyGameHandler() = delete;
    pyGameHandler(const pyGameHandler&) = delete;
    pyGameHandler(pyGameHandler&&) = delete;
    pyGameHandler(PyObject* obj)
        : fPyObj(obj, pyObjectNewRef)
    {
        ISetGameCli(Py_None);
    }

    ~pyGameHandler()
    {
        ISetGameCli(Py_None);
    }

protected:
    template<typename... _ArgsT>
    void ICallMethod(const char* name, _ArgsT&&... args) const
    {
        pyObjectRef func = PyObject_GetAttrString(fPyObj.Get(), name);
        if (!func) {
            PyErr_Clear();
            return;
        }

        if (PyCallable_Check(func.Get())) {
            pyObjectRef result = plPython::CallObject(
                func,
                std::forward<_ArgsT>(args)...
            );
            if (!result)
                PyErr_Print();
        }
    }

    void ISetGameCli(PyObject* cli)
    {
        int result = PyObject_SetAttrString(
            fPyObj.Get(),
            "gameCli",
            cli
        );
        if (result != 0)
            PyErr_Print();
    }

public:
    void OnGameCliInstance(EGameJoinError error) override
    {
        ISetGameCli(_GlueT::New(_GameHandlerT::GetGameCli()));
        ICallMethod("OnGameCliInstance", std::forward<EGameJoinError>(error));
    }

    void OnGameCliDelete() override
    {
        ISetGameCli(Py_None);
        ICallMethod("OnGameCliDelete");

        // The GameCli is going away, so it's time for us to die.
        delete this;
    }

    void OnPlayerJoined(uint32_t playerID) override
    {
        ICallMethod("OnPlayerJoined", playerID);
    }

    void OnPlayerLeft(uint32_t playerID) override
    {
        ICallMethod("OnPlayerLeft", playerID);
    }

    void OnOwnerChanged(uint32_t newOwnerID) override
    {
        ICallMethod("OnOwnerChanged", newOwnerID);
    }
};

#endif
