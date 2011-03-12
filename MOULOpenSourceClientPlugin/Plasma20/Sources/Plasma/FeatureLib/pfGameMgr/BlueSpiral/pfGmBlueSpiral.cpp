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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/BlueSpiral/pfGmBlueSpiral.cpp
*   
***/

#define USES_GAME_BLUESPIRAL
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct IBlueSpiral {
	pfGmBlueSpiral * gameCli;
	
	IBlueSpiral (pfGmBlueSpiral * gameCli);

	// pfGameCli event notification handlers
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);

	// BlueSpiral network message handlers	
	void RecvClothOrder		(const Srv2Cli_BlueSpiral_ClothOrder & msg, void * param);
	void RecvSuccessfulHit	(const Srv2Cli_BlueSpiral_SuccessfulHit & msg, void * param);
	void RecvGameWon		(const Srv2Cli_BlueSpiral_GameWon & msg, void * param);
	void RecvGameOver		(const Srv2Cli_BlueSpiral_GameOver & msg, void * param);
	void RecvGameStarted	(const Srv2Cli_BlueSpiral_GameStarted & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * BlueSpiralFactory (
	unsigned	gameId,
	plKey		receiver
) {
	return NEWZERO(pfGmBlueSpiral)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterBlueSpiralFactory) {

	static GameTypeReg reg = {
		BlueSpiralFactory,
		kGameTypeId_BlueSpiral,
		L"BlueSpiral"
	};

	GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   IBlueSpiral
*
***/

//============================================================================
IBlueSpiral::IBlueSpiral (pfGmBlueSpiral * gameCli)
:	gameCli(gameCli)
{
}

//============================================================================
void IBlueSpiral::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::RecvClothOrder (const Srv2Cli_BlueSpiral_ClothOrder & msg, void * param) {
	ref(param);
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::RecvSuccessfulHit (const Srv2Cli_BlueSpiral_SuccessfulHit & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::RecvGameWon (const Srv2Cli_BlueSpiral_GameWon & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::RecvGameOver (const Srv2Cli_BlueSpiral_GameOver & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IBlueSpiral::RecvGameStarted (const Srv2Cli_BlueSpiral_GameStarted & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}
															    

/*****************************************************************************
*
*   pfGmBlueSpiral
*
***/

//============================================================================
pfGmBlueSpiral::pfGmBlueSpiral (
	unsigned	gameId,
	plKey		receiver
)
:	pfGameCli(gameId, receiver)
{
	internal = NEWZERO(IBlueSpiral)(this);
}

//============================================================================
pfGmBlueSpiral::~pfGmBlueSpiral () {

	DEL(internal);
}

//============================================================================
void pfGmBlueSpiral::Recv (GameMsgHeader * msg, void * param) {

	#define DISPATCH(a) case kSrv2Cli_BlueSpiral_##a: {								\
		const Srv2Cli_BlueSpiral_##a & m = *(const Srv2Cli_BlueSpiral_##a *)msg;	\
		internal->Recv##a(m, param);												\
	}																				\
	break;																			//
	switch (msg->messageId) {
		DISPATCH(ClothOrder);
		DISPATCH(SuccessfulHit);
		DISPATCH(GameWon);
		DISPATCH(GameOver);
		DISPATCH(GameStarted);
		DEFAULT_FATAL(msg->messageId);
	}
	#undef DISPATCH
}

//============================================================================
void pfGmBlueSpiral::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmBlueSpiral::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmBlueSpiral::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {

	internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmBlueSpiral::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmBlueSpiral::StartGame () {

	Cli2Srv_BlueSpiral_StartGame msg;
	msg.messageId		= kCli2Srv_BlueSpiral_StartGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmBlueSpiral::HitCloth (int clothNum) {

	Cli2Srv_BlueSpiral_HitCloth msg;
	msg.messageId		= kCli2Srv_BlueSpiral_HitCloth;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.clothNum		= clothNum;

	GameMgrSend(&msg);
}
