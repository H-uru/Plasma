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

#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <simd/simd.h>

#ifndef __METAL_VERSION__
#include <type_traits>

typedef _Float16 half;
typedef __attribute__((__ext_vector_type__(2))) half half2;
typedef __attribute__((__ext_vector_type__(3))) half half3;
typedef __attribute__((__ext_vector_type__(4))) half half4;
#endif

enum plMetalShaderArgument
{
    /// Material State
    VertexShaderArgumentFixedFunctionUniforms           = 2,
    /// Uniform table for Plasma dynamic shaders
    VertexShaderArgumentMaterialShaderUniforms          = 3,
    /// Light Table
    VertexShaderArgumentLights                          = 4,
    /// Material properties for vertex lighting
    VertexShaderArgumentMaterialLighting                = 5,
    /// Blend matrix for GPU side animation blending
    VertexShaderArgumentBlendMatrix1                    = 6,
    /// Describes the state of a shadow caster for shadow cast shader
    VertexShaderArgumentShadowState                     = 9,
    
    /// Texture is a legacy argument for the simpler plate shader
    FragmentShaderArgumentTexture                       = 1,
    /// Fragment uniforms
    FragmentShaderArgumentShadowCastUniforms            = 4,
    /// Legacy argument buffer
    FragmentShaderArgumentUniforms                      = 5,
    /// Layer index of alpha for shadow fragment shader
    FragmentShaderArgumentShadowCastAlphaSrc            = 8,
    /// Light Table
    FragmentShaderArgumentLights                        = 10,
    /// Material properties for vertex lighting
    FragmentShaderArgumentMaterialLighting              = 11
};

enum plMetalVertexAttribute
{
    /// position of a vertex
    VertexAttributePosition                             = 0,
    /// UV of a vertex. Reserves IDs 1-8.
    VertexAttributeTexcoord                             = 1,
    /// Normal attribute of a vertex
    VertexAttributeNormal                               = 9,
    /// Color attribute of a vertex
    VertexAttributeColor                                = 10,
    /// Animation weight of a vertex
    VertexAttributeWeights                              = 11,
};

/// Arguments to the shader compiler to control output
enum plMetalFunctionConstant
{
    /// Numbrer of UVs in the FVF vertex layout.
    FunctionConstantNumUVs                              = 0,
    /// Number of layers the shader will need to render
    FunctionConstantNumLayers                           = 1,
    /// Source type of the material texture. Metal needs to know if the texture will
    /// be cubic or 2D in advance.  Eight values reserved.
    FunctionConstantSources                             = 2,
    /// Blend modes for each of the layers.
    FunctionConstantBlendModes                          = 10,
    /// Render flags for each layer. Eight values reserved.
    FunctionConstantLayerFlags                          = 18,
    /// Numbrer of weights in the FVF vertex layout.
    FunctionConstantNumWeights                          = 26,
    /// Per pixel lighting enable flag
    FunctionConstantPerPixelLighting                    = 27,
};

enum plMetalLayerPassType: uint8_t
{
    PassTypeTexture = 1,
    PassTypeCubicTexture = 2,
    PassTypeColor = 3
};

struct plMetalFragmentShaderArgumentBuffer
{
    __fp16 alphaThreshold;
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<plMetalFragmentShaderArgumentBuffer>, "plMetalFragmentShaderArgumentBuffer must be a trivial type!");
#endif

struct plMetalShadowCastFragmentShaderArgumentBuffer
{
    bool pointLightCast;
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<plMetalShadowCastFragmentShaderArgumentBuffer>, "plMetalShadowCastFragmentShaderArgumentBuffer must be a trivial type!");
#endif

enum plMetalFragmentShaderTextures
{
    FragmentShaderArgumentAttributeTextures = 0,
    FragmentShaderArgumentAttributeCubicTextures = 8,
    FragmentShaderArgumentAttributeUniforms = 32
};

struct plMetalShaderLightSource
{
    simd::float4 position;
    half4 ambient;
    half4 diffuse;
    half4 specular;
    simd::float3 direction;
    simd::float4 spotProps; // (falloff, theta, phi)
    __fp16 range;
    __fp16 constAtten;
    __fp16 linAtten;
    __fp16 quadAtten;
    __fp16 scale;
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<plMetalShaderLightSource>, "plMetalShaderLightSource must be a trivial type!");
#endif

struct UVOutDescriptor
{
    uint32_t UVWSrc;
    matrix_float4x4 transform;
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<UVOutDescriptor>, "UVOutDescriptor must be a trivial type!");
#endif

struct plMaterialLightingDescriptor
{
    half4 globalAmb;
    half3 ambientCol;
    uint8_t ambientSrc;
    half4 diffuseCol;
    uint8_t diffuseSrc;
    half3 emissiveCol;
    uint8_t emissiveSrc;
    half3 specularCol;
    uint8_t specularSrc;
    
    bool invertAlpha;
};

struct VertexUniforms
{
    // transformation
    matrix_float4x4 projectionMatrix;
    matrix_float4x4 localToWorldMatrix;
    matrix_float4x4 cameraToWorldMatrix;
    matrix_float4x4 worldToCameraMatrix;

    uint8_t fogExponential;
    simd::float2 fogValues;
    half3 fogColor;

    UVOutDescriptor uvTransforms[8];
#ifdef __METAL_VERSION__
    float3 sampleLocation(size_t index, thread float3 *texCoords, const float4 cameraSpaceNormal, const float4 camPosition) constant;
    half4 calcFog(float4 camPosition) constant;
#endif
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<VertexUniforms>, "VertexUniforms must be a trivial type!");
#endif

#define kMetalMaxLightCount 32

struct plMetalLights
{
    uint8_t count;
    plMetalShaderLightSource lampSources[kMetalMaxLightCount];
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<plMetalLights>, "plMetalLights must be a trivial type!");
#endif

struct plShadowState
{
    simd::float3 lightPosition;
    simd::float3 lightDirection;
    bool directional;
    float power;
    half opacity;
};
#ifndef __METAL_VERSION__
static_assert(std::is_trivial_v<plShadowState>, "plShadowState must be a trivial type!");
#endif

#endif /* ShaderTypes_h */

