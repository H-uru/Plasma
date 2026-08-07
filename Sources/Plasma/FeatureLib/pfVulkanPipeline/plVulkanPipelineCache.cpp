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

// Pipeline object construction and caching.
//
// Metal's equivalent is plMetalDevice::PipelineState / StartPipelineBuild
// (plMetalDevice.cpp:1079-1198). This builds synchronously; Metal's async build
// with a condition variable is an optimization, not a correctness requirement,
// and a VkPipelineCache on disk removes most of the reason for it.

#include "plVulkanDevice.h"
#include "plVulkanShaders.h"

#include "hsGMatState.h"
#include "plFileSystem.h"
#include "plPipeline.h"

#include "plDrawable/plGBufferGroup.h"
#include "plSurface/plShaderTable.h"

#include <string_theory/format>

#include <algorithm>
#include <vector>

namespace
{
    plFileName IPipelineCacheFile()
    {
        return plFileName::Join(plFileSystem::GetInitPath(), "vulkan_pipeline.cache");
    }

    /**
     * A pipeline's debug name.
     *
     * The key is what makes two pipelines different, so the name spells out the
     * parts of it a capture cannot recover from the handle: which pass it
     * belongs to and what its layers sample. Metal names its pipelines by pass
     * alone (plMetalPipelineState.cpp:283); the layer list is what tells one
     * material pipeline from the next.
     */
    ST::string IDescribePipelineKey(const plVulkanPipelineKey& key)
    {
        static const char* const kPassNames[] = {
            "material", "plate", "text", "text lines", "shadow caster", "shadow apply",
            "avatar", "avatar base", "blur", "gamma"
        };

        ST::string_stream name;
        name << "pipeline ";
        if (key.fPassKind < std::size(kPassNames))
            name << kPassNames[key.fPassKind];
        else
            name << "pass " << uint32_t(key.fPassKind);

        for (uint32_t i = 0; i < key.fNumLayers && i < kMaxLayers; i++) {
            switch (key.fPassTypes[i]) {
            case kPassTypeTexture:      name << " tex"; break;
            case kPassTypeCubicTexture: name << " cube"; break;
            case kPassTypeColor:        name << " color"; break;
            default:                    name << " ?"; break;
            }
        }

        if (key.fVertexShaderID || key.fFragmentShaderID) {
            name << ST::format(" shaders {}/{}", uint32_t(key.fVertexShaderID),
                               uint32_t(key.fFragmentShaderID));
        }

        return name.to_string();
    }

    /**
     * Framebuffer blend factors for a base-layer blend mode.
     *
     * Ported from plMetalRenderSpanPipelineState::ConfigureBlendMode
     * (plMetalPipelineState.cpp:166-257).
     */
    void IConfigureBlend(uint32_t blendMode, VkPipelineColorBlendAttachmentState& state)
    {
        state.blendEnable = VK_TRUE;
        state.alphaBlendOp = VK_BLEND_OP_ADD;
        state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        state.colorBlendOp = VK_BLEND_OP_ADD;
        state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        if (blendMode & hsGMatState::kBlendNoColor) {
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            return;
        }

        switch (blendMode & hsGMatState::kBlendMask) {
        // Detail is alpha with a mip chain that fades out; same blend.
        case hsGMatState::kBlendDetail:
        case hsGMatState::kBlendAlpha:
            if (blendMode & hsGMatState::kBlendInvertFinalAlpha) {
                state.srcColorBlendFactor = (blendMode & hsGMatState::kBlendAlphaPremultiplied)
                                          ? VK_BLEND_FACTOR_ONE
                                          : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            } else {
                state.srcColorBlendFactor = (blendMode & hsGMatState::kBlendAlphaPremultiplied)
                                          ? VK_BLEND_FACTOR_ONE
                                          : VK_BLEND_FACTOR_SRC_ALPHA;
                state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            }
            break;

        case hsGMatState::kBlendMult:
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            state.dstColorBlendFactor = (blendMode & hsGMatState::kBlendInvertFinalColor)
                                      ? VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR
                                      : VK_BLEND_FACTOR_SRC_COLOR;
            break;

        case hsGMatState::kBlendAdd:
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            break;

        case hsGMatState::kBlendMADD:
            state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            break;

        case hsGMatState::kBlendAddColorTimesAlpha:
            state.srcColorBlendFactor = (blendMode & hsGMatState::kBlendInvertFinalAlpha)
                                      ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                                      : VK_BLEND_FACTOR_SRC_ALPHA;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            break;

        case 0:
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;

        default:
            // kBlendSubtract and kBlendRevSubtract land here. Metal drops them
            // silently; at least say so.
            hsStatusMessageF("Vulkan: unhandled blend mode {x}",
                             blendMode & hsGMatState::kBlendMask);
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
        }
    }
}

