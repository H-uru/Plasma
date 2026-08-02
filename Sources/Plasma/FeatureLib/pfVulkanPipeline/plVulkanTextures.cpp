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

// Texture upload. Ported from plMetalDevice.cpp:732-980.
//
// Unlike buffers these are staged: an optimally-tiled image, block-compressed
// or not, cannot be host-mapped, so the pixels go through a temporary
// host-visible buffer and vkCmdCopyBufferToImage.

#include "plVulkanDevice.h"

#include "plGImage/hsCodecManager.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"

#include <string_theory/format>

#include <vk_mem_alloc.h>

#include <algorithm>
#include <cstring>
#include <vector>

namespace
{
    /** Bytes one mip level occupies, tightly packed. */
    size_t ILevelSize(VkFormat format, uint32_t width, uint32_t height)
    {
        switch (format) {
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return size_t(std::max(1u, (width + 3) / 4)) * std::max(1u, (height + 3) / 4) * 8;
        case VK_FORMAT_BC3_UNORM_BLOCK:
            return size_t(std::max(1u, (width + 3) / 4)) * std::max(1u, (height + 3) / 4) * 16;
        case VK_FORMAT_R8_UNORM:
            return size_t(width) * height;
        case VK_FORMAT_R8G8_UNORM:
            return size_t(width) * height * 2;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return size_t(width) * height * 2;
        default:
            return size_t(width) * height * 4;
        }
    }

    void ITransition(VkCommandBuffer cmd, VkImage image, uint32_t levels, uint32_t layers,
                     VkImageLayout from, VkImageLayout to,
                     VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                     VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess)
    {
        VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        barrier.srcStageMask = srcStage;
        barrier.srcAccessMask = srcAccess;
        barrier.dstStageMask = dstStage;
        barrier.dstAccessMask = dstAccess;
        barrier.oldLayout = from;
        barrier.newLayout = to;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = levels;
        barrier.subresourceRange.layerCount = layers;

        VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependency.imageMemoryBarrierCount = 1;
        dependency.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(cmd, &dependency);
    }
}

/**
 * Identity for the descriptor cache.
 *
 * Monotonic rather than derived from any Vulkan handle, which the driver is
 * free to reuse the moment a texture is destroyed.
 */
static uint64_t INextTextureCookie()
{
    static uint64_t s_nextCookie = 1;
    return s_nextCookie++;
}

/** Allocates the image and its view. Returns false and leaves the ref empty on failure. */
static bool ICreateImage(plVulkanDevice* device, VmaAllocator allocator, VkDevice vkDevice,
                         plVulkanTextureRef* tRef, uint32_t levelCount, bool cubic,
                         const ST::string& name);

void plVulkanDevice::SetupTextureRef(plBitmap* img, TextureRef* tRef)
{
    tRef->fOwner = img;
    tRef->fDevice = this;
    tRef->fCookie = INextTextureCookie();

    // A cubic's root reports a format that would decode wrong; ask a face.
    plBitmap* imageToCheck = img;
    plCubicEnvironmap* cubicImg = dynamic_cast<plCubicEnvironmap*>(img);
    if (cubicImg)
        imageToCheck = cubicImg->GetFace(0);

    if (imageToCheck->IsCompressed() && fSupportsBC) {
        switch (imageToCheck->fDirectXInfo.fCompressionType) {
        case plBitmap::DirectXInfo::kDXT1:
            tRef->fFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;
        case plBitmap::DirectXInfo::kDXT5:
            tRef->fFormat = VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        default:
            tRef->fFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        }
    } else {
        switch (imageToCheck->fUncompressedInfo.fType) {
        case plBitmap::UncompressedInfo::kRGB8888:
            tRef->fFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case plBitmap::UncompressedInfo::kRGB4444:
            // Expanded to eight bits per channel on upload, as Metal does.
            tRef->fFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case plBitmap::UncompressedInfo::kRGB1555:
            tRef->fFormat = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
            break;
        case plBitmap::UncompressedInfo::kInten8:
            // Sampled as a normalized float, matching the fixed-function path.
            tRef->fFormat = VK_FORMAT_R8_UNORM;
            break;
        case plBitmap::UncompressedInfo::kAInten88:
            tRef->fFormat = VK_FORMAT_R8G8_UNORM;
            break;
        default:
            tRef->fFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        }
    }

    tRef->SetDirty(true);

    img->SetDeviceRef(tRef);
    tRef->UnRef();
}

