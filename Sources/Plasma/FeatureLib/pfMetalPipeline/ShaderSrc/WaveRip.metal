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

typedef struct {
    matrix_float4x4 WorldToNDC;
    float4 FogSet;
    float4 Frequency;
    float4 Phase;
    float4 Amplitude;
    float4 DirectionX;
    float4 DirectionY;
    float4 QADirX;
    float4 QADirY;
    float4 Scrunch;
    float4 SinConsts;
    float4 CosConsts;
    float4 PiConsts;
    float4 NumericConsts;
    float4 CameraPos;
    float4 WindRot;
    float4 Tex0_Row0;
    float4 Tex0_Row1;
    float4 Tex0_Row2;
    float4 Tex1_Row0;
    float4 Tex1_Row1;
    float4 Tex1_Row2;
    float4 LocalToWorld;
    float4 L2WRow0;
    float4 L2WRow1;
    float4 L2WRow2;
    float4 Lengths;
    float4 WaterLevel;
    float4 DepthFalloff;
    float4 MinAtten;
    float4 TexConsts;
    float4 LifeConsts;
    float4 RampBias;
} vs_WaveRip7Uniforms;

typedef struct {
    float4 position [[position]];
    float4 c1;
    float4 texCoord0;
    float fog;
} waveRipInOut;

