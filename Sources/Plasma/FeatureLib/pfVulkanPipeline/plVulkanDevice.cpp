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

#include "plVulkanDevice.h"
#include "plVulkanPipeline.h"

#include "plProduct.h"
#include "plPipeline/plRenderTarget.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vk_mem_alloc.h>

#include <string_theory/format>

#include <algorithm>
#include <cstring>

namespace
{
    /**
     * volk has to be initialized exactly once per process, and both the
     * enumerator and the device need it.
     */
    bool s_volkReady = false;

    VKAPI_ATTR VkBool32 VKAPI_CALL IDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void*)
    {
        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            hsStatusMessageF("Vulkan: {}", data->pMessage);
        return VK_FALSE;
    }

    bool IHasLayer(const char* wanted)
    {
        uint32_t count = 0;
        if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS)
            return false;

        std::vector<VkLayerProperties> layers(count);
        if (vkEnumerateInstanceLayerProperties(&count, layers.data()) != VK_SUCCESS)
            return false;

        for (const VkLayerProperties& layer : layers) {
            if (strcmp(layer.layerName, wanted) == 0)
                return true;
        }
        return false;
    }

    bool IHasInstanceExtension(const char* wanted)
    {
        uint32_t count = 0;
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) != VK_SUCCESS)
            return false;

        std::vector<VkExtensionProperties> extensions(count);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()) != VK_SUCCESS)
            return false;

        for (const VkExtensionProperties& extension : extensions) {
            if (strcmp(extension.extensionName, wanted) == 0)
                return true;
        }
        return false;
    }

    /**
     * Returns the ref that owns a target's depth image and therefore its layout.
     * Cube faces have individual color views but share the parent's depth image.
     */
    plVulkanRenderTargetRef* IDepthImageOwner(plVulkanRenderTargetRef* target)
    {
        if (!target || !target->fOwner)
            return target;

        plRenderTarget* parent = target->fOwner->GetParent();
        if (!parent)
            return target;

        auto* parentRef = static_cast<plVulkanRenderTargetRef*>(parent->GetDeviceRef());
        return parentRef ? parentRef : target;
    }
}

bool plVulkanInitVolk()
{
    if (s_volkReady)
        return true;

    // Take the loader from SDL so both agree on which libvulkan is in play.
    PFN_vkGetInstanceProcAddr getProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());
    if (getProcAddr) {
        volkInitializeCustom(getProcAddr);
    } else if (volkInitialize() != VK_SUCCESS) {
        return false;
    }

    s_volkReady = true;
    return true;
}

plVulkanDevice::plVulkanDevice()
    : fPipeline(), fWindow(),
      fInstance(VK_NULL_HANDLE), fDebugMessenger(VK_NULL_HANDLE), fDebugUtils(),
      fPhysicalDevice(VK_NULL_HANDLE), fDevice(VK_NULL_HANDLE),
      fQueue(VK_NULL_HANDLE), fQueueFamily(UINT32_MAX),
      fSurface(VK_NULL_HANDLE), fSwapchain(VK_NULL_HANDLE),
      fSwapFormat(VK_FORMAT_UNDEFINED), fExtent{ 0, 0 },
      fDepthImage(VK_NULL_HANDLE), fDepthView(VK_NULL_HANDLE),
      fDepthAllocation(nullptr), fDepthFormat(VK_FORMAT_D32_SFLOAT),
      fMSAAColorImage(VK_NULL_HANDLE), fMSAAColorView(VK_NULL_HANDLE),
      fMSAAColorAllocation(nullptr), fSampleCount(VK_SAMPLE_COUNT_1_BIT),
      fMaxSampleCount(VK_SAMPLE_COUNT_1_BIT), fRequestedSampleCount(1),
      fImageAvailable{}, fRenderFinished{},
      fFrameIndex(), fImageIndex(),
      fTimeline(VK_NULL_HANDLE), fTimelineValue(), fAllocator(nullptr),
      fUploadPool(VK_NULL_HANDLE), fSupportsBC(), fSupportsDepthBiasClamp(),
      fMaxAnisotropy(1.f),
      fUniformSetLayout(VK_NULL_HANDLE), fTextureSetLayout(VK_NULL_HANDLE),
      fPipelineLayout(VK_NULL_HANDLE), fVertexShader(VK_NULL_HANDLE),
      fFragmentShader(VK_NULL_HANDLE), fPlateVertexShader(VK_NULL_HANDLE),
      fPlateFragmentShader(VK_NULL_HANDLE), fTextVertexShader(VK_NULL_HANDLE),
      fTextFragmentShader(VK_NULL_HANDLE), fShadowCasterVertexShader(VK_NULL_HANDLE),
      fShadowCasterFragmentShader(VK_NULL_HANDLE), fShadowApplyVertexShader(VK_NULL_HANDLE),
      fShadowApplyFragmentShader(VK_NULL_HANDLE), fAvatarVertexShader(VK_NULL_HANDLE),
      fAvatarFragmentShader(VK_NULL_HANDLE), fFullscreenVertexShader(VK_NULL_HANDLE),
      fBlurFragmentShader(VK_NULL_HANDLE), fGammaFragmentShader(VK_NULL_HANDLE),
      fBlurTarget(), fGammaSceneTarget(), fGammaLUT(),
      fPipelineCache(VK_NULL_HANDLE), fSamplers{},
      fScratch{}, fUniformAlignment(1), fRequestedAnisotropy(),
      fDescriptorPool(VK_NULL_HANDLE), fUniformSet{},
      fSwapchainDirty(), fVSync(true), fSwapchainReadable(), fImagePreacquired(),
      fFrameHasAcquireWait(), fHasPresentedFrame(),
      fSupports10BitSwapchain(), fWants10BitSwapchain(),
      fFrameOpen(), fMainDepthUsed(), fRenderingOpen(), fCurrentTarget(),
      fAcquireStallStartMs(), fAcquireStallLogged()
{
}

plVulkanDevice::~plVulkanDevice()
{
    Shutdown();
}

bool plVulkanDevice::IFail(const char* what, VkResult result)
{
    fErrorMsg = ST::format("Vulkan: {} failed ({})", what, static_cast<int>(result));
    hsStatusMessage(fErrorMsg);
    return false;
}

void plVulkanDevice::ISetObjectName(VkObjectType type, uint64_t handle, const ST::string& name)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    // vkSetDebugUtilsObjectNameEXT is only loaded when the extension is, and a
    // handle can be null on a failure path the caller has not returned from yet.
    if (!fDebugUtils || fDevice == VK_NULL_HANDLE || handle == 0 || name.empty())
        return;

    VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    info.objectType = type;
    info.objectHandle = handle;
    info.pObjectName = name.c_str();
    vkSetDebugUtilsObjectNameEXT(fDevice, &info);
#endif
}

void plVulkanDevice::BeginLabel(VkCommandBuffer cmd, const ST::string& name)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (!fDebugUtils || cmd == VK_NULL_HANDLE)
        return;

    VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
    label.pLabelName = name.c_str();
    vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
#endif
}

void plVulkanDevice::EndLabel(VkCommandBuffer cmd)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (!fDebugUtils || cmd == VK_NULL_HANDLE)
        return;

    vkCmdEndDebugUtilsLabelEXT(cmd);
#endif
}

VkFormat plVulkanDevice::CurrentColorFormat() const
{
    return fCurrentTarget ? fCurrentTarget->fFormat : fSwapFormat;
}

VkFormat plVulkanDevice::CurrentDepthFormat() const
{
    if (fCurrentTarget)
        return fCurrentTarget->fDepthFormat;
    return fDepthFormat;
}

VkExtent2D plVulkanDevice::CurrentExtent() const
{
    if (fCurrentTarget)
        return VkExtent2D{ fCurrentTarget->fWidth, fCurrentTarget->fHeight };
    return fExtent;
}

void plVulkanDevice::SetRenderTarget(plRenderTarget* target)
{
    plVulkanRenderTargetRef* ref = nullptr;
    if (target)
        ref = static_cast<plVulkanRenderTargetRef*>(target->GetDeviceRef());

    if (ref == fCurrentTarget)
        return;

    // The open scope belongs to the old target, so it has to close first.
    IEndRendering();

    fCurrentTarget = ref;
}

/**
 * Closes the current rendering scope and puts an offscreen target back into a
 * layout a shader can sample.
 */