void plVulkanDevice::CheckTexture(TextureRef* tRef)
{
    if (tRef->fImage == VK_NULL_HANDLE)
        tRef->SetDirty(true);
}

void plVulkanDevice::IConfigureAllowedLevels(TextureRef* tRef, plMipmap* mipmap)
{
    if (!mipmap->IsCompressed() || !fSupportsBC)
        return;

    mipmap->SetCurrLevel(tRef->fLevels);
    while ((mipmap->GetCurrWidth() | mipmap->GetCurrHeight()) & 0x03) {
        tRef->fLevels--;
        if (tRef->fLevels < 0) {
            tRef->fLevels = -1;
            break;
        }
        mipmap->SetCurrLevel(tRef->fLevels);
    }
}

void plVulkanDevice::IPopulateTexture(TextureRef* tRef, plMipmap* img, uint32_t slice)
{
    if (tRef->fLevels < 0 || tRef->fImage == VK_NULL_HANDLE)
        return;

    // Match Metal's fallback on devices without native BC sampling. The codec
    // returns a new strong reference, held here until the synchronous upload is
    // complete.
    hsRef<plMipmap> decoded;
    if (img->IsCompressed() && !fSupportsBC) {
        plMipmap* uncompressed = hsCodecManager::Instance().CreateUncompressedMipmap(
            img, hsCodecManager::k32BitDepth);
        if (!uncompressed)
            return;
        decoded = hsRef<plMipmap>(uncompressed, hsStealRef);
        img = uncompressed;
    }

    const bool compressed = img->IsCompressed();

    const bool expand4444 =
        !compressed && img->fUncompressedInfo.fType == plBitmap::UncompressedInfo::kRGB4444;

    // Gather every level into one staging buffer, then issue one copy per level.
    struct plLevelCopy
    {
        size_t   fOffset;
        uint32_t fWidth;
        uint32_t fHeight;
        uint32_t fLevel;
    };
    std::vector<plLevelCopy> copies;
    std::vector<uint8_t>     staging;

    for (int32_t lvl = 0; lvl <= tRef->fLevels; lvl++) {
        img->SetCurrLevel(lvl);

        const void* src = img->GetCurrLevelPtr();
        if (!src)
            continue;

        // Some cubic assets disagree about mip sizes between faces. DX keeps its
        // own table and ignores what the face claims; derive the sizes from the
        // top level so every face lines up with the image we allocated.
        const uint32_t levelWidth = std::max(1u, tRef->fWidth >> lvl);
        const uint32_t levelHeight = std::max(1u, tRef->fHeight >> lvl);

        const size_t levelBytes = ILevelSize(tRef->fFormat, levelWidth, levelHeight);
        const size_t offset = staging.size();
        staging.resize(offset + levelBytes);

        if (expand4444) {
            struct plRGBA4444 { unsigned r : 4, g : 4, b : 4, a : 4; };
            const plRGBA4444* in = static_cast<const plRGBA4444*>(src);
            uint8_t* out = staging.data() + offset;

            const size_t texels = size_t(levelWidth) * levelHeight;
            for (size_t i = 0; i < texels; i++) {
                // Widen each nibble to a full byte by replicating it.
                out[i * 4 + 0] = uint8_t(in[i].b | (in[i].b << 4));
                out[i * 4 + 1] = uint8_t(in[i].g | (in[i].g << 4));
                out[i * 4 + 2] = uint8_t(in[i].r | (in[i].r << 4));
                out[i * 4 + 3] = uint8_t(in[i].a | (in[i].a << 4));
            }
        } else {
            // Never read past what the level actually holds; the derived size
            // above can exceed it on the inconsistent cubic faces.
            const size_t available = img->GetCurrLevelSize();
            memcpy(staging.data() + offset, src, std::min(levelBytes, available));
        }

        copies.push_back(plLevelCopy{ offset, levelWidth, levelHeight, uint32_t(lvl) });
    }

    if (copies.empty())
        return;

    plVulkanBuffer stage = CreateBuffer(staging.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true,
                                        ST::format("{} staging", img->GetKeyName()));
    if (!stage.IsValid())
        return;
    memcpy(stage.fMapped, staging.data(), staging.size());

    VkCommandBuffer cmd = IBeginOneShot();
    if (cmd == VK_NULL_HANDLE) {
        RetireBuffer(stage);
        return;
    }

    const uint32_t levelCount = uint32_t(tRef->fLevels) + 1;
    const uint32_t layerCount = tRef->fIsCubic ? 6 : 1;

    ITransition(cmd, tRef->fImage, levelCount, layerCount,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

    std::vector<VkBufferImageCopy> regions;
    regions.reserve(copies.size());
    for (const plLevelCopy& copy : copies) {
        VkBufferImageCopy region{};
        region.bufferOffset = copy.fOffset;
        // Zero means tightly packed, which is how the staging data was built.
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = copy.fLevel;
        region.imageSubresource.baseArrayLayer = slice;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = { copy.fWidth, copy.fHeight, 1 };
        regions.push_back(region);
    }

    vkCmdCopyBufferToImage(cmd, stage.fBuffer, tRef->fImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           uint32_t(regions.size()), regions.data());

    ITransition(cmd, tRef->fImage, levelCount, layerCount,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

    IEndOneShot(cmd);

    // The submit above was waited on, so this is already safe to destroy; the
    // deferred queue is just the one path that owns freeing.
    RetireBuffer(stage);

    tRef->SetDirty(false);
}

static bool ICreateImage(plVulkanDevice* device, VmaAllocator allocator, VkDevice vkDevice,
                         plVulkanTextureRef* tRef, uint32_t levelCount, bool cubic,
                         const ST::string& name)
{
    const uint32_t layerCount = cubic ? 6 : 1;

    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.flags = cubic ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = tRef->fFormat;
    imageInfo.extent = { tRef->fWidth, tRef->fHeight, 1 };
    imageInfo.mipLevels = levelCount;
    imageInfo.arrayLayers = layerCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &tRef->fImage,
                       &tRef->fAllocation, nullptr) != VK_SUCCESS) {
        tRef->fImage = VK_NULL_HANDLE;
        hsStatusMessageF("Vulkan: could not allocate the {}x{} image for texture '{}'",
                         tRef->fWidth, tRef->fHeight, name);
        return false;
    }

    device->SetObjectName(VK_OBJECT_TYPE_IMAGE, tRef->fImage, name);

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = tRef->fImage;
    viewInfo.viewType = cubic ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = tRef->fFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.layerCount = layerCount;

    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &tRef->fImageView) != VK_SUCCESS) {
        vmaDestroyImage(allocator, tRef->fImage, tRef->fAllocation);
        tRef->fImage = VK_NULL_HANDLE;
        tRef->fAllocation = nullptr;
        hsStatusMessageF("Vulkan: could not create the image view for texture '{}'", name);
        return false;
    }

    device->SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, tRef->fImageView, name);

    return true;
}

