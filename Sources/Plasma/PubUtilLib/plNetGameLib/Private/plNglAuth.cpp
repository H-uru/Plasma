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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglAuth.cpp
*   
***/

#include "../Pch.h"

#include <regex>
#include "pnEncryption/plChallengeHash.h"
#include "plVault/plVaultConstants.h"

namespace Ngl { namespace Auth {
/*****************************************************************************
*
*   Private
*
***/

struct CliAuConn : hsRefCnt {
    CliAuConn ();
    ~CliAuConn ();

    // Reconnection
    AsyncTimer *        reconnectTimer;
    unsigned            reconnectStartMs;
    
    // Ping
    AsyncTimer *        pingTimer;
    unsigned            pingSendTimeMs;
    unsigned            lastHeardTimeMs;

    // This function should be called during object construction
    // to initiate connection attempts to the remote host whenever
    // the socket is disconnected.
    void AutoReconnect ();
    bool AutoReconnectEnabled ();
    void StopAutoReconnect (); // call before destruction
    void StartAutoReconnect ();
    void TimerReconnect ();
    
    // ping
    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();
    
    void Send (const uintptr_t fields[], unsigned count);

    std::recursive_mutex  critsect;
    LINK(CliAuConn) link;
    AsyncSocket     sock;
    NetCli *        cli;
    ST::string      name;
    plNetAddress    addr;
    plUUID          token;
    unsigned        seq;
    unsigned        serverChallenge;
    hsBitVector     caps;
    AsyncCancelId   cancelId;
    bool            abandoned;
};

//============================================================================
// PingRequestTrans
//============================================================================
struct PingRequestTrans : NetAuthTrans {
    FNetCliAuthPingRequestCallback  m_callback;
    void *                          m_param;
    unsigned                        m_pingAtMs;
    unsigned                        m_replyAtMs;
    std::vector<uint8_t>            m_payload;
    
