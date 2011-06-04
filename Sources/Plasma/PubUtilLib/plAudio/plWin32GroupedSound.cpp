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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plWin32GroupedSound - Grouped version of a static sound. Lots of short	//
//						  sounds stored in the buffer, all share the same	//
//						  DSound playback buffer and only one plays at a	//
//						  time.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include "plWin32GroupedSound.h"
#include "plDSoundBuffer.h"

#include "plAudioSystem.h"
#include "../plAudioCore/plSoundBuffer.h"
#include "../plAudioCore/plSoundDeswizzler.h"
#include "plgDispatch.h"
#include "../pnMessage/plSoundMsg.h"

#include "../plStatusLog/plStatusLog.h"
#include "plProfile.h"
#include "hsResMgr.h"

plProfile_Extern( MemSounds );
plProfile_Extern( StaticSndShoveTime );
plProfile_Extern( StaticSwizzleTime );

/////////////////////////////////////////////////////////////////////////////////////////////////

plWin32GroupedSound::plWin32GroupedSound()
{
	fCurrentSound = 0;
}

plWin32GroupedSound::~plWin32GroupedSound()
{
	DeActivate();
}

void	plWin32GroupedSound::SetPositionArray( UInt16 numSounds, UInt32 *posArray, hsScalar *volumeArray )
{
	UInt16	i;


	fStartPositions.SetCountAndZero( numSounds );
	fVolumes.SetCountAndZero( numSounds );
	for( i = 0; i < numSounds; i++ )
	{
		fStartPositions[ i ] = posArray[ i ];
		fVolumes[ i ] = volumeArray[ i ];
	}
}

//// IRead/IWrite ////////////////////////////////////////////////////////////

void plWin32GroupedSound::IRead( hsStream *s, hsResMgr *mgr )
{
	plWin32StaticSound::IRead( s, mgr );
	UInt16 i, n = s->ReadSwap16();
	fStartPositions.SetCountAndZero( n );
	fVolumes.SetCountAndZero( n );
	for( i = 0; i < n; i++ )
	{
		fStartPositions[ i ] = s->ReadSwap32();
		fVolumes[ i ] = s->ReadSwapScalar();
	}
}

void plWin32GroupedSound::IWrite( hsStream *s, hsResMgr *mgr )
{
	plWin32StaticSound::IWrite( s, mgr );

	s->WriteSwap16( fStartPositions.GetCount() );
	UInt16 i;
	for( i = 0; i < fStartPositions.GetCount(); i++ )
	{
		s->WriteSwap32( fStartPositions[ i ] );
		s->WriteSwapScalar( fVolumes[ i ] );
	}
}

//// LoadSound ///////////////////////////////////////////////////////////////