void plVulkanDevice::MakeTextureRef(TextureRef* tRef, plMipmap* img)
{
    if (!img->GetImage()) {
        // Nothing downstream reports this: the layer keeps its texture pass type
        // and the descriptor slot it wanted is simply never written.
        hsStatusMessageF("Vulkan: texture '{}' has no pixel data, so nothing will be bound for it",
                         img->GetKeyName());
        return;
    }

    tRef->Release();

    // A recreated image is a different resource as far as any cached descriptor
    // set is concerned, so it needs a new identity.
    tRef->fCookie = INextTextureCookie();

    tRef->fLevels = img->GetNumLevels() - 1;
    tRef->fWidth = img->GetWidth();
    tRef->fHeight = img->GetHeight();
    tRef->fIsCubic = false;
    tRef->fDevice = this;

    IConfigureAllowedLevels(tRef, img);
    if (tRef->fLevels < 0) {
        hsStatusMessageF("Vulkan: texture '{}' has no usable mip levels", img->GetKeyName());
        return;
    }

    if (!ICreateImage(this, fAllocator, fDevice, tRef, uint32_t(tRef->fLevels) + 1, false,
                      img->GetKeyName()))
        return;

    IPopulateTexture(tRef, img, 0);
    tRef->SetDirty(false);
}

