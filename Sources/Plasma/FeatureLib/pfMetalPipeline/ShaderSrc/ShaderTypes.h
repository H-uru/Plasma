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
    FunctionConstantNumLayers    = 1
};

enum plMetalLayerPassType
{
    PassTypeTexture = 1,
    PassTypeCubicTexture = 2,
    PassTypeColor = 3
};

struct plFragmentShaderLayer {
    ushort passType;
    uint uvIndex;
    uint32_t blendMode;
    uint32_t miscFlags;
    short sampleType;
};

struct plMetalFragmentShaderArgumentBuffer {
    ushort layerCount;
    float alphaThreshold;
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
    simd::float4 ambient;
    simd::float4 diffuse;
    simd::float4 specular;
    simd::float3 direction;
    simd::float4 spotProps; // (falloff, theta, phi)
    float constAtten;
    float linAtten;
    float quadAtten;
    float scale;
};

typedef struct
{
    uint UVWSrc;
    uint flags;
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
    simd::float4 globalAmb;
    simd::float4 ambientCol;
    float ambientSrc;
    simd::float4 diffuseCol;
    float diffuseSrc;
    simd::float4 emissiveCol;
    float emissiveSrc;
    simd::float4 specularCol;
    float specularSrc;
    bool invVtxAlpha;
    
    uint fogExponential;
    simd::float2 fogValues;
    simd::float3 fogColor;
    
    plMetalShaderLightSource lampSources[8];
    
    uint numUVSrcs;
    UVOutDescriptor uvTransforms[8];
} VertexUniforms;

#endif /* ShaderTypes_h */

