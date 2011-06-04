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
#include <hvdi.h>
#include <direct.h>
#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsTimer.h"
#include "hsResMgr.h"
#include "plgDispatch.h"

#include "plProfile.h"
#include "plWin32Sound.h"
#include "plAudioSystem.h"
#include "plDSoundBuffer.h"
#include "plWavFile.h"

#include "../plAudible/plWinAudible.h"
#include "../plNetMessage/plNetMessage.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../plPipeline/plPlates.h"
#include "../plStatusLog/plStatusLog.h"

plProfile_CreateMemCounter("Sounds", "Memory", MemSounds);
plProfile_Extern(SoundPlaying);

plWin32Sound::plWin32Sound() :
fFailed(false),
fPositionInited(false),
fAwaitingPosition(false),
fTotalBytes(0),
fReallyPlaying(false),
fChannelSelect(0),
fDSoundBuffer(nil)
{
}

plWin32Sound::~plWin32Sound()
{
}

void plWin32Sound::Activate( hsBool forcePlay )
{
	if( fFailed )
		return;

	plSound::Activate( forcePlay );
}

void plWin32Sound::DeActivate()
{
	plSound::DeActivate();
	IFreeBuffers();
}

void plWin32Sound::IFreeBuffers( void )
{
	if( fDSoundBuffer != nil )
	{
		delete fDSoundBuffer;
		fDSoundBuffer = nil;
		plProfile_DelMem(MemSounds, fTotalBytes);
		fTotalBytes = 0;
	}

	fPositionInited = false;
	fAwaitingPosition = false;
}

void plWin32Sound::Update()
{
	plSound::Update();
}

void plWin32Sound::IActuallyPlay( void )
{
	//plSound::Play();
	if (!fDSoundBuffer && plgAudioSys::Active())
		LoadSound( IsPropertySet( kPropIs3DSound ) );
	if(!fLoading )
	{
		if (fDSoundBuffer && plgAudioSys::Active() )
		{

			// Sometimes base/derived classes can be annoying
			IDerivedActuallyPlay();
			RefreshVolume();		
		}
		else
		{
			// If we can't load (for ex., if audio is off), then we act like we played and then stopped
			// really fast. Thus, we need to send *all* of our callbacks off immediately and then Stop().
			UInt32 i;
			for( i = 0; i < fSoundEvents.GetCount(); i++ )
			{
				fSoundEvents[ i ]->SendCallbacks();
			}

			// Now stop, 'cause we played really really really really fast
			fPlaying = false;
			fPlayOnReactivate = IsPropertySet( kPropLooping );
			IActuallyStop();
		}
	}
}

void plWin32Sound::IActuallyStop()
{
	if( fReallyPlaying )
	{
		if( fDSoundBuffer != nil )
		{
			if(IsPropertySet(kPropIncidental))
			{
				--fIncidentalsPlaying;
			}
			fDSoundBuffer->Stop();
			plStatusLog::AddLineS("impacts.log", "Stopping %s", GetKeyName());
		
		}
		fReallyPlaying = false;
	}
	else
	{
		if( fDSoundBuffer != nil && fDSoundBuffer->IsPlaying() )
		{
			plStatusLog::AddLineS( "audio.log", 0xffff0000, "WARNING: BUFFER FLAGGED AS STOPPED BUT NOT STOPPED - %s", GetKey() ? GetKeyName() : nil );
			fDSoundBuffer->Stop();
		}
	}

	// send callbacks
	plSoundEvent	*event = IFindEvent( plSoundEvent::kStop );
	if( event != nil )
	{
		event->SendCallbacks();
	}

	plSound::IActuallyStop();
}

plSoundMsg* plWin32Sound::GetStatus(plSoundMsg* pMsg)
{
	plSoundMsg* pReply = TRACKED_NEW plSoundMsg;
	pReply->AddReceiver( pMsg->GetSender() );
	pReply->SetCmd(plSoundMsg::kStatusReply);
	pReply->fLoop = IsPropertySet( kPropLooping );
	pReply->fPlaying = IsPlaying();
	return pReply;
}

