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

#include "hsResMgr.h"
#include "hsTimer.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"
#include "plProfile.h"
#include "plgDispatch.h"

#include "plAudioSystem.h"
#include "plSound.h"
#include "plWin32Sound.h"

#include "plAudioCore/plSoundBuffer.h"
#include "plDrawable/plDrawableGenerator.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plAudioSysMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "plMessage/plListenerMsg.h"
#include "plIntersect/plSoftVolume.h"
#include "plStatusLog/plStatusLog.h"
#include "plPipeline/plPlates.h"
#include "pnKeyedObject/plKey.h"
#include "pnNetCommon/plSDLTypes.h"
#include "plAnimation/plScalarChannel.h"
#include "plAnimation/plAGModifier.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plAudioInterface.h"

plProfile_CreateCounterNoReset( "Loaded", "Sound", SoundNumLoaded );
plProfile_CreateCounterNoReset( "Waiting to Die", "Sound", WaitingToDie );
plProfile_CreateAsynchTimer( "Sound Load Time", "Sound", SoundLoadTime );

plGraphPlate    *plSound::fDebugPlate = nil;
plSound         *plSound::fCurrDebugPlateSound = nil;
bool            plSound::fLoadOnDemandFlag = true;
bool            plSound::fLoadFromDiskOnDemand = true;
unsigned        plSound::fIncidentalsPlaying = 0;

plSound::plSound() :
fPlaying(false),
fActive(false),
fTime(0),
fMaxFalloff(0),
fMinFalloff(0),
fCurrVolume(0.f),
fOuterVol(0),
fOuterCone(360),
fInnerCone(360),
fLength(0.0f),
fDesiredVol(0.f),
fFading(false),
fRegisteredForTime(false),
fMuted(true),
fFadedVolume(0.f),
fSoftRegion(nil),
fSoftOcclusionRegion(nil),
fSoftVolume(0.f),
fCurrFadeParams(nil),
fRegistered(false),
fDistAttenuation(0.f),
fProperties(0),
fNotHighEnoughPriority(false),
fVirtualStartTime(0),
fOwningSceneObject(nil),
fPriority(0),
fType(plSound::kSoundFX),
fQueued(false),
fLoading(false),
fSynchedStartTimeSec(0),
fMaxVolume(0),
fFreeData(false)
{
    plProfile_Inc( SoundNumLoaded );
    f3DPosition.Set( 0.f, 0.f, 0.f );
    f3DVelocity.Set( 0.f, 0.f, 0.f );
    fDataBuffer = nil;
    fDataBufferKey = nil;
    fPlayOnReactivate = false;
    fDataBufferLoaded = false;
}

plSound::~plSound()
{
    IStopFade( true );
    plProfile_Dec( SoundNumLoaded );
}

void plSound::IPrintDbgMessage( const char *msg, bool isError )
{
    ST::string keyName = GetKey() ? GetKeyName() : ST_LITERAL("unkeyed");
    if( isError )
        plStatusLog::AddLineSF("audio.log", plStatusLog::kRed, "ERROR: {} ({})", msg, keyName);
    else
        plStatusLog::AddLineSF("audio.log", "{} ({})", msg, keyName);
}

///////////////////////////////////////////////////////////
//  Called to send more values to the debug plate, assuming this is the right
//  sound. Should be called every time any of the values change, which means
//  the best place is inside ISetActualVolume(). Since that's a pure virtual,
//  it makes the placement of the call a bit annoying, but oh well.
void plSound::IUpdateDebugPlate()
{
    if( this == fCurrDebugPlateSound )
    {
        if( fDebugPlate == nil )
        {
            plPlateManager::Instance().CreateGraphPlate( &fDebugPlate );
            fDebugPlate->SetSize( 0.50, 0.25 );
            fDebugPlate->SetPosition( -0.5, 0 );
            fDebugPlate->SetDataRange( 0, 100, 100 );
            fDebugPlate->SetColors( 0x80202000 );
            fDebugPlate->SetLabelText( "Desired", "Curr", "Soft", "Dist" );
        }

        fDebugPlate->SetTitle(GetKeyName().c_str());      // Bleah
        fDebugPlate->SetVisible( true );
        fDebugPlate->AddData( (int32_t)( fDesiredVol * 100.f ), 
                              (int32_t)( fCurrVolume * 100.f ),
                              (int32_t)( fSoftVolume * 100.f ),
                              (int32_t)( fDistAttenuation * 100.f ) );
    }
}

void plSound::SetCurrDebugPlate( const plKey& soundKey )
{
    if( soundKey == nil )
    {
        fCurrDebugPlateSound = nil;
        if( fDebugPlate != nil )
            fDebugPlate->SetVisible( false );
    }
    else
    {
        fCurrDebugPlateSound = plSound::ConvertNoRef( soundKey->GetObjectPtr() );
        fCurrDebugPlateSound->IUpdateDebugPlate();
    }
}

/////////////////////////////////////////////////////////////////////
//  We don't keep track of the current time we should be at, but rather the
//  time we started at. Since we're calling SetTime(), we should adjust the
//  start time to be accurate for the time we want to be at now. Note: we
//  don't actually move the buffer position unless it's loaded, since we
//  don't want to force a load on a buffer just from a SetTime() call.
void plSound::SetTime( double t )
{
    fVirtualStartTime = hsTimer::GetSysSeconds() - t;

    if( IActuallyLoaded() )
        ISetActualTime( t );
}

// Support for Fast forward responder
void plSound::FastForwardPlay()
{
    if(fProperties & kPropLooping)
    {
        Play();
    }
}

