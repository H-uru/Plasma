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
#ifndef plAIMsg_inc
#define plAIMsg_inc

#ifndef SERVER // we use stuff the server doesn't link with
#ifndef NO_AV_MSGS

#include "hsGeometry3.h"
#include "../pnMessage/plMessage.h"

class plAvBrainCritter;

// abstract base class for all AI-related messages
class plAIMsg : public plMessage
{
public:
	plAIMsg();
	plAIMsg(const plKey& sender, const plKey& receiver);

	CLASSNAME_REGISTER(plAIMsg);
	GETINTERFACE_ANY(plAIMsg, plMessage);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void BrainUserString(const std::string& userStr) {fBrainUserStr = userStr;}
	std::string BrainUserString() const {return fBrainUserStr;}

	// enum for all messages to make things easier for people that use us
	enum
	{
		kAIMsg_Unknown,
		kAIMsg_BrainCreated,
		kAIMsg_ArrivedAtGoal,
	};

private:
	std::string fBrainUserStr;
};

// message spammed to anyone listening so they can grab the brain's key and talk to it
// does NOT get net-propped
class plAIBrainCreatedMsg : public plAIMsg
{
public:
	plAIBrainCreatedMsg(): plAIMsg() {SetBCastFlag(plMessage::kBCastByExactType);}
	plAIBrainCreatedMsg(const plKey& sender): plAIMsg(sender, nil) {SetBCastFlag(plMessage::kBCastByExactType);}

	CLASSNAME_REGISTER(plAIBrainCreatedMsg);
	GETINTERFACE_ANY(plAIBrainCreatedMsg, plAIMsg);

	virtual void Read(hsStream* stream, hsResMgr* mgr) {plAIMsg::Read(stream, mgr);}
	virtual void Write(hsStream* stream, hsResMgr* mgr) {plAIMsg::Write(stream, mgr);}
};

// message sent when the brain arrives at it's specified goal
// does NOT get net-propped
class plAIArrivedAtGoalMsg : public plAIMsg
{
public:
	plAIArrivedAtGoalMsg(): plAIMsg(), fGoal(0, 0, 0) {}
	plAIArrivedAtGoalMsg(const plKey& sender, const plKey& receiver): plAIMsg(sender, receiver), fGoal(0, 0, 0) {}

	CLASSNAME_REGISTER(plAIArrivedAtGoalMsg);
	GETINTERFACE_ANY(plAIArrivedAtGoalMsg, plAIMsg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void Goal(hsPoint3 goal) {fGoal = goal;}
	hsPoint3 Goal() const {return fGoal;}

private:
	hsPoint3 fGoal;
};

#endif // NO_AV_MSGS
#endif // SERVER

#endif // plAIMsg_inc