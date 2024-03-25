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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDSoundBuffer - Simple wrapper class for a DirectSound buffer.         //
//                   Allows us to simplify all the work done behind the     //
//                   scenes in plWin32BufferThread.                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDSoundBuffer_h
#define _plDSoundBuffer_h

#include <list>
#include "plEAXEffects.h"

#define STREAMING_BUFFERS 16
#define STREAM_BUFFER_SIZE      4608*4

//#define VOICE_BUFFERS 4
//#define VOICE_BUFFER_SIZE 4608

class plWAVHeader;
class plAudioFileReader;


// Ported to OpenAL from DirectSound May 2006. Idealy the openal sources would be seperate from this class.
// OpenAl sound buffer, and source. 
class plDSoundBuffer
{
public:
    plDSoundBuffer( uint32_t size, plWAVHeader &bufferDesc, bool enable3D, bool looping, bool tryStatic = false, bool streaming = false );
    ~plDSoundBuffer();

    void        Play();
    void        Stop();
    void        Rewind();
    void        Pause();

    uint32_t      GetLengthInBytes() const;
    void        SetScalarVolume( float volume ); // Sets the volume, but on a range from 0 to 1

    unsigned    GetSource() { return source; }
    void        SetPosition(float x, float y, float z);
    void        SetOrientation(float x, float y, float z);
    void        SetVelocity(float x, float y, float z);
    void        SetConeAngles(int inner, int outer);
    void        SetConeOrientation(float x, float y, float z);
    void        SetConeOutsideVolume(int vol);

    void        SetLooping( bool loop );
    void        SetMinDistance( int dist);
    void        SetMaxDistance( int dist );

    bool        IsValid() const { return fValid; }
    bool        IsPlaying();
    bool        IsLooping() const { return fLooping; }
    bool        IsEAXAccelerated() const;

    bool        FillBuffer(void *data, unsigned bytes, plWAVHeader *header);

    // Streaming support
    bool        SetupStreamingSource(plAudioFileReader *stream);
    bool        SetupStreamingSource(void *data, unsigned bytes);
    int         BuffersProcessed();
    bool        StreamingFillBuffer(plAudioFileReader *stream);

    bool        SetupVoiceSource();
    bool        VoiceFillBuffer(const void *data, size_t bytes, unsigned buferId);
    void        UnQueueVoiceBuffers();

    
    unsigned    GetByteOffset();
    uint32_t      GetBufferBytePos( float timeInSecs ) const;
    uint32_t      bytePosToMSecs( uint32_t bytePos ) const;

    void            SetEAXSettings(  plEAXSourceSettings *settings, bool force = false );
    void            SetTimeOffsetBytes(unsigned bytes);
    uint8_t           GetBlockAlign() const;
    static uint32_t   GetNumBuffers() { return fNumBuffers; }
    float           GetDefaultMinDistance() { return fDefaultMinDistance; }
    bool            GetAvailableBufferId(unsigned *bufferId);
    unsigned        GetNumQueuedBuffers(){ return fNumQueuedBuffers;} // returns the max number of buffers queued on a source

    float       GetTimeOffsetSec();
    void        SetTimeOffsetSec(float seconds);
    int         BuffersQueued();

protected:

    enum BufferType
    {
        kStatic,
        kStreaming,
        kVoice,
    };

    BufferType          fType;
    bool                fValid, fLooping;
    uint32_t              fLockLength;
    void *              fLockPtr;

    bool                fStreaming;
    plWAVHeader*        fBufferDesc;
    uint32_t              fBufferSize;

    unsigned            buffer;                                 // used if this is not a streaming buffer
    unsigned            streamingBuffers[STREAMING_BUFFERS];    // used if this is a streaming buffer
    std::list<unsigned> mAvailableBuffers;                      // used for doing our own buffer management. Specifically voice chat, since we dont want old buffers queued 

    unsigned            source;
    unsigned int        fStreamingBufferSize;

    plEAXSource         fEAXSource;
    
    static uint32_t       fNumBuffers;
    static float        fDefaultMinDistance;

    unsigned            fNumQueuedBuffers;
    float            fPrevVolume;

    void    IAllocate( uint32_t size, plWAVHeader &bufferDesc, bool enable3D, bool tryStatic );
    void    IRelease();
    int     IGetALFormat(unsigned bitsPerSample, unsigned int numChannels);
};

#endif //_plDSoundBuffer_h
