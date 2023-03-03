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
//  plWin32GroupedSound - Grouped version of a static sound. Lots of short  //
//                        sounds stored in the buffer, all share the same   //
//                        DSound playback buffer and only one plays at a    //
//                        time.                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "plWin32GroupedSound.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plDSoundBuffer.h"

#include "plAudioSystem.h"
#include "plAudioCore/plSoundBuffer.h"
#include "plAudioCore/plSoundDeswizzler.h"

#include "pnMessage/plSoundMsg.h"

#include "plStatusLog/plStatusLog.h"

plProfile_Extern( MemSounds );
plProfile_Extern( StaticSndShoveTime );
plProfile_Extern( StaticSwizzleTime );

/////////////////////////////////////////////////////////////////////////////////////////////////

void    plWin32GroupedSound::SetPositionArray(size_t numSounds, const uint32_t *posArray, const float *volumeArray)
{
    fStartPositions.assign(posArray, posArray + numSounds);
    fVolumes.assign(volumeArray, volumeArray + numSounds);
}

//// IRead/IWrite ////////////////////////////////////////////////////////////

void plWin32GroupedSound::IRead( hsStream *s, hsResMgr *mgr )
{
    plWin32StaticSound::IRead( s, mgr );
    uint16_t n = s->ReadLE16();
    fStartPositions.resize(n);
    fVolumes.resize(n);
    for (uint16_t i = 0; i < n; i++)
    {
        fStartPositions[ i ] = s->ReadLE32();
        fVolumes[i] = s->ReadLEFloat();
    }
}

void plWin32GroupedSound::IWrite( hsStream *s, hsResMgr *mgr )
{
    plWin32StaticSound::IWrite( s, mgr );

    s->WriteLE16((uint16_t)fStartPositions.size());
    for (size_t i = 0; i < fStartPositions.size(); i++)
    {
        s->WriteLE32( fStartPositions[ i ] );
        s->WriteLEFloat(fVolumes[i]);
    }
}

//// LoadSound ///////////////////////////////////////////////////////////////

bool    plWin32GroupedSound::LoadSound( bool is3D )
{
    if( fFailed )
        return false;

    if( fPriority > plgAudioSys::GetPriorityCutoff() )
        return false;   // Don't set the failed flag, just return

    if (!plgAudioSys::Active() || fDSoundBuffer != nullptr)
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

    if( retVal == plSoundBuffer::kPending)  // we are still reading data. 
    {
        return true;
    }

    // We need it to be resident to read in
    if( retVal == plSoundBuffer::kError) 
    {
        ST::string str = ST::format("Unable to open .wav file {}", fDataBufferKey ? fDataBufferKey->GetName() : "nil");
        IPrintDbgMessage(str, true);
        fFailed = true;
        return false;
    }
    
    SetProperty( kPropIs3DSound, is3D );


    plWAVHeader header = buffer->GetHeader();

    // Debug flag #2
    if( fChannelSelect == 0 && header.fNumChannels > 1 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableLeftSelect ) )
    {
        // Force a fail
        fFailed = true;
        return false;
    }

    // Calculate the maximum size for our buffer. This will be the length of the longest sound we're going to 
    // have to play.
    uint32_t maxSoundSize = 0, len = 0;
    for (size_t i = 1; i < fStartPositions.size(); i++)
    {
        len = fStartPositions[ i ] - fStartPositions[ i - 1 ];
        if( len > maxSoundSize )
            maxSoundSize = len;
    }
    len = buffer->GetDataLength() - fStartPositions.back();
    if( len > maxSoundSize )
        maxSoundSize = len;

    // Based on that, allocate our buffer
    uint32_t bufferSize = maxSoundSize - ( maxSoundSize % header.fBlockAlign );

    if( header.fNumChannels > 1 && is3D )
    {
        // We can only do a single channel of 3D sound. So copy over one (later)
        bufferSize              /= header.fNumChannels;
        header.fBlockAlign      /= header.fNumChannels;
        header.fAvgBytesPerSec  /= header.fNumChannels;
        header.fNumChannels = 1;
    }
    fNumDestChannels = (uint8_t)(header.fNumChannels);
    fNumDestBytesPerSample = (uint8_t)(header.fBlockAlign);

    // Create our DSound buffer (or rather, the wrapper around it)
    fDSoundBuffer = new plDSoundBuffer( bufferSize, header, is3D, IsPropertySet( kPropLooping ), true );
    if( !fDSoundBuffer->IsValid() )
    {
        ST::string str = ST::format("Can't create sound buffer for {}.wav. This could happen if the wav file is a stereo file."
                                    " Stereo files are not supported on 3D sounds. If the file is not stereo then please report this error.",
                                    GetFileName());
        IPrintDbgMessage(str, true);
        fFailed = true;

        delete fDSoundBuffer;
        fDSoundBuffer = nullptr;

        return false;
    }
    
    IRefreshEAXSettings( true );

    // Fill the buffer with whatever our current sound is.
    IFillCurrentSound( 0 );

    // Logging
    ST::string str = ST::format("   Grouped {} {} allocated ({} msec).", buffer->GetFileName().IsValid() ? "file" : "buffer",
                                buffer->GetFileName().IsValid() ? buffer->GetFileName() : buffer->GetKeyName(),
                                //fDSoundBuffer->IsHardwareAccelerated() ? "hardware" : "software",
                                //fDSoundBuffer->IsStaticVoice() ? "static" : "dynamic",
#ifdef PL_PROFILE_ENABLED
                            gProfileVarStaticSndShoveTime.GetValue());
#else
                            0);
