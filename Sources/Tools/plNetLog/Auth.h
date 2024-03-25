/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include "plNetLog.h"

enum
{
    // Global
    kCli2Auth_PingRequest = 0,

    // Client
    kCli2Auth_ClientRegisterRequest,
    kCli2Auth_ClientSetCCRLevel,

    // Account
    kCli2Auth_AcctLoginRequest,
    kCli2Auth_AcctSetEulaVersion,
    kCli2Auth_AcctSetDataRequest,
    kCli2Auth_AcctSetPlayerRequest,
    kCli2Auth_AcctCreateRequest,
    kCli2Auth_AcctChangePasswordRequest,
    kCli2Auth_AcctSetRolesRequest,
    kCli2Auth_AcctSetBillingTypeRequest,
    kCli2Auth_AcctActivateRequest,
    kCli2Auth_AcctCreateFromKeyRequest,

    // Player
    kCli2Auth_PlayerDeleteRequest,
    kCli2Auth_PlayerUndeleteRequest,
    kCli2Auth_PlayerSelectRequest,
    kCli2Auth_PlayerRenameRequest,
    kCli2Auth_PlayerCreateRequest,
    kCli2Auth_PlayerSetStatus,
    kCli2Auth_PlayerChat,
    kCli2Auth_UpgradeVisitorRequest,
    kCli2Auth_SetPlayerBanStatusRequest,
    kCli2Auth_KickPlayer,
    kCli2Auth_ChangePlayerNameRequest,
    kCli2Auth_SendFriendInviteRequest,

    // Vault
    kCli2Auth_VaultNodeCreate,
    kCli2Auth_VaultNodeFetch,
    kCli2Auth_VaultNodeSave,
    kCli2Auth_VaultNodeDelete,
    kCli2Auth_VaultNodeAdd,
    kCli2Auth_VaultNodeRemove,
    kCli2Auth_VaultFetchNodeRefs,
    kCli2Auth_VaultInitAgeRequest,
    kCli2Auth_VaultNodeFind,
    kCli2Auth_VaultSetSeen,
    kCli2Auth_VaultSendNode,

    // Ages
    kCli2Auth_AgeRequest,

    // File-related
    kCli2Auth_FileListRequest,
    kCli2Auth_FileDownloadRequest,
    kCli2Auth_FileDownloadChunkAck,

    // Game
    kCli2Auth_PropagateBuffer,

    // Public ages
    kCli2Auth_GetPublicAgeList,
    kCli2Auth_SetAgePublic,

    // Log Messages
    kCli2Auth_LogPythonTraceback,
    kCli2Auth_LogStackDump,
    kCli2Auth_LogClientDebuggerConnect,

    // Score
    kCli2Auth_ScoreCreate,
    kCli2Auth_ScoreDelete,
    kCli2Auth_ScoreGetScores,
    kCli2Auth_ScoreAddPoints,
    kCli2Auth_ScoreTransferPoints,
    kCli2Auth_ScoreSetPoints,
    kCli2Auth_ScoreGetRanks,

    kCli2Auth_AccountExistsRequest,

    // Extended (DirtSand) messages
    kCli2Auth_AgeRequestEx = 0x1000,
    kCli2Auth_ScoreGetHighScores,

    // -------------------------------------------------------------- //

    // Global
    kAuth2Cli_PingReply = 0,
    kAuth2Cli_ServerAddr,
    kAuth2Cli_NotifyNewBuild,

    // Client
    kAuth2Cli_ClientRegisterReply,

    // Account
    kAuth2Cli_AcctLoginReply,
    kAuth2Cli_AcctData,
    kAuth2Cli_AcctPlayerInfo,
    kAuth2Cli_AcctSetPlayerReply,
    kAuth2Cli_AcctCreateReply,
    kAuth2Cli_AcctChangePasswordReply,
    kAuth2Cli_AcctSetRolesReply,
    kAuth2Cli_AcctSetBillingTypeReply,
    kAuth2Cli_AcctActivateReply,
    kAuth2Cli_AcctCreateFromKeyReply,

    // Player
    kAuth2Cli_PlayerList,
    kAuth2Cli_PlayerChat,
    kAuth2Cli_PlayerCreateReply,
    kAuth2Cli_PlayerDeleteReply,
    kAuth2Cli_UpgradeVisitorReply,
    kAuth2Cli_SetPlayerBanStatusReply,
    kAuth2Cli_ChangePlayerNameReply,
    kAuth2Cli_SendFriendInviteReply,

    // Friends
    kAuth2Cli_FriendNotify,

    // Vault
    kAuth2Cli_VaultNodeCreated,
    kAuth2Cli_VaultNodeFetched,
    kAuth2Cli_VaultNodeChanged,
    kAuth2Cli_VaultNodeDeleted,
    kAuth2Cli_VaultNodeAdded,
    kAuth2Cli_VaultNodeRemoved,
    kAuth2Cli_VaultNodeRefsFetched,
    kAuth2Cli_VaultInitAgeReply,
    kAuth2Cli_VaultNodeFindReply,
    kAuth2Cli_VaultSaveNodeReply,
    kAuth2Cli_VaultAddNodeReply,
    kAuth2Cli_VaultRemoveNodeReply,

    // Ages
    kAuth2Cli_AgeReply,

    // File-related
    kAuth2Cli_FileListReply,
    kAuth2Cli_FileDownloadChunk,

    // Game
    kAuth2Cli_PropagateBuffer,

    // Admin
    kAuth2Cli_KickedOff,

    // Public ages
    kAuth2Cli_PublicAgeList,

    // Score
    kAuth2Cli_ScoreCreateReply,
    kAuth2Cli_ScoreDeleteReply,
    kAuth2Cli_ScoreGetScoresReply,
    kAuth2Cli_ScoreAddPointsReply,
    kAuth2Cli_ScoreTransferPointsReply,
    kAuth2Cli_ScoreSetPointsReply,
    kAuth2Cli_ScoreGetRanksReply,

    kAuth2Cli_AccountExistsReply,

    // Extended (DirtSand) messages
    kAuth2Cli_AgeReplyEx = 0x1000,
    kAuth2Cli_ScoreGetHighScoresReply,
    kAuth2Cli_ServerCaps,
};

bool Auth_Factory(QTreeWidget* logger, const QString& timeFmt, int direction,
                  ChunkBuffer& buffer);
