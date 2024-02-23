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
#include "pyGlueDefinitions.h"
#include <string_theory/string>
#include <vector>

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
    void SetSoundFXVolume(float volume);
    void SetMusicVolume(float volume);
    void SetVoiceVolume(float volume);
    void SetAmbienceVolume(float volume);
    void SetGUIVolume(float volume);
    void SetNPCVoiceVolume(float volume);
    float GetSoundFXVolume() const;
    float GetMusicVolume() const;
    float GetVoiceVolume() const;
    float GetAmbienceVolume() const;
    float GetGUIVolume() const;
    float GetNPCVoiceVolume() const;

    // Switch audio on or off at runtime
    void Enable();
    void Disable();
    bool IsEnabled() const;

    // Enable or disable load-on-demand for sounds
    void SetLoadOnDemand(bool state);

    // Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
    // ...Less of a performance hit, harder on memory.
    void SetTwoStageLOD(bool state);

    // Enable EAX sound acceleration
    void UseEAXAcceleration(bool state);
    bool IsUsingEAXAcceleration() const;
    bool IsEAXSupported() const;

    // Mute or unmute all sounds
    void MuteAll();
    void UnmuteAll();
    bool IsMuted() const;

    // Enable or disable displaying speech subtitles
    void EnableSubtitles();
    void DisableSubtitles();
    bool AreSubtitlesEnabled() const;

    void SetPlaybackDevice(const ST::string& device, bool restart);
    ST::string GetPlaybackDevice() const;

    std::vector<ST::string> GetPlaybackDevices() const;

    ST::string GetFriendlyDeviceName(const ST::string& deviceName) const;


    //------------------------

    // Voice Settings

    // Sets the microphone volume, in the range of 0 to 1
    bool CanSetMicLevel() const;
    void SetMicLevel(float level);
    float GetMicLevel() const;

    // turn voice recording on or off
    void EnableVoiceRecording(bool state);
    bool IsVoiceRecordingEnabled() const;

    void EnableVoiceChat(bool enable);

    // turn voice recording icons on and off
    void ShowIcons();
    void HideIcons();

    // turn push-to-talk on or off
    void PushToTalk(bool state);

    // Set the squelch level
    void SquelchLevel(float level);

    uint8_t GetPriorityCutoff() const;
    void  SetPriorityCutoff(uint8_t cut);

    void SetCaptureDevice(const ST::string& device);
    ST::string GetCaptureDevice() const;

    std::vector<ST::string> GetCaptureDevices() const;
};

#endif // _pyAudioControl_h_