void plVulkanDevice::CreateTextureFromMemory(TextureRef* tRef, const void* data, size_t size,
                                             const ST::string& name)
{
    tRef->Release();
    tRef->fCookie = INextTextureCookie();
    tRef->fDevice = this;
    tRef->fLevels = 0;
    tRef->fIsCubic = false;

    if (!ICreateImage(this, fAllocator, fDevice, tRef, 1, false, name))
        return;

    plVulkanBuffer stage = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true,
                                        ST::format("{} staging", name));
    if (!stage.IsValid())
        return;
    memcpy(stage.fMapped, data, size);

    VkCommandBuffer cmd = IBeginOneShot();
    if (cmd == VK_NULL_HANDLE) {
        RetireBuffer(stage);
        return;
    }

    ITransition(cmd, tRef->fImage, 1, 1,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { tRef->fWidth, tRef->fHeight, 1 };

    vkCmdCopyBufferToImage(cmd, stage.fBuffer, tRef->fImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    ITransition(cmd, tRef->fImage, 1, 1,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

    IEndOneShot(cmd);
    RetireBuffer(stage);

    tRef->SetDirty(false);
}

bool plVulkanDevice::CreateRenderTargetImage(plVulkanRenderTargetRef* ref, uint32_t width,
                                            uint32_t height, bool wantsDepth, bool cubic,
                                            const ST::string& name)
{
    const uint32_t layerCount = cubic ? 6 : 1;

    ref->Release();
    ref->fDevice = this;
    ref->fCookie = INextTextureCookie();
    ref->fWidth = width;
    ref->fHeight = height;
    // Same format as the swapchain, so a target can be presented or sampled
    // without a conversion.
    ref->fFormat = fSwapFormat;
    ref->fDepthFormat = wantsDepth ? fDepthFormat : VK_FORMAT_UNDEFINED;

    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.flags = cubic ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = ref->fFormat;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = layerCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateImage(fAllocator, &imageInfo, &allocInfo, &ref->fImage,
                       &ref->fAllocation, nullptr) != VK_SUCCESS) {
        ref->fImage = VK_NULL_HANDLE;
        hsStatusMessageF("Vulkan: could not allocate the {}x{} image for render target '{}'",
                         width, height, name);
        return false;
    }

    SetObjectName(VK_OBJECT_TYPE_IMAGE, ref->fImage, name);

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = ref->fImage;
    viewInfo.viewType = cubic ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = ref->fFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = layerCount;

    if (vkCreateImageView(fDevice, &viewInfo, nullptr, &ref->fImageView) != VK_SUCCESS) {
        hsStatusMessageF("Vulkan: could not create the image view for render target '{}'", name);
        return false;
    }

    SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, ref->fImageView, name);

    if (wantsDepth) {
        // One depth buffer serves every face; they are rendered one at a time.
        imageInfo.flags = 0;
        imageInfo.arrayLayers = 1;
        imageInfo.format = ref->fDepthFormat;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (vmaCreateImage(fAllocator, &imageInfo, &allocInfo, &ref->fDepthImage,
                           &ref->fDepthAllocation, nullptr) != VK_SUCCESS) {
            ref->fDepthImage = VK_NULL_HANDLE;
            ref->fDepthFormat = VK_FORMAT_UNDEFINED;
            hsStatusMessageF("Vulkan: could not allocate the depth image for render target '{}'",
                             name);
            return false;
        }

        SetObjectName(VK_OBJECT_TYPE_IMAGE, ref->fDepthImage, ST::format("{} depth", name));

        viewInfo.image = ref->fDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = ref->fDepthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(fDevice, &viewInfo, nullptr, &ref->fDepthView) != VK_SUCCESS) {
            hsStatusMessageF("Vulkan: could not create the depth view for render target '{}'",
                             name);
            return false;
        }

        SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, ref->fDepthView, ST::format("{} depth", name));
    }

    ref->SetDirty(false);
    return true;
}

bool plVulkanDevice::CreateRenderTarget(plVulkanRenderTargetRef* ref, uint32_t width,
                                        uint32_t height, bool wantsDepth, const ST::string& name)
{
    return CreateRenderTargetImage(ref, width, height, wantsDepth, /*cubic=*/false, name);
}

bool plVulkanDevice::CreateCubicRenderTarget(plVulkanRenderTargetRef* parent,
                                             plVulkanRenderTargetRef* const* faces,
                                             uint32_t size, bool wantsDepth,
                                             const ST::string& name)
{
    if (!CreateRenderTargetImage(parent, size, size, wantsDepth, /*cubic=*/true, name))
        return false;

    // Plasma's face order is not Vulkan's. Same remap the cubic texture path
    // uses (plMetalPipeline::MakeRenderTargetRef, kFaceMapping).
    static constexpr uint32_t kFaceMapping[] = {
        1, // kLeftFace
        0, // kRightFace
        4, // kFrontFace
        5, // kBackFace
        2, // kTopFace
        3  // kBottomFace
    };

    for (uint32_t i = 0; i < 6; i++) {
        plVulkanRenderTargetRef* face = faces[i];
        if (!face)
            continue;

        face->Release();
        face->fDevice = this;
        face->fCookie = INextTextureCookie();
        face->fOwnsImage = false;
        face->fWidth = size;
        face->fHeight = size;
        face->fFormat = parent->fFormat;

        // The faces share the parent's image and depth buffer; only the view is
        // theirs, so fAllocation stays null and Release() frees just the view.
        face->fImage = parent->fImage;
        face->fDepthImage = parent->fDepthImage;
        face->fDepthView = parent->fDepthView;
        face->fDepthFormat = parent->fDepthFormat;

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = parent->fImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = parent->fFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = kFaceMapping[i];
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(fDevice, &viewInfo, nullptr, &face->fImageView) != VK_SUCCESS) {
            hsStatusMessageF("Vulkan: could not create face {} of cubic render target '{}'",
                             i, name);
            return false;
        }

        SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, face->fImageView,
                      ST::format("{} face {}", name, i));

        face->SetDirty(false);
    }

    return true;
}

