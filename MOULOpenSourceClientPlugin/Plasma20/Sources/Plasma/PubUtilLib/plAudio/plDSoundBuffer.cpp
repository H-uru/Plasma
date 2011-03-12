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
//	plDSoundBuffer - Simple wrapper class for a DirectSound buffer.			//
//					 Allows us to simplify all the work done behind the		//
//					 scenes in plWin32BufferThread.							//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsThread.h"
#include "plDSoundBuffer.h"
#include "al.h"

#include "plgDispatch.h"
#include "plAudioSystem.h"
#include "../plAudioCore/plAudioCore.h"
#include "../plAudioCore/plAudioFileReader.h"
#include "plEAXEffects.h"

#include "plProfile.h"

#include "../plStatusLog/plStatusLog.h"

#include <dsound.h>

UInt32 plDSoundBuffer::fNumBuffers = 0;
plProfile_CreateCounterNoReset( "Playing", "Sound", SoundPlaying );
plProfile_CreateCounterNoReset( "Allocated", "Sound", NumAllocated );


//// Constructor/Destructor //////////////////////////////////////////////////

plDSoundBuffer::plDSoundBuffer( UInt32 size, plWAVHeader &bufferDesc, hsBool enable3D, hsBool isLooping, hsBool tryStatic, bool streaming )
{ 
	fLooping = isLooping;
	fValid = false;
	fBufferDesc = nil;

	fLockPtr = nil;
	fLockLength = 0;

	fStreaming = streaming;

	buffer = 0;
	source = 0; 
	for(int i = 0; i < STREAMING_BUFFERS; ++i)
	{
		streamingBuffers[i] = 0;
	}

	IAllocate( size, bufferDesc, enable3D, tryStatic );
	fNumBuffers++;
}

plDSoundBuffer::~plDSoundBuffer()
{
	IRelease();
	fNumBuffers--;
	
}

//// IAllocate ///////////////////////////////////////////////////////////////

void	plDSoundBuffer::IAllocate( UInt32 size, plWAVHeader &bufferDesc, hsBool enable3D, hsBool tryStatic )
{
	// Create a DSound buffer description
	fBufferDesc = TRACKED_NEW DSBUFFERDESC;
	fBufferDesc->dwSize = sizeof( DSBUFFERDESC );
	
	fBufferDesc->dwBufferBytes = size; 
	fBufferDesc->dwReserved = 0;

	fBufferDesc->lpwfxFormat = TRACKED_NEW WAVEFORMATEX;
	fBufferDesc->lpwfxFormat->cbSize  = 0; 
	fBufferDesc->lpwfxFormat->nAvgBytesPerSec   = bufferDesc.fAvgBytesPerSec; 
	fBufferDesc->lpwfxFormat->nBlockAlign		= bufferDesc.fBlockAlign; 
	fBufferDesc->lpwfxFormat->nChannels			= bufferDesc.fNumChannels; 
	fBufferDesc->lpwfxFormat->nSamplesPerSec	= bufferDesc.fNumSamplesPerSec; 
	fBufferDesc->lpwfxFormat->wBitsPerSample	= bufferDesc.fBitsPerSample; 
	fBufferDesc->lpwfxFormat->wFormatTag		= bufferDesc.fFormatTag; 
	
	// Do we want to try EAX?
	if( plgAudioSys::UsingEAX() )
		fEAXSource.Init( this );

	fValid = true;
	plProfile_Inc( NumAllocated );
}

//// IRelease ////////////////////////////////////////////////////////////////

void	plDSoundBuffer::IRelease( void )
{
	if( IsPlaying() )
		Stop();

	// Release stuff
	fEAXSource.Release();
	alSourcei(source, AL_BUFFER, nil);
	alDeleteSources(1, &source);
	if(buffer)
		alDeleteBuffers( 1, &buffer );
	else
		alDeleteBuffers(STREAMING_BUFFERS, streamingBuffers);
	source = 0;
	buffer = 0;

	alGetError();
	
	memset(streamingBuffers, 0, STREAMING_BUFFERS * sizeof(unsigned));
	if( fBufferDesc != nil )
	{
		delete fBufferDesc->lpwfxFormat;
		fBufferDesc->lpwfxFormat = nil;		
	}

	delete fBufferDesc;
	fBufferDesc = nil;

	fValid = false;
	plProfile_Dec( NumAllocated );
}


/*****************************************************************************
*
*   OpenAL
*
***/

