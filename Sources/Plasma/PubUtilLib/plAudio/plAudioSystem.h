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

#include "HeadSpin.h"
#include <string>
#include <alc.h>

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "pnKeyedObject/hsKeyedObject.h"

class plSound;
class plSoftSoundNode;
class plgAudioSys;
class plStatusLog;
class plEAXListenerMod;

class DeviceDescriptor
{
public:
    DeviceDescriptor(const char *name, bool supportsEAX):
    fDeviceName(name),
    fSupportsEAX(supportsEAX)
    {
    }
    const char *GetDeviceName() { return fDeviceName.c_str();}
    bool SupportsEAX() { return fSupportsEAX; }

private:
    std::string fDeviceName;
    bool fSupportsEAX;
};

class plAudioSystem : public hsKeyedObject
{
public:
    plAudioSystem();
    ~plAudioSystem();

    CLASSNAME_REGISTER( plAudioSystem );
    GETINTERFACE_ANY( plAudioSystem, hsKeyedObject );

    enum
    {
        kThreadSndRef = 0,
        kRefEAXRegion
    };

    bool    Init();
    void    Shutdown();

    void    SetActive( bool b );

    void SetListenerPos(const hsPoint3 pos);
    void SetListenerVelocity(const hsVector3 vel);
    void SetListenerOrientation(const hsVector3 view, const hsVector3 up);
    void SetMaxNumberOfActiveSounds();      // sets the max number of active sounds based on the priority cutoff
    void SetDistanceModel(int i);

    virtual bool MsgReceive(plMessage* msg);
    double GetTime();

    void        NextDebugSound();
    hsPoint3    GetCurrListenerPos() const { return fCurrListenerPos; }

    /**
     * \brief Gets a vector of all available playback devices.
     * This returns a vector of all playback devices available to OpenAL. If the enumerate all
     * extension is available in the OpenAL implementation, it should include all audio endpoints
     * on the system. Otherwise, the standard wrapper "devices" will be returned for the default
     * endpoint.
     */
    std::vector<ST::string> GetPlaybackDevices() const;

    /**
     * \brief Gets the name of the default playback device.
     * This returns the string name of the system's default audio playback device. If the enumerate
     * all extension is available in the OpenAL implementation, this can be any of the audio endpoints
     * on the system. Otherwise, the standard wrapper "device" will be returned for the default
     * system audio endpoint.
     */
    ST::string GetDefaultPlaybackDevice() const;

    /** Does the current playback device support EAX? */
    bool IsEAXSupported() const { return fEAXSupported; }

    void SetFadeLength(float lengthSec);

    /**
     * \brief Begin capturing audio samples.
     * This opens the selected capture device and begins sampling audio at the requested rate.
     */
    bool BeginCapture();
    bool CaptureSamples(uint32_t samples, int16_t* data) const;
    uint32_t GetCaptureSampleCount() const;
    bool SetCaptureSampleRate(uint32_t sampleRate);
    bool EndCapture();

protected:

    friend class plgAudioSys;

    ALCdevice*     fPlaybackDevice;
    ALCcontext*    fContext;
    ALCdevice*     fCaptureDevice;
    uint32_t       fCaptureFrequency;

    plSoftSoundNode     *fSoftRegionSounds;
    plSoftSoundNode     *fActiveSofts;
    plStatusLog         *fDebugActiveSoundDisplay;

    static int32_t        fMaxNumSounds, fNumSoundsSlop;      // max number of sounds the engine is allowed to audibly play. Different than fMaxNumSources. That is the max number of sounds the audio card can play
    plSoftSoundNode     *fCurrDebugSound;
    hsTArray<plKey>     fPendingRegisters;

    hsPoint3    fCurrListenerPos;//, fCommittedListenerPos;
    bool        fActive, fUsingEAX, fRestartOnDestruct, fWaitingForShutdown;
    int64_t     fStartTime;

    hsTArray<hsKeyedObject *>       fMyRefs;
    hsTArray<plEAXListenerMod *>    fEAXRegions;

    hsPoint3            fLastPos;
    bool                fAvatarPosSet;      // used for listener stuff

    bool                fDisplayNumBuffers;

    double          fStartFade;
    float           fFadeLength;
    unsigned int    fMaxNumSources;     // max openal sources
    bool            fEAXSupported;
    double          fLastUpdateTimeMs;

    void    RegisterSoftSound( const plKey soundKey );
    void    UnregisterSoftSound( const plKey soundKey );
    void    IUpdateSoftSounds( const hsPoint3 &newPosition );
    uint32_t  IScaleVolume(float volume);
    void    IEnumerateDevices();

public:
    bool                            fListenerInit;
};

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
    static void SetMuted( bool b );
    static void EnableEAX( bool b );
    static bool Active() { return fInit; }
    static void Shutdown();
    static void Activate(bool b);
    static bool     IsMuted() { return fMuted; }
    static plAudioSystem* Sys() { return fSys; }
    static void Restart();
    static bool     UsingEAX() { return fSys->fUsingEAX; }

    static void NextDebugSound();

    static void     SetChannelVolume( ASChannel chan, float vol );
    static float GetChannelVolume( ASChannel chan );

    static void     SetGlobalFadeVolume( float vol );
    static float GetGlobalFadeVolume() { return fGlobalFadeVolume; }

    static void     SetDebugFlag( uint32_t flag, bool set = true ) { if( set ) fDebugFlags |= flag; else fDebugFlags &= ~flag; }
    static bool     IsDebugFlagSet( uint32_t flag ) { return fDebugFlags & flag; }
    static void     ClearDebugFlags() { fDebugFlags = 0; }

    static float GetStreamingBufferSize() { return fStreamingBufferSize; }
    static void     SetStreamingBufferSize( float size ) { fStreamingBufferSize = size; }

    static uint8_t    GetPriorityCutoff() { return fPriorityCutoff; }
    static void     SetPriorityCutoff( uint8_t cut ) { fPriorityCutoff = cut;  if(fSys) fSys->SetMaxNumberOfActiveSounds(); }

    static bool     AreExtendedLogsEnabled() { return fEnableExtendedLogs; }
    static void     EnableExtendedLogs( bool e ) { fEnableExtendedLogs = e; }

    static float GetStreamFromRAMCutoff() { return fStreamFromRAMCutoff; }
    static void     SetStreamFromRAMCutoff( float c ) { fStreamFromRAMCutoff = c; }

    static void SetListenerPos(const hsPoint3 pos);
    static void SetListenerVelocity(const hsVector3 vel);
    static void SetListenerOrientation(const hsVector3 view, const hsVector3 up);

    static void ShowNumBuffers(bool b) { if(fSys) fSys->fDisplayNumBuffers = b; }

    static bool LogStreamingUpdates() { return fLogStreamingUpdates; }
    static void SetLogStreamingUpdates(bool logUpdates) { fLogStreamingUpdates = logUpdates; }
    static void RegisterSoftSound( const plKey soundKey );
    static void UnregisterSoftSound( const plKey soundKey );

    static ST::string GetPlaybackDevice() { return fPlaybackDeviceName; }

    static void SetPlaybackDevice(const ST::string& name, bool restart = false)
    {
        fPlaybackDeviceName = name;
        if (restart)
            Restart();
    }

    static bool IsRestarting() {return fRestarting;}

private:
    friend class plAudioSystem;

    static plAudioSystem*       fSys;
    static bool                 fInit;
    static bool                 fActive;
    static bool                 fMuted;
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
    static bool                 fRestarting;
    static bool                 fMutedStateChange;

};

#endif //plAudioSystem_h
