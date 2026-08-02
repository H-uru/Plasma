/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

*==LICENSE==*/

#include "plVulkanDevice.h"

#include "plVulkanShaders.h"

#include <vk_mem_alloc.h>

#include <string_theory/format>

#include <algorithm>
#include <array>
#include <cstring>

namespace
{
    constexpr uint32_t kDepthMipCount = 5;

    uint32_t IDivRoundUp(uint32_t value, uint32_t divisor)
    {
        return (value + divisor - 1) / divisor;
    }
}

bool plVulkanDevice::ICreateGTAOImage(VkFormat format, uint32_t mipLevels,
                                      VkImageUsageFlags usage, const ST::string& name,
                                      VkImage& image, VmaAllocation& allocation,
                                      VkImageView& sampledView,
                                      VkImageView storageViews[5])
{
    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { fExtent.width, fExtent.height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    VkResult result = vmaCreateImage(fAllocator, &imageInfo, &allocationInfo,
                                     &image, &allocation, nullptr);
    if (result != VK_SUCCESS)
        return false;

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = 1;
    result = vkCreateImageView(fDevice, &viewInfo, nullptr, &sampledView);
    if (result != VK_SUCCESS)
        return false;

    for (uint32_t mip = 0; mip < mipLevels; ++mip) {
        viewInfo.subresourceRange.baseMipLevel = mip;
        viewInfo.subresourceRange.levelCount = 1;
        result = vkCreateImageView(fDevice, &viewInfo, nullptr, &storageViews[mip]);
        if (result != VK_SUCCESS)
            return false;
        SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, storageViews[mip],
                      ST::format("{} mip {}", name, mip));
    }

    SetObjectName(VK_OBJECT_TYPE_IMAGE, image, name);
    SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, sampledView, ST::format("{} sampled", name));
    return true;
}