void plWin32Sound::SetMin( const int m )
{
	plSound::SetMin(m);
	if(fDSoundBuffer)
	{
		fDSoundBuffer->SetMinDistance(m);
	}
}

void	plWin32Sound::SetMax( const int m )
{
	plSound::SetMax(m);
	if( fDSoundBuffer )
	{
		fDSoundBuffer->SetMaxDistance( m );
	}
}

void plWin32Sound::SetOuterVolume( const int v )
{	
	plSound::SetOuterVolume(v);
	if(fDSoundBuffer)
	{
		fDSoundBuffer->SetConeOutsideVolume(v);
	}
}

void plWin32Sound::SetConeAngles( int inner, int outer )
{
	plSound::SetConeAngles(inner, outer);
	if(fDSoundBuffer)
	{
		fDSoundBuffer->SetConeAngles(inner, outer);
	}
}

void plWin32Sound::SetConeOrientation( hsScalar x, hsScalar y, hsScalar z )
{
	plSound::SetConeOrientation(x, y, z);
	if(fDSoundBuffer)
	{
		fDSoundBuffer->SetConeOrientation(x, z, y);
	}
}

void plWin32Sound::SetVelocity( const hsVector3 vel )
{
	plSound::SetVelocity(vel);
	if( fDSoundBuffer)
		fDSoundBuffer->SetVelocity(vel.fX, vel.fZ, vel.fY);
}

void plWin32Sound::SetPosition( const hsPoint3 pos )
{
	plSound::SetPosition(pos);
	if(fDSoundBuffer)
	{
		// in openal sounds that are mono are played as positional. Since gui's may be positioned way off in space, the sound may not be audible.
		// doing this allows us to play mono gui sounds and still hear them, since this attaches the sound to the listener.
		if(fType == kGUISound)
		{
			hsPoint3 listenerPos = plgAudioSys::Sys()->GetCurrListenerPos();
			fDSoundBuffer->SetPosition(listenerPos.fX, listenerPos.fZ, listenerPos.fY);
		}
		else
		{
			fDSoundBuffer->SetPosition(pos.fX, pos.fZ, pos.fY);
		}
	}

	fPositionInited = true;
	if( fAwaitingPosition )
	{
		// If this is set, then we tried to set the volume before the position. Since
		// this results in some ghastly sound popping, we wait until we set the position
		// (here), and then call our volume again
		RefreshVolume();
		fAwaitingPosition = false;
	}
}

void plWin32Sound::ISetActualVolume(const float volume)
{
	float vol = IAttenuateActualVolume( volume ) * IGetChannelVolume();
	if( fDSoundBuffer )
	{
		if( fPositionInited || !IsPropertySet( kPropIs3DSound ) )
		{
			fDSoundBuffer->SetScalarVolume( vol );
		}
		else
		{
			// If position isn't inited, we don't want to set the volume yet,
			// so set this flag so we know to set the volume once we DO get the position
			fAwaitingPosition = true;
		}
	}
	IUpdateDebugPlate();	// Byte me.
}

//////////////////////////////////////////////////////////////
//	The base class will make sure all our params are correct and up-to-date,
//	but before it does its work, we have to make sure our buffer is ready to
//	be updated.
void plWin32Sound::IRefreshParams( void )
{
	if (!fDSoundBuffer && plgAudioSys::Active())
		LoadSound( IsPropertySet( kPropIs3DSound ) );

	else
	{
		// There is a gap between the ds buffer stopping and the sound being marked as stopped.
		// If the sound is asked to play again during this time frame it will never actually be
		// played because it is still marked as playing. Not only that, but we'll lose a hardware voice too.
		// This will fix that by starting it up again.
		if(plgAudioSys::Active())
			fDSoundBuffer->Play();
	}
	plSound::IRefreshParams();
}

void plWin32Sound::IRefreshEAXSettings( hsBool force )
{
	if( fDSoundBuffer != nil )
		fDSoundBuffer->SetEAXSettings( &GetEAXSettings(), force );
}