void plVulkanDevice::IEndRendering()
{
    if (!fRenderingOpen)
        return;

    VkCommandBuffer cmd = fFrames[fFrameIndex].fCmd;
    vkCmdEndRendering(cmd);
    fRenderingOpen = false;

    if (!fCurrentTarget || fCurrentTarget->fImage == VK_NULL_HANDLE)
        return;

    VkImageMemoryBarrier2 toRead{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toRead.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toRead.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toRead.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    toRead.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    toRead.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.image = fCurrentTarget->fImage;
    toRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toRead.subresourceRange.levelCount = 1;
    toRead.subresourceRange.layerCount = 1;

    VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dependency.imageMemoryBarrierCount = 1;
    dependency.pImageMemoryBarriers = &toRead;
    vkCmdPipelineBarrier2(cmd, &dependency);

    fCurrentTarget->fShaderReadable = true;
}

bool plVulkanDevice::GetShaderModules(uint32_t vertexID, uint32_t fragmentID,
                                     VkShaderModule* vertexOut,
                                     VkShaderModule* fragmentOut) const
{
    auto vertex = fProgrammableShaders.find(vertexID);
    auto fragment = fProgrammableShaders.find(fragmentID);

    if (vertex == fProgrammableShaders.end() || fragment == fProgrammableShaders.end())
        return false;

    if (vertexOut)
        *vertexOut = vertex->second;
    if (fragmentOut)
        *fragmentOut = fragment->second;

    return true;
}

VkCommandBuffer plVulkanDevice::CurrentCommandBuffer()
{
    if (!fFrameOpen)
        return VK_NULL_HANDLE;

    // Switching render targets closes dynamic rendering. Draw paths ask for
    // their command buffer immediately before encoding, so this is the common
    // point that resumes the restored target and its dynamic viewport state.
    if (!fRenderingOpen)
        IBeginRendering(nullptr);

    return fFrames[fFrameIndex].fCmd;
}

void plVulkanDevice::SetViewport()
{
    if (!fRenderingOpen)
        return;

    const VkExtent2D extent = CurrentExtent();

    // The pipeline's view transform carries the viewport, which is not always the
    // whole attachment: a shadow slave brings it in by a texel after clearing, so
    // that the map keeps an untouched alpha border to clamp against
    // (plVulkanPipeline::IPushShadowCastState). Metal reads it the same way
    // (plMetalDevice.cpp:596-603).
    int32_t left = 0;
    int32_t top = 0;
    int32_t width = int32_t(extent.width);
    int32_t height = int32_t(extent.height);

    if (fPipeline) {
        const plViewTransform& view = fPipeline->GetViewTransform();
        const int32_t viewLeft = view.GetViewPortLeft();
        const int32_t viewTop = view.GetViewPortTop();
        const int32_t viewWidth = view.GetViewPortWidth();
        const int32_t viewHeight = view.GetViewPortHeight();

        // Only a transform whose screen size is this target's is describing this
        // target: an avatar or projection pass binds a render target without
        // touching the transform, and there the whole attachment is what is
        // wanted. An empty or out-of-bounds rect is also a device-lost-grade
        // error here, where in a fixed-function API it was merely ignored.
        const bool describesTarget = int32_t(view.GetScreenWidth()) == width &&
                                     int32_t(view.GetScreenHeight()) == height;
        if (describesTarget && viewWidth > 0 && viewHeight > 0 &&
            viewLeft >= 0 && viewTop >= 0 &&
            viewLeft + viewWidth <= width && viewTop + viewHeight <= height) {
            left = viewLeft;
            top = viewTop;
            width = viewWidth;
            height = viewHeight;
        }
    }

    VkViewport viewport{};
    viewport.x = float(left);
    // Plasma's projection matrices use the D3D/Metal framebuffer convention.
    // Vulkan's positive viewport height maps clip-space +Y toward the bottom,
    // so anchor at the lower edge and use a negative height to map +Y upward.
    viewport.y = float(top + height);
    viewport.width = float(width);
    viewport.height = -float(height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor{};
    scissor.offset = { left, top };
    scissor.extent = { uint32_t(width), uint32_t(height) };

    VkCommandBuffer cmd = fFrames[fFrameIndex].fCmd;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

/**
 * hsMatrix44 to the shader's mat4.
 *
 * A straight copy of the row-major fMap into a column-major mat4 transposes it,
 * which is what makes the shader's `v * M` evaluate the original. Identity is
 * special-cased because hsMatrix44 does not always keep fMap current when the
 * identity flag is set.
 */
void IToShaderMatrix(const hsMatrix44& src, plVkMat4& dst)
{
    if (src.fFlags & hsMatrix44::kIsIdent) {
        memset(&dst, 0, sizeof(dst));
        dst.m[0] = dst.m[5] = dst.m[10] = dst.m[15] = 1.f;
        return;
    }

    static_assert(sizeof(src.fMap) == sizeof(dst.m), "hsMatrix44 and plVkMat4 must agree");
    memcpy(dst.m, src.fMap, sizeof(dst.m));
}

void plVulkanDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    IToShaderMatrix(src, fMatrixProj);
}

void plVulkanDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    IToShaderMatrix(src, fMatrixW2C);

    hsMatrix44 inverse;
    src.GetInverse(&inverse);
    IToShaderMatrix(inverse, fMatrixC2W);
}

void plVulkanDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
    IToShaderMatrix(src, fMatrixL2W);
}

bool plVulkanDevice::InitDevice()
{
    if (!fWindow) {
        fErrorMsg = ST_LITERAL("Vulkan: no window was set");
        return false;
    }

    if (!plVulkanInitVolk()) {
        fErrorMsg = ST_LITERAL("Vulkan: could not load the Vulkan loader");
        return false;
    }

    if (!ICreateInstance())
        return false;
    if (!ICreateSurface())
        return false;
    if (!IPickPhysicalDevice())
        return false;
    if (!ICreateDevice())
        return false;
    if (!ICreateAllocator())
        return false;
    if (!ICreateFrames())
        return false;
    if (!ICreateShaderModules())
        return false;
    if (!ICreateSamplers())
        return false;
    if (!ICreateDescriptorLayouts())
        return false;
    if (!ICreatePipelineCache())
        return false;
    if (!ICreateDescriptorPool())
        return false;
    if (!ICreateSwapchain())
        return false;

    {
        // Build the most common permutation up front: one textured layer over
        // one UV set. Costs a few milliseconds here instead of a hitch on the
        // first draw, and proves the whole state path before anything renders.
        plVulkanPipelineKey key{};
        key.fPassKind = plVulkanPipelineKey::kPassMaterial;
        key.fNumUVs = 1;
        key.fNumLayers = 1;
        key.fPassTypes[0] = kPassTypeTexture;
        key.fColorFormat = fSwapFormat;
        key.fDepthFormat = fDepthFormat;
        key.fSampleCount = CurrentSampleCount();

        if (GetPipelineState(key) == VK_NULL_HANDLE) {
            if (fErrorMsg.empty())
                fErrorMsg = ST_LITERAL("Vulkan: could not build a pipeline");
            return false;
        }
    }

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(fPhysicalDevice, &props);
    hsStatusMessageF("Vulkan: {} - {}x{} swapchain, {} images",
                     props.deviceName, fExtent.width, fExtent.height, fSwapImages.size());

    return true;
}