void plVulkanDevice::IDestroyGTAO()
{
    if (fDevice == VK_NULL_HANDLE)
        return;

    const std::array<VkPipeline*, 7> pipelines{
        &fGTAOPrefilterPipeline, &fGTAOPrefilterMSPipeline,
        &fGTAONormalsPipeline, &fGTAONormalsMSPipeline,
        &fGTAOMainPipeline, &fGTAODenoisePipeline, &fGTAOCompositePipeline
    };
    for (VkPipeline* pipeline : pipelines) {
        if (*pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(fDevice, *pipeline, nullptr);
        *pipeline = VK_NULL_HANDLE;
    }

    const std::array<VkShaderModule*, 7> modules{
        &fGTAOPrefilterModule, &fGTAOPrefilterMSModule,
        &fGTAONormalsModule, &fGTAONormalsMSModule,
        &fGTAOMainModule, &fGTAODenoiseModule, &fGTAOCompositeModule
    };
    for (VkShaderModule* module : modules) {
        if (*module != VK_NULL_HANDLE)
            vkDestroyShaderModule(fDevice, *module, nullptr);
        *module = VK_NULL_HANDLE;
    }

    if (fGTAOPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(fDevice, fGTAOPipelineLayout, nullptr);
    if (fGTAODescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(fDevice, fGTAODescriptorPool, nullptr);
    if (fGTAODescriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(fDevice, fGTAODescriptorSetLayout, nullptr);
    if (fGTAOPointSampler != VK_NULL_HANDLE)
        vkDestroySampler(fDevice, fGTAOPointSampler, nullptr);
    if (fGTAOLinearSampler != VK_NULL_HANDLE)
        vkDestroySampler(fDevice, fGTAOLinearSampler, nullptr);
    fGTAOPipelineLayout = VK_NULL_HANDLE;
    fGTAODescriptorPool = VK_NULL_HANDLE;
    fGTAODescriptorSetLayout = VK_NULL_HANDLE;
    fGTAOPointSampler = VK_NULL_HANDLE;
    fGTAOLinearSampler = VK_NULL_HANDLE;
    std::fill(std::begin(fGTAODescriptorSets), std::end(fGTAODescriptorSets), VK_NULL_HANDLE);

    for (plVulkanBuffer& buffer : fGTAOConstants) {
        if (buffer.IsValid())
            vmaDestroyBuffer(fAllocator, buffer.fBuffer, buffer.fAllocation);
        buffer = {};
    }

    auto destroyImage = [this](VkImage& image, VmaAllocation& allocation,
                               VkImageView& sampledView, VkImageView storageViews[5]) {
        for (uint32_t mip = 0; mip < kDepthMipCount; ++mip) {
            if (storageViews[mip] != VK_NULL_HANDLE)
                vkDestroyImageView(fDevice, storageViews[mip], nullptr);
            storageViews[mip] = VK_NULL_HANDLE;
        }
        if (sampledView != VK_NULL_HANDLE)
            vkDestroyImageView(fDevice, sampledView, nullptr);
        if (image != VK_NULL_HANDLE)
            vmaDestroyImage(fAllocator, image, allocation);
        sampledView = VK_NULL_HANDLE;
        image = VK_NULL_HANDLE;
        allocation = nullptr;
    };

    destroyImage(fGTAODepthImage, fGTAODepthAllocation,
                 fGTAODepthView, fGTAODepthMipViews);
    destroyImage(fGTAONormalsImage, fGTAONormalsAllocation,
                 fGTAONormalsView, fGTAONormalsStorageViews);
    destroyImage(fGTAOWorkingAOImage, fGTAOWorkingAOAllocation,
                 fGTAOWorkingAOView, fGTAOWorkingAOStorageViews);
    destroyImage(fGTAOEdgesImage, fGTAOEdgesAllocation,
                 fGTAOEdgesView, fGTAOEdgesStorageViews);
    destroyImage(fGTAOFinalAOImage, fGTAOFinalAOAllocation,
                 fGTAOFinalAOView, fGTAOFinalAOStorageViews);

    fGTAOExtent = {};
    fGTAOColorFormat = VK_FORMAT_UNDEFINED;
    fGTAOSampleCount = VK_SAMPLE_COUNT_1_BIT;
    fGTAOImagesInitialized = false;
}

bool plVulkanDevice::IEnsureGTAO()
{
    if (fGTAODescriptorSetLayout != VK_NULL_HANDLE &&
        fGTAOExtent.width == fExtent.width && fGTAOExtent.height == fExtent.height &&
        fGTAOColorFormat == fSwapFormat && fGTAOSampleCount == fSampleCount)
        return true;
    if (fGTAOWarned && fGTAODescriptorSetLayout == VK_NULL_HANDLE)
        return false;

    IDestroyGTAO();

    const struct FormatRequirement
    {
        VkFormat fFormat;
        VkFormatFeatureFlags fFeatures;
    } requirements[] = {
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT },
        { VK_FORMAT_R32_SFLOAT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT },
        { VK_FORMAT_R32_UINT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                             VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT },
        { VK_FORMAT_R8_UINT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                            VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT },
        { VK_FORMAT_R8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                             VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT },
    };
    for (const FormatRequirement& requirement : requirements) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(fPhysicalDevice, requirement.fFormat, &properties);
        if ((properties.optimalTilingFeatures & requirement.fFeatures) != requirement.fFeatures) {
            if (!fGTAOWarned) {
                hsStatusMessage("Vulkan: GTAO disabled because a required image format is unsupported");
                fGTAOWarned = true;
            }
            return false;
        }
    }

    auto fail = [this](const char* operation) {
        if (!fGTAOWarned) {
            hsStatusMessageF("Vulkan: GTAO setup failed at {}", operation);
            fGTAOWarned = true;
        }
        IDestroyGTAO();
        return false;
    };

    if (!ICreateGTAOImage(VK_FORMAT_R32_SFLOAT, kDepthMipCount,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                          ST_LITERAL("GTAO view depth"), fGTAODepthImage,
                          fGTAODepthAllocation, fGTAODepthView, fGTAODepthMipViews))
        return fail("view-depth image creation");
    if (!ICreateGTAOImage(VK_FORMAT_R32_UINT, 1,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                          ST_LITERAL("GTAO normals"), fGTAONormalsImage,
                          fGTAONormalsAllocation, fGTAONormalsView,
                          fGTAONormalsStorageViews))
        return fail("normal image creation");
    if (!ICreateGTAOImage(VK_FORMAT_R8_UINT, 1,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                          ST_LITERAL("GTAO working visibility"), fGTAOWorkingAOImage,
                          fGTAOWorkingAOAllocation, fGTAOWorkingAOView,
                          fGTAOWorkingAOStorageViews))
        return fail("working visibility image creation");
    if (!ICreateGTAOImage(VK_FORMAT_R8_UNORM, 1,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                          ST_LITERAL("GTAO edges"), fGTAOEdgesImage,
                          fGTAOEdgesAllocation, fGTAOEdgesView,
                          fGTAOEdgesStorageViews))
        return fail("edge image creation");
    if (!ICreateGTAOImage(VK_FORMAT_R8_UINT, 1,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                          ST_LITERAL("GTAO final visibility"), fGTAOFinalAOImage,
                          fGTAOFinalAOAllocation, fGTAOFinalAOView,
                          fGTAOFinalAOStorageViews))
        return fail("final visibility image creation");

    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        fGTAOConstants[frame] = CreateBuffer(sizeof(plGTAOConstants),
                                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true,
                                             ST::format("GTAO constants {}", frame));
        if (!fGTAOConstants[frame].IsValid())
            return fail("constant-buffer creation");
    }

    VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxLod = 4.f;
    if (vkCreateSampler(fDevice, &samplerInfo, nullptr, &fGTAOPointSampler) != VK_SUCCESS)
        return fail("point sampler creation");
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    if (vkCreateSampler(fDevice, &samplerInfo, nullptr, &fGTAOLinearSampler) != VK_SUCCESS)
        return fail("linear sampler creation");

    std::array<VkDescriptorSetLayoutBinding, 16> bindings{};
    const uint32_t bindingNumbers[] = { 0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    for (size_t index = 0; index < bindings.size(); ++index) {
        VkDescriptorSetLayoutBinding& binding = bindings[index];
        binding.binding = bindingNumbers[index];
        binding.descriptorCount = 1;
        binding.descriptorType = binding.binding == 0 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                               : (binding.binding >= 7 && binding.binding <= 15)
                                   ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                                   : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = binding.binding == 16 ? VK_SHADER_STAGE_FRAGMENT_BIT
                                                   : VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo setLayoutInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    setLayoutInfo.bindingCount = uint32_t(bindings.size());
    setLayoutInfo.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(fDevice, &setLayoutInfo, nullptr,
                                    &fGTAODescriptorSetLayout) != VK_SUCCESS)
        return fail("descriptor-set layout creation");

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushRange.size = sizeof(uint32_t) * 2;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &fGTAODescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
    if (vkCreatePipelineLayout(fDevice, &pipelineLayoutInfo, nullptr,
                               &fGTAOPipelineLayout) != VK_SUCCESS)
        return fail("pipeline layout creation");

    const VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kMaxFramesInFlight },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6 * kMaxFramesInFlight },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 9 * kMaxFramesInFlight },
    };
    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.maxSets = kMaxFramesInFlight;
    poolInfo.poolSizeCount = uint32_t(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;
    if (vkCreateDescriptorPool(fDevice, &poolInfo, nullptr, &fGTAODescriptorPool) != VK_SUCCESS)
        return fail("descriptor pool creation");

    const VkDescriptorSetLayout setLayouts[] = {
        fGTAODescriptorSetLayout, fGTAODescriptorSetLayout
    };
    VkDescriptorSetAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocateInfo.descriptorPool = fGTAODescriptorPool;
    allocateInfo.descriptorSetCount = kMaxFramesInFlight;
    allocateInfo.pSetLayouts = setLayouts;
    if (vkAllocateDescriptorSets(fDevice, &allocateInfo, fGTAODescriptorSets) != VK_SUCCESS)
        return fail("descriptor-set allocation");

    auto makeModule = [this](const plVulkanShaderBlob& blob, VkShaderModule& module) {
        VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleInfo.codeSize = blob.fSizeInBytes;
        moduleInfo.pCode = blob.fWords;
        return vkCreateShaderModule(fDevice, &moduleInfo, nullptr, &module) == VK_SUCCESS;
    };
    if (!makeModule(kGTAOPrefilterShader, fGTAOPrefilterModule) ||
        !makeModule(kGTAOPrefilterMSShader, fGTAOPrefilterMSModule) ||
        !makeModule(kGTAONormalsShader, fGTAONormalsModule) ||
        !makeModule(kGTAONormalsMSShader, fGTAONormalsMSModule) ||
        !makeModule(kGTAOMainShader, fGTAOMainModule) ||
        !makeModule(kGTAODenoiseShader, fGTAODenoiseModule) ||
        !makeModule(kGTAOCompositeShader, fGTAOCompositeModule))
        return fail("shader-module creation");

    auto makeComputePipeline = [this](VkShaderModule module, VkPipeline& pipeline) {
        VkPipelineShaderStageCreateInfo stage{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = module;
        stage.pName = "main";
        VkComputePipelineCreateInfo info{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        info.stage = stage;
        info.layout = fGTAOPipelineLayout;
        return vkCreateComputePipelines(fDevice, fPipelineCache, 1, &info,
                                        nullptr, &pipeline) == VK_SUCCESS;
    };
    if (!makeComputePipeline(fGTAOPrefilterModule, fGTAOPrefilterPipeline) ||
        !makeComputePipeline(fGTAOPrefilterMSModule, fGTAOPrefilterMSPipeline) ||
        !makeComputePipeline(fGTAONormalsModule, fGTAONormalsPipeline) ||
        !makeComputePipeline(fGTAONormalsMSModule, fGTAONormalsMSPipeline) ||
        !makeComputePipeline(fGTAOMainModule, fGTAOMainPipeline) ||
        !makeComputePipeline(fGTAODenoiseModule, fGTAODenoisePipeline))
        return fail("compute-pipeline creation");

    VkPipelineShaderStageCreateInfo graphicsStages[2]{};
    graphicsStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    graphicsStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    graphicsStages[0].module = fFullscreenVertexShader;
    graphicsStages[0].pName = "main";
    graphicsStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    graphicsStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    graphicsStages[1].module = fGTAOCompositeModule;
    graphicsStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPipelineViewportStateCreateInfo viewport{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewport.viewportCount = 1;
    viewport.scissorCount = 1;
    VkPipelineRasterizationStateCreateInfo raster{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_NONE;
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.lineWidth = 1.f;
    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisample.rasterizationSamples = fSampleCount;
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.blendEnable = VK_TRUE;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo colorBlend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blendAttachment;
    const VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic.dynamicStateCount = uint32_t(std::size(dynamicStates));
    dynamic.pDynamicStates = dynamicStates;
    VkPipelineRenderingCreateInfo rendering{
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachmentFormats = &fSwapFormat;
    rendering.depthAttachmentFormat = fDepthFormat;
    VkGraphicsPipelineCreateInfo graphicsInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    graphicsInfo.pNext = &rendering;
    graphicsInfo.stageCount = 2;
    graphicsInfo.pStages = graphicsStages;
    graphicsInfo.pVertexInputState = &vertexInput;
    graphicsInfo.pInputAssemblyState = &inputAssembly;
    graphicsInfo.pViewportState = &viewport;
    graphicsInfo.pRasterizationState = &raster;
    graphicsInfo.pMultisampleState = &multisample;
    graphicsInfo.pDepthStencilState = &depthStencil;
    graphicsInfo.pColorBlendState = &colorBlend;
    graphicsInfo.pDynamicState = &dynamic;
    graphicsInfo.layout = fGTAOPipelineLayout;
    if (vkCreateGraphicsPipelines(fDevice, fPipelineCache, 1, &graphicsInfo,
                                  nullptr, &fGTAOCompositePipeline) != VK_SUCCESS)
        return fail("composite-pipeline creation");

    for (uint32_t frame = 0; frame < kMaxFramesInFlight; ++frame) {
        VkDescriptorBufferInfo constantInfo{
            fGTAOConstants[frame].fBuffer, 0, sizeof(plGTAOConstants) };
        std::array<VkDescriptorImageInfo, 15> imageInfos{};
        auto sampled = [](VkSampler sampler, VkImageView view, VkImageLayout layout) {
            return VkDescriptorImageInfo{ sampler, view, layout };
        };
        imageInfos[0] = sampled(fGTAOPointSampler, fDepthView,
                                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
        imageInfos[1] = sampled(fGTAOLinearSampler, fGTAODepthView,
                                VK_IMAGE_LAYOUT_GENERAL);
        imageInfos[2] = sampled(fGTAOPointSampler, fGTAONormalsView,
                                VK_IMAGE_LAYOUT_GENERAL);
        imageInfos[3] = sampled(fGTAOPointSampler, fGTAOWorkingAOView,
                                VK_IMAGE_LAYOUT_GENERAL);
        imageInfos[4] = sampled(fGTAOPointSampler, fGTAOEdgesView,
                                VK_IMAGE_LAYOUT_GENERAL);
        for (uint32_t mip = 0; mip < kDepthMipCount; ++mip)
            imageInfos[5 + mip] = { VK_NULL_HANDLE, fGTAODepthMipViews[mip],
                                    VK_IMAGE_LAYOUT_GENERAL };
        imageInfos[10] = { VK_NULL_HANDLE, fGTAONormalsStorageViews[0],
                           VK_IMAGE_LAYOUT_GENERAL };
        imageInfos[11] = { VK_NULL_HANDLE, fGTAOWorkingAOStorageViews[0],
                           VK_IMAGE_LAYOUT_GENERAL };
        imageInfos[12] = { VK_NULL_HANDLE, fGTAOEdgesStorageViews[0],
                           VK_IMAGE_LAYOUT_GENERAL };
        imageInfos[13] = { VK_NULL_HANDLE, fGTAOFinalAOStorageViews[0],
                           VK_IMAGE_LAYOUT_GENERAL };
        imageInfos[14] = sampled(fGTAOPointSampler, fGTAOFinalAOView,
                                 VK_IMAGE_LAYOUT_GENERAL);

        const uint32_t writeBindings[] = {
            0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
        };
        std::array<VkWriteDescriptorSet, 16> writes{};
        for (size_t index = 0; index < writes.size(); ++index) {
            VkWriteDescriptorSet& write = writes[index];
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = fGTAODescriptorSets[frame];
            write.dstBinding = writeBindings[index];
            write.descriptorCount = 1;
            if (index == 0) {
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.pBufferInfo = &constantInfo;
            } else {
                write.descriptorType = (write.dstBinding >= 7 && write.dstBinding <= 15)
                                     ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                                     : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &imageInfos[index - 1];
            }
        }
        vkUpdateDescriptorSets(fDevice, uint32_t(writes.size()), writes.data(), 0, nullptr);
    }

    fGTAOExtent = fExtent;
    fGTAOColorFormat = fSwapFormat;
    fGTAOSampleCount = fSampleCount;
    fGTAOImagesInitialized = false;
    fGTAOWarned = false;
    return true;
}

void plVulkanDevice::ApplyGTAO()
{
    if (!fGTAOSettings.fEnabled || !fFrameOpen || fCurrentTarget ||
        fDepthImage == VK_NULL_HANDLE || !IEnsureGTAO())
        return;

    plGTAOConstants constants{};
    if (!plUpdateGTAOConstants(constants, fExtent.width, fExtent.height,
                               fGTAOSettings, fMatrixProj.m))
        return;
    std::memcpy(fGTAOConstants[fFrameIndex].fMapped, &constants, sizeof(constants));

    IEndRendering();
    VkCommandBuffer cmd = fFrames[fFrameIndex].fCmd;
    plVulkanDebugLabel label(*this, cmd, ST_LITERAL("XeGTAO"));

    VkImageMemoryBarrier2 depthToRead{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    depthToRead.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                               VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    depthToRead.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthToRead.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    depthToRead.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    depthToRead.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthToRead.newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthToRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthToRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthToRead.image = fDepthImage;
    depthToRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthToRead.subresourceRange.levelCount = 1;
    depthToRead.subresourceRange.layerCount = 1;

    const VkImage gtaoImages[] = {
        fGTAODepthImage, fGTAONormalsImage, fGTAOWorkingAOImage,
        fGTAOEdgesImage, fGTAOFinalAOImage
    };
    std::array<VkImageMemoryBarrier2, 6> initialBarriers{};
    initialBarriers[0] = depthToRead;
    for (size_t index = 0; index < std::size(gtaoImages); ++index) {
        VkImageMemoryBarrier2& barrier = initialBarriers[index + 1];
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = fGTAOImagesInitialized
                             ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT |
                               VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                             : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.srcAccessMask = fGTAOImagesInitialized
                              ? VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT
                              : VK_ACCESS_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
        barrier.oldLayout = fGTAOImagesInitialized ? VK_IMAGE_LAYOUT_GENERAL
                                                   : VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = gtaoImages[index];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = index == 0 ? kDepthMipCount : 1;
        barrier.subresourceRange.layerCount = 1;
    }
    VkDependencyInfo initialDependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    initialDependency.imageMemoryBarrierCount = uint32_t(initialBarriers.size());
    initialDependency.pImageMemoryBarriers = initialBarriers.data();
    vkCmdPipelineBarrier2(cmd, &initialDependency);
    fGTAOImagesInitialized = true;

    VkDescriptorSet descriptorSet = fGTAODescriptorSets[fFrameIndex];
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, fGTAOPipelineLayout,
                            0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                      fSampleCount == VK_SAMPLE_COUNT_1_BIT
                          ? fGTAOPrefilterPipeline : fGTAOPrefilterMSPipeline);
    vkCmdDispatch(cmd, IDivRoundUp(fExtent.width, 16),
                  IDivRoundUp(fExtent.height, 16), 1);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                      fSampleCount == VK_SAMPLE_COUNT_1_BIT
                          ? fGTAONormalsPipeline : fGTAONormalsMSPipeline);
    vkCmdDispatch(cmd, IDivRoundUp(fExtent.width, 8),
                  IDivRoundUp(fExtent.height, 8), 1);

    VkMemoryBarrier2 computeBarrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
    computeBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    computeBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    computeBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    computeBarrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    VkDependencyInfo computeDependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    computeDependency.memoryBarrierCount = 1;
    computeDependency.pMemoryBarriers = &computeBarrier;
    vkCmdPipelineBarrier2(cmd, &computeDependency);

    uint32_t samples[2]{};
    plGTAOQualitySamples(fGTAOSettings.fQuality, samples[0], samples[1]);
    vkCmdPushConstants(cmd, fGTAOPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(samples), samples);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, fGTAOMainPipeline);
    vkCmdDispatch(cmd, IDivRoundUp(fExtent.width, 8),
                  IDivRoundUp(fExtent.height, 8), 1);
    vkCmdPipelineBarrier2(cmd, &computeDependency);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, fGTAODenoisePipeline);
    vkCmdDispatch(cmd, IDivRoundUp(fExtent.width, 8),
                  IDivRoundUp(fExtent.height, 8), 1);

    VkImageMemoryBarrier2 finalToFragment{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    finalToFragment.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    finalToFragment.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    finalToFragment.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    finalToFragment.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    finalToFragment.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    finalToFragment.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    finalToFragment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    finalToFragment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    finalToFragment.image = fGTAOFinalAOImage;
    finalToFragment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    finalToFragment.subresourceRange.levelCount = 1;
    finalToFragment.subresourceRange.layerCount = 1;

    VkImageMemoryBarrier2 depthToAttachment{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    depthToAttachment.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    depthToAttachment.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    depthToAttachment.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                                     VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    depthToAttachment.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                      VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthToAttachment.oldLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthToAttachment.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthToAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthToAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthToAttachment.image = fDepthImage;
    depthToAttachment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthToAttachment.subresourceRange.levelCount = 1;
    depthToAttachment.subresourceRange.layerCount = 1;

    const VkImageMemoryBarrier2 finishBarriers[] = {
        finalToFragment, depthToAttachment
    };
    VkDependencyInfo finishDependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    finishDependency.imageMemoryBarrierCount = uint32_t(std::size(finishBarriers));
    finishDependency.pImageMemoryBarriers = finishBarriers;
    vkCmdPipelineBarrier2(cmd, &finishDependency);

    // The explicit barrier above already prepared depth for the resumed UI pass.
    fMainDepthUsed = false;
    IBeginRendering(nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fGTAOCompositePipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, fGTAOPipelineLayout,
                            0, 1, &descriptorSet, 0, nullptr);
    SetViewport();
    vkCmdDraw(cmd, 3, 1, 0, 0);
}
