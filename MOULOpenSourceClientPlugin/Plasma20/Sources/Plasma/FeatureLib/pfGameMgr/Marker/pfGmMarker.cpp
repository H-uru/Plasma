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

#define USES_GAME_MARKER
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct IMarker {
	pfGmMarker *	gameCli;
	
	IMarker (pfGmMarker *	gameCli);

	// pfGameCli event notification handlers
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);

	// Marker network message handlers
	void RecvTemplateCreated	(const Srv2Cli_Marker_TemplateCreated & msg, void * param);
	void RecvTeamAssigned		(const Srv2Cli_Marker_TeamAssigned & msg, void * param);
	void RecvGameType			(const Srv2Cli_Marker_GameType & msg, void * param);
	void RecvGameStarted		(const Srv2Cli_Marker_GameStarted & msg, void * param);
	void RecvGamePaused			(const Srv2Cli_Marker_GamePaused & msg, void * param);
	void RecvGameReset			(const Srv2Cli_Marker_GameReset & msg, void * param);
	void RecvGameOver			(const Srv2Cli_Marker_GameOver & msg, void * param);
	void RecvGameNameChanged	(const Srv2Cli_Marker_GameNameChanged & msg, void * param);
	void RecvTimeLimitChanged	(const Srv2Cli_Marker_TimeLimitChanged & msg, void * param);
	void RecvGameDeleted		(const Srv2Cli_Marker_GameDeleted & msg, void * param);
	void RecvMarkerAdded		(const Srv2Cli_Marker_MarkerAdded & msg, void * param);
	void RecvMarkerDeleted		(const Srv2Cli_Marker_MarkerDeleted & msg, void * param);
	void RecvMarkerNameChanged	(const Srv2Cli_Marker_MarkerNameChanged & msg, void * param);
	void RecvMarkerCaptured		(const Srv2Cli_Marker_MarkerCaptured & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * MarkerFactory (
	unsigned	gameId,
	plKey		receiver
) {
	return NEWZERO(pfGmMarker)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterMarkerFactory) {

	static GameTypeReg reg = {
		MarkerFactory,
		kGameTypeId_Marker,
		L"Marker"
	};

	GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   IMarker
*
***/

//============================================================================
IMarker::IMarker (pfGmMarker * gameCli)
:	gameCli(gameCli)
{
}

//============================================================================
void IMarker::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvTemplateCreated (const Srv2Cli_Marker_TemplateCreated & msg, void * param) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvTeamAssigned (const Srv2Cli_Marker_TeamAssigned & msg, void * param) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameType (const Srv2Cli_Marker_GameType & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameStarted (const Srv2Cli_Marker_GameStarted & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGamePaused (const Srv2Cli_Marker_GamePaused & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameReset (const Srv2Cli_Marker_GameReset & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameOver (const Srv2Cli_Marker_GameOver & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameNameChanged (const Srv2Cli_Marker_GameNameChanged & msg, void * param) {
	ref(param);
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvTimeLimitChanged (const Srv2Cli_Marker_TimeLimitChanged & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvGameDeleted (const Srv2Cli_Marker_GameDeleted & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());

	if (!msg.failed)
		DEL(gameCli);	// we're done
}

//============================================================================
void IMarker::RecvMarkerAdded (const Srv2Cli_Marker_MarkerAdded & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvMarkerDeleted (const Srv2Cli_Marker_MarkerDeleted & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvMarkerNameChanged (const Srv2Cli_Marker_MarkerNameChanged & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IMarker::RecvMarkerCaptured (const Srv2Cli_Marker_MarkerCaptured & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}
												    

/*****************************************************************************
*
*   pfGmMarker
*
***/

//============================================================================
pfGmMarker::pfGmMarker (
	unsigned	gameId,
	plKey		receiver
)
:	pfGameCli(gameId, receiver)
{
	internal = NEWZERO(IMarker)(this);
}

//============================================================================
pfGmMarker::~pfGmMarker () {

	DEL(internal);
}

//============================================================================
void pfGmMarker::Recv (GameMsgHeader * msg, void * param) {

	#define DISPATCH(a) case kSrv2Cli_Marker_##a: {							\
		const Srv2Cli_Marker_##a & m = *(const Srv2Cli_Marker_##a *)msg;	\
		internal->Recv##a(m, param);										\
	}																		\
	break;																	//
	switch (msg->messageId) {
		DISPATCH(TemplateCreated);
		DISPATCH(TeamAssigned);
		DISPATCH(GameType);
		DISPATCH(GameStarted);
		DISPATCH(GamePaused);
		DISPATCH(GameReset);
		DISPATCH(GameOver);
		DISPATCH(GameNameChanged);
		DISPATCH(TimeLimitChanged);
		DISPATCH(GameDeleted);
		DISPATCH(MarkerAdded);
		DISPATCH(MarkerDeleted);
		DISPATCH(MarkerNameChanged);
		DISPATCH(MarkerCaptured);
		DEFAULT_FATAL(msg->messageId);
	}
	#undef DISPATCH
}

//============================================================================
void pfGmMarker::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmMarker::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmMarker::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {

	internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmMarker::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmMarker::StartGame () {

	Cli2Srv_Marker_StartGame msg;
	msg.messageId		= kCli2Srv_Marker_StartGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::PauseGame () {

	Cli2Srv_Marker_StartGame msg;
	msg.messageId		= kCli2Srv_Marker_PauseGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::ResetGame () {

	Cli2Srv_Marker_StartGame msg;
	msg.messageId		= kCli2Srv_Marker_ResetGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::ChangeGameName (const wchar name[]) {

	Cli2Srv_Marker_ChangeGameName msg;
	msg.messageId		= kCli2Srv_Marker_ChangeGameName;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	StrCopy(msg.gameName, name, arrsize(msg.gameName));
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::ChangeTimeLimit (unsigned long timeLimit) {

	Cli2Srv_Marker_ChangeTimeLimit msg;
	msg.messageId		= kCli2Srv_Marker_ChangeTimeLimit;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.timeLimit = timeLimit;
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::DeleteGame () {

	Cli2Srv_Marker_DeleteGame msg;
	msg.messageId		= kCli2Srv_Marker_DeleteGame;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::AddMarker (double x, double y, double z, const wchar name[], const wchar age[]) {

	Cli2Srv_Marker_AddMarker msg;
	msg.messageId		= kCli2Srv_Marker_AddMarker;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.x = x;
	msg.y = y;
	msg.z = z;
	StrCopy(msg.name, name, arrsize(msg.name));
	StrCopy(msg.age, age, arrsize(msg.age));

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::DeleteMarker (unsigned long markerID) {

	Cli2Srv_Marker_DeleteMarker msg;
	msg.messageId		= kCli2Srv_Marker_DeleteMarker;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.markerID = markerID;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::ChangeMarkerName (unsigned long markerID, const wchar name[]) {

	Cli2Srv_Marker_ChangeMarkerName msg;
	msg.messageId		= kCli2Srv_Marker_ChangeMarkerName;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.markerID = markerID;
	StrCopy(msg.markerName, name, arrsize(msg.markerName));

	GameMgrSend(&msg);
}

//============================================================================
void pfGmMarker::CaptureMarker (unsigned long markerID) {

	Cli2Srv_Marker_CaptureMarker msg;
	msg.messageId		= kCli2Srv_Marker_CaptureMarker;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.markerID = markerID;
	
	GameMgrSend(&msg);
}