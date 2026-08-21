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
    float4 environmentTint;
    float4 waterTint;
    float4 texCoord0;
    float4 basisRowX;
    float4 basisRowY;
    float4 basisRowZ;
    float fog;
} vs_WaveFixedFin7InOut;

vertex vs_WaveFixedFin7InOut vs_WaveFixedFin7(Vertex in                     [[stage_in]],
                             constant vs_WaveFixedFin7Uniforms & uniforms   [[ buffer(VertexShaderArgumentMaterialShaderUniforms) ]])
{
    vs_WaveFixedFin7InOut out;

    // Store our input position in world space in r6
    float4 worldPosition = float4(float4(in.position, 1.f) * uniforms.LocalToWorld, 1.f);

    // Input diffuse vertex color is:
    // r = overall transparency
    // g = reflection strength (transparency)
    // b = overall wave scaling
    //
    // a is:
    // 1/(2.f * edge length)
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
    
    float4 distances = uniforms.DirectionX * worldPosition.xxxx;
    distances = (uniforms.DirectionY * worldPosition.yyyy) + distances;
    
    //
    //    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
    distances = (distances * uniforms.Frequency) + uniforms.Phase;

    float4 cosines;
    float4 sines = fast::sincos(distances, cosines);

    // Calc our depth based filtering
    float3 depthFilter = uniforms.WaterLevel.xyz - worldPosition.zzz;
    depthFilter *= uniforms.DepthFalloff.xyz;
    depthFilter += uniforms.MinAtten.xyz;
    depthFilter = clamp(depthFilter, 0, 1);

    // Calc our filter (see above).
    float4 filteredAmp = in.color.wwww * uniforms.Lengths;
    filteredAmp = clamp(filteredAmp, 0.1f, 1.f);

    sines *= filteredAmp;
    sines *= uniforms.Amplitude;
    // r5 is now T = sum(Ai * sin())
    //    height = dp4(sinDist, kOne);
    //    accumPos.z += height; (but accumPos.z is currently 0).
    float4 accumPos = 0;
    accumPos.x = dot(sines, uniforms.NumericConsts.zzzz);
    accumPos.y = accumPos.x * depthFilter.z;
    accumPos.z = accumPos.y + uniforms.WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z); // CLAMP
    // r8.x == wave height relative to 0
    // r8.y == dampened wave relative to 0
    // r8.z == dampened wave height in world space
    // r6.z == wave height clamped to never go beneath ground level
    //
    cosines *= uniforms.Amplitude;
    cosines *= filteredAmp;

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

    worldPosition.x += dot(cosines, uniforms.DirXK);
    worldPosition.y += dot(cosines, uniforms.DirYK);


    /*
     Construct a tanget basis for the deformed surface
     to sample from the normal map with.
     */
    float4 basisRowX, basisRowY, basisRowZ = 0;

    basisRowX.x = dot(sines, -uniforms.DirXSqKW);
    basisRowY.x = dot(sines, -uniforms.DirXDirYKW);
    basisRowZ.x = dot(cosines, uniforms.DirXW);
    basisRowX.x = basisRowX.x + uniforms.NumericConsts.z;

    basisRowX.y = dot(sines, -uniforms.DirXDirYKW);
    basisRowY.y = dot(sines, -uniforms.DirYSqKW);
    basisRowZ.y = dot(cosines, uniforms.DirYW);
    basisRowY.y = basisRowY.y + uniforms.NumericConsts.z;

    basisRowX.z = dot(cosines, -uniforms.DirXW);
    basisRowY.z = dot(cosines, -uniforms.DirYW);
    basisRowZ.z = dot(sines, -uniforms.WK);
    basisRowZ.z = basisRowZ.z + uniforms.NumericConsts.z;

    // Calculate our normalized vector from camera to vtx.
    // We'll use that a couple of times coming up.
    float3 camToVertex = (worldPosition - uniforms.CameraPos).xyz;
    float pertAtten = length(camToVertex);
    camToVertex = normalize(camToVertex);

    // Calculate our specular attenuation from and into r5.w.
    // r5.w starts off the distance from vtx to camera.
    // Once we've turned it into an attenuation factor, we
    // scale the x and y of our normal map (through the transform bases)
    // so that in the distance, the normal map is flat. Note that the
    // geometry in the distance isn't necessarily flat. We want to apply
    // this scale to the normal read from the normal map before it is
    // transformed into surface space.
    pertAtten += uniforms.SpecAtten.x;
    pertAtten *= uniforms.SpecAtten.y;
    pertAtten = clamp(pertAtten, 0.f, 1.f);
    pertAtten *= pertAtten; // Square it to account for perspective
    pertAtten *= uniforms.SpecAtten.z;

    // This math is detailed in the "Eye Vector" section
    // of GPU Gems Vol 1, Ch 1

    // Normally - the vector to sample the environment map
    // would be from the perspective of a viewer in the center
    // of the environment map. This corrects that vector so it is
    // from the perspective of the viewer - not the center of the map.

    // I will not derive these functions here, the book does a much
    // better job.

    // Cyan's original notes follow:

    // Big note here. All this math can blow up if the camera position
    // is outside the environment sphere. It's assumed that's dealt
    // with in the app setting up the constants. For that reason, the
    // camera position used here might not be the real local camera position,
    // which is needed for the angular attenuation, so we burn another constant
    // with our pseudo-camera position. To restrain the pseudo-camera from
    // leaving the sphere, we make:
    //  pseudoPos = envCenter + (realPos - envCenter) * dist * R / (dist + R)
    // where dist = |realPos - envCenter|

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


    // We already have the camToVertex, but alias it to D to make it
    // line up with the equation definition
    const float3 D = camToVertex;
    // For the math to work the center of the environment map must
    // be placed at a location in the scene.
    // The vector from the camera position to the center of the
    // environment map (F) is calculated by the engine and passed
    // in through EnvAdjust. This is not constant. This also includes G.
    const float3 F = uniforms.EnvAdjust.xyz;
    const float G = uniforms.EnvAdjust.w;
    // METAL NOTE: HLSL 1.1 always applies an abs operation to values it's about to sqrt
    const float d = dot(D, F);
    const float t = d + sqrt(abs((d * d) - G));// r10.z = D dot F + SQRT((D dot F)^2 - G)
    float3 envMapRay = (D * t) - F; // r0.xyz = D * t - (envCenter - camPos)

    // ATI 9000 is having trouble with eyeVec as computed. Normalizing seems to get it over the hump.
    envMapRay = normalize(envMapRay.xyz);

    // Stash the environment map ray at the end of the tangent basis
    basisRowX.w = -envMapRay.x;
    basisRowY.w = -envMapRay.y;
    basisRowZ.w = -envMapRay.z;

    basisRowX.xyz = normalize(basisRowX.xyz);
    basisRowX.xy *= pertAtten;
    out.basisRowX = basisRowX;

    basisRowY.xyz = normalize(basisRowY.xyz);
    basisRowY.xy *= pertAtten;
    out.basisRowZ = basisRowY;

    basisRowZ.xyz = normalize(basisRowZ.xyz);
    basisRowZ.xy *= pertAtten;
    out.basisRowY = basisRowZ;

    float3 normal = float3(basisRowX.z, basisRowY.z, basisRowZ.z);

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

    //
    // // Transform position to screen
    //
    //
    const float4 ndcPosition = worldPosition * uniforms.WorldToNDC;
    out.fog = (ndcPosition.w + uniforms.FogSet.x) * uniforms.FogSet.y;
    out.position = ndcPosition;

    // Transform our uvw
    out.texCoord0 = float4(in.position.xy * uniforms.UVScale.x,
                           0, 1);

    // This next section controls how the environment map
    // is blended. At shallower angles to the fragment, the
    // environment map will be much weaker.
    // We also want to weaken the reflection in shallower water.
    // Original Cyan notes follow:

    // Questionble attenuation follows
    // vector from this point to camera and normalize stashed in r5
    // Dot that with the computed normal
    float4 modColor = float4(0);
    // Remember: in.color.z is a wave scale factor
    modColor.rgba = 1.f - (dot(-camToVertex.xyz, normal) * in.color.z);
    // Remap the alpha to a range between 0.5..1
    modColor.a = (modColor.a + 1.f) * 0.5f;
    modColor *= depthFilter.yyyx; // HACKTESTCOLOR
    // r in the color is the alpha factor of the vertex
    modColor.a *= in.color.r * uniforms.WaterTint.w;
    out.environmentTint = clamp(modColor * uniforms.EnvTint, 0, 1);
    out.waterTint = uniforms.WaterTint; // SEENORM

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
    float3 normalMapValue = 2 * (normalMap.sample(colorSampler, in.texCoord0.xy).rgb - 0.5);
    // Transform the normal map by the tangent basis
    float u = dot(in.basisRowX.xyz, normalMapValue);
    float v = dot(in.basisRowY.xyz, normalMapValue);
    float w = dot(in.basisRowZ.xyz, normalMapValue);

    float3 N = float3(u, v, w);
    // Eye vector was stored at the end of the tangent basis
    float3 E = float3(in.basisRowX.w, in.basisRowY.w, in.basisRowZ.w);

    // Invert the normal to an incident ray, then reflect
    float3 coord = reflect(-E, N);

    float4 out = float4(environmentMap.sample(colorSampler, coord));
    out = (out * in.environmentTint) + in.waterTint;
    out.a = in.environmentTint.a;
    return out;
}
