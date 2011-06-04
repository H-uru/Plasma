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

#include "hsTypes.h"
#include "plAudioInterface.h"
#include "plAudible.h"
#include "../pnMessage/plAudioSysMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "hsBounds.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plKey.h"
#include "plSceneObject.h"
#include "plgDispatch.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "plCoordinateInterface.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plProxyDrawMsg.h"
#include "../pnNetCommon/plNetApp.h"
#include "hsTimer.h"


plAudioInterface::plAudioInterface()
: fAudible(nil), fAudibleInited( false )
{
	fRegisteredForASysMsg = false;
}

plAudioInterface::~plAudioInterface()
{
	
}

void plAudioInterface::SetProperty(int prop, hsBool on)
{
	plObjInterface::SetProperty(prop, on);

	if( fAudible )
		fAudible->SetProperty(prop, on);
}

plSound* plAudioInterface::GetSound(int i) const
{
	if( fAudible )
		return fAudible->GetSound(i);
	return nil;
}

int plAudioInterface::GetSoundIndex(const char *keyname)
{
	if( fAudible )
		return fAudible->GetSoundIndex(keyname);
	else
		return -1;
}

int plAudioInterface::GetNumSounds() const
{
	if( fAudible )
		return fAudible->GetNumSounds();
	return 0;
}

// set the filename of sound[index] within the audible
void plAudioInterface::SetSoundFilename(int index, const char *filename, bool isCompressed)
{
	if(fAudible)
		fAudible->SetFilename(index, filename, isCompressed);
}

void plAudioInterface::ISetSceneNode(plKey key)
{
	if( fAudible )
	{
		fAudible->SetSceneNode(key);
		if( !fAudibleInited )
		{
			int isLocal = IsLocallyOwned();

			plKey localKey = ( plNetClientApp::GetInstance() != nil ) ? plNetClientApp::GetInstance()->GetLocalPlayerKey() : nil;
			if( fOwner && fOwner->GetKey() == localKey )
				isLocal = true;
			else
				isLocal = false;

			fAudible->Init( isLocal );//( isLocal == plSynchedObject::kYes ) );
			fAudibleInited = true;
		}
	}
}

void plAudioInterface::ISetOwner(plSceneObject* owner)
{
	plObjInterface::ISetOwner(owner);

	if( owner && !fRegisteredForASysMsg )
	{
		plgDispatch::Dispatch()->RegisterForExactType(plAudioSysMsg::Index(), GetKey());
		fRegisteredForASysMsg = true;
	}
	else if( owner == nil && fRegisteredForASysMsg )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plAudioSysMsg::Index(), GetKey());
		fRegisteredForASysMsg = false;
	}

	if (fAudible)
		fAudible->SetSceneObject(owner ? owner->GetKey() : nil);
}


void plAudioInterface::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fAudible )
		fAudible->SetTransform(l2w, w2l);
}

void plAudioInterface::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);

	plIntRefMsg* refMsg = TRACKED_NEW plIntRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kAudible);
	mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
}

void plAudioInterface::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);

	mgr->WriteKey(s,fAudible);
}

void plAudioInterface::ISetAudible(plAudible* aud)
{
	fAudible = aud;
	if( fAudible )
	{
		fAudible->SetSceneNode(GetSceneNode());
		if (fOwner)
			fAudible->SetSceneObject(fOwner->GetKey());
	}

	plAudioSysMsg* pMsg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kPing );
	pMsg->SetSender(GetKey());
//	pMsg->SetBCastFlag(plMessage::kBCastByExactType, false);
	plgDispatch::MsgSend( pMsg );
}

void plAudioInterface::IRemoveAudible(plAudible* aud)
{
	hsAssert(aud == fAudible, "Removing Audible I don't have");
	fAudible = nil;
}

