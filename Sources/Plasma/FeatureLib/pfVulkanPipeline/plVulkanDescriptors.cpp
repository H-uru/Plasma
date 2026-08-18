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

// Per-draw uniform space and descriptor sets.
//
// Uniforms come out of a per-frame bump allocator and are bound with dynamic
// offsets, so set 0 is allocated once and rebound rather than reallocated.
// Textures change per material, so set 1 is cached on a content hash of the
// layers' cookies. Old entries are reused after every frame that could still
// reference them has completed.

#include "plVulkanDevice.h"

#include "plPipeline/plCubicRenderTarget.h"

#include <string_theory/format>

#include <vk_mem_alloc.h>

#include <algorithm>
#include <vector>

namespace
{
    /** How much uniform space one frame gets before the ring grows. */
    constexpr VkDeviceSize kScratchBlockSize = 256 * 1024;

    constexpr VkDeviceSize kLightDescriptorRange =
        sizeof(plShaderLightSource) * kMaxSceneLights;
}

bool plVulkanDevice::ICreateDescriptorPool()
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(fPhysicalDevice, &props);
    // One allocator feeds both dynamic uniform and dynamic storage bindings,
    // so every returned offset must satisfy the stricter of the two limits.
    fUniformAlignment = std::max(props.limits.minUniformBufferOffsetAlignment,
                                 props.limits.minStorageBufferOffsetAlignment);
    if (fUniformAlignment == 0)
        fUniformAlignment = 1;

    VkDescriptorPoolSize sizes[4]{};
    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    sizes[0].descriptorCount = kDescriptorSetsPerPool * 5;
    sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    sizes[3].descriptorCount = kDescriptorSetsPerPool;
    sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    sizes[1].descriptorCount = kDescriptorSetsPerPool * (kMaxLayers * 2 + 1);
    sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    sizes[2].descriptorCount = kDescriptorSetsPerPool * kNumSamplerSlots;

    VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    info.maxSets = kDescriptorSetsPerPool;
    info.poolSizeCount = uint32_t(std::size(sizes));
    info.pPoolSizes = sizes;

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorPool(fDevice, &info, nullptr, &pool);
    if (result != VK_SUCCESS)
        return IFail("vkCreateDescriptorPool", result);

    SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, pool,
                  ST::format("descriptor pool {}", fDescriptorPools.size()));

    fDescriptorPools.push_back(pool);
    fDescriptorPool = pool;
    return true;
}

bool plVulkanDevice::IAllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet* out,
                                            const ST::string& name)
{
    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    info.descriptorPool = fDescriptorPool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    VkResult result = vkAllocateDescriptorSets(fDevice, &info, out);
    if (result == VK_SUCCESS) {
        SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, *out, name);
        return true;
    }

    // Out of room is expected, not an error; add another pool and retry once.
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        if (!ICreateDescriptorPool())
            return false;

        info.descriptorPool = fDescriptorPool;
        result = vkAllocateDescriptorSets(fDevice, &info, out);
        if (result == VK_SUCCESS) {
            SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, *out, name);
            return true;
        }
    }

    IFail("vkAllocateDescriptorSets", result);
    return false;
}

void plVulkanDevice::IResetScratch(uint32_t frameIndex)
{
    plScratchBlock& scratch = fScratch[frameIndex];

    // If the frame overran last time round, give it what it actually asked for
    // before it starts filling up again.
    // Binding 2 always advertises kLightDescriptorRange bytes. Preserve that
    // much tail room beyond the previous frame's high-water mark so a light
    // allocation at the same point is valid as a dynamic descriptor offset.
    const VkDeviceSize required = scratch.fWanted + kLightDescriptorRange;
    if (required > scratch.fBuffer.fSize)
        IGrowScratch(frameIndex, required);

    scratch.fUsed = 0;
    scratch.fWanted = 0;
}

bool plVulkanDevice::IGrowScratch(uint32_t frameIndex, VkDeviceSize size)
{
    plScratchBlock& scratch = fScratch[frameIndex];

    // Round up so a slowly growing frame does not reallocate every time.
    VkDeviceSize target = std::max(size, kScratchBlockSize);
    target = (target + kScratchBlockSize - 1) & ~(kScratchBlockSize - 1);

    plVulkanBuffer buffer = CreateBuffer(target,
                                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                         ST::format("scratch ring frame {}", frameIndex));
    if (!buffer.IsValid())
        return false;

    // The old one may still be in flight, so it goes through the retire queue.
    RetireBuffer(scratch.fBuffer);

    scratch.fBuffer = buffer;
    scratch.fUsed = 0;

    // The descriptor set names the buffer, so a new buffer means a rewrite.
    IWriteUniformDescriptorSet(frameIndex);
    return true;
}

