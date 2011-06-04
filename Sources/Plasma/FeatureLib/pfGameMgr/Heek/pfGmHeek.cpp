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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/TicTacToe/pfGmTicTacToe.cpp
*   
***/

#define USES_GAME_HEEK
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct IHeek {
	pfGmHeek *		gameCli;

	IHeek (pfGmHeek * gameCli);

	// pfGameCli event notification handlers
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);

	// Heek network message handlers	
	void RecvPlayGame		(const Srv2Cli_Heek_PlayGame & msg, void * param);
	void RecvGoodbye		(const Srv2Cli_Heek_Goodbye & msg, void * param);
	void RecvWelcome		(const Srv2Cli_Heek_Welcome & msg, void * param);
	void RecvDrop			(const Srv2Cli_Heek_Drop & msg, void * param);
	void RecvSetup			(const Srv2Cli_Heek_Setup & msg, void * param);
	void RecvLightState		(const Srv2Cli_Heek_LightState & msg, void * param);
	void RecvInterfaceState	(const Srv2Cli_Heek_InterfaceState & msg, void * param);
	void RecvCountdownState	(const Srv2Cli_Heek_CountdownState & msg, void * param);
	void RecvWinLose		(const Srv2Cli_Heek_WinLose & msg, void * param);
	void RecvGameWin		(const Srv2Cli_Heek_GameWin & msg, void * param);
	void RecvPointUpdate	(const Srv2Cli_Heek_PointUpdate & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * HeekFactory (
unsigned	gameId,
plKey		receiver
) {
	return NEWZERO(pfGmHeek)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterHeek) {
	static GameTypeReg reg = {
		HeekFactory,
		kGameTypeId_Heek,
		L"Heek"
	};

	GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   IHeek
*
***/

//============================================================================
IHeek::IHeek (pfGmHeek * gameCli)
:	gameCli(gameCli)
{
}

//============================================================================
void IHeek::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvPlayGame (const Srv2Cli_Heek_PlayGame & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvGoodbye (const Srv2Cli_Heek_Goodbye & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvWelcome (const Srv2Cli_Heek_Welcome & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvDrop (const Srv2Cli_Heek_Drop & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvSetup (const Srv2Cli_Heek_Setup & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvLightState (const Srv2Cli_Heek_LightState & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvInterfaceState (const Srv2Cli_Heek_InterfaceState & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvCountdownState (const Srv2Cli_Heek_CountdownState & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvWinLose (const Srv2Cli_Heek_WinLose & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvGameWin (const Srv2Cli_Heek_GameWin & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IHeek::RecvPointUpdate (const Srv2Cli_Heek_PointUpdate & msg, void * param) {
	ref(param);
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}


/*****************************************************************************
*
*   pfGmHeek
*
***/

//============================================================================
pfGmHeek::pfGmHeek (
	unsigned	gameId,
	plKey		receiver
)
:	pfGameCli(gameId, receiver)
{
	internal = NEWZERO(IHeek)(this);
}

//============================================================================
pfGmHeek::~pfGmHeek () {
	DEL(internal);
}

//============================================================================
void pfGmHeek::Recv (GameMsgHeader * msg, void * param) {
#define DISPATCH(a) case kSrv2Cli_Heek_##a: {										\
						const Srv2Cli_Heek_##a & m = *(const Srv2Cli_Heek_##a *)msg;	\
						internal->Recv##a(m, param);								\
					}																\
					break;

	switch (msg->messageId) {
		DISPATCH(PlayGame);
		DISPATCH(Goodbye);
		DISPATCH(Welcome);
		DISPATCH(Drop);
		DISPATCH(Setup);
		DISPATCH(LightState);
		DISPATCH(InterfaceState);
		DISPATCH(CountdownState);
		DISPATCH(WinLose);
		DISPATCH(GameWin);
		DISPATCH(PointUpdate);
		DEFAULT_FATAL(msg->messageId);
	}
#undef DISPATCH
}

//============================================================================
void pfGmHeek::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {
	internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmHeek::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {
	internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmHeek::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmHeek::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {
	internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmHeek::PlayGame (unsigned position, dword points, const wchar name[]) {
	Cli2Srv_Heek_PlayGame msg;
	msg.messageId		= kCli2Srv_Heek_PlayGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	msg.position		= (byte)position;
	msg.points			= points;
	StrCopy(msg.name, name, arrsize(msg.name));

	GameMgrSend(&msg);
}

//============================================================================
void pfGmHeek::LeaveGame () {
	Cli2Srv_Heek_LeaveGame msg;
	msg.messageId		= kCli2Srv_Heek_LeaveGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmHeek::Choose (EHeekChoice choice) {
	Cli2Srv_Heek_Choose msg;
	msg.messageId		= kCli2Srv_Heek_Choose;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	msg.choice = (byte)choice;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmHeek::SequenceFinished (EHeekSeqFinished seq) {
	Cli2Srv_Heek_SeqFinished msg;
	msg.messageId		= kCli2Srv_Heek_SeqFinished;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	msg.seqFinished = (byte)seq;

	GameMgrSend(&msg);
}