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
#ifndef plNetLinkingMgr_h_inc
#define plNetLinkingMgr_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsBitVector.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plMessage/plLinkToAgeMsg.h"

class plMessage;
struct plNCAgeJoiner;
struct plNCAgeLeaver;

class plNetLinkingMgr
{
	static void NCAgeJoinerCallback (
		plNCAgeJoiner *			joiner,
		unsigned				type,
		void *					notify,
		void *					userState
	);
	
	static void NCAgeLeaverCallback (
		plNCAgeLeaver *			leaver,
		unsigned				type,
		void *					notify,
		void *					userState
	);

	friend struct NCAgeJoinerCallback;
	friend struct NCAgeLeaverCallback;
	
	static void ExecNextOp ();


	plNetLinkingMgr();
	plNetLinkingMgr(const plNetLinkingMgr &);

	enum Cmds
	{
		kNilCmd,
		// Sent to a player to have them call us back with info for linking to their age.
		kLinkPlayerHere,
		// Offer link to player.
		kOfferLinkToPlayer,
		// Offer link to another player from a public linking book (to my instance of the age w/out going through personal age)
		kOfferLinkFromPublicBook,
		// Sent to a player to have them link to their last age 
		kLinkPlayerToPrevAge
	};

	bool IPreProcessLink( void );
	void IPostProcessLink( void );
	bool IProcessLinkingMgrMsg( plLinkingMgrMsg * msg );
	bool IProcessLinkToAgeMsg( plLinkToAgeMsg * msg );

	bool IDispatchMsg( plMessage * msg, UInt32 playerID );


public:
	static plNetLinkingMgr * GetInstance();
	hsBool MsgReceive( plMessage *msg );	// TODO: Make this a hsKeyedObject so we can really handle messages.
	void Update();

	bool IsEnabled( void ) const { return fLinkingEnabled;}
	void SetEnabled( bool b );
	
	bool LinkedIn () const { return  fLinkedIn &&  fLinkingEnabled; }
	bool Linking () const  { return !fLinkedIn && !fLinkingEnabled; }

	// Link to an age.
	void LinkToAge( plAgeLinkStruct * link, UInt32 playerID=kInvalidPlayerID );
	void LinkToAge( plAgeLinkStruct * link, const char* linkAnim, UInt32 playerID=kInvalidPlayerID );
	// Link to my last age.
	void LinkToPrevAge( UInt32 playerID=kInvalidPlayerID );	
	// Link to my Personal Age
	void LinkToMyPersonalAge( UInt32 playerID=kInvalidPlayerID );
	// Link to my Neighborhood Age
	void LinkToMyNeighborhoodAge( UInt32 playerID=kInvalidPlayerID );
	// Link a player here.
	void LinkPlayerHere( UInt32 playerID );
	// Link player to specified age
	void LinkPlayerToAge( plAgeLinkStruct * link, UInt32 playerID );
	// Link player back to his last age
	void LinkPlayerToPrevAge( UInt32 playerID );
	// Link us to a players age.
	void LinkToPlayersAge( UInt32 playerID );
	// Offer a link to player.
	void OfferLinkToPlayer( const plAgeLinkStruct * info, UInt32 playerID, plKey replyKey );
	void OfferLinkToPlayer( const plAgeInfoStruct * info, UInt32 playerID );
	void OfferLinkToPlayer( const plAgeLinkStruct * info, UInt32 playerID );
	// Leave the current age
	void LeaveAge (bool quitting);

	// link info
	plAgeLinkStruct * GetAgeLink() { return &fAgeLink; }
	plAgeLinkStruct * GetPrevAgeLink() { return &fPrevAgeLink; }

	// lobby info
	void SetLobbyAddr( const char * ipaddr ) { fLobbyInfo.SetServerAddr( ipaddr );}
	void SetLobbyPort( int port ) { fLobbyInfo.SetServerPort( port );}
	const plNetServerSessionInfo * GetLobbyServerInfo( void ) const { return &fLobbyInfo;}

	// helpers
	static std::string GetProperAgeName( const char * ageName );	// attempt to fix wrong case age name.

private:
	bool				fLinkingEnabled;
	bool				fLinkedIn;

	// The age we are either joining or are joined with.
	plAgeLinkStruct		fAgeLink;

	// The age we just left.
	plAgeLinkStruct		fPrevAgeLink;

	// The lobby we want to talk to.
	plNetServerSessionInfo	fLobbyInfo;
};


#endif // plNetLinkingMgr_h_inc
