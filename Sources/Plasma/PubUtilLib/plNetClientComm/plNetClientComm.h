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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClientComm/plNetClientComm.h
*
*   This module is the translation layer between simple network types
*   such as byte arrays, and higher-level Plasma-specific types such
*   as the plFactory-managed types.
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H


#include "hsTypes.h"
#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plNetCommonHelpers.h"
#include "../plMessage/plNetCommMsgs.h"


class plNetMessage;


/*****************************************************************************
*
*   NetClientComm API
*
***/

struct NetCommPlayer {
    unsigned    playerInt;
    wchar       playerName[kMaxPlayerNameLength];
    char        playerNameAnsi[kMaxPlayerNameLength];
    char		avatarDatasetName[64];
	unsigned	explorer;
};

struct NetCommAccount {
    Uuid        accountUuid;
    wchar       accountName[kMaxAccountNameLength];
    ShaDigest	accountNamePassHash;
    char        accountNameAnsi[kMaxAccountNameLength];
	unsigned	accountFlags;
	unsigned	billingType;
};

struct NetCommAge {
    Uuid        ageInstId;
    unsigned	ageVaultId;
    char		ageDatasetName[kMaxAgeNameLength];
    char		spawnPtName[64];
};

const NetCommAge * const			NetCommGetAge ();
const NetCommAge * const			NetCommGetStartupAge ();
const NetCommAccount * const		NetCommGetAccount ();
const NetCommPlayer * const			NetCommGetPlayer ();
const ARRAY(NetCommPlayer) &		NetCommGetPlayerList ();
unsigned							NetCommGetPlayerCount ();
bool								NetCommIsLoginComplete ();
void								NetCommSetReadIniAccountInfo (bool readFromIni);
void								NetCommSetAccountUsernamePassword (wchar username[], const ShaDigest &	namePassHash);
void								NetCommSetAuthTokenAndOS (wchar authToken[], wchar os[]);
ENetError							NetCommGetAuthResult ();

bool								NetCommNeedToLoadAvatar ();
void								NetCommSetAvatarLoaded (bool loaded = true);
void								NetCommChangeMyPassword (const wchar password[]);

void NetCommStartup ();
void NetCommShutdown ();
void NetCommUpdate ();
void NetCommConnect ();
void NetCommDisconnect ();
void NetCommSendMsg (
    plNetMessage *  msg
);
void NetCommRecvMsg (
    plNetMessage *  msg
);
void NetCommEnableNet (
	bool			enabled,
	bool			wait
);
void NetCommActivatePostInitErrorHandler();


/*****************************************************************************
*
*   Net message handlers
*
***/

// Return this value from your registered msg handler
// to stop further dispatching of incoming msg.
const unsigned kOK_MsgConsumed  = hsOK + 1;

typedef int (FNetCommMsgHandler)(
    plNetMessage *  msg,
    void *          userState
);

// Adds a msg handler for a msg that is convertable to specified type.
void NetCommAddMsgHandlerForType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    void *                  userState
);
// Adds a msg handler for a specific msg type.
void NetCommAddMsgHandlerForExactType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    void *                  userState
);

extern const unsigned       kNetCommAllMsgClasses;
extern FNetCommMsgHandler * kNetCommAllMsgHandlers;
extern const void *         kNetCommAllUserStates;

void NetCommRemoveMsgHandler (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    const void *            userState
);

void NetCommSetDefaultMsgHandler (
    FNetCommMsgHandler *    handler,
    void *                  userState
);
void NetCommSetMsgPreHandler (
    FNetCommMsgHandler *    handler,
    void *                  userState
);

/*****************************************************************************
*
*   Network requests
*   Network replies are posted via plDispatch
*
***/

