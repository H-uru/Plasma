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
#include "Water.h"

typedef struct
{
    matrix_float4x4 WorldToNDC;
    float4 WaterTint;
    float4 Frequency;
    float4 Phase;
    float4 Amplitude;
    float4 DirectionX;
    float4 DirectionY;
    float4 UVScale;
    float4 SpecAtten;
    float4 Scrunch;
    float4 SinConsts;
    float4 CosConsts;
    float4 PiConsts;
    float4 NumericConsts;
    float4 CameraPos;
    float4 WindRot;
    float4 EnvAdjust;
    float4 EnvTint;
    float3x4 LocalToWorld;
    float4 Lengths;
    float4 WaterLevel;
    float4 DepthFalloff;
    float4 MinAtten;
    float4 FogSet;
    float4 DirXK;
    float4 DirYK;
    float4 DirXW;
    float4 DirYW;
    float4 WK;
    float4 DirXSqKW;
    float4 DirXDirYKW;
    float4 DirYSqKW;
} vs_WaveFixedFin7Uniforms;

typedef struct
{
    float4 position [[position]];
    float4 c1;
    float4 c2;
    float4 texCoord0;
    float4 texCoord1;
    float4 texCoord2;
    float4 texCoord3;
    float fog;
} vs_WaveFixedFin7InOut;

void CalcEyeRayAndBumpAttenuation(const float4 wPos,
                                  const float4 cameraPos,
                                  const float4 specAtten,
                                  thread float3 &cam2Vtx,
                                  thread float &pertAtten)
{
    cam2Vtx = wPos.xyz - cameraPos.xyz;
    pertAtten = length(cam2Vtx);
    cam2Vtx /= pertAtten;

    // Calculate our specular attenuation from and into r5.w.
    // r5.w starts off the distance from vtx to camera.
    // Once we've turned it into an attenuation factor, we
    // scale the x and y of our normal map (through the transform bases)
    // so that in the distance, the normal map is flat. Note that the
    // geometry in the distance isn't necessarily flat. We want to apply
    // this scale to the normal read from the normal map before it is
    // transformed into surface space.
    pertAtten += specAtten.x;
    pertAtten *= specAtten.y;
    pertAtten = clamp(pertAtten, 0.f, 1.f);
    pertAtten *= pertAtten; // Square it to account for perspective
    pertAtten *= specAtten.z;
}

float3 FinitizeEyeRay(const float3 cam2Vtx, const float4 envAdjust)
{
    // So, our "finitized" eyeray is:
    //  camPos + D * t - envCenter = D * t - (envCenter - camPos)
    // with
    //  D = (pos - camPos) / |pos - camPos| // normalized usual eyeray
    // and
    //  t = D dot F + sqrt( (D dot F)^2 - G )
    // with
    //  F = (envCenter - camPos)    => c19.xyz
    //  G = F^2 - R^2               => c19.w
    //  R = environment radius.     => unused
    //
    // This all derives from the positive root of equation
    //  (camPos + (pos - camPos) * t - envCenter)^2 = R^2,
    // In other words, where on a sphere of radius R centered about envCenter
    // does the ray from the real camera position through this point hit.
    //
    // Note that F, G, and R are all constants (one point, two scalars).
    //
    // So first we calculate D into r0,
    // then D dot F into r10.x,
    // then (D dot F)^2 - G into r10.y
    // then rsq( (D dot F)^2 - G ) into r9.x;
    // then t = r10.z = r10.x + r10.y * r9.x;
    // and
    // r0 = D * t - (envCenter - camPos)
    //      = r0 * r10.zzzz - F;
    //
    //https://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch01.html
    
    const float3 F = envAdjust.xyz;
    const float G = envAdjust.w;
    // METAL NOTE: HLSL 1.1 always applies an abs operation to values it's about to sqrt
    const float3 t = dot(cam2Vtx, F.xyz) + sqrt(abs(pow(abs(dot(cam2Vtx, F.xyz)), 2) - G));// r10.z = D dot F + SQRT((D dot F)^2 - G)
    const float3 eyeRay = (cam2Vtx * t) - F; // r0.xyz = D * t - (envCenter - camPos)

    // ATI 9000 is having trouble with eyeVec as computed. Normalizing seems to get it over the hump.
    return normalize(eyeRay);
}

