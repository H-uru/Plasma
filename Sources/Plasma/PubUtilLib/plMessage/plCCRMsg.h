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
#ifndef plCCRMsg_h
#define plCCRMsg_h

#include "../pnMessage/plMessage.h"
#include "../plNetCommon/plNetCommon.h"

//
// Abstract Baseclass for CCR messages.
// All CCR messages must derive from this or the server won't be able to tell that's what they are.
//
class plCCRMessage : public plMessage
{
public:
	CLASSNAME_REGISTER( plCCRMessage);
	GETINTERFACE_ANY( plCCRMessage, plMessage );	
};


//
// Player wants to petition a CCR
//
class plCCRPetitionMsg : public plCCRMessage
{
private:
	UInt8 fPetitionType;
	std::string	fNote;
	std::string fTitle;
public:
	plCCRPetitionMsg() : fPetitionType(plNetCommon::PetitionTypes::kGeneralHelp) { fBCastFlags |= kBCastByType;	}
	~plCCRPetitionMsg() {}

	CLASSNAME_REGISTER( plCCRPetitionMsg);
	GETINTERFACE_ANY( plCCRPetitionMsg, plCCRMessage );	

	// petition text
	void SetNote(const char* n) { fNote=n;	}
	const char* GetNote() const { return fNote.c_str();	}

	// title
	void SetTitle(const char* n) { fTitle=n;	}
	const char* GetTitle() const { return fTitle.c_str();	}

	// petition type
	void SetType(const UInt8 t) { fPetitionType=t;	}
	UInt8 GetType() const { return fPetitionType;	}

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

//
// A CCR has gone invisible
//
class plCCRInvisibleMsg : public plCCRMessage
{
public:
	plKey	fAvKey;
	UInt8	fInvisLevel;	 // 0 means visible

	plCCRInvisibleMsg();
	~plCCRInvisibleMsg() {}

	CLASSNAME_REGISTER( plCCRInvisibleMsg);
	GETINTERFACE_ANY( plCCRInvisibleMsg, plCCRMessage );	

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

//
// For CCR-player communication
//
class plCCRCommunicationMsg : public plCCRMessage
{
public:
	enum Type
	{
		kUnInit = 0,
		kBeginCommunication,
		kChat,
		kEndCommunication,
		kReturnChatMsg
	};
	std::string fString;
	Type fType;
	UInt32 fCCRPlayerID;

	plCCRCommunicationMsg();
	~plCCRCommunicationMsg() {}

	CLASSNAME_REGISTER( plCCRCommunicationMsg);
	GETINTERFACE_ANY( plCCRCommunicationMsg, plCCRMessage );	

	// getters and setters
	void SetMessage(const char* n) { fString=n;	}
	const char* GetMessage() const { return fString.c_str();	}

	void SetType(Type t) { fType=t;	}
	Type GetType() const { return fType; }
	
	void SetCCRPlayerID(UInt32 t) { fCCRPlayerID=t;	}
	UInt32 GetCCRPlayerID() const { return fCCRPlayerID; }
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

//
// A CCR has banned a player from leaving the age
//
class plCCRBanLinkingMsg : public plCCRMessage
{
public:
	hsBool fBan;	// ban or un ban

	plCCRBanLinkingMsg() ;
	~plCCRBanLinkingMsg() {}

	CLASSNAME_REGISTER( plCCRBanLinkingMsg);
	GETINTERFACE_ANY( plCCRBanLinkingMsg, plCCRMessage );	

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

//
// A CCR has silenced a player
//
class plCCRSilencePlayerMsg : public plCCRMessage
{
public:
	hsBool fSilence;	// ban or un ban

	plCCRSilencePlayerMsg() ;
	~plCCRSilencePlayerMsg() {}

	CLASSNAME_REGISTER( plCCRSilencePlayerMsg);
	GETINTERFACE_ANY( plCCRSilencePlayerMsg, plCCRMessage );	

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

#endif	// plCCRMsg_h