bool plVulkanDevice::ICreateInstance()
{
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = plProduct::LongName().c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Plasma";
    appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t sdlExtCount = 0;
    const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
    if (!sdlExts) {
        fErrorMsg = ST::format("Vulkan: SDL could not report the required instance extensions ({})",
                               SDL_GetError());
        return false;
    }

    std::vector<const char*> extensions(sdlExts, sdlExts + sdlExtCount);

    std::vector<const char*> layers;
#ifndef PLASMA_EXTERNAL_RELEASE
    // Two independent decisions: the layer is what checks our API use, the
    // extension is what puts names on objects. A capture in RenderDoc wants the
    // names whether or not the layer is installed.
    const bool validation = IHasLayer("VK_LAYER_KHRONOS_validation");
    if (validation)
        layers.push_back("VK_LAYER_KHRONOS_validation");

    fDebugUtils = IHasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (fDebugUtils)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else
    fDebugUtils = false;
#endif

    VkInstanceCreateInfo info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = uint32_t(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.enabledLayerCount = uint32_t(layers.size());
    info.ppEnabledLayerNames = layers.data();

    VkResult result = vkCreateInstance(&info, nullptr, &fInstance);
    if (result != VK_SUCCESS)
        return IFail("vkCreateInstance", result);

    volkLoadInstance(fInstance);

    if (fDebugUtils) {
        VkDebugUtilsMessengerCreateInfoEXT dbg{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg.pfnUserCallback = IDebugCallback;
        vkCreateDebugUtilsMessengerEXT(fInstance, &dbg, nullptr, &fDebugMessenger);
    }

    return true;
}

bool plVulkanDevice::ICreateSurface()
{
    SDL_Window* window = static_cast<SDL_Window*>(fWindow);
    if (!SDL_Vulkan_CreateSurface(window, fInstance, nullptr, &fSurface)) {
        fErrorMsg = ST::format("Vulkan: could not create a surface ({})", SDL_GetError());
        return false;
    }
    return true;
}

bool plVulkanDevice::IPickPhysicalDevice()
{
    uint32_t count = 0;
    VkResult result = vkEnumeratePhysicalDevices(fInstance, &count, nullptr);
    if (result != VK_SUCCESS || count == 0) {
        fErrorMsg = ST_LITERAL("Vulkan: no devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(count);
    result = vkEnumeratePhysicalDevices(fInstance, &count, devices.data());
    if (result != VK_SUCCESS)
        return IFail("vkEnumeratePhysicalDevices", result);

    VkPhysicalDevice fallback = VK_NULL_HANDLE;
    uint32_t fallbackFamily = UINT32_MAX;

    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);

        // The whole backend is written against features that are core in 1.3.
        if (props.apiVersion < VK_API_VERSION_1_3)
            continue;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

        uint32_t family = UINT32_MAX;
        for (uint32_t i = 0; i < familyCount; ++i) {
            if (!(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                continue;

            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, fSurface, &present);
            if (present) {
                family = i;
                break;
            }
        }

        if (family == UINT32_MAX)
            continue;

        // The device record names the adapter the selector settled on; honor it
        // when we can, otherwise take the first device that works.
        if (!fRequestedDevice.empty() && fRequestedDevice.compare_i(props.deviceName) == 0) {
            fPhysicalDevice = device;
            fQueueFamily = family;
            return true;
        }

        if (fallback == VK_NULL_HANDLE) {
            fallback = device;
            fallbackFamily = family;
        }
    }

    if (fallback == VK_NULL_HANDLE) {
        fErrorMsg = ST_LITERAL("Vulkan: no device supports Vulkan 1.3 with presentation");
        return false;
    }

    fPhysicalDevice = fallback;
    fQueueFamily = fallbackFamily;
    return true;
}

bool plVulkanDevice::ICreateDevice()
{
    const float priority = 1.f;
    VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueInfo.queueFamilyIndex = fQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;
    // glslang compiles `discard` to OpDemoteToHelperInvocation, which the
    // fragment shader needs for the alpha test.
    features13.shaderDemoteToHelperInvocation = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.timelineSemaphore = VK_TRUE;
    // The shaders declare fixed arrays of eight textures but a material rarely
    // fills all of them, so the unused slots must be allowed to stay empty.
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    // Gives uniform blocks C layout rules, which is what makes one struct
    // definition serve both C++ and GLSL. See ShaderSrc/plVulkanShaderTypes.h.
    features12.scalarBlockLayout = VK_TRUE;
    features12.pNext = &features13;

    VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    features.pNext = &features12;

    // BC compression, anisotropy, and depth-bias clamping are optional Vulkan
    // features. Enable them when present, but do not reject an otherwise usable
    // adapter when absent.
    VkPhysicalDeviceFeatures supportedFeatures{};
    vkGetPhysicalDeviceFeatures(fPhysicalDevice, &supportedFeatures);
    features.features.textureCompressionBC = supportedFeatures.textureCompressionBC;
    features.features.samplerAnisotropy = supportedFeatures.samplerAnisotropy;
    features.features.depthBiasClamp = supportedFeatures.depthBiasClamp;

    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    info.pNext = &features;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queueInfo;
    info.enabledExtensionCount = uint32_t(std::size(extensions));
    info.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateDevice(fPhysicalDevice, &info, nullptr, &fDevice);
    if (result != VK_SUCCESS)
        return IFail("vkCreateDevice", result);

    volkLoadDevice(fDevice);
    vkGetDeviceQueue(fDevice, fQueueFamily, 0, &fQueue);

    {
        // Plasma's textures ship DXT-compressed. Every desktop GPU can sample
        // them; if one cannot, the decompress-on-load path has to run instead.
        fSupportsBC = supportedFeatures.textureCompressionBC == VK_TRUE;
        fSupportsDepthBiasClamp = supportedFeatures.depthBiasClamp == VK_TRUE;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(fPhysicalDevice, &props);
        fMaxAnisotropy = supportedFeatures.samplerAnisotropy ? props.limits.maxSamplerAnisotropy : 1.f;

        const VkSampleCountFlags framebufferSamples =
            props.limits.framebufferColorSampleCounts &
            props.limits.framebufferDepthSampleCounts;
        for (VkSampleCountFlagBits count : { VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT,
                                             VK_SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_8_BIT,
                                             VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT }) {
            if (framebufferSamples & count) {
                fMaxSampleCount = count;
                break;
            }
        }
        SetMSAASampleCount(fRequestedSampleCount);

        // graphics.ini's value is the starting point; ResetDisplayDevice moves it.
        fRequestedAnisotropy = float(plPipeline::fInitialPipeParams.AnisotropicLevel);
        if (!fSupportsBC)
            hsStatusMessage("Vulkan: no BC texture support; compressed textures will be decoded in software");
    }

    {
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = fQueueFamily;

        result = vkCreateCommandPool(fDevice, &poolInfo, nullptr, &fUploadPool);
        if (result != VK_SUCCESS)
            return IFail("vkCreateCommandPool (upload)", result);

        SetObjectName(VK_OBJECT_TYPE_COMMAND_POOL, fUploadPool, ST_LITERAL("upload pool"));
    }

    VkSemaphoreTypeCreateInfo timelineType{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
    timelineType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineType.initialValue = 0;

    VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    semInfo.pNext = &timelineType;

    result = vkCreateSemaphore(fDevice, &semInfo, nullptr, &fTimeline);
    if (result != VK_SUCCESS)
        return IFail("vkCreateSemaphore (timeline)", result);

    // The one semaphore every frame's completion is proved against, so a
    // capture that stalls is usually a question about this object.
    SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, fTimeline, ST_LITERAL("frame timeline"));

    SetObjectName(VK_OBJECT_TYPE_DEVICE, fDevice, ST_LITERAL("Plasma device"));

    return true;
}

bool plVulkanDevice::ICreateAllocator()
{
    // volk resolves entry points at runtime, so VMA has to be handed the table
    // rather than resolving them itself.
    VmaVulkanFunctions functions{};
    functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo info{};
    info.vulkanApiVersion = VK_API_VERSION_1_3;
    info.instance = fInstance;
    info.physicalDevice = fPhysicalDevice;
    info.device = fDevice;
    info.pVulkanFunctions = &functions;

    VkResult result = vmaCreateAllocator(&info, &fAllocator);
    if (result != VK_SUCCESS)
        return IFail("vmaCreateAllocator", result);

    return true;
}

plVulkanBuffer plVulkanDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                            bool hostVisible, const ST::string& name)
{
    plVulkanBuffer buffer{};
    if (size == 0)
        return buffer;

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (hostVisible) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                          VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocationInfo allocated{};
    VkResult result = vmaCreateBuffer(fAllocator, &bufferInfo, &allocInfo,
                                      &buffer.fBuffer, &buffer.fAllocation, &allocated);
    if (result != VK_SUCCESS) {
        IFail("vmaCreateBuffer", result);
        return plVulkanBuffer{};
    }

    buffer.fMapped = hostVisible ? allocated.pMappedData : nullptr;
    buffer.fSize = size;

    SetObjectName(VK_OBJECT_TYPE_BUFFER, buffer.fBuffer, name);
    return buffer;
}

void plVulkanDevice::RetireBuffer(const plVulkanBuffer& buffer)
{
    if (!buffer.IsValid())
        return;

    // Retiring at the value this frame will signal, not the last one signalled:
    // the frame being recorded may still reference the buffer.
    fRetired.push_back(plRetired{ fTimelineValue + 1, buffer.fBuffer,
                                  VK_NULL_HANDLE, VK_NULL_HANDLE, buffer.fAllocation });
}

void plVulkanDevice::RetireImage(VkImage image, VkImageView view, VmaAllocation allocation)
{
    if (image == VK_NULL_HANDLE && view == VK_NULL_HANDLE)
        return;

    fRetired.push_back(plRetired{ fTimelineValue + 1, VK_NULL_HANDLE, image, view, allocation });
}

VkCommandBuffer plVulkanDevice::IBeginOneShot()
{
    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = fUploadPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(fDevice, &allocInfo, &cmd);
    if (result != VK_SUCCESS) {
        IFail("vkAllocateCommandBuffers (upload)", result);
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin);

    return cmd;
}

void plVulkanDevice::IEndOneShot(VkCommandBuffer cmd)
{
    if (cmd == VK_NULL_HANDLE)
        return;

    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    cmdInfo.commandBuffer = cmd;

    VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cmdInfo;

    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence fence = VK_NULL_HANDLE;
    vkCreateFence(fDevice, &fenceInfo, nullptr, &fence);

    if (vkQueueSubmit2(fQueue, 1, &submit, fence) == VK_SUCCESS)
        vkWaitForFences(fDevice, 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(fDevice, fence, nullptr);
    vkFreeCommandBuffers(fDevice, fUploadPool, 1, &cmd);
}

void plVulkanDevice::IDrainRetired(uint64_t completedTimeline)
{
    while (!fRetired.empty() && fRetired.front().fRetireAt <= completedTimeline) {
        const plRetired& dead = fRetired.front();

        if (dead.fImageView != VK_NULL_HANDLE)
            vkDestroyImageView(fDevice, dead.fImageView, nullptr);
        if (dead.fImage != VK_NULL_HANDLE)
            vmaDestroyImage(fAllocator, dead.fImage, dead.fAllocation);
        else if (dead.fBuffer != VK_NULL_HANDLE)
            vmaDestroyBuffer(fAllocator, dead.fBuffer, dead.fAllocation);

        fRetired.pop_front();
    }
}

bool plVulkanDevice::ICreateFrames()
{
    uint32_t index = 0;
    for (plFrame& frame : fFrames) {
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex = fQueueFamily;

        VkResult result = vkCreateCommandPool(fDevice, &poolInfo, nullptr, &frame.fPool);
        if (result != VK_SUCCESS)
            return IFail("vkCreateCommandPool", result);

        VkCommandBufferAllocateInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdInfo.commandPool = frame.fPool;
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandBufferCount = 1;

        result = vkAllocateCommandBuffers(fDevice, &cmdInfo, &frame.fCmd);
        if (result != VK_SUCCESS)
            return IFail("vkAllocateCommandBuffers", result);

        SetObjectName(VK_OBJECT_TYPE_COMMAND_POOL, frame.fPool,
                      ST::format("frame {} pool", index));
        SetObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, frame.fCmd,
                      ST::format("frame {} commands", index));
        index++;
    }

    return true;
}

bool plVulkanDevice::ICreateSwapchain()
{
    // The image views and per-image semaphores about to go away can still be
    // referenced by the other frame in flight, and the timeline wait in
    // BeginFrame only covers the frame being started. This settles the queue
    // side of that; the presentation side is what IRetireSwapchain is for.
    if (fSwapchain != VK_NULL_HANDLE)
        vkDeviceWaitIdle(fDevice);

    VkSurfaceCapabilitiesKHR caps{};
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fSurface, &caps);
    if (result != VK_SUCCESS)
        return IFail("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result);

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        // The compositor is letting us choose. Use what the window says it is.
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(fWindow), &w, &h);
        extent.width = std::clamp(uint32_t(w), caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(uint32_t(h), caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    if (extent.width == 0 || extent.height == 0) {
        // Minimized. Tear the swapchain down and stay dirty so we try again once
        // the window comes back; BeginFrame bails while there is no swapchain.
        IRetireSwapchain();
        fExtent = extent;
        return true;
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &formatCount, nullptr);
    if (formatCount == 0) {
        fErrorMsg = ST_LITERAL("Vulkan: the surface reports no formats");
        return false;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &formatCount, formats.data());

    auto isTenBit = [](VkFormat format) {
        return format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
               format == VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    };
    auto isTenBitSurface = [&](const VkSurfaceFormatKHR& format) {
        return isTenBit(format.format) &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    };

    fSupports10BitSwapchain = std::any_of(formats.begin(), formats.end(),
                                           isTenBitSurface);

    VkSurfaceFormatKHR chosen = formats[0];
    if (fWants10BitSwapchain && fSupports10BitSwapchain) {
        for (const VkSurfaceFormatKHR& format : formats) {
            if (isTenBitSurface(format)) {
                chosen = format;
                break;
            }
        }
    } else {
        for (const VkSurfaceFormatKHR& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosen = format;
                break;
            }
        }
    }

    // MAILBOX returns superseded images to the application instead of letting
    // an unconsumed FIFO frame callback retain the whole swapchain. This matters
    // on Wayland during the hidden-to-visible startup transition: RADV can
    // otherwise leave every image queued even though all GPU work completed.
    // FIFO remains the required, universally-supported fallback.
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t presentModeCount = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface,
                                                  &presentModeCount, nullptr) == VK_SUCCESS &&
        presentModeCount > 0) {
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        if (!fVSync &&
            vkGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface,
                                                      &presentModeCount,
                                                      presentModes.data()) == VK_SUCCESS) {
            if (std::find(presentModes.begin(), presentModes.end(),
                          VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end()) {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            } else if (std::find(presentModes.begin(), presentModes.end(),
                                 VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end()) {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0)
        imageCount = std::min(imageCount, caps.maxImageCount);
    imageCount = std::min(imageCount, kMaxSwapchainImages);

    VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    info.surface = fSurface;
    info.minImageCount = imageCount;
    info.imageFormat = chosen.format;
    info.imageColorSpace = chosen.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    fSwapchainReadable = (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;
    if (fSwapchainReadable)
        info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = caps.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = presentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = fSwapchain;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    result = vkCreateSwapchainKHR(fDevice, &info, nullptr, &swapchain);

    // The old swapchain is retired by the create call whether it succeeded or
    // not, so it has to be let go of now -- but only handed to the retire
    // queue, since its images may still be queued for presentation.
    IRetireSwapchain();

    if (result != VK_SUCCESS)
        return IFail("vkCreateSwapchainKHR", result);

    fSwapchain = swapchain;
    fSwapFormat = chosen.format;
    fExtent = extent;

    SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, fSwapchain, ST_LITERAL("swapchain"));

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(fDevice, fSwapchain, &count, nullptr);
    count = std::min(count, kMaxSwapchainImages);
    fSwapImages.resize(count);
    vkGetSwapchainImagesKHR(fDevice, fSwapchain, &count, fSwapImages.data());

    fSwapViews.resize(count, VK_NULL_HANDLE);

    for (uint32_t i = 0; i < count; ++i) {
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = fSwapImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = fSwapFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(fDevice, &viewInfo, nullptr, &fSwapViews[i]);
        if (result != VK_SUCCESS)
            return IFail("vkCreateImageView", result);

        SetObjectName(VK_OBJECT_TYPE_IMAGE, fSwapImages[i], ST::format("swapchain image {}", i));
        SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, fSwapViews[i],
                      ST::format("swapchain view {}", i));
    }

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        result = vkCreateSemaphore(fDevice, &semInfo, nullptr, &fImageAvailable[i]);
        if (result != VK_SUCCESS)
            return IFail("vkCreateSemaphore (acquire)", result);

        SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, fImageAvailable[i],
                      ST::format("image available {}", i));
    }

    // One present semaphore per image, not per CPU frame. Presentation can
    // outlive the submission timeline; reacquiring an image is the only core
    // Vulkan operation that proves its previous present has released it.
    for (uint32_t i = 0; i < count; ++i) {
        VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        result = vkCreateSemaphore(fDevice, &semInfo, nullptr, &fRenderFinished[i]);
        if (result != VK_SUCCESS)
            return IFail("vkCreateSemaphore (present)", result);

        SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, fRenderFinished[i],
                      ST::format("render finished {}", i));
    }

    if (!ICreateDepthBuffer())
        return false;

    if (fGammaLUT && fGammaLUT->fImage != VK_NULL_HANDLE) {
        if (!fGammaSceneTarget)
            fGammaSceneTarget = new plVulkanRenderTargetRef();
        if (!CreateRenderTarget(fGammaSceneTarget, fExtent.width, fExtent.height, false,
                                ST_LITERAL("gamma scene target")))
            return false;
    }

    fSwapchainDirty = false;
    return true;
}

