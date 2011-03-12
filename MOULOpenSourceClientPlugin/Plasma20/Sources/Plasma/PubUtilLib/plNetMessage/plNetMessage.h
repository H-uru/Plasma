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
#ifndef plNetMessage_h_inc
#define plNetMessage_h_inc

#include "hsUtils.h"
#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsStream.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
#include "plGeneric.h"
#include "../pnNetCommon/plNetServers.h"
#include "../pnNetCommon/plNetGroup.h"
#include "../pnFactory/plCreatable.h"
#include "../pnFactory/plFactory.h"
#include "../plUnifiedTime/plClientUnifiedTime.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plNetCommon/plNetCommonHelpers.h"
#include "../plNetCommon/plNetCommonConstants.h"

#include "../plStreamLogger/plStreamLogger.h"

#include "plNetMsgHelpers.h"

#include "../pnNetBase/pnNetBase.h"

#include <sstream>

class plMessage;
class plUUID;


////////////////////////////////////////////////////////////////////

//
// Base class for application network messages.
// These become the data in a plNetCommonMessage when sent over the network.
//
class plNetCommonMessage;
class plKey;

class plNetMessage : public plCreatable
{	
	friend class plNetServerMsgPlsRoutableMsg;

	UInt32 fFlags;		// indicates what is present in the message, always transmitted 
	plUnifiedTime fTimeSent;	// time msg was sent (in sender's unified timeSpace), always transmitted to and from the client
	double fTimeRecvd;		// time msg was recvd (in rcvrs timeSpace), never transmitted 
	UInt32 fBytesRead;		// amount of data we've already read, never transmitted 
	UInt32 fContext;		// set by sender, included in reply. Only written if kHasContext flag set
	UInt32 fTransactionID;	// set by originator, included in reply. Only written if kHasTransactionID flag set
	UInt32 fPlayerID;		// set by originator. Only written if kHasPlayerID flag set
	plUUID fAcctUUID;		// set by sender (app level). Only written if kHasAcctUUID flag set
	const plNetCommonMessage* fNetCoreMsg;	// not sent, set by the receiver
	UInt32 fPeekStatus;		// not sent. set on PeekBuffer, cleared on PokeBuffer
	UInt8	fProtocolVerMajor;	// conditionally sent
	UInt8	fProtocolVerMinor;	// conditionally sent
    ENetProtocol fNetProtocol;  // the server this msg should be sent to. this value is not sent over wire.

	enum ContentFlags
	{
		kNetMsgFlags,
		kNetMsgTimeSent,
		kNetMsgContext,
		kNetMsgTransactionID,
		kNetMsgPlayerID,
		kNetMsgVersion,
	};

protected:	
	virtual int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	virtual int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);

	void IWriteClassIndex(hsStream* stream);
	void IReadClassIndex(hsStream* stream);

	bool IPeeked() const { return fPeekStatus==kFullyPeeked;}
	void ISetPeekStatus(UInt32 s) { fPeekStatus=s;	}

