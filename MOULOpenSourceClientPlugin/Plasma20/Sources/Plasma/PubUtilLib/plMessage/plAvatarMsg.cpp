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
#ifndef NO_AV_MSGS

#include "plAvatarMsg.h"

#include "hsResMgr.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plSceneObject.h"

#ifndef SERVER
#include "../plAvatar/plAvBrain.h"
#endif


//////////////
// PLAVATARMSG
//////////////

// CTOR()
plAvatarMsg::plAvatarMsg()
: plMessage()
{
}

// CTOR(sender, receiver, time)
plAvatarMsg::plAvatarMsg(const plKey &sender, const plKey &receiver)
: plMessage(sender, receiver, nil)
{
}

// READ
void plAvatarMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgRead(stream, mgr);
}

// WRITE
void plAvatarMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgWrite(stream, mgr);
}

//////////////////////
// PLARMATUREUPDATEMSG
//////////////////////

// CTOR()
plArmatureUpdateMsg::plArmatureUpdateMsg()
: fIsInvis(false)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}

// CTOR sender receiver islocal isplayercontrolled
plArmatureUpdateMsg::plArmatureUpdateMsg(const plKey &sender,
										 hsBool isLocal, hsBool isPlayerControlled,
										 plArmatureMod *armature)
: plAvatarMsg(sender, nil),
  fIsLocal(isLocal),
  fIsPlayerControlled(isPlayerControlled),
  fArmature(armature),
  fIsInvis(false)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}

// READ stream mgr
void plArmatureUpdateMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}

// WRITE stream mgr
void plArmatureUpdateMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}

// ISLOCAL
hsBool plArmatureUpdateMsg::IsLocal() const
{
	return fIsLocal;
}

// ISPLAYERCONTROLLED
hsBool plArmatureUpdateMsg::IsPlayerControlled() const
{
	return fIsPlayerControlled;
}

hsBool plArmatureUpdateMsg::IsInvis() const
{
	return fIsInvis;
}

/////////////////////
// PLAVATARSETTYPEMSG
/////////////////////

// ctor
plAvatarSetTypeMsg::plAvatarSetTypeMsg()
: fIsPlayer(false)
{
}

plAvatarSetTypeMsg::plAvatarSetTypeMsg(const plKey &sender, const plKey &receiver)
: plAvatarMsg(sender, receiver),
  fIsPlayer(false)
{
}

// READ
void plAvatarSetTypeMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	fIsPlayer = stream->Readbool();
}

// WRITE
void plAvatarSetTypeMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	stream->Writebool(fIsPlayer);
}

// SETISPLAYER
void plAvatarSetTypeMsg::SetIsPlayer(bool is)
{
	fIsPlayer = is;
}

// ISPLAYER
bool plAvatarSetTypeMsg::IsPlayer()
{
	return fIsPlayer;
}



//////////////
// PLAVTASKMSG
//////////////

plAvTaskMsg::plAvTaskMsg()
: plAvatarMsg(), fTask(nil)
{
}

plAvTaskMsg::plAvTaskMsg(const plKey &sender, const plKey &receiver)
: plAvatarMsg(sender, receiver), fTask(nil)
{
}

plAvTaskMsg::plAvTaskMsg(const plKey &sender, const plKey &receiver, plAvTask *task)
: plAvatarMsg(sender, receiver),
  fTask(task)
{
}

plAvTask *plAvTaskMsg::GetTask()
{
	return fTask;
}

// READ
void plAvTaskMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Read(stream, mgr);
	if(stream->ReadBool())
		fTask = (plAvTask *)mgr->ReadCreatable(stream);

}

// WRITE
void plAvTaskMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Write(stream, mgr);
	if(fTask)
	{
		stream->WriteBool(true);
		mgr->WriteCreatable(stream, (plCreatable *)fTask);
	} else {
		stream->WriteBool(false);
	}
}

//////////////
//
// PLAVSEEKMSG
//
//////////////
// Tell the avatar to go to a specific seekpoint.
// The given key (seekKey) must be to a plSeekPointMod

// CTOR()
plAvSeekMsg::plAvSeekMsg()
: plAvTaskMsg(),
  fSeekPoint(nil),
  fDuration(0),
  fSmartSeek(true),
  fAlignType(kAlignHandle),
  fAnimName(nil),
  fNoSeek(false),
  fFlags(kSeekFlagForce3rdPersonOnStart)
{
}