void plSound::FastForwardToggle()
{
    if(fPlaying == true)
    {
        Stop();
        return;
    }
    FastForwardPlay();
}

////////////////////////////////////////////////////////////////////////
//  Our basic play function. Marks the sound as playing, and if we're actually
//  allowed to play, will actually start the sound playing as well.
void plSound::Play()
{   
    if(fLoading)    // if we are loading there is no reason to do this. Play will be called, by Update(), once the data is loaded and this floag is set to false
        return;

    if( !fActive )
    {
        // We're not active, so we can't play, but mark to make sure we'll play once we do get activated
        fPlayOnReactivate = true;
        return;
    }

    fPlaying = true;

    if(IPreLoadBuffer(true) == plSoundBuffer::kPending)
    {
        return;
    }

    fVirtualStartTime = hsTimer::GetSysSeconds();   // When we "started", even if we really don't start

    // if the sound system is not active do a fake play so callbacks get sent
    if(!plgAudioSys::Active())
    {
        // Do the (fake) actual play
        IActuallyPlay();
    }
    if( IWillBeAbleToPlay() )
    {
        IRefreshParams();

        if( fFadeInParams.fLengthInSecs > 0 )
        {
            IStartFade( &fFadeInParams);
        }
        else
        {
            // we're NOT fading!!!!
            if( fFading )
                IStopFade();
             SetVolume( fDesiredVol );
        }

        // Do the actual play
        IActuallyPlay();
    }
}

void plSound::SynchedPlay(unsigned bytes )
{
    if( fFading )
        IStopFade();
    if(fLoading)    // the sound is loading, it will be played when loading is finished
        return;

    if( !fActive )
    {
        // We're not active, so we can't play, but mark to make sure we'll play once we do get activated
        fPlayOnReactivate = true;
        return;
    }

    // Mark as playing, since we'll be calling ITryPlay() directly
    fPlaying = true;
    fPlayOnReactivate = false;

    if( IWillBeAbleToPlay() )
    {
        SetStartPos(bytes);
        Play();
    }
}

/////////////////////////////////////////////////////////////////
//  Used for synching play state. The state only knows that we're playing
//  and what time we started at, so we use that to compute what time we should
//  be at and update. Note that we also set our virtual start time to what
//  we're given, NOT the current time, 'cause, well, duh, that should be our
//  start time!
//  So think of it as "Play() but act as if you started at *this* time"...
void plSound::SynchedPlay( float virtualStartTime )
{
    if( fFading )
        IStopFade();

    ISynchedPlay( virtualStartTime );
}

////////////////////////////////////////////////////////////////
//  Only want to do the fade hack when somebody outside synch()s us.
void plSound::ISynchedPlay( double virtualStartTime )
{   
    if(fLoading)    // the sound is loading, it will be played when loading is finished
        return;

    // Store our start time
    fVirtualStartTime = virtualStartTime;

    if( !fActive )
    {
        // We're not active, so we can't play, but mark to make sure we'll play once we do get activated
        fPlayOnReactivate = true;
        return;
    }

    // Mark as playing, since we'll be calling ITryPlay() directly
    fPlaying = true;
    fPlayOnReactivate = false;

    // Do da synch, which will start us playing
    if( IWillBeAbleToPlay() )
    {
        ISynchToStartTime();
    }
}

///////////////////////////////////////////////////////////
//  Takes the virtual start time and sets us to the real time we should be at,
//  then starts us playing via ITryPlay().
void plSound::ISynchToStartTime()
{
    if( !plgAudioSys::Active() )
        return;

    // We don't want to do this until we're actually loaded, since that's when we'll know our
    // REAL length (thanks to the inaccuracies of WMA compression)
//  LoadSound( IsPropertySet( kPropIs3DSound ) );
    // Haha, the GetLength() call will do this for us

    // Calculate what time we should be at
    double deltaTime = hsTimer::GetSysSeconds() - fVirtualStartTime;
    double length = GetLength();

    if( deltaTime > length || deltaTime < 0 )
    {
        // Hmm, our time went past the length of sound, so handle that
        if( IsPropertySet( kPropLooping ) ) 
        {
            if( length <= 0 )
                deltaTime = 0;      // Error, attempt to recover
            else if( deltaTime < 0 )
            {
                int numWholeParts = (int)( -deltaTime / length );
                deltaTime += length * ( numWholeParts + 1 );
            }
            else
            {
                int numWholeParts = (int)( deltaTime / length );
                deltaTime -= length * (double)numWholeParts;
            }

            //ISetActualTime( deltaTime );
            Play();
        }
        else
            // We already played and stopped virtually, so really mark us as stopped
            Stop();
    }
    else
    {
        // Easy 'nuf...
        //ISetActualTime( deltaTime );
        Play();
    }
} 

void plSound::SetPosition(const hsPoint3 pos)
{
    f3DPosition = pos;
}

void plSound::SetVelocity(const hsVector3 vel)
{
    f3DVelocity = vel;
}

hsPoint3 plSound::GetPosition() const
{
    return f3DPosition;
}

hsVector3 plSound::GetVelocity() const
{
    return f3DVelocity;
}

void plSound::SetMin(const int m)
{
    fMinFalloff = m;
}

void plSound::SetMax(const int m)
{
    fMaxFalloff = m;
}

void plSound::SetOuterVolume(const int v)
{
    fOuterVol = v;
}

void plSound::SetConeOrientation( float x, float y, float z )
{
    fConeOrientation.Set( x, y, z );
}

void plSound::SetConeAngles( int inner, int outer )
{
    fOuterCone = outer;
    fInnerCone = inner;
}

int plSound::GetMin() const
{
    return fMinFalloff;
}

int plSound::GetMax() const
{
    return fMaxFalloff;
}