bool plVulkanDevice::ICreateShaderModules()
{
    auto makeModule = [this](const plVulkanShaderBlob& blob, VkShaderModule* out,
                             const char* name) {
        VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        info.codeSize = blob.fSizeInBytes;
        info.pCode = blob.fWords;
        VkResult result = vkCreateShaderModule(fDevice, &info, nullptr, out);
        if (result == VK_SUCCESS)
            SetObjectName(VK_OBJECT_TYPE_SHADER_MODULE, *out, ST::string::from_utf8(name));
        return result;
    };

    VkResult result = makeModule(kFixedPipelineVertexShader, &fVertexShader, "vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (vertex)", result);

    result = makeModule(kFixedPipelineFragmentShader, &fFragmentShader, "fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (fragment)", result);

    result = makeModule(kPlateVertexShader, &fPlateVertexShader, "plate vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (plate vertex)", result);

    result = makeModule(kPlateFragmentShader, &fPlateFragmentShader, "plate fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (plate fragment)", result);

    result = makeModule(kTextFontVertexShader, &fTextVertexShader, "text vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (text vertex)", result);

    result = makeModule(kTextFontFragmentShader, &fTextFragmentShader, "text fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (text fragment)", result);

    result = makeModule(kShadowCasterVertexShader, &fShadowCasterVertexShader,
                        "shadow caster vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (shadow caster vertex)", result);

    result = makeModule(kShadowCasterFragmentShader, &fShadowCasterFragmentShader,
                        "shadow caster fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (shadow caster fragment)", result);

    result = makeModule(kShadowApplyVertexShader, &fShadowApplyVertexShader,
                        "shadow apply vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (shadow apply vertex)", result);

    result = makeModule(kShadowApplyFragmentShader, &fShadowApplyFragmentShader,
                        "shadow apply fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (shadow apply fragment)", result);

    result = makeModule(kAvatarVertexShader, &fAvatarVertexShader, "avatar vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (avatar vertex)", result);

    result = makeModule(kAvatarFragmentShader, &fAvatarFragmentShader, "avatar fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (avatar fragment)", result);

    result = makeModule(kFullscreenVertexShader, &fFullscreenVertexShader,
                        "fullscreen vertex shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (fullscreen vertex)", result);

    result = makeModule(kBlurFragmentShader, &fBlurFragmentShader, "blur fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (blur fragment)", result);

    result = makeModule(kGammaFragmentShader, &fGammaFragmentShader, "gamma fragment shader");
    if (result != VK_SUCCESS)
        return IFail("vkCreateShaderModule (gamma fragment)", result);

    // The plShader pairs. Everything absent from this table falls back to the
    // fixed-function combiner, which is why the map is sparse rather than an
    // array indexed by plShaderID.
    struct plProgrammablePair
    {
        plShaderID::ID            fID;
        const plVulkanShaderBlob* fBlob;
        const char*               fName;
    };

    static const plProgrammablePair kProgrammable[] = {
        { plShaderID::vs_GrassShader,  &kGrassVertexShader,        "grass vertex"        },
        { plShaderID::ps_GrassShader,  &kGrassFragmentShader,      "grass fragment"      },
        { plShaderID::vs_CompCosines,  &kCompCosinesVertexShader,  "compcosines vertex"  },
        // The engine calls the fragment half ps_MoreCosines; Metal maps it to
        // the same function, and so does this.
        { plShaderID::ps_MoreCosines,  &kCompCosinesFragmentShader, "compcosines fragment" },
        { plShaderID::vs_BiasNormals,  &kBiasNormalsVertexShader,  "biasnormals vertex"  },
        { plShaderID::ps_BiasNormals,  &kBiasNormalsFragmentShader, "biasnormals fragment" },

        { plShaderID::vs_WaveRip7,      &kWaveRipVertexShader,       "waverip vertex"       },
        { plShaderID::ps_WaveRip,       &kWaveRipFragmentShader,     "waverip fragment"     },
        { plShaderID::vs_WaveDec1Lay_7, &kWaveDec1LayVertexShader,   "wavedec1lay vertex"   },
        { plShaderID::ps_CbaseAbase,    &kWaveDec1LayFragmentShader, "cbaseabase fragment"  },
        { plShaderID::vs_WaveDecEnv_7,  &kWaveDecEnvVertexShader,    "wavedecenv vertex"    },
        { plShaderID::ps_WaveDecEnv,    &kWaveDecEnvFragmentShader,  "wavedecenv fragment"  },
        { plShaderID::vs_WaveFixedFin7, &kWaveFixedVertexShader,     "wavefixed vertex"     },
        { plShaderID::ps_WaveFixed,     &kWaveFixedFragmentShader,   "wavefixed fragment"   },
    };

    for (const auto& entry : kProgrammable) {
        VkShaderModule module = VK_NULL_HANDLE;
        result = makeModule(*entry.fBlob, &module, entry.fName);
        if (result != VK_SUCCESS)
            return IFail(entry.fName, result);

        fProgrammableShaders[uint32_t(entry.fID)] = module;
    }

    return true;
}

