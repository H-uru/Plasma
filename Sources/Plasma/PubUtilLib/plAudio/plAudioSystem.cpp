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
#include <al.h>
#include <efx.h>
#ifdef EAX_SDK_AVAILABLE
#include <eax.h>
#endif
#include <memory>

#include "hsTimer.h"
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "plStatusLog/plStatusLog.h"

#include "plSound.h"
#include "plAudioSystem.h"
#include "plDSoundBuffer.h"
#include "plEAXEffects.h"
#include "plEAXListenerMod.h"
#include "plVoiceChat.h"

#include "pnMessage/plAudioSysMsg.h"
#include "plMessage/plRenderMsg.h"
#include "pnMessage/plRefMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "pnMessage/plTimeMsg.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"

static const ST::string s_defaultDeviceMagic = ST_LITERAL("(Default Device)");

#define FADE_TIME   3
#define MAX_NUM_SOURCES 128
#define UPDATE_TIME_MS 100

plProfile_CreateTimer("EAX Update", "Sound", SoundEAXUpdate);
plProfile_CreateTimer("Soft Update", "Sound", SoundSoftUpdate);
plProfile_CreateCounter("Max Sounds", "Sound", SoundMaxNum);
plProfile_CreateTimer("AudioUpdate", "RenderSetup", AudioUpdate);

//// Internal plSoftSoundNode Class Definition ///////////////////////////////
class plSoftSoundNode
{
    public:
        const plKey fSoundKey;
        float    fRank;

        plSoftSoundNode *fNext;
        plSoftSoundNode **fPrev;

        plSoftSoundNode *fSortNext;
        plSoftSoundNode **fSortPrev;

        plSoftSoundNode( const plKey s ) : fSoundKey( s ) { fNext = nil; fPrev = nil; }
        ~plSoftSoundNode() { Unlink(); }

        void    Link( plSoftSoundNode **prev )
        {
            fNext = *prev;
            fPrev = prev;
            if( fNext != nil )
                fNext->fPrev = &fNext;
            *prev = this;
        }

        void    Unlink()
        {
            if( fPrev != nil )
            {
                *fPrev = fNext;
                if( fNext )
                    fNext->fPrev = fPrev;
                fPrev = nil;
                fNext = nil;
            }
        }

        void    SortedLink( plSoftSoundNode **prev, float rank )
        {
            fRank = rank;

            fSortNext = *prev;
            fSortPrev = prev;
            if( fSortNext != nil )
                fSortNext->fSortPrev = &fSortNext;
            *prev = this;
        }

        // Larger values are first in the list
        void    AddToSortedLink( plSoftSoundNode *toAdd, float rank )
        {
            if( fRank > rank )
            {
                if( fSortNext != nil )
                    fSortNext->AddToSortedLink( toAdd, rank );
                else
                {
                    toAdd->SortedLink( &fSortNext, rank );
                }
            }
            else
            {
                // Cute trick here...
                toAdd->SortedLink( fSortPrev, rank );
            }
        }

        void    BootSourceOff()
        {
            plSound *sound = plSound::ConvertNoRef( fSoundKey->ObjectIsLoaded() );
            if( sound != nil )
            {
                sound->ForceUnregisterFromAudioSys();
            }
        }
};

// plAudioSystem //////////////////////////////////////////////////////////////////////////

int32_t   plAudioSystem::fMaxNumSounds = 16;
int32_t   plAudioSystem::fNumSoundsSlop = 8;

plAudioSystem::plAudioSystem() :
fPlaybackDevice(),
fCaptureFrequency(8000),
fStartTime(),
fListenerInit(),
fSoftRegionSounds(),
fActiveSofts(),
fCurrDebugSound(),
fDebugActiveSoundDisplay(),
fUsingEAX(),
fRestartOnDestruct(),
fWaitingForShutdown(),
fActive(),
fDisplayNumBuffers(),
fStartFade(),
fFadeLength(FADE_TIME),
fCaptureDevice(),
fEAXSupported(),
fLastUpdateTimeMs()
{
    fCurrListenerPos.Set( -1.e30, -1.e30, -1.e30 );
    //fCommittedListenerPos.Set( -1.e30, -1.e30, -1.e30 );
    fLastPos.Set(100, 100, 100);
}

plAudioSystem::~plAudioSystem()
{
}

