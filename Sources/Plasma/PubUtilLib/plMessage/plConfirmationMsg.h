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

#ifndef plConfirmationMsg_inc
#define plConfirmationMsg_inc

// I hope you like big STL headers...
#include <functional>
#include <variant>
#include <vector>

#include <string_theory/string>

#include "pnMessage/plMessage.h"

/** Show an in-game confirmation GUI dialog. */
class plConfirmationMsg : public plMessage
{
public:
    enum class Result : int32_t
    {
        OK = 1,
        Cancel = 0,
        Yes = 1,
        No = 0,
        Quit = 1,
        Logout = 62,
    };

    using Callback = std::variant<std::monostate, std::function<void(Result)>, plKey>;

    enum class Type : uint32_t
    {
        /** Informational item for the user with only the possibility to OK it. */
        OK,

        /**
         * The quit dialog. Do not use for an error.
         * If the user requests a quit or logout, then any optional callback will be
         * dispatched, then the client will quit or logout on the next main thread
         * evaluation.
         */
        ConfirmQuit,

        /** Notify user about an exception case, then force quit the client. */
        ForceQuit,

        /** Requests the user to answer Yes or No to a question. */
        YesNo,
    };

protected:
    ST::string fMessage;
    Type fDialogType;
    Callback fCallback;

public:
    plConfirmationMsg()
    {
        SetBCastFlag(plMessage::kBCastByType);
    }

    plConfirmationMsg(ST::string msg, Type type = Type::OK, Callback cb = {})
        : fMessage(std::move(msg)),
          fDialogType(type),
          fCallback(std::move(cb))
    {
        SetBCastFlag(plMessage::kBCastByType);
    }

    CLASSNAME_REGISTER(plConfirmationMsg);
    GETINTERFACE_ANY(plConfirmationMsg, plMessage);

    void Read(hsStream*, hsResMgr*) override { FATAL("no"); }
    void Write(hsStream*, hsResMgr*) override { FATAL("no"); }

    ST::string GetText() const { return fMessage; }
    Type GetType() const { return fDialogType; }
    Callback GetCallback() const { return fCallback; }

    void SetText(ST::string msg) { fMessage = std::move(msg); }
    void SetType(Type type) { fDialogType = type; }
    void SetCallback(Callback cb) { fCallback = std::move(cb); }
};


/** Show a localized in-game confirmation GUI dialog. */
class plLocalizedConfirmationMsg : public plConfirmationMsg
{
protected:
    std::vector<ST::string> fArgs;

public:
    plLocalizedConfirmationMsg() = default;
    plLocalizedConfirmationMsg(ST::string path, std::vector<ST::string> args = {},
                               Type type = Type::OK, Callback cb = {})
        : plConfirmationMsg(std::move(path), type, std::move(cb)),
          fArgs(std::move(args))
    { }

    CLASSNAME_REGISTER(plLocalizedConfirmationMsg);
    GETINTERFACE_ANY(plLocalizedConfirmationMsg, plConfirmationMsg);

    const std::vector<ST::string>& GetArgs() const { return fArgs; }
    std::vector<ST::string>& GetArgs() { return fArgs; }

    void SetArgs(std::vector<ST::string> args) { fArgs = std::move(args); }
};

#endif
