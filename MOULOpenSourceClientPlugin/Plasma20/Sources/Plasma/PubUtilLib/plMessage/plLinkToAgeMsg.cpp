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
#include "hsStream.h"
#include "plLinkToAgeMsg.h"
#include "hsResMgr.h"
#include "hsUtils.h"
#include "plgDispatch.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plNetCommon.h"
#include "hsBitVector.h"


/////////////////////////////////////////////////////////////////////////
//
// plLinkToAgeMsg

plLinkToAgeMsg::plLinkToAgeMsg() : fLinkInAnimName(nil)
{
}

plLinkToAgeMsg::plLinkToAgeMsg( const plAgeLinkStruct * link ) : fLinkInAnimName(nil)
{
	fAgeLink.CopyFrom( link );
}

plLinkToAgeMsg::~plLinkToAgeMsg()
{
	delete [] fLinkInAnimName;
}

// StreamVersion needed for back compatibility.
UInt8	plLinkToAgeInfo_StreamVersion = 0;
void plLinkToAgeMsg::Read(hsStream* stream, hsResMgr* mgr)	
{
	plMessage::IMsgRead( stream, mgr );
	UInt8 ltaVer = stream->ReadByte();
	fAgeLink.Read( stream, mgr );
	fLinkInAnimName = stream->ReadSafeString();
}

void plLinkToAgeMsg::Write(hsStream* stream, hsResMgr* mgr)	
{
	plMessage::IMsgWrite( stream, mgr );
	stream->WriteByte( plLinkToAgeInfo_StreamVersion );
	fAgeLink.Write( stream, mgr );
	stream->WriteSafeString(fLinkInAnimName);
}

enum LinkToAgeFlags
{
	kLinkToAgeAgeLinkStruct,
	kLinkToAgeLinkAnimName,
};

void plLinkToAgeMsg::ReadVersion(hsStream* s, hsResMgr* mgr)	
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if ( contentFlags.IsBitSet( kLinkToAgeAgeLinkStruct ) )
		fAgeLink.Read( s, mgr );
	if ( contentFlags.IsBitSet( kLinkToAgeLinkAnimName ) )
		fLinkInAnimName = s->ReadSafeString();
}

void plLinkToAgeMsg::WriteVersion(hsStream* s, hsResMgr* mgr)	
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kLinkToAgeAgeLinkStruct);
	contentFlags.SetBit(kLinkToAgeLinkAnimName);
	contentFlags.Write(s);

	// write kLinkToAgeAgeLinkStruct
	fAgeLink.Write( s, mgr );
	s->WriteSafeString(fLinkInAnimName);
}


/////////////////////////////////////////////////////////////////////////////
//
//  plLinkingMgrMsg

plLinkingMgrMsg::plLinkingMgrMsg()
: fLinkingMgrCmd( 0 /*plNetLinkingMgr::kNilCmd*/ )
{
}

plLinkingMgrMsg::~plLinkingMgrMsg()
{
}

enum LinkingMgrMsgFlags
{
	kLinkingMgrCmd,
	kLinkingMgrArgs,
};

void plLinkingMgrMsg::Read( hsStream* stream, hsResMgr* mgr )
{
	plMessage::IMsgRead( stream, mgr );

	hsBitVector contentFlags;
	contentFlags.Read( stream );

	if ( contentFlags.IsBitSet( kLinkingMgrCmd ) )
		stream->ReadSwap( &fLinkingMgrCmd );
	if ( contentFlags.IsBitSet( kLinkingMgrArgs ) )
		fArgs.Read( stream, mgr );
}

void plLinkingMgrMsg::Write( hsStream* stream, hsResMgr* mgr )
{
	plMessage::IMsgWrite( stream, mgr );

	hsBitVector contentFlags;
	contentFlags.SetBit( kLinkingMgrCmd );
	contentFlags.SetBit( kLinkingMgrArgs );
	contentFlags.Write( stream );

	stream->WriteSwap( fLinkingMgrCmd );
	fArgs.Write( stream, mgr );
}

void plLinkingMgrMsg::ReadVersion( hsStream* s, hsResMgr* mgr )
{
	// Read() does the contentFlags thing.
	Read( s, mgr );
}

void plLinkingMgrMsg::WriteVersion( hsStream* s, hsResMgr* mgr )
{
	// Write() does the contentFlags thing.
	Write( s, mgr );
}


/////////////////////////////////////////////////////////////////////////////
//
//  plLinkEffectsTriggerMsg

plLinkEffectsTriggerMsg::~plLinkEffectsTriggerMsg()
{
}

void plLinkEffectsTriggerMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead( stream, mgr );
	
	fInvisLevel = stream->ReadSwap32();
	fLeavingAge = stream->ReadBool();
	fLinkKey = mgr->ReadKey(stream);

	// This variable is for internal use only. Still read/written for backwards compatability.
	fEffects = stream->ReadSwap32();
	fEffects = 0; 

	fLinkInAnimKey = mgr->ReadKey(stream);
}

void plLinkEffectsTriggerMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite( stream, mgr );
	
	stream->WriteSwap32(fInvisLevel);
	stream->WriteBool(fLeavingAge);
	mgr->WriteKey(stream, fLinkKey);
	stream->WriteSwap32(fEffects);
	mgr->WriteKey(stream, fLinkInAnimKey);
}

enum LinkEffectsFlags
{
	kLinkEffectsLeavingAge,
	kLinkEffectsLinkKey,
	kLinkEffectsEffects,
	kLinkEffectsLinkInAnimKey,
};