void plSound::SetVolume(const float v)
{
    fDesiredVol = v;

    if( !fMuted && !fFading )
        fCurrVolume = fDesiredVol;

    RefreshVolume();
}

void plSound::RefreshVolume()
{
    this->ISetActualVolume( fCurrVolume );
}

void plSound::SetMuted( bool muted )
{
    if( muted != fMuted )
    {
        fMuted = muted;
        if( fMuted )
            fCurrVolume = 0.f;
        else if( !fFading )
            fCurrVolume = fDesiredVol;

        RefreshVolume();
    }
}

void plSound::IRefreshParams()
{
    SetMax( fMaxFalloff );
    SetMin( fMinFalloff );
    SetOuterVolume( fOuterVol );
    SetConeAngles( fInnerCone, fOuterCone );
    SetConeOrientation( fConeOrientation.fX, fConeOrientation.fY, fConeOrientation.fZ);
    SetPosition( f3DPosition );
    SetVelocity( f3DVelocity );
}

////////////////////////////////////////////////////////////////////////
//  The public interface to stopping, which also synchs the state with the
//  server.
void plSound::Stop()
{
    fPlaying = false;

    // if the audio data is loading while stop is called we need to make sure the sounds doesn't play, and the data is unloaded.
    fPlayOnReactivate = false;  
    fFreeData = true;
    
    // Do we have an ending fade?
    if( fFadeOutParams.fLengthInSecs > 0 && !plgAudioSys::IsRestarting() )
    {
        IStartFade( &fFadeOutParams );
    }
    else
    {
        if( fFading )
            IStopFade();

        fCurrVolume = 0.f;
        ISetActualVolume( fCurrVolume );
        IActuallyStop();
    }
    if(fPlayWhenLoaded)
    {
        fPlayWhenLoaded = false;
    }
}

void plSound::IActuallyStop()
{
    if( fLoadOnDemandFlag && !IsPropertySet( kPropDisableLOD ) && !IsPropertySet( kPropLoadOnlyOnCall ) )
    {
        // If we're loading on demand, we want to unload on stop
        IFreeBuffers();
    }
}

void plSound::Update() 
{
    if(fLoading)
    {       
        plSoundBuffer::ELoadReturnVal retVal = IPreLoadBuffer(fPlayWhenLoaded);
        
        if(retVal == plSoundBuffer::kError)
        {
            fLoading = false;
            fPlayWhenLoaded = false;
        }
        if(retVal == plSoundBuffer::kSuccess)
        {
            fLoading = false;
            if(fPlayWhenLoaded)
                Play();
            fPlayWhenLoaded = false;

            // ensure the sound data is released if the sound object was stopped while the audio data was being loaded.
            if(fFreeData)
            {
                fFreeData = false;
                FreeSoundData();
            }
        }
    }
}

float plSound::IGetChannelVolume() const
{
    float channelVol = plgAudioSys::GetChannelVolume( (plgAudioSys::ASChannel)fType );
    if (IsPropertySet(kPropDontFade))
        return channelVol;
    else
        return channelVol * plgAudioSys::GetGlobalFadeVolume();
}

void plSound::IStartFade( plFadeParams *params, float offsetIntoFade )
{
    fFading = true;

    if( params == &fFadeOutParams )
    {
        fFadeOutParams.fVolStart = fCurrVolume;
        fFadeOutParams.fVolEnd = fFadedVolume;
        fCurrFadeParams = &fFadeOutParams;
    }
    else if( params == &fFadeInParams )
    {
        fFadeInParams.fVolStart = fCurrVolume;  // Hopefully, we got to fFadedVolume, but maybe not
        fFadeInParams.fVolEnd = fDesiredVol;
        fCurrFadeParams = &fFadeInParams;
        plStatusLog::AddLineSF("audio.log", "Fading in {}", GetKeyName());
    }
    else
        fCurrFadeParams = params;

    fCurrFadeParams->fCurrTime = offsetIntoFade;
    ISetActualVolume( fCurrFadeParams->InterpValue() );

    if( !fRegisteredForTime )
    {
        plgDispatch::Dispatch()->RegisterForExactType( plTimeMsg::Index(), GetKey() );
        fRegisteredForTime = true;
    }
}

void plSound::IStopFade( bool shuttingDown, bool SetVolEnd)
{
    if( fCurrFadeParams != nil )
    {
        if( fCurrFadeParams == &fCoolSoftVolumeTrickParams )
        {
            plProfile_Dec( WaitingToDie );
        }

        // This can cause problems if we've exited a soft region and are doing a soft volume fade.
        // If the camera pops back into the region of this particular sound this will cause the soft volume to be zero, 
        // therefore not allowing the sound to play until the listener moves again(triggering another softsound update).
        // So if this function is called from UpdateSoftSounds this will not be performed
        if(SetVolEnd)
        {
            if( fCurrFadeParams->fFadeSoftVol )
                fSoftVolume = fCurrFadeParams->fVolEnd;
            else
                fCurrVolume = fCurrFadeParams->fVolEnd;
        }

        if( !shuttingDown )
            ISetActualVolume( fCurrVolume );

        fCurrFadeParams->fCurrTime = -1.f;
    }

    fFading = false;
    fCurrFadeParams = nil;

    // Fade done, unregister for time message
    if( fRegisteredForTime )
    {
        plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );
        fRegisteredForTime = false;
    }
}

