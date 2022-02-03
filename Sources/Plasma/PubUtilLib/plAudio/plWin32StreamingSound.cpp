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
#include "HeadSpin.h"
#include "hsTimer.h"
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "plProfile.h"

#include "plWin32Sound.h"
#include "plWin32StreamingSound.h"
#include "plDSoundBuffer.h"
#include "plAudioSystem.h"
#include "plSrtFileReader.h"

#include "plAudioCore/plAudioFileReader.h"
#include "plAudioCore/plSoundBuffer.h"
#include "plAudioCore/plSoundDeswizzler.h"
#include "pnMessage/plSoundMsg.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plStatusLog/plStatusLog.h"
#include "plMessage/plSubtitleMsg.h"

#include <regex>

#if HS_BUILD_FOR_WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

#define STREAMING_UPDATE_MS 200

plProfile_Extern(MemSounds);
plProfile_CreateAsynchTimer( "Stream Shove Time", "Sound", StreamSndShoveTime );
plProfile_CreateAsynchTimer( "Stream Swizzle Time", "Sound", StreamSwizzleTime );
plProfile_Extern( SoundLoadTime );

plWin32StreamingSound::plWin32StreamingSound()
    : fDataStream(), fBlankBufferFillCounter(), fDeswizzler(), fStreamType(kNoStream),
      fLastStreamingUpdate(), fStopping(), fPlayWhenStopped(), fStartPos(), fSrtFileReader(),
      fTimeAtBufferStart(), fIsCompressed(), fBufferLengthInSecs(plgAudioSys::GetStreamingBufferSize())
{ }

plWin32StreamingSound::~plWin32StreamingSound()
{
    /// Call before we delete our dataStream
    DeActivate();
    IUnloadDataBuffer();

    delete fDataStream;
    delete fDeswizzler;
    delete fSrtFileReader;
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
    fFailed = false;    // just in case the last sound failed to load turn this off so it can try to load the new sound
}

//////////////////////////////////////////////////////////////
//  Override, 'cause we don't want to actually LOAD the sound for streaming,
//  just make sure it's decompressed and such and ready to stream.
plSoundBuffer::ELoadReturnVal plWin32StreamingSound::IPreLoadBuffer( bool playWhenLoaded, bool isIncidental /* = false */ )
{
    if(fPlayWhenStopped)
        return plSoundBuffer::kPending;
    bool sfxPath = fNewFilename.IsValid() ? false : true;

    if (fDataStream != nullptr && !fNewFilename.IsValid())
        return plSoundBuffer::kSuccess;     // Already loaded

    if(!ILoadDataBuffer())
    {
        return plSoundBuffer::kError;
    }
    plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();      
    if(!buffer)
        return plSoundBuffer::kError;
    
    // The databuffer also needs to know if the source is compressed or not
    if (fNewFilename.IsValid())
    {
        buffer->SetFileName(fNewFilename);
        buffer->SetFlag(plSoundBuffer::kStreamCompressed, fIsCompressed);
        fNewFilename = "";
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
        
        fSrcFilename = buffer->GetFileName();

        delete fDataStream;
        fDataStream = buffer->GetAudioReader();
        if(!fDataStream)
        {
            plAudioCore::ChannelSelect select = buffer->GetReaderSelect();

            /// Open da file
            plFileName strPath = plFileSystem::GetCWD();

            if (sfxPath)
            {
                strPath = plFileName::Join(strPath, "sfx");
            }
            strPath = plFileName::Join(strPath, fSrcFilename);
            fDataStream = plAudioFileReader::CreateReader(strPath, select, type);
        }

        // check if subtitles are enabled and if fSrcFilename is a localized audio file (e.g., ending in _eng, _fre, etc.)
        // TODO: surely there is already a function somewhere to do this localization filename check?
        if (plgAudioSys::IsEnabledSubtitles() && std::regex_match(fSrcFilename.StripFileExt().AsString().c_str(), std::regex(".*_[eng|fre|ger|spa|ita|jpn]\..*", std::regex_constants::icase)))
        {
            delete fSrtFileReader;
            fSrtFileReader = new plSrtFileReader(fSrcFilename);
            fSrtFileReader->ReadFile();
        }

        if (fDataStream == nullptr || !fDataStream->IsValid())
        {
            delete fDataStream;
            fDataStream = nullptr;
            return plSoundBuffer::kError;
        }

        IPrintDbgMessage(ST::format("   Readied file {} for streaming", fSrcFilename).c_str());

        // dont free sound data until we have a chance to use it in load sound

        return fDataStream ? plSoundBuffer::kSuccess : plSoundBuffer::kError;
    }

    plStatusLog::AddLineSF("audio.log", "EnsureLoadable failed for streaming sound {}", fDataBufferKey->GetName());
    return plSoundBuffer::kError;
}

void plWin32StreamingSound::IFreeBuffers()
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
            fDataStream = nullptr;
        }
        fSrcFilename = "";
    }
}

///////////////////////////////////////////////////////////////////
//  Overload from plSound. Basically sets up the streaming file and fills the
//  first half of our buffer. We'll fill the rest of the buffer as we get
//  notifications for such.