public:
	typedef UInt16 plStrLen;
	static const UInt8 kVerMajor, kVerMinor;	// version of the networking code

	typedef UInt16 ClassIndexType;		// the type returned by plCreatable::ClassIndex()
	enum
	{
		kMaxNameLen=32
	};
	enum BitVectorFlags		// indicates what is present in the message, always transmitted
	{
		kHasTimeSent		= 0x1,		// means fTimeSent need sending
		kHasGameMsgRcvrs	= 0x2,		// means that this is a direct (not bcast) game msg
		kEchoBackToSender	= 0x4,		// if broadcasting, echo packet back to sender
		kRequestP2P			= 0x8,		// sent to gameServer on joinReq
		kAllowTimeOut		= 0x10,		// sent to gameServer on joinReq (if release code)
		kIndirectMember		= 0x20,		// sent to client on joinAck if he is behind a firewall
		// This flag is used when the servers are firewalled and NAT'ed
		// It tells a game or lobby server to ask the lookup for an external address
		kPublicIPClient		= 0x40,		// set on a client coming from a public IP
		kHasContext			= 0x80,		// whether or not to write fContext field
		// Used with StartProcess server msgs. Whether or not the ServerAgent
		// must first ask the vault for a game state associated with the
		// game about to be instanced.
		kAskVaultForGameState	=0x100,
		kHasTransactionID	= 0x200,	// whether or not to write fTransactionID field
		kNewSDLState		= 0x400,	// set by client on first state packet sent, may not be necessary anymore
		kInitialAgeStateRequest = 0x800,// initial request for the age state
		kHasPlayerID		= 0x1000,	// is fPlayerID set
		kUseRelevanceRegions= 0x2000,	// if players use relevance regions are used, this will be filtered by region, currently set on avatar physics and control msgs
		kHasAcctUUID		= 0x4000,	// is fAcctUUID set
		kInterAgeRouting	= 0x8000,	// give to pls for routing.
		kHasVersion			= 0x10000,	// if version is set
		kIsSystemMessage	= 0x20000,
		kNeedsReliableSend	= 0x40000,
		kRouteToAllPlayers	= 0x80000,	// send this message to all online players.
	};
	enum PeekOptions		// options for partial peeking
	{
		kSkipStream			= 1<<0,	// means we should not read the actual stream, just the stream info
		kBaseClassOnly		= 1<<1,	// only peek or poke the baseclass info
		kPartialPeekMask	= kSkipStream | kBaseClassOnly,
		// plNetServerMsgWithRoutingInfo derived classes need to check this
		// in their PeekBuffer methods.
		kJustRoutingInfo	= 1<<2,
		kDontClearBuffer	= 1<<3,
		// don't call base class peek/poke. plNetMsgOmnibus/plNetMsgVault uses this.
		kDEAD_NoBaseClass	= 1<<4,
		// don't worry about compressing/uncompressing things. used by plNetMsgStreamHelper, and plNetMsgOmnibus.
		kDontCompress		= 1<<5,
		kWantVersion		= 1<<6,
		kFullyPeeked		= 0xffffffff	// default fPeekStatus, set if not partial peeking
	};
	enum CompressionType	// currently only used for plNetMsgStreams
	{
		kCompressionNone,		// not compressed
		kCompressionFailed,		// failed to compress
		kCompressionZlib,		// zlib compressed
		kCompressionDont		// don't compress
	};

	CLASSNAME_REGISTER( plNetMessage );
	GETINTERFACE_ANY( plNetMessage, plCreatable );

	// plCreatable
	void Read(hsStream* s, hsResMgr* mgr);
	void Write(hsStream* s, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// ctor
	plNetMessage();
	virtual ~plNetMessage();

	static plNetMessage* CreateAndRead(const plNetCommonMessage*, plStreamLogger::EventList* el = nil);
	static plNetMessage* Create(const plNetCommonMessage*);
	int PokeBuffer(char* buf, int bufLen, UInt32 peekOptions=0);			// put msg in buffer
	int PeekBuffer(const char* buf, int bufLen, UInt32 peekOptions=0, bool forcePeek=false, plStreamLogger::EventList* el = nil);	// get msg out of buffer
	bool NeedsReliableSend() const { return IsBitSet(kNeedsReliableSend); }
	bool IsSystemMessage() const { return IsBitSet(kIsSystemMessage); 	}
	virtual void ValidatePoke() const;
	virtual void ValidatePeek() const;
	virtual bool NeedsBroadcast() const { return false; }			// should game server broadcast this message to other clients?
	virtual int ValidationSchemeID() const { return 1; }

	// getters
	int GetPackSize();
	const plUnifiedTime& GetTimeSent() const { return fTimeSent; }
	bool GetHasTimeSent() const { return IsBitSet(kHasTimeSent); }
	double GetTimeReceived() const { return fTimeRecvd; }
	bool IsBitSet(int b) const { return (fFlags & b) != 0; }
	const plNetCommonMessage* GetNetCoreMsg() const { return fNetCoreMsg; }
	UInt32 GetNetCoreMsgLen() const;
	bool GetHasContext() const { return IsBitSet(kHasContext);}
	UInt32 GetContext() const { return fContext;}
	bool GetHasTransactionID() const { return IsBitSet(kHasTransactionID);}
	UInt32 GetTransactionID() const { return fTransactionID;}
	bool GetHasPlayerID() const { return IsBitSet(kHasPlayerID);}
	UInt32 GetPlayerID() const { hsAssert(GetHasPlayerID(), "uninit playerID"); return fPlayerID; }
	UInt32 JustGetPlayerID() const { return fPlayerID; }
	bool GetHasAcctUUID() const { return IsBitSet(kHasAcctUUID); }
	const plUUID * GetAcctUUID() const { return &fAcctUUID; }
	UInt8 GetVersionMajor() const { return fProtocolVerMajor;	}
	UInt8 GetVersionMinor() const { return fProtocolVerMinor;	}
    ENetProtocol GetNetProtocol () const { return fNetProtocol; }

	// setters
	void SetTimeSent(const plUnifiedTime& t) { fTimeSent=t;SetHasTimeSent(true); }
	void SetHasTimeSent(bool value) { SetBit( kHasTimeSent, value ); }
	void SetTimeReceived(double t) { fTimeRecvd=t; }
	void SetBit(int b, bool on=true) { if (on) fFlags |= b; else fFlags &= ~b; }
	void SetNetCoreMsg(const plNetCommonMessage* ncmsg) { fNetCoreMsg=ncmsg; }
	void SetHasContext(bool value) { SetBit(kHasContext,value);}
	void SetContext(UInt32 value) { fContext=value; SetHasContext(true);}
	void SetHasTransactionID(bool value) { SetBit(kHasTransactionID,value);}
	void SetTransactionID(UInt32 value) { fTransactionID=value; SetHasTransactionID(true);}
	void SetHasPlayerID(bool value) { SetBit(kHasPlayerID,value);}
	void SetPlayerID(UInt32 value) { fPlayerID=value; SetHasPlayerID(true);}
	void SetHasAcctUUID( bool v ) { SetBit( kHasAcctUUID,v ); }
	void SetAcctUUID(const plUUID * v ) { fAcctUUID.CopyFrom(v); SetHasAcctUUID(true); }
	void SetVersion(UInt8 maj=kVerMajor, UInt8 min=kVerMinor) { SetBit(kHasVersion); fProtocolVerMajor=maj; fProtocolVerMinor=min;	}
    void SetNetProtocol (ENetProtocol v ) { fNetProtocol = v; }

	// init fContext, fTransactionID, etc. if needed.
	void InitReplyFieldsFrom(plNetMessage * msg);

	// debug
	virtual std::string AsStdString() const
	{
		char * delim = "";

		std::stringstream ss;
		if ( GetHasPlayerID() )
		{
			ss << delim << "p:" << GetPlayerID();
			delim = ",";
		}
		if ( GetHasTransactionID() )
		{
			ss << delim << "x:" << GetTransactionID();
			delim = ",";
		}
		if ( GetHasAcctUUID() )
		{
			ss << delim << "a:" << GetAcctUUID()->AsStdString();
			delim = ",";
		}
		if ( IsBitSet(kHasVersion) )
		{
			ss << delim << "v:" << (int)fProtocolVerMajor << "." << (int)fProtocolVerMinor;
			delim = ",";
		}

		return ss.str().c_str();
	}
};

