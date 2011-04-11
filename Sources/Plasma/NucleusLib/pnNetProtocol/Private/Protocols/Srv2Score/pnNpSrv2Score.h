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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Score/pnNpSrv2Score.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2SCORE_PNNPSRV2SCORE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Score/pnNpSrv2Score.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2SCORE_PNNPSRV2SCORE_H


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>

// kNetProtocolSrv2Score messages
enum {
	kSrv2Score_ScoreCreate						= 0,
	kSrv2Score_ScoreGetScores					= 1,
	kSrv2Score_ScoreAddPoints					= 2,
	kSrv2Score_ScoreTransferPoints				= 3,
	kSrv2Score_ScoreSetPoints					= 4,
	kSrv2Score_ScoreDelete						= 5,
	kSrv2Score_ScoreGetRanks					= 6,
};

enum {
	kScore2Srv_ScoreCreateReply					= 0,
	kScore2Srv_ScoreGetScoresReply				= 1,
	kScore2Srv_ScoreDeleteReply					= 2,
	kScore2Srv_ScoreGetRanksReply				= 3,
};


/*****************************************************************************
*
*   Srv2Score connect packet
*
***/

struct Srv2Score_ConnData {
    dword   dataBytes;
    dword   buildId;
    dword   srvType;
};
struct Srv2Score_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2Score_ConnData          data;
};


/*****************************************************************************
*
*   Srv2Score message structures
*
***/

struct Srv2Score_ScoreCreate : SrvMsgHeader {
	dword	ownerId;
	wchar	gameName[kMaxGameScoreNameLength];
	dword	gameType;
	dword	value;
};

struct Srv2Score_ScoreDelete : SrvMsgHeader {
	dword	scoreId;
};

struct Srv2Score_ScoreGetScores : SrvMsgHeader {
	dword	ownerId;
	wchar	gameName[kMaxGameScoreNameLength];
};

struct Srv2Score_ScoreAddPoints : SrvMsgHeader {
	dword	scoreId;
	dword	numPoints;
};

struct Srv2Score_ScoreTransferPoints : SrvMsgHeader {
	dword	srcScoreId;
	dword	destScoreId;
	dword	numPoints;
};

struct Srv2Score_ScoreSetPoints : SrvMsgHeader {
	dword	scoreId;
	dword	numPoints;
};

struct Srv2Score_ScoreGetRanks : SrvMsgHeader {
	dword ownerId;
	dword scoreGroup;
	dword parentFolderId;
	wchar gameName[kMaxGameScoreNameLength];
	dword timePeriod;
	dword numResults;
	dword pageNumber;
	dword sortDesc;
};


/*****************************************************************************
*
*   Score2Srv message structures
*
***/

struct Score2Srv_ScoreCreateReply : SrvMsgHeader { 
	dword	scoreId;
	dword	createdTime;
};

struct Score2Srv_ScoreGetScoresReply : SrvMsgHeader {
	dword	scoreCount;
	dword	byteCount;
	byte	buffer[1];
};

struct Score2Srv_ScoreGetRanksReply : SrvMsgHeader {
	dword	rankCount;
	dword	byteCount;
	byte	buffer[1];
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


/*****************************************************************************
*
*   Srv2Score functions
*
***/

bool Srv2ScoreValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Score_ConnData *        connectPtr
);