// CTOR(sender, receiver, seekKey, time)
plAvSeekMsg::plAvSeekMsg(const plKey& sender, const plKey& receiver,
						 const plKey &seekKey, float duration, hsBool smartSeek,
						 plAvAlignment alignType, char *animName, hsBool noSeek, 
						 UInt8 flags, plKey finishKey)
: plAvTaskMsg(sender, receiver),
  fSeekPoint(seekKey),
  fTargetPos(0, 0, 0),
  fTargetLookAt(0, 0, 0),
  fDuration(duration),
  fSmartSeek(smartSeek),
  fAnimName(animName),
  fAlignType(alignType),
  fNoSeek(noSeek),
  fFlags(flags),
  fFinishKey(finishKey)
{
}

hsBool plAvSeekMsg::Force3rdPersonOnStart()
{
	return fFlags & kSeekFlagForce3rdPersonOnStart;
}

hsBool plAvSeekMsg::UnForce3rdPersonOnFinish()
{
	return fFlags & kSeekFlagUnForce3rdPersonOnFinish;
}

hsBool plAvSeekMsg::NoWarpOnTimeout()
{
	return fFlags & kSeekFlagNoWarpOnTimeout;
}

hsBool plAvSeekMsg::RotationOnly()
{
	return fFlags & kSeekFlagRotationOnly;
}

// READ
void plAvSeekMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvTaskMsg::Read(stream, mgr);

	fSeekPoint = mgr->ReadKey(stream);
	if (!fSeekPoint)
	{
		fTargetPos.Read(stream);
		fTargetLookAt.Read(stream);
	}

	fDuration = stream->ReadSwapScalar();
	fSmartSeek = stream->ReadBool();
	fAnimName = stream->ReadSafeString();
	fAlignType = static_cast<plAvAlignment>(stream->ReadSwap16());
	fNoSeek = stream->ReadBool();
	fFlags = stream->ReadByte();
	fFinishKey = mgr->ReadKey(stream);
}

// WRITE
void plAvSeekMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvTaskMsg::Write(stream, mgr);

	mgr->WriteKey(stream, fSeekPoint);
	if (!fSeekPoint)
	{
		fTargetPos.Write(stream);
		fTargetLookAt.Write(stream);
	}

	stream->WriteSwapScalar(fDuration);
	stream->WriteBool(fSmartSeek);
	stream->WriteSafeString(fAnimName);
	stream->WriteSwap16(static_cast<UInt16>(fAlignType));
	stream->WriteBool(fNoSeek);
	stream->WriteByte(fFlags);
	mgr->WriteKey(stream, fFinishKey);
}

/////////////////////////////////////////////////////////////////////////////////////////

void plAvTaskSeekDoneMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Read(stream, mgr);

	fAborted = stream->ReadBool();
}

void plAvTaskSeekDoneMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Write(stream, mgr);

	stream->WriteBool(fAborted);
}

/////////////////
//
// PLAVONESHOTMSG
//
/////////////////

#include "../plMessage/plOneShotCallbacks.h"

// CTOR()
plAvOneShotMsg::plAvOneShotMsg()
: plAvSeekMsg(), fAnimName(nil), fDrivable(false), fReversible(false), fCallbacks(nil)
{
}

// CTOR(sender, receiver, seekKey, time)
plAvOneShotMsg::plAvOneShotMsg(const plKey &sender, const plKey& receiver,
						 const plKey& seekKey, float duration, hsBool smartSeek,
						 const char *animName, hsBool drivable, hsBool reversible)
: plAvSeekMsg(sender, receiver, seekKey, duration, smartSeek),
  fDrivable(drivable), fReversible(reversible), fCallbacks(nil)
{
	fAnimName = hsStrcpy(animName);
}

// DTOR
plAvOneShotMsg::~plAvOneShotMsg()
{
	hsRefCnt_SafeUnRef(fCallbacks);
	fCallbacks = nil;

	delete[] fAnimName;
}

// READ
void plAvOneShotMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvSeekMsg::Read(stream, mgr);

	delete [] fAnimName;
	fAnimName = stream->ReadSafeString();
	fDrivable = stream->ReadBool();
	fReversible = stream->ReadBool();
}