bool plVulkanDevice::ICreateSamplers()
{
    // hsGMatClampFlags is two bits: clamp U, clamp V. Four samplers cover it,
    // the same set plMetalDevice::fSamplerStates holds.
    for (uint32_t i = 0; i < kNumSamplers; i++) {
        const bool nearest = (i == kSamplerNearestObject);
        const bool clampToZero = (i == kSamplerClampToZeroObject);

        VkSamplerCreateInfo info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        info.magFilter = nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        info.minFilter = nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        info.mipmapMode = nearest ? VK_SAMPLER_MIPMAP_MODE_NEAREST
                                  : VK_SAMPLER_MIPMAP_MODE_LINEAR;
        if (clampToZero) {
            info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            // The default border is transparent black, which is what
            // Metal's address::clamp_to_zero means.
            info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        } else {
            info.addressModeU = (i & hsGMatState::kClampTextureU)
                              ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                              : VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.addressModeV = (i & hsGMatState::kClampTextureV)
                              ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                              : VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.maxLod = VK_LOD_CLAMP_NONE;

        // Text wants exact texels, so it opts out. Everything else takes what
        // the user asked for in graphics.ini, clamped to what the device allows.
        const float requested = fRequestedAnisotropy;
        if (!nearest && requested > 1.f && fMaxAnisotropy > 1.f) {
            info.anisotropyEnable = VK_TRUE;
            info.maxAnisotropy = std::min(requested, fMaxAnisotropy);
        }

        VkResult result = vkCreateSampler(fDevice, &info, nullptr, &fSamplers[i]);
        if (result != VK_SUCCESS)
            return IFail("vkCreateSampler", result);

        // Slots 0-3 are the clamp-flag combinations; the last two are the
        // special-purpose samplers the shader addresses by name.
        static const char* const kSamplerNames[kNumSamplers] = {
            "sampler repeat", "sampler clampU", "sampler clampV", "sampler clampUV",
            "sampler nearest", "sampler clampToZero"
        };
        SetObjectName(VK_OBJECT_TYPE_SAMPLER, fSamplers[i],
                      ST::string::from_utf8(kSamplerNames[i]));
    }

    return true;
}

bool plVulkanDevice::ICreateDescriptorLayouts()
{
    // Set 0: the per-draw uniforms, suballocated from the frame scratch buffer
    // and bound with dynamic offsets.
    {
        // Lighting runs in either stage depending on the per-pixel flag, so the
        // material and light bindings have to be visible to both.
        VkDescriptorSetLayoutBinding bindings[6]{};
        bindings[0].binding = kBindingVertexUniforms;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        bindings[1].binding = kBindingMaterial;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        // A storage buffer, not a uniform one: the scene light count is
        // unbounded and would blow a uniform buffer's range.
        bindings[2].binding = kBindingLights;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        // Only the shadow-apply vertex stage reads this, but every draw binds
        // the set, so non-shadow draws point it at a zeroed block.
        bindings[3].binding = kBindingShadowState;
        bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[3].descriptorCount = 1;
        bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // The programmable path's constant banks. Bound for every draw like the
        // rest; a fixed-function draw points them at a zeroed block.
        bindings[4].binding = kBindingVertexShaderConsts;
        bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[4].descriptorCount = 1;
        bindings[4].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        bindings[5].binding = kBindingFragmentShaderConsts;
        bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[5].descriptorCount = 1;
        bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.bindingCount = 6;
        info.pBindings = bindings;

        VkResult result = vkCreateDescriptorSetLayout(fDevice, &info, nullptr, &fUniformSetLayout);
        if (result != VK_SUCCESS)
            return IFail("vkCreateDescriptorSetLayout (uniforms)", result);

        SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, fUniformSetLayout,
                      ST_LITERAL("uniform set layout"));
    }

    // Set 1: the material's textures. The shader always declares eight of each
    // kind, so the slots a material does not use have to be legal to leave
    // empty -- hence PARTIALLY_BOUND.
    {
        VkDescriptorSetLayoutBinding bindings[4]{};
        bindings[0].binding = kBindingTextures2D;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        bindings[0].descriptorCount = kMaxLayers;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = kBindingTexturesCube;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        bindings[1].descriptorCount = kMaxLayers;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[2].binding = kBindingSamplers;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[2].descriptorCount = kNumSamplerSlots;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // Only the shadow-apply fragment stage reads this one.
        bindings[3].binding = kBindingShadowMap;
        bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        bindings[3].descriptorCount = 1;
        bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorBindingFlags bindingFlags[4] = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            0,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
        flagsInfo.bindingCount = 4;
        flagsInfo.pBindingFlags = bindingFlags;

        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.pNext = &flagsInfo;
        info.bindingCount = 4;
        info.pBindings = bindings;

        VkResult result = vkCreateDescriptorSetLayout(fDevice, &info, nullptr, &fTextureSetLayout);
        if (result != VK_SUCCESS)
            return IFail("vkCreateDescriptorSetLayout (textures)", result);

        SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, fTextureSetLayout,
                      ST_LITERAL("texture set layout"));
    }

    VkDescriptorSetLayout layouts[2] = { fUniformSetLayout, fTextureSetLayout };

    // Only the plate shader reads this, but a layout may declare ranges a given
    // shader ignores, and one shared layout keeps every pipeline compatible.
    VkPushConstantRange pushConstants{};
    pushConstants.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstants.offset = 0;
    pushConstants.size = sizeof(float);

    VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    layoutInfo.setLayoutCount = 2;
    layoutInfo.pSetLayouts = layouts;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstants;

    VkResult result = vkCreatePipelineLayout(fDevice, &layoutInfo, nullptr, &fPipelineLayout);
    if (result != VK_SUCCESS)
        return IFail("vkCreatePipelineLayout", result);

    SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, fPipelineLayout,
                  ST_LITERAL("pipeline layout"));

    return true;
}