hsBool	plWin32GroupedSound::LoadSound( hsBool is3D )
{
	if( fFailed )
		return false;

	if( fPriority > plgAudioSys::GetPriorityCutoff() )
		return false;	// Don't set the failed flag, just return

	if( !plgAudioSys::Active() || fDSoundBuffer != nil )
		return false;


	// Debug flag #1
	if( fChannelSelect > 0 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableRightSelect ) )
	{
		// Force a fail
		fFailed = true;
		return false;
	}

	
	// We need it to be resident to read in
	plSoundBuffer::ELoadReturnVal retVal = IPreLoadBuffer(true);
	plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();	
	if(!buffer)
	{
		return plSoundBuffer::kError;
	}

	if( retVal == plSoundBuffer::kPending)	// we are still reading data. 
	{
		return true;
	}

	// We need it to be resident to read in
	if( retVal == plSoundBuffer::kError) 
	{
		char str[ 256 ];
		sprintf( str, "Unable to open .wav file %s", fDataBufferKey ? fDataBufferKey->GetName() : "nil");
		IPrintDbgMessage( str, true );
		fFailed = true;
		return false;
	}
	
	SetProperty( kPropIs3DSound, is3D );


	plWAVHeader	header = buffer->GetHeader();

	// Debug flag #2
	if( fChannelSelect == 0 && header.fNumChannels > 1 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableLeftSelect ) )
	{
		// Force a fail
		fFailed = true;
		return false;
	}

	// Calculate the maximum size for our buffer. This will be the length of the longest sound we're going to 
	// have to play.
	UInt16 i;
	UInt32 maxSoundSize, len;
	for( i = 1, maxSoundSize = 0; i < fStartPositions.GetCount(); i++ )
	{
		len = fStartPositions[ i ] - fStartPositions[ i - 1 ];
		if( len > maxSoundSize )
			maxSoundSize = len;
	}
	len = buffer->GetDataLength() - fStartPositions[ fStartPositions.GetCount() - 1 ];
	if( len > maxSoundSize )
		maxSoundSize = len;

	// Based on that, allocate our buffer
	UInt32 bufferSize = maxSoundSize - ( maxSoundSize % header.fBlockAlign );

	if( header.fNumChannels > 1 && is3D )
	{
		// We can only do a single channel of 3D sound. So copy over one (later)
		bufferSize				/= header.fNumChannels;
		header.fBlockAlign		/= header.fNumChannels;
		header.fAvgBytesPerSec	/= header.fNumChannels;
		header.fNumChannels = 1;
	}
	fNumDestChannels = (UInt8)(header.fNumChannels);
	fNumDestBytesPerSample = (UInt8)(header.fBlockAlign);

	// Create our DSound buffer (or rather, the wrapper around it)
	fDSoundBuffer = TRACKED_NEW plDSoundBuffer( bufferSize, header, is3D, IsPropertySet( kPropLooping ), true );
	if( !fDSoundBuffer->IsValid() )
	{
		char str[256];
		sprintf(str, "Can't create sound buffer for %s.wav. This could happen if the wav file is a stereo file. Stereo files are not supported on 3D sounds. If the file is not stereo then please report this error.", GetFileName());
		IPrintDbgMessage( str, true );
		fFailed = true;

		delete fDSoundBuffer;
		fDSoundBuffer = nil;

		return false;
	}
	
	IRefreshEAXSettings( true );

	// Fill the buffer with whatever our current sound is.
	IFillCurrentSound( 0 );

	// Logging
	char str[ 256 ];
	sprintf( str, "   Grouped %s %s allocated (%d msec).", buffer->GetFileName() != nil ? "file" : "buffer", 
											buffer->GetFileName() != nil ? buffer->GetFileName() : buffer->GetKey()->GetUoid().GetObjectName(),
											//fDSoundBuffer->IsHardwareAccelerated() ? "hardware" : "software",
											//fDSoundBuffer->IsStaticVoice() ? "static" : "dynamic",
#ifdef PL_PROFILE_ENABLED
											gProfileVarStaticSndShoveTime.GetValue() );
#else
											0 );
#endif
	IPrintDbgMessage( str );
	if( GetKey() != nil && GetKeyName() != nil && strstr( GetKeyName(), "Footstep" ) != nil )
		;
	else
		plStatusLog::AddLineS( "audioTimes.log", "%s (%s)", str, GetKey() ? GetKeyName() : "unkeyed" );

	fTotalBytes = bufferSize;

	plProfile_NewMem( MemSounds, fTotalBytes );

	// All done!
//	if( fLoadFromDiskOnDemand )
//		IUnloadDataBuffer();
	FreeSoundData();
	return true;

}

//// GetSoundLength //////////////////////////////////////////////////////////
//	Gets the length (in seconds) of the given sound index from the group.

hsScalar	plWin32GroupedSound::GetSoundLength( Int16 soundIndex )
{
	plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();
	if(buffer)
	{
		return (hsScalar)IGetSoundByteLength( soundIndex ) / buffer->GetHeader().fAvgBytesPerSec;
	}
	
	return 0;
}

//// IGetSoundByteLength /////////////////////////////////////////////////////
//	Byte version of above.