// 
// for msgs which only go from the gameserver to the client
//
class plNetMsgServerToClient : public plNetMessage
{
public:
	plNetMsgServerToClient() { SetBit(kIsSystemMessage|kNeedsReliableSend);	}

	CLASSNAME_REGISTER( plNetMsgServerToClient );
	GETINTERFACE_ANY( plNetMsgServerToClient, plNetMessage );
};

////////////////////////////////////////////////////////////////////
// Stream msg - abstract base class built with plNetMsgStreamHelper
//
class plNetMsgStream : public plNetMessage
{
protected:
	plNetMsgStreamHelper fStreamHelper;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	CLASSNAME_REGISTER( plNetMsgStream );
	GETINTERFACE_ANY_AUX(plNetMsgStream,plNetMessage,plNetMsgStreamHelper,fStreamHelper)

	plNetMsgStreamHelper* StreamInfo() { return &fStreamHelper; }
	const plNetMsgStreamHelper* StreamInfo() const { return &fStreamHelper; }
};

//
// Object info msg - abstract base class built with plNetMsgObjectHelper
//
class plNetMsgObject : public plNetMessage
{
private:
	enum ContentFlags
	{
		kNetMsgObjectHelper,
	};
protected:
	plNetMsgObjectHelper fObjectHelper;
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	CLASSNAME_REGISTER( plNetMsgObject );
	GETINTERFACE_ANY_AUX(plNetMsgObject,plNetMessage,plNetMsgObjectHelper,fObjectHelper)

