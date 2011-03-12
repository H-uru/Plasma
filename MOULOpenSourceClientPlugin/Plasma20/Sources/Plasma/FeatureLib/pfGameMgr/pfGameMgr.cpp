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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/pfGameMgr.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

//============================================================================
// pfGameCli factory
//============================================================================
struct Factory : THashKeyVal<Uuid> {

	HASHLINK(Factory)	link;
	const GameTypeReg &	reg;

	Factory (const GameTypeReg & reg);
	operator= (const Factory &);	// not impl
};

//============================================================================
// pfGameCli internal state
//============================================================================
struct IGameCli : THashKeyVal<unsigned> {

	HASHLINK(IGameCli)	link;
	pfGameCli *			gameCli;
	Factory *			factory;
	plKey				receiver;
	unsigned			playerCount;
	
	IGameCli (
		pfGameCli *	gameCli,
		unsigned	gameId,
		plKey		receiver
	);

	void Recv				(GameMsgHeader * msg, void * param);
	void RecvPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg, void * param);
	void RecvPlayerLeft		(const Srv2Cli_Game_PlayerLeft & msg, void * param);
	void RecvInviteFailed	(const Srv2Cli_Game_InviteFailed & msg, void * param);
	void RecvOwnerChange	(const Srv2Cli_Game_OwnerChange & msg, void * param);
};

//============================================================================
// Transaction states
//============================================================================
struct TransState : THashKeyVal<unsigned> {

	HASHLINK(TransState)	link;
	void *					param;
	TransState (unsigned transId, void * param);
};
struct JoinTransState {

	plKey receiver;
	JoinTransState (plKey receiver)
	:	receiver(receiver)
	{ }
};

//============================================================================
// IGameMgr
//============================================================================
struct IGameMgr {

	pfGameCli * CreateGameCli (const Uuid & gameTypeId, unsigned gameId, plKey receiver);

	void Recv				(GameMsgHeader * msg);
	void RecvGameInstance	(const Srv2Cli_GameMgr_GameInstance & msg, void * param);
	void RecvInviteReceived	(const Srv2Cli_GameMgr_InviteReceived & msg, void * param);
	void RecvInviteRevoked	(const Srv2Cli_GameMgr_InviteRevoked & msg, void * param);

	static void StaticRecv (GameMsgHeader * msg);
};


/*****************************************************************************
*
*   Local data
*
***/

static HASHTABLEDECL(TransState, THashKeyVal<unsigned>, link)	s_trans;
static HASHTABLEDECL(IGameCli, THashKeyVal<unsigned>, link)		s_games;
static HASHTABLEDECL(Factory, THashKeyVal<Uuid>, link)			s_factories;

static long				s_transId;
static ARRAYOBJ(plKey)	s_receivers;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void ShutdownFactories () {

	while (Factory * factory = s_factories.Head())
		DEL(factory);
}

//============================================================================
AUTO_INIT_FUNC (SetGameMgrMsgHandler) {

	NetCliGameSetRecvGameMgrMsgHandler(IGameMgr::StaticRecv);
	atexit(ShutdownFactories);
}

//============================================================================
static inline unsigned INextTransId () {

	unsigned transId = AtomicAdd(&s_transId, 1);
	while (!transId)
		transId = AtomicAdd(&s_transId, 1);
	return transId;
}


/*****************************************************************************
*
*   IGameMgr
*
***/

//============================================================================
pfGameCli * IGameMgr::CreateGameCli (const Uuid & gameTypeId, unsigned gameId, plKey receiver) {
	
	if (Factory * factory = s_factories.Find(gameTypeId)) {
		pfGameCli * gameCli = factory->reg.create(gameId, receiver);
		gameCli->internal->factory = factory;
		return gameCli;
	}
	
	return nil;
}

//============================================================================
void IGameMgr::RecvGameInstance (const Srv2Cli_GameMgr_GameInstance & msg, void * param) {

	JoinTransState * state = (JoinTransState *)param;

	pfGameCli *	cli = nil;
	IGameCli *	internal = nil;
	if (msg.result == kGameJoinSuccess) {
		if (nil == (internal = s_games.Find(msg.newGameId)))
			cli = CreateGameCli(msg.gameTypeId, msg.newGameId, state->receiver);
		else
			cli = internal->gameCli;
	}

	DEL(state);
}