std::vector<ST::string> plAudioSystem::GetPlaybackDevices() const
{
    std::vector<ST::string> retval;
    retval.push_back(s_defaultDeviceMagic);

    const ALchar* devices = nullptr;
    if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
        devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
    } else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
        plStatusLog::AddLineS("audio.log", plStatusLog::kYellow, "ASYS: WARNING! ALC_ENUMERATE_ALL_EXT not available.");
        devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    } else {
        plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: Unable to fetch list of playback devices.");
        return retval;
    }

    const ALchar* ptr = devices;
    while (*ptr) {
        ST::string deviceName = ST::string::from_utf8(ptr);

        // No hardware devices, sorry not sorry.
        // Also, amusing, in the MOULa OpenAL32.dll, a "Generic Hardware" device is exposed but
        // is actually a software renderer.
        if (!deviceName.starts_with("Generic Hardware"))
            retval.push_back(deviceName);
        ptr += deviceName.size() + sizeof(ALchar);
    }
    return retval;
}

ST::string plAudioSystem::GetDefaultPlaybackDevice() const
{
    if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
        return ST::string::from_utf8(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));
    } else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
        return ST::string::from_utf8(alcGetString(nullptr, ALC_DEVICE_SPECIFIER));
    } else {
        plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: Unable to fetch the default playback device name.");
        return ST::null;
    }
}

//// Init ////////////////////////////////////////////////////////////////////
bool plAudioSystem::Init()
{
    plgAudioSys::fRestarting = false;
    plStatusLog::AddLineS("audio.log", plStatusLog::kBlue, "ASYS: -- Init --");

    fMaxNumSources = 0;
    plSoundBuffer::Init();

    // Try to init using the provided device. Otherwise, fall back to the default.
    std::vector<ST::string> devices = GetPlaybackDevices();
    for (const ST::string& device : devices) {
        if (device != s_defaultDeviceMagic)
            plStatusLog::AddLineSF("audio.log", plStatusLog::kGreen, "ASYS: Found device {}", device);
    }
    ST::string deviceName = plgAudioSys::fPlaybackDeviceName;
    bool defaultDeviceRequested = (deviceName.empty() || deviceName == s_defaultDeviceMagic);
    if (defaultDeviceRequested)
        deviceName = s_defaultDeviceMagic;

    plStatusLog::AddLineSF("audio.log", plStatusLog::kBlue, "ASYS: Device '{}' selected", deviceName);
    if (!defaultDeviceRequested) {
        auto deviceIt = std::find(devices.begin(), devices.end(), deviceName);
        if (deviceIt == devices.end()) {
            plStatusLog::AddLineS("audio.log", plStatusLog::kYellow, "ASYS: WARNING! Device not in list.");
        }
    }

    if (!defaultDeviceRequested) {
        fPlaybackDevice = alcOpenDevice(deviceName.c_str());
        if (!fPlaybackDevice)
            plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: ERROR! alcOpenDevice failed, retrying with default device.");
    }

    if (!fPlaybackDevice) {
        plgAudioSys::fPlaybackDeviceName = s_defaultDeviceMagic;
        fPlaybackDevice = alcOpenDevice(nullptr);
        if (!fPlaybackDevice) {
            plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: ERROR! alcOpenDevice failed on default device.");
            return false;
        }
    }

    plStatusLog::AddLineS("audio.log", plStatusLog::kGreen, "ASYS: Device Init Success!");

    fContext = alcCreateContext(fPlaybackDevice, 0);
    alcMakeContextCurrent(fContext);

    if (alGetError() != AL_NO_ERROR) {
        plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: ERROR alcMakeContextCurrent failed");
        return false;
    }

    plStatusLog::AddLineSF("audio.log", "OpenAL vendor: {}",     alGetString(AL_VENDOR));
    plStatusLog::AddLineSF("audio.log", "OpenAL version: {}",    alGetString(AL_VERSION));
    plStatusLog::AddLineSF("audio.log", "OpenAL renderer: {}",   alGetString(AL_RENDERER));
    plStatusLog::AddLineSF("audio.log", "OpenAL extensions: {}", alGetString(AL_EXTENSIONS));
    plStatusLog::AddLineS("audio.log", plStatusLog::kGreen, "ASYS: Detecting caps...");

    // Detect maximum number of voices that can be created.
    // NOTE: This was copy-pasta'd from some old code. It is probably no longer needed since OpenAL
    //       should always use software voices, meaning we are limited only by the system RAM/CPU.
    //       Besides, even if we were using HW devices, this still isn't very useful because you
    //       may not see any errors until you actually try to *play* the sources simultaneously.
    ALuint sources[MAX_NUM_SOURCES];
    ALuint i;
    for (i = 0; i < arrsize(sources); ++i) {
        alGenSources(1, &sources[i]);
        if (alGetError() != AL_NO_ERROR)
            break;
    }
    alDeleteSources(i, sources);
    fMaxNumSources = i;
    plStatusLog::AddLineSF("audio.log", "Max Number of sources: {}", fMaxNumSources);
    SetMaxNumberOfActiveSounds();

    // TODO: Detect EAX support. Not adding this in now until the replacement is implemented.

    // attempt to init the EAX listener.
    if (plgAudioSys::fEnableEAX) {
        fUsingEAX = plEAXListener::GetInstance().Init();
        if (fUsingEAX)
            plStatusLog::AddLineS("audio.log", plStatusLog::kGreen, "ASYS: EAX support detected and enabled.");
        else
            plStatusLog::AddLineS("audio.log", plStatusLog::kRed, "ASYS: EAX support NOT detected. EAX effects disabled.");
    } else {
        fUsingEAX = false;
    }

    plProfile_Set(SoundMaxNum, fMaxNumSounds);
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    fWaitingForShutdown = false;
    plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
    return true;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void plAudioSystem::Shutdown()
{
    plStatusLog::AddLineS("audio.log", plStatusLog::kBlue, "ASYS: -- Shutdown --");

    plSoundBuffer::Shutdown();

    // Delete our active sounds list
    delete fDebugActiveSoundDisplay;
    fDebugActiveSoundDisplay = nullptr;

    // Unregister soft sounds
    while (fSoftRegionSounds) {
        fSoftRegionSounds->BootSourceOff();
        delete fSoftRegionSounds;
    }
    while (fActiveSofts) {
        fActiveSofts->BootSourceOff();
        delete fActiveSofts;
    }

    while (fEAXRegions.GetCount() > 0) {
        if (fEAXRegions[0] && fEAXRegions[0]->GetKey())
            GetKey()->Release(fEAXRegions[0]->GetKey());
        fEAXRegions.Remove(0);
    }
    plEAXListener::GetInstance().ClearProcessCache();

    plSound::SetCurrDebugPlate(nullptr);
    fCurrDebugSound = nullptr;

    // Reset this, just in case
    fPendingRegisters.Reset();

    //fListenerInit = false;

    if (fUsingEAX) {
        plEAXListener::GetInstance().Shutdown();
    }

    if (fCaptureDevice) {
        alcCaptureStop(fCaptureDevice);
        alcCaptureCloseDevice(fCaptureDevice);
        fCaptureDevice = nullptr;
    }

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(fContext);
    alcCloseDevice(fPlaybackDevice);
    fContext = nullptr;
    fPlaybackDevice = nullptr;

    fStartTime = 0;
    fUsingEAX = false;
    fCurrListenerPos.Set( -1.e30, -1.e30, -1.e30 );
    //fCommittedListenerPos.Set( -1.e30, -1.e30, -1.e30 );

    if (fRestartOnDestruct) {
        fRestartOnDestruct = false;
        plgAudioSys::Activate(true);
    }

    plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
    fWaitingForShutdown = false;
}

void plAudioSystem::SetDistanceModel(int i)
{
    switch(i)
    {
        case 0: alDistanceModel(AL_NONE); break;
        case 1: alDistanceModel(AL_INVERSE_DISTANCE ); break;
        case 2: alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); break;
        case 3: alDistanceModel(AL_LINEAR_DISTANCE ); break;
        case 4: alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED ); break;
        case 5: alDistanceModel(AL_EXPONENT_DISTANCE ); break;
        case 6: alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED); break;
    }
}