bool plSound::MsgReceive( plMessage* pMsg )
{
    plTimeMsg   *time = plTimeMsg::ConvertNoRef( pMsg );
    if( time != nil )
    {
        /// Time message for handling fade ins/outs
        if( fCurrFadeParams == nil )
            return true;

        fCurrFadeParams->fCurrTime += time->DelSeconds();

        if( fCurrFadeParams->fCurrTime >= fCurrFadeParams->fLengthInSecs )
        {
            if( fCurrFadeParams->fFadeSoftVol )
                fSoftVolume = fCurrFadeParams->fVolEnd;
            else
                fCurrVolume = fCurrFadeParams->fVolEnd;
            ISetActualVolume( fCurrVolume );
            fCurrFadeParams->fCurrTime = -1.f;
                        
            // Fade done, unregister for time message
            if( fRegisteredForTime )
            {
                plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );
                fRegisteredForTime = false;
            }

            // Note: if we're done, and we were fading out, we need to STOP
            if( fCurrFadeParams->fStopWhenDone )
            {
                // REALLY STOP
                IActuallyStop();
            }

            if( fCurrFadeParams == &fCoolSoftVolumeTrickParams )
            {
                plProfile_Dec( WaitingToDie );
            }

            // Done with this one!
            fCurrFadeParams = nil;
            fFading = false;
        }   
        else
        {
            // Gotta interp
            if( fCurrFadeParams->fFadeSoftVol )
                fSoftVolume = fCurrFadeParams->InterpValue();
            else
                fCurrVolume = fCurrFadeParams->InterpValue();
                        
            ISetActualVolume( fCurrVolume );
        }

        return true;
    }

    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef( pMsg );
    if( refMsg )
    {
        if( refMsg->fType == kRefSoftVolume )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                ISetSoftRegion( plSoftVolume::ConvertNoRef(refMsg->GetRef()) );
                return true;
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
            {
                ISetSoftRegion( nil );
                return true;
            }
        }
        else if( refMsg->fType == kRefSoftOcclusionRegion )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                ISetSoftOcclusionRegion( plSoftVolume::ConvertNoRef( refMsg->GetRef() ) );
                return true;
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
            {
                ISetSoftOcclusionRegion( nil );
                return true;
            }
        }
        else if( refMsg->fType == kRefDataBuffer )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fDataBuffer = plSoundBuffer::ConvertNoRef( refMsg->GetRef() );
                SetLength( fDataBuffer->GetDataLengthInSecs() );
            }
            else
                fDataBuffer = nil;

            return true;
        }
        else if( refMsg->fType == kRefParentSceneObject )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fOwningSceneObject = plSceneObject::ConvertNoRef( refMsg->GetRef() );
            else
                fOwningSceneObject = nil;

            return true;
        }
    }

    plSoundMsg *pSoundMsg = plSoundMsg::ConvertNoRef( pMsg );
    if( pSoundMsg != nil )
    {   
        if( pSoundMsg->Cmd( plSoundMsg::kAddCallbacks ) )
        {
            AddCallbacks( pSoundMsg );
            return true;
        }
        else if( pSoundMsg->Cmd( plSoundMsg::kRemoveCallbacks ) )
        {
            RemoveCallbacks( pSoundMsg );
            return true;
        }
        return false;
    }

    plListenerMsg *listenMsg = plListenerMsg::ConvertNoRef( pMsg );
    if( listenMsg != nil )
    {
        if( fSoftOcclusionRegion != nil )
        {
            // The EAX settings have 0 as the start value and 1 as the end, and since start
            // translates to "inside the soft region", it's reversed of what the region gives us
            fEAXSettings.SetOcclusionSoftValue( 1.f - fSoftOcclusionRegion->GetListenerStrength() );
            IRefreshEAXSettings();
        }
        return true;
    }

    return plSynchedObject::MsgReceive( pMsg );
}

void plSound::ForceLoad()
{
    if( !IsPropertySet( kPropLoadOnlyOnCall ) )
        return;

    LoadSound( IsPropertySet( kPropIs3DSound ) );
}

void plSound::ForceUnload()
{
    if( !IsPropertySet( kPropLoadOnlyOnCall ) )
        return;

    Stop();
    IFreeBuffers();
}

bool plSound::ILoadDataBuffer()
{
    if(!fDataBufferLoaded)
    {
        plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->RefObject();
        if(!buffer)
        {
            hsAssert(false, "unable to load sound buffer");
            plStatusLog::AddLineSF("audio.log", "Unable to load sound buffer: {}", GetKeyName());
            return false;
        }
        SetLength( buffer->GetDataLengthInSecs() );
        fDataBufferLoaded = true;
    }
    return true;
}

void plSound::FreeSoundData() 
{
    if(!fDataBufferKey) return; // for plugins
    plSoundBuffer *buffer = (plSoundBuffer *) fDataBufferKey->ObjectIsLoaded();
    if(buffer)
    {
        buffer->UnLoad();
    }
}

void plSound::IUnloadDataBuffer()
{
    if(fDataBufferLoaded)
    {
        fDataBufferLoaded = false;
        fDataBufferKey->UnRefObject();
    }
}

/////////////////////////////////////////////////////////////////////////
// calling preload will cause the sound to play once loaded
plSoundBuffer::ELoadReturnVal plSound::IPreLoadBuffer( bool playWhenLoaded, bool isIncidental /* = false */ )
{
    if(!ILoadDataBuffer())
    {
        return plSoundBuffer::kError;
    }

    plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();

    if(buffer && buffer->IsValid() )
    {
        plProfile_BeginTiming( SoundLoadTime );
        plSoundBuffer::ELoadReturnVal retVal = buffer->AsyncLoad(buffer->HasFlag(plSoundBuffer::kStreamCompressed) ? plAudioFileReader::kStreamNative : plAudioFileReader::kStreamWAV);
        if(retVal == plSoundBuffer::kPending)
        {
            fPlayWhenLoaded = playWhenLoaded;
            fLoading = true;
        }
        
        plProfile_EndTiming( SoundLoadTime );
    
        return retVal;
    }
    else
    {
        return plSoundBuffer::kError;
    }
}