//============================================================================
void IGameMgr::RecvInviteReceived (const Srv2Cli_GameMgr_InviteReceived & msg, void * param) {
	ref(param);

	pfGameMgrMsg * gameMgrMsg = NEWZERO(pfGameMgrMsg);
	gameMgrMsg->Set(msg);
	for (unsigned i = 0; i < s_receivers.Count(); ++i)
		gameMgrMsg->AddReceiver(s_receivers[i]);
	gameMgrMsg->Send();
}

//============================================================================
void IGameMgr::RecvInviteRevoked (const Srv2Cli_GameMgr_InviteRevoked & msg, void * param) {
	ref(param);

	pfGameMgrMsg * gameMgrMsg = NEWZERO(pfGameMgrMsg);
	gameMgrMsg->Set(msg);
	for (unsigned i = 0; i < s_receivers.Count(); ++i)
		gameMgrMsg->AddReceiver(s_receivers[i]);
	gameMgrMsg->Send();
}

//============================================================================
void IGameMgr::Recv (GameMsgHeader * msg) {

	// Look for transaction state associated with this message	
	void * param;
	if (TransState * trans = s_trans.Find(msg->transId)) {
		param		= trans->param;
		DEL(trans);
	}
	else {
		param		= nil;
	}

	// If the message has a receiver gameId specified, then
	// hand it off to that game client for dispatch.
	if (unsigned gameId = msg->recvGameId) {
		if (IGameCli * node = s_games.Find(gameId)) {
			node->Recv(msg, param);
		}
		return;
	}
	
	// The message was meant for the game manager (that's us); dispatch it.
	#define DISPATCH(a) case kSrv2Cli_GameMgr_##a: {						\
		const Srv2Cli_GameMgr_##a & m = *(const Srv2Cli_GameMgr_##a *)msg;	\
		Recv##a(m, param);													\
	}																		\
	break;																	//
	switch (msg->messageId) {
		DISPATCH(GameInstance);
		DISPATCH(InviteReceived);
		DISPATCH(InviteRevoked);
		DEFAULT_FATAL(msg->messageId);
	}
	#undef DISPATCH
}

//============================================================================
void IGameMgr::StaticRecv (GameMsgHeader * msg) {

	pfGameMgr::GetInstance()->internal->Recv(msg);
}


/*****************************************************************************
*
*	pfGameMgrMsg
*
***/

//============================================================================
pfGameMgrMsg::pfGameMgrMsg () {

	netMsg = nil;
}

//============================================================================
pfGameMgrMsg::~pfGameMgrMsg () {

	FREE(netMsg);
}

//============================================================================
void pfGameMgrMsg::Set (const GameMsgHeader & msg) {

	netMsg = (GameMsgHeader *)ALLOC(msg.messageBytes);
	MemCopy(netMsg, &msg, msg.messageBytes);
}


/*****************************************************************************
*
*	pfGameCliMsg
*
***/

//============================================================================
pfGameCliMsg::pfGameCliMsg () {

	gameCli	= nil;
	netMsg	= nil;
}

//============================================================================
pfGameCliMsg::~pfGameCliMsg () {

	FREE(netMsg);
}

//============================================================================
void pfGameCliMsg::Set (pfGameCli * cli, const GameMsgHeader & msg) {

	netMsg = (GameMsgHeader *)ALLOC(msg.messageBytes);
	MemCopy(netMsg, &msg, msg.messageBytes);
	gameCli		= cli;
}

/*****************************************************************************
*
*   pfGameMgr
*
***/

//============================================================================
pfGameMgr::pfGameMgr () {
}

//============================================================================
pfGameMgr * pfGameMgr::GetInstance () {

	static pfGameMgr s_instance;
	return &s_instance;
}

//============================================================================
void pfGameMgr::GetGameIds (ARRAY(unsigned) * arr) const {

	for (IGameCli * node = s_games.Head(); node; node = s_games.Next(node))
		arr->Add(node->GetValue());
}

