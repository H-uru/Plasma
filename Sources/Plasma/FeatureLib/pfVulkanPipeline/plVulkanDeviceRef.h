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

#ifndef _plVulkanDeviceRef_h_
#define _plVulkanDeviceRef_h_

#include "hsGDeviceRef.h"

#include <volk.h>

#include <cstdint>
#include <vector>

// VMA's own declaration, repeated so this header does not have to pull in
// vk_mem_alloc.h. A duplicate identical typedef is legal.
struct VmaAllocation_T;
typedef struct VmaAllocation_T* VmaAllocation;

class plBitmap;
class plGBufferGroup;
class plRenderTarget;
class plVulkanDevice;

/**
 * A buffer plus the allocation backing it.
 *
 * Destruction has to be deferred until the GPU is done with the frame that used
 * it, so nothing here frees anything; plVulkanDevice owns that.
 */
struct plVulkanBuffer
{
    VkBuffer      fBuffer     = VK_NULL_HANDLE;
    VmaAllocation fAllocation = nullptr;

    /** Non-null only for host-visible buffers, which stay persistently mapped. */
    void*         fMapped     = nullptr;
    VkDeviceSize  fSize       = 0;

    bool IsValid() const { return fBuffer != VK_NULL_HANDLE; }
};

class plVulkanDeviceRef : public hsGDeviceRef
{
protected:
    plVulkanDeviceRef*  fNext;
    plVulkanDeviceRef** fBack;

public:
    void               Unlink();
    void               Link(plVulkanDeviceRef** back);
    plVulkanDeviceRef* GetNext() const { return fNext; }
    bool               IsLinked() const { return fBack != nullptr; }

    bool HasFlag(uint32_t f) const { return 0 != (fFlags & f); }
    void SetFlag(uint32_t f, bool on)
    {
        if (on)
            fFlags |= f;
        else
            fFlags &= ~f;
    }

    virtual void Release() = 0;

    plVulkanDeviceRef();
    virtual ~plVulkanDeviceRef();
};

/**
 * Stores and recycles buffers so the CPU can record while the GPU renders.
 *
 * A buffer the GPU is still reading must not be rewritten, and a single frame
 * may rewrite the same logical buffer several times (a reflection pass and the
 * main pass can want different index data). So the pool is two-dimensional:
 * frames deep by passes wide, the second dimension growing as an age demands.
 *
 * The pool does not allocate. Callers allocate and hand a buffer over with
 * SetBuffer; the pool owns it from then on and queues the old one for deferred
 * destruction when it is replaced. Static geometry writes once and never grows
 * the pool.
 *
 * Ported from plMetalBufferPoolRef (plMetalDeviceRef.h:99-167). The frame depth
 * is 3 against 2 frames in flight, which means a buffer written on frame N is
 * not touched again until N+3, by which point the timeline wait in
 * plVulkanDevice::BeginFrame has already proven N+1 complete.
 */
class plVulkanBufferPoolRef : public plVulkanDeviceRef
{
public:
    static constexpr uint32_t kPoolDepth = 3;

    plVulkanBufferPoolRef()
        : plVulkanDeviceRef(), fCurrentFrame(), fCurrentPass(), fLastWriteFrameTime()
    { }

    /**
     * Call before a pass writes the buffer. Advances the pool to the slot this
     * pass should use and points fBuffer at whatever is already there, so the
     * caller can decide whether to reuse it or allocate.
     */
    void PrepareForWrite()
    {
        IAdvanceWriteCursor(fFrameTime, fLastWriteFrameTime, fCurrentFrame, fCurrentPass);

        const size_t currentSize = fBuffers[fCurrentFrame].size();
        fBuffer = (fCurrentPass < currentSize) ? fBuffers[fCurrentFrame][fCurrentPass]
                                               : plVulkanBuffer{};
    }

    static void SetFrameTime(uint32_t frameTime) { fFrameTime = frameTime; }

    const plVulkanBuffer& GetBuffer() const { return fBuffer; }

