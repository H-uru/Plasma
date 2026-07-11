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

#include "plMetalArgumentBuffer.h"
#include "plMetalDeviceRef.h"
#include "plGImage/plBitmap.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plPipeline/plCubicRenderTarget.h"

// MARK: Bump argument buffer

NS::Array* plMetalBumpArgumentBuffer::GetArgumentDescriptors() const
{
    MTL::ArgumentDescriptor* descriptors[4];
    descriptors[0] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[0]->setIndex(dTangentIndexID);
    descriptors[0]->setDataType(MTL::DataTypeChar2);

    descriptors[1] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[1]->setIndex(textureID);
    descriptors[1]->setDataType(MTL::DataTypeTexture);

    descriptors[2] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[2]->setIndex(samplerID);
    descriptors[2]->setDataType(MTL::DataTypeSampler);

    descriptors[3] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[3]->setIndex(dScaleID);
    descriptors[3]->setDataType(MTL::DataTypeFloat);

    NS::Array* array = NS::Array::array((const NS::Object* const*)descriptors, 4);
    return array;
}

plMetalBumpArgumentBuffer::plMetalBumpArgumentBuffer(plMetalDevice* device, size_t numElements)
    : plMetalArgumentBuffer<plMetalBumpmap>(device, numElements)
{
    fBumps.resize(numElements);
}

void plMetalBumpArgumentBuffer::Set(const std::vector<plMetalBumpMapping>& bumps)
{
    if (CheckBuffer(bumps)) {
        return;
    }
    ConfigureBuffer();
    if (fTier == plMetalArgumentBufferTier::Tier1) {
        // Tier 1, because Tier 1 doesn't guarantee memory layout, go through the encoder
        for (size_t i = 0; i < bumps.size(); ++i) {
            auto& bump = bumps[i];
            fEncoder->setArgumentBuffer(GetBuffer(), 0, i);
            fEncoder->setTexture(bump.fTexture, textureID);
            fEncoder->setSamplerState(bump.fSampler, samplerID);
            uint8_t* cotangentUBuffer = static_cast<uint8_t*>(fEncoder->constantData(dTangentIndexID));
            memcpy(cotangentUBuffer, &bump.fDTangentUIndex, sizeof(uint8_t) * 2);
            float* scaleBuffer = static_cast<float*>(fEncoder->constantData(dScaleID));
            memcpy(scaleBuffer, &bump.fScale, sizeof(float));

            fBumps[i] = bump;
        }
    }
#ifdef METAL_3_SDK
    else {
        // Tier 2, memory layout is guaranteed, we can scrible directly on buffer memory
        for (int i = 0; i < bumps.size(); ++i) {
            auto& bump = bumps[i];
            fValue[i].bumpTexture = bump.fTexture->gpuResourceID();
            fValue[i].bumpTextureSampler = bump.fSampler->gpuResourceID();
            fValue[i].dTangentIndex = simd::make_char2(bump.fDTangentUIndex, bump.fDTangentVIndex);
            fValue[i].scale = bump.fScale;

            fBumps[i] = bump;
        }
    }
#endif
    if (GetBuffer()->storageMode() == MTL::StorageModeManaged) {
        GetBuffer()->didModifyRange(NS::Range(0, GetBuffer()->length()));
    }
}

void plMetalBumpArgumentBuffer::Bind(MTL::RenderCommandEncoder* encoder)
{
    for (const auto& bump : fBumps) {
        // These textures can't go into a heap because they're shared by multiple
        // materials, boo. Mark them as needing to be resident one at a time.
        encoder->useResource(bump.fTexture, MTL::ResourceUsageRead, MTL::RenderStageFragment);
    }
    encoder->setVertexBuffer(GetBuffer(), 0, BumpState);
    encoder->setFragmentBuffer(GetBuffer(), 0, BumpState);
}

