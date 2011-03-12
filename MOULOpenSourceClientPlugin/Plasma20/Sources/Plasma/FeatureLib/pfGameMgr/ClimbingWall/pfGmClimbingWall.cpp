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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/ClimbingWall/pfGmClimbingWall.cpp
*   
***/

#define USES_GAME_CLIMBINGWALL
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct IClimbingWall {
	pfGmClimbingWall *	gameCli;
	
	IClimbingWall (pfGmClimbingWall *	gameCli);

	// pfGameCli event notification handlers
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);

	// ClimbingWall network message handlers	
	void RecvNumBlockersChanged	(const Srv2Cli_ClimbingWall_NumBlockersChanged & msg, void * param);
	void RecvReady (const Srv2Cli_ClimbingWall_Ready & msg, void * param);
	void RecvBlockersChanged (const Srv2Cli_ClimbingWall_BlockersChanged & msg, void * param);
	void RecvPlayerEntered (const Srv2Cli_ClimbingWall_PlayerEntered & msg, void * param);
	void RecvSuitMachineLocked (const Srv2Cli_ClimbingWall_SuitMachineLocked & msg, void * param);
	void RecvGameOver (const Srv2Cli_ClimbingWall_GameOver & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * ClimbingWallFactory (
	unsigned	gameId,
	plKey		receiver
) {
	return NEWZERO(pfGmClimbingWall)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterClimbingWallFactory) {

	static GameTypeReg reg = {
		ClimbingWallFactory,
		kGameTypeId_ClimbingWall,
		L"ClimbingWall"
	};

	GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   IClimbingWall
*
***/

//============================================================================
IClimbingWall::IClimbingWall (pfGmClimbingWall * gameCli)
:	gameCli(gameCli)
{
}

//============================================================================
void IClimbingWall::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvNumBlockersChanged (const Srv2Cli_ClimbingWall_NumBlockersChanged & msg, void * param) {
	ref(param);
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvReady (const Srv2Cli_ClimbingWall_Ready & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvBlockersChanged (const Srv2Cli_ClimbingWall_BlockersChanged & msg, void * param) {
	ref (param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvPlayerEntered (const Srv2Cli_ClimbingWall_PlayerEntered & msg, void * param) {
	ref (param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvSuitMachineLocked (const Srv2Cli_ClimbingWall_SuitMachineLocked & msg, void * param) {
	ref (param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IClimbingWall::RecvGameOver (const Srv2Cli_ClimbingWall_GameOver & msg, void * param) {
	ref (param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}


/*****************************************************************************
*
*   pfGmClimbingWall
*
***/

//============================================================================
pfGmClimbingWall::pfGmClimbingWall (
	unsigned	gameId,
	plKey		receiver
)
:	pfGameCli(gameId, receiver)
{
	internal = NEWZERO(IClimbingWall)(this);
}

//============================================================================
pfGmClimbingWall::~pfGmClimbingWall () {

	DEL(internal);
}

//============================================================================
void pfGmClimbingWall::Recv (GameMsgHeader * msg, void * param) {

	#define DISPATCH(a) case kSrv2Cli_ClimbingWall_##a: {								\
		const Srv2Cli_ClimbingWall_##a & m = *(const Srv2Cli_ClimbingWall_##a *)msg;	\
		internal->Recv##a(m, param);													\
	}																					\
	break;																				//
	switch (msg->messageId) {
		DISPATCH(NumBlockersChanged);
		DISPATCH(Ready);
		DISPATCH(BlockersChanged);
		DISPATCH(PlayerEntered);
		DISPATCH(SuitMachineLocked);
		DISPATCH(GameOver);
		DEFAULT_FATAL(msg->messageId);
	}
	#undef DISPATCH
}

//============================================================================
void pfGmClimbingWall::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmClimbingWall::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmClimbingWall::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {

	internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmClimbingWall::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmClimbingWall::ChangeNumBlockers (int amountToAdjust) {

	Cli2Srv_ClimbingWall_ChangeNumBlockers msg;
	msg.messageId		= kCli2Srv_ClimbingWall_ChangeNumBlockers;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.amountToAdjust	= amountToAdjust;
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::Ready (unsigned readyType, unsigned teamNumber) {

	Cli2Srv_ClimbingWall_Ready msg;
	msg.messageId		= kCli2Srv_ClimbingWall_Ready;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.readyType		= (byte)readyType;
	msg.teamNumber		= (byte)teamNumber;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::ChangeBlocker (unsigned teamNumber, unsigned blockerNumber, bool added) {
	
	Cli2Srv_ClimbingWall_BlockerChanged msg;
	msg.messageId		= kCli2Srv_ClimbingWall_BlockerChanged;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.teamNumber		= (byte)teamNumber;
	msg.blockerNumber	= (byte)blockerNumber;
	msg.added			= added;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::Reset () {

	Cli2Srv_ClimbingWall_Reset msg;
	msg.messageId		= kCli2Srv_ClimbingWall_Reset;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::PlayerEntered (unsigned teamNumber) {

	Cli2Srv_ClimbingWall_PlayerEntered msg;
	msg.messageId		= kCli2Srv_ClimbingWall_PlayerEntered;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.teamNumber		= (byte)teamNumber;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::FinishedGame () {

	Cli2Srv_ClimbingWall_FinishedGame msg;
	msg.messageId		= kCli2Srv_ClimbingWall_FinishedGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmClimbingWall::Panic () {

	Cli2Srv_ClimbingWall_Panic msg;
	msg.messageId		= kCli2Srv_ClimbingWall_Panic;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}