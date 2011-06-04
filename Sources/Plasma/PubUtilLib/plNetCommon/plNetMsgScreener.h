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
#ifndef plNetMsgScreener_h
#define plNetMsgScreener_h

#include "../pnKeyedObject/plKey.h"
#include "../plStatusLog/plLoggable.h"

//
// Class which decides what game messages are allowed to be sent to the server.
// Used both client and server-side.
//
class plNetGameMember;
class plMessage;
class plNetMessage;
class pfKIMsg;
class plNetMsgScreener : public plLoggable
{
protected:
	enum Answer
	{
		kMaybe = -1,
		kNo,
		kYes
	};
	virtual const char* IGetSenderName(const plNetGameMember* gm) const = 0;
	virtual const char* IGetAgeName() const = 0;
	virtual bool IIsSenderCCR(const plNetGameMember* gm=nil) const = 0;
	virtual bool IIsLocalAvatarKey(plKey key, const plNetGameMember* gm) const = 0;
	virtual bool IIsLocalArmatureModKey(plKey key, const plNetGameMember* gm) const { return true; }

	virtual void ILogChatMessage(const plMessage* msg_, const plNetGameMember* gm) const {}
	virtual void ILogCCRMessage(Int16 classIndex, const plNetGameMember* gm) const {}
	
	Answer IAllowMessageType(Int16 classIndex, const plNetGameMember* gm=nil) const;
	bool IValidateMessage(const plMessage* msg, const plNetGameMember* gm=nil) const;
	void IRejectLogMsg(Int16 classIndex, const char* desc, const plNetGameMember* gm) const;
	void IRejectLogMsg(const plMessage* msg, const char* desc, const plNetGameMember* gm) const;
	virtual bool IAmClient() const = 0;
};

#endif	// plNetMsgScreener_h