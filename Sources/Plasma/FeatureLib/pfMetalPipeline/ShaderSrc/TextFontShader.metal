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
using namespace metal;
// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"


using namespace metal;

typedef struct
{
    packed_float3    position;
    uchar4     color;
    packed_float3    UV;
} Vertex;

typedef struct
{
    float4 position [[position]];
    float3 texCoord;
    float4 normal;
    half4  color;
} ColorInOut;

vertex ColorInOut textFontVertexShader(constant Vertex *in [[ buffer(0) ]],
                                       constant matrix_float4x4 & transform [[ buffer(1) ]],
                                       uint v_id [[vertex_id]])
{
    ColorInOut out;

    Vertex vert = in[v_id];
    float4 position = float4(vert.position, 1.0);
    out.position =  (transform * position);
    out.texCoord = vert.UV;
    out.normal = float4(0.0, 0.0, 1.0, 0.0);
    out.color = half4(vert.color.b, vert.color.g, vert.color.r, vert.color.a) / 255.0f;

    return out;
}

fragment half4 textFontFragmentShader(ColorInOut in [[stage_in]],
                               texture2d<half> colorMap     [[ texture(0) ]])
{
    constexpr sampler colorSampler(mip_filter::nearest,
                                   mag_filter::nearest,
                                   min_filter::nearest);

    half4 colorSample = colorMap.sample(colorSampler, in.texCoord.xy);
    colorSample *= in.color;

    return colorSample;
}
