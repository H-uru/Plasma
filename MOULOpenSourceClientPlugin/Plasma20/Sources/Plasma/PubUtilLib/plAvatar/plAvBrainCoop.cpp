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
/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// singular
#include "plAvBrainCoop.h"

// local
#include "plArmatureMod.h"
#include "plAnimStage.h"
#include "plAvTaskSeek.h"
#include "plAvatarMgr.h"

// other
#include "../plScene/plSceneNode.h"
#include "../pnNetCommon/plNetApp.h"

// messages
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plAvCoopMsg.h"
#include "../plMessage/plPickedMsg.h"
#include "../pnMessage/plNotifyMsg.h"

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
plAvBrainCoop::plAvBrainCoop(UInt32 exitFlags, float fadeIn, float fadeOut,
							 MoveMode moveMode, plKey guestKey)
: plAvBrainGeneric(exitFlags, fadeIn, fadeOut, moveMode),
  fGuestKey(guestKey)
{
	static UInt16 coopSerial = 0;

	// this particular constructor is only called by the initiator...
	fInitiatorID = plNetClientApp::GetInstance()->GetPlayerID();
	fInitiatorSerial = coopSerial++;
}

// plAvBrainCoop ----------------------------------------------------------
// --------------
plAvBrainCoop::plAvBrainCoop(UInt32 exitFlags, float fadeIn, float fadeOut,
							 MoveMode moveMode,	UInt32 initiatorID, UInt16 initiatorSerial,
							 plKey hostKey)
: plAvBrainGeneric(exitFlags, fadeIn, fadeOut, moveMode),
  fInitiatorID(initiatorID), fInitiatorSerial(initiatorSerial),
  fHostKey(hostKey)
{
}

// MsgReceive ----------------------------------
// -----------
hsBool plAvBrainCoop::MsgReceive(plMessage *msg)
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
				plAvCoopMsg *coopM = TRACKED_NEW plAvCoopMsg(plAvCoopMsg::kGuestAccepted, fInitiatorID, fInitiatorSerial);
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
		DebugMsg("COOP: Relaying multi-stage event to %d recipients (via plAvBrainCoop)", fRecipients.size());

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
UInt32 plAvBrainCoop::GetInitiatorID()
{
	return fInitiatorID;
}

// GetInitiatorSerial --------------------
// -------------------
UInt16 plAvBrainCoop::GetInitiatorSerial()
{
	return fInitiatorSerial;
}

// Read -------------------------------------------------
// -----
void plAvBrainCoop::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvBrainGeneric::Read(stream, mgr);

	fInitiatorID = stream->ReadSwap32();
	fInitiatorSerial = stream->ReadSwap16();

	if(stream->Readbool())
	{
		fHostKey = mgr->ReadKey(stream);
	}
	if(stream->Readbool())
	{
		fGuestKey = mgr->ReadKey(stream);
	}
	fWaitingForClick = stream->Readbool();

	unsigned numRecipients = stream->ReadSwap16();
	for (unsigned i = 0; i < numRecipients; i++)
		fRecipients.push_back(mgr->ReadKey(stream));
}

// Write -------------------------------------------------
// ------
void plAvBrainCoop::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvBrainGeneric::Write(stream, mgr);

	stream->WriteSwap32(fInitiatorID);
	stream->WriteSwap16(fInitiatorSerial);

	bool hasHostKey = (fHostKey != nil);
	bool hasGuestKey = (fGuestKey != nil);

	stream->Writebool(hasHostKey);
	if(hasHostKey)
		mgr->WriteKey(stream, fHostKey);

	stream->Writebool(hasGuestKey);
	if(hasGuestKey)
		mgr->WriteKey(stream, fGuestKey);

	stream->Writebool(fWaitingForClick);

	stream->WriteSwap16(fRecipients.size());
	for (unsigned i = 0; i < fRecipients.size(); i++)
		mgr->WriteKey(stream, fRecipients[i]);
}

plKey plAvBrainCoop::GetRecipient()
{
	if (fRecipients.size() == 0)
		return nil;
	return fRecipients[0];
}

void plAvBrainCoop::SetRecipient(plKey &recipient)
{
	fRecipients.push_back(recipient);
}