plFileName plSound::GetFileName() const
{
    if (fDataBufferKey->ObjectIsLoaded())
    {
        return ((plSoundBuffer *)fDataBufferKey->ObjectIsLoaded())->GetFileName();
    }

    return "";
}

/////////////////////////////////////////////////////////////////////////
//  WARNING: to do this right, we have to load the data buffer in, which could
//  force an early load of the sound if you're just calling this on a whim. So
//  it's best to call this right before you're going to load it anyway (or
//  when it's already loaded).
//  Note that if we already set the length (like at export time), we never need
//  to load the sound, so the optimization at export time is all ready to plug-
//  and-play...
double plSound::GetLength()
{
    if( ( (double)fLength == 0.f ) )
        ILoadDataBuffer();

    return fLength;
}

void plSound::ISetSoftRegion( plSoftVolume *region )
{
    /// We're either registering or unregistering
    if( fSoftRegion == nil && region != nil )
        RegisterOnAudioSys();
    else if( fSoftRegion != nil && region == nil )
        UnregisterOnAudioSys();

    fSoftRegion = region;
    fSoftVolume = 0.f;      // Set to zero until we can get our processing call
}

void plSound::ISetSoftOcclusionRegion( plSoftVolume *region )
{
    /// We're either registering or unregistering for listener messages
    if( fSoftOcclusionRegion == nil && region != nil )
    {
        plgDispatch::Dispatch()->RegisterForExactType( plListenerMsg::Index(), GetKey() );
    }
    else if( fSoftOcclusionRegion != nil && region == nil )
    {
        plgDispatch::Dispatch()->UnRegisterForExactType( plListenerMsg::Index(), GetKey() );
    }

    fSoftOcclusionRegion = region;
}

/////////////////////////////////////////////////////////////////////////
//  This function calculates our new softVolume value. Used both to update the
//  said value and so the audio system can rank us in importance.
float plSound::CalcSoftVolume( bool enable, float distToListenerSquared )
{
    // Do distance-based attenuation ourselves
#if MCN_HACK_OUR_ATTEN
    if( IsPropertySet( kPropIs3DSound ) )
    {
        float    minDist = (float)GetMin();
        if( distToListenerSquared <= minDist * minDist )
        {
            fDistAttenuation = 1.f;
        }
        else
        {
            float    maxDist = (float)GetMax();
            if( distToListenerSquared >= maxDist * maxDist )
            {
                fDistAttenuation = 0.f;
            }
            else
            {
                float d = (float)sqrt( distToListenerSquared );
                fDistAttenuation = minDist / d;

                // The following line ramps it to 0 at the maxDistance. Kinda klunky, but good for now I guess...
//              fDistAttenuation *= 1.f - ( 1.f / ( maxDist - minDist ) ) * ( d - minDist );
            }
        }
    }
    else
        fDistAttenuation = 1.f;
#endif
    // At the last 50% of our distance attenuation (squared, so it really is farther than that), 
    // ramp down to 0 so we don't get annoying popping when we stop stuff
    if( IsPropertySet( kPropIs3DSound ) )
    {
        float    maxDistSquared = (float)( GetMax() * GetMax() );
        float    distToStartSquared = (float)(maxDistSquared * 0.50);

        if( maxDistSquared < 0.f )  // Happens when the max distance is REALLY big
        {
            maxDistSquared = distToListenerSquared + 1.f;   // :)
            distToStartSquared = maxDistSquared;
        }

        if( distToListenerSquared > maxDistSquared )
            fDistAttenuation = 0.f;
        else if( distToListenerSquared > distToStartSquared )
            fDistAttenuation = ( maxDistSquared - distToListenerSquared ) / ( maxDistSquared - distToStartSquared );
        else
            fDistAttenuation = 1.f;

        fDistToListenerSquared = distToListenerSquared;
    }
    else
    {
        fDistAttenuation = 1.f;
        fDistToListenerSquared = 0.f;
    }

    // Attenuate based on our soft region, if we have one
    if( !enable )
        // We apparently don't know jack. Let the audioSystem's decision rule
        fSoftVolume = 0.f;
    else if( fSoftRegion != nil )
        fSoftVolume = fSoftRegion->GetListenerStrength();
    else
        fSoftVolume = 1.f;

    return fSoftVolume;
}

/////////////////////////////////////////////////////////////////////////
//  Wee function for the audio system. This basically returns the effective
//  current volume of this sound. Useful for doing things like ranking all
//  sounds based on volume.
float plSound::GetVolumeRank()
{
    if( !IsPlaying() && !this->IActuallyPlaying() )
        return 0.f;

    float    rank = fSoftVolume * fDesiredVol;

    if( IsPropertySet( kPropIs3DSound ) )
    {
        float minDistSquared = (float)( GetMin() * GetMin() );
        float maxDistSquared = (float) (GetMax() * GetMax());
        hsPoint3 listenerPos = plgAudioSys::GetCurrListenerPos();
        if( fDistToListenerSquared > minDistSquared )
        {
            float diff = maxDistSquared - minDistSquared;
            rank *= fabs((fDistToListenerSquared - maxDistSquared)) / diff;  
        }
    }

    return rank;
}

/////////////////////////////////////////////////////////////////////////
//  Tests to see whether, if we try to play this sound now, it'll actually
//  be able to play. Takes into account whether the sound is within range
//  of the listener and the current soft region value.
bool plSound::IWillBeAbleToPlay()
{
    if( fSoftVolume == 0.f )
        return false;

    return IsWithinRange( plgAudioSys::GetCurrListenerPos(), nil );
}

