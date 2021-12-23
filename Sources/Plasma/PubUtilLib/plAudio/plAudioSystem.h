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
#ifndef plAudioSystem_h
#define plAudioSystem_h

#include <string_theory/string>
#include <vector>

// VC++ doesn't like these being forward-declared here
#include "hsGeometry3.h"

class plgAudioSys;
class plEAXListenerMod;
class plKey;
class plSound;
class plSoftSoundNode;
class plStatusLog;

class plgAudioSys
{
public:
    enum ASChannel
    {
        kSoundFX,
        kAmbience,
        kBgndMusic,
        kGUI,
        kNPCVoice,
        kVoice,
        kNumChannels
    };

    enum DebugFlags
    {
        kDisableRightSelect = 0x00000001,
        kDisableLeftSelect  = 0x00000002
    };

    static void Init();
    static void SetActive(bool b);
    static void SetMuted(bool b);
    static void SetEnableSubtitles(bool b);
    static void EnableEAX(bool b);
    static bool Active() { return fInit; }
    static void Shutdown();
    static void Activate(bool b);
    static bool IsMuted() { return fMuted; }
    static bool IsEnabledSubtitles() { return fEnableSubtitles; }
    static void Restart();
    static bool UsingEAX();

    /** Does the current playback device support EAX? */
    static bool IsEAXSupported();

    static void NextDebugSound();

    static void  SetChannelVolume(ASChannel chan, float vol);
    static float GetChannelVolume(ASChannel chan);

    static void  SetGlobalFadeVolume(float vol);
    static float GetGlobalFadeVolume() { return fGlobalFadeVolume; }

    static void  SetDebugFlag(uint32_t flag, bool set = true) { if (set) fDebugFlags |= flag; else fDebugFlags &= ~flag; }
    static bool  IsDebugFlagSet(uint32_t flag) { return fDebugFlags & flag; }
    static void  ClearDebugFlags() { fDebugFlags = 0; }

    static float GetStreamingBufferSize() { return fStreamingBufferSize; }
    static void  SetStreamingBufferSize(float size) { fStreamingBufferSize = size; }

    static uint8_t  GetPriorityCutoff() { return fPriorityCutoff; }
    static void     SetPriorityCutoff(uint8_t cut);

    static bool     AreExtendedLogsEnabled() { return fEnableExtendedLogs; }
    static void     EnableExtendedLogs(bool e) { fEnableExtendedLogs = e; }

    static float    GetStreamFromRAMCutoff() { return fStreamFromRAMCutoff; }
    static void     SetStreamFromRAMCutoff(float c) { fStreamFromRAMCutoff = c; }

    static hsPoint3 GetCurrListenerPos();
    static void SetListenerPos(const hsPoint3& pos);
    static void SetListenerVelocity(const hsVector3& vel);
    static void SetListenerOrientation(const hsVector3& view, const hsVector3& up);

    static void ShowNumBuffers(bool b);

    static bool LogStreamingUpdates() { return fLogStreamingUpdates; }
    static void SetLogStreamingUpdates(bool logUpdates) { fLogStreamingUpdates = logUpdates; }
    static void RegisterSoftSound(const plKey& soundKey);
    static void UnregisterSoftSound(const plKey& soundKey);

    static void SetDistanceModel(int type);

    /** Returns the device name without any OpenAL device name prefixes applied. */
    static ST::string GetFriendlyDeviceName(const ST::string& deviceName);

    static ST::string GetPlaybackDevice() { return fPlaybackDeviceName; }

    /**
     * \brief Gets a vector of all available playback devices.
     * This returns a vector of all playback devices available to OpenAL. If the enumerate all
     * extension is available in the OpenAL implementation, it should include all audio endpoints
     * on the system. Otherwise, the standard wrapper "devices" will be returned for the default
     * endpoint.
     */
    static std::vector<ST::string> GetPlaybackDevices();

    /**
     * \brief Gets the name of the default playback device.
     * This returns the string name of the system's default audio playback device. If the enumerate
     * all extension is available in the OpenAL implementation, this can be any of the audio endpoints
     * on the system. Otherwise, the standard wrapper "device" will be returned for the default
     * system audio endpoint.
     */
    static ST::string GetDefaultPlaybackDevice();

    static void SetPlaybackDevice(const ST::string& name, bool restart = false)
    {
        fPlaybackDeviceName = name;
        if (restart)
            Restart();
    }

    /** Gets a vector of all available audio capture devices. */
    static std::vector<ST::string> GetCaptureDevices();

    /** Gets the name of the default audio capture device. */
    static ST::string GetDefaultCaptureDevice();

    /** Gets the internal device name of the current audio capture device. */
    static ST::string GetCaptureDevice() { return fCaptureDeviceName; }

    /** Gets the friendly user facing name of the current audio capture device. */
    static ST::string GetCaptureDeviceFriendly() { return GetFriendlyDeviceName(fCaptureDeviceName); }

    static void SetCaptureDevice(const ST::string& name);

    static bool SetCaptureSampleRate(uint32_t frequency);

    static bool IsRestarting() { return fRestarting; }

    /**
     * \brief Begin capturing audio samples.
     * This opens the selected capture device and begins sampling audio at the requested rate.
     */
    static bool BeginCapture();

    /**
     * \brief Captures audio samples from the selected capture device.
     * \desc This copies the number of audio samples requested into the provided buffer from the
     *       capture device's buffer. The provided buffer must have enough space to consume the
     *       requested number of samples.
     */
    static bool CaptureSamples(uint32_t samples, int16_t* data);

    /** Returns the number of unconsumed audio samples from the selected capture device. */
    static uint32_t GetCaptureSampleCount();
    static bool IsCapturing();
    static bool EndCapture();

    /** Returns if the endpoint's volume can be manipulated. */
    static bool CanChangeCaptureVolume();

    /**
     * Gets the volume of this audio endpoint.
     * This gets the volume of the given audio endpoint as a percentage from 0.0 to 1.0, inclusive.
     */
    static float GetCaptureVolume();

    /**
     * Sets the volume of this audio endpoint.
     * This sets the volume of the given audio endpoint as a percentage from 0.0 to 1.0, inclusive.
     */
    static bool SetCaptureVolume(float pct);

private:
    friend class plAudioSystem;

    static class plAudioSystem* fSys;
    static bool                 fInit;
    static bool                 fActive;
    static bool                 fMuted;
    static bool                 fEnableSubtitles;
    static bool                 fDelayedActivate;
    static float                fChannelVolumes[kNumChannels];
    static float                fGlobalFadeVolume;
    static uint32_t             fDebugFlags;
    static bool                 fEnableEAX;
    static float                fStreamingBufferSize;
    static uint8_t              fPriorityCutoff;
    static bool                 fEnableExtendedLogs;
    static float                fStreamFromRAMCutoff;
    static float                f2D3DBias;
    static bool                 fLogStreamingUpdates;
    static ST::string           fPlaybackDeviceName;
    static ST::string           fCaptureDeviceName;
    static bool                 fRestarting;
    static bool                 fMutedStateChange;
    static uint32_t             fCaptureSampleRate;
    static bool                 fDisplayNumBuffers;

};

#endif //plAudioSystem_h