int plDSoundBuffer::IGetALFormat(unsigned bitsPerSample, unsigned int numChannels)
{
	int format = 0;
	switch(bitsPerSample)
	{
	case 8:
		format = (numChannels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
		break;
	case 16:
		format = (numChannels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		break;
	}

	return format;
}

bool plDSoundBuffer::FillBuffer(void *data, unsigned bytes, plWAVHeader *header) 
{
	if(source)
	{
		alSourcei(source, AL_BUFFER, nil);
		alDeleteSources(1, &source);
	}
	if(buffer)
		alDeleteBuffers(1, &buffer);
	source = 0;
	buffer = 0;

	ALenum format = IGetALFormat(fBufferDesc->lpwfxFormat->wBitsPerSample, fBufferDesc->lpwfxFormat->nChannels);
	ALenum error = alGetError();
	alGenBuffers(1, &buffer);
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create sound buffer %d", error);
		return false;
	}

	alBufferData(buffer, format, data, bytes, header->fNumSamplesPerSec );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to fill sound buffer %d", error);
		return false;
	}
	alGenSources(1, &source);
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create audio source %d %d", error, source);
		return false;
	}

	// Just make it quiet for now
	SetScalarVolume(0);
	
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.3048);
	alGetError();
	if( error != AL_NO_ERROR )
	{
		return false;
	}

	alSourcei(source, AL_BUFFER, buffer);
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to attach buffer to source %d", error);
		return false;
	}
	return true;
}


//============================================================================
//	OpenAL Streaming functions
//============================================================================

// this function is used when restarting the audio system. It is needed to restart a streaming source from where it left off
bool plDSoundBuffer::SetupStreamingSource(plAudioFileReader *stream)
{
	unsigned char data[STREAM_BUFFER_SIZE];
	unsigned int size;
	ALenum error;
	
	alGetError();	
	int numBuffersToQueue = 0;
	
	// fill buffers with data
	for( int i = 0; i < STREAMING_BUFFERS; i++ )
	{
		size = stream->NumBytesLeft() < STREAM_BUFFER_SIZE ? stream->NumBytesLeft() : STREAM_BUFFER_SIZE;
		if(!size)
		{
			if(IsLooping())
			{
				stream->SetPosition(0);
			}
		}
	
		stream->Read(size, data);
		numBuffersToQueue++;

		alGenBuffers( 1, &streamingBuffers[i] );
		error = alGetError();
		if( error != AL_NO_ERROR )
		{
			plStatusLog::AddLineS("audio.log", "Failed to create sound buffer %d", error);
			return false;
		}

		ALenum format = IGetALFormat(fBufferDesc->lpwfxFormat->wBitsPerSample, fBufferDesc->lpwfxFormat->nChannels);
		alBufferData( streamingBuffers[i], format, data, size, fBufferDesc->lpwfxFormat->nSamplesPerSec );
		if( (error = alGetError()) != AL_NO_ERROR )
			plStatusLog::AddLineS("audio.log", "alBufferData");
	}

	 // Generate AL Source
    alGenSources( 1, &source );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create audio source %d %d", error, source);
		return false;
	}
	alSourcei(source, AL_BUFFER, nil);
	SetScalarVolume(0);
	
	
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.3048);
	alGetError();
	if( error != AL_NO_ERROR )
	{
		return false;
	}

	alSourceQueueBuffers( source, numBuffersToQueue, streamingBuffers );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to queue buffers %d", error);
		return false;
	}
	return true;
}

// this function is used when starting up a streaming sound, as opposed to restarting it due to an audio system restart.
bool plDSoundBuffer::SetupStreamingSource(void *data, unsigned bytes)
{
	unsigned char bufferData[STREAM_BUFFER_SIZE];
	unsigned int size;
	ALenum error;
	char *pData = (char *)data; 
	
	alGetError();	
	int numBuffersToQueue = 0;

	// fill buffers with data
	for( int i = 0; i < STREAMING_BUFFERS; i++ )
	{
		size = bytes < STREAM_BUFFER_SIZE ? bytes : STREAM_BUFFER_SIZE;
		if(!size)
			break;	

		MemCopy(bufferData, pData, size);
		pData += size;
		bytes-= size;
		numBuffersToQueue++;

		alGenBuffers( 1, &streamingBuffers[i] );
		error = alGetError();
		if( error != AL_NO_ERROR )
		{
			plStatusLog::AddLineS("audio.log", "Failed to create sound buffer %d", error);
			return false;
		}

		ALenum format = IGetALFormat(fBufferDesc->lpwfxFormat->wBitsPerSample, fBufferDesc->lpwfxFormat->nChannels);
		alBufferData( streamingBuffers[i], format, bufferData, size, fBufferDesc->lpwfxFormat->nSamplesPerSec );
		if( (error = alGetError()) != AL_NO_ERROR )
			plStatusLog::AddLineS("audio.log", "alBufferData");
	}

	 // Generate AL Source
    alGenSources( 1, &source );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create audio source %d %d", error, source);
		return false;
	}
	alSourcei(source, AL_BUFFER, nil);
	SetScalarVolume(0);
	
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.3048);
	alGetError();
	if( error != AL_NO_ERROR )
	{
		return false;
	}

	alSourceQueueBuffers( source, numBuffersToQueue, streamingBuffers );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to queue buffers %d", error);
		return false;
	}
	return true;
}

