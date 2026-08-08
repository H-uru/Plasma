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

#ifndef _plVulkanDevice_h_
#define _plVulkanDevice_h_

#include "HeadSpin.h"
#include "hsColorRGBA.h"
#include "hsMatrix44.h"

#include "plVulkanDeviceRef.h"
#include "plVulkanPipelineState.h"

#include <volk.h>

#include <unordered_map>
#include <unordered_set>

#include <string_theory/string>

#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

struct VmaAllocator_T;
typedef struct VmaAllocator_T* VmaAllocator;

class plBitmap;
class plCubicEnvironmap;
class plGBufferGroup;
class plMipmap;
class plRenderTarget;
class plVulkanPipeline;

/**
 * The Vulkan half of the pl3DPipeline<DeviceType> contract.
 *
 * The template itself only ever calls the five setters and GetErrorString, and
 * only ever forms pointers to the three *Ref types, so those can stay
 * incomplete until there is something to put in them.
 *
 * Everything else here is the frame plumbing that plVulkanPipeline drives:
 * instance and device creation, the swapchain, a ring of command buffers, and a
 * single monotonic timeline semaphore that is the only authority on "the GPU is
 * finished with frame N".
 */
class plVulkanDevice
{
public:
    /*** pl3DPipeline contract ***/
    typedef plVulkanVertexBufferRef VertexBufferRef;
    typedef plVulkanIndexBufferRef  IndexBufferRef;
    typedef plVulkanTextureRef      TextureRef;

    plVulkanDevice();
    ~plVulkanDevice();

    plVulkanDevice(const plVulkanDevice&) = delete;
    plVulkanDevice& operator=(const plVulkanDevice&) = delete;

    /**
     * Redirects rendering. Null means back to the swapchain.
     *
     * Closes whatever rendering scope is open; the next Clear or draw opens a
     * new one against the new target. The caller must invalidate any cached
     * bindings, exactly as a Metal encoder change would force.
     */
    void SetRenderTarget(plRenderTarget* target);

    /** Push our viewport to the command buffer as dynamic state. */
    void SetViewport();

    void SetProjectionMatrix(const hsMatrix44& src);
    void SetWorldToCameraMatrix(const hsMatrix44& src);
    void SetLocalToWorldMatrix(const hsMatrix44& src);

    /**
     * Empty on success, non-empty on failure.
     *
     * plClient::InitPipeline throws the pipeline away and retries with the
     * default device when this is non-empty, so every failure path below has to
     * set it.
     */
    ST::string GetErrorString() const { return fErrorMsg; }

    /*** Setup, driven by plVulkanPipeline ***/

    /** The SDL_Window to present to. Must be set before InitDevice(). */
    void SetWindow(hsWindowHndl window) { fWindow = window; }

    /** Which physical device to prefer, by name. Empty means "first suitable". */
    void SetDeviceName(ST::string name) { fRequestedDevice = std::move(name); }

    bool InitDevice();
    void Shutdown();

    /*** Per-frame ***/

    /**
     * Acquires a swapchain image and opens a command buffer.
     *
     * Returns false when there is nothing to draw to this frame -- a minimized
     * window, or a swapchain that went out of date and was rebuilt. The caller
     * must skip straight to the next frame in that case.
     *
     * No rendering pass is open on return. The engine always calls
     * ClearRenderTarget immediately after BeginRender, and that is what decides
     * the load op, so opening the pass here would clear with a stale color.
     */
    bool BeginFrame();

    /**
     * Clears the current target.
     *
     * Before any rendering pass has been opened this becomes the pass's load op,
     * which is the cheap path. Afterwards it is a mid-pass
     * vkCmdClearAttachments. Null color or depth leaves that aspect alone.
     */
    void Clear(const hsColorRGBA* color, const float* depth);

    /** Closes the rendering pass, submits, and presents. */
    void EndFrame();

