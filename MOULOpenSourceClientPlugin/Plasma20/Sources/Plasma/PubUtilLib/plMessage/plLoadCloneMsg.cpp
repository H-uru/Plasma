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
#ifndef SERVER

// singular
#include "plLoadAvatarMsg.h"

// global
#include "hsResMgr.h"

// other
#include "../pnNetCommon/plNetApp.h"



// CTOR / default
plLoadCloneMsg::plLoadCloneMsg()
: fValidMsg(false),
  fOriginatingPlayerID(0),
  fTriggerMsg(nil)
{
	SetBCastFlag(plMessage::kNetPropagate);
};

// CTOR uoidToClone, requestorKey, userData, isLoading
// this form is for creating new clones
plLoadCloneMsg::plLoadCloneMsg(const plUoid &uoidToClone, const plKey &requestorKey, UInt32 userData)
	: fRequestorKey(requestorKey),
	  fUserData(userData),
	  fValidMsg(false),
	  fTriggerMsg(nil),
	  fIsLoading(true)		// this constructor form is only used for loading
{
	SetBCastFlag(plMessage::kNetPropagate);
	
	hsKeyedObject * koRequestor = fRequestorKey->ObjectIsLoaded();
	if(koRequestor)
	{
		plNetClientApp *app = plNetClientApp::GetInstance();
		plKey originalKey = hsgResMgr::ResMgr()->FindKey(uoidToClone);

		if(originalKey)
		{
			// all is well. finish it off.
			fCloneKey = hsgResMgr::ResMgr()->CloneKey(originalKey);
			fOriginatingPlayerID = app->GetPlayerID();
			fValidMsg = true;
			this->AddReceiver(plNetApp::GetInstance()->GetKey());
		} else {
			char buffer[128];
			sprintf(buffer, "Can't find key named %s", uoidToClone.GetObjectName());				
			hsAssert(0, buffer);
		}
	} else {
		hsStatusMessage("Clone requestor is not loaded.");
	}
}

// CTOR existing, requestor, userData, isLoading
// this form is for unloading or other operations on existing clones
plLoadCloneMsg::plLoadCloneMsg(const plKey &existing, const plKey &requestor, UInt32 userData, hsBool isLoading)
: fCloneKey(existing),
  fRequestorKey(requestor),
  fUserData(userData),
  fValidMsg(true),
  fTriggerMsg(nil),
  fIsLoading(isLoading)
{
	if (plNetApp::GetInstance())
	{
		SetBCastFlag(plMessage::kNetPropagate);
		AddReceiver(plNetApp::GetInstance()->GetKey());
	}
	if (plNetClientApp::GetInstance())
		fOriginatingPlayerID = plNetClientApp::GetInstance()->GetPlayerID();
	hsAssert(fRequestorKey->ObjectIsLoaded(), "Clone (unloading) requestor is not loaded.");
}

plLoadCloneMsg::~plLoadCloneMsg()
{
	hsRefCnt_SafeUnRef(fTriggerMsg);
}

// READ
void plLoadCloneMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fCloneKey = mgr->ReadKey(stream);
	fRequestorKey = mgr->ReadKey(stream);
	fOriginatingPlayerID = stream->ReadSwap32();
	fUserData = stream->ReadSwap32();
	fValidMsg = stream->ReadBool();
	fIsLoading = stream->ReadBool();
	fTriggerMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
}

// WRITE
void plLoadCloneMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream,mgr);
	mgr->WriteKey(stream, fCloneKey);
	mgr->WriteKey(stream, fRequestorKey);
	stream->WriteSwap32(fOriginatingPlayerID);
	stream->WriteSwap32(fUserData);
	stream->WriteBool(fValidMsg);
	stream->WriteBool(fIsLoading);
	mgr->WriteCreatable(stream, fTriggerMsg);
}