//============================================================================
int	plDSoundBuffer::BuffersProcessed()
{
	if(alIsSource(source)==AL_FALSE)
	{
		plStatusLog::AddLineS("audio.log", "BuffersProcessed, source invalid");
		return 0;
	}
	ALint processed = 0;
	alGetSourcei( source, AL_BUFFERS_PROCESSED, &processed );
	if(alGetError() != AL_NO_ERROR)
	{
		plStatusLog::AddLineS("audio.log", "alGetSourcei failed");
	}
	
	return processed;
}

//============================================================================
int plDSoundBuffer::BuffersQueued()
{
	if(alIsSource(source)==AL_FALSE) return 0;
	ALint queued = 0;
	alGetSourcei( source, AL_BUFFERS_QUEUED, &queued );
	alGetError();
	
	return queued;

}

//============================================================================ 
bool plDSoundBuffer::StreamingFillBuffer(plAudioFileReader *stream)
{
	if(!source)
		return false;

	ALenum error;
	ALuint bufferId;
	unsigned char data[STREAM_BUFFER_SIZE];
	int buffersProcessed = BuffersProcessed();
	hsBool finished = false;

	for(int i = 0; i < buffersProcessed; i++)
	{
		alSourceUnqueueBuffers( source, 1, &bufferId );
		if( (error = alGetError()) != AL_NO_ERROR )
		{
			plStatusLog::AddLineS("audio.log", "Failed to unqueue buffer %d", error);
			return false;
		}

		if(!finished)
		{
			if(stream->NumBytesLeft() == 0)
			{
				// if at anytime we run out of data, and we are looping, reset the data stream and continue to fill buffers
				if(IsLooping())
				{
					stream->SetPosition(0);	// we are looping, so reset data stream, and keep filling buffers
				}
				else
				{
					finished = true;	// no more data, but we could still be playing, so we don't want to stop the sound yet
				}
			}

			if(!finished)
			{	unsigned int size = stream->NumBytesLeft() < STREAM_BUFFER_SIZE ? stream->NumBytesLeft() : STREAM_BUFFER_SIZE;
				stream->Read(size, data);

				ALenum format = IGetALFormat(fBufferDesc->lpwfxFormat->wBitsPerSample, fBufferDesc->lpwfxFormat->nChannels);
				alBufferData( bufferId, format, data, size, fBufferDesc->lpwfxFormat->nSamplesPerSec );
				if( (error = alGetError()) != AL_NO_ERROR )
				{
					plStatusLog::AddLineS("audio.log", "Failed to copy data to sound buffer %d", error);
					return false;
				}

				alSourceQueueBuffers( source, 1, &bufferId );
				if( (error = alGetError()) != AL_NO_ERROR )
				{
					plStatusLog::AddLineS("audio.log", "Failed to queue buffer %d", error);
					return false;
				}
			}
		}
	}
	if(!IsPlaying() && !finished)
	{
		alSourcePlay(source);
	}
	alGetError();
	return true;
}

/*****************************************************************************
*
*   Voice playback functions
*
***/

bool plDSoundBuffer::GetAvailableBufferId(unsigned *bufferId)
{
	if(mAvailableBuffers.empty())
	{
		return false;
	}
	*bufferId = mAvailableBuffers.front();
	mAvailableBuffers.pop_front();
	return true;
}