    /** Note a new window size. The swapchain is rebuilt on the next BeginFrame. */
    void Resize(uint32_t width, uint32_t height);

    /** Rebuilds the swapchain with the requested presentation policy. */
    void SetVSync(bool enabled);

    /** Selects the highest supported framebuffer sample count not above count. */
    void SetMSAASampleCount(uint32_t count);

    uint32_t Width() const { return fExtent.width; }
    uint32_t Height() const { return fExtent.height; }

    bool IsInited() const { return fDevice != VK_NULL_HANDLE; }

    /** Waits for all previously submitted queue work before direct CPU access. */
    void WaitForIdle();

    /*** Resources ***/

    /**
     * Allocates a buffer.
     *
     * Host-visible buffers come back persistently mapped; device-local ones need
     * a staging copy. Returns an invalid buffer on failure.
     *
     * `name` is what a capture reports this buffer as; see SetObjectName.
     */
    plVulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool hostVisible,
                                const ST::string& name = {});

    /**
     * Hands a buffer to the deferred-destroy queue.
     *
     * Nothing is destroyed until the GPU has passed the timeline value the
     * current frame will signal, so this is safe to call on a resource the
     * in-flight frames may still be reading.
     */
    void RetireBuffer(const plVulkanBuffer& buffer);
    void RetireImage(VkImage image, VkImageView view, VmaAllocation allocation);

    /*** Debug naming, VK_EXT_debug_utils ***/

    /** True when the extension is present and names actually stick. */
    bool HasDebugUtils() const { return fDebugUtils; }

    /**
     * Attaches a human-readable name to a Vulkan object.
     *
     * The name is what the validation layer and RenderDoc report instead of a
     * bare handle, so it should read the way the engine names things -- a
     * plKey name where there is one. A no-op without the extension, and
     * compiled out entirely in an external release.
     */
    template <typename T>
    void SetObjectName(VkObjectType type, T handle, const ST::string& name)
    {
        ISetObjectName(type, DebugHandle(handle), name);
    }

    /** Opens/closes a labelled region in a command buffer. See plVulkanDebugLabel. */
    void BeginLabel(VkCommandBuffer cmd, const ST::string& name);
    void EndLabel(VkCommandBuffer cmd);

    /**
     * Widens either handle representation to the uint64 the extension wants.
     *
     * Non-dispatchable handles are pointers on a 64-bit build and plain
     * integers on a 32-bit one, so neither cast alone covers both.
     */
    static uint64_t DebugHandle(uint64_t handle) { return handle; }
    template <typename T>
    static uint64_t DebugHandle(T* handle) { return reinterpret_cast<uint64_t>(handle); }

    /*** Geometry upload, ported from plMetalDevice.cpp:529-731 ***/

    void SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, VertexBufferRef* vRef);
    void CheckStaticVertexBuffer(VertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx);
    void FillVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx);
    void FillVolatileVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx);

    void SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, IndexBufferRef* iRef);
    void CheckIndexBuffer(IndexBufferRef* iRef);
    void FillIndexBufferRef(IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx);

    /*** Texture upload, ported from plMetalDevice.cpp:732-980 ***/

    void SetupTextureRef(plBitmap* img, TextureRef* tRef);
    void CheckTexture(TextureRef* tRef);
    void MakeTextureRef(TextureRef* tRef, plMipmap* img);
    void MakeCubicTextureRef(TextureRef* tRef, plCubicEnvironmap* img);

    /**
     * Uploads raw pixels into a ref that already names a format and size.
     *
     * For textures the engine hands over outside plBitmap, such as the text
     * font's glyph atlas.
     */
    void CreateTextureFromMemory(TextureRef* tRef, const void* data, size_t size,
                                 const ST::string& name = {});

    /*** Pipeline state ***/

    /**
     * The VkPipeline for this state, built on first request.
     *
     * The key is canonicalized here, so callers do not have to. Returns
     * VK_NULL_HANDLE if the build fails.
     */
    VkPipeline GetPipelineState(plVulkanPipelineKey key);

    VkPipelineLayout GetPipelineLayout() const { return fPipelineLayout; }
    VkDescriptorSetLayout GetUniformSetLayout() const { return fUniformSetLayout; }
    VkDescriptorSetLayout GetTextureSetLayout() const { return fTextureSetLayout; }

    /** One of four samplers, chosen by hsGMatState::hsGMatClampFlags. */
    VkSampler GetSampler(uint32_t clampFlags) const { return fSamplers[clampFlags & 0x3]; }

    /*** Per-draw uniforms and descriptors ***/

    /** A suballocation of the current frame's scratch uniform buffer. */
    struct plScratchAlloc
    {
        VkBuffer     fBuffer = VK_NULL_HANDLE;
        uint32_t     fOffset = 0;
        void*        fMapped = nullptr;

        bool IsValid() const { return fBuffer != VK_NULL_HANDLE; }
    };

    /**
     * Bump-allocates uniform space out of the current frame's scratch ring.
     *
     * The whole ring is reset once per frame, after the timeline proves the GPU
     * is done with it, so the returned memory is valid for exactly this frame.
     */
    /**
     * Allocates scratch bytes and, when declaredRange is nonzero, also proves
     * that a dynamic descriptor of that range fits from the returned offset.
     */
    plScratchAlloc AllocateScratch(size_t size, size_t declaredRange = 0);

    /**
     * The descriptor set naming these textures, written on first sight.
     *
     * Keyed on a content hash of the layers' cookies, so two materials that
     * happen to use the same textures share one set.
     */
    /**
     * The module pair for a plShader pair, if both halves exist.
     *
     * The out parameters may be null, which makes this a support query.
     */
    bool GetShaderModules(uint32_t vertexID, uint32_t fragmentID,
                          VkShaderModule* vertexOut, VkShaderModule* fragmentOut) const;

    /**
     * Copies an offscreen target's pixels back to the CPU.
     *
     * `dest` receives width * height BGRA8 pixels. Blocks until the copy is done,
     * which is fine for the two callers -- screenshots and ExtractMipMap -- and
     * would not be for anything in the frame loop.
     */
    bool ReadRenderTarget(const plVulkanRenderTargetRef* ref, void* dest, size_t destSize);

    /** As above, for the most recently acquired swapchain image. */
    bool ReadSwapchainImage(void* dest, size_t destSize, uint32_t* widthOut, uint32_t* heightOut);

    /**
     * `debugName` names the caller in the warning a slot with nothing to bind
     * produces. A material pass has already dropped such layers to an untextured
     * pass type by the time it gets here (plVulkanMaterialShaderRef::
     * IResolveLayer), so anything reported from here is one of the fixed
     * passes -- shadows, the avatar composite, text -- asking for a texture that
     * does not exist.
     */
    VkDescriptorSet GetTextureDescriptorSet(const plVulkanTextureRef* const* layers,
                                            const uint8_t* clampFlags,
                                            uint32_t count,
                                            const plVulkanRenderTargetRef* shadowMap = nullptr,
                                            const plVulkanRenderTargetRef* const* renderTargets = nullptr,
                                            const ST::string& debugName = {});

    /** The frame-wide set 0, rebound per draw with dynamic offsets. */
    VkDescriptorSet GetUniformDescriptorSet();

    /**
     * Returns the command buffer for a draw, opening dynamic rendering first
     * when a render-target switch closed it.
     */
    VkCommandBuffer CurrentCommandBuffer();

    /**
     * The four transforms, in the column-major form the shaders want.
     *
     * hsMatrix44 is row-major, so the setters copy fMap straight across, which
     * transposes it -- exactly what `v * M` in the shader then undoes. Same
     * trick as hsMatrix2SIMD (plMetalDevice.h:68).
     */
    plVkMat4 fMatrixProj;
    plVkMat4 fMatrixL2W;
    plVkMat4 fMatrixW2C;
    plVkMat4 fMatrixC2W;

    /** Device ceiling for anisotropic filtering. One means it is unsupported. */
    float MaxAnisotropy() const { return fMaxAnisotropy; }
    bool SupportsDepthBiasClamp() const { return fSupportsDepthBiasClamp; }
    uint32_t MaxSampleCount() const { return uint32_t(fMaxSampleCount); }
    VkSampleCountFlagBits CurrentSampleCount() const
    {
        return fCurrentTarget ? VK_SAMPLE_COUNT_1_BIT : fSampleCount;
    }

    /**
     * Rebuilds the samplers at a new anisotropy level.
     *
     * Samplers are baked once at init, so changing the level means recreating them
     * -- and dropping every cached texture descriptor set, because those name the
     * old sampler handles.
     */
    void SetAnisotropy(uint32_t level);

    /**
     * Formats of whatever is currently being rendered into.
     *
     * These feed the pipeline key, so they must describe the active target
     * rather than the swapchain -- a pipeline built for one attachment format
     * cannot be used with another.
     */
    VkFormat CurrentColorFormat() const;
    VkFormat CurrentDepthFormat() const;

    /** Extent of the current target, for viewport and scissor. */
    VkExtent2D CurrentExtent() const;

    /** Allocates the images behind an offscreen target. */
    bool CreateRenderTarget(plVulkanRenderTargetRef* ref, uint32_t width, uint32_t height,
                            bool wantsDepth, const ST::string& name = {});

    /**
     * Allocates one cube image plus a 2D view per face.
     *
     * faces must have six entries. Only the parent ref owns the image; each face
     * gets a view of a single array layer and shares the parent's depth buffer.
     */
    bool CreateCubicRenderTarget(plVulkanRenderTargetRef* parent,
                                 plVulkanRenderTargetRef* const* faces,
                                 uint32_t size, bool wantsDepth, const ST::string& name = {});

    /** Applies a separable Gaussian blur to the active offscreen target. */
    bool BlurCurrentRenderTarget(float sigma);

    /** Replaces the presentation LUT. count is 256 or 1024 entries per channel. */
    bool SetGammaLUT(const uint16_t* tabR, const uint16_t* tabG,
                     const uint16_t* tabB, uint32_t count);
    bool Supports10BitGamma() const { return fSupports10BitSwapchain; }

    plVulkanPipeline* fPipeline;

