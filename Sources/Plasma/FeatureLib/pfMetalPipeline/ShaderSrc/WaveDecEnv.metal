//
//  WaveDecEnv.metal
//  plGLClient
//
//  Created by Colin Cornaby on 1/2/22.
//

#include <metal_stdlib>
using namespace metal;

#include "ShaderVertex.h"

typedef struct {
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
    float4 Tex0_Row0;
    float4 Tex0_Row1;
    float4 Tex1_Row0;
    float4 Tex1_Row1;
    float4 L2WRow0;
    float4 L2WRow1;
    float4 L2WRow2;
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
} vs_WaveDecEnv7Uniforms;

typedef struct {
    float4 position [[position]];
    float4 c1;
    float4 texCoord0;
    float4 texCoord1;
    float4 texCoord2;
    float4 texCoord3;
    float fog;
} vs_WaveDecEnv7InOut;

vertex vs_WaveDecEnv7InOut vs_WaveDecEnv_7(Vertex in [[stage_in]],
                               constant vs_WaveDecEnv7Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]]) {
    vs_WaveDecEnv7InOut out;
    
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
    worldPosition.z += uniforms.Bias.x;
    
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
    
    // Calculate our basis vectors as input into our tex3x3vspec
    // First we get our basis set off our surface. This is
    // Okay, here we go:
    // W == sum(k w Dir.x^2 A sin()) x
    // V == sum(k w Dir.x Dir.y A sin()) x
    // U == sum(k w Dir.y^2 A sin()) x
    //
    // T == sum(A sin())
    //
    // S == sum(k Dir.x A cos())
    // R == sum(k Dir.y A cos())
    //
    // Q == sum(k w A cos()) x
    //
    // M == sum(A cos())
    //
    // P == sum(w Dir.x A cos()) x
    // N == sum(w Dir.y A cos()) x
    //
    // Then:
    // Pos = (in.x + S, in.y + R, waterheight + T) // Already done above.
    //
    // Bin = (1 - W, -V, P)
    // Tan = (-V, 1 - U, N)
    // Nor = (-P, -N, 1 - Q)
    //
    // The matrix
    //      |Bx, Tx, Nx|
    //      |By, Ty, Ny|
    //      |Bz, Tz, Nz|
    // is surface2world, but we still need to fold in
    // texture2surface. We'll go with the generalized
    // (not assuming a flat surface) partials of dPos/dU and dPos/dV
    // as coming in as uv coords v8 and v9.
    // Then, if r5 = v8 X v9, then texture to surface is
    //      |v8.x, v9.x, r5.x|
    //      |v8.y, v9.y, r5.y|
    //      |v8.z, v9.z, r5.z|
    //
    // So, let's say we calc 3 vectors,
    //      r7 = (Bx, Tx, Nx)
    //      r8 = (By, Ty, Ny)
    //      r9 = (Bz, Tz, Nz)
    //
    // Then surface2world * texture2surface =
    //      |r7 dot v8, r7 dot v9, r7 dot r5|
    //      |r8 dot v8, r8 dot v9, r8 dot r5|
    //      |r9 dot v8, r9 dot v9, r9 dot r5|
    //
    // We will need r5 as v8 X v9
    
    float4 r7 = float4(in.texCoord2, 1.0);
    float4 r5 = float4(0);
    r5.xyz = r7.yzx * in.texCoord3.zxy;
    r5.xyz = (r7.zxy * -in.texCoord3.yzx) + r5.xyz;
    
    // Okay, r1 currently has the vector of cosines, and r2 has vector of sines.
    // Everything will want that times amplitude, so go ahead and fold that in.
    cosDist *= uniforms.Amplitude;
    
    r7.x = dot(sinDist, -uniforms.DirXSqKW);
    r7.y = dot(sinDist, -uniforms.DirXDirYKW);
    r7.z = dot(cosDist, -uniforms.DirXW);
    r7.x += uniforms.NumericConsts.z;
    
    float4 r8 = float4(0);
    r8.x = dot(sinDist, -uniforms.DirXDirYKW);
    r8.y = dot(sinDist, -uniforms.DirYSqKW);
    r8.z = dot(cosDist, -uniforms.DirYW);
    r8.y = r8.y + uniforms.NumericConsts.z;
    
    float4 r9 = out.position;
    r9.z = dot(cosDist, -uniforms.WK);
    r9.x = -r7.z;
    r9.y = -r8.z;
    r9.z = r9.z + uniforms.NumericConsts.z;
    
    // Okay, got everything we need, construct r1-3 as surface2world*texture2surface.
    float4 r1, r2, r3 = float4(0);
    r1.x = dot(r7.xyz, in.texCoord2);
    r1.y = dot(r7.xyz, in.texCoord3);
    r1.z = dot(r7.xyz, r5.xyz);
    
    r2.x = dot(r8.xyz, in.texCoord2);
    r2.y = dot(r8.xyz, in.texCoord3);
    r2.z = dot(r8.xyz, r5.xyz);
    
    r3.x = dot(r9.xyz, in.texCoord2);
    r3.y = dot(r9.xyz, in.texCoord3);
    r3.z = dot(r9.xyz, r5.xyz);
    
    // Following section is debug only to skip the per-vert tangent space axes.
    //add r1, c13.zxxx, r7.zzxw;
    //add r2, c13.xzxx, r7.zzyw;
    //
    //mov r3.x, -r7.x;
    //mov r3.y, -r7.y;
    //mov r3.zw, c13.zz;

    // See vs_WaveFixedFin6.inl for derivation of the following
    float4 r0 = worldPosition - uniforms.CameraPos;
    r0 *= rsqrt(dot(r0.xyz, r0.xyz));
    
    float4 r10 = float4(0);
    r10.x = dot(r0.xyz, uniforms.EnvAdjust.xyz);
    r10.y = (r10.x * r10.x) - uniforms.EnvAdjust.w;
    
    r10.z = (r10.y * rsqrt(r10.y)) + r10.x;
    r0.xyz = (r0.xyz * r10.zzz) - uniforms.EnvAdjust.xyz;
    
    // ATI 9000 is having trouble with eyeVec as computed. Normalizing seems to get it over the hump.
    r0.xyz = normalize(r0.xyz);
    
    r1.w = -r0.x;
    r2.w = -r0.y;
    r3.w = -r0.z;
    
    // Now r1-r3 are texture2world, with the eye-ray vector in .w. We just
    // need to normalize them and bung them into output UV's 1-3.
    // Note we're accounting for our environment map being flipped from
    // D3D (and all rational thought) by putting r2 into UV3 and r3 into UV2.
    r10.w = uniforms.NumericConsts.z;
    r10.x = rsqrt(dot(r1.xyz, r1.xyz));
    out.texCoord1 = r1 * r10.xxxw;
    
    r10.x = rsqrt(dot(r3.xyz, r3.xyz));
    out.texCoord2 = r3 * r10.xxxw;
    
    r10.x = rsqrt(dot(r2.xyz, r2.xyz));
    out.texCoord3 = r2 * r10.xxxw;
    
    float4 matColor = uniforms.MatColor;
    out.c1 = clamp(float4(in.color).yyyz/255.0 * matColor, 0.0, 1.0);
    
    return out;
}