plVulkanDevice::plScratchAlloc plVulkanDevice::AllocateScratch(size_t size,
                                                               size_t declaredRange)
{
    plScratchAlloc alloc;
    if (size == 0)
        return alloc;

    plScratchBlock& scratch = fScratch[fFrameIndex];

    if (!scratch.fBuffer.IsValid() &&
        !IGrowScratch(fFrameIndex, VkDeviceSize(size) + kLightDescriptorRange))
        return alloc;

    const VkDeviceSize aligned = (VkDeviceSize(size) + fUniformAlignment - 1) &
                                 ~(fUniformAlignment - 1);

    const VkDeviceSize requiredAtOffset = std::max<VkDeviceSize>(aligned, declaredRange);
    if (scratch.fUsed + requiredAtOffset > scratch.fBuffer.fSize) {
        // Cannot grow mid-frame without invalidating everything already handed
        // out. Record the actual high-water request; IResetScratch adds the
        // descriptor tail reservation before the frame slot is reused.
        scratch.fWanted = std::max(scratch.fWanted, scratch.fUsed + aligned);
        return alloc;
    }

    alloc.fBuffer = scratch.fBuffer.fBuffer;
    alloc.fOffset = uint32_t(scratch.fUsed);
    alloc.fMapped = static_cast<uint8_t*>(scratch.fBuffer.fMapped) + scratch.fUsed;

    scratch.fUsed += aligned;
    scratch.fWanted = std::max(scratch.fWanted, scratch.fUsed);

    return alloc;
}

void plVulkanDevice::IWriteUniformDescriptorSet(uint32_t frameIndex)
{
    if (fUniformSet[frameIndex] == VK_NULL_HANDLE) {
        if (!IAllocateDescriptorSet(fUniformSetLayout, &fUniformSet[frameIndex],
                                    ST::format("uniform set frame {}", frameIndex)))
            return;
    }

    const VkBuffer buffer = fScratch[frameIndex].fBuffer.fBuffer;
    if (buffer == VK_NULL_HANDLE)
        return;

    // Every binding is dynamic, so offset stays zero here and the range is just
    // how much the shader may read from wherever the offset lands.
    VkDescriptorBufferInfo vertexInfo{ buffer, 0, sizeof(VertexUniforms) };
    VkDescriptorBufferInfo materialInfo{ buffer, 0, sizeof(plMaterialLightingDescriptor) };
    VkDescriptorBufferInfo lightInfo{ buffer, 0, kLightDescriptorRange };
    VkDescriptorBufferInfo shadowInfo{ buffer, 0, sizeof(plShadowState) };
    VkDescriptorBufferInfo constsInfo{ buffer, 0, sizeof(plVkVec4) * kMaxShaderConsts };

    VkWriteDescriptorSet writes[6]{};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = fUniformSet[frameIndex];
    writes[0].dstBinding = kBindingVertexUniforms;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[0].pBufferInfo = &vertexInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = fUniformSet[frameIndex];
    writes[1].dstBinding = kBindingMaterial;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[1].pBufferInfo = &materialInfo;

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = fUniformSet[frameIndex];
    writes[2].dstBinding = kBindingLights;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writes[2].pBufferInfo = &lightInfo;

    writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[3].dstSet = fUniformSet[frameIndex];
    writes[3].dstBinding = kBindingShadowState;
    writes[3].descriptorCount = 1;
    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[3].pBufferInfo = &shadowInfo;

    for (uint32_t i = 4; i < 6; i++) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = fUniformSet[frameIndex];
        writes[i].dstBinding = (i == 4) ? kBindingVertexShaderConsts
                                        : kBindingFragmentShaderConsts;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writes[i].pBufferInfo = &constsInfo;
    }

    vkUpdateDescriptorSets(fDevice, 6, writes, 0, nullptr);
}

VkDescriptorSet plVulkanDevice::GetUniformDescriptorSet()
{
    if (fUniformSet[fFrameIndex] == VK_NULL_HANDLE)
        IWriteUniformDescriptorSet(fFrameIndex);

    return fUniformSet[fFrameIndex];
}

void plVulkanDevice::IWarnUnboundSlot(uint64_t hash, uint32_t slot, const ST::string& debugName)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (!fWarnedTextureSets.insert(hash).second)
        return;

    hsStatusMessageF("Vulkan: {} has nothing to bind for texture slot {}; whatever samples it "
                     "reads undefined contents",
                     debugName.empty() ? ST_LITERAL("a texture set") : debugName, slot);
#endif
}

