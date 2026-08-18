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

#include "plVulkanDeviceRef.h"
#include "plVulkanDevice.h"

#include "plProfile.h"

#include "hsFastMath.h"

#include "plGLight/plLightInfo.h"

plProfile_CreateMemCounter("Vulkan Vertices", "Memory", MemVulkanVertex);
plProfile_CreateMemCounter("Vulkan Indices", "Memory", MemVulkanIndex);
plProfile_CreateMemCounter("Vulkan Textures", "Memory", MemVulkanTexture);

uint32_t plVulkanBufferPoolRef::fFrameTime = 0;

/*****************************************************************************
 ** plVulkanDeviceRef                                                       **
 *****************************************************************************/

plVulkanDeviceRef::plVulkanDeviceRef()
    : fNext(), fBack()
{
}

plVulkanDeviceRef::~plVulkanDeviceRef()
{
    Unlink();
}

void plVulkanDeviceRef::Unlink()
{
    // Not every ref lives in a list. plVulkanTextFont holds its glyph atlas as a
    // plVulkanTextureRef by value, and the destructor unlinks unconditionally,
    // so an unlinked ref has to be a no-op rather than a write through null.
    if (!fBack)
        return;

    if (fNext)
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nullptr;
    fNext = nullptr;
}

void plVulkanDeviceRef::Link(plVulkanDeviceRef** back)
{
    hsAssert(fNext == nullptr && fBack == nullptr, "Linking a ref that is already linked");

    fNext = *back;
    if (*back)
        (*back)->fBack = &fNext;
    fBack = back;
    *back = this;
}

/*****************************************************************************
 ** plVulkanBufferPoolRef                                                   **
 *****************************************************************************/

void plVulkanBufferPoolRef::SetBuffer(plVulkanDevice* device, const plVulkanBuffer& buffer)
{
    fDevice = device;
    fBuffer = buffer;

    const size_t currentSize = fBuffers[fCurrentFrame].size();
    if (fCurrentPass >= currentSize) {
        fBuffers[fCurrentFrame].resize(fCurrentPass + 1);
    } else if (fBuffers[fCurrentFrame][fCurrentPass].IsValid()) {
        // Replacing an occupied slot. The GPU may still be reading it, so it
        // goes through the deferred-destroy queue rather than being freed here.
        device->RetireBuffer(fBuffers[fCurrentFrame][fCurrentPass]);
    }

    fBuffers[fCurrentFrame][fCurrentPass] = buffer;
}

void plVulkanBufferPoolRef::Release()
{
    if (fDevice) {
        for (auto& frame : fBuffers) {
            for (const plVulkanBuffer& buffer : frame)
                fDevice->RetireBuffer(buffer);
        }
    }

    for (auto& frame : fBuffers)
        frame.clear();

    fBuffer = plVulkanBuffer{};
    SetDirty(true);
}

/*****************************************************************************
 ** plVulkanVertexBufferRef                                                 **
 *****************************************************************************/

plVulkanVertexBufferRef::~plVulkanVertexBufferRef()
{
    Release();
}

void plVulkanVertexBufferRef::Release()
{
    if (fBuffer.IsValid())
        plProfile_DelMem(MemVulkanVertex, fBuffer.fSize);

    delete[] fData;
    fData = nullptr;

    plVulkanBufferPoolRef::Release();
    SetRebuiltSinceUsed(true);
}

/*****************************************************************************
 ** plVulkanIndexBufferRef                                                  **
 *****************************************************************************/

plVulkanIndexBufferRef::~plVulkanIndexBufferRef()
{
    Release();
}

void plVulkanIndexBufferRef::Release()
{
    if (fBuffer.IsValid())
        plProfile_DelMem(MemVulkanIndex, fBuffer.fSize);

    plVulkanBufferPoolRef::Release();
    SetRebuiltSinceUsed(true);
}

/*****************************************************************************
 ** plVulkanRenderTargetRef                                                 **
 *****************************************************************************/

plVulkanRenderTargetRef::~plVulkanRenderTargetRef()
{
    Release();
}

