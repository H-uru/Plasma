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

#ifndef _PLAUDIO_PLAUDIOSYSTEMPRIVATE_H
#define _PLAUDIO_PLAUDIOSYSTEMPRIVATE_H

#include "HeadSpin.h"
#include <al.h>
#include <alc.h>
#ifdef USE_EFX
#   include <efx.h>
#else
#if __APPLE__
#   include <OpenAL/OpenAL.h>
#endif
#endif
#ifdef EAX_SDK_AVAILABLE
#   include <eax.h>
#endif
#include <memory>
#include <set>

#include "hsGeometry3.h"
#include "pnKeyedObject/hsKeyedObject.h"

class plAudioEndpointVolume;
class plEAXListenerMod;
class plSoftSoundNode;
class plStatusLog;

class plAudioSystem : public hsKeyedObject
{
public:
    plAudioSystem();

    CLASSNAME_REGISTER(plAudioSystem);
    GETINTERFACE_ANY(plAudioSystem, hsKeyedObject);

    bool Init();
    void Shutdown();

    void SetActive(bool b);

    void SetListenerPos(const hsPoint3& pos);
    void SetListenerVelocity(const hsVector3& vel);
    void SetListenerOrientation(const hsVector3& view, const hsVector3& up);
    void SetMaxNumberOfActiveSounds();
    void SetDistanceModel(int i);

    bool MsgReceive(plMessage* msg) override;

    void NextDebugSound(void);

    std::vector<ST::string> GetPlaybackDevices() const;
    ST::string GetDefaultPlaybackDevice() const;
    std::vector<ST::string> GetCaptureDevices() const;
    ST::string GetDefaultCaptureDevice() const;

    bool IsEAXSupported() const { return fEAXSupported; }

    void SetFadeLength(float lengthSec);

    bool BeginCapture();
    bool CaptureSamples(uint32_t samples, int16_t* data) const;
    uint32_t GetCaptureSampleCount() const;
    bool IsCapturing() const { return fCaptureDevice != nullptr; }
    bool EndCapture();

protected:

    friend class plgAudioSys;

    ALCdevice* fPlaybackDevice;
    ALCcontext* fContext;
    ALCdevice* fCaptureDevice;
    std::unique_ptr<plAudioEndpointVolume> fCaptureLevel;

    plSoftSoundNode* fSoftRegionSounds;
    plSoftSoundNode* fActiveSofts;
    plStatusLog* fDebugActiveSoundDisplay;

    static int32_t fMaxNumSounds, fNumSoundsSlop;
    plSoftSoundNode* fCurrDebugSound;

    hsPoint3 fCurrListenerPos;
    bool fActive, fUsingEAX, fRestartOnDestruct, fWaitingForShutdown;
    int64_t fStartTime;

    std::set<plEAXListenerMod*> fEAXRegions;

    hsPoint3 fLastPos;

    bool fDisplayNumBuffers;
    bool fListenerInit;

    double fStartFade;
    float fFadeLength;
    unsigned int fMaxNumSources;
    bool fEAXSupported;
    double fLastUpdateTimeMs;

    bool OpenCaptureDevice();
    bool RestartCapture();

    void RegisterSoftSound(const plKey& soundKey);
    void UnregisterSoftSound(const plKey& soundKey);
    void IUpdateSoftSounds(const hsPoint3& newPosition);
};

#endif