plVulkanDevice::plTextureSetSlot* plVulkanDevice::IAllocateTextureSetSlot()
{
    if (fTextureSetSlotBlocks.empty() ||
        fTextureSetSlotBlockIndex == kTextureSetSlotBlockSize) {
        fTextureSetSlotBlocks.emplace_back(
            std::make_unique<plTextureSetSlot[]>(kTextureSetSlotBlockSize));
        fTextureSetSlotBlockIndex = 0;
    }

    return &fTextureSetSlotBlocks.back()[fTextureSetSlotBlockIndex++];
}

void plVulkanDevice::IAttachTextureSetSlot(plTextureSetSlot* slot)
{
    hsAssert(slot, "Cannot attach a null Vulkan descriptor slot");

    // Append to the MRU end of the queue.
    slot->fQueueNext = nullptr;
    slot->fQueuePrev = fTextureSetQueueEnd;
    if (fTextureSetQueueEnd)
        fTextureSetQueueEnd->fQueueNext = slot;
    else
        fTextureSetQueueBegin = slot;
    fTextureSetQueueEnd = slot;

    // Insert at the head of its hash bucket.
    const size_t bucket = slot->fHash % kTextureSetHashBuckets;
    slot->fHashPrev = nullptr;
    slot->fHashNext = fTextureSetHash[bucket];
    if (slot->fHashNext)
        slot->fHashNext->fHashPrev = slot;
    fTextureSetHash[bucket] = slot;
}

void plVulkanDevice::IDetachTextureSetSlot(plTextureSetSlot* slot)
{
    hsAssert(slot, "Cannot detach a null Vulkan descriptor slot");

    if (slot->fQueuePrev)
        slot->fQueuePrev->fQueueNext = slot->fQueueNext;
    else
        fTextureSetQueueBegin = slot->fQueueNext;
    if (slot->fQueueNext)
        slot->fQueueNext->fQueuePrev = slot->fQueuePrev;
    else
        fTextureSetQueueEnd = slot->fQueuePrev;

    const size_t bucket = slot->fHash % kTextureSetHashBuckets;
    if (slot->fHashPrev)
        slot->fHashPrev->fHashNext = slot->fHashNext;
    else
        fTextureSetHash[bucket] = slot->fHashNext;
    if (slot->fHashNext)
        slot->fHashNext->fHashPrev = slot->fHashPrev;

    slot->fQueueNext = nullptr;
    slot->fQueuePrev = nullptr;
    slot->fHashNext = nullptr;
    slot->fHashPrev = nullptr;
}

bool plVulkanDevice::IReserveTextureDescriptorSets()
{
    for (uint32_t i = 0; i < kTextureSetReserveBatch; ++i) {
        plTextureSetSlot* slot = IAllocateTextureSetSlot();
        if (!IAllocateDescriptorSet(fTextureSetLayout, &slot->fSet,
                                    ST_LITERAL("reserved texture set"))) {
            return false;
        }
        fReservedTextureSets.push_back(slot);
    }
    return true;
}

plVulkanDevice::plTextureSetResult
plVulkanDevice::IResolveTextureDescriptorSet(uint64_t hash)
{
    const size_t bucket = hash % kTextureSetHashBuckets;
    for (plTextureSetSlot* slot = fTextureSetHash[bucket]; slot; slot = slot->fHashNext) {
        if (slot->fHash != hash)
            continue;

        if (fTextureSetQueueEnd != slot) {
            IDetachTextureSetSlot(slot);
            IAttachTextureSetSlot(slot);
        }
        slot->fLastUsedFrame = fFrameSerial;
        return { slot, true };
    }

    plTextureSetSlot* slot = fTextureSetQueueBegin;
    if (slot && fFrameSerial >= slot->fLastUsedFrame &&
        fFrameSerial - slot->fLastUsedFrame > kMaxFramesInFlight) {
        IDetachTextureSetSlot(slot);
        fWarnedTextureSets.erase(slot->fHash);
    } else {
        if (fReservedTextureSets.empty() && !IReserveTextureDescriptorSets())
            return {};

        slot = fReservedTextureSets.back();
        fReservedTextureSets.pop_back();
    }

    slot->fHash = hash;
    slot->fLastUsedFrame = fFrameSerial;
    IAttachTextureSetSlot(slot);
    return { slot, false };
}

void plVulkanDevice::IClearTextureDescriptorSets()
{
    std::fill(std::begin(fTextureSetHash), std::end(fTextureSetHash), nullptr);
    fTextureSetQueueBegin = nullptr;
    fTextureSetQueueEnd = nullptr;
    fReservedTextureSets.clear();
    fTextureSetSlotBlocks.clear();
    fTextureSetSlotBlockIndex = kTextureSetSlotBlockSize;
}