private:
    static constexpr uint32_t kMaxFramesInFlight = 2;

    /** Log a continuously dry swapchain after one second. */
    static constexpr uint64_t kAcquireLogDelayMs = 1000;

    /** Rebuild if the compositor has returned no image for three seconds. */
    static constexpr uint64_t kAcquireRebuildDelayMs = 3000;

    /** Upper bound on swapchain images, and so on the present semaphore set. */
    static constexpr uint32_t kMaxSwapchainImages = 8;

    /** How many replaced swapchains may be waiting to be freed at once. */
    static constexpr size_t kMaxRetiredSwapchains = 4;

    /** Per-frame-in-flight recording resources. */
    struct plFrame
    {
        VkCommandPool   fPool           = VK_NULL_HANDLE;
        VkCommandBuffer fCmd            = VK_NULL_HANDLE;

        /** Timeline value this frame's submission signals; 0 if never used. */
        uint64_t        fTimelineValue  = 0;
    };

    bool ICreateInstance();
    bool ICreateSurface();
    bool IPickPhysicalDevice();
    bool ICreateDevice();
    bool ICreateAllocator();
    bool ICreateFrames();

    /** Destroys everything the GPU has finished with. */
    void IDrainRetired(uint64_t completedTimeline);

    /**
     * Opens a command buffer for a one-off transfer.
     *
     * IEndOneShot submits it and blocks until it completes. Texture upload runs
     * during resource loading rather than inside the frame loop, so paying a
     * stall there is simpler than threading it through the frame's timeline and
     * costs nothing that matters.
     */
    VkCommandBuffer IBeginOneShot();
    void            IEndOneShot(VkCommandBuffer cmd);

    /**
     * Drops mip levels a compressed image cannot actually supply.
     *
     * Ported from plMetalDevice::ConfigureAllowedLevels: block-compressed levels
     * stop being valid once either dimension is no longer a multiple of four.
     */
    void IConfigureAllowedLevels(TextureRef* tRef, plMipmap* mipmap);

    /** Uploads every mip of one image into a slice of the texture. */
    void IPopulateTexture(TextureRef* tRef, plMipmap* img, uint32_t slice);

    bool CreateRenderTargetImage(plVulkanRenderTargetRef* ref, uint32_t width, uint32_t height,
                                 bool wantsDepth, bool cubic, const ST::string& name);

    bool ICreateShaderModules();
    bool ICreateSamplers();
    bool ICreateDescriptorLayouts();
    bool ICreatePipelineCache();
    void ISavePipelineCache();

    /** Builds one VkPipeline for an already-canonicalized key. */
    VkPipeline IBuildPipeline(const plVulkanPipelineKey& key);

    bool ICreateSwapchain();

    /**
     * Hands the current swapchain over to fRetiredSwapchains.
     *
     * Nothing here can be destroyed on the spot: vkDeviceWaitIdle drains the
     * queue but not the presentation engine, so a present that is still waiting
     * on one of the per-image semaphores keeps that semaphore in a pending-wait
     * state. Destroying it there is undefined behaviour, and on a Wayland
     * compositor it is a good way to wedge WSI for the rest of the run.
     */
    void IRetireSwapchain();

    /** Destroys retired swapchains the timeline has moved far enough past. */
    void IDrainRetiredSwapchains(uint64_t completedTimeline);

    bool ICreateDepthBuffer();

    /** Resolves the gamma-corrected scene image into the acquired swapchain image. */
    bool IPostprocessGamma();

    /** Opens the dynamic rendering pass, clearing to color if one is given. */
    void IBeginRendering(const hsColorRGBA* clearColor, float clearDepth = 1.f);

    /** Closes it, returning an offscreen target to a shader-readable layout. */
    void IEndRendering();

    /** Blocks until the GPU has retired everything up to and including value. */
    void IWaitForTimeline(uint64_t value);

    bool IFail(const char* what, VkResult result);

    void ISetObjectName(VkObjectType type, uint64_t handle, const ST::string& name);

    /** Reports, once per texture set, a slot the set has no image for. */
    void IWarnUnboundSlot(uint64_t hash, uint32_t slot, const ST::string& debugName);

    ST::string      fErrorMsg;
    ST::string      fRequestedDevice;
    hsWindowHndl    fWindow;

    VkInstance          fInstance;
    VkDebugUtilsMessengerEXT fDebugMessenger;

    /** Whether VK_EXT_debug_utils was enabled on the instance. */
    bool                fDebugUtils;

    VkPhysicalDevice    fPhysicalDevice;
    VkDevice            fDevice;
    VkQueue             fQueue;
    uint32_t            fQueueFamily;

    VkSurfaceKHR        fSurface;
    VkSwapchainKHR      fSwapchain;
    VkFormat            fSwapFormat;
    VkExtent2D          fExtent;

    /**
     * Depth buffer, recreated with the swapchain.
     *
     * Plasma's Metal "stencil states" only ever set depth compare and write, so
     * a depth-only format is enough.
     */
    VkImage       fDepthImage;
    VkImageView   fDepthView;
    VmaAllocation fDepthAllocation;
    VkFormat      fDepthFormat;

    /** Multisampled main color attachment; the swapchain image is its resolve target. */
    VkImage       fMSAAColorImage;
    VkImageView   fMSAAColorView;
    VmaAllocation fMSAAColorAllocation;
    VkSampleCountFlagBits fSampleCount;
    VkSampleCountFlagBits fMaxSampleCount;
    uint32_t      fRequestedSampleCount;

    std::vector<VkImage>     fSwapImages;
    std::vector<VkImageView> fSwapViews;

    /**
     * Acquisition semaphores are owned by frames in flight. BeginFrame waits
     * for a frame's timeline value before reusing its semaphore, which proves
     * the preceding queue submission consumed it.
     *
     * Presentation semaphores are owned by swapchain images. Reacquiring an
     * image proves presentation has finished with that image and consumed its
     * previous wait semaphore, making that semaphore safe to signal again.
     */
    VkSemaphore fImageAvailable[kMaxFramesInFlight];
    VkSemaphore fRenderFinished[kMaxSwapchainImages];

    plFrame     fFrames[kMaxFramesInFlight];
    uint32_t    fFrameIndex;
    uint32_t    fImageIndex;

    VkSemaphore fTimeline;
    uint64_t    fTimelineValue;

    VmaAllocator fAllocator;

    /** Command pool for one-shot transfers; see IBeginOneShot. */
    VkCommandPool fUploadPool;

    /*** Pipeline state ***/

    VkDescriptorSetLayout fUniformSetLayout;
    VkDescriptorSetLayout fTextureSetLayout;
    VkPipelineLayout      fPipelineLayout;

    VkShaderModule fVertexShader;
    VkShaderModule fFragmentShader;
    VkShaderModule fPlateVertexShader;
    VkShaderModule fPlateFragmentShader;
    VkShaderModule fTextVertexShader;
    VkShaderModule fTextFragmentShader;
    VkShaderModule fShadowCasterVertexShader;
    VkShaderModule fShadowCasterFragmentShader;
    VkShaderModule fShadowApplyVertexShader;
    VkShaderModule fShadowApplyFragmentShader;
    VkShaderModule fAvatarVertexShader;
    VkShaderModule fAvatarFragmentShader;
    VkShaderModule fFullscreenVertexShader;
    VkShaderModule fBlurFragmentShader;
    VkShaderModule fGammaFragmentShader;

    /** Scratch color target reused by the two blur passes. */
    plVulkanRenderTargetRef* fBlurTarget;

    /** Present-time resources, allocated only after a gamma LUT is installed. */
    plVulkanRenderTargetRef* fGammaSceneTarget;
    plVulkanTextureRef*      fGammaLUT;

    /** The plShader programmable pairs, keyed by plShaderID::ID. */
    std::unordered_map<uint32_t, VkShaderModule> fProgrammableShaders;

    /**
     * Driver-side cache, persisted under plFileSystem::GetInitPath().
     *
     * Both reference RHIs skip this and both hitch the first time a permutation
     * shows up; there is no reason to inherit that.
     */
    VkPipelineCache fPipelineCache;

    std::unordered_map<plVulkanPipelineKey, VkPipeline, plVulkanPipelineKeyHash> fPipelines;

    /**
     * The four hsGMatClampFlags combinations, then a nearest-filtered one for
     * text. These are the distinct sampler objects; the descriptor array they
     * are written into is indexed by layer. See plVulkanShaderTypes.h.
     */
    VkSampler fSamplers[kNumSamplers];

    /*** Scratch uniforms ***/

    /**
     * One host-visible buffer per frame in flight, bump-allocated.
     *
     * Deliberately a single buffer rather than a chain of blocks: everything
     * here is bound through dynamic offsets, and a dynamic offset only indexes
     * within the buffer the descriptor names. Several blocks would mean several
     * VkBuffers, and an offset into the wrong one. If a frame exhausts it the
     * buffer is grown before that frame comes round again.
     */
    struct plScratchBlock
    {
        plVulkanBuffer fBuffer;
        VkDeviceSize   fUsed;

        /** High-water mark, so the next grow knows what to aim for. */
        VkDeviceSize   fWanted;
    };

    plScratchBlock fScratch[kMaxFramesInFlight];

    /** minUniformBufferOffsetAlignment; every suballocation is rounded to it. */
    VkDeviceSize fUniformAlignment;

    void IResetScratch(uint32_t frameIndex);

    /** (Re)allocates a frame's scratch buffer and rewrites its descriptor set. */
    bool IGrowScratch(uint32_t frameIndex, VkDeviceSize size);

    /*** Descriptors ***/

    VkDescriptorPool             fDescriptorPool;
    std::vector<VkDescriptorPool> fDescriptorPools;

    static constexpr uint32_t kDescriptorSetsPerPool = 64;
    static constexpr uint32_t kTextureSetHashBuckets = 256;
    static constexpr uint32_t kTextureSetSlotBlockSize = 1024;
    static constexpr uint32_t kTextureSetReserveBatch = 64;

    /**
     * A stable descriptor slot stored in both an intrusive LRU queue and an
     * intrusive hash chain. This mirrors ref_nri's DescriptorSetAllocator and
     * avoids a separate allocation for every cache entry.
     */
    struct plTextureSetSlot
    {
        uint64_t fHash;
        uint64_t fLastUsedFrame;

        plTextureSetSlot* fQueueNext;
        plTextureSetSlot* fQueuePrev;
        plTextureSetSlot* fHashNext;
        plTextureSetSlot* fHashPrev;

        VkDescriptorSet fSet;
    };

    struct plTextureSetResult
    {
        plTextureSetSlot* fSlot;
        bool              fFound;
    };

    plTextureSetSlot* fTextureSetHash[kTextureSetHashBuckets]{};
    plTextureSetSlot* fTextureSetQueueBegin = nullptr;
    plTextureSetSlot* fTextureSetQueueEnd = nullptr;

    std::vector<std::unique_ptr<plTextureSetSlot[]>> fTextureSetSlotBlocks;
    std::vector<plTextureSetSlot*> fReservedTextureSets;
    size_t fTextureSetSlotBlockIndex = kTextureSetSlotBlockSize;

    /** Monotonic CPU frame number used to age descriptor sets safely. */
    uint64_t fFrameSerial = 0;

    plTextureSetSlot* IAllocateTextureSetSlot();
    void IAttachTextureSetSlot(plTextureSetSlot* slot);
    void IDetachTextureSetSlot(plTextureSetSlot* slot);
    bool IReserveTextureDescriptorSets();
    plTextureSetResult IResolveTextureDescriptorSet(uint64_t hash);
    void IClearTextureDescriptorSets();

    /**
     * Hashes of the sets already reported as having an unbindable slot.
     *
     * A set is written once, but the warning is only worth reading once per
     * combination even across device resets.
     */
    std::unordered_set<uint64_t> fWarnedTextureSets;

    /**
     * Set 0, one per frame in flight.
     *
     * Each names its own frame's scratch buffer, which is what makes the dynamic
     * offsets handed to vkCmdBindDescriptorSets meaningful.
     */
    VkDescriptorSet fUniformSet[kMaxFramesInFlight];

    void IWriteUniformDescriptorSet(uint32_t frameIndex);

    bool ICreateDescriptorPool();
    bool IAllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet* out,
                                const ST::string& name = {});

    /** Whether BC1/BC3 can be sampled directly, or textures need decompressing. */
    bool fSupportsBC;

    /** Whether slope-scaled decal bias can be capped before it pulls through geometry. */
    bool fSupportsDepthBiasClamp;

    /** Device ceiling for sampler anisotropy; 1 means unsupported. */
    float fMaxAnisotropy;

    /**
     * The level the user asked for, clamped to fMaxAnisotropy when sampling.
     *
     * Starts at whatever graphics.ini said and follows ResetDisplayDevice.
     */
    float fRequestedAnisotropy;

    /**
     * A resource waiting for the GPU to finish with it.
     *
     * fRetireAt is the timeline value that must complete before it is safe to
     * destroy; it is stamped when the frame that retired it is submitted.
     */
    struct plRetired
    {
        uint64_t      fRetireAt;
        VkBuffer      fBuffer;
        VkImage       fImage;
        VkImageView   fImageView;
        VmaAllocation fAllocation;
    };

    std::deque<plRetired> fRetired;

    /**
     * A swapchain that has been replaced, plus everything that hangs off it.
     *
     * fRetireAt is deliberately stamped a few frames into the future rather
     * than at the current timeline value: what has to finish here is
     * presentation, which the timeline says nothing about. By the time the new
     * swapchain has presented kMaxFramesInFlight frames of its own, the
     * compositor is long done with the old one's.
     */
    struct plRetiredSwapchain
    {
        uint64_t                 fRetireAt;
        VkSwapchainKHR           fSwapchain;
        std::vector<VkImageView> fViews;

        /** Both rings, concatenated -- they die together with the swapchain. */
        std::vector<VkSemaphore> fSemaphores;
    };

    std::deque<plRetiredSwapchain> fRetiredSwapchains;

    bool fSwapchainDirty;

    /** FIFO when true; otherwise prefer mailbox, then immediate presentation. */
    bool fVSync;

    /** Whether swapchain images were created with transfer-source usage. */
    bool fSwapchainReadable;

    /** An image acquired synchronously by ReadSwapchainImage for the next frame. */
    bool fImagePreacquired;

    /** Whether this frame's submit must consume its image-available semaphore. */
    bool fFrameHasAcquireWait;

    /** Whether this swapchain has produced an image worth reading back. */
    bool fHasPresentedFrame;

    /** Surface capability and current preference for packed 10-bit output. */
    bool fSupports10BitSwapchain;
    bool fWants10BitSwapchain;

    /** A command buffer is open and an image is acquired. */
    bool fFrameOpen;

    /** The main depth image has participated in a rendering scope this frame. */
    bool fMainDepthUsed;

    /** vkCmdBeginRendering has been issued for this frame. */
    bool fRenderingOpen;

    /** Offscreen target currently being rendered into; null is the swapchain. */
    plVulkanRenderTargetRef* fCurrentTarget;

    /** SDL tick at which the current run of unsuccessful acquire polls began. */
    uint64_t fAcquireStallStartMs;

    /** Whether the current acquire stall has already been reported. */
    bool fAcquireStallLogged;
};

