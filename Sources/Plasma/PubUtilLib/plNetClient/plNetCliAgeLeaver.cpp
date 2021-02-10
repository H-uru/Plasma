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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClient/plNetCliAgeLeaver.cpp
*   
***/

#include "plNetCliAgeLeaver.h"
#include "plNetClientMgr.h"
#include "plNetLinkingMgr.h"

#include "hsResMgr.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plAvatar/plAvatarMgr.h"
#include "plMessage/plLoadAgeMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plNetClientComm/plNetClientComm.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plVault/plVault.h"



/*****************************************************************************
*
*   Private
*
***/

struct plNCAgeLeaver {

    enum NextOp {
        kNoOp,
        kDisableClickables,
        kLinkOutFX,
        kUnloadAge,
        kNotifyAgeUnloaded,
    };

    NextOp                  nextOp;
    bool                    quitting;
    bool                    muteLinkSfx;
    bool                    complete;
    FNCAgeLeaverCallback    callback;
    void *                  userState;

    plNCAgeLeaver (
        bool                    quitting,
        bool                    muteSfx,
        FNCAgeLeaverCallback    callback,
        void *                  userState
    );
    ~plNCAgeLeaver ();

    void Start ();
    void Complete (bool success, const char msg[]);
    bool MsgReceive (plMessage * msg);
    void Update ();
    void ExecNextOp ();
};


/*****************************************************************************
*
*   plNCAgeLeaver
*
***/

//============================================================================
plNCAgeLeaver::plNCAgeLeaver (
    bool                    quitting,
    bool                    muteSfx,
    FNCAgeLeaverCallback    callback,
    void *                  userState
) : nextOp(kNoOp)
,   quitting(quitting)
,   muteLinkSfx(muteSfx)
,   complete(false)
,   callback(callback)
,   userState(userState)
{
}

//============================================================================
plNCAgeLeaver::~plNCAgeLeaver () {
}

//============================================================================
void plNCAgeLeaver::Start () {
    nextOp = kLinkOutFX;
}

//============================================================================
void plNCAgeLeaver::Complete (bool success, const char msg[]) {

    if (!complete) {
        complete = true;
        
        NCAgeLeaveCompleteNotify    notify;
        notify.success  = success;
        notify.msg      = msg;
        
        callback(this, kAgeLeaveComplete, &notify, userState);
        delete this;
    }
}

//============================================================================
bool plNCAgeLeaver::MsgReceive (plMessage * msg) {
    //========================================================================
    // Done with link out effects
    //========================================================================
    if (plLinkOutUnloadMsg * linkOutUnloadMsg = plLinkOutUnloadMsg::ConvertNoRef(msg))
    {
        if (!linkOutUnloadMsg->HasBCastFlag(plMessage::kNetNonLocal)
            && linkOutUnloadMsg->GetPlayerID() == NetCommGetPlayer()->playerInt
        ) {
            nextOp = kUnloadAge;
        }
        return true;
    }
    
    //========================================================================
    // Age data unloaded
    //========================================================================
    if (plAgeLoadedMsg * ageLoadedMsg = plAgeLoadedMsg::ConvertNoRef(msg)) {
        if (!ageLoadedMsg->fLoaded) {
            nextOp = kNotifyAgeUnloaded;
            return true;
        }
        return false;
    }
    

    return false;
}

//============================================================================
void plNCAgeLeaver::ExecNextOp () {
    plNetClientMgr *    nc = plNetClientMgr::GetInstance();
    plAvatarMgr *       am = plAvatarMgr::GetInstance();

    NextOp next = nextOp;
    nextOp      = kNoOp;
    switch (next) {
        //====================================================================
        case kNoOp: {
        }
        break;

        //====================================================================
        case kDisableClickables: {
            (new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableClickables))->Send();
            nextOp = kLinkOutFX;
        }
        break;

        //====================================================================
        case kLinkOutFX: {
            plLinkEffectsTriggerMsg* lem = new plLinkEffectsTriggerMsg();
            lem->MuteLinkSfx(muteLinkSfx);
            lem->SetLeavingAge(true);
            lem->SetLinkKey(nc->GetLocalPlayerKey());
            lem->SetBCastFlag(plMessage::kNetPropagate);
            lem->SetBCastFlag(plMessage::kNetForce);    // Necessary?
            lem->Send(hsgResMgr::ResMgr()->FindKey(plUoid(kLinkEffectsMgr_KEY)));
        }
        break;

        //====================================================================
        case kUnloadAge: {
            nc->BeginTask();
            NetCliGameDisconnect();

            // Cull nodes that were part of this age vault (but not shared by the player's vault)
            VaultCull(NetCommGetAge()->ageVaultId);

            // remove the age device inbox mappings
            VaultClearDeviceInboxMap();
            
            // Tell our local player that he's unspawning (if that is indeed the case)
            nc->IPlayerChangeAge(true /* exiting */, 0/* respawn */);
            // disconnect age vault
            
            // @@@ TODO: Unload age vault here

            plAgeLoader::GetInstance()->UnloadAge();            // unload age
            nc->ISendCameraReset(false/*leaving age*/);         // reset camera
            nc->IUnloadRemotePlayers();                         // unload other players
            nc->IUnloadNPCs();                                  // unload non-player clones

            if (NetCommNeedToLoadAvatar())
                am->UnLoadLocalPlayer();
        }
        break;

        //====================================================================
        case kNotifyAgeUnloaded: {
            // Set "Playing Game" bit to false
            nc->SetFlagsBit(plNetClientMgr::kPlayingGame, false);
            
            // Release AgeSDL object, if any
            if (nc->fAgeSDLObjectKey)
                nc->GetKey()->Release(nc->fAgeSDLObjectKey);
                
            // All done leaving age
            Complete(true, "Age unloaded");
        }
        break;

        DEFAULT_FATAL(nextOp);
    }
}

//============================================================================
void plNCAgeLeaver::Update () {
    ExecNextOp();
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NCAgeLeaverCreate (
    plNCAgeLeaver **        pleaver,
    bool                    quitting,
    bool                    muteSfx,
    FNCAgeLeaverCallback    callback,
    void *                  userState
) {
    ASSERT(pleaver);
    ASSERT(callback);
    
    plNCAgeLeaver * leaver;
    *pleaver = leaver = new plNCAgeLeaver(
        quitting,
        muteSfx,
        callback,
        userState
    );
    leaver->Start();
}

//============================================================================
bool NCAgeLeaverMsgReceive (
    plNCAgeLeaver *         leaver,
    plMessage *             msg
) {
    ASSERT(leaver);
    return leaver->MsgReceive(msg);
}

//============================================================================
void NCAgeLeaverUpdate (
    plNCAgeLeaver * leaver
) {
    ASSERT(leaver);
    leaver->Update();
}
