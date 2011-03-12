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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Intern.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_INTERN_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Intern.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_INTERN_H


namespace Ngl {


/*****************************************************************************
*
*   Module constants
*
***/

const unsigned kMaxReconnectIntervalMs				= 5 * 1000;
const unsigned kMaxImmediateDisconnects				= 5;
const unsigned kMaxFailedConnects					= 5;
const unsigned kPingIntervalMs						= 30 * 1000;
const unsigned kPingTimeoutMs						= kPingIntervalMs * 10;
const unsigned kDisconnectedTimeoutMs				= kPingIntervalMs;


/*****************************************************************************
*
*   Core
*
***/

void ReportNetError (ENetProtocol protocol, ENetError error);


/*****************************************************************************
*
*   Auth
*
***/

void AuthInitialize ();
void AuthDestroy (bool wait);

bool AuthQueryConnected ();
unsigned AuthGetConnId ();
void AuthPingEnable (bool enable);


/*****************************************************************************
*
*   Game
*
***/

void GameInitialize ();
void GameDestroy (bool wait);

bool GameQueryConnected ();
unsigned GameGetConnId ();
void GamePingEnable (bool enable);


/*****************************************************************************
*
*   File
*
***/

void FileInitialize ();
void FileDestroy (bool wait);

bool FileQueryConnected ();
unsigned FileGetConnId ();


/*****************************************************************************
*
*   Csr
*
***/

void CsrInitialize ();
void CsrDestroy (bool wait);

bool CsrQueryConnected ();
unsigned CsrGetConnId ();


/*****************************************************************************
*
*   GateKeeper
*
***/

void GateKeeperInitialize();
void GateKeeperDestroy(bool wait);
bool GateKeeperQueryConnected ();
unsigned GateKeeperGetConnId();


/*****************************************************************************
*
*   Transactions
*
***/

enum ETransType {
	//========================================================================
	// NglAuth.cpp transactions
	kPingRequestTrans,
	kLoginRequestTrans,
	kAgeRequestTrans,
	kAccountCreateRequestTrans,
	kAccountCreateFromKeyRequestTrans,
	kPlayerCreateRequestTrans,
	kPlayerDeleteRequestTrans,
	kUpgradeVisitorRequestTrans,
	kSetPlayerRequestTrans,
	kAccountChangePasswordRequestTrans,
	kGetPublicAgeListTrans,
	kAccountSetRolesRequestTrans,
	kAccountSetBillingTypeRequestTrans,
	kAccountActivateRequestTrans,
	kFileListRequestTrans,
	kFileDownloadRequestTrans,
	kRcvdFileDownloadChunkTrans,
	kRcvdPropagatedBufferTrans,
	kVaultNodeChangedTrans,
	kVaultNodeAddedTrans,
	kVaultNodeRemovedTrans,
	kVaultNodeDeletedTrans,
	kVaultFetchNodeRefsTrans,
	kVaultInitAgeTrans,
	kVaultFetchNodeTrans,
	kVaultFindNodeTrans,
	kVaultCreateNodeTrans,
	kVaultSaveNodeTrans,
	kVaultAddNodeTrans,
	kVaultRemoveNodeTrans,
	kNotifyNewBuildTrans,
	kSetPlayerBanStatusRequestTrans,
	kChangePlayerNameRequestTrans,
	kAuthConnectedNotifyTrans,
	kScoreCreateTrans,
	kScoreDeleteTrans,
	kScoreGetScoresTrans,
	kScoreAddPointsTrans,
	kScoreTransferPointsTrans,
	kScoreSetPointsTrans,
	kScoreGetRanksTrans,
	kSendFriendInviteTrans,
	
	//========================================================================
	// NglGame.cpp transactions
	kJoinAgeRequestTrans,
	kGmRcvdPropagatedBufferTrans,
	kGmRcvdGameMgrMsgTrans,
	
	//========================================================================
	// NglFile.cpp transactions
	kBuildIdRequestTrans,
	kManifestRequestTrans,
	kDownloadRequestTrans,
	kFileRcvdFileDownloadChunkTrans,
	
	//========================================================================
	// NglCsr.cpp transactions
	kCsrConnectedNotifyTrans,
	kCsrLoginTrans,
	
	//========================================================================
	// NglCore.cpp transactions
	kReportNetErrorTrans,

	//========================================================================
	// NglGateKeeper.cpp transactions
	kGkFileSrvIpAddressRequestTrans,
	kGkAuthSrvIpAddressRequestTrans,

