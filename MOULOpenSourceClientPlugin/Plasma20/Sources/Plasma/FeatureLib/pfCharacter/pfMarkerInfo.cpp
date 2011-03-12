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
#include "pfMarkerInfo.h"
#include "pfMarkerMgr.h"

#include "../plModifier/plGameMarkerModifier.h"

#include "../plMessage/plLoadCloneMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnSceneObject/plAudioInterface.h"

// For Init
#include "../pnMessage/plClientMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plKeyFinder.h"

plUoid pfMarkerInfo::fMarkerUoid;

static const int kFreezeLen = 10;		// How long a marker is frozen after you hit it

void pfMarkerInfo::Init()
{
	plResManager* resMgr = (plResManager*)hsgResMgr::ResMgr();

	// Force the client to keep the GlobalMarkers keys loaded, so we don't load them every time we clone
	plClientMsg* loadAgeKeysMsg = TRACKED_NEW plClientMsg(plClientMsg::kLoadAgeKeys);
	loadAgeKeysMsg->SetAgeName("GlobalMarkers");
	loadAgeKeysMsg->Send(resMgr->FindKey(kClient_KEY));

	//
	// Get the Uoid for the markers
	//
	plLocation markerLoc = plKeyFinder::Instance().FindLocation("GlobalMarkers", "Markers");

	if (markerLoc.IsValid())
		fMarkerUoid	= plUoid(markerLoc, plSceneObject::Index(), "MarkerRoot");
	else
		fMarkerUoid.Invalidate();
}

pfMarkerInfo::pfMarkerInfo(const hsPoint3& pos, bool isNew) :
	fMod(nil),
	fPosition(pos),
	fType(kMarkerOpen),
	fLastChange(0),
	fVisible(true),
	fIsNew(isNew),
	fSpawned(false)
{
}

void pfMarkerInfo::Spawn(MarkerType type)
{
	if (!fMarkerUoid.IsValid())
	{
		plResManager* resMgr = (plResManager*)hsgResMgr::ResMgr();
		plLocation markerLoc = plKeyFinder::Instance().FindLocation("GlobalMarkers", "Markers");

		if (markerLoc.IsValid())
			fMarkerUoid	= plUoid(markerLoc, plSceneObject::Index(), "MarkerRoot");
		else
		{
			hsAssert(false, "Unable to spawn markers because the marker age was not loaded or found");
			return;
		}
	}

	fType = type;
	fLastChange = 0;

	plLoadCloneMsg* cloneMsg = TRACKED_NEW plLoadCloneMsg(fMarkerUoid, pfMarkerMgr::Instance()->GetKey(), 0);
	cloneMsg->SetBCastFlag(plMessage::kNetPropagate, false);
	fKey = cloneMsg->GetCloneKey();

	cloneMsg->Send();
}

void pfMarkerInfo::InitSpawned(plKey markerKey)
{
	fKey = markerKey;
	fSpawned = true;

	plSceneObject* so = plSceneObject::ConvertNoRef(fKey->GetObjectPtr());
	fMod = (plGameMarkerModifier*)so->GetModifierByType(plGameMarkerModifier::Index());
	hsAssert(fMod, "Couldn't find marker modifier");
	fMod->FixupAnimKeys();

	// Warp it into position
	hsMatrix44 pos;
	pos.Reset();
	pos.SetTranslate(&fPosition);
	plWarpMsg* warpMsg = TRACKED_NEW plWarpMsg(pfMarkerMgr::Instance()->GetKey(), fKey, plWarpMsg::kFlushTransform, pos);
	warpMsg->Send();

	// update its state
	Show(fVisible);
	IPlayColor(true);
	if (fType == kMarkerLocalSelected)
		IPlayBounce(true);
}

void pfMarkerInfo::Show(bool show)
{
	fVisible = show;

	if (fSpawned) {
		plEnableMsg* msg = TRACKED_NEW plEnableMsg;
		msg->SetBCastFlag(plMessage::kPropagateToChildren);
		msg->SetCmd(plEnableMsg::kDrawable);
		msg->SetCmd(show ? plEnableMsg::kEnable : plEnableMsg::kDisable);
		msg->SetSender(pfMarkerMgr::Instance()->GetKey());
		msg->Send(fKey);
	}
}

void pfMarkerInfo::SetFrozen(double freezeStartTime)
{
	fLastChange = freezeStartTime;
	IPlayBounce(true);
}

void pfMarkerInfo::Update(double curTime)
{
	if (fLastChange != 0 && (curTime - fLastChange) > kFreezeLen)
	{
		fLastChange = 0;
		IPlayBounce(false);
	}

	if (fIsNew)
	{
		IPlaySound(true);
		fIsNew = false;
	}
}

void pfMarkerInfo::Remove()
{
	if (fKey)
	{
		plLoadCloneMsg* cloneMsg = TRACKED_NEW plLoadCloneMsg(fKey, pfMarkerMgr::Instance()->GetKey(), 0, false);
		cloneMsg->SetBCastFlag(plMessage::kNetPropagate, false);
		cloneMsg->Send();

		fKey = nil;
		fMod = nil;
	}
}

void pfMarkerInfo::SetType(pfMarkerInfo::MarkerType type)
{
	if (fType == kMarkerLocalSelected)
		IPlayBounce(false);

	IPlayColor(false);
	fType = type;
	IPlayColor(true);

	if (fType == kMarkerLocalSelected)
		IPlayBounce(true);
}

void pfMarkerInfo::IPlayBounce(bool play)
{
	if (fMod && fSpawned)
	{
		// Send anim start/stop msg
		plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
		animMsg->SetCmd(play ? plAnimCmdMsg::kContinue : plAnimCmdMsg::kStop);
		animMsg->SetCmd(plAnimCmdMsg::kSetLooping);
		animMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
		animMsg->SetSender(pfMarkerMgr::Instance()->GetKey());
		animMsg->Send(fMod->fBounceAnimKey);
	}
}

void pfMarkerInfo::IPlayColor(bool play)
{
	if (fMod && fSpawned)
	{
		// Play the correct color anim
		plKey key = nil;
		switch (fType)
		{
		case kMarkerOpen:
		case kMarkerLocal:
		case kMarkerLocalSelected:
			key = fMod->fOpenAnimKey;
			break;

		case kMarkerGreen:
			key = fMod->fGreenAnimKey;
			break;

		case kMarkerRed:
			key = fMod->fRedAnimKey;
			break;
		}

		plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
		animMsg->SetCmd(play ? plAnimCmdMsg::kContinue : plAnimCmdMsg::kStop);
		animMsg->SetCmd(plAnimCmdMsg::kSetLooping);
		animMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
		animMsg->SetSender(pfMarkerMgr::Instance()->GetKey());
		animMsg->AddReceiver(key);
		animMsg->Send();
	}
}

void pfMarkerInfo::IPlaySound(bool place)
{
	if (fMod && fSpawned)
	{
		const plAudioInterface* ai = fMod->GetTarget()->GetAudioInterface();

		plSoundMsg* msg = TRACKED_NEW plSoundMsg;
		msg->fIndex = place ? fMod->fPlaceSndIdx : fMod->fHitSndIdx;
		msg->SetCmd(plSoundMsg::kPlay);
		msg->SetSender(pfMarkerMgr::Instance()->GetKey());
		msg->Send(ai->GetKey());
	}
}