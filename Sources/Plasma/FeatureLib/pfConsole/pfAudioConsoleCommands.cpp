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
//  Audio & Listener Console Commands and Groups                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
#define LIMIT_CONSOLE_COMMANDS 1
#endif

#include <string_theory/format>
#include <string_theory/string>

#include "plgDispatch.h"
#include "hsResMgr.h"

#include "pfConsoleCommandUtilities.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnMessage/plAudioSysMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plAudioInterface.h"

#include "plAudio/plAudioSystem.h"
#include "plAudio/plVoiceChat.h"
#include "plMessage/plListenerMsg.h"
#include "plStatusLog/plStatusLog.h"

#include "pfAudio/plListener.h"
#include "pfConsoleCore/pfConsoleCmd.h"

#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Audio)
//// DO NOT REMOVE!!!!

/////////////////////////////////////////////////////////////////
//
// Please see pfConsoleCommands.cpp for detailed instructions on
// how to add console commands.
//
/////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP(Audio)

PF_CONSOLE_CMD(Audio, Enable, "bool on", "Switch Audio on or off at runtime")
{
    bool on = params[0];
    plgAudioSys::Activate( on );
}

PF_CONSOLE_CMD(Audio, UseEAX, "bool on", "Enable EAX sound acceleration (requires hardware acceleration)")
{
    bool on = params[0];
    plgAudioSys::EnableEAX( on );
}

PF_CONSOLE_CMD(Audio, Initialize, "bool on", "Set to false to completely disable audio playback in plasma")
{ 
    bool on = params[0];
    plgAudioSys::SetActive(on);
}

PF_CONSOLE_CMD(Audio, Restart, "", "Restarts the audio system" )
{ 
    plgAudioSys::Restart();
}

PF_CONSOLE_CMD(Audio, MuteAll, "bool on", "Mute or unmute all sounds")
{
    plgAudioSys::SetMuted( (bool)params[ 0 ] );
}

PF_CONSOLE_CMD(Audio, EnableSubtitles, "bool on", "Enable or disable displaying subtitles for audio files containing speech")
{
    plgAudioSys::SetEnableSubtitles((bool)params[0]);
}

PF_CONSOLE_CMD(Audio, SetDistanceModel, "int type", "Sets the distance model for all 3d sounds")
{
    plgAudioSys::SetDistanceModel((int)params[0]);
}

PF_CONSOLE_CMD(Audio, LogStreamingUpdates, "bool on", "Logs every buffer fill for streaming sounds")
{
    plgAudioSys::SetLogStreamingUpdates((bool) params[0]);
}

PF_CONSOLE_CMD(Audio, SetAllChannelVolumes, "float soundFX, float music, float ambience, float voice, float gui", "Sets the master volume of all the given audio channels.")
{
    plgAudioSys::ASChannel  chans[ 6 ] = { plgAudioSys::kSoundFX, plgAudioSys::kBgndMusic, plgAudioSys::kAmbience, plgAudioSys::kVoice, plgAudioSys::kGUI, plgAudioSys::kNPCVoice };


    int i;

    for( i = 0; i < 5; i++ )
    {
        float    vol = (float)(float)params[ i ];
        if( vol > 1.f )
            vol = 1.f;
        else if( vol < 0.f )
            vol = 0.f;

        plgAudioSys::SetChannelVolume( chans[ i ], vol );
    }
}

PF_CONSOLE_CMD(Audio, SetChannelVolume, "string channel, float percentage", "Sets the master volume of a given audio channel\n\
Valid channels are: SoundFX, BgndMusic, Voice, GUI, NPCVoice and Ambience.")
{
    plgAudioSys::ASChannel  chan;

    const ST::string& channelName = params[0];
    if (channelName.compare_i("SoundFX") == 0)
        chan = plgAudioSys::kSoundFX;
    else if (channelName.compare_i("BgndMusic") == 0)
        chan = plgAudioSys::kBgndMusic;
    else if (channelName.compare_i("Voice") == 0)
        chan = plgAudioSys::kVoice;
    else if (channelName.compare_i("Ambience") == 0)
        chan = plgAudioSys::kAmbience;
    else if (channelName.compare_i("GUI") == 0)
        chan = plgAudioSys::kGUI;
    else if (channelName.compare_i("NPCVoice") == 0)
        chan = plgAudioSys::kNPCVoice;
    else
    {
        PrintString( "Invalid channel specified. Use SoundFX, BgndMusic, Voice, Ambience or GUI." );
        return;
    }

    float    vol = (float)(float)params[ 1 ];
    if( vol > 1.f )
        vol = 1.f;
    else if( vol < 0.f )
        vol = 0.f;

    plgAudioSys::SetChannelVolume( chan, vol );

    ST::string msg;
    switch( chan )
    {
        case plgAudioSys::kSoundFX:     msg = ST::format("Setting SoundFX master volume to {4.2f}", vol); break;
        case plgAudioSys::kBgndMusic:   msg = ST::format("Setting BgndMusic master volume to {4.2f}", vol); break;
        case plgAudioSys::kVoice:       msg = ST::format("Setting Voice master volume to {4.2f}", vol); break;
        case plgAudioSys::kAmbience:    msg = ST::format("Setting Ambience master volume to {4.2f}", vol); break;
        case plgAudioSys::kGUI:         msg = ST::format("Setting GUI master volume to {4.2f}", vol); break;
        case plgAudioSys::kNPCVoice:    msg = ST::format("Setting NPC Voice master volume to {4.2f}", vol); break;
        default: break;
    }
    PrintString(msg);
}