/////////////////////////////////////////////////////////////////////////
//  Tests to see whether this sound is within range of the position given,
//  ignoring soft volumes.
bool plSound::IsWithinRange( const hsPoint3 &listenerPos, float *distSquared )
{
    if( !IsPropertySet( plSound::kPropIs3DSound ) )
    {
        if( distSquared != nil )
            *distSquared = 1.f;
        return true;
    }

    hsVector3   distance;
    hsPoint3    soundPos = GetPosition();

    distance.Set( &listenerPos, &soundPos );

    if( distSquared != nil )
        *distSquared = distance.MagnitudeSquared();

    if( GetMax() == 1000000000 )
        return true;

    float    soundRadius = (float)( GetMax() * GetMax() );

    return ( distance.MagnitudeSquared() <= soundRadius ) ? true : false;
}


//// ////////////////////////////////////////////////////////
//  Once the soft volume is calculated and our rank is computed, we can 
//  decide whether to actually enable or not.
//  Note: we might have been "enabled" by our Calc call, but the ranking
//  still could have disabled us, so we have to specify again whether
//  we're enabled.
//  Note: if we KNOW we're disabling this sound (out of range), Calc() doesn't
//  have to be called at all, and we can simply call this function with
//  enable = false.
void plSound::UpdateSoftVolume( bool enable, bool firstTime )
{
    fNotHighEnoughPriority = !enable;

    // Don't do any of this special stuff that follows if we're not supposed to be playing
    if( IsPlaying() )
    {
        if( fSoftVolume * fDistAttenuation > 0.f && !fNotHighEnoughPriority )
        {
            if( fCurrFadeParams == &fCoolSoftVolumeTrickParams )
            {
                // Stop the fade, but since we are updating the softvolume with the intention of being audible
                // tell StopFade not to set the soft volume to zero.
                IStopFade(false, false);
            }
            if( !IActuallyPlaying() )
            {
                // Must've been stopped from being out of range. Start up again...

                // Synch up to our start time.
                // If this sound is auto starting and is background music, get the current time so we don't start 
                // with the play cursor already into the piece.
                if(IsPropertySet(kPropAutoStart) && fType == kBackgroundMusic) fVirtualStartTime = hsTimer::GetSysSeconds();
                ISynchedPlay( fVirtualStartTime );
            }
        }
        else if( fCurrFadeParams != &fCoolSoftVolumeTrickParams && IActuallyPlaying() )
        {
            // Start our special trick, courtesy of Brice. Basically, we don't 
            // stop the sound immediately, but rather let it get to the end of
            // the sound and loop once more. This way, if we go away and come back soon
            // enough, it will be continuing as we expect it to, but if we wait long enough,
            // it'll stop, saving processing time.

            // Note: we just do it as a fade because it makes it easier on us that way!
            fCoolSoftVolumeTrickParams.fCurrTime = 0.f;
            fCoolSoftVolumeTrickParams.fLengthInSecs = firstTime ? 0.f : (float)fLength + ( (float)fLength - (float)GetTime() );
            fCoolSoftVolumeTrickParams.fStopWhenDone = true;
            fCoolSoftVolumeTrickParams.fFadeSoftVol = true;
            fCoolSoftVolumeTrickParams.fType = plFadeParams::kLinear;
            fCoolSoftVolumeTrickParams.fVolStart = fSoftVolume; // Don't actually change the volume this way
            fCoolSoftVolumeTrickParams.fVolEnd = 0.f;

            IStartFade( &fCoolSoftVolumeTrickParams );

            plProfile_Inc( WaitingToDie );
        }
    }

    RefreshVolume();
}

/////////////////////////////////////////////////////////////////////////
// Returns the current volume, attenuated
float plSound::QueryCurrVolume() const
{
    return IAttenuateActualVolume( fCurrVolume ) * IGetChannelVolume();
}

/////////////////////////////////////////////////////////////////////////
//  Used by ISetActualVolume(). Does the final attenuation on a volume before
//  sending it to the sound processing. Only does soft regions for now.
float plSound::IAttenuateActualVolume( float volume ) const
{
    if( fNotHighEnoughPriority )
        return 0.f;
    
    volume *= fSoftVolume;

    if( IsPropertySet( kPropIs3DSound ) )
        volume *= fDistAttenuation;

    return volume;
}

void plSound::Activate(bool forcePlay)
{
    // Our actual state...
    fActive = true;

    // re-create the sound state here:
    if( forcePlay || fPlayOnReactivate )
    {
        ISynchedPlay( hsTimer::GetSysSeconds() );
        fPlayOnReactivate = false;
    }

    RegisterOnAudioSys();
    SetMuted(plgAudioSys::IsMuted());
}

void plSound::DeActivate()
{
    UnregisterOnAudioSys();

    if( fActive )
    {
        if( IsPlaying() )
        {
            Stop();
            fPlayOnReactivate = true;
        }
        else
            fPlayOnReactivate = false;
    }

    fActive = false;
}

/////////////////////////////////////////////////////////////////////////
//  Tell the audio system about ourselves.
void plSound::RegisterOnAudioSys()
{
    if( !fRegistered )
    {
        plgAudioSys::RegisterSoftSound(GetKey());
        fRegistered = true;
    }
}

/////////////////////////////////////////////////////////////////////////
//  Tell the audio system to stop caring about us
void plSound::UnregisterOnAudioSys()
{
    if( fRegistered )
    {
        plgAudioSys::UnregisterSoftSound(GetKey());
        fRegistered = false;
    }
}

