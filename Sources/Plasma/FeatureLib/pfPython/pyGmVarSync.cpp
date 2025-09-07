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

#include "pyGmVarSync.h"

#include <type_traits>

#include "pfGameMgr/pfGmVarSync.h"

#include "plPythonCallable.h"
#include "pyGameHandler.h"
#include "pyObjectRef.h"

// ===========================================================================

void pyGmVarSync::SetVariable(uint32_t varID, const pfGmVarSyncValue& varValue) const
{
    GetGameCli()->SetVariable(varID, varValue);
}

void pyGmVarSync::CreateVariable(const ST::string& varName, const pfGmVarSyncValue& varValue) const
{
    GetGameCli()->CreateVariable(varName, varValue);
}

// ===========================================================================

namespace plPython
{
    template<>
    PyObject* ConvertFrom(pfGmVarSyncValue&& value)
    {
        return std::visit(
            [](auto&& value) -> PyObject* {
                using _ValueT = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<_ValueT, ST::string>) {
                    return PyUnicode_FromSTString(value);
               } else if constexpr (std::is_same_v<_ValueT, double>) {
                    return PyFloat_FromDouble(value);
               } else {
                    static_assert(
                        std::is_same_v<_ValueT, double> || std::is_same_v<_ValueT, ST::string>,
                        "Non-exhaustive visitor"
                    );
               }
            }, value
        );
    }
}

// ===========================================================================

class pyGmVarSyncHandler : public pyGameHandler<pfGmVarSyncHandler, pyGmVarSync>
{
public:
    pyGmVarSyncHandler(PyObject* obj)
        : pyGameHandler(obj)
    {
    }

public:
    void OnVarChanged(uint32_t varID, pfGmVarSyncValue varValue) override
    {
        ICallMethod("OnVarChanged", varID, std::move(varValue));
    }

    void OnAllVarsSent() override
    {
        ICallMethod("OnAllVarsSent");
    }

    void OnVarCreated(
        ST::string varName,
        uint32_t varID,
        pfGmVarSyncValue value
    ) override
    {
        ICallMethod(
            "OnVarCreated",
            std::move(varName),
            varID,
            std::move(value)
        );
    }
};

// ===========================================================================

void pyGmVarSync::Join(PyObject* handler)
{
    pfGmVarSync::Join(new pyGmVarSyncHandler(handler));
}

bool pyGmVarSync::IsSupported()
{
    return pfGmVarSync::IsSupported();
}