    PingRequestTrans (
        FNetCliAuthPingRequestCallback  callback,
        void *                          param,
        unsigned                        pingAtMs,
        unsigned                        payloadBytes,
        const void *                    payload
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountExistsRequestTrans
//============================================================================
struct AccountExistsRequestTrans : NetAuthTrans {
    FNetCliAuthAccountExistsRequestCallback     m_callback;
    void *                                      m_param;

    // send    
    char16_t                                    m_accountName[kMaxAccountNameLength];

    // recv
    uint8_t                                        m_exists;
    

    
    AccountExistsRequestTrans (
        FNetCliAuthAccountExistsRequestCallback callback,
        void *                          param,
        const char16_t                  accountName[]
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// LoginRequestTrans
//============================================================================
struct LoginRequestTrans : NetAuthTrans {
    FNetCliAuthLoginRequestCallback m_callback;
    void *                          m_param;

    plUUID                          m_accountId;
    unsigned                        m_accountFlags;
    unsigned                        m_billingType;
    unsigned                        m_playerCount;
    NetCliAuthPlayerInfo            m_players[kMaxPlayersPerAccount];

    LoginRequestTrans (
        FNetCliAuthLoginRequestCallback callback,
        void *                          param
    );

    void AddPlayer (
        unsigned    playerInt,
        const char16_t playerName[],
        const char16_t avatarShape[],
        unsigned    explorer
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AgeRequestTrans
//============================================================================
struct AgeRequestTrans : NetAuthTrans {
    FNetCliAuthAgeRequestCallback       m_callback;
    void *                              m_param;
    ST::string                          m_ageName;
    unsigned                            m_ageMcpId;
    plUUID                              m_ageInstId;
    unsigned                            m_ageVaultId;
    uint32_t                            m_gameSrvNode;

    AgeRequestTrans (
        const ST::string&                   ageName,
        const plUUID&                       ageInstId,
        FNetCliAuthAgeRequestCallback       callback,
        void *                              param
    );
    ~AgeRequestTrans ();

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountCreateRequestTrans
//============================================================================
struct AccountCreateRequestTrans : NetAuthTrans {
    FNetCliAuthAccountCreateRequestCallback m_callback;
    void *                                  m_param;

    // send    
    char16_t                                m_accountName[kMaxAccountNameLength];
    ShaDigest                               m_namePassHash;
    unsigned                                m_accountFlags;
    unsigned                                m_billingType;

    // recv
    plUUID                                  m_accountId;

    AccountCreateRequestTrans (
        const char16_t                          accountName[],
        const char16_t                          password[],
        unsigned                                accountFlags,
        unsigned                                billingType,
        FNetCliAuthAccountCreateRequestCallback callback,
        void *                                  param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountCreateFromKeyRequestTrans
//============================================================================
struct AccountCreateFromKeyRequestTrans : NetAuthTrans {
    FNetCliAuthAccountCreateFromKeyRequestCallback  m_callback;
    void *                                          m_param;

    // send    
    char16_t                                m_accountName[kMaxAccountNameLength];
    ShaDigest                               m_namePassHash;
    plUUID                                  m_key;
    unsigned                                m_billingType;

    // recv
    plUUID                                  m_accountId;
    plUUID                                  m_activationKey;

    AccountCreateFromKeyRequestTrans(
        const char16_t                                  accountName[],
        const char16_t                                  password[],
        const plUUID&                                   key,
        unsigned                                        billingType,
        FNetCliAuthAccountCreateFromKeyRequestCallback  callback,
        void *                                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// PlayerCreateRequestTrans
//============================================================================
struct PlayerCreateRequestTrans : NetAuthTrans {
    FNetCliAuthPlayerCreateRequestCallback  m_callback;
    void *                                  m_param;

    // send
    ST::string                              m_playerName;
    ST::string                              m_avatarShape;
    ST::string                              m_friendInvite;

    // recv
    NetCliAuthPlayerInfo                    m_playerInfo;


    PlayerCreateRequestTrans (
        const ST::string&                       playerName,
        const ST::string&                       avatarShape,
        const ST::string&                       friendInvite,
        FNetCliAuthPlayerCreateRequestCallback  callback,
        void *                                  param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// PlayerDeleteRequestTrans
//============================================================================
struct PlayerDeleteRequestTrans : NetAuthTrans {
    FNetCliAuthPlayerDeleteRequestCallback  m_callback;
    void *                                  m_param;

    // send
    unsigned                                m_playerId;

    // recv
    NetCliAuthPlayerInfo                    m_playerInfo;


    PlayerDeleteRequestTrans (
        unsigned                                playerId,
        FNetCliAuthPlayerDeleteRequestCallback  callback,
        void *                                  param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// UpgradeVisitorRequestTrans
//============================================================================
struct UpgradeVisitorRequestTrans : NetAuthTrans {
    FNetCliAuthUpgradeVisitorRequestCallback    m_callback;
    void *                                      m_param;

    // send
    unsigned                                m_playerId;

    UpgradeVisitorRequestTrans (
        unsigned                                    playerId,
        FNetCliAuthUpgradeVisitorRequestCallback    callback,
        void *                                      param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// SetPlayerRequestTrans
//============================================================================
struct SetPlayerRequestTrans : NetAuthTrans {
    FNetCliAuthSetPlayerRequestCallback m_callback;
    void *                              m_param;
    unsigned                            m_playerInt;

    SetPlayerRequestTrans (
        unsigned                            playerInt,
        FNetCliAuthSetPlayerRequestCallback callback,
        void *                              param
    );
    
    // This transaction doesn't timeout since a client starting from a clean
    // directory can take a long time between issuing this transaction and
    // receiving a reply.
    bool TimedOut() override { return false; }
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountChangePasswordRequestTrans
//============================================================================
struct AccountChangePasswordRequestTrans : NetAuthTrans {
    FNetCliAuthAccountChangePasswordRequestCallback m_callback;
    void *                                          m_param;

    // send
    ST::string                              m_accountName;
    ShaDigest                               m_namePassHash;

    AccountChangePasswordRequestTrans (
        const ST::string&                               accountName,
        const ST::string&                               password,
        FNetCliAuthAccountChangePasswordRequestCallback callback,
        void *                                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// GetPublicAgeListTrans
//============================================================================
struct GetPublicAgeListTrans : NetAuthTrans {
    FNetCliAuthGetPublicAgeListCallback     m_callback;
    void *                                  m_param;
    
    // send
    ST::string                              m_ageName;

    // recv
    std::vector<NetAgeInfo>                 m_ages;
    
    GetPublicAgeListTrans (
        const ST::string&                   ageName,
        FNetCliAuthGetPublicAgeListCallback callback,
        void *                              param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountSetRolesRequestTrans
//============================================================================
struct AccountSetRolesRequestTrans : NetAuthTrans {
    FNetCliAuthAccountSetRolesRequestCallback   m_callback;
    void *                                      m_param;

    // send    
    char16_t                                m_accountName[kMaxAccountNameLength];
    unsigned                                m_accountFlags;

    AccountSetRolesRequestTrans (
        const char16_t                              accountName[],
        unsigned                                    accountFlags,
        FNetCliAuthAccountSetRolesRequestCallback   callback,
        void *                                      param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountSetBillingTypeRequestTrans
//============================================================================
struct AccountSetBillingTypeRequestTrans : NetAuthTrans {
    FNetCliAuthAccountSetBillingTypeRequestCallback m_callback;
    void *                                          m_param;

    // send    
    char16_t                                m_accountName[kMaxAccountNameLength];
    unsigned                                m_billingType;

    AccountSetBillingTypeRequestTrans (
        const char16_t                                  accountName[],
        unsigned                                        billingType,
        FNetCliAuthAccountSetBillingTypeRequestCallback callback,
        void *                                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AccountActivateRequestTrans
//============================================================================
struct AccountActivateRequestTrans : NetAuthTrans {
    FNetCliAuthAccountActivateRequestCallback   m_callback;
    void *                                      m_param;

    // send    
    plUUID                                      m_activationKey;

    AccountActivateRequestTrans (
        const plUUID&                               activationKey,
        FNetCliAuthAccountActivateRequestCallback   callback,
        void *                                      param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// FileListRequestTrans
//============================================================================
struct FileListRequestTrans : NetAuthTrans {
    FNetCliAuthFileListRequestCallback  m_callback;
    void *                              m_param;

    char16_t                            m_directory[kCli2File_FilenameSize];
    char16_t                            m_ext[MAX_EXT];

    std::vector<NetCliAuthFileInfo>       m_fileInfoArray;

    FileListRequestTrans (
        FNetCliAuthFileListRequestCallback  callback,
        void *                              param,
        const char16_t                      directory[],
        const char16_t                      ext[]
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// FileDownloadRequestTrans
//============================================================================
struct FileDownloadRequestTrans : NetAuthTrans {
    FNetCliAuthFileRequestCallback  m_callback;
    void *                          m_param;

    plFileName                      m_filename;
    hsStream *                      m_writer;

    FileDownloadRequestTrans (
        FNetCliAuthFileRequestCallback  callback,
        void *                          param,
        const plFileName &              filename,
        hsStream *                      writer
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// RcvdFileDownloadChunkTrans
//============================================================================
struct RcvdFileDownloadChunkTrans : NetNotifyTrans {

    unsigned    bytes;
    unsigned    offset;
    uint8_t *   data;
    hsStream *  writer;

    RcvdFileDownloadChunkTrans ()
        : NetNotifyTrans(kRcvdFileDownloadChunkTrans),
          bytes(), offset(), data(), writer()
    { }
    ~RcvdFileDownloadChunkTrans ();
    void Post() override;
};


//============================================================================
// RcvdPropagatedBufferTrans
//============================================================================
struct RcvdPropagatedBufferTrans : NetNotifyTrans {

    unsigned        bufferType;
    unsigned        bufferBytes;
    uint8_t *          bufferData;

    RcvdPropagatedBufferTrans()
        : NetNotifyTrans(kRcvdPropagatedBufferTrans),
          bufferType(), bufferBytes(), bufferData()
    { }
    ~RcvdPropagatedBufferTrans ();
    void Post() override;
};

//============================================================================
// VaultNodeChangedTrans
//============================================================================
struct VaultNodeChangedTrans : NetNotifyTrans {

    unsigned        m_nodeId;
    plUUID          m_revId;

    VaultNodeChangedTrans ()
        : NetNotifyTrans(kVaultNodeChangedTrans), m_nodeId()
    { }
    void Post() override;
};

//============================================================================
// VaultNodeAddedTrans
//============================================================================
struct VaultNodeAddedTrans : NetNotifyTrans {

    unsigned        m_parentId;
    unsigned        m_childId;
    unsigned        m_ownerId;

    VaultNodeAddedTrans ()
        : NetNotifyTrans(kVaultNodeAddedTrans),
          m_parentId(), m_childId(), m_ownerId()
    { }
    void Post() override;
};

//============================================================================
// VaultNodeRemovedTrans
//============================================================================
struct VaultNodeRemovedTrans : NetNotifyTrans {

    unsigned        m_parentId;
    unsigned        m_childId;

    VaultNodeRemovedTrans ()
        : NetNotifyTrans(kVaultNodeRemovedTrans), m_parentId(), m_childId()
    { }
    void Post() override;
};

//============================================================================
// VaultNodeDeletedTrans
//============================================================================
struct VaultNodeDeletedTrans : NetNotifyTrans {

    unsigned        m_nodeId;

    VaultNodeDeletedTrans ()
        : NetNotifyTrans(kVaultNodeDeletedTrans), m_nodeId()
    { }
    void Post() override;
};

//============================================================================
// VaultFetchNodeRefsTrans
//============================================================================
struct VaultFetchNodeRefsTrans : NetAuthTrans {

    unsigned                        m_nodeId;
    FNetCliAuthVaultNodeRefsFetched m_callback;
    void *                          m_param;

    std::vector<NetVaultNodeRef>    m_refs;

    VaultFetchNodeRefsTrans (
        unsigned                        nodeId,
        FNetCliAuthVaultNodeRefsFetched callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultInitAgeTrans
//============================================================================
struct VaultInitAgeTrans : NetAuthTrans {
    FNetCliAuthAgeInitCallback  m_callback;
    void *                      m_param;

    plUUID                      m_ageInstId;
    plUUID                      m_parentAgeInstId;
    ST::string                  m_ageFilename;
    ST::string                  m_ageInstName;
    ST::string                  m_ageUserName;
    ST::string                  m_ageDesc;
    unsigned                    m_ageSequenceNumber;
    unsigned                    m_ageLanguage;

    unsigned                    m_ageId;
    unsigned                    m_ageInfoId;

    VaultInitAgeTrans(
        FNetCliAuthAgeInitCallback  callback,           // optional
        void *                      param,              // optional
        const plUUID&               ageInstId,          // optional. is used in match
        const plUUID&               parentAgeInstId,    // optional. is used in match
        const ST::string&           ageFilename,        // optional. is used in match
        const ST::string&           ageInstName,        // optional. not used in match
        const ST::string&           ageUserName,        // optional. not used in match
        const ST::string&           ageDesc,            // optional. not used in match
        unsigned                    ageSequenceNumber,  // optional. not used in match
        unsigned                    ageLanguage         // optional. not used in match
    );
    ~VaultInitAgeTrans ();

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultFetchNodeTrans
//============================================================================
struct VaultFetchNodeTrans : NetAuthTrans {

    unsigned                    m_nodeId;
    FNetCliAuthVaultNodeFetched m_callback;
    void *                      m_param;
    
    hsRef<NetVaultNode>         m_node;
    
    VaultFetchNodeTrans (
        unsigned                    nodeId,
        FNetCliAuthVaultNodeFetched callback,
        void *                      param
    );
    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultFindNodeTrans
//============================================================================
struct VaultFindNodeTrans : NetAuthTrans {

    std::vector<uint8_t>        m_buffer;
    std::vector<unsigned>       m_nodeIds;
    FNetCliAuthVaultNodeFind    m_callback;
    void *                      m_param;

    VaultFindNodeTrans (
        NetVaultNode *              templateNode,
        FNetCliAuthVaultNodeFind    callback,
        void *                      param
    );

    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultCreateNodeTrans
//============================================================================
struct VaultCreateNodeTrans : NetAuthTrans {

    std::vector<uint8_t>            m_buffer;
    FNetCliAuthVaultNodeCreated     m_callback;
    void *                          m_param;
    unsigned                        m_nodeId;

    VaultCreateNodeTrans (
        NetVaultNode *                  templateNode,
        FNetCliAuthVaultNodeCreated     callback,
        void *                          param
    );
    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultSaveNodeTrans
//============================================================================
struct VaultSaveNodeTrans : NetAuthTrans {

    unsigned                            m_nodeId;
    plUUID                              m_revisionId;
    std::vector<uint8_t>                m_buffer;
    FNetCliAuthVaultNodeSaveCallback    m_callback;
    void *                              m_param;

    VaultSaveNodeTrans (
        unsigned                            nodeId,
        const plUUID&                       revisionId,
        unsigned                            dataCount,
        const void *                        data,
        FNetCliAuthVaultNodeSaveCallback    callback,
        void *                              param
    );
    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultAddNodeTrans
//============================================================================
struct VaultAddNodeTrans : NetAuthTrans {

    unsigned                        m_parentId;
    unsigned                        m_childId;
    unsigned                        m_ownerId;
    FNetCliAuthVaultNodeAddCallback m_callback;
    void *                          m_param;
    
    VaultAddNodeTrans (
        unsigned                        parentId,
        unsigned                        childId,
        unsigned                        ownerId,
        FNetCliAuthVaultNodeAddCallback callback,
        void *                          param
    );
    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// VaultRemoveNodeTrans
//============================================================================
struct VaultRemoveNodeTrans : NetAuthTrans {

    unsigned                            m_parentId;
    unsigned                            m_childId;
    FNetCliAuthVaultNodeRemoveCallback  m_callback;
    void *                              m_param;

    VaultRemoveNodeTrans (
        unsigned                            parentId,
        unsigned                            childId,
        FNetCliAuthVaultNodeRemoveCallback  callback,
        void *                              param
    );
    
    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// NotifyNewBuildTrans
//============================================================================
struct NotifyNewBuildTrans : NetNotifyTrans {

    NotifyNewBuildTrans () : NetNotifyTrans(kNotifyNewBuildTrans) {}
    void Post() override;
};

//============================================================================
// SetPlayerBanStatusRequestTrans
//============================================================================
struct SetPlayerBanStatusRequestTrans : NetAuthTrans {
    FNetCliAuthSetPlayerBanStatusRequestCallback    m_callback;
    void *                                          m_param;

    // send    
    unsigned                                            m_playerId;
    unsigned                                            m_banned;

    SetPlayerBanStatusRequestTrans (
        unsigned                                        playerId,
        unsigned                                        banned,
        FNetCliAuthSetPlayerBanStatusRequestCallback    callback,
        void *                                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ChangePlayerNameRequestTrans
//============================================================================
struct ChangePlayerNameRequestTrans : NetAuthTrans {
    FNetCliAuthChangePlayerNameRequestCallback  m_callback;
    void *                                      m_param;

    // send    
    unsigned                                        m_playerId;
    char16_t                                        m_newName[kMaxPlayerNameLength];

    ChangePlayerNameRequestTrans (
        unsigned                                    playerId,
        const char16_t                              newName[],
        FNetCliAuthChangePlayerNameRequestCallback  callback,
        void *                                      param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// SendFriendInviteTrans
//============================================================================
struct SendFriendInviteTrans : NetAuthTrans {
    FNetCliAuthSendFriendInviteCallback m_callback;
    void *                              m_param;

    // send
    ST::string                            m_emailAddress;
    ST::string                            m_toName;
    plUUID                                m_inviteUuid;

    SendFriendInviteTrans(
        const ST::string&                       emailAddr,
        const ST::string&                       toName,
        const plUUID&                           inviteUuid,
        FNetCliAuthSendFriendInviteCallback     callback,
        void *                                  param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AuthConnectedNotifyTrans
//============================================================================
struct AuthConnectedNotifyTrans : NetNotifyTrans {

    AuthConnectedNotifyTrans () : NetNotifyTrans(kAuthConnectedNotifyTrans) {}
    void Post() override;
};


//============================================================================
// ScoreCreateTrans
//============================================================================
struct ScoreCreateTrans : NetAuthTrans {
    FNetCliAuthCreateScoreCallback  m_callback;
    void *                          m_param;

    // send    
    unsigned                        m_ownerId;
    ST::string                      m_gameName;
    unsigned                        m_gameType;
    int                             m_value;

    // recv
    unsigned                        m_scoreId;
    uint32_t                        m_createdTime;

    ScoreCreateTrans (
        unsigned                        ownerId,
        const ST::string&               gameName,
        unsigned                        gameType,
        int                             value,
        FNetCliAuthCreateScoreCallback  callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreDeleteTrans
//============================================================================
struct ScoreDeleteTrans : NetAuthTrans {
    FNetCliAuthScoreUpdateCallback  m_callback;
    void *                          m_param;

    // send    
    unsigned                        m_scoreId;

    ScoreDeleteTrans (
        unsigned                        scoreId,
        FNetCliAuthScoreUpdateCallback  callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreGetScoresTrans
//============================================================================
struct ScoreGetScoresTrans : NetAuthTrans {
    FNetCliAuthGetScoresCallback    m_callback;
    void *                          m_param;

    // send
    unsigned                        m_ownerId;
    ST::string                      m_gameName;

    // recv
    NetGameScore *                  m_scores;
    unsigned                        m_scoreCount;

    ScoreGetScoresTrans (
        unsigned                        ownerId,
        const ST::string&               gameName,
        FNetCliAuthGetScoresCallback    callback,
        void *                          param
    );

    ~ScoreGetScoresTrans();

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreAddPointsTrans
//============================================================================
struct ScoreAddPointsTrans : NetAuthTrans {
    FNetCliAuthScoreUpdateCallback  m_callback;
    void *                          m_param;

    // send    
    unsigned                        m_scoreId;
    int                             m_numPoints;

    ScoreAddPointsTrans (
        unsigned                        scoreId,
        int                             numPoints,
        FNetCliAuthScoreUpdateCallback  callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreTransferPointsTrans
//============================================================================
struct ScoreTransferPointsTrans : NetAuthTrans {
    FNetCliAuthScoreUpdateCallback  m_callback;
    void *                          m_param;

    // send    
    unsigned                        m_srcScoreId;
    unsigned                        m_destScoreId;
    int                             m_numPoints;

    ScoreTransferPointsTrans (
        unsigned                        srcScoreId,
        unsigned                        destScoreId,
        int                             numPoints,
        FNetCliAuthScoreUpdateCallback  callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreSetPointsTrans
//============================================================================
struct ScoreSetPointsTrans : NetAuthTrans {
    FNetCliAuthScoreUpdateCallback  m_callback;
    void *                          m_param;

    // send    
    unsigned                        m_scoreId;
    int                             m_numPoints;

    ScoreSetPointsTrans (
        unsigned                        scoreId,
        int                             numPoints,
        FNetCliAuthScoreUpdateCallback  callback,
        void *                          param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreGetRanksTrans
//============================================================================
struct ScoreGetRanksTrans : NetAuthTrans {
    FNetCliAuthGetRanksCallback     m_callback;
    void *                          m_param;

    // send
    unsigned                        m_ownerId;
    unsigned                        m_scoreGroup;
    unsigned                        m_parentFolderId;
    ST::string                      m_gameName;
    unsigned                        m_timePeriod;
    unsigned                        m_numResults;
    unsigned                        m_pageNumber;
    unsigned                        m_sortDesc;

    // recv
    NetGameRank *                   m_ranks;
    unsigned                        m_rankCount;

    ScoreGetRanksTrans (
        unsigned                    ownerId,
        unsigned                    scoreGroup,
        unsigned                    parentFolderId,
        const ST::string&           gameName,
        unsigned                    timePeriod,
        unsigned                    numResults,
        unsigned                    pageNumber,
        bool                        sortDesc,
        FNetCliAuthGetRanksCallback callback,
        void *                      param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ScoreGetHighScoresTrans
//============================================================================
struct ScoreGetHighScoresTrans : NetAuthTrans {
    FNetCliAuthGetScoresCallback    m_callback;
    void *                          m_param;

    // send
    unsigned                        m_ageId;
    unsigned                        m_maxScores;
    ST::string                      m_gameName;

    // recv
    NetGameScore *                  m_scores;
    unsigned                        m_scoreCount;

    ScoreGetHighScoresTrans(
        unsigned                        ageId,
        unsigned                        maxScores,
        const ST::string&               gameName,
        FNetCliAuthGetScoresCallback    callback,
        void *                          param
        );

    ~ScoreGetHighScoresTrans();

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};


/*****************************************************************************
*
*   Private data
*
***/

enum {
    kPerfConnCount,
    kPingDisabled,
    kAutoReconnectDisabled,
    kNumPerf
};

static bool                         s_running;
static std::recursive_mutex         s_critsect;
static LISTDECL(CliAuConn, link)    s_conns;
static CliAuConn *                  s_active;
static ST::string                   s_accountName;
static ShaDigest                    s_accountNamePassHash;
static char16_t                     s_authToken[kMaxPublisherAuthKeyLength];
static char16_t                     s_os[kMaxGTOSIdLength];

static std::atomic<long>            s_perf[kNumPerf];

static uint32_t                     s_encryptionKey[4];

static FNetCliAuthRecvBufferHandler         s_bufRcvdCb;
static FNetCliAuthConnectCallback           s_connectedCb;

// Vault notification handlers
static FNetCliAuthVaultNodeChanged  s_vaultNodeChangedHandler;
static FNetCliAuthVaultNodeAdded    s_vaultNodeAddedHandler;
static FNetCliAuthVaultNodeRemoved  s_vaultNodeRemovedHandler;
static FNetCliAuthVaultNodeDeleted  s_vaultNodeDeletedHandler;

static FNotifyNewBuildHandler       s_notifyNewBuildHandler;


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static ENetError FixupPlayerName (ST::string& name) {
    ASSERT(!name.empty());

    // Trim leading and trailing whitespace
    name = name.trim(" \t\n\r");

    // Convert remaining internal whitespace to a single space.
    // Kind of hacky, but meh.
    std::vector<ST::string> things = name.tokenize(" \t\n\r");

    ST::string_stream ss;
    for (auto it = things.begin(); it != things.end(); ++it) {
        ss << *it;
        if ((it + 1) != things.end())
            ss << " ";
    }
    name = ss.to_string();

    // Now, check to see if we have the appropriate length
    // We could count the characters, but lazy...
    if (name.replace(" ", "").size() < 3)
        return kNetErrPlayerNameInvalid;
    return kNetSuccess;
}

//===========================================================================
static unsigned GetNonZeroTimeMs () {
    if (unsigned ms = hsTimer::GetMilliSeconds<uint32_t>())
        return ms;
    return 1;
}

//============================================================================
static CliAuConn * GetConnIncRef_CS (const char tag[]) {
    if (CliAuConn * conn = s_active) {
        conn->Ref(tag);
        return conn;
    }
    return nullptr;
}

//============================================================================
static CliAuConn * GetConnIncRef (const char tag[]) {
    hsLockGuard(s_critsect);
    return GetConnIncRef_CS(tag);
}

//============================================================================
static void UnlinkAndAbandonConn_CS (CliAuConn * conn) {
    s_conns.Unlink(conn);
    conn->abandoned = true;

    conn->StopAutoReconnect();

    if (conn->cancelId) {
        AsyncSocketConnectCancel(conn->cancelId);
        conn->cancelId  = nullptr;
    }
    else if (conn->sock) {
        AsyncSocketDisconnect(conn->sock, true);
    }
    else {
        conn->UnRef("Lifetime");
    }
}

//============================================================================
static void SendClientRegisterRequest (CliAuConn * conn) {
    const uintptr_t msg[] = {
        kCli2Auth_ClientRegisterRequest,
        plProduct::BuildId(),
    };

    conn->Send(msg, std::size(msg));
}

//============================================================================
static bool ConnEncrypt (ENetError error, void * param) {
    CliAuConn * conn = (CliAuConn *) param;
        
    if (IS_NET_SUCCESS(error)) {

        SendClientRegisterRequest(conn);

        if (!s_perf[kPingDisabled])
            conn->AutoPing();
            
        AuthConnectedNotifyTrans * trans = new AuthConnectedNotifyTrans;
        NetTransSend(trans);
    }

    return IS_NET_SUCCESS(error);
}

//============================================================================
static void NotifyConnSocketConnect (CliAuConn * conn) {

    conn->TransferRef("Connecting", "Connected");
    conn->cli = NetCliConnectAccept(
        conn->sock,
        kNetProtocolCli2Auth,
        false,
        ConnEncrypt,
        0,
        nullptr,
        conn
    );
}

//============================================================================
static void CheckedReconnect (CliAuConn * conn, ENetError error) {

    unsigned disconnectedMs = GetNonZeroTimeMs() - conn->lastHeardTimeMs;

    // no auto-reconnect or haven't heard from the server in a while?
    if (!conn->AutoReconnectEnabled() || (int)disconnectedMs >= (int)kDisconnectedTimeoutMs) {
        // Cancel all transactions in progress on this connection.
        NetTransCancelByConnId(conn->seq, kNetErrTimeout);
        // conn is dead.
        conn->UnRef("Lifetime");
        ReportNetError(kNetProtocolCli2Auth, error);
    }
    else {
        if (conn->token != kNilUuid)
            // previously encrypted; reconnect quickly
            conn->reconnectStartMs = GetNonZeroTimeMs() + 500;
        else
            // never encrypted; reconnect slowly
            conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;

        // clean up the socket and start reconnect
        if (conn->cli)
            NetCliDelete(conn->cli, true);
        conn->cli = nullptr;
        conn->sock = nullptr;
        
        conn->StartAutoReconnect();
    }
}

//============================================================================
static void NotifyConnSocketConnectFailed (CliAuConn * conn) {

    {
        hsLockGuard(s_critsect);
        conn->cancelId = nullptr;
        s_conns.Unlink(conn);

        if (conn == s_active)
            s_active = nullptr;
    }
    
    CheckedReconnect(conn, kNetErrConnectFailed);
    
    conn->UnRef("Connecting");
}

//============================================================================
static void NotifyConnSocketDisconnect (CliAuConn * conn) {

    conn->StopAutoPing();

    {
        hsLockGuard(s_critsect);
        conn->cancelId = nullptr;
        s_conns.Unlink(conn);
            
        if (conn == s_active)
            s_active = nullptr;
    }


    CheckedReconnect(conn, kNetErrDisconnected);

    conn->UnRef("Connected");
}

//============================================================================
static bool NotifyConnSocketRead (CliAuConn * conn, AsyncNotifySocketRead * read) {
    // TODO: Only dispatch messages from the active auth server
    conn->lastHeardTimeMs = GetNonZeroTimeMs();
    bool result = NetCliDispatch(conn->cli, read->buffer, read->bytes, conn);
    read->bytesProcessed += read->bytes;
    return result;
}

//============================================================================
static bool SocketNotifyCallback (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    bool result = true;
    CliAuConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess:
            conn = (CliAuConn *) notify->param;
            *userState = conn;
            bool abandoned;
            {
                hsLockGuard(s_critsect);
                conn->sock      = sock;
                conn->cancelId  = nullptr;
                abandoned       = conn->abandoned;
            }
            if (abandoned)
                AsyncSocketDisconnect(sock, true);
            else
                NotifyConnSocketConnect(conn);
        break;

        case kNotifySocketConnectFailed:
            conn = (CliAuConn *) notify->param;
            NotifyConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (CliAuConn *) *userState;
            NotifyConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (CliAuConn *) *userState;
            result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;

        case kNotifySocketWrite:
            // No action
        break;
    }
    
    return result;
}

//============================================================================
static void Connect (
    CliAuConn * conn
) {
    ASSERT(s_running);

    conn->pingSendTimeMs = 0;

    {
        hsLockGuard(s_critsect);
        while (CliAuConn * oldConn = s_conns.Head()) {
            if (oldConn != conn)
                UnlinkAndAbandonConn_CS(oldConn);
            else
                s_conns.Unlink(oldConn);
        }
        s_conns.Link(conn);
    }
    
    Cli2Auth_Connect connect;
    connect.hdr.connType        = kConnTypeCliToAuth;
    connect.hdr.hdrBytes        = sizeof(connect.hdr);
    connect.hdr.buildId         = plProduct::BuildId();
    connect.hdr.buildType       = plProduct::BuildType();
    connect.hdr.branchId        = plProduct::BranchId();
    connect.hdr.productId       = plProduct::UUID();
    connect.data.token          = conn->token;
    connect.data.dataBytes      = sizeof(connect.data);

    AsyncSocketConnect(
        &conn->cancelId,
        conn->addr,
        SocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect)
    );
}

//============================================================================
static void Connect (
    const ST::string&   name,
    const plNetAddress& addr
) {
    ASSERT(s_running);
    
    CliAuConn * conn        = new CliAuConn;
    conn->addr              = addr;
    conn->seq               = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();   // used in connect timeout, and ping timeout
    conn->name              = name;

    conn->Ref("Lifetime");
    conn->AutoReconnect();
}

//============================================================================
static void AsyncLookupCallback(void* param, const ST::string& name,
                                const std::vector<plNetAddress>& addrs)
{
    if (addrs.empty()) {
        ReportNetError(kNetProtocolCli2Auth, kNetErrNameLookupFailed);
        return;
    }

    for (const plNetAddress& addr : addrs)
        Connect(name, addr);
}


/*****************************************************************************
*
*   CliAuConn
*
***/

//===========================================================================
static unsigned CliAuConnTimerDestroyed (void * param) {
    CliAuConn * conn = (CliAuConn *) param;
    conn->UnRef("TimerDestroyed");
    return kAsyncTimeInfinite;
}

//===========================================================================
static unsigned CliAuConnReconnectTimerProc (void * param) {
    ((CliAuConn *) param)->TimerReconnect();
    return kAsyncTimeInfinite;
}

//===========================================================================
static unsigned CliAuConnPingTimerProc (void * param) {
    ((CliAuConn *) param)->TimerPing();
    return kPingIntervalMs;
}

//============================================================================
CliAuConn::CliAuConn ()
    : hsRefCnt(0), reconnectTimer(), reconnectStartMs()
    , pingTimer(), pingSendTimeMs(), lastHeardTimeMs()
    , sock(), cli(), seq(), serverChallenge()
    , cancelId(), abandoned()
{
    ++s_perf[kPerfConnCount];
}

//============================================================================
CliAuConn::~CliAuConn () {
    if (cli)
        NetCliDelete(cli, true);
    --s_perf[kPerfConnCount];
}

//===========================================================================
void CliAuConn::TimerReconnect () {
    ASSERT(!sock);
    ASSERT(!cancelId);
    
    if (!s_running) {
        hsLockGuard(s_critsect);
        UnlinkAndAbandonConn_CS(this);
    }
    else {
        Ref("Connecting");

        // Remember the time we started the reconnect attempt, guarding against
        // TimeGetMs() returning zero (unlikely), as a value of zero indicates
        // a first-time connect condition to StartAutoReconnect()
        reconnectStartMs = GetNonZeroTimeMs();

        Connect(this);
    }
}

//===========================================================================
// This function is called when after a disconnect to start a new connection
void CliAuConn::StartAutoReconnect () {
    hsLockGuard(critsect);
    if (reconnectTimer && !s_perf[kAutoReconnectDisabled]) {
        // Make reconnect attempts at regular intervals. If the last attempt
        // took more than the specified max interval time then reconnect
        // immediately; otherwise wait until the time interval is up again
        // then reconnect.
        unsigned remainingMs = 0;
        if (reconnectStartMs) {
            remainingMs = reconnectStartMs - GetNonZeroTimeMs();
            if ((signed)remainingMs < 0)
                remainingMs = 0;
            LogMsg(kLogPerf, "Auth auto-reconnecting in {} ms", remainingMs);
        }
        AsyncTimerUpdate(reconnectTimer, remainingMs);
    }
}

//===========================================================================
// This function should be called during object construction
// to initiate connection attempts to the remote host whenever
// the socket is disconnected.
void CliAuConn::AutoReconnect () {
        
    ASSERT(!reconnectTimer);
    Ref("ReconnectTimer");
    hsLockGuard(critsect);
    reconnectTimer = AsyncTimerCreate(
        CliAuConnReconnectTimerProc,
        0,  // immediate callback
        this
    );
}

//============================================================================
void CliAuConn::StopAutoReconnect () {
    hsLockGuard(critsect);
    if (AsyncTimer * timer = reconnectTimer) {
        reconnectTimer = nullptr;
        AsyncTimerDeleteCallback(timer, CliAuConnTimerDestroyed);
    }
}

//============================================================================
bool CliAuConn::AutoReconnectEnabled () {
    
    return (reconnectTimer != nullptr) && !s_perf[kAutoReconnectDisabled];
}

//============================================================================
void CliAuConn::AutoPing () {
    ASSERT(!pingTimer);
    Ref("PingTimer");
    hsLockGuard(critsect);
    pingTimer = AsyncTimerCreate(
        CliAuConnPingTimerProc,
        sock ? 0 : kAsyncTimeInfinite,
        this
    );
}

//============================================================================
void CliAuConn::StopAutoPing () {
    hsLockGuard(critsect);
    if (pingTimer) {
        AsyncTimerDeleteCallback(pingTimer, CliAuConnTimerDestroyed);
        pingTimer = nullptr;
    }
}

//============================================================================
void CliAuConn::TimerPing () {
    // Send a ping request
    pingSendTimeMs = GetNonZeroTimeMs();

    const uintptr_t msg[] = {
        kCli2Auth_PingRequest,
        pingSendTimeMs,
        0,      // not a transaction
        0,      // no payload
        reinterpret_cast<uintptr_t>(nullptr)
    };

    Send(msg, std::size(msg));
}

//============================================================================
void CliAuConn::Send (const uintptr_t fields[], unsigned count) {
    hsLockGuard(critsect);
    NetCliSend(cli, fields, count);
    NetCliFlush(cli);
}


/*****************************************************************************
*
*   Cli2Auth message handlers
*
***/

template<typename T>
bool RecvMsg(const uint8_t msg[], unsigned bytes, void* param)
{
    // Stupid nested namespaces...
    return ::NetTransRecvFromMsgGeneric<T>(msg, bytes, param);
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_ClientRegisterReply>(const uint8_t msg[], unsigned, void* param)
{
    const Auth2Cli_ClientRegisterReply & reply = *(const Auth2Cli_ClientRegisterReply *)msg;

    CliAuConn * conn = (CliAuConn *) param;
    conn->serverChallenge = reply.serverChallenge;

    // Make this the active server
    hsLockGuard(s_critsect);
    s_active = conn;

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_KickedOff>(const uint8_t buffer[], unsigned bytes, void* param)
{
    const Auth2Cli_KickedOff & msg = *(const Auth2Cli_KickedOff *)buffer;

    ReportNetError(kNetProtocolCli2Auth, msg.reason);
    NetCliAuthDisconnect();

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_VaultNodeChanged>(const uint8_t msg[], unsigned, void*)
{
    const Auth2Cli_VaultNodeChanged & notify = *(const Auth2Cli_VaultNodeChanged *)msg;

    VaultNodeChangedTrans * trans = new VaultNodeChangedTrans;
    trans->m_nodeId     = notify.nodeId;
    trans->m_revId      = notify.revisionId;
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_VaultNodeAdded>(const uint8_t msg[], unsigned, void*)
{
    const Auth2Cli_VaultNodeAdded & notify = *(const Auth2Cli_VaultNodeAdded *)msg;

    VaultNodeAddedTrans * trans = new VaultNodeAddedTrans;
    trans->m_parentId   = notify.parentId;
    trans->m_childId    = notify.childId;
    trans->m_ownerId    = notify.ownerId;
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_VaultNodeRemoved>(const uint8_t msg[], unsigned, void*) {
    const Auth2Cli_VaultNodeRemoved& notify = *(const Auth2Cli_VaultNodeRemoved *)msg;

    VaultNodeRemovedTrans* trans = new VaultNodeRemovedTrans;
    trans->m_parentId = notify.parentId;
    trans->m_childId = notify.childId;
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_VaultNodeDeleted>(const uint8_t msg[], unsigned, void*)
{
    const Auth2Cli_VaultNodeDeleted& notify = *(const Auth2Cli_VaultNodeDeleted *)msg;

    VaultNodeDeletedTrans * trans = new VaultNodeDeletedTrans;
    trans->m_nodeId = notify.nodeId;
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_ServerAddr>(const uint8_t in[], unsigned, void*)
{
    // the auth server has given us its actual address (behind any load-balancer)
    // so that when we attempt a reconnect, we'll reconnect with our state on
    // the auth (but only if we reconnect in a short period of time!)
    const Auth2Cli_ServerAddr& msg = *(const Auth2Cli_ServerAddr*)in;

    hsLockGuard(s_critsect);
    if (s_active) {
        s_active->token = msg.token;
        s_active->addr.SetHost(msg.srvAddr);

        LogMsg(kLogPerf, "SrvAuth addr: {}", s_active->addr.GetHostString());
    }

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_NotifyNewBuild>(const uint8_t[], unsigned, void*) {
    NotifyNewBuildTrans * trans = new NotifyNewBuildTrans;
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Auth2Cli_ServerCaps>(const uint8_t msg[], unsigned bytes, void* param)
{
    const Auth2Cli_ServerCaps& caps = *(const Auth2Cli_ServerCaps*)msg;

    hsReadOnlyStream stream(caps.byteCount, reinterpret_cast<const void*>(caps.buffer));
    ((CliAuConn*)param)->caps.Read(&stream);

    return true;
}

/*****************************************************************************
*
*   Cli2Auth protocol
*
***/

#define MSG(s)  &kNetMsg_Cli2Auth_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest)              },
    { MSG(ClientRegisterRequest)    },
    { MSG(AcctLoginRequest)         },
    { MSG(AcctCreateRequest)        },
    { MSG(AcctCreateFromKeyRequest) },
    { MSG(PlayerCreateRequest)      },
    { MSG(PlayerDeleteRequest)      },
    { MSG(UpgradeVisitorRequest)    },
    { MSG(AcctSetPlayerRequest)     },
    { MSG(AcctChangePasswordRequest)},
    { MSG(AcctSetRolesRequest)      },
    { MSG(AcctSetBillingTypeRequest)},
    { MSG(AcctActivateRequest)      },
    { MSG(AgeRequest)               },
    { MSG(FileListRequest)          },
    { MSG(FileDownloadRequest)      },
    { MSG(FileDownloadChunkAck)     },
    { MSG(VaultFetchNodeRefs)       },
    { MSG(VaultNodeAdd)             },
    { MSG(VaultNodeRemove)          },
    { MSG(VaultNodeFetch)           },
    { MSG(VaultNodeCreate)          },
    { MSG(VaultNodeSave)            },
    { MSG(VaultInitAgeRequest)      },
    { MSG(VaultNodeFind)            },
    { MSG(VaultSetSeen)             },
    { MSG(VaultSendNode)            },
    { MSG(GetPublicAgeList)         },
    { MSG(SetAgePublic)             },
    { MSG(PropagateBuffer)          },
    { MSG(ClientSetCCRLevel)        },
    { MSG(LogPythonTraceback)       },
    { MSG(LogStackDump)             },
    { MSG(LogClientDebuggerConnect) },
    { MSG(SetPlayerBanStatusRequest)},
    { MSG(KickPlayer)               },
    { MSG(ChangePlayerNameRequest)  },
    { MSG(SendFriendInviteRequest)  },
    { MSG(ScoreCreate)              },
    { MSG(ScoreDelete)              },
    { MSG(ScoreGetScores)           },
    { MSG(ScoreAddPoints)           },
    { MSG(ScoreTransferPoints)      },
    { MSG(ScoreSetPoints)           },
    { MSG(ScoreGetRanks)            },
    { MSG(AccountExistsRequest)     },
    { MSG(ScoreGetHighScores)       },
};
#undef MSG

#define MSG(s)  &kNetMsg_Auth2Cli_##s, RecvMsg<Auth2Cli_##s>
static NetMsgInitRecv s_recv[] = {
    { MSG(PingReply)                },
    { MSG(ClientRegisterReply)      },
    { MSG(AcctPlayerInfo)           },
    { MSG(AcctLoginReply)           },
    { MSG(AcctCreateReply)          },
    { MSG(AcctCreateFromKeyReply)   },
    { MSG(PlayerCreateReply)        },
    { MSG(PlayerDeleteReply)        },
    { MSG(UpgradeVisitorReply)      },
    { MSG(AcctSetPlayerReply)       },
    { MSG(AcctChangePasswordReply)  },
    { MSG(AcctSetRolesReply)        },
    { MSG(AcctSetBillingTypeReply)  },
    { MSG(AcctActivateReply)        },
    { MSG(AgeReply)                 },
    { MSG(FileListReply)            },
    { MSG(FileDownloadChunk)        },
    { MSG(KickedOff)                },
    { MSG(VaultNodeRefsFetched)     },
    { MSG(VaultNodeFetched)         },
    { MSG(VaultNodeCreated)         },
    { MSG(VaultNodeChanged)         },
    { MSG(VaultNodeAdded)           },
    { MSG(VaultNodeRemoved)         },
    { MSG(VaultNodeDeleted)         },
    { MSG(VaultSaveNodeReply)       },
    { MSG(VaultAddNodeReply)        },
    { MSG(VaultRemoveNodeReply)     },
    { MSG(VaultInitAgeReply)        },
    { MSG(VaultNodeFindReply)       },
    { MSG(PublicAgeList)            },
    { MSG(ServerAddr)               },
    { MSG(NotifyNewBuild)           },
    { MSG(SetPlayerBanStatusReply)  },
    { MSG(ChangePlayerNameReply)    },
    { MSG(SendFriendInviteReply)    },
    { MSG(ScoreCreateReply)         },
    { MSG(ScoreDeleteReply)         },
    { MSG(ScoreGetScoresReply)      },
    { MSG(ScoreAddPointsReply)      },
    { MSG(ScoreTransferPointsReply) },
    { MSG(ScoreSetPointsReply)      },
    { MSG(ScoreGetRanksReply)       },
    { MSG(AccountExistsReply)       },
    { MSG(ScoreGetHighScoresReply)  },
    { MSG(ServerCaps)               },
};
#undef MSG


/*****************************************************************************
*
*   PingRequestTrans
*
***/

//============================================================================
PingRequestTrans::PingRequestTrans (
    FNetCliAuthPingRequestCallback  callback,
    void *                          param,
    unsigned                        pingAtMs,
    unsigned                        payloadBytes,
    const void *                    payload
) : NetAuthTrans(kPingRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_pingAtMs(pingAtMs)
,   m_payload((const uint8_t *)payload, (const uint8_t *)payload + payloadBytes)
{
}

//============================================================================
bool PingRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_PingRequest,
                        m_pingAtMs,
                        m_transId,
                        m_payload.size(),
        (uintptr_t)  m_payload.data(),
    };
    
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void PingRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_pingAtMs,
        m_replyAtMs,
        m_payload.size(),
        m_payload.data()
    );
}

//============================================================================
bool PingRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_PingReply & reply = *(const Auth2Cli_PingReply *)msg;

    m_payload.assign(reply.payload, reply.payload + reply.payloadBytes);
    m_replyAtMs     = hsTimer::GetMilliSeconds<uint32_t>();
    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;

    return true;
}


/*****************************************************************************
*
*   AccountExistsRequestTrans
*
***/

//============================================================================
AccountExistsRequestTrans::AccountExistsRequestTrans(
        FNetCliAuthAccountExistsRequestCallback callback,
        void* param, const char16_t accountName[])
    : NetAuthTrans(kPingRequestTrans), m_callback(callback), m_param(param),
      m_exists()
{
    StrCopy(m_accountName, accountName, kMaxAccountNameLength);
}

//============================================================================
bool AccountExistsRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AccountExistsRequest,
                        m_transId,
                        (uintptr_t)m_accountName
    };
    
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void AccountExistsRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_exists
    );
}

//============================================================================
bool AccountExistsRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AccountExistsReply & reply = *(const Auth2Cli_AccountExistsReply *)msg;

    m_exists        = reply.exists;
    m_result        = reply.result;
    m_state         = kTransStateComplete;

    return true;
}


/*****************************************************************************
*
*   LoginRequestTrans
*
***/

//============================================================================
LoginRequestTrans::LoginRequestTrans(
        FNetCliAuthLoginRequestCallback callback, void* param)
    : NetAuthTrans(kLoginRequestTrans), m_callback(callback), m_param(param),
      m_accountId(kNilUuid), m_accountFlags(), m_playerCount(), m_billingType()
{
    memset(&m_players, 0, sizeof(m_players));
}

//============================================================================
void LoginRequestTrans::AddPlayer (
    unsigned      playerInt,
    const char16_t playerName[],
    const char16_t avatarShape[],
    unsigned      explorer
) {
    unsigned index = m_playerCount++;
    ASSERT(index < kMaxPlayersPerAccount);
    m_players[index].playerInt   = playerInt;
    m_players[index].explorer    = explorer;
    m_players[index].playerName  = ST::string::from_utf16(playerName);
    m_players[index].avatarShape = ST::string::from_utf16(avatarShape);
}

//============================================================================
bool LoginRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    ShaDigest challengeHash;
    uint32_t clientChallenge = 0;

    // Regex search for primary email domain
    static const std::regex re_domain("[^@]+@([^.]+\\.)*([^.]+)\\.[^.]+");
    std::cmatch match;
    std::regex_search(s_accountName.c_str(), match, re_domain);
    if (match.empty() || ST::string(match[2].str().c_str()).compare_i("gametap") == 0) {
        memcpy(challengeHash, s_accountNamePassHash, sizeof(ShaDigest));
    } else {
        CryptCreateRandomSeed(
            sizeof(clientChallenge),
            (uint8_t *) &clientChallenge
        );

        CryptHashPasswordChallenge(
            clientChallenge,
            s_active->serverChallenge,
            s_accountNamePassHash,
            challengeHash
        );
    }

    ST::utf16_buffer accountName = s_accountName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_AcctLoginRequest,
        m_transId,
        clientChallenge,
        (uintptr_t) accountName.data(),
        (uintptr_t) &challengeHash,
        (uintptr_t) s_authToken,
        (uintptr_t) s_os,
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void LoginRequestTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_accountId,
        m_accountFlags,
        m_billingType,
        m_players,
        m_playerCount
    );
}

//============================================================================
bool LoginRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    uint32_t msgId = (uint32_t)*msg;
    switch (msgId) {
        case kAuth2Cli_AcctPlayerInfo: {
            const Auth2Cli_AcctPlayerInfo & reply = *(const Auth2Cli_AcctPlayerInfo *) msg;
            AddPlayer(reply.playerInt, reply.playerName, reply.avatarShape, reply.explorer);
        }
        break;

        case kAuth2Cli_AcctLoginReply: {
            const Auth2Cli_AcctLoginReply & reply = *(const Auth2Cli_AcctLoginReply *) msg;
            m_result        = reply.result;
            m_state         = kTransStateComplete;
            m_accountId     = reply.accountId;
            m_accountFlags  = reply.accountFlags;
            m_billingType   = reply.billingType;

            unsigned memSize = std::min(std::size(s_encryptionKey), std::size(reply.encryptionKey));
            memSize *= sizeof(uint32_t);
            memcpy(s_encryptionKey, reply.encryptionKey, memSize);
        }
        break;

        DEFAULT_FATAL(msgId);
    };

    return true;
}


/*****************************************************************************
*
*   AgeRequestTrans
*
***/

//============================================================================
AgeRequestTrans::AgeRequestTrans(
        const ST::string& ageName, const plUUID& ageInstId,
        FNetCliAuthAgeRequestCallback callback, void* param)
    : NetAuthTrans(kAgeRequestTrans), m_ageName(ageName), m_ageInstId(ageInstId),
      m_callback(callback), m_param(param), m_ageMcpId(), m_ageVaultId(),
      m_gameSrvNode()
{ }

//============================================================================
AgeRequestTrans::~AgeRequestTrans () {
}

//============================================================================
bool AgeRequestTrans::Send () {
    if (!AcquireConn())
        return true;

    ST::utf16_buffer ageName = m_ageName.to_utf16();

    const uintptr_t msg[] = {
                     kCli2Auth_AgeRequest,
                     m_transId,
        (uintptr_t)  ageName.data(),
        (uintptr_t) &m_ageInstId,
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void AgeRequestTrans::Post () {
    plNetAddress addr;
    addr.SetHost(hsToBE32(m_gameSrvNode));

    m_callback(
        m_result,
        m_param,
        m_ageMcpId,
        m_ageVaultId,
        m_ageInstId,
        addr
    );
}

//============================================================================
bool AgeRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AgeReply & reply = *(const Auth2Cli_AgeReply *) msg;
    m_gameSrvNode   = reply.gameSrvNode;
    m_ageMcpId      = reply.ageMcpId;
    m_ageVaultId    = reply.ageVaultId;
    m_ageInstId     = reply.ageInstId;
    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   AccountCreateRequestTrans
*
***/

//============================================================================
AccountCreateRequestTrans::AccountCreateRequestTrans (
    const char16_t                          accountName[],
    const char16_t                          password[],
    unsigned                                accountFlags,
    unsigned                                billingType,
    FNetCliAuthAccountCreateRequestCallback callback,
    void *                                  param
) : NetAuthTrans(kAccountCreateRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_accountFlags(accountFlags)
,   m_billingType(billingType)
{
    StrCopy(m_accountName, accountName, std::size(m_accountName));

    CryptHashPassword(
        ST::string::from_utf16(m_accountName),
        ST::string::from_utf16(password),
        m_namePassHash
    );
}

//============================================================================
bool AccountCreateRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctCreateRequest,
                        m_transId,
        (uintptr_t)  m_accountName,
        (uintptr_t)  &m_namePassHash,
                        m_accountFlags,
                        m_billingType,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountCreateRequestTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_accountId
    );
}

//============================================================================
bool AccountCreateRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctCreateReply & reply = *(const Auth2Cli_AcctCreateReply *) msg;

    m_result    = reply.result;
    m_accountId = reply.accountId;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   AccountCreateFromKeyRequestTrans
*
***/

//============================================================================
AccountCreateFromKeyRequestTrans::AccountCreateFromKeyRequestTrans (
    const char16_t                                  accountName[],
    const char16_t                                  password[],
    const plUUID&                                   key,
    unsigned                                        billingType,
    FNetCliAuthAccountCreateFromKeyRequestCallback  callback,
    void *                                          param
) : NetAuthTrans(kAccountCreateFromKeyRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_billingType(billingType)
,   m_key(key)
{
    StrCopy(m_accountName, accountName, std::size(m_accountName));

    CryptHashPassword(
        ST::string::from_utf16(m_accountName),
        ST::string::from_utf16(password),
        m_namePassHash
    );
}

//============================================================================
bool AccountCreateFromKeyRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctCreateFromKeyRequest,
                        m_transId,
        (uintptr_t)  m_accountName,
        (uintptr_t)  &m_namePassHash,
        (uintptr_t)  &m_key,
                        m_billingType,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountCreateFromKeyRequestTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_accountId,
        m_activationKey
    );
}

//============================================================================
bool AccountCreateFromKeyRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctCreateFromKeyReply & reply = *(const Auth2Cli_AcctCreateFromKeyReply *) msg;

    m_result        = reply.result;
    m_accountId     = reply.accountId;
    m_activationKey = reply.activationKey;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   PlayerCreateRequestTrans
*
***/

//============================================================================
PlayerCreateRequestTrans::PlayerCreateRequestTrans (
    const ST::string&                       playerName,
    const ST::string&                       avatarShape,
    const ST::string&                       friendInvite,
    FNetCliAuthPlayerCreateRequestCallback  callback,
    void *                                  param
) : NetAuthTrans(kPlayerCreateRequestTrans)
,   m_playerName(playerName)
,   m_avatarShape(avatarShape)
,   m_friendInvite(friendInvite)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool PlayerCreateRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer playerName = m_playerName.to_utf16();
    ST::utf16_buffer avatarShape = m_avatarShape.to_utf16();
    ST::utf16_buffer friendInvite = m_friendInvite.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_PlayerCreateRequest,
                        m_transId,
        (uintptr_t)     playerName.data(),
        (uintptr_t)     avatarShape.data(),
        (uintptr_t)     friendInvite.data(),
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void PlayerCreateRequestTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_playerInfo
    );
}

//============================================================================
bool PlayerCreateRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_PlayerCreateReply & reply = *(const Auth2Cli_PlayerCreateReply *) msg;
    if (!IS_NET_ERROR(reply.result)) {
        m_playerInfo.playerInt   = reply.playerInt;
        m_playerInfo.explorer    = reply.explorer;
        m_playerInfo.playerName  = ST::string::from_utf16(reply.playerName);
        m_playerInfo.avatarShape = ST::string::from_utf16(reply.avatarShape);
    }
    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   PlayerDeleteRequestTrans
*
***/

//============================================================================
PlayerDeleteRequestTrans::PlayerDeleteRequestTrans (
    unsigned                                playerId,
    FNetCliAuthPlayerDeleteRequestCallback  callback,
    void *                                  param
) : NetAuthTrans(kPlayerDeleteRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_playerId(playerId)
{
}

//============================================================================
bool PlayerDeleteRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_PlayerDeleteRequest,
                        m_transId,
                        m_playerId
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void PlayerDeleteRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool PlayerDeleteRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    
) {
    const Auth2Cli_PlayerDeleteReply & reply = *(const Auth2Cli_PlayerDeleteReply *) msg;
    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   UpgradeVisitorRequestTrans
*
***/

//============================================================================
UpgradeVisitorRequestTrans::UpgradeVisitorRequestTrans (
    unsigned                                    playerId,
    FNetCliAuthUpgradeVisitorRequestCallback    callback,
    void *                                      param
) : NetAuthTrans(kUpgradeVisitorRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_playerId(playerId)
{
}

//============================================================================
bool UpgradeVisitorRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_UpgradeVisitorRequest,
                        m_transId,
                        m_playerId
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void UpgradeVisitorRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool UpgradeVisitorRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    
) {
    const Auth2Cli_UpgradeVisitorReply & reply = *(const Auth2Cli_UpgradeVisitorReply *) msg;
    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   SetPlayerRequestTrans
*
***/

//============================================================================
SetPlayerRequestTrans::SetPlayerRequestTrans (
    unsigned                            playerInt,
    FNetCliAuthSetPlayerRequestCallback callback,
    void *                              param

) : NetAuthTrans(kSetPlayerRequestTrans)
,   m_playerInt(playerInt)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool SetPlayerRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctSetPlayerRequest,
        m_transId,
        m_playerInt,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void SetPlayerRequestTrans::Post () {
    m_callback(m_result, m_param);
}

//============================================================================
bool SetPlayerRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctSetPlayerReply & reply = *(const Auth2Cli_AcctSetPlayerReply *) msg;
    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   AccountChangePasswordRequestTrans
*
***/

//============================================================================
AccountChangePasswordRequestTrans::AccountChangePasswordRequestTrans (
    const ST::string&                               accountName,
    const ST::string&                               password,
    FNetCliAuthAccountChangePasswordRequestCallback callback,
    void *                                          param
) : NetAuthTrans(kAccountChangePasswordRequestTrans)
,   m_accountName(accountName)
,   m_callback(callback)
,   m_param(param)
{
    CryptHashPassword(
        m_accountName,
        password,
        m_namePassHash
    );
}

//============================================================================
bool AccountChangePasswordRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer accountName = m_accountName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_AcctChangePasswordRequest,
                        m_transId,
        (uintptr_t)  accountName.data(),
        (uintptr_t)  &m_namePassHash,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountChangePasswordRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool AccountChangePasswordRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctChangePasswordReply & reply = *(const Auth2Cli_AcctChangePasswordReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   GetPublicAgeListTrans
*
***/

//============================================================================
GetPublicAgeListTrans::GetPublicAgeListTrans (
    const ST::string&                   ageName,
    FNetCliAuthGetPublicAgeListCallback callback,
    void *                              param
) : NetAuthTrans(kGetPublicAgeListTrans)
,   m_ageName(ageName)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool GetPublicAgeListTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer ageName = m_ageName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_GetPublicAgeList,
                        m_transId,
        (uintptr_t)     ageName.data(),
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void GetPublicAgeListTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_ages
    );
}

//============================================================================
bool GetPublicAgeListTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_PublicAgeList & reply = *(const Auth2Cli_PublicAgeList *) msg;
    
    if (IS_NET_SUCCESS(reply.result))
        m_ages.assign(reply.ages, reply.ages + reply.ageCount);

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   AccountSetRolesRequestTrans
*
***/

//============================================================================
AccountSetRolesRequestTrans::AccountSetRolesRequestTrans (
    const char16_t                              accountName[],
    unsigned                                    accountFlags,
    FNetCliAuthAccountSetRolesRequestCallback   callback,
    void *                                      param
) : NetAuthTrans(kAccountSetRolesRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_accountFlags(accountFlags)
{
    StrCopy(m_accountName, accountName, std::size(m_accountName));
}

//============================================================================
bool AccountSetRolesRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctSetRolesRequest,
                        m_transId,
        (uintptr_t)  m_accountName,
                        m_accountFlags,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountSetRolesRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool AccountSetRolesRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctSetRolesReply & reply = *(const Auth2Cli_AcctSetRolesReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   AccountSetBillingTypeRequestTrans
*
***/

//============================================================================
AccountSetBillingTypeRequestTrans::AccountSetBillingTypeRequestTrans (
    const char16_t                                  accountName[],
    unsigned                                        billingType,
    FNetCliAuthAccountSetBillingTypeRequestCallback callback,
    void *                                          param
) : NetAuthTrans(kAccountSetBillingTypeRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_billingType(billingType)
{
    StrCopy(m_accountName, accountName, std::size(m_accountName));
}

//============================================================================
bool AccountSetBillingTypeRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctSetBillingTypeRequest,
                        m_transId,
        (uintptr_t)  m_accountName,
                        m_billingType,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountSetBillingTypeRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool AccountSetBillingTypeRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctSetBillingTypeReply & reply = *(const Auth2Cli_AcctSetBillingTypeReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   AccountActivateRequestTrans
*
***/

//============================================================================
AccountActivateRequestTrans::AccountActivateRequestTrans (
    const plUUID&                               activationKey,
    FNetCliAuthAccountActivateRequestCallback   callback,
    void *                                      param
) : NetAuthTrans(kAccountActivateRequestTrans)
,   m_callback(callback)
,   m_param(param)
{
    m_activationKey = activationKey;
}

//============================================================================
bool AccountActivateRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_AcctActivateRequest,
                        m_transId,
        (uintptr_t)  &m_activationKey,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void AccountActivateRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool AccountActivateRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_AcctActivateReply & reply = *(const Auth2Cli_AcctActivateReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   FileListRequestTrans
*
***/

//============================================================================
FileListRequestTrans::FileListRequestTrans (
    FNetCliAuthFileListRequestCallback  callback,
    void *                              param,
    const char16_t                      directory[],
    const char16_t                      ext[]
) : NetAuthTrans(kFileListRequestTrans)
,   m_callback(callback)
,   m_param(param)
{
    StrCopy(m_directory, directory, std::size(m_directory));
    StrCopy(m_ext, ext, std::size(m_ext));
}

//============================================================================
bool FileListRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_FileListRequest,
        m_transId,
        (uintptr_t) m_directory,
        (uintptr_t) m_ext,
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void FileListRequestTrans::Post () {
    m_callback(m_result, m_param, m_fileInfoArray.data(), m_fileInfoArray.size());
}

//============================================================================
bool FileListRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_FileListReply & reply = *(const Auth2Cli_FileListReply *) msg;

    uint32_t wcharCount = reply.wcharCount;
    const char16_t* curChar = reply.fileData;
    // if wcharCount is 2, the data only contains the terminator "\0\0" and we
    // don't need to convert anything
    if (wcharCount == 2)
        m_fileInfoArray.clear();
    else
    {
        // fileData format: "filename\0size\0filename\0size\0...\0\0"
        bool done = false;
        while (!done) {
            if (wcharCount == 0)
            {
                done = true;
                break;
            }

            // read in the filename
            char16_t filename[kCli2File_FilenameSize];
            StrCopy(filename, curChar, std::size(filename));
            filename[std::size(filename) - 1] = L'\0'; // make sure it's terminated

            unsigned filenameLen = std::char_traits<char16_t>::length(filename);
            curChar += filenameLen; // advance the pointer
            wcharCount -= filenameLen; // keep track of the amount remaining
            if ((*curChar != L'\0') || (wcharCount <= 0))
                return false; // something is screwy, abort and disconnect

            curChar++; // point it at the first part of the size value (format: 0xHHHHLLLL)
            wcharCount--;
            if (wcharCount < 4) // we have to have 2 chars for the size, and 2 for terminator at least
                return false; // screwy data
            unsigned size = ((*curChar) << 16) + (*(curChar + 1));
            curChar += 2;
            wcharCount -= 2;
            if ((*curChar != L'\0') || (wcharCount <= 0))
                return false; // screwy data

            // save the data in our array
            NetCliAuthFileInfo& info = m_fileInfoArray.emplace_back();
            StrCopy(info.filename, filename, std::size(info.filename));
            info.filesize = size;

            // point it at either the second part of the terminator, or the next filename
            curChar++;
            wcharCount--;
            if (*curChar == L'\0')
            {
                // we hit the terminator
                if (wcharCount != 1)
                    return false; // invalid data, we shouldn't have any more
                done = true; // we're done
            }
            else if (wcharCount < 6) // we must have at least a 1 char string, '\0', size, and "\0\0" terminator left
                return false; // screwy data
        }
    }

    m_result    = reply.result;
    m_state     = kTransStateComplete;

    return true;
}

/*****************************************************************************
*
*   FileDownloadRequestTrans
*
***/

//============================================================================
FileDownloadRequestTrans::FileDownloadRequestTrans (
    FNetCliAuthFileRequestCallback  callback,
    void *                          param,
    const plFileName &              filename,
    hsStream *                      writer
) : NetAuthTrans(kFileDownloadRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_filename(filename)
,   m_writer(writer)
{
    // This transaction issues "sub transactions" which must complete
    // before this one even though they were issued after us.
    m_hasSubTrans = true;
}

//============================================================================
bool FileDownloadRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    char16_t filename[kCli2File_FilenameSize] {};
    const ST::utf16_buffer buffer = m_filename.AsString().to_utf16();
    memcpy(filename, buffer.data(), std::min(sizeof(filename), buffer.size() * sizeof(char16_t)));

    const uintptr_t msg[] = {
        kCli2Auth_FileDownloadRequest,
        m_transId,
        reinterpret_cast<uintptr_t>(filename),
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void FileDownloadRequestTrans::Post () {
    m_callback(m_result, m_param, m_filename, m_writer);
}

//============================================================================
bool FileDownloadRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_FileDownloadChunk & reply = *(const Auth2Cli_FileDownloadChunk *) msg;

    if (IS_NET_ERROR(reply.result)) {
        // we have a problem... indicate we are done and abort
        m_result    = reply.result;
        m_state     = kTransStateComplete;
        return true;
    }

    // we have data to write, so queue it for write in the main thread (we're
    // currently in a net recv thread)
    if (reply.chunkSize) {
        RcvdFileDownloadChunkTrans * writeTrans = new RcvdFileDownloadChunkTrans;
        writeTrans->writer  = m_writer;
        writeTrans->bytes   = reply.chunkSize;
        writeTrans->offset  = reply.chunkOffset;
        writeTrans->data    = (uint8_t *)malloc(reply.chunkSize);
        memcpy(writeTrans->data, reply.chunkData, reply.chunkSize);
        NetTransSend(writeTrans);
    }

    if (reply.chunkOffset + reply.chunkSize >= reply.fileSize) {
        // all bytes received, mark as complete
        m_result    = reply.result;
        m_state     = kTransStateComplete;
    }
    
    // Ack the data
    const uintptr_t ack[] = {
        kCli2Auth_FileDownloadChunkAck,
        m_transId
    };
    m_conn->Send(ack, std::size(ack));
    
    return true;
}


/*****************************************************************************
*
*   RcvdFileDownloadChunkTrans
*
***/

//============================================================================
RcvdFileDownloadChunkTrans::~RcvdFileDownloadChunkTrans () {
    free(data);
}

//============================================================================
void RcvdFileDownloadChunkTrans::Post () {
    
    if (writer->GetPosition() != offset)
        writer->SetPosition(offset);
    
    writer->Write(bytes, data);
    m_result = kNetSuccess;
    m_state  = kTransStateComplete;
}


/*****************************************************************************
*
*   RcvdPropagatedBufferTrans
*
***/

//============================================================================
RcvdPropagatedBufferTrans::~RcvdPropagatedBufferTrans () {
    free(bufferData);
}

//============================================================================
void RcvdPropagatedBufferTrans::Post () {
    if (s_bufRcvdCb)
        s_bufRcvdCb(bufferType, bufferBytes, bufferData);
}


/*****************************************************************************
*
*   VaultNodeChangedTrans
*
***/

//============================================================================
void VaultNodeChangedTrans::Post () {
    if (s_vaultNodeChangedHandler)
        s_vaultNodeChangedHandler(m_nodeId, m_revId);
}


/*****************************************************************************
*
*   VaultNodeAddedTrans
*
***/

//============================================================================
void VaultNodeAddedTrans::Post () {
    if (s_vaultNodeAddedHandler)
        s_vaultNodeAddedHandler(m_parentId, m_childId, m_ownerId);
}


/*****************************************************************************
*
*   VaultNodeRemovedTrans
*
***/

//============================================================================
void VaultNodeRemovedTrans::Post () {
    if (s_vaultNodeRemovedHandler)
        s_vaultNodeRemovedHandler(m_parentId, m_childId);
}


/*****************************************************************************
*
*   VaultNodeDeletedTrans
*
***/

//============================================================================
void VaultNodeDeletedTrans::Post () {
    if (s_vaultNodeDeletedHandler)
        s_vaultNodeDeletedHandler(m_nodeId);
}


/*****************************************************************************
*
*   VaultFetchNodeRefsTrans
*
***/

//============================================================================
VaultFetchNodeRefsTrans::VaultFetchNodeRefsTrans (
    unsigned                        nodeId,
    FNetCliAuthVaultNodeRefsFetched callback,
    void *                          param
) : NetAuthTrans(kVaultFetchNodeRefsTrans)
,   m_nodeId(nodeId)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool VaultFetchNodeRefsTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_VaultFetchNodeRefs,
        m_transId,
        m_nodeId,
    };
            
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void VaultFetchNodeRefsTrans::Post () {
    if (m_callback)
        m_callback(
            m_result,
            m_param,
            m_refs.data(),
            m_refs.size()
        );
}

//============================================================================
bool VaultFetchNodeRefsTrans::Recv (
    const uint8_t  msg[],
    unsigned
) {
    const Auth2Cli_VaultNodeRefsFetched & reply = *(const Auth2Cli_VaultNodeRefsFetched *) msg;

    if (IS_NET_SUCCESS(reply.result))
        m_refs.assign(reply.refs, reply.refs + reply.refCount);

    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}


/*****************************************************************************
*
*   VaultInitAgeTrans
*
***/

//============================================================================
VaultInitAgeTrans::VaultInitAgeTrans (
    FNetCliAuthAgeInitCallback  callback,           // optional
    void *                      param,              // optional
    const plUUID&               ageInstId,          // optional. is used in match
    const plUUID&               parentAgeInstId,    // optional. is used in match
    const ST::string&           ageFilename,      // optional. is used in match
    const ST::string&           ageInstName,      // optional. not used in match
    const ST::string&           ageUserName,      // optional. not used in match
    const ST::string&           ageDesc,          // optional. not used in match
    unsigned                    ageSequenceNumber,  // optional. not used in match
    unsigned                    ageLanguage         // optional. not used in match
) : NetAuthTrans(kVaultInitAgeTrans)
,   m_callback(callback)
,   m_param(param)
,   m_ageInstId(ageInstId)
,   m_parentAgeInstId(parentAgeInstId)
,   m_ageFilename(ageFilename)
,   m_ageInstName(ageInstName)
,   m_ageUserName(ageUserName)
,   m_ageDesc(ageDesc)
,   m_ageSequenceNumber(ageSequenceNumber)
,   m_ageLanguage(ageLanguage)
,   m_ageId(0)
,   m_ageInfoId(0)
{
}

//============================================================================
VaultInitAgeTrans::~VaultInitAgeTrans () {
}

//============================================================================
bool VaultInitAgeTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer ageFilename = m_ageFilename.to_utf16();
    ST::utf16_buffer ageInstName = m_ageInstName.to_utf16();
    ST::utf16_buffer ageUserName = m_ageUserName.to_utf16();
    ST::utf16_buffer ageDesc = m_ageDesc.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_VaultInitAgeRequest,
                        m_transId,
        (uintptr_t) &m_ageInstId,
        (uintptr_t) &m_parentAgeInstId,
        (uintptr_t)  ageFilename.data(),
        (uintptr_t)  ageInstName.data(),
        (uintptr_t)  ageUserName.data(),
        (uintptr_t)  ageDesc.data(),
                     m_ageSequenceNumber,
                     m_ageLanguage,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void VaultInitAgeTrans::Post () {
    if (m_callback)
        m_callback(
            m_result,
            m_param,
            m_ageId,
            m_ageInfoId
        );
}

//============================================================================
bool VaultInitAgeTrans::Recv (
    const uint8_t  msg[],
    unsigned    
) {
    const Auth2Cli_VaultInitAgeReply & reply = *(const Auth2Cli_VaultInitAgeReply *) msg;
    
    m_ageId     = reply.ageVaultId;
    m_ageInfoId = reply.ageInfoVaultId;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    
    return true;
}


/*****************************************************************************
*
*   VaultFetchNodeTrans
*
***/

//============================================================================
VaultFetchNodeTrans::VaultFetchNodeTrans (
    unsigned                    nodeId,
    FNetCliAuthVaultNodeFetched callback,
    void *                      param
) : NetAuthTrans(kVaultFetchNodeTrans)
,   m_nodeId(nodeId)
,   m_callback(callback)
,   m_param(param)
,   m_node()
{
}

//============================================================================
bool VaultFetchNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_VaultNodeFetch,
        m_transId,
        m_nodeId,
    };
            
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void VaultFetchNodeTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_node.Get()
    );
}

//============================================================================
bool VaultFetchNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultNodeFetched & reply = *(const Auth2Cli_VaultNodeFetched *) msg;
    
    if (IS_NET_SUCCESS(reply.result)) {
        m_node.Steal(new NetVaultNode);
        m_node->Read(reply.nodeBuffer, reply.nodeBytes);
    }

    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}


/*****************************************************************************
*
*   VaultFindNodeTrans
*
***/

//============================================================================
VaultFindNodeTrans::VaultFindNodeTrans (
    NetVaultNode *              templateNode,
    FNetCliAuthVaultNodeFind    callback,
    void *                      param
) : NetAuthTrans(kVaultFindNodeTrans)
,   m_callback(callback)
,   m_param(param)
{
    templateNode->Write(&m_buffer, 0);
}

//============================================================================
bool VaultFindNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
            kCli2Auth_VaultNodeFind,
            m_transId,
            m_buffer.size(),
            (uintptr_t)m_buffer.data(),
    };
    m_conn->Send(msg, std::size(msg));
    return true;
}

//============================================================================
void VaultFindNodeTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_nodeIds.size(),
        m_nodeIds.data()
    );
}

//============================================================================
bool VaultFindNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultNodeFindReply & reply = *(const Auth2Cli_VaultNodeFindReply *) msg;

    if (IS_NET_SUCCESS(reply.result)) {
        static_assert(sizeof(unsigned) == sizeof(uint32_t), "unsigned is not the same size as uint32_t");
        m_nodeIds.assign((unsigned *)reply.nodeIds, (unsigned *)reply.nodeIds + reply.nodeIdCount);
    }

    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}


/*****************************************************************************
*
*   VaultCreateNodeTrans
*
***/

//============================================================================
VaultCreateNodeTrans::VaultCreateNodeTrans (
    NetVaultNode *                  templateNode,
    FNetCliAuthVaultNodeCreated     callback,
    void *                          param
) : NetAuthTrans(kVaultCreateNodeTrans)
,   m_callback(callback)
,   m_param(param)
,   m_nodeId(0)
{
    templateNode->Write(&m_buffer, 0);
}

//============================================================================
bool VaultCreateNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
            kCli2Auth_VaultNodeCreate,
            m_transId,
            m_buffer.size(),
            (uintptr_t)m_buffer.data()
    };
    m_conn->Send(msg, std::size(msg));
    return true;
}

//============================================================================
void VaultCreateNodeTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_nodeId
    );
}

//============================================================================
bool VaultCreateNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultNodeCreated & reply = *(const Auth2Cli_VaultNodeCreated *) msg;
    
    if (IS_NET_SUCCESS(reply.result))
        m_nodeId = reply.nodeId;

    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}

/*****************************************************************************
*
*   VaultSaveNodeTrans
*
***/

//============================================================================
VaultSaveNodeTrans::VaultSaveNodeTrans (
    unsigned                            nodeId,
    const plUUID&                       revisionId,
    unsigned                            dataCount,
    const void *                        data,
    FNetCliAuthVaultNodeSaveCallback    callback,
    void *                              param
) : NetAuthTrans(kVaultSaveNodeTrans)
,   m_nodeId(nodeId)
,   m_revisionId(revisionId)
,   m_buffer((const uint8_t *)data, (const uint8_t *)data + dataCount)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool VaultSaveNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_VaultNodeSave,
                        m_transId,
                        m_nodeId,
        (uintptr_t)  &m_revisionId,
                        m_buffer.size(),
        (uintptr_t)  m_buffer.data(),
    };
            
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void VaultSaveNodeTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool VaultSaveNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultSaveNodeReply & reply = *(const Auth2Cli_VaultSaveNodeReply *) msg;
    
    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}

/*****************************************************************************
*
*   VaultAddNodeTrans
*
***/

//============================================================================
VaultAddNodeTrans::VaultAddNodeTrans (
    unsigned                        parentId,
    unsigned                        childId,
    unsigned                        ownerId,
    FNetCliAuthVaultNodeAddCallback callback,
    void *                          param
) : NetAuthTrans(kVaultAddNodeTrans)
,   m_parentId(parentId)
,   m_childId(childId)
,   m_ownerId(ownerId)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool VaultAddNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_VaultNodeAdd,
                    m_transId,
                    m_parentId,
                    m_childId,
                    m_ownerId,
    };
            
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void VaultAddNodeTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool VaultAddNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultAddNodeReply & reply = *(const Auth2Cli_VaultAddNodeReply *) msg;
    
    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}

/*****************************************************************************
*
*   VaultRemoveNodeTrans
*
***/

//============================================================================
VaultRemoveNodeTrans::VaultRemoveNodeTrans (
    unsigned                            parentId,
    unsigned                            childId,
    FNetCliAuthVaultNodeRemoveCallback  callback,
    void *                              param
) : NetAuthTrans(kVaultRemoveNodeTrans)
,   m_parentId(parentId)
,   m_childId(childId)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool VaultRemoveNodeTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_VaultNodeRemove,
                    m_transId,
                    m_parentId,
                    m_childId,
    };
            
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void VaultRemoveNodeTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool VaultRemoveNodeTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_VaultRemoveNodeReply & reply = *(const Auth2Cli_VaultRemoveNodeReply *) msg;
    
    m_result = reply.result;
    m_state  = kTransStateComplete;
    
    return true;
}

/*****************************************************************************
*
*   NotifyNewBuildTrans
*
***/

//============================================================================
void NotifyNewBuildTrans::Post () {

    if (s_notifyNewBuildHandler)
        s_notifyNewBuildHandler();
}

/*****************************************************************************
*
*   SetPlayerBanStatusRequestTrans
*
***/

//============================================================================
SetPlayerBanStatusRequestTrans::SetPlayerBanStatusRequestTrans (
    unsigned                                        playerId,
    unsigned                                        banned,
    FNetCliAuthSetPlayerBanStatusRequestCallback    callback,
    void *                                          param
) : NetAuthTrans(kSetPlayerBanStatusRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_playerId(playerId)
,   m_banned(banned)
{
}

//============================================================================
bool SetPlayerBanStatusRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_SetPlayerBanStatusRequest,
                        m_transId,
                        m_playerId,
                        m_banned,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void SetPlayerBanStatusRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool SetPlayerBanStatusRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_SetPlayerBanStatusReply & reply = *(const Auth2Cli_SetPlayerBanStatusReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ChangePlayerNameRequestTrans
*
***/

//============================================================================
ChangePlayerNameRequestTrans::ChangePlayerNameRequestTrans (
    unsigned                                    playerId,
    const char16_t                              newName[],
    FNetCliAuthChangePlayerNameRequestCallback  callback,
    void *                                      param
) : NetAuthTrans(kChangePlayerNameRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_playerId(playerId)
{
    StrCopy(m_newName, newName, std::size(m_newName));
}

//============================================================================
bool ChangePlayerNameRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_ChangePlayerNameRequest,
                        m_transId,
                        m_playerId,
        (uintptr_t)  m_newName,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ChangePlayerNameRequestTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool ChangePlayerNameRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ChangePlayerNameReply & reply = *(const Auth2Cli_ChangePlayerNameReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   SendFriendInviteTrans
*
***/

//============================================================================
SendFriendInviteTrans::SendFriendInviteTrans (
    const ST::string&                       emailAddr,
    const ST::string&                       toName,
    const plUUID&                           inviteUuid,
    FNetCliAuthSendFriendInviteCallback     callback,
    void *                                  param
) : NetAuthTrans(kSendFriendInviteTrans)
,   m_callback(callback)
,   m_param(param)
,   m_inviteUuid(inviteUuid)
,   m_toName(toName)
,   m_emailAddress(emailAddr)
{
}

//============================================================================
bool SendFriendInviteTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer emailAddress = m_emailAddress.to_utf16();
    ST::utf16_buffer toName = m_toName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_SendFriendInviteRequest,
                        m_transId,
        (uintptr_t) &m_inviteUuid,
        (uintptr_t)  emailAddress.data(),
        (uintptr_t)  toName.data(),
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void SendFriendInviteTrans::Post () {
    m_callback(
        m_result,
        m_param
    );
}

//============================================================================
bool SendFriendInviteTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_SendFriendInviteReply & reply = *(const Auth2Cli_SendFriendInviteReply *) msg;

    m_result    = reply.result;
    m_state     = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   AuthConnectedNotifyTrans
*
***/

//============================================================================
void AuthConnectedNotifyTrans::Post() {
    if (s_connectedCb != nullptr)
        s_connectedCb();
}


/*****************************************************************************
*
*   ScoreCreateTrans
*
***/

//============================================================================
ScoreCreateTrans::ScoreCreateTrans (
    unsigned                        ownerId,
    const ST::string&               gameName,
    unsigned                        gameType,
    int                             value,
    FNetCliAuthCreateScoreCallback  callback,
    void *                          param
) : NetAuthTrans(kScoreCreateTrans)
,   m_callback(callback)
,   m_param(param)
,   m_ownerId(ownerId)
,   m_gameName(gameName)
,   m_gameType(gameType)
,   m_value(value)
,   m_scoreId(0)
,   m_createdTime(0)
{
}

//============================================================================
bool ScoreCreateTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer gameName = m_gameName.to_utf16();

    const uintptr_t msg[] = {
                        kCli2Auth_ScoreCreate,
                        m_transId,
                        m_ownerId,
           (uintptr_t)  gameName.data(),
                        m_gameType,
           (uintptr_t)  m_value
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreCreateTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param,
            m_scoreId,
            m_createdTime,
            m_ownerId,
            m_gameName,
            m_gameType,
            m_value
        );
    }
}

//============================================================================
bool ScoreCreateTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreCreateReply & reply = *(const Auth2Cli_ScoreCreateReply *) msg;

    m_scoreId       = reply.scoreId;
    m_createdTime   = reply.createdTime;

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreDeleteTrans
*
***/

//============================================================================
ScoreDeleteTrans::ScoreDeleteTrans (
    unsigned                        scoreId,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) : NetAuthTrans(kScoreDeleteTrans)
,   m_callback(callback)
,   m_param(param)
,   m_scoreId(scoreId)
{
}

//============================================================================
bool ScoreDeleteTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_ScoreDelete,
                        m_transId,
                        m_scoreId,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreDeleteTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool ScoreDeleteTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreDeleteReply & reply = *(const Auth2Cli_ScoreDeleteReply *) msg;

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreGetScoresTrans
*
***/

//============================================================================
ScoreGetScoresTrans::ScoreGetScoresTrans (
    unsigned                        ownerId,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback,
    void *                          param
) : NetAuthTrans(kScoreGetScoresTrans)
,   m_callback(callback)
,   m_param(param)
,   m_ownerId(ownerId)
,   m_gameName(gameName)
,   m_scores(nullptr)
,   m_scoreCount(0)
{
}

//============================================================================
ScoreGetScoresTrans::~ScoreGetScoresTrans () {
    delete[] m_scores;
}

//============================================================================
bool ScoreGetScoresTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer gameName = m_gameName.to_utf16();

    const uintptr_t msg[] = {
                    kCli2Auth_ScoreGetScores,
                    m_transId,
                    m_ownerId,
        (uintptr_t) gameName.data()
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreGetScoresTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param,
            m_scores,
            m_scoreCount
        );
    }
}

//============================================================================
bool ScoreGetScoresTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreGetScoresReply & reply = *(const Auth2Cli_ScoreGetScoresReply *) msg;

    if (reply.scoreCount > 0) {
        m_scoreCount    = reply.scoreCount;
        m_scores        = new NetGameScore[m_scoreCount];

        uint8_t*       bufferPos = const_cast<uint8_t*>(reply.buffer);
        unsigned    bufferLength = reply.byteCount;

        for (unsigned i = 0; i < m_scoreCount; ++i) {
            bufferLength -= m_scores[i].Read(bufferPos, bufferLength, &bufferPos);
        }
    }
    else {
        m_scoreCount    = 0;
        m_scores        = nullptr;
    }

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreAddPointsTrans
*
***/

//============================================================================
ScoreAddPointsTrans::ScoreAddPointsTrans (
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) : NetAuthTrans(kScoreAddPointsTrans)
,   m_callback(callback)
,   m_param(param)
,   m_scoreId(scoreId)
,   m_numPoints(numPoints)
{
}

//============================================================================
bool ScoreAddPointsTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_ScoreAddPoints,
                        m_transId,
                        m_scoreId,
                        (uintptr_t)m_numPoints,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreAddPointsTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool ScoreAddPointsTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreAddPointsReply & reply = *(const Auth2Cli_ScoreAddPointsReply *) msg;

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreTransferPointsTrans
*
***/

//============================================================================
ScoreTransferPointsTrans::ScoreTransferPointsTrans (
    unsigned                        srcScoreId,
    unsigned                        destScoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) : NetAuthTrans(kScoreTransferPointsTrans)
,   m_callback(callback)
,   m_param(param)
,   m_srcScoreId(srcScoreId)
,   m_destScoreId(destScoreId)
,   m_numPoints(numPoints)
{
}

//============================================================================
bool ScoreTransferPointsTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_ScoreTransferPoints,
                        m_transId,
                        m_srcScoreId,
                        m_destScoreId,
                        (uintptr_t)m_numPoints,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreTransferPointsTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool ScoreTransferPointsTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreTransferPointsReply & reply = *(const Auth2Cli_ScoreTransferPointsReply *) msg;

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreSetPointsTrans
*
***/

//============================================================================
ScoreSetPointsTrans::ScoreSetPointsTrans (
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) : NetAuthTrans(kScoreSetPointsTrans)
,   m_callback(callback)
,   m_param(param)
,   m_scoreId(scoreId)
,   m_numPoints(numPoints)
{
}

//============================================================================
bool ScoreSetPointsTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Auth_ScoreSetPoints,
                        m_transId,
                        m_scoreId,
                        (uintptr_t)m_numPoints,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreSetPointsTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param
        );
    }
}

//============================================================================
bool ScoreSetPointsTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreSetPointsReply & reply = *(const Auth2Cli_ScoreSetPointsReply *) msg;

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}


/*****************************************************************************
*
*   ScoreGetRanksTrans
*
***/

//============================================================================
ScoreGetRanksTrans::ScoreGetRanksTrans(
        unsigned ownerId, unsigned scoreGroup, unsigned parentFolderId,
        const ST::string& gameName, unsigned timePeriod, unsigned numResults,
        unsigned pageNumber, bool sortDesc, FNetCliAuthGetRanksCallback callback,
        void* param)
    : NetAuthTrans(kScoreGetRanksTrans), m_callback(callback), m_param(param),
      m_ownerId(ownerId),  m_scoreGroup(scoreGroup), m_parentFolderId(parentFolderId),
      m_gameName(gameName), m_timePeriod(timePeriod), m_numResults(numResults),
      m_pageNumber(pageNumber), m_sortDesc(sortDesc), m_ranks(), m_rankCount()
{ }

//============================================================================
bool ScoreGetRanksTrans::Send () {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer gameName = m_gameName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_ScoreGetRanks,
                        m_transId,
                        m_ownerId,
                        m_scoreGroup,
                        m_parentFolderId,
        (uintptr_t)     gameName.data(),
                        m_timePeriod,
                        m_numResults,
                        m_pageNumber,
                        m_sortDesc,
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreGetRanksTrans::Post () {
    if (m_callback) {
        m_callback(
            m_result,
            m_param,
            m_ranks,
            m_rankCount
        );
    }
}

//============================================================================
bool ScoreGetRanksTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Auth2Cli_ScoreGetRanksReply & reply = *(const Auth2Cli_ScoreGetRanksReply *) msg;

    if (reply.rankCount > 0) {
        m_rankCount = reply.rankCount;
        m_ranks     = new NetGameRank[m_rankCount];

        uint8_t*       bufferPos = const_cast<uint8_t*>(reply.buffer);
        unsigned    bufferLength = reply.byteCount;

        for (unsigned i = 0; i < m_rankCount; ++i) {
            bufferLength -= m_ranks[i].Read(bufferPos, bufferLength, &bufferPos);
        }
    }
    else {
        m_rankCount = 0;
        m_ranks     = nullptr;
    }

    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   ScoreGetHighScoresTrans
*
***/

//============================================================================
ScoreGetHighScoresTrans::ScoreGetHighScoresTrans(
    unsigned                        ageId,
    unsigned                        maxScores,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback,
    void *                          param
    ) : NetAuthTrans(kScoreGetHighScoresTrans)
    , m_callback(callback)
    , m_param(param)
    , m_ageId(ageId)
    , m_maxScores(maxScores)
    , m_gameName(gameName)
    , m_scores(nullptr)
    , m_scoreCount(0)
{
}

//============================================================================
ScoreGetHighScoresTrans::~ScoreGetHighScoresTrans() {
    delete[] m_scores;
}

//============================================================================
bool ScoreGetHighScoresTrans::Send() {
    if (!AcquireConn())
        return false;

    ST::utf16_buffer gameName = m_gameName.to_utf16();

    const uintptr_t msg[] = {
        kCli2Auth_ScoreGetHighScores,
        m_transId,
        m_ageId,
        m_maxScores,
        (uintptr_t)gameName.data()
    };

    m_conn->Send(msg, std::size(msg));

    return true;
}

//============================================================================
void ScoreGetHighScoresTrans::Post() {
    if (m_callback) {
        m_callback(
            m_result,
            m_param,
            m_scores,
            m_scoreCount
            );
    }
}

//============================================================================
bool ScoreGetHighScoresTrans::Recv(
    const uint8_t msg[],
    unsigned      bytes
    ) {
    const Auth2Cli_ScoreGetHighScoresReply & reply = *(const Auth2Cli_ScoreGetHighScoresReply *)msg;

    if (reply.scoreCount > 0) {
        m_scoreCount = reply.scoreCount;
        m_scores = new NetGameScore[m_scoreCount];

        uint8_t*    bufferPos = const_cast<uint8_t*>(reply.buffer);
        unsigned    bufferLength = reply.byteCount;

        for (unsigned i = 0; i < m_scoreCount; ++i) {
            bufferLength -= m_scores[i].Read(bufferPos, bufferLength, &bufferPos);
        }
    }
    else {
        m_scoreCount = 0;
        m_scores = nullptr;
    }

    m_result = reply.result;
    m_state = kTransStateComplete;
    return true;
}

} using namespace Auth;


/*****************************************************************************
*
*   NetAuthTrans
*
***/

//============================================================================
NetAuthTrans::NetAuthTrans (ETransType transType)
:   NetTrans(kNetProtocolCli2Auth, transType)
,   m_conn()
{
}

//============================================================================
NetAuthTrans::~NetAuthTrans () {
    ReleaseConn();
}

//============================================================================
bool NetAuthTrans::AcquireConn () {
    if (!m_conn)
        m_conn = GetConnIncRef("AcquireConn");
    return m_conn != nullptr;
}

//============================================================================
void NetAuthTrans::ReleaseConn () {
    if (m_conn) {
        m_conn->UnRef("AcquireConn");
        m_conn = nullptr;
    }
}


/*****************************************************************************
*
*   Protected functions
*
***/

//============================================================================
void AuthInitialize () {
    s_running = true;
    NetMsgProtocolRegister(
        kNetProtocolCli2Auth,
        false,
        s_send, std::size(s_send),
        s_recv, std::size(s_recv),
        kAuthDhGValue,
        plBigNum(sizeof(kAuthDhXData), kAuthDhXData),
        plBigNum(sizeof(kAuthDhNData), kAuthDhNData)
    );
}

//============================================================================
void AuthDestroy (bool wait) {
    s_running = false;
    
    s_bufRcvdCb = nullptr;
    s_connectedCb = nullptr;
    s_vaultNodeChangedHandler   = nullptr;
    s_vaultNodeAddedHandler     = nullptr;
    s_vaultNodeRemovedHandler   = nullptr;
    s_vaultNodeDeletedHandler   = nullptr;
    s_notifyNewBuildHandler     = nullptr;
    

    NetTransCancelByProtocol(
        kNetProtocolCli2Auth,
        kNetErrRemoteShutdown
    );    
    NetMsgProtocolDestroy(
        kNetProtocolCli2Auth,
        false
    );

    {
        hsLockGuard(s_critsect);
        while (CliAuConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_active = nullptr;
    }

    if (!wait)
        return;
        
    while (s_perf[kPerfConnCount]) {
        NetTransUpdate();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//============================================================================
bool AuthQueryConnected () {
    hsLockGuard(s_critsect);
    return (s_active && s_active->cli);
}

//============================================================================
unsigned AuthGetConnId () {
    hsLockGuard(s_critsect);
    return (s_active) ? s_active->seq : 0;
}

//============================================================================
void AuthPingEnable (bool enable) {
    s_perf[kPingDisabled] = !enable;
    hsLockGuard(s_critsect);
    for (;;) {
        if (!s_active)
            break;
        if (enable)
            s_active->AutoPing();
        else
            s_active->StopAutoPing();
        break;
    }
}


} using namespace Ngl;


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliAuthStartConnect (
    const ST::string  authAddrList[],
    uint32_t          authAddrCount
) {
    // TEMP: Only connect to one auth server until we fill out this module
    // to choose the "best" auth connection.
    authAddrCount = std::min(authAddrCount, 1u);

    for (unsigned i = 0; i < authAddrCount; ++i) {
        // Do we need to lookup the address?
        const char* name = authAddrList[i].c_str();
        while (unsigned ch = *name) {
            ++name;
            if (!(isdigit(ch) || ch == L'.' || ch == L':')) {
                AsyncAddressLookupName(
                    AsyncLookupCallback,
                    authAddrList[i],
                    GetClientPort(),
                    nullptr
                );
                break;
            }
        }
        if (!name[0]) {
            plNetAddress addr(authAddrList[i], GetClientPort());
            Connect(authAddrList[i], addr);
        }
    }
}

//============================================================================
bool NetCliAuthQueryConnected () {

    return AuthQueryConnected();
}

//============================================================================
void NetCliAuthAutoReconnectEnable (bool enable) {

    s_perf[kAutoReconnectDisabled] = !enable;
}

//============================================================================
void NetCliAuthDisconnect () {

    hsLockGuard(s_critsect);
    while (CliAuConn * conn = s_conns.Head())
        UnlinkAndAbandonConn_CS(conn);
    s_active = nullptr;
}

//============================================================================
void NetCliAuthUnexpectedDisconnect () {
    hsLockGuard(s_critsect);
    if (s_active && s_active->sock)
        AsyncSocketDisconnect(s_active->sock, true);
}

//============================================================================
void NetCliAuthSetConnectCallback (
    FNetCliAuthConnectCallback  callback
) {
    s_connectedCb = callback;
}

//============================================================================
bool NetCliAuthCheckCap (uint32_t cap) {
    hsLockGuard(s_critsect);
    return s_active ? s_active->caps.IsBitSet(cap) : false;
}

//============================================================================
void NetCliAuthPingRequest (
    unsigned                        pingTimeMs,
    unsigned                        payloadBytes,
    const void *                    payload,
    FNetCliAuthPingRequestCallback  callback,
    void *                          param
) {
    PingRequestTrans * trans = new PingRequestTrans(
        callback,
        param,
        pingTimeMs,
        payloadBytes,
        payload
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountExistsRequest (
    const char16_t                              accountName[],
    FNetCliAuthAccountExistsRequestCallback     callback,
    void *                                      param
) {
    AccountExistsRequestTrans * trans = new AccountExistsRequestTrans(
        callback,
        param,
        accountName
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthLoginRequest (
    const ST::string&               accountName,
    const ShaDigest *               accountNamePassHash,
    const char16_t                  authToken[],
    const char16_t                  os[],
    FNetCliAuthLoginRequestCallback callback,
    void *                          param
) {
    // Cache updated login info if provided.
    if (!accountName.empty())
        s_accountName = accountName;
    if (accountNamePassHash)
        memcpy(s_accountNamePassHash, *accountNamePassHash, sizeof(ShaDigest));
    if (authToken)
        StrCopy(s_authToken, authToken, std::size(s_authToken));
    if (os)
        StrCopy(s_os, os, std::size(s_os));

    LoginRequestTrans * trans = new LoginRequestTrans(callback, param);
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAgeRequest (
    const ST::string&                   ageName,
    const plUUID&                       ageInstId,
    FNetCliAuthAgeRequestCallback       callback,
    void *                              param
) {
    AgeRequestTrans * trans = new AgeRequestTrans(
        ageName,
        ageInstId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthGetEncryptionKey (
    uint32_t    key[],
    size_t      size
) {
    unsigned memSize = std::min(std::size(s_encryptionKey), size);
    memSize *= sizeof(uint32_t);
    memcpy(key, s_encryptionKey, memSize);
}

//============================================================================
void NetCliAuthAccountCreateRequest (
    const char16_t                          accountName[],
    const char16_t                          password[],
    unsigned                                accountFlags,
    unsigned                                billingType,
    FNetCliAuthAccountCreateRequestCallback callback,
    void *                                  param
) {
    AccountCreateRequestTrans * trans = new AccountCreateRequestTrans(
        accountName,
        password,
        accountFlags,
        billingType,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountCreateFromKeyRequest (
    const char16_t                                  accountName[],
    const char16_t                                  password[],
    plUUID                                          key,
    unsigned                                        billingType,
    FNetCliAuthAccountCreateFromKeyRequestCallback  callback,
    void *                                          param
) {
    AccountCreateFromKeyRequestTrans * trans = new AccountCreateFromKeyRequestTrans(
        accountName,
        password,
        key,
        billingType,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthPlayerCreateRequest (
    const ST::string&                       playerName,
    const ST::string&                       avatarShape,
    const ST::string&                       friendInvite,
    FNetCliAuthPlayerCreateRequestCallback  callback,
    void *                                  param
) {
    ST::string name = playerName;
    ENetError error = FixupPlayerName(name);
    if (IS_NET_ERROR(error)) {
        NetCliAuthPlayerInfo playerInfo;
        callback(error, param, playerInfo);
    }
    else {
        PlayerCreateRequestTrans * trans = new PlayerCreateRequestTrans(
            name,
            avatarShape,
            friendInvite,
            callback,
            param
        );
        NetTransSend(trans);
    }
}

//============================================================================
void NetCliAuthPlayerDeleteRequest (
    unsigned                                playerId,
    FNetCliAuthPlayerDeleteRequestCallback  callback,
    void *                                  param
) {
    PlayerDeleteRequestTrans * trans = new PlayerDeleteRequestTrans(
        playerId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthUpgradeVisitorRequest (
    unsigned                                    playerId,
    FNetCliAuthUpgradeVisitorRequestCallback    callback,
    void *                                      param
) {
    UpgradeVisitorRequestTrans * trans = new UpgradeVisitorRequestTrans(
        playerId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthSetCCRLevel (
    unsigned        ccrLevel
) {
    CliAuConn * conn = GetConnIncRef("SetCCRLevel");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Auth_ClientSetCCRLevel,
                    ccrLevel,
    };
    
    conn->Send(msg, std::size(msg));
    conn->UnRef("SetCCRLevel");
}

//============================================================================
void NetCliAuthSetPlayerRequest (
    unsigned                            playerInt,
    FNetCliAuthSetPlayerRequestCallback callback,
    void *                              param
) {
    SetPlayerRequestTrans * trans = new SetPlayerRequestTrans(
        playerInt,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthSetAgePublic (
    unsigned                    ageInfoId,
    bool                        publicOrNot
) {
    CliAuConn * conn = GetConnIncRef("SetAgePublic");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Auth_SetAgePublic,
                    ageInfoId,
                    publicOrNot,
    };
    
    conn->Send(msg, std::size(msg));

    conn->UnRef("SetAgePublic");
}

//============================================================================
void NetCliAuthGetPublicAgeList (
    const ST::string&                   ageName,
    FNetCliAuthGetPublicAgeListCallback callback,
    void *                              param
) {
    GetPublicAgeListTrans * trans = new GetPublicAgeListTrans(
        ageName,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountChangePasswordRequest (
    const ST::string&                               accountName,
    const ST::string&                               password,
    FNetCliAuthAccountChangePasswordRequestCallback callback,
    void *                                          param
) {
    AccountChangePasswordRequestTrans * trans = new AccountChangePasswordRequestTrans(
        accountName,
        password,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountSetRolesRequest (
    const char16_t                              accountName[],
    unsigned                                    accountFlags,
    FNetCliAuthAccountSetRolesRequestCallback   callback,
    void *                                      param
) {
    AccountSetRolesRequestTrans * trans = new AccountSetRolesRequestTrans(
        accountName,
        accountFlags,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountSetBillingTypeRequest (
    const char16_t                                  accountName[],
    unsigned                                        billingType,
    FNetCliAuthAccountSetBillingTypeRequestCallback callback,
    void *                                          param
) {
    AccountSetBillingTypeRequestTrans * trans = new AccountSetBillingTypeRequestTrans(
        accountName,
        billingType,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthAccountActivateRequest (
    const plUUID&                               activationKey,
    FNetCliAuthAccountActivateRequestCallback   callback,
    void *                                      param
) {
    AccountActivateRequestTrans * trans = new AccountActivateRequestTrans(
        activationKey,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthFileListRequest (
    const char16_t                      dir[],
    const char16_t                      ext[],
    FNetCliAuthFileListRequestCallback  callback,
    void *                              param
) {
    FileListRequestTrans * trans = new FileListRequestTrans(
        callback,
        param,
        dir,
        ext
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthFileRequest (
    const plFileName &              filename,
    hsStream *                      writer,
    FNetCliAuthFileRequestCallback  callback,
    void *                          param
) {
    FileDownloadRequestTrans * trans = new FileDownloadRequestTrans(
        callback,
        param,
        filename,
        writer
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultSetRecvNodeChangedHandler (
    FNetCliAuthVaultNodeChanged handler
) {
    s_vaultNodeChangedHandler   = handler;
}

//============================================================================
void NetCliAuthVaultSetRecvNodeAddedHandler (
    FNetCliAuthVaultNodeAdded   handler
) {
    s_vaultNodeAddedHandler     = handler;
}

//============================================================================
void NetCliAuthVaultSetRecvNodeRemovedHandler (
    FNetCliAuthVaultNodeRemoved handler
) {
    s_vaultNodeRemovedHandler   = handler;
}

//============================================================================
void NetCliAuthVaultSetRecvNodeDeletedHandler (
    FNetCliAuthVaultNodeDeleted handler
) {
    s_vaultNodeDeletedHandler   = handler;
}

//============================================================================
void NetCliAuthVaultNodeCreate (
    NetVaultNode *              templateNode,
    FNetCliAuthVaultNodeCreated callback,
    void *                      param
) {
    VaultCreateNodeTrans * trans = new VaultCreateNodeTrans(
        templateNode,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultNodeFetch (
    unsigned                    nodeId,
    FNetCliAuthVaultNodeFetched callback,
    void *                      param
) {
    VaultFetchNodeTrans * trans = new VaultFetchNodeTrans(
        nodeId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultNodeFind (
    NetVaultNode *              templateNode,
    FNetCliAuthVaultNodeFind    callback,
    void *                      param
) {
    VaultFindNodeTrans * trans = new VaultFindNodeTrans(
        templateNode,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
unsigned NetCliAuthVaultNodeSave (
    NetVaultNode *                      node,
    FNetCliAuthVaultNodeSaveCallback    callback,
    void *                              param
) {
    if (!node->IsDirty())
        return 0;

    if (!node->GetNodeId())
        return 0;

    // We're definitely saving this node, so assign a revisionId
    node->GenerateRevision();

    // Lots of hacks for MOULa :(
    uint32_t ioFlags = NetVaultNode::kDirtyOnly | NetVaultNode::kClearDirty;
    if (node->GetNodeType() == plVault::kNodeType_SDL)
        ioFlags |= NetVaultNode::kDirtyString64_1;
    ioFlags |= NetVaultNode::kDirtyNodeType;

    std::vector<uint8_t> buffer;
    node->Write(&buffer, ioFlags);

    VaultSaveNodeTrans * trans = new VaultSaveNodeTrans(
        node->GetNodeId(),
        node->GetRevision(),
        buffer.size(),
        buffer.data(),
        callback,
        param
    );
    NetTransSend(trans);
    return buffer.size();
}

//============================================================================
void NetCliAuthVaultNodeDelete (
    unsigned                    nodeId
) {
    hsAssert(false, "eric, implement me");
}

//============================================================================
void NetCliAuthVaultNodeAdd (
    unsigned                        parentId,
    unsigned                        childId,
    unsigned                        ownerId,
    FNetCliAuthVaultNodeAddCallback callback,
    void *                          param
) {
    VaultAddNodeTrans * trans = new VaultAddNodeTrans(
        parentId,
        childId,
        ownerId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultNodeRemove (
    unsigned                            parentId,
    unsigned                            childId,
    FNetCliAuthVaultNodeRemoveCallback  callback,
    void *                              param
) {
    VaultRemoveNodeTrans * trans = new VaultRemoveNodeTrans(
        parentId,
        childId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultFetchNodeRefs (
    unsigned                        nodeId,
    FNetCliAuthVaultNodeRefsFetched callback,
    void *                          param
) {
    VaultFetchNodeRefsTrans * trans = new VaultFetchNodeRefsTrans(
        nodeId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthVaultSetSeen (
    unsigned    parentId,
    unsigned    childId,
    bool        seen
) {
    CliAuConn * conn = GetConnIncRef("SetSeen");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Auth_VaultSetSeen,
                    parentId,
                    childId,
                    seen,
    };
    
    conn->Send(msg, std::size(msg));

    conn->UnRef("SetSeen");
}

//============================================================================
void NetCliAuthVaultSendNode (
    unsigned    srcNodeId,
    unsigned    dstPlayerId
) {
    CliAuConn * conn = GetConnIncRef("SendNode");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Auth_VaultSendNode,
                    srcNodeId,
                    dstPlayerId,
    };
    
    conn->Send(msg, std::size(msg));

    conn->UnRef("SendNode");
}

//============================================================================
void NetCliAuthVaultInitAge (
    const plUUID&               ageInstId,          // optional. is used in match
    const plUUID&               parentAgeInstId,    // optional. is used in match
    const ST::string&           ageFilename,        // optional. is used in match
    const ST::string&           ageInstName,        // optional. not used in match
    const ST::string&           ageUserName,        // optional. not used in match
    const ST::string&           ageDesc,            // optional. not used in match
    unsigned                    ageSequenceNumber,  // optional. not used in match
    unsigned                    ageLanguage,        // optional. not used in match
    FNetCliAuthAgeInitCallback  callback,           // optional
    void *                      param               // optional
) {
    VaultInitAgeTrans * trans = new VaultInitAgeTrans(
        callback,
        param,
        ageInstId,
        parentAgeInstId,
        ageFilename,
        ageInstName,
        ageUserName,
        ageDesc,
        ageSequenceNumber,
        ageLanguage
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthSetRecvBufferHandler (
    FNetCliAuthRecvBufferHandler    handler
) {
    s_bufRcvdCb = handler;
}

//============================================================================
void NetCliAuthSendCCRPetition (
    const ST::string&     petitionText
) {
    hsAssert(false, "eric, implement me.");
}

//============================================================================
void NetCliAuthPropagateBuffer (
    unsigned                        type,
    unsigned                        bytes,
    const uint8_t                      buffer[]
) {
    CliAuConn * conn = GetConnIncRef("PropBuffer");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Auth_PropagateBuffer,
        type,
        bytes,
        (uintptr_t) buffer,
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef("PropBuffer");

}

//============================================================================
void NetCliAuthLogPythonTraceback (const char16_t traceback[]) {
    CliAuConn * conn = GetConnIncRef("LogTraceback");
    if (!conn)
        return;

    const uintptr_t msg[] = {
       kCli2Auth_LogPythonTraceback,
       (uintptr_t) traceback
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef("LogTraceback");
}


//============================================================================
void NetCliAuthLogStackDump (const char16_t stackdump[]) {
    CliAuConn * conn = GetConnIncRef("LogStackDump");
    if (!conn)
        return;

    const uintptr_t msg[] = {
       kCli2Auth_LogStackDump,
       (uintptr_t) stackdump
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef("LogStackDump");
}

//============================================================================
void NetCliAuthLogClientDebuggerConnect () {
    CliAuConn * conn = GetConnIncRef("");
    if (!conn)
        return;

    unsigned nothing = 0;

    const uintptr_t msg[] = {
       kCli2Auth_LogClientDebuggerConnect,
        nothing
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef();
}

//============================================================================
void NetCliAuthSetNotifyNewBuildHandler (FNotifyNewBuildHandler handler) {

    s_notifyNewBuildHandler = handler;
}

//============================================================================
void NetCliAuthSetPlayerBanStatusRequest (
    unsigned                                        playerId,
    unsigned                                        banned,
    FNetCliAuthSetPlayerBanStatusRequestCallback    callback,
    void *                                          param
) {
    SetPlayerBanStatusRequestTrans * trans = new SetPlayerBanStatusRequestTrans(
        playerId,
        banned,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthKickPlayer (
    unsigned                                playerId
) {
    CliAuConn * conn = GetConnIncRef("KickPlayer");
    if (!conn)
        return;

    const uintptr_t msg[] = {
       kCli2Auth_KickPlayer,
                    playerId
    };

    conn->Send(msg, std::size(msg));
    conn->UnRef("KickPlayer");
}

//============================================================================
void NetCliAuthChangePlayerNameRequest (
    unsigned                                    playerId,
    const char16_t                              newName[],
    FNetCliAuthChangePlayerNameRequestCallback  callback,
    void *                                      param
) {
    ChangePlayerNameRequestTrans * trans = new ChangePlayerNameRequestTrans(
        playerId,
        newName,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthSendFriendInvite (
    const ST::string&                   emailAddress,
    const ST::string&                   toName,
    const plUUID&                       inviteUuid,
    FNetCliAuthSendFriendInviteCallback callback,
    void *                              param
) {
    SendFriendInviteTrans * trans = new SendFriendInviteTrans(
        emailAddress,
        toName,
        inviteUuid,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreCreate (
    unsigned                        ownerId,
    const ST::string&               gameName,
    unsigned                        gameType,
    int                             value,
    FNetCliAuthCreateScoreCallback  callback,
    void *                          param
) {
    ScoreCreateTrans * trans = new ScoreCreateTrans(
        ownerId,
        gameName,
        gameType,
        value,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreDelete(
    unsigned                        scoreId,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) {
    ScoreDeleteTrans * trans = new ScoreDeleteTrans(
        scoreId,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreGetScores(
    unsigned                        ownerId,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback,
    void *                          param
) {
    ScoreGetScoresTrans * trans = new ScoreGetScoresTrans(
        ownerId,
        gameName,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreAddPoints(
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) {
    ScoreAddPointsTrans * trans = new ScoreAddPointsTrans(
        scoreId,
        numPoints,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreTransferPoints(
    unsigned                        srcScoreId,
    unsigned                        destScoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) {
    ScoreTransferPointsTrans * trans = new ScoreTransferPointsTrans(
        srcScoreId,
        destScoreId,
        numPoints,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreSetPoints(
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback,
    void *                          param
) {
    ScoreSetPointsTrans * trans = new ScoreSetPointsTrans(
        scoreId,
        numPoints,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreGetRankList(
    unsigned                    ownerId,
    unsigned                    scoreGroup,
    unsigned                    parentFolderId,
    const ST::string&           gameName,
    unsigned                    timePeriod,
    unsigned                    numResults,
    unsigned                    pageNumber,
    bool                        sortDesc,
    FNetCliAuthGetRanksCallback callback,
    void *                      param
) {
    ScoreGetRanksTrans * trans = new ScoreGetRanksTrans(
        ownerId,
        scoreGroup,
        parentFolderId,
        gameName,
        timePeriod,
        numResults,
        pageNumber,
        sortDesc,
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliAuthScoreGetHighScores(
    unsigned                        ageId,
    unsigned                        maxScores,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback,
    void *                          param
    ) {
    ScoreGetHighScoresTrans * trans = new ScoreGetHighScoresTrans(
        ageId,
        maxScores,
        gameName,
        callback,
        param
    );
    NetTransSend(trans);
}