// Set the number of active sounds the audio system is allowed to play, based on the priority cutoff
void plAudioSystem::SetMaxNumberOfActiveSounds()
{
    uint16_t priorityCutoff = plgAudioSys::GetPriorityCutoff();
    int maxNumSounds = 24;

    // Keep this to a reasonable amount based on the users hardware, since we want the sounds to be played in hardware
    if (maxNumSounds > fMaxNumSources && fMaxNumSources != 0 )
        maxNumSounds = fMaxNumSources / 2;

    // Allow a certain number of sounds based on a user specified setting.
    // Above that limit, we want 1/2 more sounds (8 for the max of 16) for a slop buffer, 
    // if they fit, so that sounds that shouldn't be playing can still play in case they suddenly pop
    // back into priority range (so we don't incur performance hits when sounds hover on
    // the edge of being high enough priority to play)
    fMaxNumSounds = maxNumSounds;
    fNumSoundsSlop = fMaxNumSounds / 2;

    plStatusLog::AddLineSF("audio.log", "Max Number of Sounds Set to: {}", fMaxNumSounds);
}

void plAudioSystem::SetListenerPos(const hsPoint3 pos)
{
    fCurrListenerPos = pos;
    alListener3f(AL_POSITION, pos.fX, pos.fZ, -pos.fY);     // negate z coord, since openal uses opposite handedness
}

void plAudioSystem::SetListenerVelocity(const hsVector3 vel)
{
    alListener3f(AL_VELOCITY, 0, 0, 0); // no doppler shift
}