bool plVulkanDevice::ICreateDepthBuffer()
{
    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = fDepthFormat;
    imageInfo.extent = { fExtent.width, fExtent.height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = fSampleCount;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkResult result = vmaCreateImage(fAllocator, &imageInfo, &allocInfo,
                                     &fDepthImage, &fDepthAllocation, nullptr);
    if (result != VK_SUCCESS)
        return IFail("vmaCreateImage (depth)", result);

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = fDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = fDepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(fDevice, &viewInfo, nullptr, &fDepthView);
    if (result != VK_SUCCESS)
        return IFail("vkCreateImageView (depth)", result);

    SetObjectName(VK_OBJECT_TYPE_IMAGE, fDepthImage, ST_LITERAL("swapchain depth"));
    SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, fDepthView, ST_LITERAL("swapchain depth view"));

    if (fSampleCount != VK_SAMPLE_COUNT_1_BIT) {
        VkImageCreateInfo colorInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        colorInfo.imageType = VK_IMAGE_TYPE_2D;
        colorInfo.format = fSwapFormat;
        colorInfo.extent = { fExtent.width, fExtent.height, 1 };
        colorInfo.mipLevels = 1;
        colorInfo.arrayLayers = 1;
        colorInfo.samples = fSampleCount;
        colorInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        colorInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        colorInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        result = vmaCreateImage(fAllocator, &colorInfo, &allocInfo,
                                &fMSAAColorImage, &fMSAAColorAllocation, nullptr);
        if (result != VK_SUCCESS)
            return IFail("vmaCreateImage (MSAA color)", result);

        viewInfo.image = fMSAAColorImage;
        viewInfo.format = fSwapFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        result = vkCreateImageView(fDevice, &viewInfo, nullptr, &fMSAAColorView);
        if (result != VK_SUCCESS)
            return IFail("vkCreateImageView (MSAA color)", result);

        SetObjectName(VK_OBJECT_TYPE_IMAGE, fMSAAColorImage, ST_LITERAL("MSAA color"));
        SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, fMSAAColorView,
                      ST_LITERAL("MSAA color view"));
    }

    return true;
}