vertex vs_WaveFixedFin7InOut vs_WaveFixedFin7(Vertex in [[stage_in]],
                               constant vs_WaveFixedFin7Uniforms & uniforms [[ buffer(VertexShaderArgumentMaterialShaderUniforms) ]]) {
    vs_WaveFixedFin7InOut out;

    // Store our input position in world space in r6
    float4 worldPosition = float4(transpose(uniforms.LocalToWorld) * float4(in.position, 1.0), 1.0);

    //

    // Input diffuse v5 color is:
    // v5.r = overall transparency
    // v5.g = reflection strength (transparency)
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
    //  c25 = waterlevel + offset
    //  c26 = (maxAtten - minAtten) / depthFalloff
    //  c27 = minAtten.
    // And in particular:
    //  c25.w = waterlevel
    //  c26.w = 1.f;
    //  c27.w = 0;
    // So r4.w is the depth of this vertex in feet.

    // Dot our position with our direction vectors.
    
    float4 distance = uniforms.DirectionX * worldPosition.xxxx;
    distance = (uniforms.DirectionY * worldPosition.yyyy) + distance;
    
    //
    //    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
    distance = (distance * uniforms.Frequency) + uniforms.Phase;
    
    /*
     Metal note: This section of the shader originally implemented a fast sin/cos
     algorithm in HLSL - including the GPU Gems Ch 1 version. Metal has a built in
     fast cos/sin algorithm. When porting this shader to a different shading language,
     make sure fast math or a fast algorithm is available for best performance. Fast
     math is on for the MSL compiler, but I'm making the fast version explicit here.
     */
    float4 cosDist = fast::cos(distance);
    float4 sinDist = fast::sin(distance);


    // Calc our depth based filtering here into r4 (because we don't use it again
    // after here, and we need our filtering shortly).
    float3 depthFilter = CalcDepthFilter(uniforms.WaterLevel, uniforms.DepthFalloff, worldPosition, uniforms.MinAtten);

    // Calc our filter (see above).
    const float4 inColor = float4(in.color) / 255.0f;
    float4 filter = inColor.wwww * uniforms.Lengths;
    filter = clamp(filter, 0.0f, 1.0f);

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
    float4 accumPos = 0;
    accumPos.x = dot(sinDist, float4(1.0f));
    accumPos.y = accumPos.x * depthFilter.z;
    accumPos.z = accumPos.y + uniforms.WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z); // CLAMP
    // r8.x == wave height relative to 0
    // r8.y == dampened wave relative to 0
    // r8.z == dampened wave height in world space
    // r6.z == wave height clamped to never go beneath ground level
    //
    //    cosDist *= kAmplitude.xyzw; // Combine?
    //METAL NOTE: cosDist is now r7
    cosDist *= uniforms.Amplitude;
    //    cosDist *= filter;
    cosDist *= filter;
    // r7 is now M = sum(Ai * cos())

    // Okay, here we go:
    // W == sum(k w Dir.x^2 A sin())
    // V == sum(k w Dir.x Dir.y A sin())
    // U == sum(k w Dir.y^2 A sin())
    //
    // T == sum(A sin())
    //
    // S == sum(k Dir.x A cos())
    // R == sum(k Dir.y A cos())
    //
    // Q == sum(k w A cos())
    //
    // M == sum(A cos())
    //
    // P == sum(w Dir.x A cos())
    // N == sum(w Dir.y A cos())
    //
    // Then:
    // Pos = (in.x + S, in.y + R, waterheight + T)
    //
    // Bin = (1 - W, -V, P)
    // Tan = (-V, 1 - U, N)
    // Nor = (-P, -N, 1 - Q)
    //
    // But we want the transpose of that to go into r1-r3

    // CalcFinalPosition
    
    worldPosition.x += dot(cosDist, uniforms.DirXK);
    worldPosition.y += dot(cosDist, uniforms.DirYK);

    // CalcTangentBasis
    float4 r1, r2, r3 = 0;

    r1.x = dot(sinDist, -uniforms.DirXSqKW);
    r2.x = dot(sinDist, -uniforms.DirXDirYKW);
    r3.x = dot(cosDist, uniforms.DirXW);
    r1.x = r1.x + uniforms.NumericConsts.z;

    r1.y = dot(sinDist, -uniforms.DirXDirYKW);
    r2.y = dot(sinDist, -uniforms.DirYSqKW);
    r3.y = dot(cosDist, uniforms.DirYW);
    r2.y = r2.y + uniforms.NumericConsts.z;

    r1.z = dot(cosDist, -uniforms.DirXW);
    r2.z = dot(cosDist, -uniforms.DirYW);
    r3.z = dot(sinDist, -uniforms.WK);
    r3.z = r3.z + uniforms.NumericConsts.z;

    float3 cam2Vtx;
    float pertAtten;
    
    CalcEyeRayAndBumpAttenuation(worldPosition, uniforms.CameraPos, uniforms.SpecAtten, cam2Vtx, pertAtten);

    float3 eyeRay = FinitizeEyeRay(cam2Vtx, uniforms.EnvAdjust);

    r1.w = -eyeRay.x;
    r2.w = -eyeRay.y;
    r3.w = -eyeRay.z;

    float4 r0 = float4(0);
    r0.zw = uniforms.NumericConsts.xz;

    float4 r11 = float4(0);

    r0.x = dot(r1.xyz, r1.xyz);
    r0.xy = rsqrt(r0.x);
    r0.x *= pertAtten;
    out.texCoord1 = r1 * r0.xxyw;
    r11.x = r1.z * r0.y;

    r0.x = dot(r2.xyz, r2.xyz);
    r0.xy = rsqrt(r0.x);
    r0.x *= pertAtten;
    out.texCoord3 = r2 * r0.xxyw;
    r11.y = r2.z * r0.y;

    r0.x = dot(r3.xyz, r3.xyz);
    r0.xy = rsqrt(r0.x);
    r0.x *= pertAtten;
    out.texCoord2 = r3 * r0.xxyw;
    r11.z = r3.z * r0.y;
    
    /*
    // Want:
    //    oT1 = (BIN.x, TAN.x, NORM.x, view2pos.x)
    //    oT2 = (BIN.y, TAN.y, NORM.y, view2pos.y)
    //    ot3 = (BIN.z, TAN.z, NORM.z, view2pos.z)
    // with BIN, TAN, and NORM normalized.
    // Unnormalized, we have
    //    BIN = (1, 0, -r7.x) where r7 == accumCos
    //    TAN = (0, 1, -r7.y)
    //    NORM= (r7.x, r7.y, 1)
    // So, unnormalized, we have
    //    oT1 = (1, 0, r7.x, view2pos.x)
    //    oT2 = (0, 1, r7.y, view2pos.y)
    //    oT3 = (-r7.x, -r7.y, 1, view2pos.z)
    // which is just reversing the signs on the accumCos
    // terms above. So the normalized version is just
    // reversing the signs on the normalized version above.
    */
    //mov oT3, r4;

    //
    // // Transform position to screen
    //
    //
    // CalcScreenPosAndFog
    float4 r9, r10;
    r9 = worldPosition * uniforms.WorldToNDC;
    r10.x = r9.w + uniforms.FogSet.x;
    out.fog = r10.x * uniforms.FogSet.y;
    out.position = r9;

    // Transform our uvw
    out.texCoord0 = float4(in.position.xy * uniforms.UVScale.x,
                           0, 1);

    // Questionble attenuation follows
    // vector from this point to camera and normalize stashed in r5
    // Dot that with the computed normal
    r1.x = dot(-cam2Vtx.xyz, r11.xyz);
    r1.x = r1.x * inColor.z;
    r1.xyzw = uniforms.NumericConsts.z - r1.x;
    r1.w += uniforms.NumericConsts.z;
    r1.w *= uniforms.NumericConsts.y;
    // No need to clamp, since the destination register (in the pixel shader)
    // will saturate [0..1] anyway.
    r1 *= depthFilter.yyyx; // HACKTESTCOLOR
    //R in the in color is the alpha value, but remember it's encoded ARGB
    r1.w *= inColor.g;
    r1.w *= uniforms.WaterTint.w;
    out.c1 = clamp(r1 * uniforms.EnvTint, 0, 1);
    out.c2 = uniforms.WaterTint; // SEENORM

    return out;
}