void plVulkanDevice::MakeCubicTextureRef(TextureRef* tRef, plCubicEnvironmap* img)
{
    plMipmap* face0 = img->GetFace(0);
    if (!face0) {
        hsStatusMessageF("Vulkan: cubic texture '{}' has no faces", img->GetKeyName());
        return;
    }

    tRef->Release();

    tRef->fCookie = INextTextureCookie();

    tRef->fLevels = face0->GetNumLevels() - 1;
    tRef->fWidth = face0->GetWidth();
    tRef->fHeight = face0->GetHeight();
    tRef->fIsCubic = true;
    tRef->fDevice = this;

    IConfigureAllowedLevels(tRef, face0);
    if (tRef->fLevels < 0) {
        hsStatusMessageF("Vulkan: cubic texture '{}' has no usable mip levels", img->GetKeyName());
        return;
    }

    if (!ICreateImage(this, fAllocator, fDevice, tRef, uint32_t(tRef->fLevels) + 1, true,
                      img->GetKeyName()))
        return;

    // Plasma's face order is not Vulkan's; this is the same remap Metal uses.
    static constexpr uint32_t kFaceMapping[] = {
        1, // kLeftFace
        0, // kRightFace
        4, // kFrontFace
        5, // kBackFace
        2, // kTopFace
        3  // kBottomFace
    };

    for (uint32_t i = 0; i < 6; i++)
        IPopulateTexture(tRef, img->GetFace(i), kFaceMapping[i]);

    tRef->SetDirty(false);
}

/*** Readback ****************************************************************/

/**
 * Copies an image to a host-visible buffer and waits for it.
 *
 * The layout dance is the cost of doing this outside the frame: the image is
 * wherever the last render left it, so it is transitioned to a transfer source
 * and then back.
 */
static bool IReadImage(plVulkanDevice* device, VkCommandBuffer cmd, VkImage image,
                       VkImageLayout oldLayout, VkImageLayout newLayout,
                       uint32_t width, uint32_t height, VkBuffer staging)
{
    auto barrier = [&](VkImageLayout from, VkImageLayout to, VkPipelineStageFlags2 srcStage,
                       VkAccessFlags2 srcAccess, VkPipelineStageFlags2 dstStage,
                       VkAccessFlags2 dstAccess) {
        VkImageMemoryBarrier2 imageBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        imageBarrier.srcStageMask = srcStage;
        imageBarrier.srcAccessMask = srcAccess;
        imageBarrier.dstStageMask = dstStage;
        imageBarrier.dstAccessMask = dstAccess;
        imageBarrier.oldLayout = from;
        imageBarrier.newLayout = to;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = image;
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.levelCount = 1;
        imageBarrier.subresourceRange.layerCount = 1;

        VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependency.imageMemoryBarrierCount = 1;
        dependency.pImageMemoryBarriers = &imageBarrier;
        vkCmdPipelineBarrier2(cmd, &dependency);
    };

    barrier(oldLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { width, height, 1 };

    vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging, 1, &region);

    barrier(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, newLayout,
            VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT);

    return true;
}

