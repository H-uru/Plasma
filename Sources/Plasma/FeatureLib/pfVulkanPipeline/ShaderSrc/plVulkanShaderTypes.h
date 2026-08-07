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

/**
 * The contract between the Vulkan backend and its shaders.
 *
 * This header is compiled twice: once by C++, once by glslangValidator. Keeping
 * one definition is the only way the two sides cannot drift, and it is the same
 * trick the Metal backend uses (see ShaderSrc/ShaderTypes.h there).
 *
 * Uniform blocks that use these structs must be declared `layout(scalar)`,
 * which requires GL_EXT_scalar_block_layout in the shader and the
 * `scalarBlockLayout` feature on the device (core in Vulkan 1.2). That gives
 * the blocks C layout rules, so the structs below map across byte for byte
 * with no std140 padding surprises. The static_asserts at the bottom hold C++
 * to it; a mismatch is a compile error rather than a rendering mystery.
 *
 * Where Metal uses `half` / `half3` / `half4` / `__fp16` this uses `float`. We
 * author both sides, nothing forces 16-bit, and it drops a dependency on
 * shaderFloat16 plus a class of layout bugs (`half3` is 8 bytes, not 6).
 */

#ifndef _plVulkanShaderTypes_h_
#define _plVulkanShaderTypes_h_

#ifdef __cplusplus
#   include <cstdint>
#   include <cstddef>

    struct plVkVec2 { float x, y; };
    struct plVkVec3 { float x, y, z; };
    struct plVkVec4 { float x, y, z, w; };

    /**
     * Column-major, laid out exactly like a GLSL mat4.
     *
     * hsMatrix44 is row-major, so copying its fMap straight in transposes it,
     * which is precisely what the shader wants: `m * vec4(pos, 1)` then
     * evaluates the row-major matrix. plMetalDevice.h:68 does the same memcpy.
     */
    struct plVkMat4 { float m[16]; };

#   define PLVK_UINT  uint32_t
#   define PLVK_FLOAT float
#   define PLVK_VEC2  plVkVec2
#   define PLVK_VEC3  plVkVec3
#   define PLVK_VEC4  plVkVec4
#   define PLVK_MAT4  plVkMat4
#else
#   define PLVK_UINT  uint
#   define PLVK_FLOAT float
#   define PLVK_VEC2  vec2
#   define PLVK_VEC3  vec3
#   define PLVK_VEC4  vec4
#   define PLVK_MAT4  mat4

    // Pull the material state flags straight out of the engine so the shader
    // and hsGMatState can never disagree. GLSL has no enums, so each value
    // becomes a constant. C++ callers include hsGMatState.h instead.
#   define GMAT_STATE_ENUM_START(name)
#   define GMAT_STATE_ENUM_VALUE(name, val) const uint name = uint(val);
#   define GMAT_STATE_ENUM_END(name)
#   include "hsGMatStateEnums.h"
#   undef GMAT_STATE_ENUM_START
#   undef GMAT_STATE_ENUM_VALUE
#   undef GMAT_STATE_ENUM_END

    // plUVWSrcModifiers, from plSurface/plLayerInterface.h:79-85. That header is
    // C++ only, so the values are restated for the shader. The low nibble of
    // UVWSrc selects a UV channel; these select generated coordinates instead.
    //
    // GLSL only: in C++ these names belong to plLayerInterface's enum, and
    // defining them as macros here would rewrite that declaration.
#   define kUVWPassThru    0x00000000
#   define kUVWIdxMask     0x0000ffff
#   define kUVWNormal      0x00010000
#   define kUVWPosition    0x00020000
#   define kUVWReflect     0x00030000
#endif

//
// Vertex attribute locations.
//
// These mirror plMetalVertexAttribute (ShaderTypes.h:88-100). Texcoord reserves
// 1..8. Note there is deliberately no specular attribute: the vertex stride
// reserves four bytes for it but nothing reads it, matching
// plMetalRenderSpanPipelineState::ConfigureVertexDescriptor.
//
#define kVtxAttrPosition    0
#define kVtxAttrTexcoord    1
#define kVtxAttrNormal      9
#define kVtxAttrColor       10
#define kVtxAttrWeight      11

//
// Specialization constant IDs. One-to-one with plMetalFunctionConstant
// (ShaderTypes.h:103-120) so the two backends stay comparable.
//
#define kSpecNumUVs             0
#define kSpecNumLayers          1
#define kSpecPassTypes          2   // reserves 2..9
#define kSpecBlendModes         10  // reserves 10..17
#define kSpecLayerFlags         18  // reserves 18..25
#define kSpecNumWeights         26
#define kSpecPerPixelLighting   27

// Shadow permutation. Metal passes both of these as buffers; here they are
// specialization constants because between them they have six values, and both
// decide control flow the compiler can fold away entirely.
#define kSpecPointLightCast     28
#define kSpecShadowAlphaSrc     29

