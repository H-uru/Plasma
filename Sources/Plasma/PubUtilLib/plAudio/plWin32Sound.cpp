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
#include "plWin32Sound.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsGeometry3.h"
#include "plProfile.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "plAudioSystem.h"
#include "plDSoundBuffer.h"

#include "pnMessage/plEventCallbackMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plAudible/plWinAudible.h"
#include "plAudioCore/plSrtFileReader.h"
#include "plAudioCore/plWavFile.h"
#include "plMessage/plSubtitleMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plPipeline/plPlates.h"
#include "plStatusLog/plStatusLog.h"

#if HS_BUILD_FOR_WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

plProfile_CreateMemCounter("Sounds", "Memory", MemSounds);
plProfile_Extern(SoundPlaying);

void plWin32Sound::Activate( bool forcePlay )
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

void plWin32Sound::IFreeBuffers()
{
    if (fDSoundBuffer != nullptr)
    {
        delete fDSoundBuffer;
        fDSoundBuffer = nullptr;
        plProfile_DelMem(MemSounds, fTotalBytes);
        fTotalBytes = 0;
    }

    fPositionInited = false;
    fAwaitingPosition = false;
}

void plWin32Sound::Update()
{
    plSoundBuffer* buf = GetDataBuffer();
    if (buf != nullptr) {
        plSrtFileReader* srtReader = buf->GetSrtReader();
        if (srtReader != nullptr) {
            uint32_t currentTimeMs = (uint32_t)(GetActualTimeSec() * 1000.0f);
            if (currentTimeMs <= srtReader->GetLastEntryEndTime()) {
                while (plSrtEntry* nextEntry = srtReader->GetNextEntryStartingBeforeTime(currentTimeMs)) {
                    if (plgAudioSys::AreSubtitlesEnabled()) {
                        // add a plSubtitleMsg to go... to whoever is listening (probably the KI)
                        plSubtitleMsg* msg = new plSubtitleMsg(nextEntry->GetSubtitleText(), nextEntry->GetSpeakerName());
                        msg->Send();
                    }
                }
            }
        }
    }

    plSound::Update();
}

