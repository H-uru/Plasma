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
#include "hsStlUtils.h"
#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "pnKeyedObject/hsKeyedObject.h"

#define DEFAULT_AUDIO_DEVICE_NAME "Generic Software"

typedef wchar_t WCHAR;

class plSound;
class plSoftSoundNode;
class plgAudioSys;
class plStatusLog;
class plEAXListenerMod;

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;


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

    bool    Init(hsWindowHndl hWnd);
    void    Shutdown();

    void    SetActive( bool b );
    
    void SetListenerPos(const hsPoint3 pos);
    void SetListenerVelocity(const hsVector3 vel);
    void SetListenerOrientation(const hsVector3 view, const hsVector3 up);
    void SetMaxNumberOfActiveSounds();      // sets the max number of active sounds based on the priority cutoff
    void SetDistanceModel(int i);
    
    virtual bool MsgReceive(plMessage* msg);
    double GetTime();
    
    void        NextDebugSound( void );
    hsPoint3    GetCurrListenerPos( void ) const { return fCurrListenerPos; }

    int         GetNumAudioDevices();
    const char *GetAudioDeviceName(int index);
    bool        SupportsEAX(const char *deviceName);

    void        SetFadeLength(float lengthSec);
    
protected:

    friend class plgAudioSys;

    ALCdevice *     fDevice;
    ALCcontext *    fContext;
    ALCdevice *     fCaptureDevice;
    
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
    
    std::vector<DeviceDescriptor> fDeviceList;      // list of openal device names

    double          fStartFade;
    float           fFadeLength;
    unsigned int    fMaxNumSources;     // max openal sources
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

    enum AudioMode
    {
        kDisabled,
        kSoftware,
        kHardware,
        kHardwarePlusEAX,
    };
    static void Init(hsWindowHndl hWnd);
    static bool Hardware() { return fUseHardware; }
    static void SetUseHardware(bool b);
    static void SetActive(bool b);
    static void SetMuted( bool b );
    static void EnableEAX( bool b );
    static bool Active() { return fInit; }
    static void Shutdown();
    static void Activate(bool b);
    static bool     IsMuted( void ) { return fMuted; }
    static hsWindowHndl hWnd() { return fWnd; }
    static plAudioSystem* Sys() { return fSys; }
    static void Restart( void );
    static bool     UsingEAX( void ) { return fSys->fUsingEAX; }

    static void NextDebugSound( void );

    static void     SetChannelVolume( ASChannel chan, float vol );
    static float GetChannelVolume( ASChannel chan );

    static void     Set2D3DBias( float bias );
    static float Get2D3Dbias();

    static void     SetGlobalFadeVolume( float vol );
    static float GetGlobalFadeVolume( void ) { return fGlobalFadeVolume; }

    static void     SetDebugFlag( uint32_t flag, bool set = true ) { if( set ) fDebugFlags |= flag; else fDebugFlags &= ~flag; }
    static bool     IsDebugFlagSet( uint32_t flag ) { return fDebugFlags & flag; }
    static void     ClearDebugFlags( void ) { fDebugFlags = 0; }

    static float GetStreamingBufferSize( void ) { return fStreamingBufferSize; }
    static void     SetStreamingBufferSize( float size ) { fStreamingBufferSize = size; }

    static uint8_t    GetPriorityCutoff( void ) { return fPriorityCutoff; }
    static void     SetPriorityCutoff( uint8_t cut ) { fPriorityCutoff = cut;  if(fSys) fSys->SetMaxNumberOfActiveSounds(); }

    static bool     AreExtendedLogsEnabled( void ) { return fEnableExtendedLogs; }
    static void     EnableExtendedLogs( bool e ) { fEnableExtendedLogs = e; }

    static float GetStreamFromRAMCutoff( void ) { return fStreamFromRAMCutoff; }
    static void     SetStreamFromRAMCutoff( float c ) { fStreamFromRAMCutoff = c; }

    static void SetListenerPos(const hsPoint3 pos);
    static void SetListenerVelocity(const hsVector3 vel);
    static void SetListenerOrientation(const hsVector3 view, const hsVector3 up);

    static void ShowNumBuffers(bool b) { if(fSys) fSys->fDisplayNumBuffers = b; }

    static void SetAudioMode(AudioMode mode);
    static int GetAudioMode();
    static bool LogStreamingUpdates() { return fLogStreamingUpdates; }
    static void SetLogStreamingUpdates(bool logUpdates) { fLogStreamingUpdates = logUpdates; }
    static void SetDeviceName(const char *device, bool restart = false);
    static const char *GetDeviceName() { return fDeviceName.c_str(); }
    static int GetNumAudioDevices();
    static const char *GetAudioDeviceName(int index);
    static ALCdevice *GetCaptureDevice();
    static bool SupportsEAX(const char *deviceName);
    static void RegisterSoftSound( const plKey soundKey );
    static void UnregisterSoftSound( const plKey soundKey );

    static bool IsRestarting() {return fRestarting;}

private:
    friend class plAudioSystem;

    static plAudioSystem*       fSys;
    static bool                 fInit;
    static bool                 fActive;
    static bool                 fMuted;
    static hsWindowHndl         fWnd;
    static bool                 fUseHardware;
    static bool                 fDelayedActivate;
    static float             fChannelVolumes[ kNumChannels ];
    static float             fGlobalFadeVolume;
    static uint32_t               fDebugFlags;
    static bool                 fEnableEAX;
    static float             fStreamingBufferSize;
    static uint8_t                fPriorityCutoff;
    static bool                 fEnableExtendedLogs;
    static float             fStreamFromRAMCutoff;
    static float             f2D3DBias;
    static bool                 fLogStreamingUpdates;
    static std::string          fDeviceName;
    static bool                 fRestarting;
    static bool                 fMutedStateChange;

};

#endif //plAudioSystem_h