void plAudioSystem::SetListenerOrientation(const hsVector3 view, const hsVector3 up)
{
    ALfloat orientation[] = { view.fX, view.fZ, -view.fY, up.fX, up.fZ, -up.fY };
    alListenerfv(AL_ORIENTATION, orientation);
}

void    plAudioSystem::SetActive( bool b ) 
{
    fActive = b;
    if( fActive )
    {
        // Clear to send activate message (if listener not inited yet, delay until then)
        plgDispatch::MsgSend( new plAudioSysMsg( plAudioSysMsg::kActivate ) );
    }
}

//// IRegisterSoftSound //////////////////////////////////////////////////////
//  Note: each sound might kick another off the top-n-ranked-sounds list, so
//  we call IUpdateSoftSounds() each time one is added. Ugly as sin, but at
//  least all the calc code is in one place. Possible optimization in the
//  future: when calling IUpdate(), any sounds that are already active don't
//  need to be recalced, just resorted.
void    plAudioSystem::RegisterSoftSound( const plKey soundKey )
{
    plSoftSoundNode *node = new plSoftSoundNode( soundKey );
    node->Link( &fSoftRegionSounds );

    fCurrDebugSound = nil;
    plSound::SetCurrDebugPlate( nil );
}

//// IUnregisterSoftSound ////////////////////////////////////////////////////

void    plAudioSystem::UnregisterSoftSound( const plKey soundKey )
{
    plSoftSoundNode *node;
    for( node = fActiveSofts; node != nil; node = node->fNext )
    {
        if( node->fSoundKey == soundKey )
        {
            delete node;
            return;
        }
    }

    for( node = fSoftRegionSounds; node != nil; node = node->fNext )
    {
        if( node->fSoundKey == soundKey )
        {
            delete node;
            return;
        }
    }

    // We might have unregistered it ourselves on destruction, so don't bother

    fCurrDebugSound = nil;
    plSound::SetCurrDebugPlate( nil );
}

//// IUpdateSoftSounds ///////////////////////////////////////////////////////
//  OK, so the listener moved. Since our sound soft volumes are all based on 
//  the listener's position, we have to update all sounds with soft volumes 
//  now. This involves some steps:
//      - Determining what volumes the listener has moved out of
//      - Determining what volumes the listener has changed position in
//      - Determining what volumes the listener has entered
//      - Updating the levels of all sounds associated with all of the above 
//          volumes
//  The first two tests are easy, since we'll have kept track of what volumes 
//  the listener was in last time. The last part is the tricky one and the 
//  one that will kill us in performance if we're not careful. However, we 
//  can first check the bounding box of the sounds in question, since outside
//  of them the soft volume won't have any effect. The last part is simply a 
//  matter of querying the sound volumes based on the listener's position and
//  setting the soft volume attenuation on the sound to that level.
//
//  Note: for each sound that is still in range, we call CalcSoftVolume() and
//  use the resulting value to rank each sound. Then, we take the top n sounds
//  and disable the rest. We *could* rank by distance to listener, which would
//  be far faster; however, we could have a sound (or background music) that
//  is technically closest to the listener but completely faded by a soft volume,
//  thus needlessly cutting off another sound that might be more important.
//  This way is slower, but better quality.
//  Also note: to differentiate between two sounds in the same soft volume at
//  full strength, we divide the soft volume ranks by the distSquared, thus
//  giving us a reasonable approximation at a good rank...<sigh>

