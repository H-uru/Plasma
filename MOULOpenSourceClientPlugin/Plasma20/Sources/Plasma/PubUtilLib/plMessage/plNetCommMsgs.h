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

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCOMMMSGS_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCOMMMSGS_H


#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnMessage/plMessage.h"
#include "../pnNetProtocol/pnNetProtocol.h"


class plNetCommReplyMsg : public plMessage {
public:
    enum EParamType {
		kParamTypeOther	= 0,
		kParamTypePython,
	};

    ENetError   result;
    void *      param;
    EParamType	ptype;

    plNetCommReplyMsg () { SetBCastFlag(kBCastByExactType); }

	void Read (hsStream * s, hsResMgr * mgr) { plMessage::IMsgRead(s, mgr); }
	void Write (hsStream * s, hsResMgr * mgr) { plMessage::IMsgWrite(s, mgr); }
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

	void Read (hsStream * s, hsResMgr * mgr) { plMessage::IMsgRead(s, mgr); }
	void Write (hsStream * s, hsResMgr * mgr) { plMessage::IMsgWrite(s, mgr); }
};

struct NetCliAuthFileInfo;
class plNetCommFileListMsg : public plNetCommReplyMsg {
public:
    FARRAY(NetCliAuthFileInfo)  fileInfoArr;

    CLASSNAME_REGISTER(plNetCommFileListMsg);
    GETINTERFACE_ANY(plNetCommFileListMsg, plMessage);
};

class plNetCommFileDownloadMsg : public plNetCommReplyMsg {
public:
    wchar       filename[MAX_PATH];
    hsStream *  writer;

    CLASSNAME_REGISTER(plNetCommFileDownloadMsg);
    GETINTERFACE_ANY(plNetCommFileDownloadMsg, plMessage);
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
    
    ARRAY(struct NetAgeInfo)	ages;
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

class plNetCommDisconnectedMsg : public plMessage {
public:
    ENetProtocol    protocol;

	void Read (hsStream * s, hsResMgr * mgr) { plMessage::IMsgRead(s, mgr); }
	void Write (hsStream * s, hsResMgr * mgr) { plMessage::IMsgWrite(s, mgr); }
};



#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCOMMMSGS_H
