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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClientComm/plNetClientComm.h
*
*   This module is the translation layer between simple network types
*   such as byte arrays, and higher-level Plasma-specific types such
*   as the plFactory-managed types.
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H


#include "HeadSpin.h"

#include "pnEncryption/plChecksum.h"
#include "pnNetBase/pnNbError.h"
#include "pnUUID/pnUUID.h"

#include "plMessage/plNetCommMsgs.h"
#include "plNetCommon/plNetCommonHelpers.h"

#include <string_theory/string>
#include <vector>


class plNetMessage;


/*****************************************************************************
*
*   NetClientComm API
*
***/

struct NetCommPlayer {
    unsigned    playerInt;
    ST::string  playerName;
    ST::string  avatarDatasetName;
    unsigned    explorer;

    NetCommPlayer() : playerInt(), explorer() { }
    NetCommPlayer(unsigned id, const ST::string& name, const ST::string& shape, unsigned ex)
        : playerInt(id), playerName(name), avatarDatasetName(shape), explorer(ex)
    { }
};

struct NetCommAccount {
    plUUID      accountUuid;
    ST::string  accountName;
    ShaDigest   accountNamePassHash;
    unsigned    accountFlags;
    unsigned    billingType;
};

struct NetCommAge {
    plUUID      ageInstId;
    unsigned    ageVaultId;
    ST::string  ageDatasetName;
    char        spawnPtName[64];

    NetCommAge() : ageVaultId(0)
    {
        memset(spawnPtName, 0, sizeof(spawnPtName));
    }
};

const NetCommAge *                  NetCommGetAge ();
const NetCommAge *                  NetCommGetStartupAge ();
const NetCommAccount *              NetCommGetAccount ();
const NetCommPlayer *               NetCommGetPlayer ();
const std::vector<NetCommPlayer> &  NetCommGetPlayerList ();
unsigned                            NetCommGetPlayerCount ();
bool                                NetCommIsLoginComplete ();
void                                NetCommSetIniPlayerId(unsigned playerId);
void                                NetCommSetIniStartUpAge(const ST::string& ageName);
void                                NetCommSetAccountUsernamePassword (const ST::string& username, const ShaDigest &  namePassHash);
void                                NetCommSetAuthTokenAndOS (const char16_t* authToken, const char16_t* os);
ENetError                           NetCommGetAuthResult ();

bool                                NetCommNeedToLoadAvatar ();
void                                NetCommSetAvatarLoaded (bool loaded = true);
void                                NetCommChangeMyPassword (const ST::string& password);

void NetCommStartup ();
void NetCommShutdown ();
void NetCommUpdate ();
void NetCommConnect ();
void NetCommDisconnect ();
void NetCommSendMsg (
    plNetMessage *  msg
);
void NetCommRecvMsg (
    plNetMessage *  msg
);
void NetCommEnableNet (
    bool            enabled,
    bool            wait
);
void NetCommActivatePostInitErrorHandler();
void NetCommActivateMsgDispatchers();


/*****************************************************************************
*
*   Net message handlers
*
***/

// Return this value from your registered msg handler
// to stop further dispatching of incoming msg.
const unsigned kOK_MsgConsumed  = hsOK + 1;

typedef int (FNetCommMsgHandler)(
    plNetMessage *  msg,
    void *          userState
);

// Adds a msg handler for a msg that is convertable to specified type.
void NetCommAddMsgHandlerForType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    void *                  userState
);
// Adds a msg handler for a specific msg type.
void NetCommAddMsgHandlerForExactType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    void *                  userState
);

extern const unsigned       kNetCommAllMsgClasses;
extern FNetCommMsgHandler * kNetCommAllMsgHandlers;
extern const void *         kNetCommAllUserStates;

void NetCommRemoveMsgHandler (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    handler,
    const void *            userState
);

void NetCommSetDefaultMsgHandler (
    FNetCommMsgHandler *    handler,
    void *                  userState
);
void NetCommSetMsgPreHandler (
    FNetCommMsgHandler *    handler,
    void *                  userState
);

/*****************************************************************************
*
*   Network requests
*   Network replies are posted via plDispatch
*
***/