void    plAudioSystem::IUpdateSoftSounds( const hsPoint3 &newPosition )
{
    plSoftSoundNode *node, *myNode;
    float        distSquared, rank;
    plSoftSoundNode *sortedList = nil;
    int32_t           i;
    
    plProfile_BeginTiming(SoundSoftUpdate);
    
    // Check the sounds the listener is already inside of. If we moved out, stop sound, else
    // just change attenuation
    for( node = fActiveSofts; node != nil; )
    {
        plSound *sound = plSound::ConvertNoRef( node->fSoundKey->ObjectIsLoaded() );

        bool notActive = false;
        if(sound)
            sound->Update();
        
        // Quick checks for not-active
        if( sound == nil )
            notActive = true;
        else if( !sound->IsWithinRange( newPosition, &distSquared )  )
            notActive = true;
        else if(sound->GetPriority() > plgAudioSys::GetPriorityCutoff())
            notActive = true;

        if(plgAudioSys::fMutedStateChange)
        {
            sound->SetMuted(plgAudioSys::fMuted);
        }
        
        if( !notActive )
        {
            /// Our initial guess is that it's enabled...
            sound->CalcSoftVolume( true, distSquared );
            rank = sound->GetVolumeRank();
            if( rank <= 0.f )
                notActive = true;
            else
            {
                /// Queue up in our sorted list...
                if( sortedList == nil )
                    node->SortedLink( &sortedList, (10.0f - sound->GetPriority()) * rank );
                else
                    sortedList->AddToSortedLink( node, (10.0f - sound->GetPriority()) * rank );
                /// Still in radius, so consider it still "active". 
                node = node->fNext;
            }
        }

        if( notActive )
        {
            /// Moved out of range of the sound--stop the sound entirely and move it to our
            /// yeah-they're-registered-but-not-active list
            myNode = node;
            node = node->fNext;
            myNode->Unlink();
            myNode->Link( &fSoftRegionSounds );

            /// We know this sound won't be enabled, so skip the Calc() call
            if( sound != nil )
                sound->UpdateSoftVolume( false );
        }
    }
    
    // Now check remaining sounds to see if the listener moved into them
    for( node = fSoftRegionSounds; node != nil; )
    {
        if( !fListenerInit )
        {
            node = node->fNext;
            continue;
        }
    
        plSound *sound = plSound::ConvertNoRef( node->fSoundKey->ObjectIsLoaded() ); 
        if( !sound || sound->GetPriority() > plgAudioSys::GetPriorityCutoff() )
        {   
            node = node->fNext;
            continue;
        }

        sound->Update();
        if(plgAudioSys::fMutedStateChange)
        {
            sound->SetMuted(plgAudioSys::fMuted);
        }
        
        if( sound->IsWithinRange( newPosition, &distSquared ) )
        {
            /// Our initial guess is that it's enabled...
            sound->CalcSoftVolume( true, distSquared );
            rank = sound->GetVolumeRank();

            if( rank > 0.f )
            {           
                /// We just moved into its range, so move it to our active list and start the sucker
                myNode = node;
                node = node->fNext;
                myNode->Unlink();
                myNode->Link( &fActiveSofts );

                /// Queue up in our sorted list...
                if( sortedList == nil )
                    myNode->SortedLink( &sortedList, (10.0f - sound->GetPriority()) * rank );
                else
                    sortedList->AddToSortedLink( myNode, (10.0f - sound->GetPriority()) * rank );
            }
            else
            {
                /// Do NOT notify sound, since we were outside of its range and still are
                // (but if we're playing, we shouldn't be, so better update)
                if( sound->IsPlaying() )
                    sound->UpdateSoftVolume( false );

                node = node->fNext;
            }
        }
        else
        {
            /// Do NOT notify sound, since we were outside of its range and still are
            node = node->fNext;     
            sound->Disable();       // ensure that dist attenuation is set to zero so we don't accidentally play
        }
    }
    
    plgAudioSys::fMutedStateChange = false;
    /// Go through sorted list, enabling only the first n sounds and disabling the rest
    // DEBUG: Create a screen-only statusLog to display which sounds are what
    if( fDebugActiveSoundDisplay == nil )
        fDebugActiveSoundDisplay = plStatusLogMgr::GetInstance().CreateStatusLog( 32, "Active Sounds", plStatusLog::kDontWriteFile | plStatusLog::kDeleteForMe | plStatusLog::kFilledBackground );
    fDebugActiveSoundDisplay->Clear();

    if(fDisplayNumBuffers)
        fDebugActiveSoundDisplay->AddLineF(0xffffffff, "Num Buffers: {}", plDSoundBuffer::GetNumBuffers());
    fDebugActiveSoundDisplay->AddLine(plStatusLog::kGreen, "Not streamed");
    fDebugActiveSoundDisplay->AddLine(plStatusLog::kYellow, "Disk streamed");
    fDebugActiveSoundDisplay->AddLine(plStatusLog::kWhite, "RAM streamed");
    fDebugActiveSoundDisplay->AddLine(plStatusLog::kRed, "Ogg streamed");
    fDebugActiveSoundDisplay->AddLine(0xff00ffff, "Incidentals");
    fDebugActiveSoundDisplay->AddLine("--------------------");
    
    for( i = 0; sortedList != nil && i < fMaxNumSounds; sortedList = sortedList->fSortNext )
    {
        plSound *sound = plSound::ConvertNoRef( sortedList->fSoundKey->GetObjectPtr() );
        if(!sound) continue;
    
        /// Notify sound that it really is still enabled
        sound->UpdateSoftVolume( true );
        
        uint32_t color = plStatusLog::kGreen;
        switch (sound->GetStreamType())
        {
            case plSound::kStreamFromDisk:      color = plStatusLog::kYellow;   break;
            case plSound::kStreamFromRAM:       color = plStatusLog::kWhite;    break;
            case plSound::kStreamCompressed:    color = plStatusLog::kRed;      break;
            default: break;
        }
        if(sound->GetType() == plgAudioSys::kVoice) color = 0xffff8800;
        if(sound->IsPropertySet(plSound::kPropIncidental)) color = 0xff00ffff;
        
        if( fUsingEAX && sound->GetEAXSettings().IsEnabled() )
        {
            fDebugActiveSoundDisplay->AddLineF(
                color, 
                "{} {1.2f} {1.2f} ({} occ) {}",
                sound->GetPriority(),
                sortedList->fRank,
                sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0,
                sound->GetEAXSettings().GetCurrSofts().GetOcclusion(),
                sound->GetKeyName());
        }
        else 
        {
            fDebugActiveSoundDisplay->AddLineF(
                color,
                "{} {1.2f} {1.2f} {}",
                sound->GetPriority(),
                sortedList->fRank,
                sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0,
                sound->GetKeyName());
        }
        i++;
    }

    for( ; sortedList != nil; sortedList = sortedList->fSortNext, i++ )
    {
        plSound *sound = plSound::ConvertNoRef( sortedList->fSoundKey->GetObjectPtr() );
        if(!sound) continue;

        /// These unlucky sounds don't get to play (yet). Also, be extra mean
        /// and pretend we're updating for "the first time", which will force them to
        /// stop immediately
        // Update: since being extra mean can incur a nasty performance hit when sounds hover back and
        // forth around the fMaxNumSounds mark, we have a "slop" allowance: i.e. sounds that we're going
        // to say shouldn't be playing but we'll let them play for a bit anyway just in case they raise
        // in priority. So only be mean to the sounds outside this slop range
        sound->UpdateSoftVolume( false, ( i < fMaxNumSounds + fNumSoundsSlop ) ? false : true );
        fDebugActiveSoundDisplay->AddLineF(
            0xff808080,
            "{} {1.2f} {}",
            sound->GetPriority(),
            sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0,
            sound->GetKeyName());
    }
    
    plProfile_EndTiming(SoundSoftUpdate);
}