bool plDSoundBuffer::SetupVoiceSource()
{
	ALenum error;
	alGetError();

	 // Generate AL Buffers
	alGenBuffers( STREAMING_BUFFERS, streamingBuffers );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create sound buffer %d", error);
		return false;
	}

	for( int i = 0; i < STREAMING_BUFFERS; i++ )
	{
		mAvailableBuffers.push_back(streamingBuffers[i]);
	}

	 // Generate AL Source
    alGenSources( 1, &source );
	error = alGetError();
	if( error != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to create audio source %d %d", error, source);
		return false;
	}

	SetScalarVolume(0);
	
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.3048);
	alGetError();
	if( error != AL_NO_ERROR )
	{
		return false;
	}
	alSourcei(source, AL_BUFFER, nil);
	alGetError();
	//alSourcei(source, AL_PITCH, 0);

	// dont queue any buffers here
	return true;
}

//============================================================================
void plDSoundBuffer::UnQueueVoiceBuffers()
{
	unsigned buffersProcessed = BuffersProcessed();
	if(buffersProcessed)
		plStatusLog::AddLineS("audio.log", "unqueuing buffers %d", buffersProcessed);
	for(int i = 0; i < buffersProcessed; i++)
	{
		ALuint unQueued;
		alSourceUnqueueBuffers( source, 1, &unQueued );
		if(alGetError() == AL_NO_ERROR)
		{
			mAvailableBuffers.push_back(unQueued);
		}
		else
		{
			plStatusLog::AddLineS("audio.log", "Failed to unqueue buffer");
		}
	}
}

//============================================================================ 
bool plDSoundBuffer::VoiceFillBuffer(void *data, unsigned bytes, unsigned bufferId)
{
	if(!source)
		return false;
 
	ALenum error;
	unsigned int size = bytes < STREAM_BUFFER_SIZE ? bytes : STREAM_BUFFER_SIZE;

	ALenum format = IGetALFormat(fBufferDesc->lpwfxFormat->wBitsPerSample, fBufferDesc->lpwfxFormat->nChannels);
	alBufferData( bufferId, format, data, size, fBufferDesc->lpwfxFormat->nSamplesPerSec );
	if( (error = alGetError()) != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to copy data to sound buffer %d", error);
		return false;
	}
	alSourceQueueBuffers( source, 1, &bufferId );
	if( (error = alGetError()) != AL_NO_ERROR )
	{
		plStatusLog::AddLineS("audio.log", "Failed to queue buffer %d", error);
		return false;
	}
	if(!IsPlaying())
	{
		alSourcePlay(source);
	}
	alGetError();
	
	return true;
}

//// SetLooping //////////////////////////////////////////////////////////////

void	plDSoundBuffer::SetLooping( hsBool loop )
{
	fLooping = loop;
}

void plDSoundBuffer::SetMinDistance( int dist )
{
	alSourcei(source, AL_REFERENCE_DISTANCE, dist);
	ALenum error;
	if((error = alGetError()) != AL_NO_ERROR)
		plStatusLog::AddLineS("audio.log", "Failed to set min distance");
}

void plDSoundBuffer::SetMaxDistance( int dist )
{
	alSourcei(source, AL_MAX_DISTANCE, dist);
	ALenum error;
	if((error = alGetError()) != AL_NO_ERROR)
		plStatusLog::AddLineS("audio.log", "Failed to set min distance");
}

//// Play ////////////////////////////////////////////////////////////////////

void	plDSoundBuffer::Play( void )
{
	if(!source)
		return;
	ALenum error = alGetError();	// clear error

	// we dont want openal to loop our streaming buffers, or the buffer will loop back on itself. We will handle looping in the streaming sound
	if(fLooping && !fStreaming)
		alSourcei(source, AL_LOOPING, AL_TRUE);
	else
		alSourcei(source, AL_LOOPING, AL_FALSE);

	error = alGetError();
	alSourcePlay(source);
	error = alGetError();
	if(error != AL_NO_ERROR)
		plStatusLog::AddLineS("voice.log", "Play failed");
	
	plProfile_Inc( SoundPlaying );

}

//// Stop ////////////////////////////////////////////////////////////////////

void	plDSoundBuffer::Stop( void )
{
	if(!source)
		return;
	alSourceStop(source);
	alGetError();
	plProfile_Dec( SoundPlaying );
}

//============================================================================
void plDSoundBuffer::SetPosition(float x, float y, float z)
{
	alSource3f(source, AL_POSITION, x, y, -z);	// negate z coord, since openal uses opposite handedness
	alGetError();
}

//============================================================================
void plDSoundBuffer::SetOrientation(float x, float y, float z)
{
	alSource3f(source, AL_ORIENTATION, x, y, -z);	// negate z coord, since openal uses opposite handedness
	alGetError();
}