void NetCommAuthenticate (  // --> plNetCommAuthMsg
    void *                  param
);
void NetCommGetFileList (   // --> plNetCommFileListMsg
    const char16_t          dir[],
    const char16_t          ext[],
    void *                  param
);
void NetCommGetFile (       // --> plNetCommFileDownloadMsg
    const char16_t          filename[],
    hsStream *              writer,
    void *                  param
);
void NetCommLinkToAge (     // --> plNetCommLinkToAgeMsg
    const NetCommAge &      age,
    void *                  param
);
void NetCommSetActivePlayer (//--> plNetCommActivePlayerMsg
    unsigned                desiredPlayerInt,
    void *                  param
);
void NetCommCreatePlayer (  // --> plNetCommCreatePlayerMsg
    const ST::string&       playerName,
    const ST::string&       avatarShape,
    const ST::string&       friendInvite,
    unsigned                createFlags,
    void *                  param
);
void NetCommDeletePlayer (  // --> plNetCommDeletePlayerMsg
    unsigned                playerInt,
    void *                  param
);
void NetCommGetPublicAgeList (//-> plNetCommPublicAgeListMsg
    const ST::string&               ageName,
    void *                          param,
    plNetCommReplyMsg::EParamType   ptype = plNetCommReplyMsg::kParamTypeOther
);
void NetCommSetAgePublic (  // --> no msg
    unsigned                ageInfoId,
    bool                    makePublic
);
void NetCommCreatePublicAge (// --> plNetCommPublicAgeMsg
    const char              ageName[],
    const plUUID&           ageInstId,
    void *                  param
);
void NetCommRemovePublicAge(// --> plNetCommPublicAgeMsg
    const plUUID&           ageInstId,
    void *                  param
);
void NetCommRegisterOwnedAge (
    const NetCommAge &      age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
);
void NetCommUnregisterOwnedAge (
    const char              ageName[],
    unsigned                playerInt,
    void *                  param
);
void NetCommRegisterVisitAge (
    const NetCommAge &      age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
);
void NetCommUnregisterVisitAge (
    const plUUID&           ageInstId,
    unsigned                playerInt,
    void *                  param
);
void NetCommConnectPlayerVault (
    void *                  param
);
void NetCommDisconnectPlayerVault ();
void NetCommConnectAgeVault (
    const plUUID&           ageInstId,
    void *                  param
);
void NetCommDisconnectAgeVault ();
void NetCommUpgradeVisitorToExplorer (
    unsigned                playerInt,
    void *                  param
);
void NetCommSetCCRLevel (
    unsigned                ccrLevel
);
void NetCommSendFriendInvite (
    const ST::string& emailAddress,
    const ST::string& toName,
    const plUUID&     inviteUuid
);

#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENTCOMM_PLNETCLIENTCOMM_H




/*****************************************************************************
*
*   Old code...
*
***/
#ifndef plNetClientComm_h_inc
#define plNetClientComm_h_inc

////////////////////////////////////////////////////////////////////
// plNetClientComm
//  - Sends/Recvs messages with a server.
//  - Performs common network tasks (eg. auth, ping, join, leave,
//      create player, delete player, etc.).
//  - Calls you back when network task is complete.
//  - Fwds unsolicited messages to your supplied handler.
//      - You can register a msg handler msg type family (i.e. msg
//          type or base of type).
//      - You can register a msg handler for an exact msg type.
//  - Sends periodic alive msg to server when authenticated.
//  - Tracks message receipts for message types you specify.
//  - Checks for server silence.
//

class plNetClientComm
{
public:
    // Message handler for unsolicited msgs or registered for specific msg types.
    class MsgHandler
    {
    public:
        static int StaticMsgHandler(plNetMessage * msg, void * userState);
        virtual int HandleMessage( plNetMessage* msg ) = 0;
    };

    ////////////////////////////////////////////////////////////////

    plNetClientComm();
    ~plNetClientComm();

    ////////////////////////////////////////////////////////////////

    // Adds a msg handler for a msg that is convertable to specified type.
    void    AddMsgHandlerForType( uint16_t msgClassIdx, MsgHandler* handler );

    // Adds a msg handler for a specific msg type.
    void    AddMsgHandlerForExactType( uint16_t msgClassIdx, MsgHandler* handler );

    bool    RemoveMsgHandler( MsgHandler* handler );

    // Msgs not part of a task controlled by this
    // object, and doesn't have a handler set for its type
    // are sent to this handler (if set).
    void    SetDefaultHandler( MsgHandler* msgHandler );
};

////////////////////////////////////////////////////////////////////
#endif // plNetClientComm_h_inc