void plVulkanDevice::IRetireSwapchain()
{
    if (fDevice == VK_NULL_HANDLE)
        return;

    fImagePreacquired = false;
    fFrameHasAcquireWait = false;
    fHasPresentedFrame = false;

    // The depth buffer is ours alone -- no present ever touches it -- so the
    // ordinary timeline-stamped queue is enough for it.
    if (fDepthView != VK_NULL_HANDLE || fDepthImage != VK_NULL_HANDLE) {
        fRetired.push_back(plRetired{ fTimelineValue + 1, VK_NULL_HANDLE,
                                      fDepthImage, fDepthView, fDepthAllocation });
        fDepthView = VK_NULL_HANDLE;
        fDepthImage = VK_NULL_HANDLE;
        fDepthAllocation = nullptr;
    }
    if (fMSAAColorView != VK_NULL_HANDLE || fMSAAColorImage != VK_NULL_HANDLE) {
        fRetired.push_back(plRetired{ fTimelineValue + 1, VK_NULL_HANDLE,
                                      fMSAAColorImage, fMSAAColorView,
                                      fMSAAColorAllocation });
        fMSAAColorView = VK_NULL_HANDLE;
        fMSAAColorImage = VK_NULL_HANDLE;
        fMSAAColorAllocation = nullptr;
    }

    if (fSwapchain == VK_NULL_HANDLE && fSwapViews.empty() &&
        fImageAvailable[0] == VK_NULL_HANDLE && fRenderFinished[0] == VK_NULL_HANDLE)
        return;

    // Nothing drains this queue but a frame that gets submitted, so a run of
    // rebuilds that never manages to present would grow it without bound. Well
    // before that matters, stop and settle up the hard way -- we are already
    // paying for a rebuild here, so the wait costs nothing anyone can see.
    if (fRetiredSwapchains.size() >= kMaxRetiredSwapchains) {
        vkDeviceWaitIdle(fDevice);
        IDrainRetiredSwapchains(UINT64_MAX);
    }

    plRetiredSwapchain dead{};
    // Not the current timeline value: what has to drain here is presentation,
    // which the timeline does not track. Give the compositor a full pipeline's
    // worth of new frames to let go of the old images first.
    dead.fRetireAt = fTimelineValue + kMaxFramesInFlight + 1;
    dead.fSwapchain = fSwapchain;
    dead.fViews = std::move(fSwapViews);

    // Both rings go with it. A retired swapchain can easily be leaving a
    // signalled semaphore behind -- a present that was rejected never ran its
    // wait -- which is exactly why they are never reused across a rebuild.
    for (VkSemaphore& sem : fImageAvailable) {
        if (sem != VK_NULL_HANDLE)
            dead.fSemaphores.push_back(sem);
        sem = VK_NULL_HANDLE;
    }
    for (VkSemaphore& sem : fRenderFinished) {
        if (sem != VK_NULL_HANDLE)
            dead.fSemaphores.push_back(sem);
        sem = VK_NULL_HANDLE;
    }

    fRetiredSwapchains.push_back(std::move(dead));

    fSwapViews.clear();
    fSwapImages.clear();
    fSwapchain = VK_NULL_HANDLE;
}