/////////////////////////////////////////////////////////////////////////
//  Called by the audio system when we've been booted off (audio system is
//  shutting down). Normally, we should already be shut down, but in case
//  we're not, this function makes sure everything is cleaned up before
//  the audio system itself shuts down.
void plSound::ForceUnregisterFromAudioSys()
{
    DeActivate();
    fRegistered = false;
}

void plSound::Read(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Read(s, mgr);

    IRead( s, mgr );

    // If we're autostart, start playing
    if( IsPropertySet( kPropAutoStart ) )
        Play();

    // Make sure we synch or don't synch
    if( IsPropertySet( kPropLocalOnly ) )
        SetLocalOnly(true);

    // If we're not playing, but we're going to, and we're going to fade in,
    // then our current state is faded out, so set fFading
    if( fFadeInParams.fLengthInSecs > 0 && !fPlaying )
        fFading = true;
    else if( fFadeInParams.fLengthInSecs <= 0 && !fPlaying )
        fFading = false;

    ILoadDataBuffer();      // make sure our sound buffer is loaded

    // Force load on read
    if( !fLoadOnDemandFlag || IsPropertySet( kPropDisableLOD ) )
    {
        LoadSound( IsPropertySet( kPropIs3DSound ) );
    }
    else
    {
        // Loading on demand, but we still need the length. But that's ok, we'll get it when we get the fDataBuffer ref.
        // But we want to preload the data, so go ahead and do that
        if( !fLoadFromDiskOnDemand && !IsPropertySet( kPropLoadOnlyOnCall ) && fPriority <= plgAudioSys::GetPriorityCutoff())
        {           
            IPreLoadBuffer(false);          
        }
    }
}

void plSound::Write(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Write(s, mgr);
    IWrite( s, mgr );
}

void plSound::IRead( hsStream *s, hsResMgr *mgr )
{
    fPlaying = s->ReadBool();
    fVirtualStartTime = hsTimer::GetSysSeconds();   // Need if we're autostart
    fTime = s->ReadLEDouble();
    fMaxFalloff = s->ReadLE32();
    fMinFalloff = s->ReadLE32();
    s->ReadLE( &fCurrVolume );
    s->ReadLE( &fDesiredVol );

    /// mcn debugging - Thanks to some of my older sound code, it's possible that a few volumes
    /// will come in too large. This will only happen with scenes that were exported with that intermediate
    /// code, which should be limited to my test scenes locally. Otherwise, things should be fine. This
    /// is to compensate for those bogus files. (The fix is to reset the volume in MAX, since the bogusness
    /// is in the range of the volume slider itself).
    if( fDesiredVol > 1.f )
        fDesiredVol = 1.f;
    if( fCurrVolume > 1.f )
        fCurrVolume = 1.f;
    fMaxVolume = fDesiredVol;

    fOuterVol = s->ReadLE32();
    fInnerCone = s->ReadLE32();
    fOuterCone = s->ReadLE32();
    s->ReadLE( &fFadedVolume );
    s->ReadLE( &fProperties );

    fType = s->ReadByte();
    fPriority = s->ReadByte();

    // HACK FOR OLDER EXPORTERS that thought Auto-start meant set fPlaying true
    if( fPlaying )
        SetProperty( kPropAutoStart, true );

    // Read in fade params
    fFadeInParams.Read( s );
    fFadeOutParams.Read( s );

    // Read in soft volume key
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, kRefSoftVolume ), plRefFlags::kActiveRef );

    // Read in the data buffer key
    fDataBufferKey = mgr->ReadKey( s );

    // EAX params
    fEAXSettings.Read( s );

    // EAX soft keys
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, kRefSoftOcclusionRegion ), plRefFlags::kActiveRef );
}

void plSound::IWrite( hsStream *s, hsResMgr *mgr )
{
    s->WriteBool(fPlaying);
    s->WriteLEDouble(fTime);
    s->WriteLE32(fMaxFalloff);
    s->WriteLE32(fMinFalloff);
    s->WriteLE( fCurrVolume );
    s->WriteLE( fDesiredVol );
    s->WriteLE32(fOuterVol);
    s->WriteLE32(fInnerCone);
    s->WriteLE32(fOuterCone);
    s->WriteLE( fFadedVolume );
    s->WriteLE( fProperties );
    s->WriteByte( fType );
    s->WriteByte( fPriority );
    
    // Write out fade params
    fFadeInParams.Write( s );
    fFadeOutParams.Write( s );

    // Write out soft volume key
    mgr->WriteKey( s, fSoftRegion );
    
    // Write out data buffer key
    if(fDataBuffer)
        mgr->WriteKey( s, fDataBuffer->GetKey() );
    else
        mgr->WriteKey( s, fDataBufferKey );

    // EAX params
    fEAXSettings.Write( s );

    // EAX Soft keys
    mgr->WriteKey( s, fSoftOcclusionRegion );
}

void plSound::plFadeParams::Read( hsStream *s )
{
    s->ReadLE( &fLengthInSecs );
    s->ReadLE( &fVolStart );
    s->ReadLE( &fVolEnd );
    s->ReadLE( &fType );
    s->ReadLE( &fCurrTime );
    fStopWhenDone = s->ReadBOOL();
    fFadeSoftVol = s->ReadBOOL();
}

void plSound::plFadeParams::Write( hsStream *s )
{
    s->WriteLE( fLengthInSecs );
    s->WriteLE( fVolStart );
    s->WriteLE( fVolEnd );
    s->WriteLE( fType );
    s->WriteLE( fCurrTime );
    s->WriteBOOL( fStopWhenDone );
    s->WriteBOOL( fFadeSoftVol );
}