bool plWin32StreamingSound::LoadSound( bool is3D )
{
    if( fFailed )
        return false;
    if( !plgAudioSys::Active() || fDSoundBuffer )
        return false;

    if( fPriority > plgAudioSys::GetPriorityCutoff() )
        return false;   // Don't set the failed flag, just return

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
        ST::string str = ST::format("Unable to open streaming source {}",
                                    fDataBufferKey->GetName());
        IPrintDbgMessage( str.c_str(), true );
        fFailed = true;
        return false;
    }

    SetProperty( kPropIs3DSound, is3D );

    plWAVHeader header = fDataStream->GetHeader();
    uint32_t bufferSize = (uint32_t)(fBufferLengthInSecs * header.fAvgBytesPerSec); 

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
        bufferSize              /= header.fNumChannels;
        header.fBlockAlign      /= header.fNumChannels;
        header.fAvgBytesPerSec  /= header.fNumChannels;
        header.fNumChannels = 1;
    }

    // Actually create the buffer now (always looping)
    fDSoundBuffer = new plDSoundBuffer( bufferSize, header, is3D, IsPropertySet(kPropLooping), false, true );
    if( !fDSoundBuffer->IsValid() )
    {
        fDataStream->Close();
        delete fDataStream;
        fDataStream = nullptr;

        delete fDSoundBuffer;
        fDSoundBuffer = nullptr;

        ST::string str = ST::format("Can't create sound buffer for {}.wav. This could happen if the wav file is a stereo file."
                                    " Stereo files are not supported on 3D sounds. If the file is not stereo then please report this error.",
                                    GetFileName());
        IPrintDbgMessage(str.c_str(), true);
        fFailed = true;
        return false;
    }

    fTotalBytes = (uint32_t)bufferSize;
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
            plStatusLog::AddLineSF("syncaudio.log", "startpos {}", fStartPos);
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
        fDataStream = nullptr;
        delete fDSoundBuffer;
        fDSoundBuffer = nullptr;

        plStatusLog::AddLineSF("audio.log", "Could not play streaming sound, no voices left {}", GetKeyName());
        return false;
    }
    FreeSoundData();
    
    IRefreshEAXSettings( true );

    // Debug info
    ST::string dbg = ST::format("   Streaming {}.", fSrcFilename);
    IPrintDbgMessage(dbg.c_str());

    plStatusLog::AddLineSF("audioTimes.log", 0xffffffff, "Streaming {4.2f} secs of {}",
                           fDataStream->GetLengthInSecs(), GetKey()->GetUoid().GetObjectName() );

    // Get pertinent info
    SetLength( (float)fDataStream->GetLengthInSecs() );

    // Set up our deswizzler, if necessary
    delete fDeswizzler;
    if( fDataStream->GetHeader().fNumChannels != header.fNumChannels )
        fDeswizzler = new plSoundDeswizzler( (uint32_t)(fBufferLengthInSecs * fDataStream->GetHeader().fAvgBytesPerSec), 
                                             (uint8_t)(fDataStream->GetHeader().fNumChannels), 
                                             header.fBitsPerSample / 8 );
    else
        fDeswizzler = nullptr;

    // LEAVE THE WAV FILE OPEN! (We *are* streaming, after all :)
    return true;
}

void plWin32StreamingSound::IStreamUpdate()
{
    if(!fReallyPlaying)
        return;

    if(hsTimer::GetMilliSeconds() - fLastStreamingUpdate < STREAMING_UPDATE_MS) // filter out update requests so we aren't doing this more that we need to 
        return;

    if (fSrtFileReader != nullptr)
    {
        plSrtEntry* nextEntry = nullptr;
        do
        {
            nextEntry = fSrtFileReader->GetNextEntryStartingBeforeTime((int)(this->GetActualTimeSec() * 1000.0f));

            if (nextEntry != nullptr)
            {
                // add a plSubtitleMsg to go... to whoever is listening (probably the KI)
                plSubtitleMsg* msg = new plSubtitleMsg(nextEntry->GetSubtitleText());
                msg->Send();
            }
        } while (nextEntry != nullptr);
    }

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
            plStatusLog::AddLineSF("audio.log", "{} Streaming buffer fill failed", GetKeyName());
        }
    }
    plProfile_EndTiming( StreamSndShoveTime );
}

void plWin32StreamingSound::Update()
{
    plWin32Sound::Update();
    IStreamUpdate();
}

void plWin32StreamingSound::IDerivedActuallyPlay()
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

                // throw away any subtitles that would end before the synched start time
                if (fSrtFileReader != nullptr) {
                    plSrtEntry* nextEntry = nullptr;
                    do
                    {
                        nextEntry = fSrtFileReader->GetNextEntryEndingBeforeTime(fSynchedStartTimeSec * 1000.0);
                    } while (nextEntry != nullptr);
                }
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
    plSoundEvent    *event = IFindEvent( plSoundEvent::kStart );
    if (event != nullptr)
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
    if (fDataStream && fDSoundBuffer)
    {
        uint32_t totalSize = fDataStream->GetDataSize();
        uint32_t bytesRemaining = fDataStream->NumBytesLeft();
        unsigned bytesQueued = fDSoundBuffer->BuffersQueued() * STREAM_BUFFER_SIZE;
        unsigned offset = fDSoundBuffer->GetByteOffset();
        long byteoffset = ((totalSize - bytesRemaining) - bytesQueued) + offset;

        return byteoffset < 0 ? totalSize - std::abs(byteoffset) : byteoffset;
    }

    return 0;
}

float plWin32StreamingSound::GetActualTimeSec()
{
    if (fDataStream && fDSoundBuffer)
        return fDSoundBuffer->bytePosToMSecs(this->GetByteOffset()) / 1000.0f;
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
    //  fStartTimeSec = t;
}

bool plWin32StreamingSound::MsgReceive( plMessage* pMsg )
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