    /** Takes ownership. Retires whatever occupied this slot. */
    void SetBuffer(plVulkanDevice* device, const plVulkanBuffer& buffer);

    void Release() override;

protected:
    static void IAdvanceWriteCursor(uint32_t frameTime, uint32_t& lastWriteFrameTime,
                                    uint32_t& currentFrame, uint32_t& currentPass)
    {
        if (lastWriteFrameTime != frameTime) {
            currentPass = 0;
            lastWriteFrameTime = frameTime;
            currentFrame = (currentFrame + 1) % kPoolDepth;
        } else {
            currentPass++;
        }
    }

    static uint32_t fFrameTime;

    uint32_t fCurrentFrame;
    uint32_t fCurrentPass;
    uint32_t fLastWriteFrameTime;

    plVulkanBuffer              fBuffer;
    std::vector<plVulkanBuffer> fBuffers[kPoolDepth];

    /** Kept so Release() can retire the pool's contents. */
    plVulkanDevice* fDevice = nullptr;
};

class plVulkanVertexBufferRef : public plVulkanBufferPoolRef
{
public:
    plGBufferGroup* fOwner;
    uint32_t        fCount;
    uint32_t        fIndex;
    uint32_t        fVertexSize;
    int32_t         fOffset;
    uint8_t         fFormat;

    /**
     * CPU shadow copy, non-null only for volatile buffers.
     *
     * ISoftwareVertexBlend writes skinned positions and normals here, and
     * IRefreshDynVertices uploads it.
     */
    uint8_t* fData;

    uint32_t fRefTime;

    enum
    {
        kRebuiltSinceUsed = 0x10, // kDirty = 0x1 is in hsGDeviceRef
        kVolatile         = 0x20,
        kSkinned          = 0x40
    };

    bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
    void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

    bool Volatile() const { return HasFlag(kVolatile); }
    void SetVolatile(bool b) { SetFlag(kVolatile, b); }

    bool Skinned() const { return HasFlag(kSkinned); }
    void SetSkinned(bool b) { SetFlag(kSkinned, b); }

    bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
    void SetRefTime(uint32_t t) { fRefTime = t; }

    plVulkanVertexBufferRef()
        : plVulkanBufferPoolRef(), fOwner(), fCount(), fIndex(), fVertexSize(),
          fOffset(), fFormat(), fData(), fRefTime()
    { }

    ~plVulkanVertexBufferRef() override;

    void Link(plVulkanVertexBufferRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanVertexBufferRef* GetNext() const { return (plVulkanVertexBufferRef*)fNext; }

    void Release() override;
};

class plVulkanIndexBufferRef : public plVulkanBufferPoolRef
{
public:
    plGBufferGroup* fOwner;
    uint32_t        fCount;
    uint32_t        fIndex;
    uint32_t        fRefTime;

    enum
    {
        kRebuiltSinceUsed = 0x10,
        kVolatile         = 0x20
    };

    bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
    void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

    bool Volatile() const { return HasFlag(kVolatile); }
    void SetVolatile(bool b) { SetFlag(kVolatile, b); }

    bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
    void SetRefTime(uint32_t t) { fRefTime = t; }

    plVulkanIndexBufferRef()
        : plVulkanBufferPoolRef(), fOwner(), fCount(), fIndex(), fRefTime()
    { }

    ~plVulkanIndexBufferRef() override;

    void Link(plVulkanIndexBufferRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanIndexBufferRef* GetNext() const { return (plVulkanIndexBufferRef*)fNext; }

    void Release() override;
};

class plVulkanTextureRef : public plVulkanDeviceRef
{
public:
    plBitmap* fOwner;

    VkImage       fImage      = VK_NULL_HANDLE;
    VkImageView   fImageView  = VK_NULL_HANDLE;
    VmaAllocation fAllocation = nullptr;

    VkFormat fFormat = VK_FORMAT_UNDEFINED;
    uint32_t fWidth  = 0;
    uint32_t fHeight = 0;