PF_CONSOLE_CMD(Audio, ShowNumActiveBuffers, "bool b", "Shows the number of Direct sounds buffers in use")
{
    plgAudioSys::ShowNumBuffers((bool)params[0]);
}

PF_CONSOLE_CMD(Audio, SetDeviceName, "string deviceName", "Meant for plClient init only")
{
    plgAudioSys::SetPlaybackDevice(params[0]);
}

PF_CONSOLE_CMD(Audio, SetCaptureDeviceName, "string deviceName", "Sets the audio capture device name")
{
    plgAudioSys::SetCaptureDevice(params[0]);
}

PF_CONSOLE_CMD(Audio, ShowIcons, "bool b", "turn voice recording icons on and off")
{
    bool b = params[0];
    plVoiceRecorder::EnableIcons(b);

}

PF_CONSOLE_CMD(Audio, SquelchLevel, "float f", "Set the squelch level")
{
    float f = params[0];
    plVoiceRecorder::SetSquelch(f);

}


PF_CONSOLE_CMD(Audio, PushToTalk, "bool b", "turn push-to-talk on or off")
{
    bool b = params[0];
    plVoiceRecorder::EnablePushToTalk(b);

}

PF_CONSOLE_CMD(Audio, SetVoiceCodec, "string codec", "Sets the codec used for voice chat")
{
    const ST::string& codec = params[0];
    if (codec.compare_i("none") == 0)
        plVoiceRecorder::SetVoiceFlags(0);
    else if (codec.compare_i("speex") == 0)
        plVoiceRecorder::SetVoiceFlags(plVoiceFlags::kEncoded | plVoiceFlags::kEncodedSpeex);
    else if (codec.compare_i("opus") == 0)
        plVoiceRecorder::SetVoiceFlags(plVoiceFlags::kEncoded | plVoiceFlags::kEncodedOpus);
    else
        PrintString("Invalid codec specified");
}

PF_CONSOLE_CMD(Audio, SetVoiceSampleRate, "int rate", "Sets the voice chat sampling rate")
{
    plVoiceRecorder::SetSampleRate((int)params[0]);
}

PF_CONSOLE_CMD(Audio, SetVoiceQuality, "int q", "Set quality of voice encoding")
{
    int q = params[0];
    plVoiceRecorder::SetQuality(q);
}

PF_CONSOLE_CMD(Audio, SetVBR, "bool q", "Toggle variable bit rate")
{
    bool q = params[0];
    plVoiceRecorder::SetVBR(q);
}

PF_CONSOLE_CMD(Audio, EnableVoiceRecording, "bool b", "turn voice recording on or off")
{
    bool b = params[0];
    plVoiceRecorder::EnableRecording(b);

}

PF_CONSOLE_CMD(Audio, EnableVoiceChat, "bool b", "Enable Voice chat")
{
    plVoicePlayer::Enable((bool) params[0]);
    plVoiceRecorder::EnableRecording((bool) params[0]);
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, ShowVoiceGraph, "bool b", "Show voice chat graph")
{
    plVoiceRecorder::ShowGraph((bool)params[0]);
}

PF_CONSOLE_CMD(Audio, NextDebugPlate, "", "Cycles through the volume displays for all registered sounds")
{
    plgAudioSys::NextDebugSound();
}