bool plMetalBumpArgumentBuffer::CheckBuffer(const std::vector<plMetalBumpMapping>& bumps)
{
    if (bumps.size() != fNumElements || GetBuffer() == nullptr) {
        return false;
    }
    for (int i = 0; i < bumps.size(); ++i) {
        auto& bump = bumps[i];
        if (bump.fTexture != fBumps[i].fTexture)
            return false;
        if (bump.fDTangentUIndex != fBumps[i].fDTangentUIndex ||
            bump.fDTangentVIndex != fBumps[i].fDTangentVIndex) {
            return false;
        }
        if (bump.fScale != fBumps[i].fScale) {
            return false;
        }
    }
    return true;
}

// MARK: Layer argument buffer

NS::Array* plMetalLayerListArgumentBuffer::GetArgumentDescriptors() const
{
    MTL::ArgumentDescriptor* descriptors[3];
    descriptors[0] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[0]->setIndex(0);
    descriptors[0]->setDataType(MTL::DataTypeTexture);

    descriptors[1] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[1]->setIndex(1);
    descriptors[1]->setDataType(MTL::DataTypeTexture);

    descriptors[2] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[2]->setIndex(2);
    descriptors[2]->setDataType(MTL::DataTypeSampler);

    NS::Array* array = NS::Array::array((const NS::Object* const*)descriptors, 3);
    return array;
}

plMetalLayerListArgumentBuffer::plMetalLayerListArgumentBuffer(plMetalDevice* device, size_t numElements) : plMetalArgumentBuffer<plMetalLayer>(device, numElements)
{
    fLayers.resize(numElements);
    fBoundBufferIndex = -1;
}

void plMetalLayerListArgumentBuffer::Set(const plLayerInterface* layer, const size_t layerIndex)
{
    assert(layerIndex < fNumElements);
    if (CheckBuffer(layer, layerIndex)) {
        return;
    }
    // Check and see if the layer buffer has been bound for a draw.
    // If it has - we'll have to swap to the next buffer in the ring
    // so we don't cause a data race.
    // If the current buffer has never been bound (because
    // we're still in the middle of encoding multiple layers before
    // a draw) then we're ok and don't need to swap buffers.
    if(fBoundBufferIndex == fCurrentBufferIndex)
    {
        MTL::Buffer* currentBuffer = fCurrentBufferIndex > -1 ? fBuffer[fCurrentBufferIndex].get() : nullptr;
        ConfigureBuffer();
        // Copy the previous layer list buffer into the new one
        // Only some layers might be updated - so we want to preserve the unchanged ones
        if (currentBuffer) {
            memcpy(fBuffer[fCurrentBufferIndex]->contents(), currentBuffer->contents(), fBufferSize);
            if (GetBuffer()->storageMode() == MTL::StorageModeManaged) {
                GetBuffer()->didModifyRange(NS::Range(0, sizeof(plMetalLayer) * fNumElements));
            }
        }
    }

    // Older tier 1 devices don't have aligned structs and pointer sizes with the CPU
    // so an encoder has to be used as a go between.
    // Tier 2 devices are aligned with the CPU so we can send structs and pointers
    // directly without an encoder.
    if (fTier == plMetalArgumentBufferTier::Tier1) {
        auto &layerRecord = fLayers[layerIndex];
        plBitmap* texture = layer->GetTexture();
        fEncoder->setArgumentBuffer(GetBuffer(), 0, layerIndex);
        if (texture == nullptr) {
            layerRecord.texture = nullptr;
            layerRecord.texture3D = nullptr;
            return;
        }
        plMetalTextureRef* deviceTexture = (plMetalTextureRef*)texture->GetDeviceRef();
        if (!deviceTexture) {
            layerRecord.texture = nullptr;
            layerRecord.texture3D = nullptr;
            return;
        }
        if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
            layerRecord.texture3D = deviceTexture->fTexture;
            layerRecord.texture = nullptr;
            fEncoder->setTexture(deviceTexture->fTexture, 1);
        } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
            layerRecord.texture = deviceTexture->fTexture;
            layerRecord.texture3D = nullptr;
            fEncoder->setTexture(deviceTexture->fTexture, 0);
        }

        MTL::SamplerState* samplerState = fDevice->SampleStateForClampFlags(hsGMatState::hsGMatClampFlags(layer->GetClampFlags()));
        fLayers[layerIndex].sampler = samplerState;
        fEncoder->setSamplerState(samplerState, 2);
    }