	plNetMsgObjectHelper* ObjectInfo() { return &fObjectHelper; }
	const plNetMsgObjectHelper* ObjectInfo() const { return &fObjectHelper; }

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// debug
	std::string AsStdString() const
	{
		std::string s;
		char tmp[256];
		xtl::format(s,"object=%s, %s",fObjectHelper.GetUoid().StringIze(tmp), plNetMessage::AsStdString().c_str());
		return s;
	}

};

//
// abstract baseclass which has both object and stream info
//
class plNetMsgStreamedObject : public plNetMsgObject
{
private:
	enum ContentFlags
	{
		kStreamHelper
	};
protected:
	plNetMsgStreamHelper fStreamHelper;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgStreamedObject() {}
	~plNetMsgStreamedObject() {}

	CLASSNAME_REGISTER( plNetMsgStreamedObject );
	GETINTERFACE_ANY_AUX(plNetMsgStreamedObject,plNetMsgObject,plNetMsgStreamHelper,fStreamHelper)

	plNetMsgStreamHelper* StreamInfo() { return &fStreamHelper; }
	const plNetMsgStreamHelper* StreamInfo() const { return &fStreamHelper; }

	//virtuals
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

};

//
// New SaveState system
//
class plNetMsgSDLState : public plNetMsgStreamedObject
{
private:
	enum ContentFlags
	{
		kSDLStateStream,
		kSDLIsInitialState,
		kSDLPersist,
		kSDLAvatarState,
	};

	void ISetDescName() const;
	bool	fIsInitialState;

protected:
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);

	bool fPersistOnServer;
	bool fIsAvatarState;
	mutable std::string fDescName;		// for debugging output only, not read/written
public:
	CLASSNAME_REGISTER( plNetMsgSDLState );
	GETINTERFACE_ANY(plNetMsgSDLState, plNetMsgStreamedObject);

	plNetMsgSDLState() : fIsInitialState(0), fPersistOnServer(true), fIsAvatarState(false) { SetBit(kNeedsReliableSend); }

	bool PersistOnServer() const { return fPersistOnServer != 0; }
	void SetPersistOnServer(bool b) { fPersistOnServer = b; }

	bool IsAvatarState() const { return fIsAvatarState != 0; }
	void SetIsAvatarState(bool b) { fIsAvatarState = b; }
	
	// debug
	std::string AsStdString() const;
	bool IsInitialState() const {return fIsInitialState!=0; }
	void SetIsInitialState( bool v ) { fIsInitialState=v; }

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

//
// New SaveState system
//
class plNetMsgSDLStateBCast : public plNetMsgSDLState
{
protected:	
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:	
	CLASSNAME_REGISTER( plNetMsgSDLStateBCast );
	GETINTERFACE_ANY(plNetMsgSDLStateBCast, plNetMsgSDLState);

	// virtuals 
	bool NeedsBroadcast() const { return true; }

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

//
//  Object state request msg
//
class plNetMsgObjStateRequest : public plNetMsgObject
{
public:
	plNetMsgObjStateRequest() { SetBit(kIsSystemMessage|kNeedsReliableSend); 	}

	CLASSNAME_REGISTER( plNetMsgObjStateRequest );
	GETINTERFACE_ANY( plNetMsgObjStateRequest, plNetMsgObject );
};


//
// abstract message class which contains a list of rooms
//
class plNetMsgRoomsList : public plNetMessage
{
protected:
	std::vector<plLocation> fRooms;	
	std::vector<char*> fRoomNames;		// for debug usage only

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgRoomsList() {}
	~plNetMsgRoomsList();

	CLASSNAME_REGISTER( plNetMsgRoomsList );
	GETINTERFACE_ANY( plNetMsgRoomsList, plNetMessage );

	void AddRoom(plKey rmKey);
	void AddRoomLocation(plLocation loc, const char* rmName);
	int FindRoomLocation(plLocation loc);

	int GetNumRooms() const { return fRooms.size(); }
	plLocation GetRoomLoc(int i) const { return fRooms[i]; }
	const char* GetRoomName(int i) const { return fRoomNames[i]; }		// debug
};

//
// Game msg - wraps a plMessage.
//
class hsResMgr;

class plNetMsgGameMessage: public plNetMsgStream
{
protected:
	plClientUnifiedTime fDeliveryTime;	// for future timestamping

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgGameMessage() { SetBit(kNeedsReliableSend); 	}