fragment float4 ps_WaveFixed(vs_WaveFixedFin7InOut in           [[stage_in]],
                             texture2d<float> normalMap         [[ texture(0) ]],
                             texturecube<float> environmentMap  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 3) ]])
{
    // Short pixel shader. Use the texm3x3vspec to do a per-pixel
    // reflected lookup into our environment map.
    // Input:
    //    t0    - Normal map in tangent space. Apply _bx2 modifier to shift
    //             [0..255] -> [-1..1]
    //    t1    - UVW = tangent + eye2pos.x, map ignored.
    //    t2    - UVW = binormal + eye2pos.y, map ignored
    //    t3    - UVW = normal + eye2pos.z, map = environment cube map
    //    v0    - attenuating color/alpha.
    //    See docs on texm3x3vspec for explanation of the eye2pos wackiness.
    // Output:
    //    r0 = reflected lookup from environment map X input v0.
    //    Since environment map has alpha = 255, the output of this
    //    shader can be used for either alpha or additive blending,
    //    as long as v0 is fed in appropriately.

    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    float3 t0 = 2 * (normalMap.sample(colorSampler, in.texCoord0.xy).rgb - 0.5);
    float u = dot(in.texCoord1.xyz, t0);
    float v = dot(in.texCoord2.xyz, t0);
    float w = dot(in.texCoord3.xyz, t0);

    float3 N = float3(u, v, w);
    float3 E = float3(in.texCoord1.w, in.texCoord2.w, in.texCoord3.w);

    float3 reflectCoord = reflect(E, N);

    float4 out = float4(environmentMap.sample(colorSampler, reflectCoord));
    out = (out * in.c1) + in.c2;
    out.a = in.c1.a;
    return out;
}
