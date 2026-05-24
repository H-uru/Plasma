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

// Used in Ahnonay on the edge of the sphere

#include <metal_stdlib>
using namespace metal;

#include "ShaderVertex.h"

typedef struct
{
    matrix_float4x4 WorldToNDC;
    float4 Frequency;
    float4 Phase;
    float4 Amplitude;
    float4 DirectionX;
    float4 DirectionY;
    float4 Scrunch; // UNUSED
    float4 SinConsts;
    float4 CosConsts;
    float4 PiConsts;
    float4 NumericConsts;
    float2x4 Tex0;
    float4 Tex1_Row0;
    float4 Tex1_Row1;
    float3x4 L2W;
    float4 Lengths;
    float4 WaterLevel;
    float4 DepthFalloff;
    float4 MinAtten;
    float4 Bias; // Only using one slot
    float4 MatColor;
    float4 CameraPos; // Only used by DecalEnv
    float4 EnvAdjust; // Only used by DecalEnv
    float4 FogSet;
    float4 QADirX;
    float4 QADirY;

    float4 DirXW; // Only used by DecalEnv
    float4 DirYW; // Only used by DecalEnv
    float4 WK; // Only used by DecalEnv
    float4 DirXSqKW; // Only used by DecalEnv
    float4 DirXDirYKW; // Only used by DecalEnv
    float4 DirYSqKW; // Only used by DecalEnv
} vs_WaveDev1Lay_7Uniforms;
    
typedef struct
{
    float4 position [[position]];
    half4 c0;
    float4 texCoord0;
    half4 fog;
} vs_WaveDev1Lay_7InOut;