vertex waveRipInOut vs_WaveRip7(Vertex in [[stage_in]],
                               constant vs_WaveRip7Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]]) {
    waveRipInOut out;
    
    // Store our input position in world space in r6
    float4 worldPosition = float4(0);
    worldPosition.x = dot(float4(in.position, 1.0), uniforms.L2WRow0);
    worldPosition.y = dot(float4(in.position, 1.0), uniforms.L2WRow1);
    worldPosition.z = dot(float4(in.position, 1.0), uniforms.L2WRow2);
    // Fill out our w (m4x3 doesn't touch w).
    worldPosition.w = 1.0;
    
    //

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
    float4 distance = uniforms.DirectionX * worldPosition.xxxx;
    distance += uniforms.DirectionY * worldPosition.yyyy;
    
    //
    //    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
    distance = (distance * uniforms.Frequency) + uniforms.Phase;
    
    //    // Now we need dist mod'd into range [-Pi..Pi]
    //    dist *= rcp(kTwoPi);
    distance += uniforms.PiConsts.zzzz;
    distance *= 1.0f / uniforms.PiConsts.wwww;
    
    //    dist = frac(dist);
    distance = fract(distance);
    //    dist *= kTwoPi;
    distance *= uniforms.PiConsts.wwww;
    //    dist += -kPi;
    distance -= uniforms.PiConsts.zzzz;
    
    //
    //    sincos(dist, sinDist, cosDist);
    // sin = r0 + r0^3 * vSin.y + r0^5 * vSin.z
    // cos = 1 + r0^2 * vCos.y + r0^4 * vCos.z
    
    float4 pow2 = distance * distance; // r0^2
    float4 pow3 = pow2 * distance; // r0^3 - probably stall
    float4 pow4 = pow2 * pow2; // r0^4
    float4 pow5 = pow2 * pow3; // r0^5
    float4 pow7 = pow2 * pow5; // r0^7
    
    //r1
    float4 cosDist = 1 + pow2 * uniforms.CosConsts.y + pow4 * uniforms.CosConsts.z;
    //r2
    float4 sinDist = distance + pow3 * uniforms.SinConsts.y + pow5 * uniforms.SinConsts.z;
    
    cosDist = ((pow3 * pow3) * uniforms.CosConsts.w) + cosDist;
    sinDist = (pow7 * uniforms.SinConsts.w) + sinDist;
    
    // Calc our depth based filtering here into r4 (because we don't use it again
    // after here, and we need our filtering shortly).
    float4 depth = uniforms.WaterLevel - worldPosition.zzzz;
    depth *= uniforms.DepthFalloff;
    depth += uniforms.MinAtten;
    // Clamp .xyz to range [0..1]
    depth = clamp(depth, 0, 1);
    
    // Calc our filter (see above).
    float4 inColor = float4(in.color) / 255.0f;
    float4 filter = inColor.wwww * uniforms.Lengths;
    filter = max(filter, uniforms.NumericConsts.xxxx);
    filter = min(filter, uniforms.NumericConsts.zzzz);
    
    //mov    r2, r1;
    // r2 == sinDist
    // r1 == cosDist
    //    sinDist *= filter;
    sinDist *= filter;
    //    sinDist *= kAmplitude.xyzw
    sinDist *= uniforms.Amplitude;
    // r5 is now T = sum(Ai * sin())
    // METAL NOTE: from here on, r5 is sinDist
    //    height = dp4(sinDist, kOne);
    //    accumPos.z += height; (but accumPos.z is currently 0).
    float4 accumPos = float4(0);
    accumPos.x = dot(sinDist, uniforms.NumericConsts.zzzz);
    accumPos.y = accumPos.x * depth.z;
    accumPos.z = accumPos.y + uniforms.WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z); // CLAMP
    // r8.x == wave height relative to 0
    // r8.y == dampened wave relative to 0
    // r8.z == dampened wave height in world space
    // r6.z == wave height clamped to never go beneath ground level
    //
    //    cosDist *= filter;
    cosDist *= filter;
    // Pos = (in.x + S, in.y + R, r6.z)
    // S = sum(k Dir.x A cos())
    // R = sum(k Dir.y A cos())
    // c30 = k Dir.x A
    // c31 = k Dir.y A
    //    S = sum(cosDist * c30);
    worldPosition.xy += float2(
                              dot(cosDist, uniforms.QADirX),
                              dot(cosDist, uniforms.QADirY)
                              );
    
    // Bias our vert up a bit to compensate for precision errors.
    // In particular, our filter coefficients are coming in as
    // interpolated bytes, so there's bound to be a lot of slop
    // from that. We've got a free slot in c25.x, so we'll use that.
    // A better implementation would be to bias and scale our screen
    // vert, effectively pushing the vert toward the camera without
    // actually moving it, but this is easier and might work just
    // as well.
    worldPosition.z += uniforms.RampBias.z;
    
    //
    // // Transform position to screen
    //
    //
    out.position = worldPosition * uniforms.WorldToNDC;
    out.fog = (out.position.w + uniforms.FogSet.x) * uniforms.FogSet.y;
    
    // Now onto texture coordinate generation.
    //
    // First is the usual texture transform
    out.texCoord0 = float4(
                           dot(float4(in.texCoord1, 1.0), uniforms.Tex0_Row0),
                           dot(float4(in.texCoord1, 1.0), uniforms.Tex0_Row1),
                           uniforms.NumericConsts.zz
                           );
    
    // Dyna Stuff
    // Constants
    // c33 = fC1U, fC2U, fC1V, fC2V
    // c34 = fInitAtten, t, life, 1.f / (life-decay)
    // c35 = ramp, 1.f / ramp, BIAS (positive is up), FREE
    //
    // Vertex Info
    // v7.z = fBirth (because we don't use it for anything else).
    //
    // Initialize r1.zw to 0,1
    
    float4 r1 = float4(0,0,0,1);
    // Calc r1.x = age, r1.y = atten
    // age = t - birth.
    r1.x = uniforms.LifeConsts.y - in.position.z;
    // atten = clamp0_1(age / ramp) * clamp0_1((life-age) / (life-decay));
    // first clamp0_1(age/ramp)
    r1.y = r1.x - uniforms.RampBias.y;
    r1.y = min(r1.y, 1.0f);
    // now clamp0_1((life-age) / (life-decay));
    r1.z = uniforms.LifeConsts.z - in.position.x;
    r1.z *= uniforms.LifeConsts.w;
    r1.z = clamp(r1.z, 0.0f, 1.0f);
    r1.y *= r1.z;
    
    // color is (atten, atten, atten, 1.f)
    // Need to calculate opacity we would have had from vs_WaveFixedFin7.inl
    // Right now that's just modulating by r4.y.
    
    out.c1 = (depth * uniforms.LifeConsts.x) * r1.yyyw;
    
    // UVW = (inUVW - 0.5) * scale + 0.5
    // where:
    // scale = (fC1U / (age * fC2U + 1.f)), fC1V / (age * fC2U + 1.f), 1.f, 1.f
    float4 r2 = float4(0,0,0,1);
    r2.xy = r1.xx * uniforms.TexConsts.yw;
    r2.xy += 1.0f;
    r2.xy = (1.0f/r2.xy);
    r2.xy *= uniforms.TexConsts.xz;
    r1.xy = in.position.xy - 0.5f;
    r1.xy *= r2.xy;
    r1.xy += 0.5f;
    out.texCoord0 = r1;
    
    return out;
}

fragment half4 ps_WaveRip(waveRipInOut in [[stage_in]],
                             texture2d<half> texture [[ texture(0) ]]) {
    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    half4 t0 = texture.sample(colorSampler, in.texCoord0.xy);
    
    return t0 * half4(in.c1);
}