void plWin32Sound::IActuallyPlay()
{
    //plSound::Play();
    if (!fDSoundBuffer && plgAudioSys::Active())
        LoadSound( IsPropertySet( kPropIs3DSound ) );
    if(!fLoading )
    {
        if (fDSoundBuffer && plgAudioSys::Active() )
        {
            if (!fReallyPlaying && fSynchedStartTimeSec > 0) {
                // advance past any subtitles that would end before the synched start time
                // not sure when this actually happens...
                plSoundBuffer* buf = GetDataBuffer();
                if (buf != nullptr) {
                    plSrtFileReader* srtReader = buf->GetSrtReader();
                    if (srtReader != nullptr) {
                        srtReader->AdvanceToTime(fSynchedStartTimeSec * 1000.0);
                    }
                }
            }

            // Sometimes base/derived classes can be annoying
            IDerivedActuallyPlay();
            RefreshVolume();        
        }
        else
        {
            // If we can't load (for ex., if audio is off), then we act like we played and then stopped
            // really fast. Thus, we need to send *all* of our callbacks off immediately and then Stop().
            for (plSoundEvent* event : fSoundEvents)
                event->SendCallbacks();

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
        if (fDSoundBuffer != nullptr)
        {
            if(IsPropertySet(kPropIncidental))
            {
                --fIncidentalsPlaying;
            }
            fDSoundBuffer->Stop();
            plStatusLog::AddLineSF("impacts.log", "Stopping {}", GetKeyName());
        
        }
        fReallyPlaying = false;
    }
    else
    {
        if (fDSoundBuffer != nullptr && fDSoundBuffer->IsPlaying())
        {
            plStatusLog::AddLineSF( "audio.log", 0xffff0000, "WARNING: BUFFER FLAGGED AS STOPPED BUT NOT STOPPED - {}", GetKey() ? GetKeyName() : ST_LITERAL("(nil)") );
            fDSoundBuffer->Stop();
        }
    }

    // send callbacks
    plSoundEvent    *event = IFindEvent( plSoundEvent::kStop );
    if (event != nullptr)
    {
        event->SendCallbacks();
    }

    plSound::IActuallyStop();
}

plSoundMsg* plWin32Sound::GetStatus(plSoundMsg* pMsg)
{
    plSoundMsg* pReply = new plSoundMsg;
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

void    plWin32Sound::SetMax( const int m )
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

void plWin32Sound::SetConeOrientation( float x, float y, float z )
{
    plSound::SetConeOrientation(x, y, z);
    if(fDSoundBuffer)
    {
        fDSoundBuffer->SetConeOrientation(x, z, y);
    }
}

void plWin32Sound::SetVelocity( const hsVector3& vel )
{
    plSound::SetVelocity(vel);
    if( fDSoundBuffer)
        fDSoundBuffer->SetVelocity(vel.fX, vel.fZ, vel.fY);
}

void plWin32Sound::SetPosition( const hsPoint3& pos )
{
    plSound::SetPosition(pos);
    if(fDSoundBuffer)
    {
        // in openal sounds that are mono are played as positional. Since gui's may be positioned way off in space, the sound may not be audible.
        // doing this allows us to play mono gui sounds and still hear them, since this attaches the sound to the listener.
        if(fType == kGUISound)
        {
            hsPoint3 listenerPos = plgAudioSys::GetCurrListenerPos();
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

void plWin32Sound::ISetActualVolume(float volume)
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
    IUpdateDebugPlate();    // uint8_t me.
}

//////////////////////////////////////////////////////////////
//  The base class will make sure all our params are correct and up-to-date,
//  but before it does its work, we have to make sure our buffer is ready to
//  be updated.
void plWin32Sound::IRefreshParams()
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

void plWin32Sound::IRefreshEAXSettings( bool force )
{
    if (fDSoundBuffer != nullptr)
        fDSoundBuffer->SetEAXSettings( &GetEAXSettings(), force );
}

void plWin32Sound::IAddCallback( plEventCallbackMsg *pMsg )
{
    plSoundEvent::Types type = plSoundEvent::GetTypeFromCallbackMsg( pMsg );
    plSoundEvent        *event;

    if( type == plSoundEvent::kTime )
    {
        uint32_t byteTime = (fDSoundBuffer != nullptr) ? fDSoundBuffer->GetBufferBytePos(pMsg->fEventTime) : 0;

        event = IFindEvent( type, byteTime );

        if (event == nullptr)
        {
            // Add a new sound event for this guy
            event = new plSoundEvent( type, byteTime, this );
            //fDSoundBuffer->AddPosNotify( byteTime );
            fSoundEvents.emplace_back(event);
        }
    }
    else
    {
        event = IFindEvent( type );

        if (event == nullptr)
        {
            // Add a new sound event for this guy
            event = new plSoundEvent( type, this );
            fSoundEvents.emplace_back(event);
        }
    }

    event->AddCallback( pMsg );
}

void plWin32Sound::IRemoveCallback( plEventCallbackMsg *pMsg )
{
    plSoundEvent::Types type = plSoundEvent::GetTypeFromCallbackMsg( pMsg );

    for (auto iter = fSoundEvents.begin(); iter != fSoundEvents.end(); ++iter)
    {
        plSoundEvent* event = *iter;
        if (event->GetType() == type)
        {
            if (event->RemoveCallback(pMsg))
            {
                if (event->GetNumCallbacks() == 0)
                {
                    //if (event->GetType() == plSoundEvent::kTime)
                        //fDSoundBuffer->RemovePosNotify(event->GetTime());

                    delete event;
                    fSoundEvents.erase(iter);
                }
                break;
            }
        }
    }
}

plSoundEvent *plWin32Sound::IFindEvent( plSoundEvent::Types type, uint32_t bytePos )
{
    for (plSoundEvent* event : fSoundEvents)
    {
        if (event->GetType() == type)
        {
            if (type != plSoundEvent::kTime || bytePos == event->GetTime())
                return event;
        }
    }

    return nullptr;
}

void plWin32Sound::RemoveCallbacks(plSoundMsg* pSoundMsg)
{
    for (size_t i = 0; i < pSoundMsg->GetNumCallbacks(); ++i)
        IRemoveCallback( pSoundMsg->GetEventCallback( i ) );
}

void plWin32Sound::AddCallbacks(plSoundMsg* pSoundMsg)
{
    for (size_t i = 0; i < pSoundMsg->GetNumCallbacks(); ++i)
        IAddCallback( pSoundMsg->GetEventCallback( i ) );
}

bool plWin32Sound::MsgReceive( plMessage* pMsg )
{
    plEventCallbackMsg *e = plEventCallbackMsg::ConvertNoRef( pMsg );
    if (e != nullptr)
    {
        if( e->fEvent == kStop )
        {
            fPlaying = false;
            fPlayOnReactivate = false;  // Just to make sure...

            // Do we have an ending fade?
            if( fFadeOutParams.fLengthInSecs > 0 )
            {
                IStartFade( &fFadeOutParams );
                plSoundEvent    *event = IFindEvent( plSoundEvent::kStop );
                if (event != nullptr)
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