bool plVulkanDevice::ReadRenderTarget(const plVulkanRenderTargetRef* ref, void* dest,
                                      size_t destSize)
{
    if (!ref || ref->fImage == VK_NULL_HANDLE || !dest)
        return false;

    const size_t needed = size_t(ref->fWidth) * ref->fHeight * 4;
    if (destSize < needed)
        return false;

    plVulkanBuffer staging = CreateBuffer(needed, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
    if (!staging.IsValid())
        return false;

    VkCommandBuffer cmd = IBeginOneShot();
    if (cmd == VK_NULL_HANDLE) {
        RetireBuffer(staging);
        return false;
    }

    // A target that has been rendered to is shader-readable; one that has not is
    // still undefined, and reading it back would give nothing useful anyway.
    const VkImageLayout layout = ref->fShaderReadable
                               ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                               : VK_IMAGE_LAYOUT_UNDEFINED;

    IReadImage(this, cmd, ref->fImage, layout, layout, ref->fWidth, ref->fHeight,
               staging.fBuffer);

    IEndOneShot(cmd);

    memcpy(dest, staging.fMapped, needed);
    RetireBuffer(staging);

    return true;
}

bool plVulkanDevice::ReadSwapchainImage(void* dest, size_t destSize, uint32_t* widthOut,
                                        uint32_t* heightOut)
{
    if (!dest || fDevice == VK_NULL_HANDLE || fSwapchain == VK_NULL_HANDLE ||
        !fSwapchainReadable || fSwapchainDirty || !fHasPresentedFrame || fFrameOpen)
        return false;

    const bool bgra8 = fSwapFormat == VK_FORMAT_B8G8R8A8_UNORM ||
                       fSwapFormat == VK_FORMAT_B8G8R8A8_SRGB;
    const bool tenBit = fSwapFormat == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
                        fSwapFormat == VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    if (!bgra8 && !tenBit)
        return false;

    const uint32_t width = fExtent.width;
    const uint32_t height = fExtent.height;
    const size_t needed = size_t(width) * height * 4;
    if (destSize < needed)
        return false;

    if (!fImagePreacquired) {
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence = VK_NULL_HANDLE;
        VkResult result = vkCreateFence(fDevice, &fenceInfo, nullptr, &fence);
        if (result != VK_SUCCESS)
            return IFail("vkCreateFence (capture)", result);

        constexpr uint64_t kCaptureAcquireTimeoutNs = 1'000'000'000ull;
        result = vkAcquireNextImageKHR(fDevice, fSwapchain, kCaptureAcquireTimeoutNs,
                                       VK_NULL_HANDLE, fence, &fImageIndex);
        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
            const VkResult waitResult = vkWaitForFences(fDevice, 1, &fence, VK_TRUE, UINT64_MAX);
            vkDestroyFence(fDevice, fence, nullptr);
            if (waitResult != VK_SUCCESS)
                return IFail("vkWaitForFences (capture)", waitResult);
            fImagePreacquired = true;
        } else {
            vkDestroyFence(fDevice, fence, nullptr);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
                fSwapchainDirty = true;
            if (result != VK_TIMEOUT && result != VK_NOT_READY &&
                result != VK_ERROR_OUT_OF_DATE_KHR) {
                IFail("vkAcquireNextImageKHR (capture)", result);
            }
            return false;
        }
    }

    if (fImageIndex >= fSwapImages.size()) {
        fSwapchainDirty = true;
        return false;
    }

    plVulkanBuffer staging = CreateBuffer(needed, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
    if (!staging.IsValid())
        return false;

    VkCommandBuffer cmd = IBeginOneShot();
    if (cmd == VK_NULL_HANDLE) {
        RetireBuffer(staging);
        return false;
    }

    // The acquire fence transferred this image back from WSI. Restore PRESENT
    // afterward so BeginFrame can either discard it for rendering or a second
    // capture can read it again before the next frame begins.
    IReadImage(this, cmd, fSwapImages[fImageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, width, height,
               staging.fBuffer);

    IEndOneShot(cmd);

    if (bgra8) {
        memcpy(dest, staging.fMapped, needed);
    } else {
        const uint32_t* input = static_cast<const uint32_t*>(staging.fMapped);
        uint32_t* output = static_cast<uint32_t*>(dest);
        const bool blueLow = fSwapFormat == VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        for (size_t i = 0; i < size_t(width) * height; ++i) {
            const uint32_t packed = input[i];
            const uint32_t low = packed & 0x3ffu;
            const uint32_t green = (packed >> 10) & 0x3ffu;
            const uint32_t high = (packed >> 20) & 0x3ffu;
            const uint32_t red = blueLow ? high : low;
            const uint32_t blue = blueLow ? low : high;
            const uint32_t alpha = (packed >> 30) & 0x3u;
            const auto to8 = [](uint32_t value) { return (value * 255u + 511u) / 1023u; };
            output[i] = (alpha * 85u << 24) | (to8(red) << 16) |
                        (to8(green) << 8) | to8(blue);
        }
    }
    RetireBuffer(staging);

    if (widthOut)
        *widthOut = width;
    if (heightOut)
        *heightOut = height;

    return true;
}