VkDescriptorSet plVulkanDevice::GetTextureDescriptorSet(const plVulkanTextureRef* const* layers,
                                                        const uint8_t* clampFlags, uint32_t count,
                                                        const plVulkanRenderTargetRef* shadowMap,
                                                        const plVulkanRenderTargetRef* const* renderTargets,
                                                        const ST::string& debugName)
{
    // Hash the layers' stable cookies rather than their view handles, so a
    // texture that was destroyed and recreated cannot alias a cached set.
    constexpr uint64_t kOffsetBasis = 0xcbf29ce484222325ull;
    constexpr uint64_t kPrime = 0x100000001b3ull;

    uint64_t hash = kOffsetBasis;
    for (uint32_t i = 0; i < count; i++) {
        const plVulkanTextureRef* texture = layers ? layers[i] : nullptr;
        const plVulkanRenderTargetRef* renderTarget = renderTargets ? renderTargets[i] : nullptr;
        const uint64_t cookie = texture ? texture->fCookie
                                       : renderTarget ? renderTarget->fCookie : 0;
        hash ^= cookie;
        hash *= kPrime;
        // Position matters: the same textures in a different order is a
        // different material.
        hash ^= i + 1;
        hash *= kPrime;
        // The sampler slot is part of the set's contents, so two materials that
        // share textures but clamp them differently must not share a set.
        hash ^= clampFlags ? (clampFlags[i] & 0x3u) : 0u;
        hash *= kPrime;
    }

    hash ^= shadowMap ? shadowMap->fCookie : 0;
    hash *= kPrime;

    const plTextureSetResult result = IResolveTextureDescriptorSet(hash);
    if (!result.fSlot)
        return VK_NULL_HANDLE;
    if (result.fFound)
        return result.fSlot->fSet;

    const VkDescriptorSet set = result.fSlot->fSet;
    const ST::string setName = debugName.empty() ? ST_LITERAL("texture set") : debugName;
    SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, set, setName);

    // Fixed-size, so the pImageInfo pointers below cannot be invalidated by
    // growth the way a vector's would be.
    VkDescriptorImageInfo images[kMaxLayers]{};
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(count + 2);

    for (uint32_t i = 0; i < count && i < kMaxLayers; i++) {
        const plVulkanTextureRef* texture = layers ? layers[i] : nullptr;
        const plVulkanRenderTargetRef* renderTarget = renderTargets ? renderTargets[i] : nullptr;
        const VkImageView imageView = texture ? texture->fImageView
                                             : renderTarget ? renderTarget->fImageView
                                                            : VK_NULL_HANDLE;
        if (imageView == VK_NULL_HANDLE) {
            // PARTIALLY_BOUND makes leaving the slot empty legal, so a shader
            // that samples it anyway reads undefined contents and neither the
            // driver nor the validation layer has anything to say about it.
            IWarnUnboundSlot(hash, i, debugName);
            continue;
        }

        images[i].imageView = imageView;
        images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        const bool isCubic =
            texture ? texture->fIsCubic
                    : plCubicRenderTarget::ConvertNoRef(renderTarget->fOwner) != nullptr;

        // The binding is PARTIALLY_BOUND, so writing only the slots a material
        // actually uses is legal; the rest stay empty and are never sampled.
        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = set;
        write.dstBinding = isCubic ? kBindingTexturesCube : kBindingTextures2D;
        write.dstArrayElement = i;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = &images[i];
        writes.push_back(write);
    }

    // Slot N is the sampler layer N wants; every unused slot still has to be
    // written, because this binding is not partially bound.
    VkDescriptorImageInfo shadowInfo{};
    if (shadowMap && shadowMap->fImageView != VK_NULL_HANDLE) {
        shadowInfo.imageView = shadowMap->fImageView;
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = set;
        write.dstBinding = kBindingShadowMap;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = &shadowInfo;
        writes.push_back(write);
    }

    VkDescriptorImageInfo samplers[kNumSamplerSlots]{};
    for (uint32_t i = 0; i < kMaxLayers; i++) {
        const uint32_t clamp = (clampFlags && i < count) ? (clampFlags[i] & 0x3u) : 0u;
        samplers[i].sampler = fSamplers[clamp];
    }
    samplers[kSamplerNearest].sampler = fSamplers[kSamplerNearestObject];
    samplers[kSamplerClampBoth].sampler = fSamplers[kSamplerClampBothObject];
    samplers[kSamplerClampToZero].sampler = fSamplers[kSamplerClampToZeroObject];

    VkWriteDescriptorSet samplerWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    samplerWrite.dstSet = set;
    samplerWrite.dstBinding = kBindingSamplers;
    samplerWrite.descriptorCount = kNumSamplerSlots;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.pImageInfo = samplers;
    writes.push_back(samplerWrite);

    vkUpdateDescriptorSets(fDevice, uint32_t(writes.size()), writes.data(), 0, nullptr);

    return set;
}
