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

#include "plAudioEndpointVolume.h"
#include "plStatusLog/plStatusLog.h"

extern ST::string kDefaultDeviceMagic;

#if defined(HS_BUILD_FOR_WIN32)

#include <hsWindows.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <propsys.h>
#include <functiondiscoverykeys_devpkey.h>

class plWinCoreAudioEndpointVolume : public plAudioEndpointVolume
{
    IMMDevice* fDevice;

    IAudioEndpointVolume* GetVolumeCtrl() const;

public:
    plWinCoreAudioEndpointVolume();
    ~plWinCoreAudioEndpointVolume();

    float GetVolume() const override;
    bool SetDefaultDevice(plAudioEndpointType endpoint) override;
    bool SetDevice(plAudioEndpointType endpoint, const ST::string& deviceName) override;
    bool SetVolume(float fct) override;
    bool Supported() const override;
};

// =============================================================================

plWinCoreAudioEndpointVolume::plWinCoreAudioEndpointVolume()
    : fDevice()
{
    hsRequireCOM();
}

plWinCoreAudioEndpointVolume::~plWinCoreAudioEndpointVolume()
{
    if (fDevice)
        fDevice->Release();
}

template<typename T>
static inline void ICOMRelease(T*& ptr)
{
    if (ptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

static inline EDataFlow IGetDataFlow(plAudioEndpointType endpoint)
{
    switch (endpoint) {
    case plAudioEndpointType::kCapture:
        return eCapture;
    case plAudioEndpointType::kPlayback:
        return eRender;
    }

    return eAll;
}

IAudioEndpointVolume* plWinCoreAudioEndpointVolume::GetVolumeCtrl() const
{
    if (!fDevice)
        return nullptr;

    IAudioEndpointVolume* volume = nullptr;
    hsCOMError hr = fDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)& volume);
    if (FAILED(hr)) {
        plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                               "WinCoreAudioEndpointVolume::GetVolumeCtrl(): Failed to open volume control, {}",
                               hr);
        return nullptr;
    }

    return volume;
}

float plWinCoreAudioEndpointVolume::GetVolume() const
{
    float pct = 0.f;
    auto volume = GetVolumeCtrl();
    if (volume) {
        hsCOMError hr = volume->GetMasterVolumeLevelScalar(&pct);
        volume->Release();
        if (FAILED(hr)) {
            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                   "WinCoreAudioEndpointVolume::GetVolume(): Failed to get master level, {}",
                                   hr);
        }
    }
    return pct;
}

bool plWinCoreAudioEndpointVolume::SetDefaultDevice(plAudioEndpointType endpoint)
{
    ICOMRelease(fDevice);

    IMMDeviceEnumerator* enumerator = nullptr;
    hsCOMError hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                     __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) {
        plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                               "WinCoreAudioEndpointVolume::SetDefaultDevice(): Failed to get enumerator, {}",
                               hr);
        return false;
    }

    ERole role = (endpoint == plAudioEndpointType::kCapture) ? eCommunications : eMultimedia;
    hr = enumerator->GetDefaultAudioEndpoint(IGetDataFlow(endpoint), role, &fDevice);
    if (FAILED(hr)) {
        plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                               "WinCoreAudioEndpointVolume::SetDefaultDevice(): Failed to get default device, {}",
                               hr);
        return false;
    }

    enumerator->Release();
    return (fDevice != nullptr);
}

