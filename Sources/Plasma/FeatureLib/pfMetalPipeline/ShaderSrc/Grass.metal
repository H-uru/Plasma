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

#include "ShaderVertex.h"

// ignoring the int and pi constants here and using whats built in
// but reserving space for them in the buffer
typedef struct
{
    matrix_float4x4 Local2NDC;
    float4 intConstants;
    float4 time;
    float4 piConstants;
    float4 sinConstants;
    float4 waveDistortX;
    float4 waveDistortY;
    float4 waveDistortZ;
    float4 waveDirX;
    float4 waveDirY;
    float4 waveSpeed;
} vs_GrassUniforms;
    
typedef struct
{
    float4 position [[position]];
    float4 color;
    float4 texCoord;
} vs_GrassInOut;

vertex vs_GrassInOut vs_GrassShader(Vertex in                                       [[stage_in]],
                                           constant vs_GrassUniforms & uniforms     [[ buffer(VertexShaderArgumentMaterialShaderUniforms) ]])
{
    vs_GrassInOut out;

    float4 r0 = (in.position.x * uniforms.waveDirX) + (in.position.y * uniforms.waveDirX);

    r0 += (uniforms.time.x * uniforms.waveSpeed); // scale by speed and add to X,Y input
    r0 = fract(r0);

    r0 = (r0 - 0.5f) * M_PI_F * 2.f;

    float4 pow2 = r0 * r0;
    float4 pow3 = pow2 * r0;
    float4 pow5 = pow2 * pow3;
    float4 pow7 = pow2 * pow5;
    float4 pow9 = pow2 * pow7;

    r0 += pow3 * uniforms.sinConstants.x;
    r0 += pow5 * uniforms.sinConstants.y;
    r0 += pow7 * uniforms.sinConstants.z;
    r0 += pow9 * uniforms.sinConstants.w;

    float3 offset = float3(
                             dot(r0, uniforms.waveDistortX),
                             dot(r0, uniforms.waveDistortY),
                             dot(r0, uniforms.waveDistortZ)
                             );

    offset *= (2.f * (1.f - in.texCoord1.y)); // mult by Y tex coord. So the waves only affect the top verts

    float4 position = float4(in.position.xyz + offset, 1);
    out.position = position * uniforms.Local2NDC;

    out.color = float4(in.color.r, in.color.g, in.color.b, in.color.a) / 255.f;
    out.texCoord = float4(in.texCoord1, 0.f);

    return out;
}

fragment half4 ps_GrassShader(vs_GrassInOut in      [[stage_in]],
                             texture2d<half> t0     [[ texture(0) ]])
{
    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);

    half4 out = t0.sample(colorSampler, in.texCoord.xy);
    out *= half4(in.color);
    if (out.a <= 0.1h)
        discard_fragment();
    return out;
}
