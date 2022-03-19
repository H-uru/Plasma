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
typedef _Float16 half;
typedef __attribute__((__ext_vector_type__(2))) half half2;
typedef __attribute__((__ext_vector_type__(3))) half half3;
typedef __attribute__((__ext_vector_type__(4))) half half4;
#endif

enum plMetalShaderArgumentIndex
{
    //Texture is a legacy argument for the simpler plate shader
    Texture    = 1,
    BufferIndexState      = 2,
    BufferIndexUniforms      = 3,
    BufferIndexFragArgBuffer  = 5,
    BufferIndexShadowCastFragArgBuffer  = 4
};

enum plMetalVertexShaderUniform
{
    VertexAttributePosition  = 0,
    VertexAttributeTexcoord  = 1,
    VertexAttributeNormal  = 9,
    VertexAttributeUVCount = 10,
    VertexAttributeColor = 11
};

enum plMetalFragmentShaderUniform
{
    FragmentShaderArgumentShadowAlphaSrc = 8,
    FragmentShaderArgumentPiggybackLayers = 9,
    FragmentShaderArgumentNumPiggybackLayers = 10,
    FragmentShaderOverrideLayer = 11
};

enum plMetalFunctionConstant
{
    FunctionConstantNumUVs    = 0,
    FunctionConstantNumLayers    = 1,
    FunctionConstantSources    = 2,
    FunctionConstantBlendModes    = 10,
    FunctionConstantLayerFlags    = 18
};

enum plMetalLayerPassType: uint8_t
{
    PassTypeTexture = 1,
    PassTypeCubicTexture = 2,
    PassTypeColor = 3
};

struct plFragmentShaderLayer {
    uint8_t sampleType;
};

struct plMetalFragmentShaderArgumentBuffer {
    uint8_t layerCount;
    __fp16 alphaThreshold;
    plFragmentShaderLayer layers[8];
};

struct plMetalShadowCastFragmentShaderArgumentBuffer {
    bool pointLightCast;
};

enum plMetalFragmentShaderTextures {
    FragmentShaderArgumentAttributeTextures = 0,
    FragmentShaderArgumentAttributeCubicTextures = 8,
    FragmentShaderArgumentAttributeColors = 16,
    FragmentShaderArgumentAttributeUniforms = 32
};

struct plMetalShaderLightSource {
    simd::float4 position;
    half4 ambient;
    half4 diffuse;
    half4 specular;
    simd::float3 direction;
    simd::float4 spotProps; // (falloff, theta, phi)
    __fp16 constAtten;
    __fp16 linAtten;
    __fp16 quadAtten;
    __fp16 scale;
};

typedef struct
{
    uint32_t UVWSrc;
    uint32_t flags;
    matrix_float4x4 transform;
} UVOutDescriptor;

typedef struct
{
    //transformation
    matrix_float4x4 projectionMatrix;
    matrix_float4x4 localToWorldMatrix;
    matrix_float4x4 worldToLocalMatrix;
    matrix_float4x4 cameraToWorldMatrix;
    matrix_float4x4 worldToCameraMatrix;
    
    //lighting
    half4 globalAmb;
    half4 ambientCol;
    uint8_t ambientSrc;
    half4 diffuseCol;
    uint8_t diffuseSrc;
    half4 emissiveCol;
    uint8_t emissiveSrc;
    half4 specularCol;
    uint8_t specularSrc;
    bool invVtxAlpha;
    
    uint8_t fogExponential;
    simd::float2 fogValues;
    half3 fogColor;
    
    plMetalShaderLightSource lampSources[8];
    
    uint8_t numUVSrcs;
    UVOutDescriptor uvTransforms[8];
} VertexUniforms;

#endif /* ShaderTypes_h */