//============================================================================
pfGameCli * pfGameMgr::GetGameCli (unsigned gameId) const {

	if (IGameCli * node = s_games.Find(gameId))
		return node->gameCli;
	return nil;
}

//============================================================================
const wchar * pfGameMgr::GetGameNameByTypeId (const Uuid & gameTypeId) const {

	if (Factory * factory = s_factories.Find(gameTypeId))
		return factory->reg.name;
	return nil;
}

//============================================================================
void pfGameMgr::RemoveReceiver (plKey receiver) {

	for (unsigned i = 0; i < s_receivers.Count(); ++i) {
		if (s_receivers[i] == receiver) {
			s_receivers.DeleteUnordered(i);
			break;
		}
	}
}

//============================================================================
void pfGameMgr::AddReceiver (plKey receiver) {

	RemoveReceiver(receiver);
	s_receivers.Add(receiver);
}

//============================================================================
void pfGameMgr::JoinGame (
	plKey			receiver,
	unsigned		gameId
) {
	Cli2Srv_GameMgr_JoinGame msg;
	ZERO(msg);
	
	msg.messageId		= kCli2Srv_GameMgr_JoinGame;
	msg.recvGameId		= 0;			// send to GameMgr on server
	msg.newGameId		= gameId;		// the GameSrv we wish to join

	// Don't send "common game" message fields	
	unsigned msgBytes
		= sizeof(msg)
		- sizeof(msg.gameTypeId)
		- sizeof(msg.createDataBytes)
		- sizeof(msg.createData);
		
	msg.messageBytes	= msgBytes;

	GameMgrSend(&msg, NEWZERO(JoinTransState)(receiver));
}

//============================================================================
void pfGameMgr::CreateGame (
	plKey			receiver,
	const Uuid &	gameTypeId,
	unsigned		createOptions,
	unsigned		initBytes,
	const void *	initData
) {
	Cli2Srv_GameMgr_CreateGame * msg;

	unsigned msgBytes
		= sizeof(*msg)
		- sizeof(msg->createData)
		+ initBytes;
		
	msg = (Cli2Srv_GameMgr_CreateGame *)_alloca(msgBytes);
		
	msg->messageId			= kCli2Srv_GameMgr_CreateGame;
	msg->recvGameId			= 0;			// send to GameMgr on server
	msg->gameTypeId			= gameTypeId;	// The type of game we wish to create
	msg->createOptions		= createOptions;
	msg->messageBytes		= msgBytes;
	msg->createDataBytes	= initBytes;
	MemCopy(msg->createData, initData, initBytes);

	GameMgrSend(msg, NEWZERO(JoinTransState)(receiver));
}

//============================================================================
void pfGameMgr::JoinCommonGame (
	plKey			receiver,
	const Uuid &	gameTypeId,
	unsigned		gameNumber,
	unsigned		initBytes,
	const void *	initData
) {
	Cli2Srv_GameMgr_JoinGame * msg;

	unsigned msgBytes
		= sizeof(*msg)
		- sizeof(msg->createData)
		+ initBytes;
		
	msg = (Cli2Srv_GameMgr_JoinGame *)_alloca(msgBytes);
		
	msg->messageId			= kCli2Srv_GameMgr_JoinGame;
	msg->recvGameId			= 0;			// send to GameMgr on server
	msg->gameTypeId			= gameTypeId;	// the type of common game we with to join
	msg->newGameId			= gameNumber;	// the "table number" of th common game we wish to join
	msg->createOptions		= kGameJoinCommon;
	msg->messageBytes		= msgBytes;
	msg->createDataBytes	= initBytes;
	MemCopy(msg->createData, initData, initBytes);

	GameMgrSend(msg, NEWZERO(JoinTransState)(receiver));
}


/*****************************************************************************
*
*   pfGameCli
*
***/

//============================================================================
pfGameCli::pfGameCli (
	unsigned	gameId,
	plKey		receiver
) {
	internal = NEWZERO(IGameCli)(this, gameId, receiver);
}

//============================================================================
pfGameCli::~pfGameCli () {

	DEL(internal);
}