#ifdef METAL_3_SDK
    else {
        // Even though this path could track state differences
        // directly with the buffer, still populate the record.
        // Other functions in this class check the record and we don't
        // need to fork those between tier 1/2.
        auto &layerRecord = fLayers[layerIndex];
        auto &buffer = fValue[layerIndex];
        plBitmap* texture = layer->GetTexture();
        if (texture == nullptr) {
            layerRecord.texture = nullptr;
            layerRecord.texture3D = nullptr;
            return;
        }
        plMetalTextureRef* deviceTexture = (plMetalTextureRef*)texture->GetDeviceRef();
        if (!deviceTexture) {
            layerRecord.texture = nullptr;
            layerRecord.texture3D = nullptr;
            return;
        }
        if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
            layerRecord.texture3D = deviceTexture->fTexture;
            layerRecord.texture = nullptr;
            buffer.texture3D = deviceTexture->fTexture->gpuResourceID();
        } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
            layerRecord.texture = deviceTexture->fTexture;
            buffer.texture = deviceTexture->fTexture->gpuResourceID();
        }

        MTL::SamplerState* samplerState = fDevice->SampleStateForClampFlags(hsGMatState::hsGMatClampFlags(layer->GetClampFlags()));
        layerRecord.sampler = samplerState;
        buffer.sampler = samplerState->gpuResourceID();
    }
#endif
    if (GetBuffer()->storageMode() == MTL::StorageModeManaged) {
        GetBuffer()->didModifyRange(NS::Range(sizeof(plMetalLayer) * layerIndex, sizeof(plMetalLayer)));
    }
}

bool plMetalLayerListArgumentBuffer::CheckBuffer(const plLayerInterface* layer, const size_t i)
{
    plBitmap* texture = layer->GetTexture();
    if ((texture == nullptr) != (fLayers[i].texture == nullptr)) {
        return false;
    }
    // If texture is null, then that implies fLayers[i].texture is null
    // See check above
    if (texture == nullptr) {
        return true;
    }
    plMetalTextureRef* deviceTexture = (plMetalTextureRef*)texture->GetDeviceRef();
    if (!deviceTexture) {
        if (fLayers[i].texture != nullptr && fLayers[i].texture3D != nullptr)
            return false;
    }
    if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
        if (fLayers[i].texture3D != deviceTexture->fTexture)
            return false;
    } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
        if (fLayers[i].texture != deviceTexture->fTexture)
            return false;
    }

    
    MTL::SamplerState* samplerState = fDevice->SampleStateForClampFlags(hsGMatState::hsGMatClampFlags(layer->GetClampFlags()));
    if (fLayers[i].sampler != samplerState)
        return false;
    return true;
}

void plMetalLayerListArgumentBuffer::Bind(MTL::RenderCommandEncoder* encoder)
{
    for (const auto& layer : fLayers) {
        // These textures can't go into a heap because they're shared by multiple
        // materials, boo. Mark them as needing to be resident one at a time.
        if (layer.texture!=nullptr)
            encoder->useResource(layer.texture, MTL::ResourceUsageRead, MTL::RenderStageFragment);
        if (layer.texture3D!=nullptr)
            encoder->useResource(layer.texture3D, MTL::ResourceUsageRead, MTL::RenderStageFragment);
    }
    encoder->setFragmentBuffer(GetBuffer(), 0, FragmentShaderLayers);
    fBoundBufferIndex = fCurrentBufferIndex;
}
