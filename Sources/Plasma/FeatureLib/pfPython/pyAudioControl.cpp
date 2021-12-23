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


#include "pyAudioControl.h"

#include "plAudio/plAudioSystem.h"
#include "plAudio/plVoiceChat.h"

// Sets the master volume of a given audio channel
void pyAudioControl::SetSoundFXVolume(float volume)
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

void pyAudioControl::SetMusicVolume(float volume)
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

void pyAudioControl::SetVoiceVolume(float volume)
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

void pyAudioControl::SetAmbienceVolume(float volume)
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

void pyAudioControl::SetGUIVolume(float volume)
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

void pyAudioControl::SetNPCVoiceVolume(float volume)
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

float pyAudioControl::GetSoundFXVolume() const
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kSoundFX;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetMusicVolume() const
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kBgndMusic;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetVoiceVolume() const
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kVoice;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetAmbienceVolume() const
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kAmbience;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetGUIVolume() const
{
    plgAudioSys::ASChannel  chan;
    chan = plgAudioSys::kGUI;
    return plgAudioSys::GetChannelVolume(chan);
}

float pyAudioControl::GetNPCVoiceVolume() const
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

bool pyAudioControl::IsEnabled() const
{
    return plgAudioSys::Active();
}


// Enable or disable load-on-demand for sounds
void pyAudioControl::SetLoadOnDemand(bool state)
{
    plSound::SetLoadOnDemand(state);
}


// Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
// ...Less of a performance hit, harder on memory.
void pyAudioControl::SetTwoStageLOD(bool state)
{
    // For two-stage LOD, we want to disable LoadFromDiskOnDemand, so that we'll load into RAM at startup but not
    // into sound buffers until demanded to do so. Enabling LoadFromDiskOnDemand basically conserves as much memory
    // as possible
    plSound::SetLoadFromDiskOnDemand(!state);
}

// Enable EAX sound acceleration (requires hardware acceleration)
void pyAudioControl::UseEAXAcceleration(bool state)
{
    plgAudioSys::EnableEAX(state);
}

bool pyAudioControl::IsUsingEAXAcceleration() const
{
    return plgAudioSys::UsingEAX();
}

bool pyAudioControl::IsEAXSupported() const
{
    return plgAudioSys::IsEAXSupported();
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

bool pyAudioControl::IsMuted() const
{
    return plgAudioSys::IsMuted();
}

// Enable or disable displaying speech subtitles
void pyAudioControl::EnableSubtitles()
{
    plgAudioSys::SetEnableSubtitles(true);
}

void pyAudioControl::DisableSubtitles()
{
    plgAudioSys::SetEnableSubtitles(false);
}

bool pyAudioControl::IsEnabledSubtitles() const
{
    return plgAudioSys::IsEnabledSubtitles();
}


//------------------------
// Voice Settings

bool pyAudioControl::CanSetMicLevel() const
{
    return plgAudioSys::CanChangeCaptureVolume();
}

void pyAudioControl::SetMicLevel(float level)
{
    plgAudioSys::SetCaptureVolume(level);
}

float pyAudioControl::GetMicLevel() const
{
    return plgAudioSys::GetCaptureVolume();
}


// turn voice recording on or off
void pyAudioControl::EnableVoiceRecording(bool state)
{
    plVoiceRecorder::EnableRecording(state);
}

bool pyAudioControl::IsVoiceRecordingEnabled() const
{
    return plVoiceRecorder::RecordingEnabled();
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
void pyAudioControl::PushToTalk(bool state)
{
    plVoiceRecorder::EnablePushToTalk(state);
}

// Set the squelch level
void pyAudioControl::SquelchLevel(float level)
{
    plVoiceRecorder::SetSquelch(level);
}

uint8_t pyAudioControl::GetPriorityCutoff() const
{
    return plgAudioSys::GetPriorityCutoff();
}

void pyAudioControl::SetPriorityCutoff( uint8_t cut )
{
    plgAudioSys::SetPriorityCutoff( cut );
}

void pyAudioControl::SetPlaybackDevice(const ST::string& device, bool restart)
{
    plgAudioSys::SetPlaybackDevice(device, restart);
}

ST::string pyAudioControl::GetPlaybackDevice() const
{
    return plgAudioSys::GetPlaybackDevice();
}

std::vector<ST::string> pyAudioControl::GetPlaybackDevices() const
{
    return plgAudioSys::GetPlaybackDevices();
}

ST::string pyAudioControl::GetFriendlyDeviceName(const ST::string& deviceName) const
{
    return plgAudioSys::GetFriendlyDeviceName(deviceName);
}

void pyAudioControl::SetCaptureDevice(const ST::string& device)
{
    plgAudioSys::SetCaptureDevice(device);
}

ST::string pyAudioControl::GetCaptureDevice() const
{
    return plgAudioSys::GetCaptureDevice();
}

std::vector<ST::string> pyAudioControl::GetCaptureDevices() const
{
    return plgAudioSys::GetCaptureDevices();
}
