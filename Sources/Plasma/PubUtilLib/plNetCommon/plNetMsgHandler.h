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
#ifndef plNetMsgHandler_inc
#define plNetMsgHandler_inc

class plNetMessage;
class plNetApp;

#define kMsgSendInterval        0.25f   // time between message re-send
#define kNetOperationTimeout    10.f    // amount of time we try a network operation before complaining

//
// Base msg handler class
//
class plNetMsgHandler
{
protected:
    plNetApp* fNetApp;
public:
    enum class Status
    {
        kError,
        kHandled,
    };

    plNetMsgHandler() : fNetApp() { }
    virtual ~plNetMsgHandler() {}

    void SetNetApp(plNetApp* na) { fNetApp=na; }
    plNetApp* GetNetApp() { return fNetApp; }

    virtual Status ReceiveMsg(plNetMessage* netMsg) = 0;
};

#define MSG_HANDLER(msgClassName) msgClassName##HandleMsg

//
// Use to declare msg handler fxns in your MsgHandler .h class header
//
#define MSG_HANDLER_DECL(msgClassName) \
virtual plNetMsgHandler::Status MSG_HANDLER(msgClassName)(plNetMessage* netMsg); 

//
// Use to define msg handler fxns in your MsgHandler .cpp file
//
#define MSG_HANDLER_DEFN(handlerClassName, msgClassName) \
plNetMsgHandler::Status handlerClassName::MSG_HANDLER(msgClassName)(plNetMessage* netMsg)

//
// Use in the switch statement in your ReceiveMsg function
//
#define MSG_HANDLER_CASE(msgClassName) \
case CLASS_INDEX_SCOPED(msgClassName): \
    { \
        return MSG_HANDLER(msgClassName)(netMsg); \
    } 


#endif  //  plNetMsgHandler_inc