bool plVulkanDevice::ICreatePipelineCache()
{
    std::vector<uint8_t> initial;

    plFileName cacheFile = IPipelineCacheFile();
    if (plFileInfo(cacheFile).Exists()) {
        FILE* fp = plFileSystem::Open(cacheFile, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            const long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if (size > 0) {
                initial.resize(size_t(size));
                if (fread(initial.data(), 1, initial.size(), fp) != initial.size())
                    initial.clear();
            }
            fclose(fp);
        }
    }

    VkPipelineCacheCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    info.initialDataSize = initial.size();
    info.pInitialData = initial.empty() ? nullptr : initial.data();

    // A cache from another driver or GPU is rejected by the implementation
    // rather than misused, so feeding it back blind is safe.
    VkResult result = vkCreatePipelineCache(fDevice, &info, nullptr, &fPipelineCache);
    if (result != VK_SUCCESS) {
        // Not fatal; it only costs compile time.
        hsStatusMessage("Vulkan: could not create the pipeline cache");
        fPipelineCache = VK_NULL_HANDLE;
    }

    return true;
}

void plVulkanDevice::ISavePipelineCache()
{
    if (fPipelineCache == VK_NULL_HANDLE)
        return;

    size_t size = 0;
    if (vkGetPipelineCacheData(fDevice, fPipelineCache, &size, nullptr) != VK_SUCCESS || size == 0)
        return;

    std::vector<uint8_t> data(size);
    if (vkGetPipelineCacheData(fDevice, fPipelineCache, &size, data.data()) != VK_SUCCESS)
        return;

    plFileSystem::CreateDir(plFileSystem::GetInitPath(), true);
    FILE* fp = plFileSystem::Open(IPipelineCacheFile(), "wb");
    if (!fp)
        return;

    fwrite(data.data(), 1, size, fp);
    fclose(fp);
}