PF_CONSOLE_CMD(Audio, ShowDebugPlate, "string object, int soundIdx", "Shows the volume display for a registered sound")
{
    ST::string status;
    plKey key = FindSceneObjectByName(params[0], {}, status);
    if (!key) {
        PrintString(status);
        plSound::SetCurrDebugPlate(nullptr);
        return;
    }

    plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
    if (!so) {
        PrintString("Invalid SceneObject");
        return;
    }

    const plAudioInterface* ai = so->GetAudioInterface();
    if (ai) {
        plSound* sound = ai->GetSound(params[1]);
        // sue me
        plSound::SetCurrDebugPlate(sound->GetKey());
    } else {
        PrintString("SceneObject has no AudioInterface");
    }
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, SetLoadOnDemand, "bool on", "Enable or disable load-on-demand for sounds")
{
    plSound::SetLoadOnDemand( (bool)params[ 0 ] );
}

PF_CONSOLE_CMD(Audio, SetTwoStageLOD, "bool on", "Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers. Less of a performance hit, harder on memory.")
{
    // For two-stage LOD, we want to disable LoadFromDiskOnDemand, so that we'll load into RAM at startup but not
    // into sound buffers until demanded to do so. Enabling LoadFromDiskOnDemand basically conserves as much memory
    // as possible
    plSound::SetLoadFromDiskOnDemand( !(bool)params[ 0 ] );
}

PF_CONSOLE_CMD(Audio, SetVolume, "string obj, float vol", "Sets the volume on a given object. 1 is max volume, 0 is silence" )
{
    ST::string status;
    plKey key = FindSceneObjectByName(params[0], {}, status);
    if (key == nullptr) {
        PrintString(status);
        return;
    }

    plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
    if( !obj )
        return;

    plSoundMsg* cmd = new plSoundMsg;
    cmd->SetSender(hsgResMgr::ResMgr()->FindKey(kClient_KEY));
    cmd->SetCmd( plSoundMsg::kSetVolume );
    cmd->fVolume = params[ 1 ];
    cmd->AddReceiver( key );
    plgDispatch::MsgSend(cmd);
}


#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, IsolateSound, "string soundComponentName", "Mutes all sounds except the given sound. Use Audio.MuteAll false to remove the isolation.")
{
    const ST::string& soundComponentName = params[0];
    plKey           key;
    plAudioSysMsg   *asMsg;

    ST::string status;
    key = FindSceneObjectByName(soundComponentName, {}, status);
    if (key == nullptr)
    {
        PrintString(ST::format("Cannot find sound {}: {}", soundComponentName, status));
        return;
    }

    plSceneObject *obj = plSceneObject::ConvertNoRef( key->GetObjectPtr() );
    if( !obj )
    {
        PrintString(ST::format("Cannot get sceneObject {}", soundComponentName));
        return;
    }

    const plAudioInterface  *ai = obj->GetAudioInterface();
    if (ai == nullptr)
    {
        PrintString(ST::format("sceneObject {} has no audio interface", soundComponentName));
        return;
    }

    asMsg = new plAudioSysMsg( plAudioSysMsg::kMuteAll );
    plgDispatch::MsgSend( asMsg );

    asMsg = new plAudioSysMsg( plAudioSysMsg::kUnmuteAll );
    asMsg->AddReceiver( ai->GetKey() );
    asMsg->SetBCastFlag( plMessage::kBCastByExactType, false );
    plgDispatch::MsgSend( asMsg );


    PrintString(ST::format("Sounds on sceneObject {} isolated.", soundComponentName));
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, SetMicVolume, "float volume", "Sets the microphone volume, in the range of 0 to 1")
{
    if (!plgAudioSys::SetCaptureVolume((float)params[0])) {
        PrintString("Unable to set microphone level");
    }
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, MCNTest, "int which", "")
{
    if( (int)params[ 0 ] == 0 )
        plgAudioSys::ClearDebugFlags();
    else if( (int)params[ 0 ] == 1 )
        plgAudioSys::SetDebugFlag( plgAudioSys::kDisableRightSelect );
    else if( (int)params[ 0 ] == 2 )
        plgAudioSys::SetDebugFlag( plgAudioSys::kDisableLeftSelect );
}

PF_CONSOLE_CMD(Audio, Mark, "", "")
{
    static int markNum = 0;
    plStatusLog::AddLineSF( "threadfun.log", "******* Mark #{} *******", markNum++ );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Audio, SetStreamingBufferSize, "float sizeInSecs", "Sets the size of the streaming buffer for each streaming sound.")
{
    plgAudioSys::SetStreamingBufferSize( (float)params[ 0 ] );
    PrintString( "Changes won't take effect until you restart the audio system." );
}