vertex vs_WaveDev1Lay_7InOut vs_WaveDec1Lay_7(Vertex in                         [[stage_in]],
                               constant vs_WaveDev1Lay_7Uniforms & uniforms     [[ buffer(VertexShaderArgumentMaterialShaderUniforms) ]])
{
    vs_WaveDev1Lay_7InOut out;
    // Store our input position in world space in r6
    float4 worldPosition = float4(float4(in.position, 1.f) * uniforms.L2W, 1.f);

    // Input diffuse v5 color is:
    // v5.r = overall transparency
    // v5.g = illumination
    // v5.b = overall wave scaling
    //
    // v5.a is:
    // v5.w = 1/(2.f * edge length)
    // So per wave filtering is:
    // min(max( (waveLen * v5.wwww) - 1), 0), 1.f);
    // So a wave effect starts dying out when the wave is 4 times the sampling frequency,
    // and is completely filtered at 2 times sampling frequency.

    // We'd like to make this autocalculated based on the depth of the water.
    // The frequency filtering (v5.w) still needs to be calculated offline, because
    // it's dependent on edge length, but the first 3 filterings can be calculated
    // based on this vertex.
    // Basically, we want the transparency, reflection strength, and wave scaling
    // to go to zero as the water depth goes to zero. Linear falloffs are as good
    // a place to start as any.
    //
    // depth = waterlevel - r6.z        => depth in feet (may be negative)
    // depthNorm = depth / depthFalloff => zero at watertable, one at depthFalloff beneath
    // atten = minAtten + depthNorm * (maxAtten - minAtten);
    // These are all vector ops.
    // This provides separate ramp ups for each of the channels (they reach full unfiltered
    // values at different depths), but doesn't provide separate controls for where they
    // go to zero (they all go to zero at zero depth). For that we need an offset. An offset
    // in feet (depth) is probably the most intuitive. So that changes the first calculation
    // of depth to:
    // depth = waterlevel - r6.z + offset
    //      = (waterlevel + offset) - r6.z
    // And since we only need offsets for 3 channels, we can make the waterlevel constant
    // waterlevel[chan] = watertableheight + offset[chan],
    // with waterlevel.w = watertableheight.
    //
    // So:
    //  c22 = waterlevel + offset
    //  c23 = (maxAtten - minAtten) / depthFalloff
    //  c24 = minAtten.
    // And in particular:
    //  c22.w = waterlevel
    //  c23.w = 1.f;
    //  c24.w = 0;
    // So r4.w is the depth of this vertex in feet.

    // Dot our position with our direction vectors.
    float4 phases = uniforms.DirectionX * worldPosition.xxxx;
    phases += uniforms.DirectionY * worldPosition.yyyy;

    //
    //    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
    phases = (phases * uniforms.Frequency) + uniforms.Phase;
    
    float4 cosPhases;
    float4 sinPhases = fast::sincos(phases, cosPhases);

    // Calc our depth based filtering here into r4 (because we don't use it again
    // after here, and we need our filtering shortly).
    float4 depth = uniforms.WaterLevel - worldPosition.zzzz;
    depth *= uniforms.DepthFalloff;
    depth += uniforms.MinAtten;
    // Clamp .xyz to range [0..1]
    depth = clamp(depth, 0, 1);

    // Calc our filter (see above).
    float4 filter = in.color.wwww * uniforms.Lengths;
    filter = max(filter, uniforms.NumericConsts.xxxx);
    filter = min(filter, uniforms.NumericConsts.zzzz);

    //mov    r2, r1;
    // r2 == sinDist
    // r1 == cosDist
    //    sinDist *= filter;
    sinPhases *= filter;
    //    sinDist *= kAmplitude.xyzw
    sinPhases *= uniforms.Amplitude;
    // r5 is now T = sum(Ai * sin())
    // METAL NOTE: from here on, r5 is sinDist
    //    height = dp4(sinDist, kOne);
    //    accumPos.z += height; (but accumPos.z is currently 0).
    float4 accumPos = float4(0);
    accumPos.x = dot(sinPhases, uniforms.NumericConsts.zzzz);
    accumPos.y = accumPos.x * depth.z;
    accumPos.z = accumPos.y + uniforms.WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z); // CLAMP
    // r8.x == wave height relative to 0
    // r8.y == dampened wave relative to 0
    // r8.z == dampened wave height in world space
    // r6.z == wave height clamped to never go beneath ground level
    //
    //    cosDist *= kAmplitude.xyzw; // Combine?
    //METAL NOTE: cosDist is now r7
    cosPhases *= uniforms.Amplitude;
    //    cosDist *= filter;
    cosPhases *= filter;
    // Pos = (in.x + S, in.y + R, r6.z)
    // S = sum(k Dir.x A cos())
    // R = sum(k Dir.y A cos())
    // c30 = k Dir.x A
    // c31 = k Dir.y A
    //    S = sum(cosDist * c30);
    worldPosition.xy += float2(
                              dot(cosPhases, uniforms.QADirX),
                              dot(cosPhases, uniforms.QADirY)
                              );

    // Bias our vert up a bit to compensate for precision errors.
    // In particular, our filter coefficients are coming in as
    // interpolated bytes, so there's bound to be a lot of slop
    // from that. We've got a free slot in c25.x, so we'll use that.
    // A better implementation would be to bias and scale our screen
    // vert, effectively pushing the vert toward the camera without
    // actually moving it, but this is easier and might work just
    // as well.
    worldPosition.z += uniforms.Bias.x;

    //
    // // Transform position to screen
    //
    //
    out.position = worldPosition * uniforms.WorldToNDC;
    out.fog = (out.position.w + uniforms.FogSet.x) * uniforms.FogSet.y;

    // Output color is vertex green
    // Output alpha is vertex red (vtx alpha is used for wave filtering)
    // Whole thing modulated by material color/opacity.

    out.c0 = half4(in.color.yyyz) * half4(uniforms.MatColor);

    // Usual texture transform
    out.texCoord0 = float4(float4(in.texCoord1, 1.0) * uniforms.Tex0, 0.f, 0.f);

    return out;
}

fragment half4 ps_CbaseAbase(vs_WaveDev1Lay_7InOut in   [[stage_in]],
                             texture2d<half> texture    [[ texture(0) ]])
{

    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    return texture.sample(colorSampler, in.texCoord0.xy) * in.c0;
}