void NetCommAuthenticate (  // --> plNetCommAuthMsg
    void *                  param
);
void NetCommGetFileList (   // --> plNetCommFileListMsg
    const wchar             dir[],
    const wchar             ext[],
    void *                  param
);
void NetCommGetFile (       // --> plNetCommFileDownloadMsg
    const wchar             filename[],
    hsStream *              writer,
    void *                  param
);
void NetCommLinkToAge (     // --> plNetCommLinkToAgeMsg
    const NetCommAge &		age,
    void *                  param
);
void NetCommSetActivePlayer (//--> plNetCommActivePlayerMsg
	unsigned				desiredPlayerInt,
    void *                  param
);
void NetCommCreatePlayer (  // --> plNetCommCreatePlayerMsg
    const char				playerName[],
    const char				avatarShape[],
	const char				friendInvite[],
    unsigned                createFlags,
    void *                  param
);
void NetCommCreatePlayer (	// --> plNetCommCreatePlayerMsg
	const wchar				playerName[],
	const wchar				avatarShape[],
	const wchar				friendInvite[],
	unsigned				createFlags,
	void *					param
);
void NetCommDeletePlayer (  // --> plNetCommDeletePlayerMsg
    unsigned                playerInt,
    void *                  param
);
void NetCommGetPublicAgeList (//-> plNetCommPublicAgeListMsg
	const char						ageName[],
    void *							param,
    plNetCommReplyMsg::EParamType	ptype = plNetCommReplyMsg::kParamTypeOther
);
void NetCommSetAgePublic (  // --> no msg
    unsigned				ageInfoId,
    bool                    makePublic
);
void NetCommCreatePublicAge (// --> plNetCommPublicAgeMsg
	const char				ageName[],
    const Uuid &            ageInstId,
    void *                  param
);
void NetCommRemovePublicAge(// --> plNetCommPublicAgeMsg
    const Uuid &            ageInstId,
    void *                  param
);
void NetCommRegisterOwnedAge (
	const NetCommAge &		age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
);
void NetCommUnregisterOwnedAge (
	const char				ageName[],
    unsigned                playerInt,
    void *                  param
);
void NetCommRegisterVisitAge (
	const NetCommAge &		age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
);
void NetCommUnregisterVisitAge (
    const Uuid &            ageInstId,
    unsigned                playerInt,
    void *                  param
);
void NetCommConnectPlayerVault (
	void *					param
);
void NetCommDisconnectPlayerVault ();
void NetCommConnectAgeVault (
	const Uuid &			ageInstId,
	void *					param
);
void NetCommDisconnectAgeVault ();
void NetCommUpgradeVisitorToExplorer (
	unsigned				playerInt,
	void *					param
);
void NetCommSetCCRLevel (
	unsigned				ccrLevel
);
void NetCommSendFriendInvite (
	const wchar		emailAddress[],
	const wchar		toName[],
	const Uuid&		inviteUuid
);

#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H




/*****************************************************************************
*
*   Old code...
*
***/
#ifndef plNetClientComm_h_inc
#define plNetClientComm_h_inc


////////////////////////////////////////////////////////////////////

class plCreatable;
class plStatusLog;
class plAgeLinkStruct;
class plNetClientCommTask;
class plNetMessage;
class	plNetMsgTerminated;
class plPlayerMigrationPkg;

////////////////////////////////////////////////////////////////////
// plNetClientComm
//  - Sends/Recvs messages with a server.
//	- Performs common network tasks (eg. auth, ping, join, leave,
//		create player, delete player, etc.).
//  - Calls you back when network task is complete.
//	- Fwds unsolicited messages to your supplied handler.
//		- You can register a msg handler msg type family (i.e. msg
//			type or base of type).
//		- You can register a msg handler for an exact msg type.
//	- Sends periodic alive msg to server when authenticated.
//	- Tracks message receipts for message types you specify.
//	- Checks for server silence.
//

class plNetClientComm
{
public:
	////////////////////////////////////////////////////////////////
	// CALLBACK CLASSES

	// Callback object sent with calls to our API as optional arg.
	class Callback
	{
	public:
		// OPERATION-SPECIFIC RESULT ARGS.
		//-------------------------------
		// Auth:			0=guid of server that authenticated us, if success.
		// Leave:			None
		// Alive:			None
		// Ping:			0=The plNetMsgPing rcvd.
		// FindAge:			0=plNetServerSessionInfo for spawned game server.
		// Join:			0=fireWalled bool, 1=joinOrder int, 2=experimentalLevel int, 3=initialAgeSDL rec
		// GetPlayerList:	0=numPlayers int, 1=player1ID int, 2=player1Name string, 3=flags ...
		// SetActivePlayer:	None
		// CreatePlayer:	0=newPlayerID int
		// DeletePlayer:	None
		// GetPublicAgeList:0=plCreatableStream of plAgeInfoStruct, 1=plCreatableStream of nPlayers
		// PublicAgeCreated:0=plAgeInfoStruct
		plCreatableListHelper	fCbArgs;
		//-------------------------------
		virtual ~Callback(){}
		virtual void OperationStarted( UInt32 context ) = 0;
		virtual void OperationComplete( UInt32 context, int resultCode ) = 0;
	};
	class StubbedCallback : public Callback
	{
	public:
		void OperationStarted( UInt32 context ) {}
		void OperationComplete( UInt32 context, int resultCode ) {}
	};

	// Message handler for unsolicited msgs or registered for specific msg types.
	class MsgHandler
	{
	public:
        static int StaticMsgHandler(plNetMessage * msg, void * userState);
		virtual int HandleMessage( plNetMessage* msg ) = 0;
	};

	////////////////////////////////////////////////////////////////

	plNetClientComm();
	~plNetClientComm();

	////////////////////////////////////////////////////////////////

	// Adds a msg handler for a msg that is convertable to specified type.
	void	AddMsgHandlerForType( UInt16 msgClassIdx, MsgHandler* handler );

	// Adds a msg handler for a specific msg type.
	void	AddMsgHandlerForExactType( UInt16 msgClassIdx, MsgHandler* handler );

	bool	RemoveMsgHandler( MsgHandler* handler );

	// Msgs not part of a task controlled by this
	// object, and doesn't have a handler set for its type
	// are sent to this handler (if set).
	void	SetDefaultHandler( MsgHandler* msgHandler );
};

////////////////////////////////////////////////////////////////////
#endif // plNetClientComm_h_inc