    /** Mip count actually uploaded; see plMetalDevice::ConfigureAllowedLevels. */
    int32_t fLevels = 0;

    /** True for a cube map, which needs a different sampler type in the shader. */
    bool fIsCubic = false;

    /**
     * Stable identity, independent of fImageView.
     *
     * Vulkan reuses handles, so a freed-and-recreated view can land on the same
     * address as one a cached descriptor set still refers to. Hashing this
     * instead means a recreated texture can never be mistaken for the old one.
     */
    uint64_t fCookie = 0;

    plVulkanTextureRef() : plVulkanDeviceRef(), fOwner() { }
    ~plVulkanTextureRef() override;

    void Link(plVulkanTextureRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanTextureRef* GetNext() const { return (plVulkanTextureRef*)fNext; }

    void Release() override;

    plVulkanDevice* fDevice = nullptr;
};

/**
 * An offscreen target: a color image, optionally a depth image, and views.
 *
 * Kept separate from plVulkanTextureRef because the engine reaches for it
 * through plRenderTarget rather than plBitmap, and because it owns a depth
 * buffer a plain texture never has.
 */
class plVulkanRenderTargetRef : public plVulkanDeviceRef
{
public:
    plRenderTarget* fOwner = nullptr;

    VkImage       fImage      = VK_NULL_HANDLE;
    VkImageView   fImageView  = VK_NULL_HANDLE;
    VmaAllocation fAllocation = nullptr;

    VkImage       fDepthImage      = VK_NULL_HANDLE;
    VkImageView   fDepthView       = VK_NULL_HANDLE;
    VmaAllocation fDepthAllocation = nullptr;

    VkFormat fFormat      = VK_FORMAT_UNDEFINED;
    VkFormat fDepthFormat = VK_FORMAT_UNDEFINED;
    uint32_t fWidth       = 0;
    uint32_t fHeight      = 0;

    /** Whether the image is currently readable by a shader. */
    bool fShaderReadable = false;

    /**
     * Current layout of the owned depth image.
     *
     * Cube faces share their parent's depth image, so users must resolve the
     * image-owning parent before reading or updating this value.
     */
    VkImageLayout fDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    /**
     * False for a cube face, which only owns its view.
     *
     * The six faces of a plCubicRenderTarget share one cube image belonging to
     * the parent, so a face must not free the image out from under its siblings.
     */
    bool fOwnsImage = true;

    /** Identity for the descriptor cache; see plVulkanTextureRef::fCookie. */
    uint64_t fCookie = 0;

    plVulkanRenderTargetRef() : plVulkanDeviceRef() { }
    ~plVulkanRenderTargetRef() override;

    void Link(plVulkanRenderTargetRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanRenderTargetRef* GetNext() const { return (plVulkanRenderTargetRef*)fNext; }

    void Release() override;

    plVulkanDevice* fDevice = nullptr;
};

class plLightInfo;
struct plShaderLightSource;

/**
 * A registered light's slot in the per-frame light buffer.
 *
 * Ported from plMetalLightRef. Owns no GPU memory of its own: every light is
 * written into one buffer each frame, and this just remembers where.
 */
class plVulkanLightRef : public plVulkanDeviceRef
{
public:
    plLightInfo* fOwner = nullptr;

    /** Slot in the frame's light buffer, assigned by LoadLightsOnDevice. */
    uint32_t fBufferIndex = 0;

    /** Slot in the current draw's active-light list, assigned by ILoadLight. */
    size_t fPassIndex = 0;

    plVulkanLightRef() : plVulkanDeviceRef() { }
    ~plVulkanLightRef() override;

    void Link(plVulkanLightRef** back) { plVulkanDeviceRef::Link((plVulkanDeviceRef**)back); }
    plVulkanLightRef* GetNext() const { return (plVulkanLightRef*)fNext; }

    void Release() override;

    /** Flattens the owner's type-specific parameters into the shader struct. */
    void UpdateShaderInfo(plShaderLightSource* dst) const;
};

#endif // _plVulkanDeviceRef_h_
