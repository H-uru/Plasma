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

#include "plVulkanPipeline.h"

#include "plPipeline/pl3DPipeline.h"

#include <string_theory/format>

#include <vector>

namespace
{
    ST::string IVersionString(uint32_t version)
    {
        return ST::format("{}.{}.{}", VK_API_VERSION_MAJOR(version),
                          VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));
    }

    bool IFillDeviceRecord(hsG3DDeviceRecord& devRec, const VkPhysicalDeviceProperties& props,
                           hsDisplayHndl display)
    {
        // The backend is written against dynamic rendering, synchronization2 and
        // timeline semaphores, all of which are core in 1.3.
        if (props.apiVersion < VK_API_VERSION_1_3)
            return false;

        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeVulkan);
        devRec.SetDriverName(ST_LITERAL("Vulkan"));
        devRec.SetDeviceDesc(props.deviceName);
        devRec.SetDriverDesc(IVersionString(props.apiVersion));
        devRec.SetDriverVersion(IVersionString(props.driverVersion));

        devRec.SetCap(hsG3DDeviceSelector::kCapsMipmap);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPerspective);
        devRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsDoesSmallTextures);
        devRec.SetCap(hsG3DDeviceSelector::kCapsPixelShader);
        devRec.SetCap(hsG3DDeviceSelector::kCapsHardware);

        devRec.SetLayersAtOnce(8);

        plDisplayHelper* displayHelper = plDisplayHelper::GetInstance();
        if (displayHelper) {
            for (const auto& mode : displayHelper->GetSupportedDisplayModes(display))
                devRec.GetModes().emplace_back(mode.Width, mode.Height, mode.ColorDepth);
            devRec.SetDefaultModeIndex(0);
        } else {
            // Just make a fake mode so the device selector will let it through
            devRec.GetModes().emplace_back(
                hsG3DDeviceSelector::kDefaultWidth,
                hsG3DDeviceSelector::kDefaultHeight,
                hsG3DDeviceSelector::kDefaultDepth
            );
        }

        devRec.SetFudgeFactors(hsG3DDeviceRecord::kDefaultChipset);

        return true;
    }
}

void plVulkanEnumerate::Enumerate(std::vector<hsG3DDeviceRecord>& records, hsDisplayHndl display)
{
    if (!plVulkanInitVolk())
        return;

    // A throwaway instance, just to ask what hardware is here. It needs no
    // surface: presentation support is re-checked against the real surface when
    // the device is actually created.
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "Plasma";
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instInfo.pApplicationInfo = &appInfo;

    VkInstance instance = VK_NULL_HANDLE;
    if (vkCreateInstance(&instInfo, nullptr, &instance) != VK_SUCCESS)
        return;

    volkLoadInstanceOnly(instance);

    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(instance, &count, nullptr) == VK_SUCCESS && count > 0) {
        std::vector<VkPhysicalDevice> devices(count);
        if (vkEnumeratePhysicalDevices(instance, &count, devices.data()) == VK_SUCCESS) {
            for (VkPhysicalDevice device : devices) {
                VkPhysicalDeviceProperties props{};
                vkGetPhysicalDeviceProperties(device, &props);

                hsG3DDeviceRecord devRec;
                if (IFillDeviceRecord(devRec, props, display))
                    records.emplace_back(std::move(devRec));
            }
        }
    }

    vkDestroyInstance(instance, nullptr);

    // vkDestroyInstance leaves volk holding instance-level entry points that now
    // dangle. plVulkanDevice::ICreateInstance reloads them against its own
    // instance, but anything enumerating twice would trip over this.
}