void    plAudioSystem::NextDebugSound()
{
    plSoftSoundNode     *node;

    if( fCurrDebugSound == nil )
        fCurrDebugSound = ( fSoftRegionSounds == nil ) ? fActiveSofts : fSoftRegionSounds;
    else
    {
        node = fCurrDebugSound;
        fCurrDebugSound = fCurrDebugSound->fNext;
        if( fCurrDebugSound == nil )
        {
            // Trace back to find which list we were in
            for( fCurrDebugSound = fSoftRegionSounds; fCurrDebugSound != nil; fCurrDebugSound = fCurrDebugSound->fNext )
            {
                if( fCurrDebugSound == node )   // Was in first list, move to 2nd
                {
                    fCurrDebugSound = fActiveSofts;     
                    break;
                }
            }
            // else Must've been in 2nd list, so keep nil
        }
    }

    if( fCurrDebugSound != nil )
        plSound::SetCurrDebugPlate( fCurrDebugSound->fSoundKey );
    else
        plSound::SetCurrDebugPlate( nil );
}

void plAudioSystem::SetFadeLength(float lengthSec)
{
    fFadeLength = lengthSec;
}
    
bool plAudioSystem::MsgReceive(plMessage* msg)
{
    if(plTimeMsg *time = plTimeMsg::ConvertNoRef( msg ) )
    {
        if(!plgAudioSys::IsMuted())
        {
            double currTime = hsTimer::GetSeconds();
            if(fStartFade == 0)
            {
                plStatusLog::AddLineSF("audio.log", "Starting Fade {f}", currTime);
            }
            if((currTime - fStartFade) > fFadeLength) 
            {
                fStartFade = 0;
                plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );
                plStatusLog::AddLineSF("audio.log", "Stopping Fade {f}", currTime);
                plgAudioSys::SetGlobalFadeVolume( 1.0 ); 
            }
            else
            {
                plgAudioSys::SetGlobalFadeVolume( (float)((currTime-fStartFade) / fFadeLength) );
            }
        }
        return true;
    }
    
    if (plAudioSysMsg* pASMsg = plAudioSysMsg::ConvertNoRef( msg ))
    {
        if (pASMsg->GetAudFlag() == plAudioSysMsg::kPing && fListenerInit)
        {
            plAudioSysMsg* pMsg = new plAudioSysMsg( plAudioSysMsg::kActivate );
            pMsg->AddReceiver( pASMsg->GetSender() );
            pMsg->SetBCastFlag(plMessage::kBCastByExactType, false);
            plgDispatch::MsgSend( pMsg );
            return true;
        }
        else if (pASMsg->GetAudFlag() == plAudioSysMsg::kSetVol)
        {
            return true;
        }
        else if( pASMsg->GetAudFlag() == plAudioSysMsg::kDestroy )
        {
            if( fWaitingForShutdown )
                Shutdown();
            return true;
        }
        else if( pASMsg->GetAudFlag() == plAudioSysMsg::kUnmuteAll )
        {
            if( !pASMsg->HasBCastFlag( plMessage::kBCastByExactType ) )
                plgAudioSys::SetMuted( false );
            return true;
        }
    }
    
    if(plRenderMsg* pRMsg = plRenderMsg::ConvertNoRef( msg ))
    {
        //if( fListener )
        {
            plProfile_BeginLap(AudioUpdate, this->GetKey()->GetUoid().GetObjectName().c_str());
            if(hsTimer::GetMilliSeconds() - fLastUpdateTimeMs > UPDATE_TIME_MS)
            {
                IUpdateSoftSounds( fCurrListenerPos );

                if( fUsingEAX )
                {
                    plProfile_BeginTiming(SoundEAXUpdate);
                    plEAXListener::GetInstance().ProcessMods( fEAXRegions );
                    plProfile_EndTiming(SoundEAXUpdate);
                }
                //fCommittedListenerPos = fCurrListenerPos;
            }
            plProfile_EndLap(AudioUpdate, this->GetKey()->GetUoid().GetObjectName().c_str());
        }
        
        return true;
    }

    if(plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg ))
    {
        if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
        {
            fEAXRegions.Append( plEAXListenerMod::ConvertNoRef( refMsg->GetRef() ) );
            plEAXListener::GetInstance().ClearProcessCache();
        }
        else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
        {
            int idx = fEAXRegions.Find( plEAXListenerMod::ConvertNoRef( refMsg->GetRef() ) );
            if( idx != fEAXRegions.kMissingIndex )
                fEAXRegions.Remove( idx );
            plEAXListener::GetInstance().ClearProcessCache();
        }
        return true;
    }
    
    if(plAgeLoadedMsg *pALMsg = plAgeLoadedMsg::ConvertNoRef(msg))
    {
        if(!pALMsg->fLoaded)
        {
            fLastPos = fCurrListenerPos;
            fListenerInit = false;
        }
        else
        {
            fListenerInit = true;
        }
    }

    return hsKeyedObject::MsgReceive(msg);
}