void plVulkanRenderTargetRef::Release()
{
    if (fDevice) {
        if (fOwnsImage) {
            fDevice->RetireImage(fImage, fImageView, fAllocation);
            fDevice->RetireImage(fDepthImage, fDepthView, fDepthAllocation);
        } else {
            // A cube face owns its view and nothing else.
            fDevice->RetireImage(VK_NULL_HANDLE, fImageView, nullptr);
        }
    }

    fImage = VK_NULL_HANDLE;
    fImageView = VK_NULL_HANDLE;
    fAllocation = nullptr;
    fDepthImage = VK_NULL_HANDLE;
    fDepthView = VK_NULL_HANDLE;
    fDepthAllocation = nullptr;
    fShaderReadable = false;
    fDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SetDirty(true);
}

/*****************************************************************************
 ** plVulkanLightRef                                                        **
 *****************************************************************************/

plVulkanLightRef::~plVulkanLightRef()
{
    Release();
}

void plVulkanLightRef::Release()
{
    SetDirty(true);
}

// Ported from plMetalLightRef::UpdateMetalInfo (plMetalDeviceRefs.cpp:162-214).
void plVulkanLightRef::UpdateShaderInfo(plShaderLightSource* dst) const
{
    memset(dst, 0, sizeof(*dst));

    const hsColorRGBA amb = fOwner->GetAmbient();
    dst->ambient = { amb.r, amb.g, amb.b, amb.a };

    const hsColorRGBA diff = fOwner->GetDiffuse();
    dst->diffuse = { diff.r, diff.g, diff.b, diff.a };

    const hsColorRGBA spec = fOwner->GetSpecular();
    dst->specular = { spec.r, spec.g, spec.b, spec.a };

    // Effectively unbounded unless the light says otherwise.
    constexpr float kMaxRange = 32767.f;
    dst->range = kMaxRange;

    if (plDirectionalLightInfo* dirLight = plDirectionalLightInfo::ConvertNoRef(fOwner)) {
        const hsVector3 lightDir = dirLight->GetWorldDirection();

        // w of zero is what marks this directional in the shader.
        dst->position = { lightDir.fX, lightDir.fY, lightDir.fZ, 0.f };
        dst->direction = { lightDir.fX, lightDir.fY, lightDir.fZ };

        dst->constAtten = 1.f;
        dst->linAtten = 0.f;
        dst->quadAtten = 0.f;
        return;
    }

    plOmniLightInfo* omniLight = plOmniLightInfo::ConvertNoRef(fOwner);
    if (!omniLight)
        return;

    const hsPoint3 pos = omniLight->GetWorldPosition();
    dst->position = { pos.fX, pos.fY, pos.fZ, 1.f };

    dst->constAtten = omniLight->GetConstantAttenuation();
    dst->linAtten = omniLight->GetLinearAttenuation();
    dst->quadAtten = omniLight->GetQuadraticAttenuation();

    if (omniLight->GetRadius() != 0.f)
        dst->range = omniLight->GetRadius();

    // A projected light has no cone; it is handled by the projection pass.
    plSpotLightInfo* spotLight = omniLight->GetProjection()
                              ? nullptr
                              : plSpotLightInfo::ConvertNoRef(omniLight);
    if (spotLight) {
        const hsVector3 lightDir = spotLight->GetWorldDirection();
        dst->direction = { lightDir.fX, lightDir.fY, lightDir.fZ };

        const float falloff = spotLight->GetFalloff();
        const float gamma = cosf(spotLight->GetSpotInner());
        const float phi = cosf(spotLight->GetProjection() ? hsConstants::half_pi<float>
                                                         : spotLight->GetSpotOuter());

        dst->spotProps = { falloff, gamma, phi, 0.f };
    }
}

/*****************************************************************************
 ** plVulkanTextureRef                                                      **
 *****************************************************************************/

plVulkanTextureRef::~plVulkanTextureRef()
{
    Release();
}

void plVulkanTextureRef::Release()
{
    if (fImage != VK_NULL_HANDLE) {
        plProfile_DelMem(MemVulkanTexture, fWidth * fHeight * 4);

        if (fDevice)
            fDevice->RetireImage(fImage, fImageView, fAllocation);

        fImage = VK_NULL_HANDLE;
        fImageView = VK_NULL_HANDLE;
        fAllocation = nullptr;
    }

    SetDirty(true);
}
