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
#include <direct.h>
#include "HeadSpin.h"
#include "hsTimer.h"
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "plProfile.h"

#include "plWin32Sound.h"
#include "plWin32StreamingSound.h"
#include "plDSoundBuffer.h"
#include "plAudioSystem.h"

#include "../plAudioCore/plAudioFileReader.h"
#include "../plAudioCore/plSoundBuffer.h"
#include "../plAudioCore/plSoundDeswizzler.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../plStatusLog/plStatusLog.h"

#define STREAMING_UPDATE_MS 200

plProfile_Extern(MemSounds);
plProfile_CreateAsynchTimer( "Stream Shove Time", "Sound", StreamSndShoveTime );
plProfile_CreateAsynchTimer( "Stream Swizzle Time", "Sound", StreamSwizzleTime );
plProfile_Extern( SoundLoadTime );

plWin32StreamingSound::plWin32StreamingSound() :
fDataStream(nil),
fBlankBufferFillCounter(0),
fDeswizzler(nil),
fStreamType(kNoStream),
fLastStreamingUpdate(0),
fStopping(false),
fPlayWhenStopped(false),
fStartPos(0)
{
	fBufferLengthInSecs = plgAudioSys::GetStreamingBufferSize();
}

plWin32StreamingSound::~plWin32StreamingSound()
{
	/// Call before we delete our dataStream
	DeActivate();
	IUnloadDataBuffer();

	delete fDataStream;
	fDataStream = nil;
	fSrcFilename[ 0 ] = 0;

	delete fDeswizzler;
}

void plWin32StreamingSound::DeActivate()
{
	plWin32Sound::DeActivate();
}

// Change the filename used by this streaming sound, so we can play a different file
void plWin32StreamingSound::SetFilename(const char *filename, bool isCompressed)
{	
	fNewFilename = filename;
	fIsCompressed = isCompressed;
	fFailed = false;	// just in case the last sound failed to load turn this off so it can try to load the new sound
}

//////////////////////////////////////////////////////////////
//	Override, 'cause we don't want to actually LOAD the sound for streaming,
//	just make sure it's decompressed and such and ready to stream.
plSoundBuffer::ELoadReturnVal plWin32StreamingSound::IPreLoadBuffer( hsBool playWhenLoaded, hsBool isIncidental /* = false */ )
{
	if(fPlayWhenStopped)
		return plSoundBuffer::kPending;
	hsBool sfxPath = fNewFilename.size() ? false : true;
	
	if( fDataStream != nil && fNewFilename.size() == 0)
		return plSoundBuffer::kSuccess;		// Already loaded
		
	if(!ILoadDataBuffer())
	{
		return plSoundBuffer::kError;
	}
	plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();		
	if(!buffer)
		return plSoundBuffer::kError;
	
	// The databuffer also needs to know if the source is compressed or not
	if(fNewFilename.length())
	{
		buffer->SetFileName(fNewFilename.c_str());
		buffer->SetFlag(plSoundBuffer::kStreamCompressed, fIsCompressed);
		fNewFilename.clear();
		if(fReallyPlaying)
		{
			fPlayWhenStopped = true;
			return plSoundBuffer::kPending;
		}
	}

	if( buffer->IsValid() )
	{
		plAudioFileReader::StreamType type = plAudioFileReader::kStreamNative;
		fStreamType = kStreamCompressed;

		if(!buffer->HasFlag(plSoundBuffer::kStreamCompressed))
		{
			if(buffer->GetDataLengthInSecs() > plgAudioSys::GetStreamFromRAMCutoff( ))
			{
				fStreamType = kStreamFromDisk;
				type = plAudioFileReader::kStreamWAV;
			}
			else
			{
				fStreamType = kStreamFromRAM;
				type = plAudioFileReader::kStreamRAM;
			}
		}

		if(!fStartPos)
		{
			if(buffer->AsyncLoad(type, isIncidental ? 0 : STREAMING_BUFFERS * STREAM_BUFFER_SIZE ) == plSoundBuffer::kPending)
			{
				fPlayWhenLoaded = playWhenLoaded;
				fLoading = true;
				return plSoundBuffer::kPending;
			}
		}
		
		char str[ 256 ];
		strncpy( fSrcFilename, buffer->GetFileName(), sizeof( fSrcFilename ) );
		bool streamCompressed = (buffer->HasFlag(plSoundBuffer::kStreamCompressed) != 0);

		delete fDataStream;
		fDataStream = buffer->GetAudioReader();
		if(!fDataStream)
		{
			plAudioCore::ChannelSelect select = buffer->GetReaderSelect();

			bool streamCompressed = (buffer->HasFlag(plSoundBuffer::kStreamCompressed) != 0);

			/// Open da file
			CHAR strPath[ MAX_PATH ];
				
			_getcwd(strPath, MAX_PATH);
			if(sfxPath)
				strcat( strPath, "\\sfx\\" );
			else
			{
				// if we've changing the filename don't append 'sfx', just append a '\' since a path to the folder is expected
				strcat( strPath, "\\");
			}
			strcat( strPath, fSrcFilename );
			fDataStream = plAudioFileReader::CreateReader(strPath, select,type);
		}			
		
		if( fDataStream == nil || !fDataStream->IsValid() )
		{
			delete fDataStream;
			fDataStream = nil;
			return plSoundBuffer::kError;
		}

		sprintf( str, "   Readied file %s for streaming", fSrcFilename );
		IPrintDbgMessage( str );
		
		// dont free sound data until we have a chance to use it in load sound

		return fDataStream ? plSoundBuffer::kSuccess : plSoundBuffer::kError;
	}
		
	plStatusLog::AddLineS("audio.log", "EnsureLoadable failed for streaming sound %d", fDataBufferKey->GetName());
	return plSoundBuffer::kError;
}