VkPipeline plVulkanDevice::GetPipelineState(plVulkanPipelineKey key)
{
    key.Canonicalize();

    auto it = fPipelines.find(key);
    if (it != fPipelines.end())
        return it->second;

    VkPipeline pipeline = IBuildPipeline(key);
    fPipelines.emplace(key, pipeline);
    return pipeline;
}

VkPipeline plVulkanDevice::IBuildPipeline(const plVulkanPipelineKey& key)
{
    //
    // Specialization. The IDs match Metal's function constants one for one.
    //
    struct plSpecData
    {
        int32_t  fNumUVs;
        int32_t  fNumLayers;
        uint32_t fPassTypes[kMaxLayers];
        uint32_t fBlendModes[kMaxLayers];
        uint32_t fLayerFlags[kMaxLayers];
        int32_t  fNumWeights;
        uint32_t fPerPixelLighting;
        uint32_t fPointLightCast;
        int32_t  fShadowAlphaSrc;
    } spec{};

    spec.fNumUVs = key.fNumUVs;
    spec.fNumLayers = key.fNumLayers;
    spec.fNumWeights = key.fNumWeights;
    spec.fPerPixelLighting = key.fUsePerPixelLighting;
    spec.fPointLightCast = key.fPointLightCast;
    spec.fShadowAlphaSrc = key.fShadowAlphaSrc;
    for (uint32_t i = 0; i < kMaxLayers; i++) {
        spec.fPassTypes[i] = key.fPassTypes[i];
        spec.fBlendModes[i] = key.fBlendFlags[i];
        spec.fLayerFlags[i] = key.fMiscFlags[i];
    }

    std::vector<VkSpecializationMapEntry> entries;
    entries.push_back({ kSpecNumUVs, offsetof(plSpecData, fNumUVs), sizeof(int32_t) });
    entries.push_back({ kSpecNumLayers, offsetof(plSpecData, fNumLayers), sizeof(int32_t) });
    for (uint32_t i = 0; i < kMaxLayers; i++) {
        entries.push_back({ kSpecPassTypes + i,
                            uint32_t(offsetof(plSpecData, fPassTypes) + i * sizeof(uint32_t)),
                            sizeof(uint32_t) });
        entries.push_back({ kSpecBlendModes + i,
                            uint32_t(offsetof(plSpecData, fBlendModes) + i * sizeof(uint32_t)),
                            sizeof(uint32_t) });
        entries.push_back({ kSpecLayerFlags + i,
                            uint32_t(offsetof(plSpecData, fLayerFlags) + i * sizeof(uint32_t)),
                            sizeof(uint32_t) });
    }
    entries.push_back({ kSpecNumWeights, offsetof(plSpecData, fNumWeights), sizeof(int32_t) });
    entries.push_back({ kSpecPerPixelLighting, offsetof(plSpecData, fPerPixelLighting),
                        sizeof(uint32_t) });
    entries.push_back({ kSpecPointLightCast, offsetof(plSpecData, fPointLightCast),
                        sizeof(uint32_t) });
    entries.push_back({ kSpecShadowAlphaSrc, offsetof(plSpecData, fShadowAlphaSrc),
                        sizeof(int32_t) });

    VkSpecializationInfo specInfo{};
    specInfo.mapEntryCount = uint32_t(entries.size());
    specInfo.pMapEntries = entries.data();
    specInfo.dataSize = sizeof(spec);
    specInfo.pData = &spec;

    // Plates have their own shader pair and take no specialization: they are a
    // textured quad with an identity projection, not a material.
    const bool isPlate = key.fPassKind == plVulkanPipelineKey::kPassPlate;
    const bool isText = key.fPassKind == plVulkanPipelineKey::kPassText ||
                        key.fPassKind == plVulkanPipelineKey::kPassTextLines;
    const bool isShadowCaster = key.fPassKind == plVulkanPipelineKey::kPassShadowCaster;
    const bool isShadowApply = key.fPassKind == plVulkanPipelineKey::kPassShadowApply;
    const bool isAvatar = key.fPassKind == plVulkanPipelineKey::kPassAvatar ||
                          key.fPassKind == plVulkanPipelineKey::kPassAvatarBase;
    const bool isBlur = key.fPassKind == plVulkanPipelineKey::kPassBlur;
    const bool isGamma = key.fPassKind == plVulkanPipelineKey::kPassGamma;
    const bool isFullscreen = isBlur || isGamma;

    // A programmable pass names its own pair; everything else is one of ours.
    const bool isProgrammable = key.fVertexShaderID != 0 &&
                                GetShaderModules(key.fVertexShaderID, key.fFragmentShaderID,
                                                 nullptr, nullptr);

    VkShaderModule vertexModule = fVertexShader;
    VkShaderModule fragmentModule = fFragmentShader;
    if (isProgrammable) {
        GetShaderModules(key.fVertexShaderID, key.fFragmentShaderID,
                         &vertexModule, &fragmentModule);
    } else if (isText) {
        vertexModule = fTextVertexShader;
        fragmentModule = fTextFragmentShader;
    } else if (isPlate) {
        vertexModule = fPlateVertexShader;
        fragmentModule = fPlateFragmentShader;
    } else if (isShadowCaster) {
        vertexModule = fShadowCasterVertexShader;
        fragmentModule = fShadowCasterFragmentShader;
    } else if (isShadowApply) {
        vertexModule = fShadowApplyVertexShader;
        fragmentModule = fShadowApplyFragmentShader;
    } else if (isAvatar) {
        vertexModule = fAvatarVertexShader;
        fragmentModule = fAvatarFragmentShader;
    } else if (isFullscreen) {
        vertexModule = fFullscreenVertexShader;
        fragmentModule = isGamma ? fGammaFragmentShader : fBlurFragmentShader;
    }

    // Plates and text have their own shader pairs and take no specialization:
    // they are textured quads, not materials. Shadow casters take none either --
    // the map only ever records one distance per fragment.
    // A programmable pair has no specialization constants: it is a hand-written
    // shader, not a permutation of the combiner.
    const bool specialized = !isPlate && !isText && !isShadowCaster && !isAvatar &&
                             !isFullscreen &&
                             !isProgrammable;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertexModule;
    stages[0].pName = "main";
    stages[0].pSpecializationInfo = specialized ? &specInfo : nullptr;

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragmentModule;
    stages[1].pName = "main";
    stages[1].pSpecializationInfo = specialized ? &specInfo : nullptr;

    //
    // Vertex layout. Offsets follow ConfigureVertexDescriptor
    // (plMetalPipelineState.cpp:125-164), which is not the order
    // plGBufferGroup::ICalcVertexSize declares.
    //
    const uint32_t skinWeightOffset = 12 + (key.fHasSkinIndices ? 4 : 0);
    const uint32_t normOffset = skinWeightOffset + 4 * key.fNumWeights;
    const uint32_t colorOffset = normOffset + 12;
    // Four bytes of specular sit between the diffuse color and the UVs. Nothing
    // reads them, and Metal does not declare an attribute for them either.
    const uint32_t baseUvOffset = colorOffset + 8;
    const uint32_t stride = baseUvOffset + 12 * key.fNumUVs;

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = stride;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributes;

    if (isFullscreen) {
        // gl_VertexIndex generates one oversized triangle; no vertex buffer.
    } else if (isText) {
        // plTextFont::plFontVertex: hsPoint3 position, packed ARGB, hsPoint3 UV.
        binding.stride = sizeof(float) * 3 + sizeof(uint32_t) + sizeof(float) * 3;
        attributes.push_back({ kVtxAttrPosition, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        attributes.push_back({ kVtxAttrColor, 0, VK_FORMAT_R8G8B8A8_UNORM, sizeof(float) * 3 });
        attributes.push_back({ kVtxAttrTexcoord, 0, VK_FORMAT_R32G32B32_SFLOAT,
                               sizeof(float) * 3 + sizeof(uint32_t) });
    } else if (isPlate) {
        // vec2 position then vec3 texcoord, interleaved. See
        // plVulkanPlateManager::ICreateGeometry.
        binding.stride = sizeof(float) * 5;
        attributes.push_back({ kVtxAttrPosition, 0, VK_FORMAT_R32G32_SFLOAT, 0 });
        attributes.push_back({ kVtxAttrTexcoord, 0, VK_FORMAT_R32G32B32_SFLOAT,
                               sizeof(float) * 2 });
    } else if (isAvatar) {
        // plAvatarTexVert: clip-space position then UV, both vec2.
        binding.stride = sizeof(float) * 4;
        attributes.push_back({ kVtxAttrPosition, 0, VK_FORMAT_R32G32_SFLOAT, 0 });
        attributes.push_back({ kVtxAttrTexcoord, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2 });
    } else {
    attributes.push_back({ kVtxAttrPosition, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
    attributes.push_back({ kVtxAttrNormal, 0, VK_FORMAT_R32G32B32_SFLOAT, normOffset });
    attributes.push_back({ kVtxAttrColor, 0, VK_FORMAT_R8G8B8A8_UNORM, colorOffset });

    // The shader declares the weight and all eight UV inputs unconditionally --
    // GLSL cannot gate an input declaration on a specialization constant the way
    // MSL gates a [[stage_in]] member. Every declared location therefore needs
    // an attribute or the pipeline is invalid. Slots the layout does not
    // actually carry are aimed at offset 0; the shader never reads them,
    // because sampleLocation bounds-checks against numUVs.
    attributes.push_back({ kVtxAttrWeight, 0, VK_FORMAT_R32_SFLOAT,
                           key.fNumWeights > 0 ? skinWeightOffset : 0 });
    for (uint32_t i = 0; i < kMaxLayers; i++) {
        attributes.push_back({ kVtxAttrTexcoord + i, 0, VK_FORMAT_R32G32B32_SFLOAT,
                               i < key.fNumUVs ? baseUvOffset + 12 * i : 0 });
    }
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInput.vertexBindingDescriptionCount = isFullscreen ? 0 : 1;
    vertexInput.pVertexBindingDescriptions = isFullscreen ? nullptr : &binding;
    vertexInput.vertexAttributeDescriptionCount = uint32_t(attributes.size());
    vertexInput.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    if (isAvatar) {
        // IDrawClothingQuad submits Metal's P1, P2, P0, P3 ordering as four
        // vertices.  It forms the complete quad only as a triangle strip; a
        // triangle list consumes the first three and silently drops P3,
        // leaving most of the generated avatar texture undefined.
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    } else if (key.fPassKind == plVulkanPipelineKey::kPassTextLines) {
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    } else {
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    VkPipelineViewportStateCreateInfo viewport{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewport.viewportCount = 1;
    viewport.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    raster.polygonMode = key.fWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_NONE;  // dynamic
    // A negative viewport height reverses framebuffer winding. Plasma's other
    // backends use clockwise as the front face, so retain that convention here.
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.depthBiasEnable = VK_TRUE;     // amount is dynamic
    raster.lineWidth = 1.f;

    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisample.rasterizationSamples = VkSampleCountFlagBits(key.fSampleCount);

    VkPipelineDepthStencilStateCreateInfo depthStencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    // All three are dynamic; the values here only have to be legal.
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState blendAttachment{};
    if (isShadowCaster) {
        // The map keeps the nearest caster: RGB accumulates color one plus the
        // destination scaled by the incoming light-space depth. Alpha itself is
        // replaced by that depth, matching Metal's default alpha blend factors.
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    } else if (isBlur) {
        // The convolution shader produces the complete RGBA result. In
        // particular, alpha carries the caster's light-space depth and must not
        // preserve the render target's cleared alpha as ordinary materials do.
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    } else if (key.fPassKind == plVulkanPipelineKey::kPassAvatarBase) {
        // The base element seeds the composite, alpha included: the target is
        // never cleared, so nothing else in the pass initializes its alpha and a
        // pooled target would hand the avatar whatever the last outfit left
        // there. Metal builds this state with blending off outright
        // (plMetalPipeline.cpp:2979).
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    } else if (key.fPassKind == plVulkanPipelineKey::kPassAvatar) {
        // Each element over the last, straight alpha. Alpha itself is left
        // alone; the base layer above is what put it there.
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    } else if (isShadowApply) {
        // Darkening, not drawing: the fragment's color is subtracted from what
        // is already there (plMetalPipelineState.cpp:341-347).
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    } else {
        IConfigureBlend(key.fBlendFlags[0], blendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blendAttachment;

    const VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_CULL_MODE,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
        VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
        VK_DYNAMIC_STATE_DEPTH_BIAS,
    };

    VkPipelineDynamicStateCreateInfo dynamic{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic.dynamicStateCount = uint32_t(std::size(dynamicStates));
    dynamic.pDynamicStates = dynamicStates;

    const VkFormat colorFormat = VkFormat(key.fColorFormat);
    VkPipelineRenderingCreateInfo rendering{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachmentFormats = &colorFormat;
    rendering.depthAttachmentFormat = VkFormat(key.fDepthFormat);

    VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    info.pNext = &rendering;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &vertexInput;
    info.pInputAssemblyState = &inputAssembly;
    info.pViewportState = &viewport;
    info.pRasterizationState = &raster;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depthStencil;
    info.pColorBlendState = &colorBlend;
    info.pDynamicState = &dynamic;
    info.layout = fPipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines(fDevice, fPipelineCache, 1, &info,
                                                nullptr, &pipeline);
    if (result != VK_SUCCESS) {
        IFail("vkCreateGraphicsPipelines", result);
        return VK_NULL_HANDLE;
    }

    SetObjectName(VK_OBJECT_TYPE_PIPELINE, pipeline, IDescribePipelineKey(key));

    return pipeline;
}
