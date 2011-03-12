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

#ifndef _plDSoundBuffer_h
#define _plDSoundBuffer_h

#include "hsStlUtils.h"
#include "hsTemplates.h"
#include "plEAXEffects.h"
#define STREAMING_BUFFERS 16
#define	STREAM_BUFFER_SIZE	    4608*4

//#define VOICE_BUFFERS 4
//#define VOICE_BUFFER_SIZE	4608

class plWAVHeader;
class plAudioFileReader;

typedef struct tWAVEFORMATEX WAVEFORMATEX;
typedef struct _DSBUFFERDESC DSBUFFERDESC;


// Ported to OpenAL from DirectSound May 2006. Idealy the openal sources would be seperate from this class.
// OpenAl sound buffer, and source. 
class plDSoundBuffer
{
public:
	plDSoundBuffer( UInt32 size, plWAVHeader &bufferDesc, hsBool enable3D, hsBool looping, hsBool tryStatic = false, bool streaming = false );
	~plDSoundBuffer();

	void		Play( void );
	void		Stop( void );
	void		Rewind() ;
	
	UInt32		GetLengthInBytes( void ) const;
	void		SetScalarVolume( hsScalar volume );	// Sets the volume, but on a range from 0 to 1

	unsigned	GetSource() { return source; }
	void		SetPosition(float x, float y, float z);
	void		SetOrientation(float x, float y, float z);
	void		SetVelocity(float x, float y, float z);
	void		SetConeAngles(int inner, int outer);
	void		SetConeOrientation(float x, float y, float z);
	void		SetConeOutsideVolume(int vol);

	void		SetLooping( hsBool loop );
	void		SetMinDistance( int dist);
	void		SetMaxDistance( int dist );

	hsBool		IsValid( void ) const { return fValid; }
	hsBool		IsPlaying( void );
	hsBool		IsLooping( void ) const { return fLooping; }
	hsBool		IsEAXAccelerated( void ) const;

	bool		FillBuffer(void *data, unsigned bytes, plWAVHeader *header);

	// Streaming support
	bool		SetupStreamingSource(plAudioFileReader *stream);
	bool		SetupStreamingSource(void *data, unsigned bytes);
	int			BuffersProcessed();
	bool		StreamingFillBuffer(plAudioFileReader *stream);

	bool		SetupVoiceSource();
	bool		VoiceFillBuffer(void *data, unsigned bytes, unsigned buferId);
	void		UnQueueVoiceBuffers();

	
	unsigned	GetByteOffset();
	UInt32		GetBufferBytePos( hsScalar timeInSecs ) const;
	UInt32		BytePosToMSecs( UInt32 bytePos ) const;

	void			SetEAXSettings(  plEAXSourceSettings *settings, hsBool force = false );
	void			SetTimeOffsetBytes(unsigned bytes);
	UInt8			GetBlockAlign( void ) const;
	static UInt32	GetNumBuffers() { return fNumBuffers; }
	float			GetDefaultMinDistance() { return fDefaultMinDistance; }
	bool			GetAvailableBufferId(unsigned *bufferId);
	unsigned		GetNumQueuedBuffers(){ return fNumQueuedBuffers;} // returns the max number of buffers queued on a source

	float		GetTimeOffsetSec();
	void		SetTimeOffsetSec(float seconds);
	int			BuffersQueued();

protected:

	enum BufferType
	{
		kStatic,
		kStreaming,
		kVoice,
	};

	BufferType			fType;
	hsBool				fValid, fLooping;
	UInt32				fLockLength;
	void *				fLockPtr;
	
	hsTArray<UInt32>	fPosNotifys;
	bool				fStreaming;
	DSBUFFERDESC*		fBufferDesc;

	unsigned			buffer;									// used if this is not a streaming buffer
	unsigned			streamingBuffers[STREAMING_BUFFERS];	// used if this is a streaming buffer
	std::list<unsigned>	mAvailableBuffers;						// used for doing our own buffer management. Specifically voice chat, since we dont want old buffers queued 

	unsigned			source;
	unsigned int		fStreamingBufferSize;

	plEAXSource			fEAXSource;
	
	static UInt32		fNumBuffers;
	static float		fDefaultMinDistance;

	unsigned			fNumQueuedBuffers;
	hsScalar			fPrevVolume;

	void	IAllocate( UInt32 size, plWAVHeader &bufferDesc, hsBool enable3D, hsBool tryStatic );
	void	IRelease( void );
	int		IGetALFormat(unsigned bitsPerSample, unsigned int numChannels);
};

#endif //_plDSoundBuffer_h
