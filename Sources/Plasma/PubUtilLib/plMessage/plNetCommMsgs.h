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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plMessage/plNetCommMsgs.h
*   
***/

#ifndef plNetCommMsgs_inc
#define plNetCommMsgs_inc


#include "pnNetBase/pnNbError.h"
#include "pnMessage/plMessage.h"

struct NetAgeInfo;
struct NetCliAuthFileInfo;

class plNetCommReplyMsg : public plMessage {
public:
    enum EParamType {
        kParamTypeOther = 0,
        kParamTypePython,
    };

    ENetError   result;
    void *      param;
    EParamType  ptype;

    plNetCommReplyMsg () { SetBCastFlag(kBCastByExactType); }

    void Read(hsStream * s, hsResMgr * mgr) override { plMessage::IMsgRead(s, mgr); }
    void Write(hsStream * s, hsResMgr * mgr) override { plMessage::IMsgWrite(s, mgr); }
};

class plNetCommAuthMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommAuthMsg);
    GETINTERFACE_ANY(plNetCommAuthMsg, plMessage);
};

class plNetCommAuthConnectedMsg : public plMessage {
public:
    plNetCommAuthConnectedMsg () { SetBCastFlag(kBCastByExactType); }

    CLASSNAME_REGISTER(plNetCommAuthConnectedMsg);
    GETINTERFACE_ANY(plNetCommAuthConnectedMsg, plMessage);

    void Read(hsStream * s, hsResMgr * mgr) override { plMessage::IMsgRead(s, mgr); }
    void Write(hsStream * s, hsResMgr * mgr) override { plMessage::IMsgWrite(s, mgr); }
};

class plNetCommFileListMsg : public plNetCommReplyMsg {
public:
    std::vector<NetCliAuthFileInfo>  fileInfoArr;

    CLASSNAME_REGISTER(plNetCommFileListMsg);
    GETINTERFACE_ANY(plNetCommFileListMsg, plMessage);

    plNetCommFileListMsg();
    plNetCommFileListMsg(const plNetCommFileListMsg&) = delete;
    plNetCommFileListMsg(plNetCommFileListMsg&&) = delete;

    ~plNetCommFileListMsg();
};

class plNetCommLinkToAgeMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommLinkToAgeMsg);
    GETINTERFACE_ANY(plNetCommLinkToAgeMsg, plMessage);
};

class plNetCommPlayerListMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommPlayerListMsg);
    GETINTERFACE_ANY(plNetCommPlayerListMsg, plMessage);
};

class plNetCommActivePlayerMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommActivePlayerMsg);
    GETINTERFACE_ANY(plNetCommActivePlayerMsg, plMessage);
};

class plNetCommCreatePlayerMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommCreatePlayerMsg);
    GETINTERFACE_ANY(plNetCommCreatePlayerMsg, plMessage);
};

class plNetCommDeletePlayerMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommDeletePlayerMsg);
    GETINTERFACE_ANY(plNetCommDeletePlayerMsg, plMessage);
};

class plNetCommPublicAgeListMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommPublicAgeListMsg);
    GETINTERFACE_ANY(plNetCommPublicAgeListMsg, plMessage);
    
    std::vector<NetAgeInfo>    ages;

    plNetCommPublicAgeListMsg();
    plNetCommPublicAgeListMsg(const plNetCommPublicAgeListMsg&) = delete;
    plNetCommPublicAgeListMsg(plNetCommPublicAgeListMsg&&) = delete;

    ~plNetCommPublicAgeListMsg();
};

class plNetCommPublicAgeMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommPublicAgeMsg);
    GETINTERFACE_ANY(plNetCommPublicAgeMsg, plMessage);
};

class plNetCommRegisterAgeMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommRegisterAgeMsg);
    GETINTERFACE_ANY(plNetCommRegisterAgeMsg, plMessage);
};

class plNetCommUnregisterAgeMsg : public plNetCommReplyMsg {
public:
    CLASSNAME_REGISTER(plNetCommUnregisterAgeMsg);
    GETINTERFACE_ANY(plNetCommUnregisterAgeMsg, plMessage);
};

#endif // plNetCommMsgs_inc
