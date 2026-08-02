/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

*==LICENSE==*/

#include "plMetalDevice.h"

#include <algorithm>
#include <cstring>

namespace
{
    constexpr NS::UInteger kGTAOMipCount = 5;

    NS::UInteger IDivRoundUp(NS::UInteger value, NS::UInteger divisor)
    {
        return (value + divisor - 1) / divisor;
    }
}

void plMetalDevice::IDestroyGTAO()
{
    MTL::ComputePipelineState** computeStates[] = {
        &fGTAOPrefilterState, &fGTAOPrefilterMSState,
        &fGTAONormalsState, &fGTAONormalsMSState,
        &fGTAOMainState, &fGTAODenoiseState
    };
    for (MTL::ComputePipelineState** state : computeStates) {
        if (*state)
            (*state)->release();
        *state = nullptr;
    }
    if (fGTAOCompositeState)
        fGTAOCompositeState->release();
    fGTAOCompositeState = nullptr;

    for (MTL::Texture*& mip : fGTAODepthMips) {
        if (mip)
            mip->release();
        mip = nullptr;
    }
    MTL::Texture** textures[] = {
        &fGTAODepth, &fGTAONormals, &fGTAOWorkingAO, &fGTAOEdges, &fGTAOFinalAO
    };
    for (MTL::Texture** texture : textures) {
        if (*texture)
            (*texture)->release();
        *texture = nullptr;
    }

    fGTAOWidth = 0;
    fGTAOHeight = 0;
    fGTAOSampleCount = 0;
    fGTAOColorFormat = MTL::PixelFormatInvalid;
}