	CLASSNAME_REGISTER( plNetMsgGameMessage );
	GETINTERFACE_ANY( plNetMsgGameMessage, plNetMsgStream );
	enum ContentsFlags
	{
		kNetGameMsgDeliveryTime,
		kNetGameMsgGameMsg,
	};
	plClientUnifiedTime& GetDeliveryTime() { return fDeliveryTime; }
	void SetDeliveryTime(plClientUnifiedTime& ut) { fDeliveryTime=ut; }

	plMessage* GetContainedMsg(hsResMgr* resmgr = nil);

	// virtuals
	bool NeedsBroadcast() const { return true; }
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// debug
	std::string AsStdString() const
	{
		std::string s;
		const char* noc=plFactory::GetTheFactory()->GetNameOfClass(StreamInfo()->GetStreamType());
		xtl::format(s,"%s %s",plNetMsgStream::AsStdString().c_str(), noc ? noc : "?");
		return s;
	}
};

//
// special game msg for loading clones/avatars
//
class plNetMsgLoadClone : public plNetMsgGameMessage
{
private:
	enum ContentFlags
	{
		kObjectHelper,
		kIsPlayer,
		kIsLoading,
		kIsInitialState,
	};
protected:
	bool fIsPlayer;
	bool fIsLoading;
	bool fIsInitialState;
	plNetMsgObjectHelper fObjectHelper;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);	
public:

	CLASSNAME_REGISTER( plNetMsgLoadClone );
	GETINTERFACE_ANY_AUX(plNetMsgLoadClone, plNetMsgGameMessage,plNetMsgObjectHelper,fObjectHelper)

	plNetMsgLoadClone() : fIsPlayer(true),fIsLoading(true),fIsInitialState(false) {}

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	plNetMsgObjectHelper* ObjectInfo() { return &fObjectHelper; }
	const plNetMsgObjectHelper* ObjectInfo() const { return &fObjectHelper; }

	bool GetIsPlayer() const	{ return fIsPlayer!=0;	}
	bool GetIsLoading() const	{ return fIsLoading!=0;	}
	bool GetIsInitialState() const { return fIsInitialState!=0; }

	void SetIsPlayer(bool p)	{ fIsPlayer=p;	}
	void SetIsLoading(bool p)	{ fIsLoading=p;	}
	void SetIsInitialState(bool p) {fIsInitialState=p; }


	// debug
	std::string AsStdString() const
	{
		std::string s;
		char tmp[256];
		xtl::format(s,"object=%s initial=%d, %s",fObjectHelper.GetUoid().StringIze(tmp), fIsInitialState,
			plNetMsgGameMessage::AsStdString().c_str());
		return s;
	}
};

//
// special msg when a player is paged in/out
//
class plNetMsgPlayerPage : public plNetMessage
{
protected:
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);	
public:
	plUoid	fUoid;
	bool fUnload;

	plNetMsgPlayerPage() : fUnload(false) { SetBit(kNeedsReliableSend); }
	plNetMsgPlayerPage(plUoid uoid, hsBool unload) : fUoid(uoid),	fUnload(unload) { }

	CLASSNAME_REGISTER( plNetMsgPlayerPage );
	GETINTERFACE_ANY( plNetMsgPlayerPage, plNetMessage);
};

//
// a game message that takes a list of other clients as receivers
//
class plNetMsgGameMessageDirected : public plNetMsgGameMessage
{
private:
	enum ContentFlags
	{
		kRecievers,
	};
protected:
	plNetMsgReceiversListHelper fReceivers;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);	
public:
	CLASSNAME_REGISTER( plNetMsgGameMessageDirected );
	GETINTERFACE_ANY_AUX(plNetMsgGameMessageDirected,plNetMsgGameMessage,
		plNetMsgReceiversListHelper,fReceivers)

	plNetMsgReceiversListHelper* Receivers() { return &fReceivers; }	

	// virtuals
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

//
// Message when a room is paged in/out
//
class plNetMsgPagingRoom : public plNetMsgRoomsList
{
public:
	enum PageFlags
	{
		kPagingOut=0x1,		// else paging in
		kResetList=0x2,		// server should reset his existing list before using this msg
		kRequestState=0x4,	// also want current room state sent to me
		kFinalRoomInAge=0x8	// done paging in the age
	};
protected:
	UInt8 fPageFlags;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgPagingRoom() : fPageFlags(0) { SetBit(kIsSystemMessage|kNeedsReliableSend); }
	~plNetMsgPagingRoom() {}

