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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/VarSync/pfGmVarSync.cpp
*   
***/

#define USES_GAME_VARSYNC
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct IVarSync {
	pfGmVarSync *	gameCli;
	
	IVarSync (pfGmVarSync *	gameCli);

	// pfGameCli event notification handlers
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);

	// VarSync network message handlers	
	void RecvStringVarChanged	(const Srv2Cli_VarSync_StringVarChanged & msg, void * param);
	void RecvNumericVarChanged	(const Srv2Cli_VarSync_NumericVarChanged & msg, void * param);
	void RecvAllVarsSent		(const Srv2Cli_VarSync_AllVarsSent & msg, void * param);
	void RecvStringVarCreated	(const Srv2Cli_VarSync_StringVarCreated & msg, void * param);
	void RecvNumericVarCreated	(const Srv2Cli_VarSync_NumericVarCreated & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * VarSyncFactory (
	unsigned	gameId,
	plKey		receiver
) {
	return NEWZERO(pfGmVarSync)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterVarSyncFactory) {

	static GameTypeReg reg = {
		VarSyncFactory,
		kGameTypeId_VarSync,
		L"VarSync"
	};

	GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   IVarSync
*
***/

//============================================================================
IVarSync::IVarSync (pfGmVarSync * gameCli)
:	gameCli(gameCli)
{
}

//============================================================================
void IVarSync::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
	
	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::RecvStringVarChanged (const Srv2Cli_VarSync_StringVarChanged & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::RecvNumericVarChanged (const Srv2Cli_VarSync_NumericVarChanged & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::RecvAllVarsSent (const Srv2Cli_VarSync_AllVarsSent & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::RecvStringVarCreated (const Srv2Cli_VarSync_StringVarCreated & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void IVarSync::RecvNumericVarCreated (const Srv2Cli_VarSync_NumericVarCreated & msg, void * param) {
	ref(param);

	pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
	gameCliMsg->Set(gameCli, msg);
	gameCliMsg->Send(gameCli->GetReceiver());
}				    

/*****************************************************************************
*
*   pfGmVarSync
*
***/

//============================================================================
pfGmVarSync::pfGmVarSync (
	unsigned	gameId,
	plKey		receiver
)
:	pfGameCli(gameId, receiver)
{
	internal = NEWZERO(IVarSync)(this);
}

//============================================================================
pfGmVarSync::~pfGmVarSync () {

	DEL(internal);
}

//============================================================================
void pfGmVarSync::Recv (GameMsgHeader * msg, void * param) {

	#define DISPATCH(a) case kSrv2Cli_VarSync_##a: {						\
		const Srv2Cli_VarSync_##a & m = *(const Srv2Cli_VarSync_##a *)msg;	\
		internal->Recv##a(m, param);										\
	}																		\
	break;																	//
	switch (msg->messageId) {
		DISPATCH(StringVarChanged);
		DISPATCH(NumericVarChanged);
		DISPATCH(AllVarsSent);
		DISPATCH(StringVarCreated);
		DISPATCH(NumericVarCreated);
		DEFAULT_FATAL(msg->messageId);
	}
	#undef DISPATCH
}

//============================================================================
void pfGmVarSync::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

	internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmVarSync::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

	internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmVarSync::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {

	internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmVarSync::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

	internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmVarSync::SetStringVar (unsigned long id, const wchar* val) {

	Cli2Srv_VarSync_SetStringVar msg;
	msg.messageId		= kCli2Srv_VarSync_SetStringVar;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.varID			= id;
	StrCopy(msg.varValue, val, arrsize(msg.varValue));
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmVarSync::SetNumericVar (unsigned long id, double val) {

	Cli2Srv_VarSync_SetNumericVar msg;
	msg.messageId		= kCli2Srv_VarSync_SetNumericVar;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	msg.varID			= id;
	msg.varValue		= val;

	GameMgrSend(&msg);
}

//============================================================================
void pfGmVarSync::RequestAllVars () {

	Cli2Srv_VarSync_RequestAllVars msg;
	msg.messageId		= kCli2Srv_VarSync_RequestAllVars;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	
	GameMgrSend(&msg);
}

//============================================================================
void pfGmVarSync::CreateStringVar (const wchar* name, const wchar* val) {

	Cli2Srv_VarSync_CreateStringVar msg;
	msg.messageId		= kCli2Srv_VarSync_CreateStringVar;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	StrCopy(msg.varName, name, arrsize(msg.varName));
	StrCopy(msg.varValue, val, arrsize(msg.varValue));

	GameMgrSend(&msg);
}

//============================================================================
void pfGmVarSync::CreateNumericVar (const wchar* name, double val) {

	Cli2Srv_VarSync_CreateNumericVar msg;
	msg.messageId		= kCli2Srv_VarSync_CreateNumericVar;
	msg.messageBytes	= sizeof(msg);
	msg.recvGameId		= GetGameId();	// send to GameSrv on server
	msg.transId			= 0;
	StrCopy(msg.varName, name, arrsize(msg.varName));
	msg.varValue		= val;

	GameMgrSend(&msg);
}