void plWin32Sound::IAddCallback( plEventCallbackMsg *pMsg )
{
	plSoundEvent::Types type = plSoundEvent::GetTypeFromCallbackMsg( pMsg );
	plSoundEvent		*event;

	if( type == plSoundEvent::kTime )
	{
		UInt32 byteTime = ( fDSoundBuffer != nil ) ? fDSoundBuffer->GetBufferBytePos( pMsg->fEventTime ) : 0;

		event = IFindEvent( type, byteTime );

		if( event == nil )
		{
			// Add a new sound event for this guy
			event = TRACKED_NEW plSoundEvent( type, byteTime, this );
			//fDSoundBuffer->AddPosNotify( byteTime );
			fSoundEvents.Append( event );
		}
	}
	else
	{
		event = IFindEvent( type );

		if( event == nil )
		{
			// Add a new sound event for this guy
			event = TRACKED_NEW plSoundEvent( type, this );
			fSoundEvents.Append( event );
		}
	}

	event->AddCallback( pMsg );
}

void plWin32Sound::IRemoveCallback( plEventCallbackMsg *pMsg )
{
	plSoundEvent::Types type = plSoundEvent::GetTypeFromCallbackMsg( pMsg );

	for(int i = 0; i < fSoundEvents.GetCount(); ++i)
	{
		if( fSoundEvents[ i ]->GetType() == type )
		{
			if( fSoundEvents[ i ]->RemoveCallback( pMsg ) )
			{
				if( fSoundEvents[ i ]->GetNumCallbacks() == 0 )
				{
					//if( fSoundEvents[ i ]->GetType() == plSoundEvent::kTime )
						//fDSoundBuffer->RemovePosNotify( fSoundEvents[ i ]->GetTime() );

					delete fSoundEvents[ i ];
					fSoundEvents.Remove( i );
				}
				break;
			}
		}
	}
}

plSoundEvent *plWin32Sound::IFindEvent( plSoundEvent::Types type, UInt32 bytePos )
{
	for(int i = 0; i < fSoundEvents.GetCount(); ++i )
	{
		if( fSoundEvents[ i ]->GetType() == type )
		{
			if( type != plSoundEvent::kTime || bytePos == fSoundEvents[ i ]->GetTime() )
				return fSoundEvents[ i ];
		}
	}

	return nil;
}

void plWin32Sound::RemoveCallbacks(plSoundMsg* pSoundMsg)
{
	for(int i = 0; i < pSoundMsg->GetNumCallbacks(); ++i )
		IRemoveCallback( pSoundMsg->GetEventCallback( i ) );
}

void plWin32Sound::AddCallbacks(plSoundMsg* pSoundMsg)
{
	for(int i = 0; i < pSoundMsg->GetNumCallbacks(); ++i )
		IAddCallback( pSoundMsg->GetEventCallback( i ) );
}

hsBool plWin32Sound::MsgReceive( plMessage* pMsg )
{
	plEventCallbackMsg *e = plEventCallbackMsg::ConvertNoRef( pMsg );
	if( e != nil )
	{
		if( e->fEvent == kStop )
		{
			fPlaying = false;
			fPlayOnReactivate = false;	// Just to make sure...

			// Do we have an ending fade?
			if( fFadeOutParams.fLengthInSecs > 0 )
			{
				IStartFade( &fFadeOutParams );
				plSoundEvent	*event = IFindEvent( plSoundEvent::kStop );
				if( event != nil )
					event->SendCallbacks();
			}
			else
			{
				if( fFading )
					IStopFade();

				fCurrVolume = 0.f;
				this->ISetActualVolume( fCurrVolume );
			}
			this->IActuallyStop();
			return true;
		}
	}
	return plSound::MsgReceive( pMsg );
}

void plWin32Sound::IRead( hsStream *s, hsResMgr *mgr )
{
	plSound::IRead( s, mgr );
	fChannelSelect = s->ReadByte();
}

void plWin32Sound::IWrite( hsStream *s, hsResMgr *mgr )
{
	plSound::IWrite( s, mgr );
	s->WriteByte( fChannelSelect );
}