//============================================================================
void plDSoundBuffer::SetVelocity(float x, float y, float z)
{
	alSource3f(source, AL_VELOCITY, 0, 0, 0);	// no doppler shift
	alGetError();
}

//============================================================================
void plDSoundBuffer::SetConeAngles(int inner, int outer)
{
	alSourcei(source, AL_CONE_INNER_ANGLE, inner);
	alSourcei(source, AL_CONE_OUTER_ANGLE, outer);
	alGetError();
}

//============================================================================
void plDSoundBuffer::SetConeOrientation(float x, float y, float z)
{
	alSource3f(source, AL_DIRECTION, x, y, -z);	// negate z coord, since openal uses opposite handedness
	alGetError();
}

//============================================================================
// vol range: -5000 - 0
void plDSoundBuffer::SetConeOutsideVolume(int vol)
{
	float volume = (float)vol / 5000.0f + 1.0f;		// mb to scalar
	alSourcef(source, AL_CONE_OUTER_GAIN, volume);
	alGetError();
}

//============================================================================
void plDSoundBuffer::Rewind() 
{
	alSourceRewind(source);
	alGetError();
}

//// IsPlaying ///////////////////////////////////////////////////////////////

hsBool	plDSoundBuffer::IsPlaying( void )
{
	ALint state = AL_STOPPED;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	alGetError();
	return state == AL_PLAYING;
}

//// IsEAXAccelerated ////////////////////////////////////////////////////////

hsBool	plDSoundBuffer::IsEAXAccelerated( void ) const
{
	return fEAXSource.IsValid();
}

//// BytePosToMSecs //////////////////////////////////////////////////////////

UInt32	plDSoundBuffer::BytePosToMSecs( UInt32 bytePos ) const
{
	return (UInt32)(bytePos * 1000 / (hsScalar)fBufferDesc->lpwfxFormat->nAvgBytesPerSec);
}

//// GetBufferBytePos ////////////////////////////////////////////////////////

UInt32	plDSoundBuffer::GetBufferBytePos( hsScalar timeInSecs ) const
{
	hsAssert( fBufferDesc != nil && fBufferDesc->lpwfxFormat != nil, "Nil buffer description when calling GetBufferBytePos()" );

	UInt32	byte = (UInt32)( timeInSecs * (hsScalar)fBufferDesc->lpwfxFormat->nSamplesPerSec );
	byte *= fBufferDesc->lpwfxFormat->nBlockAlign;

	return byte;
}

//// GetLengthInBytes ////////////////////////////////////////////////////////

UInt32	plDSoundBuffer::GetLengthInBytes( void ) const
{
	return (UInt32)fBufferDesc->dwBufferBytes;
}

//// SetEAXSettings //////////////////////////////////////////////////////////

void	plDSoundBuffer::SetEAXSettings(  plEAXSourceSettings *settings, hsBool force )
{
	fEAXSource.SetFrom( settings, source, force );
}

//// GetBlockAlign ///////////////////////////////////////////////////////////

UInt8	plDSoundBuffer::GetBlockAlign( void ) const
{
	return ( fBufferDesc != nil && fBufferDesc->lpwfxFormat != nil ) ? fBufferDesc->lpwfxFormat->nBlockAlign : 0;
}

//// SetScalarVolume /////////////////////////////////////////////////////////
// Sets the volume, but on a range from 0 to 1

void	plDSoundBuffer::SetScalarVolume( hsScalar volume )
{
	if(source)
	{
		ALenum error;
		alSourcef(source, AL_GAIN, volume); 
		if((error = alGetError()) != AL_NO_ERROR)
			plStatusLog::AddLineS("audio.log", "failed to set volume on source %d", error);
	}
}

unsigned plDSoundBuffer::GetByteOffset()
{
	ALint bytes;
	alGetSourcei(source, AL_BYTE_OFFSET, &bytes);
	ALenum error = alGetError();
	return bytes;
}

float plDSoundBuffer::GetTimeOffsetSec()
{
	float time;
	alGetSourcef(source, AL_SEC_OFFSET, &time);
	ALenum error = alGetError();
	return time;
}

void plDSoundBuffer::SetTimeOffsetSec(float seconds)
{
	alSourcef(source, AL_SEC_OFFSET, seconds);
	ALenum error = alGetError();
}

void plDSoundBuffer::SetTimeOffsetBytes(unsigned bytes)
{
	alSourcef(source, AL_BYTE_OFFSET, bytes);
	ALenum error = alGetError();
}
		