	CLASSNAME_REGISTER( plNetMsgPagingRoom );
	GETINTERFACE_ANY( plNetMsgPagingRoom, plNetMsgRoomsList );

	void SetPageFlags(UInt8 f) { fPageFlags=f; }
	UInt8 GetPageFlags() const { return fPageFlags; }
	
	void SetPagingOut(hsBool b) { if (b) fPageFlags |= kPagingOut; else fPageFlags&=~kPagingOut; }
	hsBool GetPagingOut() const { return (fPageFlags & kPagingOut) != 0; }

	void SetResetList(hsBool b) { if (b) fPageFlags |= kResetList; else fPageFlags &=~kResetList; }
	hsBool GetResetList() const { return (fPageFlags & kResetList) != 0; }

	void SetRequestingState(hsBool b) { if (b) fPageFlags |= kRequestState; else fPageFlags &=~kRequestState; }
	hsBool GetRequestingState() const { return (fPageFlags & kRequestState) != 0; }	

	// debug
	std::string AsStdString() const
	{
		std::string s;
		xtl::format(s,"pageFlags:%02X, paging %s, requestingState:%s, resetting=%d",fPageFlags,
			(fPageFlags&kPagingOut)?"out":"in", (fPageFlags&kRequestState)?"yes":"no",
			(fPageFlags & kResetList)!=0);
		return s;
	}
};

//
// Client requests game state update by rooms.
// an empty rooms list means ALL rooms I have loaded.
//
class plNetMsgGameStateRequest : public plNetMsgRoomsList
{
public:
	plNetMsgGameStateRequest() { SetBit(kIsSystemMessage|kNeedsReliableSend); }

	CLASSNAME_REGISTER( plNetMsgGameStateRequest );
	GETINTERFACE_ANY( plNetMsgGameStateRequest, plNetMsgRoomsList );	
};

//
// sent by game server to change clients ownership of a netGroup
//
class plNetMsgGroupOwner: public plNetMsgServerToClient
{
public:
	class GroupInfo
	{
	public:
		plNetGroupId fGroupID;
		bool fOwnIt;	// else not the owner

		void Read(hsStream* s) { fGroupID.Read(s); s->LogReadSwap(&fOwnIt,"GroupOwner OwnIt"); }
		void Write(hsStream* s) { fGroupID.Write(s); s->WriteSwap(fOwnIt); }

      GroupInfo() : fGroupID(plNetGroup::kNetGroupUnknown), fOwnIt(false) {}
		GroupInfo(plNetGroupId gID, hsBool o) : fGroupID(gID),fOwnIt(o) {}
	};
protected:
	std::vector<GroupInfo> fGroups;	

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	CLASSNAME_REGISTER( plNetMsgGroupOwner );
	GETINTERFACE_ANY( plNetMsgGroupOwner, plNetMsgServerToClient );

	// getters
	int GetNumGroups() const { return fGroups.size(); }
	GroupInfo GetGroupInfo(int i) const { return fGroups[i]; }

	// setters
	void AddGroupInfo(GroupInfo gi) { fGroups.push_back(gi); }
	void ClearGroupInfo() { fGroups.clear(); }

	bool IsOwner() { return fGroups[0].fOwnIt; }
};

//
// voice recording buffer
//
class plNetMsgVoice: public plNetMessage
{
private:
	enum ContentFlags
	{
		kDead_FrameSize,
		kReceivers,
		kVoiceFlags,
		kVoiceData
	};
protected:
	UInt8 fFlags;		// voice flags
	UInt8 fNumFrames; // number of frames encoded
	std::string fVoiceData;

	plNetMsgReceiversListHelper fReceivers;
 
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgVoice(): fFlags(0), fNumFrames(0) {  }
	~plNetMsgVoice() {}

	CLASSNAME_REGISTER( plNetMsgVoice );
	GETINTERFACE_ANY_AUX(plNetMsgVoice,plNetMessage,plNetMsgReceiversListHelper,fReceivers)
		
	void SetFlag(int f) { fFlags |= f; }
	int GetFlags() { return fFlags;  }
	
	void SetNumFrames(UInt8 f) { fNumFrames = f; }
	UInt8 GetNumFrames() const { return fNumFrames; }
	