	kNumTransTypes
};

static char * s_transTypes[] = {
	// NglAuth.cpp
	"PingRequestTrans",
	"LoginRequestTrans",
	"AgeRequestTrans",
	"AccountCreateRequestTrans",
	"AccountCreateFromKeyRequestTrans",
	"PlayerCreateRequestTrans",
	"PlayerDeleteRequestTrans",
	"UpgradeVisitorRequestTrans",
	"SetPlayerRequestTrans",
	"AccountChangePasswordRequestTrans",
	"GetPublicAgeListTrans",
	"AccountSetRolesRequestTrans",
	"AccountSetBillingTypeRequestTrans",
	"AccountActivateRequestTrans",
	"FileListRequestTrans",
	"FileDownloadRequestTrans",
	"RcvdFileDownloadChunkTrans",
	"RcvdPropagatedBufferTrans",
	"VaultNodeChangedTrans",
	"VaultNodeAddedTrans",
	"VaultNodeRemovedTrans",
	"VaultNodeDeletedTrans",
	"VaultFetchNodeRefsTrans",
	"VaultInitAgeTrans",
	"VaultFetchNodeTrans",
	"VaultFindNodeTrans",
	"VaultCreateNodeTrans",
	"VaultSaveNodeTrans",
	"VaultAddNodeTrans",
	"VaultRemoveNodeTrans",
	"NotifyNewBuildTrans",
	"SetPlayerBanStatusfRequestTrans",
	"ChangePlayerNameRequestTrans",
	"AuthConnectedNotifyTrans",
	"ScoreCreateTrans",
	"ScoreDeleteTrans",
	"ScoreGetScoresTrans",
	"ScoreAddPointsTrans",
	"ScoreTransferPointsTrans",
	"ScoreSetPointsTrans",
	"ScoreGetRanksTrans",
	"SendFriendInviteTrans",
	
	// NglGame.cpp
	"JoinAgeRequestTrans",
	"GmRcvdPropagatedBufferTrans",
	"GmRcvdGameMgrMsgTrans",

	// NglFile.cpp
	"BuildIdRequestTrans",
	"ManifestRequestTrans",
	"DownloadRequestTrans",
	"FileRcvdFileDownloadChunkTrans",

	// NglCsr.cpp
	"CsrConnectedNotifyTrans",
	"CsrLoginTrans",
	
	// NglCore.cpp
	"ReportNetErrorTrans",

	// NglGateKeeper.cpp
	"GkFileSrvIpAddress",
	"GkAuthSrvIpAddress",

};
COMPILER_ASSERT(arrsize(s_transTypes) == kNumTransTypes);

static long s_perfTransCount[kNumTransTypes];


namespace Auth { struct CliAuConn; }
namespace Game { struct CliGmConn; }
namespace Csr  { struct CliCsConn; }
namespace File { struct CliFileConn; }
namespace GateKeeper { struct CliGkConn; }

enum ENetTransState {
    kTransStateWaitServerConnect,
    kTransStateWaitServerResponse,
    kTransStateComplete,
};

struct NetTrans : AtomicRef {
    LINK(NetTrans)  m_link;
    ENetTransState  m_state;
    ENetError       m_result;
    unsigned        m_transId;
    unsigned        m_connId;
    ENetProtocol    m_protocol;
	bool			m_hasSubTrans;	// set to specify this transaction should be completed after all others in the frame
	unsigned		m_timeoutAtMs;	// curTime + s_timeoutMs (set upon send)
	ETransType		m_transType;

    NetTrans (ENetProtocol protocol, ETransType transType);
    virtual ~NetTrans ();

    virtual bool CanStart () const;
	virtual bool TimedOut () { return true; } // return true if we really did time out, false to reset the timeout timer
    virtual bool Send () = 0;
    virtual void Post () = 0;
    virtual bool Recv ( // return false to disconnect from server
        const byte  msg[],
        unsigned    bytes
    ) = 0;
};

struct NetAuthTrans : NetTrans {
    Auth::CliAuConn * m_conn;

    NetAuthTrans (ETransType transType);
    ~NetAuthTrans ();

    bool AcquireConn ();
    void ReleaseConn ();
};

struct NetGameTrans : NetTrans {
    Game::CliGmConn * m_conn;

    NetGameTrans (ETransType transType);
    ~NetGameTrans ();

    bool AcquireConn ();
    void ReleaseConn ();
};

struct NetFileTrans : NetTrans {
	File::CliFileConn * m_conn;

	NetFileTrans (ETransType transType);
	~NetFileTrans ();

	bool AcquireConn ();
	void ReleaseConn ();
};

struct NetCsrTrans : NetTrans {
    Csr::CliCsConn * m_conn;

    NetCsrTrans (ETransType transType);
    ~NetCsrTrans ();

    bool AcquireConn ();
    void ReleaseConn ();
};


struct NetGateKeeperTrans : NetTrans {
	GateKeeper::CliGkConn * m_conn;

    NetGateKeeperTrans (ETransType transType);
    ~NetGateKeeperTrans ();

    bool AcquireConn ();
    void ReleaseConn ();
};


struct NetNotifyTrans : NetTrans {
    NetNotifyTrans (ETransType transType);
    bool CanStart () const { return true; }
    bool Send () { m_state = kTransStateComplete; return true; }
    bool Recv (
        const byte [],
        unsigned
    ) { return true; }
};

void NetTransInitialize ();
void NetTransDestroy (bool wait);
void NetTransSetTimeoutMs (unsigned ms);
unsigned NetTransGetTimeoutMs ();

void NetTransSend (NetTrans * trans);
bool NetTransRecv (unsigned transId, const byte msg[], unsigned bytes);
void NetTransCancel (unsigned transId, ENetError error);
void NetTransCancelByProtocol (ENetProtocol protocol, ENetError error);
void NetTransCancelByConnId (unsigned connId, ENetError error);
void NetTransCancelAll (ENetError error);
void NetTransUpdate ();


/*****************************************************************************
*
*   Misc
*
***/

unsigned ConnNextSequence ();
unsigned ConnGetId (ENetProtocol protocol);


} using namespace Ngl;