bool plAudioSystem::BeginCapture()
{
    if (fCaptureDevice) {
        if (!EndCapture())
            return false;
    }

    ALCsizei bufferSize = fCaptureFrequency * sizeof(int16_t) * BUFFER_LEN_SECONDS;
    fCaptureDevice = alcCaptureOpenDevice(nullptr, fCaptureFrequency, AL_FORMAT_MONO16, bufferSize);
    if (fCaptureDevice) {
        alcCaptureStart(fCaptureDevice);
        return true;
    } else {
        return false;
    }
}

bool plAudioSystem::CaptureSamples(uint32_t samples, int16_t* data) const
{
    if (!fCaptureDevice)
        return false;

    alcCaptureSamples(fCaptureDevice, data, samples);
    return true;
}

uint32_t plAudioSystem::GetCaptureSampleCount() const
{
    if (!fCaptureDevice)
        return 0;

    ALCsizei samples;
    alcGetIntegerv(fCaptureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
    return samples;
}

bool plAudioSystem::SetCaptureSampleRate(uint32_t sampleRate)
{
    if (fCaptureFrequency != sampleRate) {
        fCaptureFrequency = sampleRate;
        if (fCaptureDevice) {
            if (!EndCapture())
                return false;
            return BeginCapture();
        }
    }
    return true;
}

bool plAudioSystem::EndCapture()
{
    if (!fCaptureDevice)
        return false;

    alcCaptureStop(fCaptureDevice);

    // discard any samples not processed
    ALCsizei samples;
    alcGetIntegerv(fCaptureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples) {
        auto buf = std::make_unique<int16_t[]>(samples);
        alcCaptureSamples(fCaptureDevice, buf.get(), samples);
    }

    alcCaptureCloseDevice(fCaptureDevice);
    fCaptureDevice = nullptr;
    return true;
}

// plgAudioSystem //////////////////////////////////////////////////////////////////////

plAudioSystem*  plgAudioSys::fSys = nullptr;
bool            plgAudioSys::fInit = false;
bool            plgAudioSys::fActive = false;
bool            plgAudioSys::fMuted = true;
bool            plgAudioSys::fDelayedActivate = false;
bool            plgAudioSys::fEnableEAX = false;
float           plgAudioSys::fChannelVolumes[kNumChannels] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
uint32_t        plgAudioSys::fDebugFlags = 0;
float           plgAudioSys::fStreamingBufferSize = 2.f;
float           plgAudioSys::fStreamFromRAMCutoff = 10.f;
uint8_t         plgAudioSys::fPriorityCutoff = 9;           // We cut off sounds above this priority
bool            plgAudioSys::fEnableExtendedLogs = false;
float           plgAudioSys::fGlobalFadeVolume = 1.f;
bool            plgAudioSys::fLogStreamingUpdates = false;
ST::string      plgAudioSys::fPlaybackDeviceName;
bool            plgAudioSys::fRestarting = false;
bool            plgAudioSys::fMutedStateChange = false;

void plgAudioSys::Init()
{
    fSys = new plAudioSystem;
    fSys->RegisterAs(kAudioSystem_KEY);
    plgDispatch::Dispatch()->RegisterForExactType(plAudioSysMsg::Index(), fSys->GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), fSys->GetKey());

    if (fMuted)
        SetGlobalFadeVolume(0.0f);

    if (fDelayedActivate)
        Activate( true );
}