	void SetVoiceData(char *data, int len );
	int GetVoiceDataLen() const { return fVoiceData.length(); }
	const char *GetVoiceData() const;
	
	plNetMsgReceiversListHelper* Receivers() { return &fReceivers; }

	// virtuals
	bool NeedsBroadcast() const { return true; }

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// debug
	std::string AsStdString() const
	{
		std::string s;
		xtl::format(s,"len=%d",fVoiceData.size());
		return s;
	}
};

//
// base class for dealing with plNetSharedState
//
class plNetSharedState;
class plNetMsgSharedState : public plNetMsgStreamedObject
{
private:
	enum ContentFlags
	{
		kLockRequest,
	};
protected:
	bool fLockRequest;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgSharedState() : fLockRequest(false) {}
	~plNetMsgSharedState() {}

	CLASSNAME_REGISTER( plNetMsgSharedState );
	GETINTERFACE_ANY(plNetMsgSharedState, plNetMsgStreamedObject);

	void CopySharedState(plNetSharedState* ss);

	void SetLockRequest(hsBool b) { fLockRequest=b; }
	hsBool GetLockRequest() const { return fLockRequest; }	

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// debug
	std::string AsStdString() const
	{
		std::string s;
		xtl::format(s,"lockReq=%d, %s",fLockRequest, plNetMsgStreamedObject::AsStdString().c_str());
		return s;
	}
};

//
// attempt to lock/unlock and set generic shared state on server.
// lock attempts will generate server reply messages confirming or denying the action.
//
class plNetMsgTestAndSet : public plNetMsgSharedState
{
public:
	plNetMsgTestAndSet() { SetBit(kNeedsReliableSend); }

	CLASSNAME_REGISTER( plNetMsgTestAndSet );
	GETINTERFACE_ANY(plNetMsgTestAndSet, plNetMsgSharedState);
};

//
// provides a way to query sharedState on the server
//
class plNetMsgGetSharedState : public plNetMsgObject
{
protected:
	char fSharedStateName[kMaxNameLen];

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgGetSharedState()  { *fSharedStateName=0; SetBit(kNeedsReliableSend); }
	~plNetMsgGetSharedState() {}

	CLASSNAME_REGISTER( plNetMsgGetSharedState );
	GETINTERFACE_ANY( plNetMsgGetSharedState, plNetMsgObject );

	void SetSharedStateName(char* n) { if (n) hsStrncpy(fSharedStateName, n, kMaxNameLen); }
	char* GetSharedStateName() { return fSharedStateName; }
};

//
// msg which sets the update frequency for a group of objects on the server
//
class plNetMsgObjectUpdateFilter : public plNetMessage
{
protected:
	plNetMsgObjectListHelper fObjectListHelper;
	float fMaxUpdateFreq;	// in secs

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgObjectUpdateFilter() : fMaxUpdateFreq(-1) {}
	~plNetMsgObjectUpdateFilter() {}

	CLASSNAME_REGISTER( plNetMsgObjectUpdateFilter );
	GETINTERFACE_ANY_AUX(plNetMsgObjectUpdateFilter,plNetMessage,plNetMsgObjectListHelper,fObjectListHelper)

	plNetMsgObjectListHelper* ObjectListInfo() { return &fObjectListHelper; }

	void SetMaxUpdateFreq(float f) { fMaxUpdateFreq=f; }
	float GetMaxUpdateFreq() const { return fMaxUpdateFreq; }
};

//
// Client wants a list of all members in the session
//
class plNetMsgMembersListReq : public plNetMessage
{
public:
	plNetMsgMembersListReq() { SetBit(kIsSystemMessage|kNeedsReliableSend); }

	CLASSNAME_REGISTER( plNetMsgMembersListReq );
	GETINTERFACE_ANY( plNetMsgMembersListReq, plNetMessage );
};

//
// Server returns a list of all members in the session
//
class plNetMsgMembersList : public plNetMsgServerToClient
{
protected:
	plNetMsgMemberListHelper fMemberListHelper;
protected:
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	CLASSNAME_REGISTER( plNetMsgMembersList );
	GETINTERFACE_ANY_AUX(plNetMsgMembersList,plNetMsgServerToClient,plNetMsgMemberListHelper,fMemberListHelper)