#endif
    IPrintDbgMessage(str);
    if (GetKey() != nullptr && GetKeyName().contains("Footstep"))
        ;
    else
        plStatusLog::AddLineSF( "audioTimes.log", "{} ({})", str, GetKey() ? GetKeyName() : ST_LITERAL("unkeyed") );

    fTotalBytes = bufferSize;

    plProfile_NewMem( MemSounds, fTotalBytes );

    // All done!
//  if( fLoadFromDiskOnDemand )
//      IUnloadDataBuffer();
    FreeSoundData();
    return true;

}

//// GetSoundLength //////////////////////////////////////////////////////////
//  Gets the length (in seconds) of the given sound index from the group.

float    plWin32GroupedSound::GetSoundLength(uint16_t soundIndex)
{
    plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();
    if(buffer)
    {
        return (float)IGetSoundbyteLength( soundIndex ) / buffer->GetHeader().fAvgBytesPerSec;
    }
    
    return 0;
}

//// IGetSoundbyteLength /////////////////////////////////////////////////////
//  uint8_t version of above.

uint32_t      plWin32GroupedSound::IGetSoundbyteLength(uint16_t soundIndex)
{
    if (soundIndex == fStartPositions.size() - 1)
        return ((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetDataLength() - fStartPositions[ soundIndex ];
    else
        return fStartPositions[ soundIndex + 1 ] - fStartPositions[ soundIndex ];
}

//// IGetDataPointer/Length //////////////////////////////////////////////////
// Abstracting a few things here for the incidentalMgr

void    *plWin32GroupedSound::IGetDataPointer() const
{
    return (fDataBufferKey->ObjectIsLoaded()) ? (void *)((uint8_t *)((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetData() + fStartPositions[fCurrentSound]) : nullptr;
}

uint32_t  plWin32GroupedSound::IGetDataLength() const
{
    return ( fDataBufferKey->ObjectIsLoaded() ) ? fCurrentSoundLength : 0;
}

//// IFillCurrentSound ///////////////////////////////////////////////////////
//  Fills the DSoundBuffer with data from the current sound from our sound
//  group, optionally switching what our current sound is.

void    plWin32GroupedSound::IFillCurrentSound( int16_t newCurrent /*= -1*/ )
{
    //void  *dataPtr;
    //uint32_t    dataLength;

    if( !fDSoundBuffer && plgAudioSys::Active() )
        LoadSound( IsPropertySet( kPropIs3DSound ) );

    plProfile_BeginTiming( StaticSndShoveTime );

    // Make sure we're stopped first. Don't want to be filling while we're playing
    Stop();

    if( newCurrent != -1 )
    {
        fCurrentSound = (uint16_t)newCurrent;

        if (fCurrentSound >= fStartPositions.size())
        {
            // Invalid index!
            hsAssert( false, "Invalid index in plWin32GroupedSound::IFillCurrentSound()" );
            fCurrentSound = -1;
            return;
        }

        // Set our length based on the current sound
        fCurrentSoundLength = IGetSoundbyteLength( fCurrentSound );
        if( fDataBufferKey->ObjectIsLoaded() )
            SetLength( fCurrentSoundLength / ((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetHeader().fAvgBytesPerSec );
    
        // Update our volume as well
        SetVolume( fVolumes[ fCurrentSound ] );
    }

    if (fDSoundBuffer != nullptr)
    {
        /// Lock our buffer
        //fDSoundBuffer->Lock( dataLength, dataPtr );

        /// Copy or de-swizzle?
        //if( fDataBuffer->GetHeader().fNumChannels == fNumDestChannels )
        {
            // Just copy
            //memcpy( dataPtr, (uint8_t *)fDataBuffer->GetData() + fStartPositions[ fCurrentSound ], fCurrentSoundLength );
            //dataPtr = (uint8_t *)dataPtr + fCurrentSoundLength;
            //dataLength -= fCurrentSoundLength;
        }
        //else
        {
            // We're extracting a single channel of sound into our sound buffer, so it isn't a straight copy...

            /*plProfile_BeginTiming( StaticSwizzleTime );

            plSoundDeswizzler   deswiz( (uint8_t *)fDataBuffer->GetData() + fStartPositions[ fCurrentSound ], fCurrentSoundLength, 
                                        (uint8_t)(fDataBuffer->GetHeader().fNumChannels), fNumDestBytesPerSample );

            deswiz.Extract( fChannelSelect, dataPtr );

            dataPtr = (uint8_t *)dataPtr + fCurrentSoundLength / fDataBuffer->GetHeader().fNumChannels;
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

void plWin32GroupedSound::IDerivedActuallyPlay()
{
    // Ensure there's a stop notify for us
    if( !fReallyPlaying )
    {
        fDSoundBuffer->Play();
        fReallyPlaying = true;
    }

    plSoundEvent    *event = IFindEvent( plSoundEvent::kStart );
    if (event != nullptr)
        event->SendCallbacks();
}

bool    plWin32GroupedSound::MsgReceive( plMessage* pMsg )
{
    plSoundMsg *soundMsg = plSoundMsg::ConvertNoRef( pMsg );
    if (soundMsg != nullptr && soundMsg->Cmd(plSoundMsg::kSelectFromGroup))
    {
        IFillCurrentSound( soundMsg->fIndex );
        return true;
    }
    else if (soundMsg != nullptr && soundMsg->Cmd(plSoundMsg::kPlay))
    {
        Play();
        //plIncidentalMgr::GetInstance()->Play( this, plIncidentalMgr::kNormal );
        return true;
    }

    return plWin32StaticSound::MsgReceive( pMsg );
}