#include "../pnNetCommon/plNetApp.h"
void plLinkEffectsTriggerMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);
	
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kLinkEffectsLeavingAge))
		fLeavingAge = s->ReadBool();
	if (contentFlags.IsBitSet(kLinkEffectsLinkKey))
		fLinkKey = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kLinkEffectsEffects))
		fEffects = s->ReadSwap32();
	if (contentFlags.IsBitSet(kLinkEffectsLinkInAnimKey))
		fLinkInAnimKey = mgr->ReadKey(s);

	if (plNetClientApp::GetInstance() && fLinkKey == plNetClientApp::GetInstance()->GetLocalPlayerKey())
	{
		SetBCastFlag(plMessage::kNetStartCascade, true);
		SetBCastFlag(plMessage::kNetNonLocal | plMessage::kNetPropagate, false);
	}
}

void plLinkEffectsTriggerMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);
	
	hsBitVector contentFlags;
	contentFlags.SetBit(kLinkEffectsLeavingAge);
	contentFlags.SetBit(kLinkEffectsLinkKey);
	contentFlags.SetBit(kLinkEffectsEffects);
	contentFlags.SetBit(kLinkEffectsLinkInAnimKey);
	contentFlags.Write(s);

	// kLinkEffectsLeavingAge
	s->WriteBool(fLeavingAge);
	// kLinkEffectsLinkKey
	mgr->WriteKey(s, fLinkKey);
	// kLinkEffectsEffects
	s->WriteSwap32(fEffects);
	// kLinkEffectsLinkInAnimKey
	mgr->WriteKey(s, fLinkInAnimKey);
}

void plLinkEffectsTriggerMsg::SetLinkKey(plKey &key)
{
	fLinkKey = key; 
}

void plLinkEffectsTriggerMsg::SetLinkInAnimKey(plKey &key)
{
	fLinkInAnimKey = key;
}

/////////////////////////////////////////////////////////////////////////////
//
//  plLinkEffectsPrepMsg

plLinkEffectsTriggerPrepMsg::~plLinkEffectsTriggerPrepMsg()
{
	if (fTrigger)
		hsRefCnt_SafeUnRef(fTrigger);
}

void plLinkEffectsTriggerPrepMsg::SetTrigger(plLinkEffectsTriggerMsg *msg)
{
	if (fTrigger)
		hsRefCnt_SafeUnRef(fTrigger);
	
	hsRefCnt_SafeRef(msg);
	fTrigger = msg;
}

/////////////////////////////////////////////////////////////////////////////
//
// plLinkEffectBCMsg

plLinkEffectBCMsg::plLinkEffectBCMsg() : fLinkKey(nil), fLinkFlags(0) { SetBCastFlag(plMessage::kBCastByExactType); }

void plLinkEffectBCMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);

	fLinkKey = mgr->ReadKey(stream);
	fLinkFlags = stream->ReadSwap32();
}

void plLinkEffectBCMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	mgr->WriteKey(stream, fLinkKey);
	stream->WriteSwap32(fLinkFlags);
}

void plLinkEffectBCMsg::SetLinkFlag(UInt32 flag, hsBool on /* = true */)
{
	if (on)
		fLinkFlags |= flag;
	else
		fLinkFlags &= ~flag;
}

hsBool plLinkEffectBCMsg::HasLinkFlag(UInt32 flag)
{
	return fLinkFlags & flag;
}

/////////////////////////////////////////////////////////////////////////////
//
// plLinkEffectPrepBCMsg

plLinkEffectPrepBCMsg::plLinkEffectPrepBCMsg() : fLinkKey(nil), fLeavingAge(false) { SetBCastFlag(plMessage::kBCastByExactType); }

/////////////////////////////////////////////////////////////////////////////
//
// plLinkCallbacks

void plLinkCallbackMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plEventCallbackMsg::Read(stream, mgr);

	fLinkKey = mgr->ReadKey(stream);
}

void plLinkCallbackMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plEventCallbackMsg::Write(stream, mgr);

	mgr->WriteKey(stream, fLinkKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkEffectMsg

plPseudoLinkEffectMsg::plPseudoLinkEffectMsg() : fLinkObjKey(nil), fAvatarKey(nil)
{
	SetBCastFlag(plMessage::kNetPropagate | plMessage::kBCastByExactType);
}

void plPseudoLinkEffectMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fLinkObjKey = mgr->ReadKey(stream);
	fAvatarKey = mgr->ReadKey(stream);
}

void plPseudoLinkEffectMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	mgr->WriteKey(stream, fLinkObjKey);
	mgr->WriteKey(stream, fAvatarKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkAnimTriggerMsg

plPseudoLinkAnimTriggerMsg::plPseudoLinkAnimTriggerMsg() : fForward(false)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}


plPseudoLinkAnimTriggerMsg::plPseudoLinkAnimTriggerMsg(hsBool forward, plKey avatarKey)
{
	fForward = forward;
	fAvatarKey = avatarKey;
	SetBCastFlag(plMessage::kBCastByExactType);
}

void plPseudoLinkAnimTriggerMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream,mgr);
	fForward = stream->ReadBool();
	fAvatarKey = mgr->ReadKey(stream);
}

void plPseudoLinkAnimTriggerMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream,mgr);
	stream->WriteBool(fForward);
	mgr->WriteKey(stream, fAvatarKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkAnimCallbackMsg


void plPseudoLinkAnimCallbackMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream,mgr);
	mgr->WriteKey(stream, fAvatarKey);
}

void plPseudoLinkAnimCallbackMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream,mgr);
	mgr->WriteKey(stream, fAvatarKey);
}