	plNetMsgMemberListHelper* MemberListInfo() { return &fMemberListHelper; }
};

//
// server tells client to add or remove a session member
//
class plNetMsgMemberUpdate : public plNetMsgServerToClient
{
protected:
	plNetMsgMemberInfoHelper fMemberInfo;
	bool fAddMember;		// else remove member
protected:
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	CLASSNAME_REGISTER( plNetMsgMemberUpdate );
	GETINTERFACE_ANY_AUX(plNetMsgMemberUpdate,plNetMsgServerToClient,plNetMsgMemberInfoHelper,fMemberInfo)

	bool AddingMember() { return fAddMember; }
	void SetAddingMember(bool b) { fAddMember=b; }

	plNetMsgMemberInfoHelper* MemberInfo() { return &fMemberInfo; }
	bool NeedsBroadcast() const { return true; }	// send to all clients
};


//
// ListenList updater msgs.  For voice-broadcasting purposes.
// Contains a list of other players which I am [not] listening to.
// Sent client-client or client-server.
//
class plNetMsgListenListUpdate : public plNetMessage
{
private:
	plNetMsgReceiversListHelper fReceivers;		// used by server, the players we're listening to 
	bool fAdding;							// else removing

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgListenListUpdate() : fAdding(false) {}
	~plNetMsgListenListUpdate() {}

	CLASSNAME_REGISTER( plNetMsgListenListUpdate );
	GETINTERFACE_ANY_AUX(plNetMsgListenListUpdate,plNetMessage,plNetMsgReceiversListHelper,fReceivers)
	
	plNetMsgReceiversListHelper* Receivers() { return &fReceivers; }
	
	bool GetAdding() const { return fAdding;	}
	void SetAdding(bool a) { fAdding=a;	}
			
	// virtuals
	bool NeedsBroadcast() const { return true; }		// use rcvrs list
};


///////////////////////////////////////////////////////////////////
class plNetMsgInitialAgeStateSent : public plNetMsgServerToClient
{
	UInt32	fNumInitialSDLStates;
	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgInitialAgeStateSent():fNumInitialSDLStates(0){}
	CLASSNAME_REGISTER( plNetMsgInitialAgeStateSent );
	GETINTERFACE_ANY( plNetMsgInitialAgeStateSent, plNetMsgServerToClient);
	void SetNumInitialSDLStates( UInt32 n ) { fNumInitialSDLStates=n; }
	UInt32 GetNumInitialSDLStates() const { return fNumInitialSDLStates; }
};

//
// msg which sets the update frequency for a group of objects on the server
//
class plNetMsgRelevanceRegions : public plNetMessage
{
protected:
	hsBitVector fRegionsImIn;
	hsBitVector fRegionsICareAbout;

	int IPokeBuffer(hsStream* stream, UInt32 peekOptions=0);
	int IPeekBuffer(hsStream* stream, UInt32 peekOptions=0);
public:
	plNetMsgRelevanceRegions() { SetBit(kNeedsReliableSend); }
	~plNetMsgRelevanceRegions() {}

	CLASSNAME_REGISTER( plNetMsgRelevanceRegions );
	GETINTERFACE_ANY(plNetMsgRelevanceRegions, plNetMessage)

	void SetRegionsICareAbout(const hsBitVector& r) { fRegionsICareAbout=r; }
	void SetRegionsImIn(const hsBitVector& r)		{ fRegionsImIn=r; }

	const hsBitVector& GetRegionsICareAbout() const { return fRegionsICareAbout;	}
	const hsBitVector& GetRegionsImIn() const		{ return fRegionsImIn;	}

	std::string AsStdString() const
	{
		std::string s;
		std::string b1, b2;
		int i;
		for(i=0;i<fRegionsImIn.GetNumBitVectors(); i++)
			b1 += xtl::format("0x%x ", fRegionsImIn.GetBitVector(i)).c_str();
		for(i=0;i<fRegionsICareAbout.GetNumBitVectors(); i++)
			b2 += xtl::format("0x%x ", fRegionsICareAbout.GetBitVector(i)).c_str();
		xtl::format( s, "rgnsImIn:%s, rgnsICareAbout:%s, %s",
			b1.c_str(), b2.c_str(), plNetMessage::AsStdString().c_str() );
		return s;
	}
};

#endif	// plNetMessage_h_inc
////////////////////////////////////////////////////////////////////
// End.


