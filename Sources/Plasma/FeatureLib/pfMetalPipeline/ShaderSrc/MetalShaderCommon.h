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

#include <metal_stdlib>
#include <simd/simd.h>

#define GMAT_STATE_ENUM_START(name)       enum name {
#define GMAT_STATE_ENUM_VALUE(name, val)    name = val,
#define GMAT_STATE_ENUM_END(name)         };

#include "hsGMatStateEnums.h"

#undef GMAT_STATE_ENUM_START
#undef GMAT_STATE_ENUM_VALUE
#undef GMAT_STATE_ENUM_END

#include "ShaderTypes.h"

using namespace metal;

struct plTier1Bumpmap
{
    char2 dTangentIndex [[ id(dTangentIndexID) ]];
    texture2d<half> bumpTexture [[ id(textureID) ]];
    sampler bumpTextureSampler  [[ id(samplerID) ]];
    float3 scale [[ id(dScaleID) ]];
};
    
struct plTier1Layer
{
    texture2d<half> texture [[ id(0) ]];
    texture3d<half> texture3D [[ id(1) ]];
    sampler         sampler [[ id(2) ]];
};
    
#if __METAL_VERSION__ >= 300
typedef plMetalBumpmap ShaderBumpMapType;
#else
typedef plTier1Bumpmap ShaderBumpMapType;
#endif
    
#if __METAL_VERSION__ >= 300
typedef plMetalLayer ShaderLayerType;
#else
typedef plTier1Layer ShaderLayerType;
#endif

struct FragmentShaderArguments
{
    device ShaderLayerType *layers [[ buffer(FragmentShaderLayers) ]];
    const constant plMetalFragmentShaderArgumentBuffer*     bufferedUniforms   [[ buffer(FragmentShaderArgumentUniforms)   ]];
    half4 sampleLayer(const size_t index, const half4 vertexColor, const uint8_t passType, float3 sampleCoord) const;
};

#define MAX_BLEND_PASSES 8

constant const uint32_t miscFlags1 [[ function_constant(FunctionConstantLayerFlags + 0)    ]];
constant const uint32_t miscFlags2 [[ function_constant(FunctionConstantLayerFlags + 1)    ]];
constant const uint32_t miscFlags3 [[ function_constant(FunctionConstantLayerFlags + 2)    ]];
constant const uint32_t miscFlags4 [[ function_constant(FunctionConstantLayerFlags + 3)    ]];
constant const uint32_t miscFlags5 [[ function_constant(FunctionConstantLayerFlags + 4)    ]];
constant const uint32_t miscFlags6 [[ function_constant(FunctionConstantLayerFlags + 5)    ]];
constant const uint32_t miscFlags7 [[ function_constant(FunctionConstantLayerFlags + 6)    ]];
constant const uint32_t miscFlags8 [[ function_constant(FunctionConstantLayerFlags + 7)    ]];

constant const uint32_t miscFlags[MAX_BLEND_PASSES] = { miscFlags1, miscFlags2, miscFlags3, miscFlags4, miscFlags5, miscFlags6, miscFlags7, miscFlags8};