enum LoadCloneMsgFlags
{
	kLoadCloneMsgCloneKey,
	kLoadCloneMsgRequestorKey,
	kLoadCloneMsgOrigPlayerID,
	kLoadCloneMsgUserData,
	kLoadCloneMsgValidMsg,
	kLoadCloneMsgIsLoading,
	kLoadCloneMsgTriggerMsg,
};

void plLoadCloneMsg::ReadVersion(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(stream, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(stream);

	if (contentFlags.IsBitSet(kLoadCloneMsgCloneKey))
		fCloneKey = mgr->ReadKey(stream);

	if (contentFlags.IsBitSet(kLoadCloneMsgRequestorKey))
		fRequestorKey = mgr->ReadKey(stream);

	if (contentFlags.IsBitSet(kLoadCloneMsgOrigPlayerID))
		fOriginatingPlayerID = stream->ReadSwap32();

	if (contentFlags.IsBitSet(kLoadCloneMsgUserData))
		fUserData = stream->ReadSwap32();

	if (contentFlags.IsBitSet(kLoadCloneMsgValidMsg))
		fValidMsg = stream->ReadBool();

	if (contentFlags.IsBitSet(kLoadCloneMsgIsLoading))
		fIsLoading = stream->ReadBool();

	if (contentFlags.IsBitSet(kLoadCloneMsgTriggerMsg))
		fTriggerMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
}

void plLoadCloneMsg::WriteVersion(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(stream,mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kLoadCloneMsgCloneKey);
	contentFlags.SetBit(kLoadCloneMsgRequestorKey);
	contentFlags.SetBit(kLoadCloneMsgOrigPlayerID);
	contentFlags.SetBit(kLoadCloneMsgUserData);
	contentFlags.SetBit(kLoadCloneMsgValidMsg);
	contentFlags.SetBit(kLoadCloneMsgIsLoading);
	contentFlags.SetBit(kLoadCloneMsgTriggerMsg);
	contentFlags.Write(stream);

	// kLoadCloneMsgCloneKey
	mgr->WriteKey(stream, fCloneKey);
	// kLoadCloneMsgRequestorKey
	mgr->WriteKey(stream, fRequestorKey);
	// kLoadCloneMsgOrigPlayerID
	stream->WriteSwap32(fOriginatingPlayerID);
	// kLoadCloneMsgUserData
	stream->WriteSwap32(fUserData);
	// kLoadCloneMsgValidMsg
	stream->WriteBool(fValidMsg);
	// kLoadCloneMsgIsLoading
	stream->WriteBool(fIsLoading);
	// kLoadCloneMsgTriggerMSg
	mgr->WriteCreatable(stream, fTriggerMsg);
}

// GETCLONEKEY
plKey plLoadCloneMsg::GetCloneKey()
{
	return fCloneKey;
}

// GETREQUESTORKEY
plKey plLoadCloneMsg::GetRequestorKey()
{
	return fRequestorKey;
}

// ISVALIDMESSAGE
hsBool plLoadCloneMsg::IsValidMessage()
{
	return fValidMsg;
}

// GETUSERDATA
UInt32 plLoadCloneMsg::GetUserData()
{
	return fUserData;
}

// GETORIGINATINGPLAYERID
UInt32 plLoadCloneMsg::GetOriginatingPlayerID()
{
	return fOriginatingPlayerID;
}

void plLoadCloneMsg::SetOriginatingPlayerID(UInt32 playerId)
{
	fOriginatingPlayerID = playerId;
}

hsBool plLoadCloneMsg::GetIsLoading()
{
	return fIsLoading;
}

void plLoadCloneMsg::SetTriggerMsg(plMessage *msg)
{
	if (fTriggerMsg != nil)
		hsRefCnt_SafeUnRef(fTriggerMsg);

	hsRefCnt_SafeRef(msg);
	fTriggerMsg = msg;
}

plMessage *plLoadCloneMsg::GetTriggerMsg()
{
	return fTriggerMsg;
}

#endif // ndef SERVER
#endif // ndef NO_AV_MSGS