void plWin32StreamingSound::IFreeBuffers( void )
{
	plWin32Sound::IFreeBuffers();
	if( fLoadFromDiskOnDemand && !IsPropertySet( kPropLoadOnlyOnCall ) )
	{
		// if the audio system has just restarted, dont delete the datastream. This way we can pick up where we left off instead of restarting the sound.
		if(!plgAudioSys::IsRestarting())
		{
			// we are deleting the stream, we must release the sound data.
			FreeSoundData();	
			delete fDataStream;
			fDataStream = nil;
		}
		fSrcFilename[ 0 ] = 0;
	}
}

///////////////////////////////////////////////////////////////////
//	Overload from plSound. Basically sets up the streaming file and fills the
//	first half of our buffer. We'll fill the rest of the buffer as we get
//	notifications for such.

hsBool plWin32StreamingSound::LoadSound( hsBool is3D )
{
	if( fFailed )
		return false;
	if( !plgAudioSys::Active() || fDSoundBuffer )
		return false;

	if( fPriority > plgAudioSys::GetPriorityCutoff() )
		return false;	// Don't set the failed flag, just return

	// Debug flag #1
	if( is3D && fChannelSelect > 0 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableRightSelect ) )
	{
		// Force a fail
		fFailed = true;
		return false;	
	}
	plSoundBuffer::ELoadReturnVal retVal = IPreLoadBuffer(true);
	if(retVal == plSoundBuffer::kPending)
		return true;

	if( retVal == plSoundBuffer::kError )
	{
		char str[ 256 ];
		sprintf( str, "Unable to open streaming source %s", fDataBufferKey->GetName() );
		IPrintDbgMessage( str, true );
		fFailed = true;
		return false;
	}

	SetProperty( kPropIs3DSound, is3D );

	plWAVHeader	header = fDataStream->GetHeader();
	UInt32 bufferSize = (UInt32)(fBufferLengthInSecs * header.fAvgBytesPerSec); 

	// Debug flag #2
	if( is3D && fChannelSelect == 0 && header.fNumChannels > 1 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableLeftSelect ) )
	{
		// Force a fail
		fFailed = true;
		return false;
	}

	if( header.fNumChannels > 1 && is3D )
	{
		// We can only do a single channel of 3D sound. So copy over one (later)
		bufferSize				/= header.fNumChannels;
		header.fBlockAlign		/= header.fNumChannels;
		header.fAvgBytesPerSec	/= header.fNumChannels;
		header.fNumChannels = 1;
	}

	// Actually create the buffer now (always looping)
	fDSoundBuffer = TRACKED_NEW plDSoundBuffer( bufferSize, header, is3D, IsPropertySet(kPropLooping), false, true );
	if( !fDSoundBuffer->IsValid() )
	{
		fDataStream->Close();
		delete fDataStream;
		fDataStream = nil;

		delete fDSoundBuffer;
		fDSoundBuffer = nil;

		char str[256];
		sprintf(str, "Can't create sound buffer for %s.wav. This could happen if the wav file is a stereo file. Stereo files are not supported on 3D sounds. If the file is not stereo then please report this error.", GetFileName());
		IPrintDbgMessage( str, true );
		fFailed = true;
		return false;
	}

	fTotalBytes = (UInt32)bufferSize;
	plProfile_NewMem(MemSounds, fTotalBytes);

	plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();		
	if(!buffer)
		return false;

	bool setupSource = true;
	if(!buffer->GetData() || fStartPos)
	{ 
		if(fStartPos && fStartPos <= fDataStream->NumBytesLeft())
		{
			fDataStream->SetPosition(fStartPos);
			plStatusLog::AddLineS("syncaudio.log", "startpos %d", fStartPos);
		}

		// if we get here we are not starting from the beginning of the sound. We still have an audio loaded and need to pick up where we left off
		if(!fDSoundBuffer->SetupStreamingSource(fDataStream))
		{
			setupSource = false;
		}
	}
	else
	{
		// this sound is starting from the beginning. Get the data and start it.
		if(!fDSoundBuffer->SetupStreamingSource(buffer->GetData(), buffer->GetAsyncLoadLength()))
		{
			setupSource = false;
		}
	}
	
	if(!setupSource)
	{
		fDataStream->Close();
		delete fDataStream;
		fDataStream = nil;
		delete fDSoundBuffer;
		fDSoundBuffer = nil;

		plStatusLog::AddLineS("audio.log", "Could not play streaming sound, no voices left %s", GetKeyName());
		return false;
	}
	FreeSoundData();
	
	IRefreshEAXSettings( true );

	// Debug info
	char str[ 256 ];
	sprintf( str, "   Streaming %s.", fSrcFilename);
	IPrintDbgMessage( str );

	plStatusLog::AddLineS( "audioTimes.log", 0xffffffff, "Streaming %4.2f secs of %s", fDataStream->GetLengthInSecs(), GetKey()->GetUoid().GetObjectName() );

	// Get pertinent info
	SetLength( (hsScalar)fDataStream->GetLengthInSecs() );

	// Set up our deswizzler, if necessary
	delete fDeswizzler;
	if( fDataStream->GetHeader().fNumChannels != header.fNumChannels )
		fDeswizzler = TRACKED_NEW plSoundDeswizzler( (UInt32)(fBufferLengthInSecs * fDataStream->GetHeader().fAvgBytesPerSec), 
											 (UInt8)(fDataStream->GetHeader().fNumChannels), 
											 header.fBitsPerSample / 8 );
	else
		fDeswizzler = nil;

	// LEAVE THE WAV FILE OPEN! (We *are* streaming, after all :)
	return true;
}