bool plMetalDevice::IEnsureGTAO()
{
    if (!fCurrentDrawableDepthTexture || !fCurrentFragmentOutputTexture)
        return false;

    const NS::UInteger width = fCurrentDrawableDepthTexture->width();
    const NS::UInteger height = fCurrentDrawableDepthTexture->height();
    const MTL::PixelFormat colorFormat = fCurrentFragmentOutputTexture->pixelFormat();
    if (fGTAOCompositeState && fGTAOWidth == width && fGTAOHeight == height &&
        fGTAOSampleCount == fSampleCount && fGTAOColorFormat == colorFormat)
        return true;
    if (fGTAOWarned && !fGTAOCompositeState)
        return false;

    IDestroyGTAO();

    auto fail = [this](const char* operation, NS::Error* error = nullptr) {
        if (!fGTAOWarned) {
            if (error) {
                hsStatusMessageF("Metal: GTAO setup failed at {}: {}", operation,
                                 error->localizedDescription()->cString(NS::UTF8StringEncoding));
            } else {
                hsStatusMessageF("Metal: GTAO setup failed at {}", operation);
            }
            fGTAOWarned = true;
        }
        IDestroyGTAO();
        return false;
    };

    MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatR32Float, width, height, true);
    descriptor->setMipmapLevelCount(kGTAOMipCount);
    descriptor->setStorageMode(MTL::StorageModePrivate);
    descriptor->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
    fGTAODepth = fMetalDevice->newTexture(descriptor);
    if (!fGTAODepth)
        return fail("view-depth texture creation");
    fGTAODepth->setLabel(MTLSTR("GTAO view depth"));
    for (NS::UInteger mip = 0; mip < kGTAOMipCount; ++mip) {
        fGTAODepthMips[mip] = fGTAODepth->newTextureView(
            MTL::PixelFormatR32Float, MTL::TextureType2D,
            NS::Range::Make(mip, 1), NS::Range::Make(0, 1));
        if (!fGTAODepthMips[mip])
            return fail("view-depth mip creation");
    }

    auto createTexture = [this, width, height](MTL::PixelFormat format,
                                               const char* label) {
        MTL::TextureDescriptor* textureDescriptor =
            MTL::TextureDescriptor::texture2DDescriptor(format, width, height, false);
        textureDescriptor->setStorageMode(MTL::StorageModePrivate);
        textureDescriptor->setUsage(MTL::TextureUsageShaderRead |
                                    MTL::TextureUsageShaderWrite);
        MTL::Texture* texture = fMetalDevice->newTexture(textureDescriptor);
        if (texture)
            texture->setLabel(NS::String::string(label, NS::UTF8StringEncoding));
        return texture;
    };
    fGTAONormals = createTexture(MTL::PixelFormatR32Uint, "GTAO normals");
    fGTAOWorkingAO = createTexture(MTL::PixelFormatR8Uint, "GTAO working visibility");
    fGTAOEdges = createTexture(MTL::PixelFormatR8Unorm, "GTAO edges");
    fGTAOFinalAO = createTexture(MTL::PixelFormatR8Uint, "GTAO final visibility");
    if (!fGTAONormals || !fGTAOWorkingAO || !fGTAOEdges || !fGTAOFinalAO)
        return fail("intermediate texture creation");

    auto createComputeState = [this](const char* name, MTL::ComputePipelineState*& state,
                                     NS::Error*& error) {
        MTL::Function* function = fShaderLibrary->newFunction(
            NS::String::string(name, NS::UTF8StringEncoding));
        if (!function)
            return false;
        state = fMetalDevice->newComputePipelineState(function, &error);
        function->release();
        return state != nullptr;
    };
    NS::Error* error = nullptr;
    if (!createComputeState("gtaoPrefilter", fGTAOPrefilterState, error) ||
        !createComputeState("gtaoPrefilterMS", fGTAOPrefilterMSState, error) ||
        !createComputeState("gtaoNormals", fGTAONormalsState, error) ||
        !createComputeState("gtaoNormalsMS", fGTAONormalsMSState, error) ||
        !createComputeState("gtaoMain", fGTAOMainState, error) ||
        !createComputeState("gtaoDenoise", fGTAODenoiseState, error))
        return fail("compute-pipeline creation", error);

    MTL::Function* vertexFunction = fShaderLibrary->newFunction(MTLSTR("gtaoCompositeVertex"));
    MTL::Function* fragmentFunction = fShaderLibrary->newFunction(MTLSTR("gtaoCompositeFragment"));
    if (!vertexFunction || !fragmentFunction) {
        if (vertexFunction)
            vertexFunction->release();
        if (fragmentFunction)
            fragmentFunction->release();
        return fail("composite shader lookup");
    }

    MTL::RenderPipelineDescriptor* pipelineDescriptor =
        MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setLabel(MTLSTR("XeGTAO composite"));
    pipelineDescriptor->setVertexFunction(vertexFunction);
    pipelineDescriptor->setFragmentFunction(fragmentFunction);
    pipelineDescriptor->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float_Stencil8);
    pipelineDescriptor->setSampleCount(fSampleCount);
    MTL::RenderPipelineColorAttachmentDescriptor* color =
        pipelineDescriptor->colorAttachments()->object(0);
    color->setPixelFormat(colorFormat);
    color->setBlendingEnabled(true);
    color->setSourceRGBBlendFactor(MTL::BlendFactorZero);
    color->setDestinationRGBBlendFactor(MTL::BlendFactorSourceColor);
    color->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
    color->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
    error = nullptr;
    fGTAOCompositeState = fMetalDevice->newRenderPipelineState(pipelineDescriptor, &error);
    pipelineDescriptor->release();
    vertexFunction->release();
    fragmentFunction->release();
    if (!fGTAOCompositeState)
        return fail("composite-pipeline creation", error);

    fGTAOWidth = width;
    fGTAOHeight = height;
    fGTAOSampleCount = fSampleCount;
    fGTAOColorFormat = colorFormat;
    fGTAOWarned = false;
    return true;
}

