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
#ifndef _pyAutoControl_h_
#define _pyAutoControl_h_

//////////////////////////////////////////////////////////////////////
//
// pyAudioControl   - a wrapper class all the audio control functions
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pyGlueHelpers.h"

class pyAudioControl
{
protected:
    pyAudioControl() {};

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptAudioControl);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAudioControl object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAudioControl); // converts a PyObject to a pyAudioControl (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    //-----------------------
    // Audio settings

    // Sets the master volume of a given audio channel
    virtual void SetSoundFXVolume( float volume );
    virtual void SetMusicVolume( float volume );
    virtual void SetVoiceVolume( float volume );
    virtual void SetAmbienceVolume( float volume );
    virtual void SetGUIVolume( float volume );
    virtual void SetNPCVoiceVolume( float volume );
    virtual float GetSoundFXVolume();
    virtual float GetMusicVolume();
    virtual float GetVoiceVolume();
    virtual float GetAmbienceVolume();
    virtual float GetGUIVolume();
    virtual float GetNPCVoiceVolume();

    // Switch DirectX Audio on or off at runtime
    virtual void Enable();
    virtual void Disable();
    virtual hsBool IsEnabled();

    // Enable or disable load-on-demand for sounds
    virtual void SetLoadOnDemand( hsBool state );

    // Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
    // ...Less of a performance hit, harder on memory.
    virtual void SetTwoStageLOD( hsBool state );

    // Enable audio hardware acceleration
    virtual void UseHardwareAcceleration( hsBool state );
    virtual hsBool IsHardwareAccelerated();

    // Enable EAX sound acceleration (requires hardware acceleration)
    virtual void UseEAXAcceleration( hsBool state );
    virtual hsBool IsUsingEAXAcceleration();

    // Mute or unmute all sounds
    virtual void MuteAll();
    virtual void UnmuteAll();
    virtual hsBool IsMuted();

    virtual void SetAudioSystemMode(int mode);  // sets the current mode
    virtual int  GetAudioSystemMode();          // returns the current mode
    virtual int  GetHighestAudioMode();         // returns the highest mode the card is capable of handling
    virtual int GetNumAudioDevices();
    virtual const char *GetAudioDeviceName(int index);
    virtual void SetDeviceName(const char *device, bool restart);
    virtual const char *GetDeviceName();



    //------------------------
    // Voice Settings

    // Sets the microphone volume, in the range of 0 to 1
    virtual hsBool CanSetMicLevel();
    virtual void SetMicLevel( float level );
    virtual float GetMicLevel();

    // turn voice recording on or off
    virtual void EnableVoiceRecording( hsBool state );
    virtual hsBool IsVoiceRecordingEnabled();

    // turn voice compression on and off
    virtual void EnableVoiceCompression( hsBool state );
    virtual hsBool IsVoiceCompressionEnabled();

    // turn voice-over-net on and off
    virtual void EnableVoiceNetBroadcast( hsBool state );
    virtual hsBool IsVoiceNetBroadcastEnabled();

    void EnableVoiceChat(hsBool enable);

    // turn voice recording icons on and off
    virtual void ShowIcons();
    virtual void HideIcons();

    // turn push-to-talk on or off
    virtual void PushToTalk( hsBool state );

    // Set the squelch level
    virtual void SquelchLevel( float level );

    // Adjust voice packet frame size
    virtual void RecordFrame( int32_t size );

    // Set the sample rate for recording
    virtual void RecordSampleRate( int32_t sample_rate );

    virtual uint8_t GetPriorityCutoff( void );
    virtual void  SetPriorityCutoff( uint8_t cut );

    // does the device specified support EAX
    virtual hsBool SupportEAX(const char *deviceName);

};

#endif // _pyAudioControl_h_
