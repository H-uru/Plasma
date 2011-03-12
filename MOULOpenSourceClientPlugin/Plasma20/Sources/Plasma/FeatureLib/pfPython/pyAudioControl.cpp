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
//////////////////////////////////////////////////////////////////////
//
// pyAudioControl   - a wrapper class all the audio control functions
//
//////////////////////////////////////////////////////////////////////

#include "pyAudioControl.h"

#include "../plAudio/plAudioSystem.h"
#include "../plAudio/plVoiceChat.h"
#include "../plAudio/plWinMicLevel.h"

#include "../plAudio/plAudioCaps.h"

// Sets the master volume of a given audio channel
void pyAudioControl::SetSoundFXVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kSoundFX;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetMusicVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kBgndMusic;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetVoiceVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kVoice;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetAmbienceVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kAmbience;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetGUIVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kGUI;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

void pyAudioControl::SetNPCVoiceVolume( hsScalar volume )
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kNPCVoice;

	// make sure the volume is within range
	if( volume > 1.f )
		volume = 1.f;
	else if( volume < 0.f )
		volume = 0.f;

	plgAudioSys::SetChannelVolume( chan, volume );
}

hsScalar pyAudioControl::GetSoundFXVolume()
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kSoundFX;
	return plgAudioSys::GetChannelVolume(chan);
}

hsScalar pyAudioControl::GetMusicVolume()
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kBgndMusic;
	return plgAudioSys::GetChannelVolume(chan);
}

hsScalar pyAudioControl::GetVoiceVolume()
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kVoice;
	return plgAudioSys::GetChannelVolume(chan);
}

hsScalar pyAudioControl::GetAmbienceVolume()
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kAmbience;
	return plgAudioSys::GetChannelVolume(chan);
}

hsScalar pyAudioControl::GetGUIVolume()
{
	plgAudioSys::ASChannel	chan;
	chan = plgAudioSys::kGUI;
	return plgAudioSys::GetChannelVolume(chan);
}

hsScalar pyAudioControl::GetNPCVoiceVolume()
{
	plgAudioSys::ASChannel	chan;
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

hsBool pyAudioControl::IsEnabled()
{
	return plgAudioSys::Active();
}


// Enable or disable load-on-demand for sounds
void pyAudioControl::SetLoadOnDemand( hsBool state )
{
	plSound::SetLoadOnDemand(state);
}


// Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
// ...Less of a performance hit, harder on memory.
void pyAudioControl::SetTwoStageLOD( hsBool state )
{
	// For two-stage LOD, we want to disable LoadFromDiskOnDemand, so that we'll load into RAM at startup but not
	// into sound buffers until demanded to do so. Enabling LoadFromDiskOnDemand basically conserves as much memory
	// as possible
	plSound::SetLoadFromDiskOnDemand( !state );
}


// Enable audio hardware acceleration
void pyAudioControl::UseHardwareAcceleration( hsBool state )
{
	plgAudioSys::SetUseHardware(state);
}

hsBool pyAudioControl::IsHardwareAccelerated()
{
	return plgAudioSys::Hardware();
}


// Enable EAX sound acceleration (requires hardware acceleration)
void pyAudioControl::UseEAXAcceleration( hsBool state )
{
	plgAudioSys::EnableEAX(state);
}

hsBool pyAudioControl::IsUsingEAXAcceleration()
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

hsBool pyAudioControl::IsMuted()
{
	return plgAudioSys::IsMuted();
}

hsBool pyAudioControl::SupportEAX(const char *deviceName)
{
	return plgAudioSys::SupportsEAX(deviceName);
}


//------------------------
// Voice Settings

// Sets the microphone volume, in the range of 0 to 1
hsBool pyAudioControl::CanSetMicLevel()
{
	return plWinMicLevel::CanSetLevel();
}

void pyAudioControl::SetMicLevel( hsScalar level )
{
	// make sure the volume is within range
	if( level > 1.f )
		level = 1.f;
	else if( level < 0.f )
		level = 0.f;
	if( CanSetMicLevel() )
		plWinMicLevel::SetLevel( level );
}

hsScalar pyAudioControl::GetMicLevel()
{
	return plWinMicLevel::GetLevel();
}


// turn voice recording on or off
void pyAudioControl::EnableVoiceRecording( hsBool state )
{
	plVoiceRecorder::EnableRecording(state);
}

hsBool pyAudioControl::IsVoiceRecordingEnabled()
{
	return plVoiceRecorder::RecordingEnabled();
}


// turn voice compression on and off
void pyAudioControl::EnableVoiceCompression( hsBool state )
{
}

hsBool pyAudioControl::IsVoiceCompressionEnabled()
{
	return true;
}


// turn voice-over-net on and off
void pyAudioControl::EnableVoiceNetBroadcast( hsBool state )
{
	//plWinRecorder::EnableNetVoice(state);
}

hsBool pyAudioControl::IsVoiceNetBroadcastEnabled()
{
	
	return true;
}

void pyAudioControl::EnableVoiceChat(hsBool enable)
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
void pyAudioControl::PushToTalk( hsBool state )
{
	plVoiceRecorder::EnablePushToTalk(state);
}

// Set the squelch level
void pyAudioControl::SquelchLevel( hsScalar level )
{
	plVoiceRecorder::SetSquelch(level);
}


// Adjust voice packet frame size
void pyAudioControl::RecordFrame( Int32 size )
{
}


// Set the sample rate for recording
void pyAudioControl::RecordSampleRate( Int32 sample_rate )
{
}

UInt8 pyAudioControl::GetPriorityCutoff( void )
{
	return plgAudioSys::GetPriorityCutoff();
}

void pyAudioControl::SetPriorityCutoff( UInt8 cut )
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
		if ( 1 )		// This is taken care of in the audio system
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