void plgAudioSys::SetActive(bool b)
{
    fActive = b;
}

void plgAudioSys::SetMuted( bool b )
{
    fMuted = b;
    fMutedStateChange = true;

    if(fMuted)
        SetGlobalFadeVolume(0.0f);
    else
        SetGlobalFadeVolume(1.0);
}

void plgAudioSys::EnableEAX( bool b )
{
    fEnableEAX = b;
    if( fActive )
        Restart();
}

void plgAudioSys::Restart()
{
    if( fSys )
    {
        fSys->fRestartOnDestruct = true;
        Activate( false );
        fRestarting = true;
    }
}

void plgAudioSys::Shutdown()
{
    Activate( false );
    if( fSys )
    {
        fSys->UnRegisterAs( kAudioSystem_KEY );
    }
}

void plgAudioSys::Activate(bool b)
{ 
    if( fSys == nil )
    {
        fDelayedActivate = true;
        return;
    }

    if (b == fInit)
        return;
    if (!fActive)
        return;
    if( b )
    {
        plStatusLog::AddLineS("audio.log", plStatusLog::kBlue, "ASYS: -- Attempting audio system init --");
        if( !fSys->Init() )
        {
            // Cannot init audio system. Don't activate
            return; 
        }
        fInit = true;
        fSys->SetActive( true );

        if( !IsMuted() )
        {
            SetMuted( true );
            plAudioSysMsg *msg = new plAudioSysMsg( plAudioSysMsg::kUnmuteAll );
            msg->SetTimeStamp( hsTimer::GetSysSeconds() );
            msg->AddReceiver( fSys->GetKey() );
            msg->SetBCastFlag( plMessage::kBCastByExactType, false );
            msg->Send();
        }
        return;
    }

    fSys->SetActive( false );
    
    plStatusLog::AddLineS("audio.log", plStatusLog::kBlue, "ASYS: -- Sending deactivate/destroy messages --");
    plgDispatch::MsgSend( new plAudioSysMsg( plAudioSysMsg::kDeActivate ) );

    // Send ourselves a shutdown message, so that the deactivates get processed first
    fSys->fWaitingForShutdown = true;
    plAudioSysMsg *msg = new plAudioSysMsg( plAudioSysMsg::kDestroy );
    msg->SetBCastFlag( plMessage::kBCastByExactType, false );
    msg->Send( fSys->GetKey() );
//  fSys->Shutdown();

    fInit = false;
}

void    plgAudioSys::SetChannelVolume( ASChannel chan, float vol )
{
    fChannelVolumes[ chan ] = vol;
}

void    plgAudioSys::SetGlobalFadeVolume( float vol )
{
    if(!fMuted)
        fGlobalFadeVolume = vol;
    else
        fGlobalFadeVolume = 0;
}

float    plgAudioSys::GetChannelVolume( ASChannel chan )
{
    return fChannelVolumes[ chan ];
}

void    plgAudioSys::NextDebugSound()
{
    fSys->NextDebugSound();
}

void plgAudioSys::RegisterSoftSound( const plKey soundKey )
{
    if(fSys)
    {
        fSys->RegisterSoftSound(soundKey);
    }
}

void plgAudioSys::UnregisterSoftSound( const plKey soundKey )
{
    if(fSys)
    {
        fSys->UnregisterSoftSound(soundKey);
    }
}

void plgAudioSys::SetListenerPos(const hsPoint3 pos)
{
    if(fSys)
    {
        fSys->SetListenerPos(pos);
    }
}

void plgAudioSys::SetListenerVelocity(const hsVector3 vel)
{
    if(fSys)
    {
        fSys->SetListenerVelocity(vel);
    }
}

void plgAudioSys::SetListenerOrientation(const hsVector3 view, const hsVector3 up)
{
    if(fSys)
    {
        fSys->SetListenerOrientation(view, up);
    }
}
