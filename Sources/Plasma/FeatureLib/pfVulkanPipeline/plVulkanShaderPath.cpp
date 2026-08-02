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

// The plShader programmable path.
//
// A handful of Plasma materials -- water, grass, and the wave decals -- carry an
// authored vertex/pixel shader pair instead of relying on the fixed-function
// combiner. Each pair is identified by a plShaderID, and each backend supplies
// its own implementation of the ones it supports; the engine just hands over a
// bank of float4 registers.
//
// Ported from plMetalPipeline::ISetShaders (:1992-2053) and ISetPipeConsts
// (:2790 region), plus the ID-to-function mapping in
// plMetalPipelineState.cpp:377-473.

#include "plVulkanPipeline.h"

#include "plVulkanDevice.h"
#include "plVulkanShaderRef.h"

#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plPipeline/plFogEnvironment.h"
#include "plSurface/plShader.h"

#include <cstring>

/**
 * Whether both halves of a pair have an implementation here.
 *
 * A material asking for a pair we do not have falls back to the fixed-function
 * combiner, which is wrong but draws something, rather than not drawing at all.
 */
bool plVulkanPipeline::IShaderPairSupported(plShaderID::ID vertexID, plShaderID::ID fragmentID)
{
    return fDevice.GetShaderModules(vertexID, fragmentID, nullptr, nullptr);
}

// Ported from plMetalPipeline::ISetPipeConsts (:2790 region).
//
// Pipe constants are the registers a shader wants filled from pipeline state
// rather than from the material: transforms, the camera position, layer colors.
void plVulkanPipeline::ISetPipeConsts(plShader* shader)
{
    const size_t n = shader->GetNumPipeConsts();

    for (size_t i = 0; i < n; i++) {
        const plPipeConst& pc = shader->GetPipeConst(i);

        switch (pc.fType) {
        case plPipeConst::kFogSet: {
            float set[4] = { 1.f, 0.f, 0.f, 1.f };
            hsColorRGBA unusedColor;
            float start = 0.f;
            float end = 0.f;
            fView.GetDefaultFog().GetPipelineParams(&start, &end, &unusedColor);
            if (end > start) {
                set[0] = -end;
                set[1] = 1.f / (start - end);
            }
            shader->SetFloat4(pc.fReg, set);
        }
            break;

        case plPipeConst::kLayAmbient:
            if (fCurrLay)
                shader->SetColor(pc.fReg, fCurrLay->GetAmbientColor());
            break;

        case plPipeConst::kLayRuntime:
            if (fCurrLay) {
                hsColorRGBA col = fCurrLay->GetRuntimeColor();
                col.a = fCurrLay->GetOpacity();
                shader->SetColor(pc.fReg, col);
            }
            break;

        case plPipeConst::kLaySpecular:
            if (fCurrLay)
                shader->SetColor(pc.fReg, fCurrLay->GetSpecularColor());
            break;

        case plPipeConst::kTex3x4_0:
        case plPipeConst::kTex3x4_1:
        case plPipeConst::kTex3x4_2:
        case plPipeConst::kTex3x4_3:
        case plPipeConst::kTex3x4_4:
        case plPipeConst::kTex3x4_5:
        case plPipeConst::kTex3x4_6:
        case plPipeConst::kTex3x4_7:
            {
                const uint32_t stage = pc.fType - plPipeConst::kTex3x4_0;
                if (const hsMatrix44* xfm = IGetStageTransform(stage))
                    shader->SetMatrix34(pc.fReg, *xfm);
            }
            break;

        case plPipeConst::kTex2x4_0:
        case plPipeConst::kTex2x4_1:
        case plPipeConst::kTex2x4_2:
        case plPipeConst::kTex2x4_3:
        case plPipeConst::kTex2x4_4:
        case plPipeConst::kTex2x4_5:
        case plPipeConst::kTex2x4_6:
        case plPipeConst::kTex2x4_7:
            {
                const uint32_t stage = pc.fType - plPipeConst::kTex2x4_0;
                if (const hsMatrix44* xfm = IGetStageTransform(stage))
                    shader->SetMatrix24(pc.fReg, *xfm);
            }
            break;

        case plPipeConst::kTex1x4_0:
        case plPipeConst::kTex1x4_1:
        case plPipeConst::kTex1x4_2:
        case plPipeConst::kTex1x4_3:
        case plPipeConst::kTex1x4_4:
        case plPipeConst::kTex1x4_5:
        case plPipeConst::kTex1x4_6:
        case plPipeConst::kTex1x4_7:
            {
                const uint32_t stage = pc.fType - plPipeConst::kTex1x4_0;
                if (const hsMatrix44* xfm = IGetStageTransform(stage))
                    shader->SetFloat4(pc.fReg, xfm->fMap[0]);
            }
            break;

        case plPipeConst::kLocalToNDC:
            shader->SetMatrix44(pc.fReg, IGetCameraToNDC() *
                                         GetViewTransform().GetWorldToCamera() *
                                         GetLocalToWorld());
            break;

        case plPipeConst::kCameraToNDC:
            shader->SetMatrix44(pc.fReg, IGetCameraToNDC());
            break;

        case plPipeConst::kWorldToNDC:
            shader->SetMatrix44(pc.fReg, IGetCameraToNDC() *
                                         GetViewTransform().GetWorldToCamera());
            break;

        case plPipeConst::kLocalToWorld:
            shader->SetMatrix34(pc.fReg, GetLocalToWorld());
            break;

        case plPipeConst::kWorldToLocal:
            shader->SetMatrix34(pc.fReg, GetWorldToLocal());
            break;

        case plPipeConst::kWorldToCamera:
            shader->SetMatrix34(pc.fReg, GetViewTransform().GetWorldToCamera());
            break;

        case plPipeConst::kCameraToWorld:
            shader->SetMatrix34(pc.fReg, GetViewTransform().GetCameraToWorld());
            break;

        case plPipeConst::kLocalToCamera:
            shader->SetMatrix34(pc.fReg, GetViewTransform().GetWorldToCamera() *
                                         GetLocalToWorld());
            break;

        case plPipeConst::kCameraToLocal:
            shader->SetMatrix34(pc.fReg, GetWorldToLocal() *
                                         GetViewTransform().GetCameraToWorld());
            break;

        case plPipeConst::kCamPosWorld:
            shader->SetVectorW(pc.fReg,
                               GetViewTransform().GetCameraToWorld().GetTranslate(), 1.f);
            break;

        case plPipeConst::kCamPosLocal:
            shader->SetVectorW(pc.fReg,
                               GetWorldToLocal() *
                                   GetViewTransform().GetCameraToWorld().GetTranslate(),
                               1.f);
            break;

        case plPipeConst::kObjPosWorld:
            shader->SetVectorW(pc.fReg, GetLocalToWorld().GetTranslate(), 1.f);
            break;

        default:
            // The light and color-filter constants; Metal asserts on these and
            // no shipped shader asks for them.
            hsAssert(false, "Unimplemented pipe constant requested by a shader");
            break;
        }
    }
}

