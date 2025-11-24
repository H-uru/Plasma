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
#include "plMetalDeviceRef.h"
#include "plMetalLightRef.h"
#include "plMetalPipeline.h"
#include "plPipeline/hsWinRef.h"
#include "plProfile.h"
#include "plStatusLog/plStatusLog.h"

plProfile_CreateMemCounter("Vertices", "Memory", MemVertex);
plProfile_CreateMemCounter("Indices", "Memory", MemIndex);
plProfile_CreateMemCounter("Textures", "Memory", MemTexture);

/*****************************************************************************
 ** Generic plGLDeviceRef Functions                                         **
 *****************************************************************************/
plMetalDeviceRef::plMetalDeviceRef()
    : fNext(),
      fBack()
{
}

plMetalDeviceRef::~plMetalDeviceRef()
{
    if (fNext != nullptr || fBack != nullptr)
        Unlink();
}

void plMetalDeviceRef::Unlink()
{
    hsAssert(fBack, "plGLDeviceRef not in list");

    if (fNext)
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nullptr;
    fNext = nullptr;
}

uint32_t plMetalBufferPoolRef::fFrameTime(0);

void plMetalDeviceRef::Link(plMetalDeviceRef **back)
{
    hsAssert(fNext == nullptr && fBack == nullptr, "Trying to link a plMetalDeviceRef that's already linked");

    fNext = *back;
    if (*back)
        (*back)->fBack = &fNext;
    fBack = back;
    *back = this;
}

/*****************************************************************************
 ** Vertex buffer cleanup Functions                                         **
 *****************************************************************************/

plMetalVertexBufferRef::~plMetalVertexBufferRef()
{
    delete fData;
    Release();
}

void plMetalVertexBufferRef::Release()
{
    SetDirty(true);
}

/*****************************************************************************
 ** Index buffer cleanup Functions                                          **
 *****************************************************************************/

plMetalIndexBufferRef::~plMetalIndexBufferRef()
{
    Release();
}

void plMetalIndexBufferRef::Release()
{
    SetDirty(true);
}

/*****************************************************************************
 ** Texture cleanup Functions                                               **
 *****************************************************************************/

void plMetalTextureRef::Release()
{
    if (fTexture) {
        fTexture->release();
        fTexture = nullptr;
    }
    SetDirty(true);
}

plMetalTextureRef::~plMetalTextureRef()
{
    Release();

    if (fNext != nullptr || fBack != nullptr)
        Unlink();
}

/*****************************************************************************
 ** FrameBuffer cleanup Functions                                           **
 *****************************************************************************/

plMetalRenderTargetRef::~plMetalRenderTargetRef()
{
    Release();
}

void plMetalRenderTargetRef::Release()
{
    if (fDepthBuffer) {
        fDepthBuffer->release();
        fDepthBuffer = nullptr;
    }
    plMetalTextureRef::Release();
    SetDirty(true);
}

void plMetalLightRef::UpdateMetalInfo(plMetalShaderLightSource* dev)
{
    hsColorRGBA amb = fOwner->GetAmbient();
    dev->ambient = {static_cast<half>(amb.r), static_cast<half>(amb.g), static_cast<half>(amb.b), static_cast<half>(amb.a)};

    hsColorRGBA diff = fOwner->GetDiffuse();
    dev->diffuse = {static_cast<half>(diff.r), static_cast<half>(diff.g), static_cast<half>(diff.b), static_cast<half>(diff.a)};

    hsColorRGBA spec = fOwner->GetSpecular();
    dev->specular = {static_cast<half>(spec.r), static_cast<half>(spec.g), static_cast<half>(spec.b), static_cast<half>(spec.a)};

    plDirectionalLightInfo* dirLight = nullptr;
    plOmniLightInfo*        omniLight = nullptr;
    plSpotLightInfo*        spotLight = nullptr;

    constexpr float kMaxRange = 32767.f;
    dev->range = kMaxRange;

    if ((dirLight = plDirectionalLightInfo::ConvertNoRef(fOwner)) != nullptr) {
        hsVector3 lightDir = dirLight->GetWorldDirection();
        dev->position = {lightDir.fX, lightDir.fY, lightDir.fZ, 0.0};
        dev->direction = {lightDir.fX, lightDir.fY, lightDir.fZ};

        dev->constAtten = 1.0f;
        dev->linAtten = 0.0f;
        dev->quadAtten = 0.0f;

    } else if ((omniLight = plOmniLightInfo::ConvertNoRef(fOwner)) != nullptr) {
        hsPoint3 pos = omniLight->GetWorldPosition();
        dev->position = {pos.fX, pos.fY, pos.fZ, 1.0};

        dev->constAtten = omniLight->GetConstantAttenuation();
        dev->linAtten = omniLight->GetLinearAttenuation();
        dev->quadAtten = omniLight->GetQuadraticAttenuation();

        if (omniLight->GetRadius() != 0.f) {
            dev->range = omniLight->GetRadius();
        }

        if (!omniLight->GetProjection() && (spotLight = plSpotLightInfo::ConvertNoRef(omniLight)) != nullptr) {
            hsVector3 lightDir = spotLight->GetWorldDirection();
            dev->direction = {lightDir.fX, lightDir.fY, lightDir.fZ};

            float falloff = spotLight->GetFalloff();
            float gamma = cosf(spotLight->GetSpotInner());
            float phi = cosf(spotLight->GetProjection() ? hsConstants::half_pi<float> : spotLight->GetSpotOuter());

            dev->spotProps = {falloff, gamma, phi};
        } else {
            dev->spotProps = {0.0f, 0.0f, 0.0f};
        }
    }
}

plMetalLightRef::~plMetalLightRef()
{
    Release();
}

void plMetalLightRef::Release()
{
    // Nothing to do here
    // The DX version had to release DX resources,
    // but once this light is gone is just won't be sent
    // to the next render
}