float plSound::plFadeParams::InterpValue()
{
    float    val;

    switch( fType )
    {
        case kLinear:
            val = ( ( fCurrTime / fLengthInSecs ) * ( fVolEnd - fVolStart ) ) + fVolStart;
            break;
        case kLogarithmic:
            val = fCurrTime / fLengthInSecs;
            val = ( ( val * val ) * ( fVolEnd - fVolStart ) ) + fVolStart;
            break;
        case kExponential:
            val = fCurrTime / fLengthInSecs;
            val = ( (float)sqrt( val ) * ( fVolEnd - fVolStart ) ) + fVolStart;
            break;
        default:
            val = 0.f;
    }
    return val;
}

void plSound::SetFadeInEffect( plSound::plFadeParams::Type type, float length )
{
    fFadeInParams.fLengthInSecs = length;
    fFadeInParams.fType = type;
    fFadeInParams.fVolStart = 0.f;  // Will be set when activated
    fFadeInParams.fVolEnd = 1.f;    // Will be set when activated
    fFadeInParams.fCurrTime = -1.f;

    // If we're not playing, but we're going to, and we're going to fade in,
    // then our current state is faded out, so set fFading
    if( fFadeInParams.fLengthInSecs > 0 && !fPlaying )
        fFading = true;
    else if( fFadeInParams.fLengthInSecs <= 0 && !fPlaying )
        fFading = false;
}

void plSound::SetFadeOutEffect( plSound::plFadeParams::Type type, float length )
{
    fFadeOutParams.fLengthInSecs = length;
    fFadeOutParams.fType = type;
    fFadeOutParams.fVolStart = 1.f; // Will be set when activated
    fFadeOutParams.fVolEnd = 0.f;   // Will be set when activated
    fFadeOutParams.fCurrTime = -1.f;
    fFadeOutParams.fStopWhenDone = true;
}

plDrawableSpans* plSound::CreateProxy(const hsMatrix44& l2w, hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo)
{
    plDrawableSpans* myDraw = addTo;

    if( fOuterCone < 360 )
    {
        float len = (float)GetMax();
        float halfAng = hsDegreesToRadians(float(fInnerCone) * 0.5f);
        float radius = len * tanf(halfAng);
        if( fInnerCone < 180 )
            len = -len;
        myDraw = plDrawableGenerator::GenerateConicalDrawable(
            radius, 
            len, 
            mat, 
            l2w, 
            true,
            &hsColorRGBA().Set(1.f, 0.5f, 0.5f, 1.f),
            &idx,
            myDraw);

        len = (float)GetMin();
        halfAng = hsDegreesToRadians(float(fOuterCone) * 0.5f);
        radius = len * tanf(halfAng);
        if( fOuterCone < 180 )
            len = -len;

        myDraw = plDrawableGenerator::GenerateConicalDrawable(
            radius, 
            len, 
            mat, 
            l2w, 
            true,
            &hsColorRGBA().Set(0.25f, 0.25f, 0.5f, 1.f),
            &idx,
            myDraw);
    }
    else
    {
        myDraw = plDrawableGenerator::GenerateSphericalDrawable(
            hsPoint3(0,0,0), 
            (float)GetMin(), 
            mat, 
            l2w, 
            true,
            &hsColorRGBA().Set(1.f, 0.5f, 0.5f, 1.f),
            &idx,
            myDraw);

        myDraw = plDrawableGenerator::GenerateSphericalDrawable(
            hsPoint3(0,0,0), 
            (float)GetMax(), 
            mat, 
            l2w, 
            true,
            &hsColorRGBA().Set(0.25f, 0.25f, 0.5f, 1.f),
            &idx,
            myDraw);
    }
    return myDraw;
}


// call when state has changed
bool plSound::DirtySynchState(const ST::string& sdlName /* kSDLSound */, uint32_t sendFlags)
{
    /*
    if( sdlName.empty() )
        sdlName = kSDLSound;

    if( fOwningSceneObject != nil )
        return fOwningSceneObject->DirtySynchState(sdlName, sendFlags);
*/
    return false;
}


//////////////////////////////////////////////////////////////////////////////
//// plSoundVolumeApplicator /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void plSoundVolumeApplicator::IApply( const plAGModifier *mod, double time )
{
    plScalarChannel *chan = plScalarChannel::ConvertNoRef( fChannel );
    if(chan)
    {
        float volume = chan->Value( time );

        float digitalVolume = (float)pow( 10.f, volume / 20.f );

        // Find the audio interface and thus the plSound from it
        plSceneObject *so = mod->GetTarget( 0 );
        if( so != nil )
        {
            const plAudioInterface *ai = so->GetAudioInterface();
            if( ai != nil && fIndex < ai->GetNumSounds() )
            {
                plSound *sound = ai->GetSound( fIndex );
                if( sound != nil )
                {
                    sound->SetVolume( digitalVolume );
                    return;
                }
            }
        }
    }
}

plAGApplicator *plSoundVolumeApplicator::CloneWithChannel( plAGChannel *channel )
{
    plSoundVolumeApplicator *clone = (plSoundVolumeApplicator *)plAGApplicator::CloneWithChannel( channel );
    clone->fIndex = fIndex;
    return clone;
}

void plSoundVolumeApplicator::Write( hsStream *stream, hsResMgr *mgr )
{
    plAGApplicator::Write( stream, mgr );
    stream->WriteLE32( fIndex );
}

void plSoundVolumeApplicator::Read( hsStream *s, hsResMgr *mgr )
{
    plAGApplicator::Read( s, mgr );
    fIndex = s->ReadLE32();
}