void plWin32StreamingSound::IStreamUpdate()
{
	if(!fReallyPlaying)
		return;

	if(hsTimer::GetMilliSeconds() - fLastStreamingUpdate < STREAMING_UPDATE_MS)	// filter out update requests so we aren't doing this more that we need to 
		return;

	plProfile_BeginTiming( StreamSndShoveTime );
	if(fDSoundBuffer)
	{
		if(fDSoundBuffer->BuffersQueued() == 0 && fDataStream->NumBytesLeft() == 0 && !fDSoundBuffer->IsLooping())
		{
			// If we are fading out it's possible that we will hit this multiple times causing this sound to try and fade out multiple times, never allowing it to actually stop
			if(!fStopping)
			{
				fStopping = true;
				Stop();
				plProfile_EndTiming( StreamSndShoveTime );
			}
			return;
		}
		
		if(!fDSoundBuffer->StreamingFillBuffer(fDataStream))
		{
			plStatusLog::AddLineS("audio.log", "%s Streaming buffer fill failed", GetKeyName());
		}
	}
	plProfile_EndTiming( StreamSndShoveTime );
}

void plWin32StreamingSound::Update()
{
	plWin32Sound::Update();
	IStreamUpdate();
}

void plWin32StreamingSound::IDerivedActuallyPlay( void )
{
	fStopping = false;
	if( !fReallyPlaying )
	{
		for(;;)
		{
			if(fSynchedStartTimeSec)
			{
				// if we are synching to another sound this is our latency time
				fDSoundBuffer->SetTimeOffsetSec((float)(hsTimer::GetSeconds() - fSynchedStartTimeSec));
				fSynchedStartTimeSec = 0;
			}

			if(IsPropertySet(kPropIncidental))
			{
				if(fIncidentalsPlaying >= MAX_INCIDENTALS)
					break;
				++fIncidentalsPlaying;
			}
			fDSoundBuffer->Play();
			fReallyPlaying = true;
			break;
		}
	}

	/// Send start callbacks
	plSoundEvent	*event = IFindEvent( plSoundEvent::kStart );
	if( event != nil )
		event->SendCallbacks();
}