// WRITE
void plAvOneShotMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvSeekMsg::Write(stream, mgr);

	stream->WriteSafeString(fAnimName);
	stream->WriteBool(fDrivable);
	stream->WriteBool(fReversible);
}


////////////////
//
// plAvBrainGenericMsg
//
////////////////

// default CTOR
plAvBrainGenericMsg::plAvBrainGenericMsg()
: fType(kNextStage),		// default verb is goto next stage
  fWhichStage(0),			// default stage is 0
  fSetTime(false),			// don't set the time of the target stage
  fNewTime(0.0f),			// if we do set, set it to zero
  fSetDirection(false),		// don't set the direction of the brain
  fNewDirection(true),		// if we do set the direction, set it to forward
  fNewLoopCount(0)
{
}

// canonical CTOR sender receiver type stage rewind transitionTime
plAvBrainGenericMsg::plAvBrainGenericMsg(const plKey& sender, const plKey &receiver,
					plAvBrainGenericMsg::Type type, int stage, hsBool rewind, hsScalar transitionTime)
: plAvatarMsg(sender, receiver),
  fType(type),
  fWhichStage(stage),
  fSetTime(rewind),
  fNewTime(0.0f),
  fSetDirection(false),
  fNewDirection(true),
  fNewLoopCount(0)
{
	
}

plAvBrainGenericMsg::plAvBrainGenericMsg(const plKey& sender, const plKey &receiver,
										 Type type, int stage, hsBool setTime, hsScalar newTime,
										 hsBool setDirection, bool isForward, hsScalar transitiontime)
: plAvatarMsg(sender, receiver),
  fType(type),
  fWhichStage(stage),
  fSetTime(setTime),
  fNewTime(newTime),
  fSetDirection(setDirection),
  fNewDirection(isForward),
  fNewLoopCount(0)
{
}

plAvBrainGenericMsg::plAvBrainGenericMsg(plKey sender, plKey receiver,
										 Type type, int stage, int newLoopCount)
: plAvatarMsg(sender, receiver),
  fType(type),
  fWhichStage(stage),
  fSetTime(false),					// unused
  fNewTime(0),						// unused
  fSetDirection(false),				// unused
  fNewDirection(false),				// unused
  fNewLoopCount(newLoopCount)
{
	hsAssert(type == kSetLoopCount, "This constructor form is only for the kSetLoopCount command.");
}


void plAvBrainGenericMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Write(stream, mgr);
	stream->WriteSwap32(fType);
	stream->WriteSwap32(fWhichStage);
	stream->WriteBool(fSetTime);
	stream->WriteSwapScalar(fNewTime);
	stream->WriteBool(fSetDirection);
	stream->WriteBool(fNewDirection);
	stream->WriteSwapScalar(fTransitionTime);
}

void plAvBrainGenericMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvatarMsg::Read(stream, mgr);
	fType = static_cast<plAvBrainGenericMsg::Type>(stream->ReadSwap32());
	fWhichStage = stream->ReadSwap32();
	fSetTime = stream->ReadBool();
	fNewTime = stream->ReadSwapScalar();
	fSetDirection = stream->ReadBool();
	fNewDirection = stream->ReadBool();
	fTransitionTime = stream->ReadSwapScalar();
}

enum AvBrainGenericFlags
{
	kAvBrainGenericType,
	kAvBrainGenericWhich,
	kAvBrainGenericSetTime,
	kAvBrainGenericNewTime,
	kAvBrainGenericSetDir,
	kAvBrainGenericNewDir,
	kAvBrainGenericTransTime,
};

void plAvBrainGenericMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kAvBrainGenericType);
	contentFlags.SetBit(kAvBrainGenericWhich);
	contentFlags.SetBit(kAvBrainGenericSetTime);
	contentFlags.SetBit(kAvBrainGenericNewTime);
	contentFlags.SetBit(kAvBrainGenericSetDir);
	contentFlags.SetBit(kAvBrainGenericNewDir);
	contentFlags.SetBit(kAvBrainGenericTransTime);
	contentFlags.Write(s);

	// kAvBrainGenericType
	s->WriteSwap32(fType);
	// kAvBrainGenericWhich	
	s->WriteSwap32(fWhichStage);
	// kAvBrainGenericSetTime	
	s->WriteBool(fSetTime);
	// kAvBrainGenericNewTime	
	s->WriteSwapScalar(fNewTime);
	// kAvBrainGenericSetDir	
	s->WriteBool(fSetDirection);
	// kAvBrainGenericNewDir	
	s->WriteBool(fNewDirection);
	// kAvBrainGenericTransTime	
	s->WriteSwapScalar(fTransitionTime);
}

void plAvBrainGenericMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kAvBrainGenericType))
		fType = static_cast<plAvBrainGenericMsg::Type>(s->ReadSwap32());
	if (contentFlags.IsBitSet(kAvBrainGenericWhich))
		fWhichStage = s->ReadSwap32();
	if (contentFlags.IsBitSet(kAvBrainGenericSetTime))
		fSetTime = s->ReadBool();
	if (contentFlags.IsBitSet(kAvBrainGenericNewTime))
		fNewTime = s->ReadSwapScalar();
	if (contentFlags.IsBitSet(kAvBrainGenericSetDir))
		fSetDirection = s->ReadBool();
	if (contentFlags.IsBitSet(kAvBrainGenericNewDir))
		fNewDirection = s->ReadBool();
	if (contentFlags.IsBitSet(kAvBrainGenericTransTime))
		fTransitionTime = s->ReadSwapScalar();
}

///////////////////
//
// PLAVPUSHBRAINMSG
//
///////////////////

#ifndef SERVER

// default ctor
plAvPushBrainMsg::plAvPushBrainMsg()
: fBrain(nil)
{
}

// canonical ctor
plAvPushBrainMsg::plAvPushBrainMsg(const plKey& sender, const plKey &receiver, plArmatureBrain *brain)
: plAvTaskMsg(sender, receiver)
{
	fBrain = brain;
}

// dtor
plAvPushBrainMsg::~plAvPushBrainMsg()
{
}

// READ
void plAvPushBrainMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvTaskMsg::Read(stream, mgr);

	fBrain = plArmatureBrain::ConvertNoRef(mgr->ReadCreatable(stream));
	hsAssert(fBrain, "PLAVPUSHBRAINMSG: Problem reading brain from stream.");
}

// WRITE
void plAvPushBrainMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvTaskMsg::Write(stream, mgr);
	mgr->WriteCreatable(stream, fBrain);
}



//////////////////
//
// PLAVPOPBRAINMSG
//
//////////////////

// default ctor
plAvPopBrainMsg::plAvPopBrainMsg()
{
}

// canonical ctor
plAvPopBrainMsg::plAvPopBrainMsg(const plKey &sender, const plKey &receiver)
: plAvTaskMsg(sender, receiver)
{
}

#endif // SERVER


/////////////////
////
//// PLAVEMOTEMSG
////
/////////////////
//
//// default ctor
//plAvEmoteMsg::plAvEmoteMsg()
//: fAnimName(nil)
//{
//}
//
//// canonical ctor
//plAvEmoteMsg::plAvEmoteMsg(plKey sender, plKey receiver, char *name)
//: plAvTaskMsg(sender, receiver)
//{
//	fAnimName = hsStrcpy(name);
//}
//
//// READ
//void plAvEmoteMsg::Read(hsStream *stream, hsResMgr *mgr)
//{
//	plAvTaskMsg::Read(stream, mgr);
//	fAnimName = stream->ReadSafeString();
//}
//
//// WRITE
//void plAvEmoteMsg::Write(hsStream *stream, hsResMgr *mgr)
//{
//	plAvTaskMsg::Write(stream, mgr);
//	stream->WriteSafeString(fAnimName);




///////////////////////////
//
// PLAVATARSTEALTHMODEMSG
//
///////////////////////////

plAvatarStealthModeMsg::plAvatarStealthModeMsg() : plAvatarMsg(), fMode(kStealthVisible), fLevel(0) 
{ 
	SetBCastFlag(plMessage::kBCastByExactType); 
}

plAvatarStealthModeMsg::~plAvatarStealthModeMsg() {}

// READ stream mgr
void plAvatarStealthModeMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}

// WRITE stream mgr
void plAvatarStealthModeMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}


#endif // ndef NO_AV_MSGS