fragment float4 ps_WaveDecEnv(vs_WaveDecEnv7InOut in [[stage_in]],
                             texture2d<float> normalMap [[ texture(0) ]],
                             texturecube<float> environmentMap [[ texture(FragmentShaderArgumentAttributeCubicTextures + 1) ]]) {
    // Very simular to ps_WaveFixed.inl. Only the final coloring is different.
    // Even though so far they are identical.
    
    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    float4 t0 = 2 * (normalMap.sample(colorSampler, in.texCoord0.xy) - 0.5);
    float u = dot(in.texCoord1.xyz, t0.xyz);
    float v = dot(in.texCoord2.xyz, t0.xyz);
    float w = dot(in.texCoord3.xyz, t0.xyz);
    
    float3 N = float3(u, v, w);
    float3 E = float3(in.texCoord1.w, in.texCoord2.w, in.texCoord3.w);
    
    //float3 coord = reflect(E, N);
    float3 coord = 2*(dot(N, E) / dot(N, N))*N - E;
    
    // t3 now has our reflected environment map value
    // We've (presumably) attenuated the effect on a vertex basis
    // and have our color w/ attenuated alpha in v0. So all we need
    // is to multiply t3 by v0 into r0 and we're done.
    float4 out = float4(environmentMap.sample(colorSampler, coord));
    out.rgb = (out.rgb * in.c1.rgb);
    out.a = t0.a * in.c1.a;
    return out;
}