//============================================================================
unsigned pfGameCli::GetGameId () const {

	return internal->GetValue();
}

//============================================================================
const Uuid & pfGameCli::GetGameTypeId () const {

	return internal->factory->GetValue();
}

//============================================================================
const wchar * pfGameCli::GetName () const {

	return internal->factory->reg.name;
}

//============================================================================
plKey pfGameCli::GetReceiver () const {

	return internal->receiver;
}

//============================================================================
unsigned pfGameCli::GetPlayerCount () const {

	return internal->playerCount;
}

//============================================================================
void pfGameCli::InvitePlayer (unsigned playerId) {

	Cli2Srv_Game_Invite msg;
	msg.messageId		= kCli2Srv_Game_Invite;
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.playerId		= playerId;
	msg.messageBytes	= sizeof(msg);

	GameMgrSend(&msg);
}

//============================================================================
void pfGameCli::UninvitePlayer (unsigned playerId) {

	Cli2Srv_Game_Uninvite msg;
	msg.messageId		= kCli2Srv_Game_Uninvite;
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.playerId		= playerId;
	msg.messageBytes	= sizeof(msg);

	GameMgrSend(&msg);
}

//============================================================================
void pfGameCli::LeaveGame () {
	
	Cli2Srv_Game_LeaveGame msg;
	msg.messageId		= kCli2Srv_Game_LeaveGame;
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.messageBytes	= sizeof(msg);

	GameMgrSend(&msg);
}


/*****************************************************************************
*
*   IGameCli
*
***/

//============================================================================
IGameCli::IGameCli (
	pfGameCli *	gameCli,
	unsigned	gameId,
	plKey		receiver
) :	THashKeyVal<unsigned>(gameId)
,	gameCli(gameCli)
,	receiver(receiver)
{
	s_games.Add(this);
}

//============================================================================
void IGameCli::Recv (GameMsgHeader * msg, void * param) {

	#define DISPATCH(a)	case kSrv2Cli_Game_##a: {						\
		const Srv2Cli_Game_##a & m = *(const Srv2Cli_Game_##a *)msg;	\
		Recv##a(m, param);												\
	}																	\
	break;
	switch (msg->messageId) {
		DISPATCH(PlayerJoined);
		DISPATCH(PlayerLeft);
		DISPATCH(InviteFailed);
		DISPATCH(OwnerChange);
		default:
			gameCli->Recv(msg, param);
	}
	#undef DISPATCH
}

//============================================================================
void IGameCli::RecvPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg, void * param) {
	ref(param);

	++playerCount;
	gameCli->OnPlayerJoined(msg);
}

//============================================================================
void IGameCli::RecvPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg, void * param) {
	ref(param);

	--playerCount;
	gameCli->OnPlayerLeft(msg);
}

//============================================================================
void IGameCli::RecvInviteFailed (const Srv2Cli_Game_InviteFailed & msg, void * param) {
	ref(param);

	gameCli->OnInviteFailed(msg);
}

//============================================================================
void IGameCli::RecvOwnerChange (const Srv2Cli_Game_OwnerChange & msg, void * param) {
	ref(param);

	gameCli->OnOwnerChange(msg);
}


/*****************************************************************************
*
*   Factory
*
***/

//============================================================================
Factory::Factory (const GameTypeReg & reg)
:	reg(reg)
,	THashKeyVal<Uuid>(reg.typeId)
{
	s_factories.Add(this);
}


/*****************************************************************************
*
*   TransState
*
***/

//============================================================================
TransState::TransState (unsigned transId, void * param)
:	THashKeyVal<unsigned>(transId)
,	param(param)
{
	s_trans.Add(this);
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void GameMgrRegisterGameType (const GameTypeReg & reg) {

	(void)NEWZERO(Factory)(reg);
}

//============================================================================
void GameMgrSend (GameMsgHeader * msg, void * param) {

	if (param) {
		msg->transId = INextTransId();
		(void)NEW(TransState)(msg->transId, param);
	}
	else {
		msg->transId = 0;
	}

	NetCliGameSendGameMgrMsg(msg);
}
