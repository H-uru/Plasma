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
//////////////////////////////////////////////////////////////////////
//
// pyAudioControl   - a wrapper class all the audio control functions
//
//////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "pyAudioControl.h"

#include "plAudio/plAudioSystem.h"
#include "plAudio/plVoiceChat.h"
#include "plAudio/plWinMicLevel.h"

#include "plAudio/plAudioCaps.h"

// Sets the master volume of a given audio channel
void pyAudioControl::SetSoundFXVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kSoundFX;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetMusicVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kBgndMusic;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetVoiceVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kVoice;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetAmbienceVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kAmbience;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetGUIVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kGUI;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetNPCVoiceVolume( float volume )
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kNPCVoice;

    // make sure the volume is within range
    if( volume > 1.f )
        volume = 1.f;
    else if( volume < 0.f )
        volume = 0.f;

    plgAudioSys::SetChannelVolume( chan, volume );
}

float pyAudioControl::GetSoundFXVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kSoundFX;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetMusicVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kBgndMusic;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetVoiceVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kVoice;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetAmbienceVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kAmbience;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetGUIVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kGUI;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetNPCVoiceVolume()
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kNPCVoice;
    return plgAudioSys::GetChannelVolume(chan);
}


// Switch DirectX Audio on or off at runtime
void pyAudioControl::Enable()
{
    plgAudioSys::Activate(true);
}

void pyAudioControl::Disable()
{
    plgAudioSys::Activate(false);
}

bool pyAudioControl::IsEnabled()
{
    return plgAudioSys::Active();
}


// Enable or disable load-on-demand for sounds
void pyAudioControl::SetLoadOnDemand( bool state )
{
    plSound::SetLoadOnDemand(state);
}


// Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
// ...Less of a performance hit, harder on memory.
void pyAudioControl::SetTwoStageLOD( bool state )
{
    // For two-stage LOD, we want to disable LoadFromDiskOnDemand, so that we'll load into RAM at startup but not
    // into sound buffers until demanded to do so. Enabling LoadFromDiskOnDemand basically conserves as much memory
    // as possible
    plSound::SetLoadFromDiskOnDemand( !state );
}


// Enable audio hardware acceleration
void pyAudioControl::UseHardwareAcceleration( bool state )
{
    plgAudioSys::SetUseHardware(state);
}

bool pyAudioControl::IsHardwareAccelerated()
{
    return plgAudioSys::Hardware();
}


// Enable EAX sound acceleration (requires hardware acceleration)
void pyAudioControl::UseEAXAcceleration( bool state )
{
    plgAudioSys::EnableEAX(state);
}

bool pyAudioControl::IsUsingEAXAcceleration()
{
    return plgAudioSys::UsingEAX();
}


// Mute or unmute all sounds
void pyAudioControl::MuteAll()
{
    plgAudioSys::SetMuted(true);
}

void pyAudioControl::UnmuteAll()
{
    plgAudioSys::SetMuted(false);
}

bool pyAudioControl::IsMuted()
{
    return plgAudioSys::IsMuted();
}

bool pyAudioControl::SupportEAX(const char *deviceName)
{
    return plgAudioSys::SupportsEAX(deviceName);
}


//------------------------
// Voice Settings

// Sets the microphone volume, in the range of 0 to 1
bool pyAudioControl::CanSetMicLevel()
{
    return plWinMicLevel::CanSetLevel();
}

void pyAudioControl::SetMicLevel( float level )
{
    // make sure the volume is within range
    if( level > 1.f )
        level = 1.f;
    else if( level < 0.f )
        level = 0.f;
    if( CanSetMicLevel() )
        plWinMicLevel::SetLevel( level );
}

float pyAudioControl::GetMicLevel()
{
    return plWinMicLevel::GetLevel();
}


// turn voice recording on or off
void pyAudioControl::EnableVoiceRecording( bool state )
{
    plVoiceRecorder::EnableRecording(state);
}

bool pyAudioControl::IsVoiceRecordingEnabled()
{
    return plVoiceRecorder::RecordingEnabled();
}


// turn voice compression on and off
void pyAudioControl::EnableVoiceCompression( bool state )
{
}

bool pyAudioControl::IsVoiceCompressionEnabled()
{
    return true;
}


// turn voice-over-net on and off
void pyAudioControl::EnableVoiceNetBroadcast( bool state )
{
    //plWinRecorder::EnableNetVoice(state);
}

bool pyAudioControl::IsVoiceNetBroadcastEnabled()
{
    
    return true;
}

void pyAudioControl::EnableVoiceChat(bool enable)
{
    plVoicePlayer::Enable(enable);
}


// turn voice recording icons on and off
void pyAudioControl::ShowIcons()
{
    plVoiceRecorder::EnableIcons(true);
}

void pyAudioControl::HideIcons()
{
    plVoiceRecorder::EnableIcons(false);
}


// turn push-to-talk on or off
void pyAudioControl::PushToTalk( bool state )
{
    plVoiceRecorder::EnablePushToTalk(state);
}

// Set the squelch level
void pyAudioControl::SquelchLevel( float level )
{
    plVoiceRecorder::SetSquelch(level);
}


// Adjust voice packet frame size
void pyAudioControl::RecordFrame( int32_t size )
{
}


// Set the sample rate for recording
void pyAudioControl::RecordSampleRate( int32_t sample_rate )
{
}

uint8_t pyAudioControl::GetPriorityCutoff( void )
{
    return plgAudioSys::GetPriorityCutoff();
}

void pyAudioControl::SetPriorityCutoff( uint8_t cut )
{
    plgAudioSys::SetPriorityCutoff( cut );
}

void pyAudioControl::SetAudioSystemMode(int mode)
{
    switch (mode)
    {
    case plgAudioSys::kDisabled:
        plgAudioSys::SetAudioMode(plgAudioSys::kDisabled);
        break;
    case plgAudioSys::kSoftware:
        plgAudioSys::SetAudioMode(plgAudioSys::kSoftware);
        break;
    case plgAudioSys::kHardware:
        plgAudioSys::SetAudioMode(plgAudioSys::kHardware);
        break;
    case plgAudioSys::kHardwarePlusEAX:
        plgAudioSys::SetAudioMode(plgAudioSys::kHardwarePlusEAX);
        break;
    default:
        break;
    }
}

int pyAudioControl::GetAudioSystemMode()
{
    return plgAudioSys::GetAudioMode();
}

int pyAudioControl::GetHighestAudioMode()
{
    int highestMode = plgAudioSys::kDisabled;
    plAudioCaps caps = plAudioCapsDetector::Detect();

    if ( caps.IsEAXAvailable() )
    {
        highestMode = plgAudioSys::kHardwarePlusEAX;
    }
    else
    {
        if ( 1 )        // This is taken care of in the audio system
        {
            highestMode = plgAudioSys::kHardware;
        }
        else
        {
            if ( caps.IsAvailable() )
            {
                highestMode = plgAudioSys::kSoftware;
            }
        }
    }

    return highestMode;
}

int pyAudioControl::GetNumAudioDevices()
{
    return plgAudioSys::GetNumAudioDevices();
}

const char *pyAudioControl::GetAudioDeviceName(int index)
{
    return plgAudioSys::GetAudioDeviceName(index);
}

void pyAudioControl::SetDeviceName(const char *device, bool restart)
{
    plgAudioSys::SetDeviceName(device, restart);
}

const char * pyAudioControl::GetDeviceName()
{
    return plgAudioSys::GetDeviceName();
}