// Layer pass types, matching plMetalLayerPassType (ShaderTypes.h:122-127).
#define kPassTypeTexture        1
#define kPassTypeCubicTexture   2
#define kPassTypeColor          3

#define kMaxLayers  8

//
// Descriptor set and binding assignments.
//
// Set 0 holds per-draw uniforms, suballocated from the frame's scratch ring and
// bound with dynamic offsets. Set 1 holds the material's textures and samplers
// and is content-hash cached.
//
#define kDescSetUniforms        0
#define kBindingVertexUniforms  0
#define kBindingMaterial        1
#define kBindingLights          2
#define kBindingShadowState     3

// The plShader programmable path: a flat array of float4 registers, which is
// exactly what a D3D vertex-shader constant bank was. Vertex and fragment get
// their own, because a plShader pair sets them independently.
#define kBindingVertexShaderConsts   4
#define kBindingFragmentShaderConsts 5

/**
 * Register count each constant bank holds.
 *
 * D3D vs.1.1 had 96 vertex constants and ps.1.1 had 8; the widest shader Plasma
 * ships uses well under that. The block is allocated at this size regardless,
 * because it is bound by dynamic offset from the frame's scratch ring.
 */
#define kMaxShaderConsts        96

#define kDescSetTextures        1
#define kBindingTextures2D      0   // array of kMaxLayers
#define kBindingTexturesCube    1   // array of kMaxLayers
#define kBindingSamplers        2   // array of kNumSamplerSlots
#define kBindingShadowMap       3

//
// Samplers.
//
// Two counts, because the pool and the descriptor array are different things.
//
// The device holds kNumSamplers distinct sampler objects: the four
// hsGMatClampFlags combinations, which a layer's clamp flags index directly,
// plus a nearest-filtered one for text, which has to stay crisp at its authored
// size.
//
// The descriptor array is one slot per layer, so slot N is already the sampler
// layer N wants and the shader never has to know about clamp flags -- the same
// thing plMetalMaterialShaderRef does by writing sampler states into its
// argument buffer. The trailing slot is text's.
//
#define kNumSamplers            6
#define kSamplerNearestObject   4

/** Reads transparent black outside the image, rather than smearing its edge. */
#define kSamplerClampToZeroObject 5

// Slots kMaxLayers and above are not layers: text wants nearest filtering,
// shadow maps and decals are clamped whatever the material around them does, and
// the avatar composite wants transparent black outside each element.
#define kNumSamplerSlots        (kMaxLayers + 3)
#define kSamplerNearest         kMaxLayers
#define kSamplerClampBoth       (kMaxLayers + 1)
#define kSamplerClampToZero     (kMaxLayers + 2)

/** Clamp in both U and V, which is the sampler object a shadow map wants. */
#define kSamplerClampBothObject 3

//
// Lighting.
//
// Every registered light is uploaded once per frame; a draw then names the
// subset that reaches it by index. That is Metal's arrangement, and it keeps the
// per-draw payload to a couple of hundred bytes instead of the whole scene.
//
#define kMaxActiveLights 32

/** Maximum scene-light array covered by the dynamic storage-buffer binding. */
#define kMaxSceneLights 1024

struct plShaderLightSource
{
    /** w == 0 marks a directional light, which does not attenuate. */
    PLVK_VEC4  position;

    PLVK_VEC4  ambient;
    PLVK_VEC4  diffuse;
    PLVK_VEC4  specular;

    PLVK_VEC3  direction;

    /** (falloff, inner cutoff, outer cutoff); x > 0 marks a spot light. */
    PLVK_VEC4  spotProps;

    PLVK_FLOAT range;
    PLVK_FLOAT constAtten;
    PLVK_FLOAT linAtten;
    PLVK_FLOAT quadAtten;
};

/**
 * The light a shadow is cast from.
 *
 * Ported from plShadowState (ShaderTypes.h:241-248). DX cast shadows by
 * rewriting the light table mid-render, turning lights on and off to control
 * each shadow's strength. Metal replaced that with this struct, so shadow
 * strength is a property of the shadow rather than a mutation of scene lighting,
 * and the light table is written once per frame. That is what makes the
 * per-frame light buffer in plVulkanLights.cpp viable here too.
 */
struct plShadowState
{
    PLVK_VEC4  lightPosition;

    /** Only meaningful when directional. */
    PLVK_VEC4  lightDirection;

    PLVK_UINT  directional;

    /** Falloff exponent applied to N-dot-L. */
    PLVK_FLOAT power;

    PLVK_FLOAT opacity;
    PLVK_FLOAT pad0;
};

/** One light's contribution to a draw: which light, and how far faded in. */
struct plShaderActiveLight
{
    PLVK_UINT  index;
    PLVK_FLOAT scale;
};