/**
 * Scopes a labelled region of a command buffer.
 *
 * The Metal backend pushes and pops its debug groups by hand, and the two
 * halves ended up guarded differently (plMetalPipeline.cpp:1335 vs :1403), so
 * the nesting depth is not the same in every build. Pairing them in a scope
 * guard makes that impossible here.
 */
class plVulkanDebugLabel
{
public:
    plVulkanDebugLabel(plVulkanDevice& device, VkCommandBuffer cmd, const ST::string& name)
        : fDevice(device), fCmd(cmd)
    {
        fDevice.BeginLabel(fCmd, name);
    }

    ~plVulkanDebugLabel() { fDevice.EndLabel(fCmd); }

    plVulkanDebugLabel(const plVulkanDebugLabel&) = delete;
    plVulkanDebugLabel& operator=(const plVulkanDebugLabel&) = delete;

private:
    plVulkanDevice& fDevice;
    VkCommandBuffer fCmd;
};

/** Points volk at SDL's Vulkan loader. Safe to call more than once. */
bool plVulkanInitVolk();

/**
 * hsMatrix44 to the shader's mat4.
 *
 * The row-major to column-major copy transposes, which is what makes the
 * shader's `v * M` evaluate the original matrix.
 */
void IToShaderMatrix(const hsMatrix44& src, plVkMat4& dst);

#endif // _plVulkanDevice_h_
