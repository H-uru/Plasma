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
/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// singular
#include "plAvBrainCoop.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvTaskSeek.h"

// other
#include "hsResMgr.h"
#include "hsStream.h"

#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plMessage/plAvatarMsg.h"
#include "plMessage/plAvCoopMsg.h"
#include "plMessage/plPickedMsg.h"
#include "plScene/plSceneNode.h"
#include "plStatusLog/plStatusLog.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// STATIC
//
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
//
// CODE
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
// plAvBrainCoop
//
/////////////////////////////////////////////////////////////////////////////////////////

plAvBrainCoop::plAvBrainCoop()
: fInitiatorID(0),
  fInitiatorSerial(0)
{
}

// plAvBrainCoop ----------------------------------------------------------
// --------------
plAvBrainCoop::plAvBrainCoop(uint32_t exitFlags, float fadeIn, float fadeOut,
                             MoveMode moveMode, plKey guestKey)
: plAvBrainGeneric(exitFlags, fadeIn, fadeOut, moveMode),
  fGuestKey(std::move(guestKey))
{
    static uint16_t coopSerial = 0;

    // this particular constructor is only called by the initiator...
    fInitiatorID = plNetClientApp::GetInstance()->GetPlayerID();
    fInitiatorSerial = coopSerial++;
}

// plAvBrainCoop ----------------------------------------------------------
// --------------
plAvBrainCoop::plAvBrainCoop(uint32_t exitFlags, float fadeIn, float fadeOut,
                             MoveMode moveMode, uint32_t initiatorID, uint16_t initiatorSerial,
                             plKey hostKey)
: plAvBrainGeneric(exitFlags, fadeIn, fadeOut, moveMode),
  fInitiatorID(initiatorID), fInitiatorSerial(initiatorSerial),
  fHostKey(std::move(hostKey))
{
}

// MsgReceive ----------------------------------
// -----------
bool plAvBrainCoop::MsgReceive(plMessage *msg)
{
    plPickedMsg *pickM = plPickedMsg::ConvertNoRef(msg);
    if(pickM)
    {
        if(fWaitingForClick)
        {
            fWaitingForClick = false;
            // clicks are never network propagated, so we can be sure that the
            // click was performed by the local player.
            plKey localPlayer = plNetClientApp::GetInstance()->GetLocalPlayerKey();

            if(localPlayer == fGuestKey)
            {
                plAvCoopMsg *coopM = new plAvCoopMsg(plAvCoopMsg::kGuestAccepted, fInitiatorID, fInitiatorSerial);
                coopM->SetBCastFlag(plMessage::kNetPropagate);
                coopM->Send();

            }
        }
    }
    return plAvBrainGeneric::MsgReceive(msg);
}

// RelayNotifyMsg ----------------------------------
// ---------------
bool plAvBrainCoop::RelayNotifyMsg(plNotifyMsg *msg)
{
    // add a coop event so the receiver will know what coop this message is from
    msg->AddCoopEvent(fInitiatorID, fInitiatorSerial);

    proMultiStageEventData * mtevt = static_cast<proMultiStageEventData *>(msg->FindEventRecord(proEventData::kMultiStage));
    if(mtevt)
        plAvatarMgr::GetInstance()->GetLog()->AddLineF("COOP: Relaying multi-stage event to {} recipients (via plAvBrainCoop)", fRecipients.size());

    if(fRecipients.size() != 0)
    {
        bool foundARecipient = false;
        for (unsigned curRecipient = 0; curRecipient < fRecipients.size(); curRecipient++)
        {
            if (fRecipients[curRecipient])
            {
                foundARecipient = true;
                msg->AddReceiver(fRecipients[curRecipient]);
            }
        }

        if (!foundARecipient)
            return false;

        msg->Send();
        return true;
    }
    else
        return false;
}


void plAvBrainCoop::EnableGuestClick()
{
    fWaitingForClick = true;
}

// GetInitiatorID --------------------
// ---------------
uint32_t plAvBrainCoop::GetInitiatorID()
{
    return fInitiatorID;
}

// GetInitiatorSerial --------------------
// -------------------
uint16_t plAvBrainCoop::GetInitiatorSerial()
{
    return fInitiatorSerial;
}

// Read -------------------------------------------------
// -----
void plAvBrainCoop::Read(hsStream *stream, hsResMgr *mgr)
{
    plAvBrainGeneric::Read(stream, mgr);

    fInitiatorID = stream->ReadLE32();
    fInitiatorSerial = stream->ReadLE16();

    if(stream->ReadBool())
    {
        fHostKey = mgr->ReadKey(stream);
    }
    if(stream->ReadBool())
    {
        fGuestKey = mgr->ReadKey(stream);
    }
    fWaitingForClick = stream->ReadBool();

    unsigned numRecipients = stream->ReadLE16();
    for (unsigned i = 0; i < numRecipients; i++)
        fRecipients.push_back(mgr->ReadKey(stream));
}

// Write -------------------------------------------------
// ------
void plAvBrainCoop::Write(hsStream *stream, hsResMgr *mgr)
{
    plAvBrainGeneric::Write(stream, mgr);

    stream->WriteLE32(fInitiatorID);
    stream->WriteLE16(fInitiatorSerial);

    bool hasHostKey = (fHostKey != nullptr);
    bool hasGuestKey = (fGuestKey != nullptr);

    stream->WriteBool(hasHostKey);
    if(hasHostKey)
        mgr->WriteKey(stream, fHostKey);

    stream->WriteBool(hasGuestKey);
    if(hasGuestKey)
        mgr->WriteKey(stream, fGuestKey);

    stream->WriteBool(fWaitingForClick);

    size_t numRecipients = fRecipients.size();
    hsAssert(numRecipients < std::numeric_limits<uint16_t>::max(), "Too many recipients");
    stream->WriteLE16(uint16_t(numRecipients));
    for (const plKey& recipient : fRecipients)
        mgr->WriteKey(stream, recipient);
}

plKey plAvBrainCoop::GetRecipient()
{
    if (fRecipients.size() == 0)
        return nullptr;
    return fRecipients[0];
}