PF_CONSOLE_CMD(Audio, SetStreamFromRAMCutoff, "float cutoffInSecs", "Sets the cutoff between streaming from RAM and streaming directly from disk.")
{
    plgAudioSys::SetStreamFromRAMCutoff( (float)params[ 0 ] );
    PrintString( "Changes won't take effect until you restart the audio system." );
}

PF_CONSOLE_CMD(Audio, SetPriorityCutoff, "int cutoff", "Stops sounds from loading whose priority is greater than this cutoff.")
{
    plgAudioSys::SetPriorityCutoff( (int)params[ 0 ] );
}

PF_CONSOLE_CMD(Audio, EnableExtendedLogs, "bool enable", "Enables or disables the extended audio logs.")
{
    plgAudioSys::EnableExtendedLogs( (bool)params[ 0 ] );
}



////////////////////////////////////////////////////////////////////////
//// Listener System Group Commands ////////////////////////////////////
////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP(Listener)

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Listener, ShowDebugInfo, "bool show", "Shows or hides debugging info")
{
    plListener::ShowDebugInfo( (bool)params[ 0 ] );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(Listener, UseCameraOrientation, "", "Use the camera's orientation to orient the listener")
{
    plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nullptr, true);
    set->Send();
}

PF_CONSOLE_CMD(Listener, UseCameraPosition, "", "Use the canera's position to position the listener")
{
    plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nullptr, true);
    set->Send();
}

PF_CONSOLE_CMD(Listener, UseCameraVelocity, "", "Use the camera's velocity to set the listener velocity")
{
    plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kVelocity, nullptr, true);
    set->Send();
}

PF_CONSOLE_CMD(Listener, UsePlayerOrientation, "", "Use the player's orientation to orient the listener")
{
    plKey pKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    if( pKey )
    {
        plSetListenerMsg *set = new plSetListenerMsg( plSetListenerMsg::kFacing, pKey, true );
        set->Send();
    }
}

PF_CONSOLE_CMD(Listener, UsePlayerPosition, "", "Use the player's position to position the listener")
{
    plKey pKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    if (pKey)
    {
        plSetListenerMsg *set = new plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
        set->Send();
    }
}

PF_CONSOLE_CMD(Listener, UsePlayerVelocity, "", "Use the player's velocity to set the listener velocity")
{
    plKey pKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    if (pKey)
    {
        plSetListenerMsg *set = new plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
        set->Send();
    }
}

PF_CONSOLE_CMD(Listener, XMode, "bool b", "Sets velocity and position to avatar, and orientation to camera")
{
    static uint32_t oldPosType = 0, oldFacingType = 0, oldVelType = 0;
    
    plSetListenerMsg *set = nullptr;
    plKey pKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    plListener* pListener = nullptr;

    if( (bool)params[ 0 ] )
    {
        // Get the listener object
        plUoid lu(kListenerMod_KEY);
        plKey pLKey = hsgResMgr::ResMgr()->FindKey(lu);
        if (pLKey)
        {   
            pListener = plListener::ConvertNoRef(pLKey->GetObjectPtr());
        }

        if(pListener)
        {
            // Save old types
            oldPosType = pListener->GetAttachedPosType();
            oldFacingType = pListener->GetAttachedFacingType();
            oldVelType = pListener->GetAttachedVelType();
        }
        
        plStatusLog::AddLineS("audio.log", "XMode on");
        
        plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nullptr, true);
        set->Send();
        if (pKey)
        {
            set = new plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
            set->Send();
            set = new plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
            set->Send();
        }
    }
    else 
    {
        if(oldPosType == plListener::kCamera)
        {
            plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nullptr, true);
            set->Send();
        }
        else
        {
            set = new plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
            set->Send();
        }
        if(oldFacingType == plListener::kCamera)
        {
            set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nullptr, true);
            set->Send();
        }
        else
        {
            set = new plSetListenerMsg( plSetListenerMsg::kFacing, pKey, true );
            set->Send();
        }
        if(oldVelType == plListener::kCamera)
        {
            set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kVelocity, nullptr, true);
            set->Send();
        }
        else
        {
            set = new plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
            set->Send();
        }
        plStatusLog::AddLineSF("audio.log", "XMode off, {}, {}, {}", oldPosType, oldFacingType, oldVelType);
    }
}