UInt32		plWin32GroupedSound::IGetSoundByteLength( Int16 soundIndex )
{

	if( soundIndex == fStartPositions.GetCount() - 1 )
		return ((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetDataLength() - fStartPositions[ soundIndex ];
	else
		return fStartPositions[ soundIndex + 1 ] - fStartPositions[ soundIndex ];
}

//// IGetDataPointer/Length //////////////////////////////////////////////////
// Abstracting a few things here for the incidentalMgr

void	*plWin32GroupedSound::IGetDataPointer( void ) const
{
	return ( fDataBufferKey->ObjectIsLoaded() ) ? (void *)( (UInt8 *)((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetData() + fStartPositions[ fCurrentSound ] ) : nil;
}

UInt32	plWin32GroupedSound::IGetDataLength( void ) const
{
	return ( fDataBufferKey->ObjectIsLoaded() ) ? fCurrentSoundLength : 0;
}

//// IFillCurrentSound ///////////////////////////////////////////////////////
//	Fills the DSoundBuffer with data from the current sound from our sound
//	group, optionally switching what our current sound is.

void	plWin32GroupedSound::IFillCurrentSound( Int16 newCurrent /*= -1*/ )
{
	//void	*dataPtr;
	//UInt32	dataLength;

	if( !fDSoundBuffer && plgAudioSys::Active() )
		LoadSound( IsPropertySet( kPropIs3DSound ) );

	plProfile_BeginTiming( StaticSndShoveTime );

	// Make sure we're stopped first. Don't want to be filling while we're playing
	Stop();

	if( newCurrent != -1 )
	{
		fCurrentSound = (UInt16)newCurrent;

		if( fCurrentSound >= fStartPositions.GetCount() )
		{
			// Invalid index!
			hsAssert( false, "Invalid index in plWin32GroupedSound::IFillCurrentSound()" );
			fCurrentSound = -1;
			return;
		}

		// Set our length based on the current sound
		fCurrentSoundLength = IGetSoundByteLength( fCurrentSound );
		if( fDataBufferKey->ObjectIsLoaded() )
			SetLength( fCurrentSoundLength / ((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetHeader().fAvgBytesPerSec );
	
		// Update our volume as well
		SetVolume( fVolumes[ fCurrentSound ] );
	}

	if( fDSoundBuffer != nil )
	{
		/// Lock our buffer
		//fDSoundBuffer->Lock( dataLength, dataPtr );

		/// Copy or de-swizzle?
		//if( fDataBuffer->GetHeader().fNumChannels == fNumDestChannels )
		{
			// Just copy
			//memcpy( dataPtr, (Byte *)fDataBuffer->GetData() + fStartPositions[ fCurrentSound ], fCurrentSoundLength );
			//dataPtr = (Byte *)dataPtr + fCurrentSoundLength;
			//dataLength -= fCurrentSoundLength;
		}
		//else
		{
			// We're extracting a single channel of sound into our sound buffer, so it isn't a straight copy...

			/*plProfile_BeginTiming( StaticSwizzleTime );

			plSoundDeswizzler	deswiz( (Byte *)fDataBuffer->GetData() + fStartPositions[ fCurrentSound ], fCurrentSoundLength, 
										(UInt8)(fDataBuffer->GetHeader().fNumChannels), fNumDestBytesPerSample );

			deswiz.Extract( fChannelSelect, dataPtr );

			dataPtr = (Byte *)dataPtr + fCurrentSoundLength / fDataBuffer->GetHeader().fNumChannels;
			dataLength -= fCurrentSoundLength / fDataBuffer->GetHeader().fNumChannels;

			plProfile_EndTiming( StaticSwizzleTime );*/
		}

		/// Fill the remaining part with empty space
		//memset( dataPtr, 0, dataLength );

		/// Finally, unlock!
		//fDSoundBuffer->Unlock();
	}

	/// All done!
	plProfile_EndTiming( StaticSndShoveTime );
}

void plWin32GroupedSound::IDerivedActuallyPlay( void )
{
	// Ensure there's a stop notify for us
	if( !fReallyPlaying )
	{
		fDSoundBuffer->Play();
		fReallyPlaying = true;
	}

	plSoundEvent	*event = IFindEvent( plSoundEvent::kStart );
	if( event != nil )
		event->SendCallbacks();
}

hsBool	plWin32GroupedSound::MsgReceive( plMessage* pMsg )
{
	plSoundMsg *soundMsg = plSoundMsg::ConvertNoRef( pMsg );
	if( soundMsg != nil && soundMsg->Cmd( plSoundMsg::kSelectFromGroup ) )
	{
		IFillCurrentSound( soundMsg->fIndex );
		return true;
	}
	else if( soundMsg != nil && soundMsg->Cmd( plSoundMsg::kPlay ) )
	{
		Play();
		//plIncidentalMgr::GetInstance()->Play( this, plIncidentalMgr::kNormal );
		return true;
	}

	return plWin32StaticSound::MsgReceive( pMsg );
}