//
// Per-layer UV generation. Mirrors UVOutDescriptor (ShaderTypes.h:181-188).
//
// UVWSrc is the layer's plUVWSrcModifiers value: the low nibble selects a UV
// channel, and kUVWNormal / kUVWPosition / kUVWReflect select generated
// coordinates instead.
//
struct UVOutDescriptor
{
    PLVK_UINT UVWSrc;
    PLVK_MAT4 transform;
};

//
// Everything the vertex stage needs that is not a vertex attribute. Mirrors
// VertexUniforms (ShaderTypes.h:212-236).
//
// At ~824 bytes this is far past any push-constant budget, so it lives in the
// per-frame scratch uniform buffer.
//
struct VertexUniforms
{
    PLVK_MAT4  projectionMatrix;
    PLVK_MAT4  localToWorldMatrix;
    PLVK_MAT4  cameraToWorldMatrix;
    PLVK_MAT4  worldToCameraMatrix;

    PLVK_UINT  fogExponential;
    PLVK_VEC2  fogValues;
    PLVK_VEC3  fogColor;

    UVOutDescriptor uvTransforms[kMaxLayers];

    /**
     * Second matrix for hardware one-bone skinning.
     *
     * Only read when numWeights is 1. Appended after uvTransforms so every
     * offset above stays where it was.
     */
    PLVK_MAT4  blendMatrix1;
};

//
// Material colors and where each one comes from. Mirrors
// plMaterialLightingDescriptor (ShaderTypes.h:190-210).
//
// The *Src fields are 0/1 mix selectors, not enums: 0 takes the vertex color,
// 1 takes the material color. plMetalPipeline::ICalcLighting fills them from
// the span's kLiteMask lighting model.
//
struct plMaterialLightingDescriptor
{
    PLVK_VEC4  globalAmb;

    PLVK_VEC3  ambientCol;
    PLVK_UINT  ambientSrc;

    PLVK_VEC4  diffuseCol;
    PLVK_UINT  diffuseSrc;

    PLVK_VEC3  emissiveCol;
    PLVK_UINT  emissiveSrc;

    PLVK_VEC3  specularCol;
    PLVK_UINT  specularSrc;

    /** Set from hsGMatState::kBlendInvertVtxAlpha. */
    PLVK_UINT  invertAlpha;

    /**
     * Fragments with alpha below this are discarded.
     *
     * Metal keeps this in a separate argument buffer
     * (plMetalFragmentShaderArgumentBuffer); folding it in here saves a binding.
     * plMetalMaterialShaderRef.cpp:520-534 computes it: 64/255 for
     * kBlendAlphaTestHigh, else 1/255, else 0.
     */
    PLVK_FLOAT alphaThreshold;

    /** How many of activeLights are in use. Zero means unlit. */
    PLVK_UINT  lightCount;

    plShaderActiveLight activeLights[kMaxActiveLights];
};

#ifdef __cplusplus

static_assert(sizeof(plVkMat4) == 64, "plVkMat4 must be a bare 4x4 of floats");
static_assert(sizeof(UVOutDescriptor) == 68, "UVOutDescriptor must be scalar-packed");
static_assert(offsetof(UVOutDescriptor, transform) == 4, "UVOutDescriptor transform offset");

static_assert(offsetof(VertexUniforms, localToWorldMatrix) == 64, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, cameraToWorldMatrix) == 128, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, worldToCameraMatrix) == 192, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, fogExponential) == 256, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, fogValues) == 260, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, fogColor) == 268, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, uvTransforms) == 280, "VertexUniforms layout");
static_assert(offsetof(VertexUniforms, blendMatrix1) == 280 + 68 * kMaxLayers,
              "VertexUniforms layout");
static_assert(sizeof(VertexUniforms) == 280 + 68 * kMaxLayers + 64, "VertexUniforms size");

static_assert(offsetof(plMaterialLightingDescriptor, diffuseCol) == 32,
              "plMaterialLightingDescriptor layout");
static_assert(offsetof(plMaterialLightingDescriptor, invertAlpha) == 84,
              "plMaterialLightingDescriptor layout");
static_assert(offsetof(plMaterialLightingDescriptor, alphaThreshold) == 88,
              "plMaterialLightingDescriptor layout");
static_assert(offsetof(plMaterialLightingDescriptor, lightCount) == 92,
              "plMaterialLightingDescriptor layout");
static_assert(sizeof(plShaderActiveLight) == 8, "plShaderActiveLight must be scalar-packed");
static_assert(sizeof(plMaterialLightingDescriptor) == 96 + 8 * kMaxActiveLights,
              "plMaterialLightingDescriptor size");
static_assert(sizeof(plShaderLightSource) == 108, "plShaderLightSource must be scalar-packed");
static_assert(sizeof(plShadowState) == 48, "plShadowState must be scalar-packed");
static_assert(offsetof(plShadowState, directional) == 32, "plShadowState layout");
static_assert(offsetof(plShadowState, opacity) == 40, "plShadowState layout");

#endif // __cplusplus

#endif // _plVulkanShaderTypes_h_