void plWin32StreamingSound::IActuallyStop()
{	
	fStopping = false;
	plWin32Sound::IActuallyStop();
	if(fPlayWhenStopped)
	{
		fPlayWhenStopped = false;
		Play();
	}
}

unsigned plWin32StreamingSound::GetByteOffset()
{
	if(fDataStream && fDSoundBuffer)
	{	
		unsigned bytesQueued = fDSoundBuffer->BuffersQueued() * STREAM_BUFFER_SIZE;
		unsigned offset = fDSoundBuffer->GetByteOffset();
		long byteoffset = ((fDataStream->GetDataSize() - fDataStream->NumBytesLeft()) - bytesQueued) + offset;
		
		return byteoffset < 0 ? fDataStream->GetDataSize() - abs(byteoffset) : byteoffset;
	}
	return 0;
}

float plWin32StreamingSound::GetActualTimeSec()
{
	if(fDataStream && fDSoundBuffer)
		return fDSoundBuffer->BytePosToMSecs(fDataStream->NumBytesLeft()) / 1000.0f;
	return 0.0f;
}

void plWin32StreamingSound::SetStartPos(unsigned bytes)
{
	fStartPos = bytes;
}

void plWin32StreamingSound::ISetActualTime( double t )
{
	//fStartTimeSec = 0;
	if(fDataStream && fDSoundBuffer)
		fDataStream->SetPosition(fDSoundBuffer->GetBufferBytePos((float)t));
	//else
	//	fStartTimeSec = t;
}

hsBool plWin32StreamingSound::MsgReceive( plMessage* pMsg )
{
	return plWin32Sound::MsgReceive( pMsg );
}

void plWin32StreamingSound::IRemoveCallback( plEventCallbackMsg *pCBMsg )
{
	plWin32Sound::IRemoveCallback( pCBMsg );
}

void plWin32StreamingSound::IAddCallback( plEventCallbackMsg *pCBMsg )
{
	if( plSoundEvent::GetTypeFromCallbackMsg( pCBMsg ) != plSoundEvent::kStop &&
		plSoundEvent::GetTypeFromCallbackMsg( pCBMsg ) != plSoundEvent::kStart )
	{
		hsAssert( false, "Streaming sounds only support start and stop callbacks at this time." );
		return;
	}
	plWin32Sound::IAddCallback( pCBMsg );
}