/**
 * The texture transform of a layer relative to the current pass.
 *
 * Null when the shader asks for a stage the pass does not have, which means the
 * shader and the material disagree.
 */
const hsMatrix44* plVulkanPipeline::IGetStageTransform(uint32_t stage)
{
    if (!fCurrMaterial || stage >= fCurrNumLayers) {
        hsAssert(false, "Shader asking for higher stage transform than we have");
        return nullptr;
    }

    plLayerInterface* layer = fCurrMaterial->GetLayer(fCurrLayerIdx + stage);
    if (!layer)
        return nullptr;

    return &layer->GetTransform();
}

/**
 * Copies a shader's registers into the frame's scratch ring.
 *
 * Returns the offset to bind, or the fallback offset when the shader has no
 * constants -- the binding still has to name something.
 */
uint32_t plVulkanPipeline::IUploadShaderConsts(plShader* shader)
{
    const size_t count = shader->GetNumConsts();
    if (count == 0)
        return fEmptyShadowState.fOffset;

    hsAssert(count <= kMaxShaderConsts, "Shader wants more constants than the bank holds");
    const size_t clamped = std::min(count, size_t(kMaxShaderConsts));

    plVulkanDevice::plScratchAlloc alloc =
        fDevice.AllocateScratch(sizeof(plVkVec4) * kMaxShaderConsts);
    if (!alloc.IsValid())
        return fEmptyShadowState.fOffset;

    // The bank is bound at its full size whatever the shader uses, so the tail
    // has to be zeroed rather than left as whatever the ring held.
    memset(alloc.fMapped, 0, sizeof(plVkVec4) * kMaxShaderConsts);
    memcpy(alloc.fMapped, shader->GetConstBasePtr(), sizeof(plVkVec4) * clamped);

    return alloc.fOffset;
}