void plVulkanDevice::IDrainRetiredSwapchains(uint64_t completedTimeline)
{
    if (fDevice == VK_NULL_HANDLE)
        return;

    while (!fRetiredSwapchains.empty() &&
           fRetiredSwapchains.front().fRetireAt <= completedTimeline) {
        plRetiredSwapchain& dead = fRetiredSwapchains.front();

        // Views first -- they name images the swapchain owns. The swapchain
        // next, because destroying it is what releases any presentation still
        // pending on the semaphores. Only then are the semaphores unreferenced
        // and safe to destroy.
        for (VkImageView view : dead.fViews) {
            if (view != VK_NULL_HANDLE)
                vkDestroyImageView(fDevice, view, nullptr);
        }

        if (dead.fSwapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(fDevice, dead.fSwapchain, nullptr);

        for (VkSemaphore sem : dead.fSemaphores) {
            if (sem != VK_NULL_HANDLE)
                vkDestroySemaphore(fDevice, sem, nullptr);
        }

        fRetiredSwapchains.pop_front();
    }
}

void plVulkanDevice::IWaitForTimeline(uint64_t value)
{
    if (value == 0)
        return;

    VkSemaphoreWaitInfo wait{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
    wait.semaphoreCount = 1;
    wait.pSemaphores = &fTimeline;
    wait.pValues = &value;

    vkWaitSemaphores(fDevice, &wait, UINT64_MAX);
}

void plVulkanDevice::Resize(uint32_t width, uint32_t height)
{
    fSwapchainDirty = true;
}

void plVulkanDevice::WaitForIdle()
{
    if (fDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(fDevice);
}

void plVulkanDevice::SetVSync(bool enabled)
{
    if (fVSync == enabled)
        return;
    fVSync = enabled;
    fSwapchainDirty = true;
}

void plVulkanDevice::SetMSAASampleCount(uint32_t count)
{
    fRequestedSampleCount = std::max(1u, count);

    VkSampleCountFlagBits selected = VK_SAMPLE_COUNT_1_BIT;
    for (VkSampleCountFlagBits candidate : { VK_SAMPLE_COUNT_64_BIT,
                                             VK_SAMPLE_COUNT_32_BIT,
                                             VK_SAMPLE_COUNT_16_BIT,
                                             VK_SAMPLE_COUNT_8_BIT,
                                             VK_SAMPLE_COUNT_4_BIT,
                                             VK_SAMPLE_COUNT_2_BIT }) {
        if (uint32_t(candidate) <= fRequestedSampleCount &&
            uint32_t(candidate) <= uint32_t(fMaxSampleCount)) {
            selected = candidate;
            break;
        }
    }

    if (selected != fSampleCount) {
        fSampleCount = selected;
        fSwapchainDirty = true;
    }
}

bool plVulkanDevice::SetGammaLUT(const uint16_t* tabR, const uint16_t* tabG,
                                 const uint16_t* tabB, uint32_t count)
{
    if (!tabR || !tabG || !tabB || (count != 256 && count != 1024) ||
        fDevice == VK_NULL_HANDLE || fFrameOpen) {
        return false;
    }

    const bool wantsTenBit = count == 1024;
    if (wantsTenBit && !fSupports10BitSwapchain)
        return false;
    if (fWants10BitSwapchain != wantsTenBit) {
        fWants10BitSwapchain = wantsTenBit;
        fSwapchainDirty = true;
    }

    std::vector<uint16_t> pixels(size_t(count) * 3);
    memcpy(pixels.data(), tabR, size_t(count) * sizeof(uint16_t));
    memcpy(pixels.data() + count, tabG, size_t(count) * sizeof(uint16_t));
    memcpy(pixels.data() + count * 2, tabB, size_t(count) * sizeof(uint16_t));

    if (!fGammaLUT)
        fGammaLUT = new plVulkanTextureRef();
    fGammaLUT->fFormat = VK_FORMAT_R16_UNORM;
    fGammaLUT->fWidth = count;
    fGammaLUT->fHeight = 3;
    CreateTextureFromMemory(fGammaLUT, pixels.data(), pixels.size() * sizeof(uint16_t),
                            ST_LITERAL("gamma LUT"));
    if (fGammaLUT->fImage == VK_NULL_HANDLE)
        return false;

    if (fExtent.width != 0 && fExtent.height != 0) {
        if (!fGammaSceneTarget)
            fGammaSceneTarget = new plVulkanRenderTargetRef();
        if (fGammaSceneTarget->fImage == VK_NULL_HANDLE ||
            fGammaSceneTarget->fWidth != fExtent.width ||
            fGammaSceneTarget->fHeight != fExtent.height ||
            fGammaSceneTarget->fFormat != fSwapFormat) {
            if (!CreateRenderTarget(fGammaSceneTarget, fExtent.width, fExtent.height, false,
                                    ST_LITERAL("gamma scene target")))
                return false;
        }
    }

    return true;
}

void plVulkanDevice::SetAnisotropy(uint32_t level)
{
    if (fDevice == VK_NULL_HANDLE)
        return;

    if (float(level) == fRequestedAnisotropy)
        return;

    fRequestedAnisotropy = float(level);

    // Descriptor sets in flight name the samplers about to be destroyed.
    vkDeviceWaitIdle(fDevice);

    for (VkSampler& sampler : fSamplers) {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(fDevice, sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
    }

    // The cached sets hold the old handles, so they have to go with them. Every
    // pool is reset, not just the current one: sets allocated from an earlier pool
    // would otherwise survive naming a destroyed sampler.
    IClearTextureDescriptorSets();
    for (VkDescriptorPool pool : fDescriptorPools)
        vkResetDescriptorPool(fDevice, pool, 0);

    // Resetting the pool freed the uniform sets too; they are rewritten lazily.
    for (VkDescriptorSet& set : fUniformSet)
        set = VK_NULL_HANDLE;

    ICreateSamplers();
}

bool plVulkanDevice::BeginFrame()
{
    if (fDevice == VK_NULL_HANDLE || fFrameOpen)
        return false;

    plFrame& frame = fFrames[fFrameIndex];

    // Do not touch this frame's command pool until the GPU is done with the last
    // submission that used it.
    IWaitForTimeline(frame.fTimelineValue);

    // Whatever the GPU has passed is now safe to destroy.
    uint64_t completed = 0;
    if (vkGetSemaphoreCounterValue(fDevice, fTimeline, &completed) == VK_SUCCESS) {
        IDrainRetired(completed);
        IDrainRetiredSwapchains(completed);
    }

    // The wait above proves the GPU is done reading this frame's uniforms.
    IResetScratch(fFrameIndex);

    // The one place the swapchain is ever rebuilt. Everything else -- Resize, a
    // rejected present, a run of unsuccessful acquire polls -- only raises the flag.
    //
    // The extent is compared against what the surface reports now, not against
    // what was last asked for, so a compositor that clamps the size does not
    // turn into a rebuild every frame. This compare is what lets a SUBOPTIMAL
    // present be ignored: the frame it produced was fine, and if the size really
    // did drift, this catches it on the next frame.
    if (!fSwapchainDirty && fSwapchain != VK_NULL_HANDLE) {
        VkSurfaceCapabilitiesKHR caps{};
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fSurface, &caps) == VK_SUCCESS &&
            caps.currentExtent.width != UINT32_MAX &&
            (caps.currentExtent.width != fExtent.width ||
             caps.currentExtent.height != fExtent.height)) {
            fSwapchainDirty = true;
        }
    }

    if (fSwapchainDirty || fSwapchain == VK_NULL_HANDLE) {
        if (!ICreateSwapchain())
            return false;
        if (fSwapchain == VK_NULL_HANDLE)
            return false; // minimized
    }

    VkResult result = VK_SUCCESS;
    fFrameHasAcquireWait = false;
    if (fImagePreacquired) {
        // ReadSwapchainImage acquired this image with a fence and waited for it,
        // so no semaphore has to be consumed by this submission.
        fImagePreacquired = false;
    } else {
        // Do not enter a driver-side wait here. RADV's Wayland WSI can occasionally
        // remain in its syncobj wait when a window is being mapped or resized, even
        // when a finite Vulkan timeout was supplied. A nonblocking acquire lets the
        // caller pump SDL again; the one-millisecond backoff below prevents a dry
        // swapchain from turning into a busy loop.
        result = vkAcquireNextImageKHR(fDevice, fSwapchain, 0,
                                       fImageAvailable[fFrameIndex],
                                       VK_NULL_HANDLE, &fImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // The acquire semaphore was not signalled, so it is still clean. Rebuild
            // and let the caller skip this frame -- the rebuild is what refreshes
            // the ring, and nothing here waits on a semaphore that never fired.
            fSwapchainDirty = true;
            fAcquireStallStartMs = 0;
            fAcquireStallLogged = false;
            return false;
        }
        if (result == VK_TIMEOUT || result == VK_NOT_READY) {
            // The semaphore was not signalled, so this frame slot remains reusable.
            const uint64_t now = SDL_GetTicks();
            if (fAcquireStallStartMs == 0)
                fAcquireStallStartMs = now;
            const uint64_t elapsed = now - fAcquireStallStartMs;

            if (!fAcquireStallLogged && elapsed >= kAcquireLogDelayMs) {
                hsStatusMessageF("Vulkan: no swapchain image has been available for {} ms; "
                                 "is the window visible?", elapsed);
                fAcquireStallLogged = true;
            }

            if (elapsed >= kAcquireRebuildDelayMs) {
                hsStatusMessage("Vulkan: rebuilding a swapchain that remained unavailable");
                fSwapchainDirty = true;
                fAcquireStallStartMs = 0;
                fAcquireStallLogged = false;
            }

            SDL_Delay(1);
            return false;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            return IFail("vkAcquireNextImageKHR", result);

        // Past this point fImageAvailable[fFrameIndex] is signalled and the image
        // belongs to us. Every way out of here has to either submit the frame, which
        // consumes the semaphore, or throw the swapchain away -- there is no path
        // that may leave it signalled and go round again.
        fFrameHasAcquireWait = true;
    }

    fAcquireStallStartMs = 0;
    fAcquireStallLogged = false;
    ++fFrameSerial;

    vkResetCommandPool(fDevice, frame.fPool, 0);

    VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    result = vkBeginCommandBuffer(frame.fCmd, &begin);
    if (result != VK_SUCCESS) {
        fSwapchainDirty = true;
        return IFail("vkBeginCommandBuffer", result);
    }

    VkImageMemoryBarrier2 toAttachment{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toAttachment.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    toAttachment.srcAccessMask = VK_ACCESS_2_NONE;
    toAttachment.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toAttachment.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toAttachment.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toAttachment.image = fSwapImages[fImageIndex];
    toAttachment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toAttachment.subresourceRange.levelCount = 1;
    toAttachment.subresourceRange.layerCount = 1;

    // The main depth buffer is discarded by this UNDEFINED transition every
    // frame. Rendering scopes still store it so an offscreen-target switch can
    // resume the main target later in the same command buffer.
    VkImageMemoryBarrier2 toDepth{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toDepth.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    toDepth.srcAccessMask = VK_ACCESS_2_NONE;
    toDepth.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    toDepth.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    toDepth.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toDepth.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    toDepth.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDepth.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDepth.image = fDepthImage;
    toDepth.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    toDepth.subresourceRange.levelCount = 1;
    toDepth.subresourceRange.layerCount = 1;

    VkImageMemoryBarrier2 toMSAA{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toMSAA.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    toMSAA.srcAccessMask = VK_ACCESS_2_NONE;
    toMSAA.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toMSAA.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toMSAA.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toMSAA.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toMSAA.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toMSAA.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toMSAA.image = fMSAAColorImage;
    toMSAA.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toMSAA.subresourceRange.levelCount = 1;
    toMSAA.subresourceRange.layerCount = 1;

    VkImageMemoryBarrier2 barriers[3] = { toAttachment, toDepth, toMSAA };

    VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dependency.imageMemoryBarrierCount = (fMSAAColorImage != VK_NULL_HANDLE) ? 3
                                       : (fDepthImage != VK_NULL_HANDLE) ? 2 : 1;
    dependency.pImageMemoryBarriers = barriers;
    vkCmdPipelineBarrier2(frame.fCmd, &dependency);

    // BeginFrame made the main depth image ready for its first scope. Any later
    // scope this frame needs a depth dependency before loading it again.
    fMainDepthUsed = false;

    if (fGammaLUT && fGammaLUT->fImage != VK_NULL_HANDLE && fGammaSceneTarget &&
        fGammaSceneTarget->fImage != VK_NULL_HANDLE) {
        VkImageMemoryBarrier2 toScene{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        toScene.srcStageMask = fGammaSceneTarget->fShaderReadable
                             ? VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                             : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        toScene.srcAccessMask = fGammaSceneTarget->fShaderReadable
                              ? VK_ACCESS_2_SHADER_SAMPLED_READ_BIT
                              : VK_ACCESS_2_NONE;
        toScene.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        toScene.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        toScene.oldLayout = fGammaSceneTarget->fShaderReadable
                          ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                          : VK_IMAGE_LAYOUT_UNDEFINED;
        toScene.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toScene.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toScene.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toScene.image = fGammaSceneTarget->fImage;
        toScene.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        toScene.subresourceRange.levelCount = 1;
        toScene.subresourceRange.layerCount = 1;

        VkDependencyInfo sceneDependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        sceneDependency.imageMemoryBarrierCount = 1;
        sceneDependency.pImageMemoryBarriers = &toScene;
        vkCmdPipelineBarrier2(frame.fCmd, &sceneDependency);
        fGammaSceneTarget->fShaderReadable = false;
    }

    // Closed in EndFrame, which cannot run without fFrameOpen, so the two
    // always pair even when a frame is abandoned before it draws anything.
    BeginLabel(frame.fCmd, ST::format("Frame {}", fTimelineValue + 1));

    fFrameOpen = true;
    return true;
}

void plVulkanDevice::IBeginRendering(const hsColorRGBA* clearColor, float clearDepth)
{
    if (!fFrameOpen || fRenderingOpen)
        return;

    // A target put into shader-readable layout by IEndRendering has valid color
    // contents. The swapchain is also valid here: its first scope is always
    // opened by Clear, so a no-clear open can only be a resume after rendering
    // an offscreen target.
    const bool preserveContents = !clearColor &&
                                  (!fCurrentTarget || fCurrentTarget->fShaderReadable);

    // An offscreen target starts UNDEFINED (or shader-readable from last frame)
    // and has to become a color attachment before the scope opens.
    if (fCurrentTarget && fCurrentTarget->fImage != VK_NULL_HANDLE) {
        VkImageMemoryBarrier2 toAttachment{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        toAttachment.srcStageMask = fCurrentTarget->fShaderReadable
                                  ? VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                                  : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        toAttachment.srcAccessMask = fCurrentTarget->fShaderReadable
                                   ? VK_ACCESS_2_SHADER_SAMPLED_READ_BIT
                                   : VK_ACCESS_2_NONE;
        toAttachment.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        toAttachment.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        toAttachment.oldLayout = fCurrentTarget->fShaderReadable
                               ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                               : VK_IMAGE_LAYOUT_UNDEFINED;
        toAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        toAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toAttachment.image = fCurrentTarget->fImage;
        toAttachment.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        toAttachment.subresourceRange.levelCount = 1;
        toAttachment.subresourceRange.layerCount = 1;

        VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependency.imageMemoryBarrierCount = 1;
        dependency.pImageMemoryBarriers = &toAttachment;
        vkCmdPipelineBarrier2(fFrames[fFrameIndex].fCmd, &dependency);

        fCurrentTarget->fShaderReadable = false;
    }

    // Dynamic rendering does not perform attachment layout transitions. The
    // main depth image was transitioned in BeginFrame, but offscreen targets
    // start UNDEFINED and every resumed depth attachment needs an explicit
    // dependency on its previous reads and writes.
    const VkImage depthImage = fCurrentTarget ? fCurrentTarget->fDepthImage : fDepthImage;
    if (depthImage != VK_NULL_HANDLE) {
        plVulkanRenderTargetRef* depthOwner = IDepthImageOwner(fCurrentTarget);
        const VkImageLayout oldLayout = depthOwner ? depthOwner->fDepthLayout
                                                   : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        const bool needsBarrier = depthOwner || fMainDepthUsed;

        if (needsBarrier) {
            constexpr VkPipelineStageFlags2 kDepthStages =
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

            VkImageMemoryBarrier2 depthBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
            depthBarrier.srcStageMask = oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                      ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : kDepthStages;
            depthBarrier.srcAccessMask = oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                       ? VK_ACCESS_2_NONE
                                       : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                         VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            depthBarrier.dstStageMask = kDepthStages;
            depthBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                         VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            depthBarrier.oldLayout = oldLayout;
            depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            depthBarrier.image = depthImage;
            depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthBarrier.subresourceRange.levelCount = 1;
            depthBarrier.subresourceRange.layerCount = 1;

            VkDependencyInfo depthDependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            depthDependency.imageMemoryBarrierCount = 1;
            depthDependency.pImageMemoryBarriers = &depthBarrier;
            vkCmdPipelineBarrier2(fFrames[fFrameIndex].fCmd, &depthDependency);
        }

        if (depthOwner)
            depthOwner->fDepthLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        else
            fMainDepthUsed = true;
    }

    const VkExtent2D extent = CurrentExtent();

    VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    const bool multisampledMain = !fCurrentTarget && fSampleCount != VK_SAMPLE_COUNT_1_BIT;
    const VkImageView mainOutput = (fGammaLUT && fGammaLUT->fImage != VK_NULL_HANDLE &&
                                    fGammaSceneTarget &&
                                    fGammaSceneTarget->fImage != VK_NULL_HANDLE)
                                 ? fGammaSceneTarget->fImageView
                                 : fSwapViews[fImageIndex];
    color.imageView = fCurrentTarget ? fCurrentTarget->fImageView
                    : multisampledMain ? fMSAAColorView : mainOutput;
    color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color.loadOp = clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR
                              : (preserveContents ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                  : VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if (multisampledMain) {
        color.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        color.resolveImageView = mainOutput;
        color.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if (clearColor)
        color.clearValue.color = { { clearColor->r, clearColor->g, clearColor->b, clearColor->a } };

    const VkImageView depthView = fCurrentTarget ? fCurrentTarget->fDepthView : fDepthView;

    VkRenderingAttachmentInfo depth{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    depth.imageView = depthView;
    depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth.loadOp = preserveContents ? VK_ATTACHMENT_LOAD_OP_LOAD
                                    : VK_ATTACHMENT_LOAD_OP_CLEAR;
    // A render-target switch may resume this attachment later in the frame.
    depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth.clearValue.depthStencil.depth = clearDepth;

    VkRenderingInfo rendering{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering.renderArea.offset = { 0, 0 };
    rendering.renderArea.extent = extent;
    rendering.layerCount = 1;
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachments = &color;
    if (depthView != VK_NULL_HANDLE)
        rendering.pDepthAttachment = &depth;

    vkCmdBeginRendering(fFrames[fFrameIndex].fCmd, &rendering);

    fRenderingOpen = true;
    SetViewport();
}

void plVulkanDevice::Clear(const hsColorRGBA* color, const float* depth)
{
    if (!fFrameOpen || (!color && !depth))
        return;

    if (!fRenderingOpen) {
        // Fold the clear into the pass's load op when no rendering scope exists.
        // A reverse-Z shadow slave must supply its depth value at this point.
        IBeginRendering(color, depth ? *depth : 1.f);
        return;
    }

    VkClearAttachment clears[2]{};
    uint32_t clearCount = 0;

    if (color) {
        VkClearAttachment& clear = clears[clearCount++];
        clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear.colorAttachment = 0;
        clear.clearValue.color = { { color->r, color->g, color->b, color->a } };
    }

    const VkImageView depthView = fCurrentTarget ? fCurrentTarget->fDepthView : fDepthView;
    if (depth && depthView != VK_NULL_HANDLE) {
        VkClearAttachment& clear = clears[clearCount++];
        clear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        clear.clearValue.depthStencil.depth = *depth;
    }

    if (clearCount == 0)
        return;

    VkClearRect rect{};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = CurrentExtent();
    rect.layerCount = 1;

    vkCmdClearAttachments(fFrames[fFrameIndex].fCmd, clearCount, clears, 1, &rect);
}

bool plVulkanDevice::IPostprocessGamma()
{
    if (!fGammaSceneTarget || fGammaSceneTarget->fImage == VK_NULL_HANDLE ||
        !fGammaLUT || fGammaLUT->fImage == VK_NULL_HANDLE || fCurrentTarget)
        return false;

    VkCommandBuffer cmd = fFrames[fFrameIndex].fCmd;

    VkImageMemoryBarrier2 toRead{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toRead.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toRead.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toRead.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    toRead.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    toRead.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.image = fGammaSceneTarget->fImage;
    toRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toRead.subresourceRange.levelCount = 1;
    toRead.subresourceRange.layerCount = 1;

    VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dependency.imageMemoryBarrierCount = 1;
    dependency.pImageMemoryBarriers = &toRead;
    vkCmdPipelineBarrier2(cmd, &dependency);
    fGammaSceneTarget->fShaderReadable = true;

    VkRenderingAttachmentInfo color{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    color.imageView = fSwapViews[fImageIndex];
    color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo rendering{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering.renderArea.extent = fExtent;
    rendering.layerCount = 1;
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachments = &color;
    vkCmdBeginRendering(cmd, &rendering);
    fRenderingOpen = true;

    plVulkanPipelineKey key{};
    key.fPassKind = plVulkanPipelineKey::kPassGamma;
    key.fColorFormat = fSwapFormat;
    key.fDepthFormat = VK_FORMAT_UNDEFINED;
    key.fSampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkPipeline pipeline = GetPipelineState(key);

    plVulkanTextureRef scene;
    scene.fImageView = fGammaSceneTarget->fImageView;
    scene.fCookie = fGammaSceneTarget->fCookie;
    const plVulkanTextureRef* layers[] = { &scene, fGammaLUT };
    const uint8_t clamp[] = { hsGMatState::kClampTexture,
                              hsGMatState::kClampTexture };
    VkDescriptorSet textures = GetTextureDescriptorSet(layers, clamp, 2, nullptr, nullptr,
                                                       ST_LITERAL("the gamma resolve"));

    if (pipeline != VK_NULL_HANDLE && textures != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        SetViewport();
        vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
        vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
        vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);
        vkCmdSetDepthBias(cmd, 0.f, 0.f, 0.f);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                fPipelineLayout, kDescSetTextures, 1,
                                &textures, 0, nullptr);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    vkCmdEndRendering(cmd);
    fRenderingOpen = false;
    return pipeline != VK_NULL_HANDLE && textures != VK_NULL_HANDLE;
}

void plVulkanDevice::EndFrame()
{
    if (!fFrameOpen)
        return;
    fFrameOpen = false;

    plFrame& frame = fFrames[fFrameIndex];

    IEndRendering();

    if (fGammaLUT && fGammaLUT->fImage != VK_NULL_HANDLE &&
        fGammaSceneTarget && !IPostprocessGamma())
        hsStatusMessage("Vulkan: gamma postprocess failed");

    VkImageMemoryBarrier2 toPresent{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    toPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    toPresent.dstAccessMask = VK_ACCESS_2_NONE;
    toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresent.image = fSwapImages[fImageIndex];
    toPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toPresent.subresourceRange.levelCount = 1;
    toPresent.subresourceRange.layerCount = 1;

    VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dependency.imageMemoryBarrierCount = 1;
    dependency.pImageMemoryBarriers = &toPresent;
    vkCmdPipelineBarrier2(frame.fCmd, &dependency);

    EndLabel(frame.fCmd);

    VkResult result = vkEndCommandBuffer(frame.fCmd);
    if (result != VK_SUCCESS) {
        // Nothing will ever wait on the acquire semaphore now, and the image is
        // never going to be presented. Both only come back with the swapchain.
        fSwapchainDirty = true;
        fFrameHasAcquireWait = false;
        IFail("vkEndCommandBuffer", result);
        return;
    }

    VkSemaphoreSubmitInfo wait{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    wait.semaphore = fImageAvailable[fFrameIndex];
    wait.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo signals[2]{};
    signals[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signals[0].semaphore = fRenderFinished[fImageIndex];
    signals[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    frame.fTimelineValue = ++fTimelineValue;
    signals[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signals[1].semaphore = fTimeline;
    signals[1].value = frame.fTimelineValue;
    signals[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkCommandBufferSubmitInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    cmdInfo.commandBuffer = frame.fCmd;

    VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    submit.waitSemaphoreInfoCount = fFrameHasAcquireWait ? 1 : 0;
    submit.pWaitSemaphoreInfos = fFrameHasAcquireWait ? &wait : nullptr;
    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cmdInfo;
    submit.signalSemaphoreInfoCount = 2;
    submit.pSignalSemaphoreInfos = signals;

    result = vkQueueSubmit2(fQueue, 1, &submit, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        // The submission that was going to consume the acquire semaphore and
        // hand the image back never happened; start over from a new swapchain.
        fSwapchainDirty = true;
        fFrameHasAcquireWait = false;
        // Nothing is going to signal the value that was just claimed, and every
        // later wait would sit on a number the timeline can never reach.
        --fTimelineValue;
        frame.fTimelineValue = 0;
        IFail("vkQueueSubmit2", result);
        return;
    }
    fFrameHasAcquireWait = false;

    VkPresentInfoKHR present{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &fRenderFinished[fImageIndex];
    present.swapchainCount = 1;
    present.pSwapchains = &fSwapchain;
    present.pImageIndices = &fImageIndex;

    result = vkQueuePresentKHR(fQueue, &present);
    fHasPresentedFrame = result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // The present was rejected, so its wait on fRenderFinished[i] never ran
        // and that semaphore is left signalled. Reusing this swapchain would
        // double-signal it and deadlock the next submit; only a rebuild, which
        // replaces the whole ring, gets out of this cleanly.
        fSwapchainDirty = true;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        // A rejected present may not have consumed its wait semaphore. Retire
        // the whole swapchain rather than ever attempting to signal it again.
        fSwapchainDirty = true;
        IFail("vkQueuePresentKHR", result);
    }
    // SUBOPTIMAL needs no action here: the present did execute, so the semaphore
    // was consumed. The extent compare in BeginFrame rebuilds if it matters.

    // A skipped frame leaves this slot exactly as it found it. BeginFrame waits
    // for its timeline before the slot (and acquire semaphore) is reused.
    fFrameIndex = (fFrameIndex + 1) % kMaxFramesInFlight;
}

void plVulkanDevice::Shutdown()
{
    if (fDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(fDevice);

    delete fBlurTarget;
    fBlurTarget = nullptr;
    delete fGammaSceneTarget;
    fGammaSceneTarget = nullptr;
    delete fGammaLUT;
    fGammaLUT = nullptr;
    // Retire and drain rather than destroy: the ordering the drain uses --
    // views, then the swapchain, then the semaphores it was presenting with --
    // is the point, and it is the same ordering a rebuild needs.
    IRetireSwapchain();
    IDrainRetiredSwapchains(UINT64_MAX);

    if (fDevice != VK_NULL_HANDLE) {
        for (plFrame& frame : fFrames) {
            if (frame.fPool != VK_NULL_HANDLE)
                vkDestroyCommandPool(fDevice, frame.fPool, nullptr);
            frame = plFrame{};
        }

        ISavePipelineCache();

        // Descriptor sets die with their pools, so the cache only needs clearing.
        IClearTextureDescriptorSets();
        for (VkDescriptorSet& set : fUniformSet)
            set = VK_NULL_HANDLE;
        for (VkDescriptorPool pool : fDescriptorPools)
            vkDestroyDescriptorPool(fDevice, pool, nullptr);
        fDescriptorPools.clear();
        fDescriptorPool = VK_NULL_HANDLE;

        for (plScratchBlock& scratch : fScratch) {
            RetireBuffer(scratch.fBuffer);
            scratch = plScratchBlock{};
        }

        for (auto& entry : fPipelines) {
            if (entry.second != VK_NULL_HANDLE)
                vkDestroyPipeline(fDevice, entry.second, nullptr);
        }
        fPipelines.clear();

        if (fPipelineCache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(fDevice, fPipelineCache, nullptr);
            fPipelineCache = VK_NULL_HANDLE;
        }
        if (fPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(fDevice, fPipelineLayout, nullptr);
            fPipelineLayout = VK_NULL_HANDLE;
        }
        if (fUniformSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(fDevice, fUniformSetLayout, nullptr);
            fUniformSetLayout = VK_NULL_HANDLE;
        }
        if (fTextureSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(fDevice, fTextureSetLayout, nullptr);
            fTextureSetLayout = VK_NULL_HANDLE;
        }
        for (VkSampler& sampler : fSamplers) {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(fDevice, sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        }
        if (fVertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fVertexShader, nullptr);
            fVertexShader = VK_NULL_HANDLE;
        }
        if (fFragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fFragmentShader, nullptr);
            fFragmentShader = VK_NULL_HANDLE;
        }
        if (fPlateVertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fPlateVertexShader, nullptr);
            fPlateVertexShader = VK_NULL_HANDLE;
        }
        if (fPlateFragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fPlateFragmentShader, nullptr);
            fPlateFragmentShader = VK_NULL_HANDLE;
        }
        if (fTextVertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fTextVertexShader, nullptr);
            fTextVertexShader = VK_NULL_HANDLE;
        }
        if (fTextFragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(fDevice, fTextFragmentShader, nullptr);
            fTextFragmentShader = VK_NULL_HANDLE;
        }
        for (VkShaderModule* module : { &fShadowCasterVertexShader, &fShadowCasterFragmentShader,
                                        &fShadowApplyVertexShader, &fShadowApplyFragmentShader,
                                        &fAvatarVertexShader, &fAvatarFragmentShader,
                                        &fFullscreenVertexShader, &fBlurFragmentShader,
                                        &fGammaFragmentShader }) {
            if (*module != VK_NULL_HANDLE) {
                vkDestroyShaderModule(fDevice, *module, nullptr);
                *module = VK_NULL_HANDLE;
            }
        }
        for (auto& [id, module] : fProgrammableShaders) {
            if (module != VK_NULL_HANDLE)
                vkDestroyShaderModule(fDevice, module, nullptr);
        }
        fProgrammableShaders.clear();

        if (fUploadPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(fDevice, fUploadPool, nullptr);
            fUploadPool = VK_NULL_HANDLE;
        }

        // Last, because everything above retires into this queue and VMA
        // asserts if an allocation outlives its allocator. The queue is only
        // drained here once the GPU is idle, so the timeline no longer matters.
        IDrainRetired(UINT64_MAX);

        if (fAllocator) {
            vmaDestroyAllocator(fAllocator);
            fAllocator = nullptr;
        }

        if (fTimeline != VK_NULL_HANDLE) {
            vkDestroySemaphore(fDevice, fTimeline, nullptr);
            fTimeline = VK_NULL_HANDLE;
        }

        vkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }

    if (fInstance != VK_NULL_HANDLE) {
        if (fSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(fInstance, fSurface, nullptr);
            fSurface = VK_NULL_HANDLE;
        }
        if (fDebugMessenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(fInstance, fDebugMessenger, nullptr);
            fDebugMessenger = VK_NULL_HANDLE;
        }
        vkDestroyInstance(fInstance, nullptr);
        fInstance = VK_NULL_HANDLE;
    }
}
