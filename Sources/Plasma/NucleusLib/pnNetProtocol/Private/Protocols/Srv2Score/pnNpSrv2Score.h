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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
#pragma pack(push,1)

// kNetProtocolSrv2Score messages
enum {
    kSrv2Score_ScoreCreate                      = 0,
    kSrv2Score_ScoreGetScores                   = 1,
    kSrv2Score_ScoreAddPoints                   = 2,
    kSrv2Score_ScoreTransferPoints              = 3,
    kSrv2Score_ScoreSetPoints                   = 4,
    kSrv2Score_ScoreDelete                      = 5,
    kSrv2Score_ScoreGetRanks                    = 6,
};

enum {
    kScore2Srv_ScoreCreateReply                 = 0,
    kScore2Srv_ScoreGetScoresReply              = 1,
    kScore2Srv_ScoreDeleteReply                 = 2,
    kScore2Srv_ScoreGetRanksReply               = 3,
};


/*****************************************************************************
*
*   Srv2Score connect packet
*
***/

struct Srv2Score_ConnData {
    uint32_t   dataBytes;
    uint32_t   buildId;
    uint32_t   srvType;
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
    uint32_t   ownerId;
    wchar_t   gameName[kMaxGameScoreNameLength];
    uint32_t   gameType;
    uint32_t   value;
};

struct Srv2Score_ScoreDelete : SrvMsgHeader {
    uint32_t   scoreId;
};

struct Srv2Score_ScoreGetScores : SrvMsgHeader {
    uint32_t   ownerId;
    wchar_t   gameName[kMaxGameScoreNameLength];
};

struct Srv2Score_ScoreAddPoints : SrvMsgHeader {
    uint32_t   scoreId;
    uint32_t   numPoints;
};

struct Srv2Score_ScoreTransferPoints : SrvMsgHeader {
    uint32_t   srcScoreId;
    uint32_t   destScoreId;
    uint32_t   numPoints;
};

struct Srv2Score_ScoreSetPoints : SrvMsgHeader {
    uint32_t   scoreId;
    uint32_t   numPoints;
};

struct Srv2Score_ScoreGetRanks : SrvMsgHeader {
    uint32_t ownerId;
    uint32_t scoreGroup;
    uint32_t parentFolderId;
    wchar_t gameName[kMaxGameScoreNameLength];
    uint32_t timePeriod;
    uint32_t numResults;
    uint32_t pageNumber;
    uint32_t sortDesc;
};


/*****************************************************************************
*
*   Score2Srv message structures
*
***/

struct Score2Srv_ScoreCreateReply : SrvMsgHeader { 
    uint32_t   scoreId;
    uint32_t   createdTime;
};

struct Score2Srv_ScoreGetScoresReply : SrvMsgHeader {
    uint32_t   scoreCount;
    uint32_t   byteCount;
    uint8_t    buffer[1];
};

struct Score2Srv_ScoreGetRanksReply : SrvMsgHeader {
    uint32_t   rankCount;
    uint32_t   byteCount;
    uint8_t    buffer[1];
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#pragma pack(pop)


/*****************************************************************************
*
*   Srv2Score functions
*
***/

bool Srv2ScoreValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Score_ConnData *        connectPtr
);