void plMetalDevice::ApplyGTAO()
{
    if (!fGTAOSettings.fEnabled || fCurrentRenderTarget || !fCurrentCommandBuffer ||
        !fCurrentRenderTargetCommandEncoder || !fCurrentDrawableDepthTexture ||
        (fCurrentDrawableDepthTexture->usage() & MTL::TextureUsageShaderRead) == 0 ||
        !IEnsureGTAO())
        return;

    plGTAOConstants constants{};
    const float* projection = reinterpret_cast<const float*>(&fMatrixProj);
    if (!plUpdateGTAOConstants(constants, uint32_t(fGTAOWidth), uint32_t(fGTAOHeight),
                               fGTAOSettings, projection))
        return;

    fCurrentRenderTargetCommandEncoder->endEncoding();
    fCurrentRenderTargetCommandEncoder->release();
    fCurrentRenderTargetCommandEncoder = nullptr;

    const MTL::Size threadsPerGroup(8, 8, 1);
    auto dispatch = [this, threadsPerGroup](MTL::ComputePipelineState* state,
                                           NS::UInteger width, NS::UInteger height,
                                           auto&& encodeResources,
                                           bool needsThreadgroupMemory = false) {
        MTL::ComputeCommandEncoder* encoder = fCurrentCommandBuffer->computeCommandEncoder();
        encoder->setComputePipelineState(state);
        encodeResources(encoder);
        if (needsThreadgroupMemory)
            encoder->setThreadgroupMemoryLength(64 * sizeof(float), 0);
        encoder->dispatchThreadgroups(
            MTL::Size(IDivRoundUp(width, 8), IDivRoundUp(height, 8), 1),
            threadsPerGroup);
        encoder->endEncoding();
    };

    dispatch(fSampleCount == 1 ? fGTAOPrefilterState : fGTAOPrefilterMSState,
             IDivRoundUp(fGTAOWidth, 2), IDivRoundUp(fGTAOHeight, 2),
             [this, &constants](MTL::ComputeCommandEncoder* encoder) {
                 encoder->setBytes(&constants, sizeof(constants), 0);
                 encoder->setTexture(fCurrentDrawableDepthTexture, 0);
                 for (NS::UInteger mip = 0; mip < kGTAOMipCount; ++mip)
                     encoder->setTexture(fGTAODepthMips[mip], mip + 1);
             }, true);

    dispatch(fSampleCount == 1 ? fGTAONormalsState : fGTAONormalsMSState,
             fGTAOWidth, fGTAOHeight,
             [this, &constants](MTL::ComputeCommandEncoder* encoder) {
                 encoder->setBytes(&constants, sizeof(constants), 0);
                 encoder->setTexture(fCurrentDrawableDepthTexture, 0);
                 encoder->setTexture(fGTAONormals, 1);
             });

    struct Samples
    {
        uint32_t fSlices;
        uint32_t fSteps;
    } samples{};
    plGTAOQualitySamples(fGTAOSettings.fQuality, samples.fSlices, samples.fSteps);
    dispatch(fGTAOMainState, fGTAOWidth, fGTAOHeight,
             [this, &constants, &samples](MTL::ComputeCommandEncoder* encoder) {
                 encoder->setBytes(&constants, sizeof(constants), 0);
                 encoder->setBytes(&samples, sizeof(samples), 1);
                 encoder->setTexture(fGTAODepth, 0);
                 encoder->setTexture(fGTAONormals, 1);
                 encoder->setTexture(fGTAOWorkingAO, 2);
                 encoder->setTexture(fGTAOEdges, 3);
             });

    dispatch(fGTAODenoiseState, fGTAOWidth, fGTAOHeight,
             [this, &constants](MTL::ComputeCommandEncoder* encoder) {
                 encoder->setBytes(&constants, sizeof(constants), 0);
                 encoder->setTexture(fGTAOWorkingAO, 0);
                 encoder->setTexture(fGTAOEdges, 1);
                 encoder->setTexture(fGTAOFinalAO, 2);
             });

    fGTAOResumePass = true;
    BeginNewRenderPass();
    fCurrentRenderTargetCommandEncoder->setRenderPipelineState(fGTAOCompositeState);
    fCurrentRenderTargetCommandEncoder->setFragmentTexture(fGTAOFinalAO, 0);
    fCurrentRenderTargetCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle,
                                                       NS::UInteger(0), NS::UInteger(3));
}