hsBool plAudioInterface::MsgReceive(plMessage* msg)
{
	plIntRefMsg* intRefMsg = plIntRefMsg::ConvertNoRef(msg);
	if( intRefMsg )
	{
		switch( intRefMsg->fType )
		{
		case plIntRefMsg::kAudible:
			if( intRefMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				IRemoveAudible(plAudible::ConvertNoRef(intRefMsg->GetRef()));
			}
			else
			{
				ISetAudible(plAudible::ConvertNoRef(intRefMsg->GetRef()));
			}
			return true;
		default:
			break;
		}
	}

	plSoundMsg* pSoundMsg = plSoundMsg::ConvertNoRef( msg );
	if (pSoundMsg)
	{	
		if (!fAudible)
			return false;
		if (pSoundMsg->Cmd( plSoundMsg::kAddCallbacks))
			fAudible->AddCallbacks( pSoundMsg );
		if (pSoundMsg->Cmd( plSoundMsg::kRemoveCallbacks))
			fAudible->RemoveCallbacks( pSoundMsg );
		if (pSoundMsg->Cmd( plSoundMsg::kStop ) )
			fAudible->Stop(pSoundMsg->fIndex);
		if (pSoundMsg->Cmd( plSoundMsg::kGoToTime ) )
			fAudible->SetTime(pSoundMsg->fTime, pSoundMsg->fIndex);
		if (pSoundMsg->Cmd( plSoundMsg::kPlay ) )
			fAudible->Play(pSoundMsg->fIndex);
		if(pSoundMsg->Cmd( plSoundMsg::kSynchedPlay))
			fAudible->SynchedPlay(pSoundMsg->fIndex);
		if (pSoundMsg->Cmd( plSoundMsg::kSetLooping ) )
			fAudible->SetLooping(true,pSoundMsg->fIndex);
		if (pSoundMsg->Cmd( plSoundMsg::kUnSetLooping ) )
			fAudible->SetLooping(false,pSoundMsg->fIndex);
		if (pSoundMsg->Cmd( plSoundMsg::kToggleState ) )
		{
			if (fAudible->IsPlaying(pSoundMsg->fIndex))
				fAudible->Stop(pSoundMsg->fIndex);
			else
				fAudible->Play(pSoundMsg->fIndex);
		}
		if (pSoundMsg->Cmd( plSoundMsg::kGetStatus ) )
		{
			fAudible->GetStatus(pSoundMsg);
		}
		if (pSoundMsg->Cmd( plSoundMsg::kGetNumSounds ) )
		{
			plSoundMsg* pReply = TRACKED_NEW plSoundMsg;
			pReply->fIndex = fAudible->GetNumSounds();
			pReply->AddReceiver(pSoundMsg->GetSender());
			pReply->SetCmd( plSoundMsg::kGetNumSounds );
			plgDispatch::MsgSend(pReply);
		}
		if( pSoundMsg->Cmd( plSoundMsg::kSetVolume ) )
		{
			fAudible->SetVolume( pSoundMsg->fVolume, pSoundMsg->fIndex ); 
		}
		if ( pSoundMsg->Cmd( plSoundMsg::kSetTalkIcon ) )
		{
			fAudible->SetTalkIcon(pSoundMsg->fIndex, pSoundMsg->fNameStr);
		}
		if ( pSoundMsg->Cmd( plSoundMsg::kClearTalkIcon ) )
		{
			fAudible->ClearTalkIcon();
		}
 		if ( pSoundMsg->Cmd( plSoundMsg::kSetFadeIn ) )
		{
			fAudible->SetFadeIn( (int)pSoundMsg->fFadeType, pSoundMsg->fVolume, pSoundMsg->fIndex );
		}
		if ( pSoundMsg->Cmd( plSoundMsg::kSetFadeOut ) )
		{
			fAudible->SetFadeOut( (int)pSoundMsg->fFadeType, pSoundMsg->fVolume, pSoundMsg->fIndex );
		}
		if( pSoundMsg->Cmd( plSoundMsg::kFastForwardPlay ) )
		{
			fAudible->FastForwardPlay(pSoundMsg->fIndex);
		}
		if(pSoundMsg->Cmd(plSoundMsg::kFastForwardToggle) )
		{
			fAudible->FastForwardToggle(pSoundMsg->fIndex);
		}

		return true;
	}


	plAudioSysMsg* pASMsg = plAudioSysMsg::ConvertNoRef( msg );
	if (pASMsg)
	{
		if (pASMsg->GetAudFlag() == plAudioSysMsg::kActivate)
		{	
			if( fAudible )
			{
				if( !fAudibleInited )
				{
					int isLocal = IsLocallyOwned();
					fAudible->Init( ( isLocal == plSynchedObject::kYes ) );
					fAudibleInited = true;
				}
				if( !fAudibleInited )
				{
					// Arrgh, can't activate yet, so attempt to re-activate some time in the near future
					pASMsg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kActivate );
					pASMsg->SetBCastFlag( plMessage::kBCastByExactType, false );
					pASMsg->SetBCastFlag( plMessage::kNetPropagate, false );
					pASMsg->SetTimeStamp( hsTimer::GetSysSeconds() + 1.f );
					pASMsg->Send( GetKey() );
					return true;
				}
				fAudible->Activate();
				if (GetOwner() && GetOwner()->GetCoordinateInterface())
				{
					hsMatrix44 l2w = GetOwner()->GetCoordinateInterface()->GetLocalToWorld();
					hsMatrix44 w2l = GetOwner()->GetCoordinateInterface()->GetWorldToLocal();;
					fAudible->SetTransform(l2w,w2l);
				}
			}
		}
		if (pASMsg->GetAudFlag() == plAudioSysMsg::kDeActivate)
		{
			if( fAudible )
				fAudible->DeActivate();
		}
		if( pASMsg->GetAudFlag() == plAudioSysMsg::kMuteAll )
		{
			if( fAudible )
				fAudible->SetMuted( true );
		}
		else if( pASMsg->GetAudFlag() == plAudioSysMsg::kUnmuteAll )
		{
			if( fAudible )
				fAudible->SetMuted( false );
		}
		else if( pASMsg->GetAudFlag() == plAudioSysMsg::kChannelVolChanged )
		{
			if( fAudible )
				return fAudible->MsgReceive( msg );
		}
	}

 	plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef( msg );
	if (pEnableMsg)
	{
		SetProperty( kDisable, pEnableMsg->Cmd(kDisable) );
		return true;
	}

	// proxyDrawMsg handling--just pass it on to the audible
	plProxyDrawMsg *pdMsg = plProxyDrawMsg::ConvertNoRef( msg );
	if( pdMsg != nil )
	{
		if( fAudible )
			return fAudible->MsgReceive( pdMsg );

		return true;
	}

	return plObjInterface::MsgReceive(msg);
}


void plAudioInterface::ReleaseData()
{
	if (fAudible)
	{
		// To get rid of our data, we need to release our active ref and tell the SceneNode
		// to dump it. It will autodestruct after those two active refs are released, unless
		// someone else has a ref on it as well (in which case we don't want to be nuking it
		// anyway).
		fAudible->SetSceneNode(nil);
		// Audible key is gone already, I guess the audioInterface doesn't have a ref -Colin
//		GetKey()->Release(fAudible->GetKey());
	}
}