bool plWinCoreAudioEndpointVolume::SetDevice(plAudioEndpointType endpoint, const ST::string& deviceName)
{
    if (deviceName == kDefaultDeviceMagic)
        return SetDefaultDevice(endpoint);

    // Unset any previously bound device and prepare to bind a new one.
    ICOMRelease(fDevice);

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* devices = nullptr;
    IMMDevice* device = nullptr;
    IPropertyStore* props = nullptr;
    PROPVARIANT deviceFriendlyName{ 0 };

    do {
        // Windows Core Audio was added in Windows Vista. My hope is that on Windows XP, this will
        // just return a failure.
        hsCOMError hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                         __uuidof(IMMDeviceEnumerator), (void**)& enumerator);
        if (FAILED(hr)) {
            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                   "WinCoreAudioEndpointVolume::SetDevice(): Failed to get enumerator, {}",
                                   hr);
            break;
        }

        // Unfortunately for us, we cannot get the device name by string (I think). What we are using in
        // in OpenAL-land is called the "Friendly Name" in the Win32 Core Audio API. So, we'll need to
        // actually enumerate the devices and match. The API has support for friendly name collisions,
        // but that shouldn't happen (I hope?)
        hr = enumerator->EnumAudioEndpoints(IGetDataFlow(endpoint), DEVICE_STATE_ACTIVE, &devices);
        if (FAILED(hr)) {
            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                   "WinCoreAudioEndpointVolume::SetDevice(): Failed to get device collection, {}",
                                   hr);
            break;
        }

        UINT count = 0;
        devices->GetCount(&count);
        for (UINT i = 0; i < count; ++i) {
            // In case of error on previous run...
            PropVariantClear(&deviceFriendlyName);
            ICOMRelease(props);
            ICOMRelease(device);

            hr = devices->Item(i, &device);
            // Unlikely error
            if (FAILED(hr)) {
                plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                       "WinCoreAudioEndpointVolume::SetDevice(): Failed to open device #{}, {}",
                                       i, hr);
                continue;
            }

            hr = device->OpenPropertyStore(STGM_READ, &props);
            // Unlikely error
            if (FAILED(hr)) {
                plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                      "WinCoreAudioEndpointVolume::SetDevice(): Failed get device #{} properties, %{}",
                                      i, hr);
                continue;
            }

            hr = props->GetValue(PKEY_Device_FriendlyName, &deviceFriendlyName);
            if (FAILED(hr)) {
                plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                       "WinCoreAudioEndpointVolume::SetDevice(): Failed get device #%u friendly name, %s",
                                       i, hr);
                continue;
            }

            // While it would be nice to compare the strings exactly, OpenAL will prepend junk to the
            // name of the device. That SHOULD have been filtered out by the caller of this method, but
            // someone could always start using NewCoolAL, which uses a new prefix that ISN'T removed.
            ST::string currDeviceName = ST::string::from_wchar(deviceFriendlyName.pwszVal);
            if (deviceName.contains(currDeviceName)) {
                fDevice = device;
                device = nullptr;
                break;
            }
        }
    } while (0);

    PropVariantClear(&deviceFriendlyName);
    ICOMRelease(props);
    ICOMRelease(device);
    ICOMRelease(devices);
    ICOMRelease(enumerator);

    if (!fDevice) {
        plStatusLog::AddLineSF("audio.log", plStatusLog::kYellow,
                               "WinCoreAudioEndpointVolume::SetDevice(): Unable to find endpoint {}",
                               deviceName);
        return false;
    }

    return true;
}

bool plWinCoreAudioEndpointVolume::SetVolume(float pct)
{
    auto volume = GetVolumeCtrl();
    if (volume) {
        pct = std::clamp(pct, 0.f, 1.f);

        hsCOMError hr = volume->SetMasterVolumeLevelScalar(pct, nullptr);
        volume->Release();
        if (FAILED(hr)) {
            plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                                   "WinCoreAudioEndpointVolume::SetVolume(): Failed to set master volume, {}",
                                   hr);
            return false;
        }
        return true;
    }

    return false;
}

bool plWinCoreAudioEndpointVolume::Supported() const
{
    if (!fDevice)
        return false;

    DWORD state;
    hsCOMError hr = fDevice->GetState(&state);
    if (FAILED(hr)) {
        plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                               "WinCoreAudioEndpointVolume::Supported(): Failed get device state, {}",
                               hr);
        return false;
    }

    if (state != DEVICE_STATE_ACTIVE)
        return false;

    // Just for kicks, try to activate the device just like we would to change the volume...
    auto volume = GetVolumeCtrl();
    if (volume) {
        volume->Release();
        return true;
    } else {
        return false;
    }
}

// =============================================================================

plAudioEndpointVolume* plAudioEndpointVolume::Create()
{
    return new plWinCoreAudioEndpointVolume;
}

#else

class plNullAudioEndpointVolume : public plAudioEndpointVolume
{
public:
    float GetVolume() const override { return 0.f; }
    bool SetDefaultDevice(plAudioEndpointType) override { return false; }
    bool SetDevice(plAudioEndpointType, const ST::string&) override { return false; }
    bool SetVolume(float) override { return false; }
    bool Supported() const override { return false; }
};

plAudioEndpointVolume* plAudioEndpointVolume::Create()
{
    return new plNullAudioEndpointVolume;
}

#endif