// Ported from plMetalPipeline::ISetShaders (:1992-2053).
bool plVulkanPipeline::ISetShaders(VkCommandBuffer cmd, const plVulkanVertexBufferRef* vRef,
                                  const hsGMatState& state, plShader* vShader, plShader* pShader,
                                  uint32_t* vertexConstOffset, uint32_t* fragmentConstOffset)
{
    hsAssert(vShader && pShader, "Can't handle programmable passes without both shaders");
    if (!vShader || !pShader)
        return false;

    hsAssert(vShader->IsVertexShader(), "Wrong type shader as vertex shader");
    hsAssert(pShader->IsPixelShader(), "Wrong type shader as pixel shader");

    const plShaderID::ID vertexID = vShader->GetDecl()->GetID();
    const plShaderID::ID fragmentID = pShader->GetDecl()->GetID();

    plVulkanPipelineKey key{};
    key.fPassKind = plVulkanPipelineKey::kPassMaterial;
    plVulkanFillVertexKey(key, vRef);
    key.fVertexShaderID = uint16_t(vertexID);
    key.fFragmentShaderID = uint16_t(fragmentID);
    key.fBlendFlags[0] = state.fBlendFlags;
    key.fColorFormat = fDevice.CurrentColorFormat();
    key.fDepthFormat = fDevice.CurrentDepthFormat();
    key.fSampleCount = fDevice.CurrentSampleCount();

    VkPipeline pipeline = fDevice.GetPipelineState(key);
    if (pipeline == VK_NULL_HANDLE)
        return false;

    if (fState.fPipeline != pipeline) {
        fState.fPipeline = pipeline;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // Pipe constants first: they write into the shader's own register bank, so
    // they have to land before it is copied to the GPU.
    ISetPipeConsts(vShader);
    ISetPipeConsts(pShader);

    for (plShader* shader : { vShader, pShader }) {
        plVulkanShaderRef* ref = static_cast<plVulkanShaderRef*>(shader->GetDeviceRef());
        if (!ref) {
            ref = new plVulkanShaderRef(shader);
            shader->SetDeviceRef(ref);
            ref->UnRef();
        }
        if (!ref->IsLinked())
            ref->Link(&fShaderRefList);
    }

    *vertexConstOffset = IUploadShaderConsts(vShader);
    *fragmentConstOffset = IUploadShaderConsts(pShader);

    // Cull mode depends on the handedness of local-to-camera as well as on the
    // material, so it is handled here rather than baked into the pipeline.
    ISetCullMode(cmd, state);

    return true;
}

/**
 * Binds the descriptors a programmable pass needs.
 *
 * Metal binds every one of the material's layers, not just the pass's, because a
 * hand-written shader indexes its textures however it likes
 * (plMetalPipeline.cpp:1585-1613). Returns false when a layer it would need has
 * no texture, which is what Metal does too.
 */
bool plVulkanPipeline::IBindShaderPassResources(VkCommandBuffer cmd, hsGMaterial* material,
                                               size_t pass, const plSpan* span,
                                               uint32_t vertexConstOffset,
                                               uint32_t fragmentConstOffset)
{
    const plVulkanTextureRef* layers[kMaxLayers] = {};
    uint8_t clampFlags[kMaxLayers] = {};

    const size_t count = std::min(material->GetNumLayers(), size_t(kMaxLayers));
    for (size_t i = 0; i < count; i++) {
        plLayerInterface* layer = material->GetLayer(i);
        if (!layer)
            return false;

        CheckTextureRef(layer);

        plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());
        if (!img)
            return false;

        plVulkanTextureRef* texRef = static_cast<plVulkanTextureRef*>(img->GetDeviceRef());
        if (!texRef || texRef->fImageView == VK_NULL_HANDLE)
            return false;

        layers[i] = texRef;
        clampFlags[i] = uint8_t(layer->GetClampFlags());
    }

    // The fixed-function blocks are bound but unread: a programmable shader takes
    // everything it needs from its own constant bank.
    plVulkanDevice::plScratchAlloc vertexAlloc = fDevice.AllocateScratch(sizeof(VertexUniforms));
    plVulkanDevice::plScratchAlloc materialAlloc =
        fDevice.AllocateScratch(sizeof(plMaterialLightingDescriptor));
    if (!vertexAlloc.IsValid() || !materialAlloc.IsValid())
        return false;

    memcpy(vertexAlloc.fMapped, &fCurrentUniforms, sizeof(VertexUniforms));
    memcpy(materialAlloc.fMapped, &fCurrentMaterial, sizeof(plMaterialLightingDescriptor));

    VkDescriptorSet uniformSet = fDevice.GetUniformDescriptorSet();
    VkDescriptorSet textureSet =
        fDevice.GetTextureDescriptorSet(layers, clampFlags, uint32_t(count), nullptr, nullptr,
                                        material->GetKeyName());
    if (uniformSet == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE)
        return false;

    const uint32_t dynamicOffsets[6] = { vertexAlloc.fOffset, materialAlloc.fOffset,
                                         fLightBuffer.fOffset, fEmptyShadowState.fOffset,
                                         vertexConstOffset, fragmentConstOffset };
    VkDescriptorSet sets[2] = { uniformSet, textureSet };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fDevice.GetPipelineLayout(),
                            0, 2, sets, 6, dynamicOffsets);

    return true;
}
