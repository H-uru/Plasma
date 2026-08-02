/*
 * Copyright (c) 2016-2021, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * Native Metal port of the production, non-bent-normal XeGTAO path from
 * GameTechDev/XeGTAO commit a5b1686c7ea37788eeb3576b5be47f7c03db532c.
 */

#include <metal_stdlib>
using namespace metal;

constant float GTAO_PI = 3.14159265358979323846f;
constant float GTAO_HALF_PI = 1.57079632679489661923f;
constant float GTAO_OCCLUSION_SCALE = 1.5f;

struct GTAOConstants
{
    int2 viewportSize;
    float2 viewportPixelSize;
    float2 depthUnpackConsts;
    float2 cameraTanHalfFOV;
    float2 ndcToViewMul;
    float2 ndcToViewAdd;
    float2 ndcToViewMulPixelSize;
    float effectRadius;
    float effectFalloffRange;
    float radiusMultiplier;
    float padding0;
    float finalValuePower;
    float denoiseBlurBeta;
    float sampleDistributionPower;
    float thinOccluderCompensation;
    float depthMIPSamplingOffset;
    int noiseIndex;
};

struct GTAOSamples
{
    uint sliceCount;
    uint stepsPerSlice;
};

inline float GTAOSaturate(float value) { return clamp(value, 0.0f, 1.0f); }
inline float4 GTAOSaturate(float4 value) { return clamp(value, 0.0f, 1.0f); }

inline float GTAOLinearizeDepth(float screenDepth, constant GTAOConstants& gtao)
{
    float denominator = gtao.depthUnpackConsts.y - screenDepth;
    denominator = abs(denominator) < 1e-7f ? copysign(1e-7f, denominator) : denominator;
    return gtao.depthUnpackConsts.x / denominator;
}

inline float3 GTAOViewPosition(float2 screenPos, float viewDepth,
                               constant GTAOConstants& gtao)
{
    return float3((gtao.ndcToViewMul * screenPos + gtao.ndcToViewAdd) * viewDepth,
                  viewDepth);
}

inline float4 GTAOCalculateEdges(float center, float left, float right,
                                 float top, float bottom)
{
    float4 edges = float4(left, right, top, bottom) - center;
    float2 slope = (edges.yw - edges.xz) * 0.5f;
    edges = min(abs(edges), abs(float4(edges.xz + slope, edges.yw - slope)));
    return GTAOSaturate(float4(1.25f) - edges / max(center * 0.011f, 1e-6f));
}

inline float GTAOPackEdges(float4 edges)
{
    edges = round(GTAOSaturate(edges) * 2.9f);
    return dot(edges, float4(64.0f / 255.0f, 16.0f / 255.0f,
                             4.0f / 255.0f, 1.0f / 255.0f));
}

inline float4 GTAOUnpackEdges(float packed)
{
    uint value = uint(round(packed * 255.0f));
    return float4((value >> 6u) & 3u, (value >> 4u) & 3u,
                  (value >> 2u) & 3u, value & 3u) / 3.0f;
}

inline float3 GTAOCalculateNormal(float4 edges, float3 center, float3 left,
                                  float3 right, float3 top, float3 bottom)
{
    float4 accepted = GTAOSaturate(float4(edges.x * edges.z, edges.z * edges.y,
                                           edges.y * edges.w, edges.w * edges.x) + 0.01f);
    float3 normal = cross(left - center, top - center) * accepted.x +
                    cross(top - center, right - center) * accepted.y +
                    cross(right - center, bottom - center) * accepted.z +
                    cross(bottom - center, left - center) * accepted.w;
    float lengthSquared = dot(normal, normal);
    return lengthSquared > 1e-12f ? normalize(normal) : float3(0.0f, 0.0f, -1.0f);
}

inline uint GTAOPackNormal(float3 normal)
{
    uint3 packed = uint3(round(clamp(normal * 0.5f + 0.5f, 0.0f, 1.0f) *
                               float3(1023.0f, 1023.0f, 1023.0f)));
    return packed.x | (packed.y << 10u) | (packed.z << 20u);
}

inline float3 GTAOUnpackNormal(uint packed)
{
    float3 normal = float3(packed & 1023u, (packed >> 10u) & 1023u,
                           (packed >> 20u) & 1023u) / 1023.0f * 2.0f - 1.0f;
    return normalize(normal);
}

inline float GTAOFastSqrt(float value)
{
    return sqrt(max(0.0f, value));
}

inline float GTAOFastAcos(float value)
{
    float x = abs(value);
    float result = (-0.156583f * x + GTAO_HALF_PI) * GTAOFastSqrt(1.0f - x);
    return value >= 0.0f ? result : GTAO_PI - result;
}

inline float GTAODepthMipFilter(float d0, float d1, float d2, float d3,
                                constant GTAOConstants& gtao)
{
    float maxDepth = max(max(d0, d1), max(d2, d3));
    float range = max(gtao.effectRadius * gtao.effectFalloffRange, 1e-6f);
    float falloffFrom = gtao.effectRadius * (1.0f - gtao.effectFalloffRange);
    float mul = -1.0f / range;
    float add = falloffFrom / range + 1.0f;
    float4 weights = clamp((maxDepth - float4(d0, d1, d2, d3)) * mul + add,
                           0.0f, 1.0f);
    return dot(weights, float4(d0, d1, d2, d3)) /
           max(dot(weights, float4(1.0f)), 1e-6f);
}

inline uint GTAOHilbertIndex(uint2 pixel)
{
    constexpr uint width = 64u;
    uint index = 0u;
    for (uint level = width / 2u; level > 0u; level /= 2u) {
        uint regionX = (pixel.x & level) != 0u;
        uint regionY = (pixel.y & level) != 0u;
        index += level * level * ((3u * regionX) ^ regionY);
        if (regionY == 0u) {
            if (regionX == 1u)
                pixel = (width - 1u) - pixel;
            pixel = pixel.yx;
        }
    }
    return index;
}

inline void GTAOPrefilterStore(float4 depths, uint2 base, uint2 pixel,
                               uint2 local, threadgroup float* scratchDepth,
                               texture2d<float, access::write> outDepth0,
                               texture2d<float, access::write> outDepth1,
                               texture2d<float, access::write> outDepth2,
                               texture2d<float, access::write> outDepth3,
                               texture2d<float, access::write> outDepth4,
                               constant GTAOConstants& gtao)
{
    if (pixel.x < uint(gtao.viewportSize.x) && pixel.y < uint(gtao.viewportSize.y))
        outDepth0.write(float4(depths.x), pixel);
    if (pixel.x + 1u < uint(gtao.viewportSize.x) && pixel.y < uint(gtao.viewportSize.y))
        outDepth0.write(float4(depths.y), pixel + uint2(1, 0));
    if (pixel.x < uint(gtao.viewportSize.x) && pixel.y + 1u < uint(gtao.viewportSize.y))
        outDepth0.write(float4(depths.z), pixel + uint2(0, 1));
    if (pixel.x + 1u < uint(gtao.viewportSize.x) && pixel.y + 1u < uint(gtao.viewportSize.y))
        outDepth0.write(float4(depths.w), pixel + uint2(1, 1));

    float mip1 = GTAODepthMipFilter(depths.x, depths.y, depths.z, depths.w, gtao);
    if (base.x < outDepth1.get_width() && base.y < outDepth1.get_height())
        outDepth1.write(float4(mip1), base);
    scratchDepth[local.y * 8u + local.x] = mip1;
    threadgroup_barrier(mem_flags::mem_threadgroup);

    if (all((local & 1u) == 0u)) {
        uint offset = local.y * 8u + local.x;
        float value = GTAODepthMipFilter(scratchDepth[offset], scratchDepth[offset + 1u],
                                         scratchDepth[offset + 8u], scratchDepth[offset + 9u], gtao);
        uint2 destination = base / 2u;
        if (destination.x < outDepth2.get_width() && destination.y < outDepth2.get_height())
            outDepth2.write(float4(value), destination);
        scratchDepth[offset] = value;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (all((local & 3u) == 0u)) {
        uint offset = local.y * 8u + local.x;
        float value = GTAODepthMipFilter(scratchDepth[offset], scratchDepth[offset + 2u],
                                         scratchDepth[offset + 16u], scratchDepth[offset + 18u], gtao);
        uint2 destination = base / 4u;
        if (destination.x < outDepth3.get_width() && destination.y < outDepth3.get_height())
            outDepth3.write(float4(value), destination);
        scratchDepth[offset] = value;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (all(local == 0u)) {
        float value = GTAODepthMipFilter(scratchDepth[0], scratchDepth[4],
                                         scratchDepth[32], scratchDepth[36], gtao);
        uint2 destination = base / 8u;
        if (destination.x < outDepth4.get_width() && destination.y < outDepth4.get_height())
            outDepth4.write(float4(value), destination);
    }
}

kernel void gtaoPrefilter(constant GTAOConstants& gtao [[buffer(0)]],
                          depth2d<float, access::read> rawDepth [[texture(0)]],
                          texture2d<float, access::write> outDepth0 [[texture(1)]],
                          texture2d<float, access::write> outDepth1 [[texture(2)]],
                          texture2d<float, access::write> outDepth2 [[texture(3)]],
                          texture2d<float, access::write> outDepth3 [[texture(4)]],
                          texture2d<float, access::write> outDepth4 [[texture(5)]],
                          uint2 base [[thread_position_in_grid]],
                          uint2 local [[thread_position_in_threadgroup]],
                          threadgroup float* scratchDepth [[threadgroup(0)]])
{
    uint2 pixel = base * 2u;
    uint2 maxPixel = uint2(gtao.viewportSize - 1);
    float4 depths;
    depths.x = GTAOLinearizeDepth(rawDepth.read(min(pixel, maxPixel)), gtao);
    depths.y = GTAOLinearizeDepth(rawDepth.read(min(pixel + uint2(1, 0), maxPixel)), gtao);
    depths.z = GTAOLinearizeDepth(rawDepth.read(min(pixel + uint2(0, 1), maxPixel)), gtao);
    depths.w = GTAOLinearizeDepth(rawDepth.read(min(pixel + uint2(1, 1), maxPixel)), gtao);
    GTAOPrefilterStore(depths, base, pixel, local, scratchDepth,
                       outDepth0, outDepth1, outDepth2, outDepth3, outDepth4, gtao);
}

inline float GTAOMinMSDepth(depth2d_ms<float, access::read> rawDepth, uint2 pixel)
{
    float depth = rawDepth.read(pixel, 0);
    for (uint sample = 1u; sample < rawDepth.get_num_samples(); ++sample)
        depth = min(depth, rawDepth.read(pixel, sample));
    return depth;
}

kernel void gtaoPrefilterMS(constant GTAOConstants& gtao [[buffer(0)]],
                            depth2d_ms<float, access::read> rawDepth [[texture(0)]],
                            texture2d<float, access::write> outDepth0 [[texture(1)]],
                            texture2d<float, access::write> outDepth1 [[texture(2)]],
                            texture2d<float, access::write> outDepth2 [[texture(3)]],
                            texture2d<float, access::write> outDepth3 [[texture(4)]],
                            texture2d<float, access::write> outDepth4 [[texture(5)]],
                            uint2 base [[thread_position_in_grid]],
                            uint2 local [[thread_position_in_threadgroup]],
                            threadgroup float* scratchDepth [[threadgroup(0)]])
{
    uint2 pixel = base * 2u;
    uint2 maxPixel = uint2(gtao.viewportSize - 1);
    float4 depths;
    depths.x = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, min(pixel, maxPixel)), gtao);
    depths.y = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, min(pixel + uint2(1, 0), maxPixel)), gtao);
    depths.z = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, min(pixel + uint2(0, 1), maxPixel)), gtao);
    depths.w = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, min(pixel + uint2(1, 1), maxPixel)), gtao);
    GTAOPrefilterStore(depths, base, pixel, local, scratchDepth,
                       outDepth0, outDepth1, outDepth2, outDepth3, outDepth4, gtao);
}

inline void GTAONormalsStore(float centerZ, float leftZ, float rightZ,
                             float topZ, float bottomZ, uint2 pixel,
                             texture2d<uint, access::write> outNormals,
                             constant GTAOConstants& gtao)
{
    int2 p = int2(pixel);
    int2 maxPixel = gtao.viewportSize - 1;
    int2 leftPixel = clamp(p + int2(-1, 0), int2(0), maxPixel);
    int2 rightPixel = clamp(p + int2(1, 0), int2(0), maxPixel);
    int2 topPixel = clamp(p + int2(0, -1), int2(0), maxPixel);
    int2 bottomPixel = clamp(p + int2(0, 1), int2(0), maxPixel);
    float2 centerUV = (float2(p) + 0.5f) * gtao.viewportPixelSize;
    float4 edges = GTAOCalculateEdges(centerZ, leftZ, rightZ, topZ, bottomZ);
    float3 normal = GTAOCalculateNormal(
        edges,
        GTAOViewPosition(centerUV, centerZ, gtao),
        GTAOViewPosition((float2(leftPixel) + 0.5f) * gtao.viewportPixelSize, leftZ, gtao),
        GTAOViewPosition((float2(rightPixel) + 0.5f) * gtao.viewportPixelSize, rightZ, gtao),
        GTAOViewPosition((float2(topPixel) + 0.5f) * gtao.viewportPixelSize, topZ, gtao),
        GTAOViewPosition((float2(bottomPixel) + 0.5f) * gtao.viewportPixelSize, bottomZ, gtao));
    outNormals.write(uint4(GTAOPackNormal(normal)), pixel);
}

kernel void gtaoNormals(constant GTAOConstants& gtao [[buffer(0)]],
                        depth2d<float, access::read> rawDepth [[texture(0)]],
                        texture2d<uint, access::write> outNormals [[texture(1)]],
                        uint2 pixel [[thread_position_in_grid]])
{
    if (any(pixel >= uint2(gtao.viewportSize)))
        return;
    int2 p = int2(pixel);
    int2 maxPixel = gtao.viewportSize - 1;
    float center = GTAOLinearizeDepth(rawDepth.read(pixel), gtao);
    float left = GTAOLinearizeDepth(rawDepth.read(uint2(clamp(p + int2(-1, 0), int2(0), maxPixel))), gtao);
    float right = GTAOLinearizeDepth(rawDepth.read(uint2(clamp(p + int2(1, 0), int2(0), maxPixel))), gtao);
    float top = GTAOLinearizeDepth(rawDepth.read(uint2(clamp(p + int2(0, -1), int2(0), maxPixel))), gtao);
    float bottom = GTAOLinearizeDepth(rawDepth.read(uint2(clamp(p + int2(0, 1), int2(0), maxPixel))), gtao);
    GTAONormalsStore(center, left, right, top, bottom, pixel, outNormals, gtao);
}

kernel void gtaoNormalsMS(constant GTAOConstants& gtao [[buffer(0)]],
                          depth2d_ms<float, access::read> rawDepth [[texture(0)]],
                          texture2d<uint, access::write> outNormals [[texture(1)]],
                          uint2 pixel [[thread_position_in_grid]])
{
    if (any(pixel >= uint2(gtao.viewportSize)))
        return;
    int2 p = int2(pixel);
    int2 maxPixel = gtao.viewportSize - 1;
    float center = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, pixel), gtao);
    float left = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, uint2(clamp(p + int2(-1, 0), int2(0), maxPixel))), gtao);
    float right = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, uint2(clamp(p + int2(1, 0), int2(0), maxPixel))), gtao);
    float top = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, uint2(clamp(p + int2(0, -1), int2(0), maxPixel))), gtao);
    float bottom = GTAOLinearizeDepth(GTAOMinMSDepth(rawDepth, uint2(clamp(p + int2(0, 1), int2(0), maxPixel))), gtao);
    GTAONormalsStore(center, left, right, top, bottom, pixel, outNormals, gtao);
}

constant sampler gtaoDepthSampler(coord::normalized, address::clamp_to_edge,
                                  filter::linear, mip_filter::linear);

inline float GTAOVisibility(int2 pixel, uint sliceCount, uint stepsPerSlice,
                            float2 noise, float3 normal,
                            texture2d<float, access::sample> viewDepth,
                            constant GTAOConstants& gtao)
{
    float2 screen = (float2(pixel) + 0.5f) * gtao.viewportPixelSize;
    float centerDepth = viewDepth.read(uint2(pixel), 0).r;
    if (centerDepth >= 65500.0f)
        return 1.0f;

    float3 center = GTAOViewPosition(screen, centerDepth * 0.99920f, gtao);
    float3 view = normalize(-center);
    float radius = gtao.effectRadius * gtao.radiusMultiplier;
    float falloffRange = gtao.effectFalloffRange * radius;
    float falloffFrom = radius * (1.0f - gtao.effectFalloffRange);
    float falloffMul = -1.0f / max(falloffRange, 1e-6f);
    float falloffAdd = falloffFrom / max(falloffRange, 1e-6f) + 1.0f;
    float screenRadius = abs(radius / max(centerDepth * gtao.ndcToViewMulPixelSize.x, 1e-6f));
    float visibility = GTAOSaturate((10.0f - screenRadius) / 100.0f) * 0.5f;
    float minSample = 1.3f / max(screenRadius, 1e-6f);

    for (uint slice = 0u; slice < sliceCount; ++slice) {
        float phi = (float(slice) + noise.x) / float(sliceCount) * GTAO_PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float2 omega = float2(cosPhi, -sinPhi) * screenRadius;
        float3 direction = float3(cosPhi, sinPhi, 0.0f);
        float3 ortho = direction - dot(direction, view) * view;
        float3 axis = normalize(cross(ortho, view));
        float3 projectedNormal = normal - axis * dot(normal, axis);
        float projectedLength = max(length(projectedNormal), 1e-6f);
        float cosNormal = GTAOSaturate(dot(projectedNormal, view) / projectedLength);
        float n = sign(dot(ortho, projectedNormal)) * GTAOFastAcos(cosNormal);
        float low0 = cos(n + GTAO_HALF_PI);
        float low1 = cos(n - GTAO_HALF_PI);
        float horizon0 = low0;
        float horizon1 = low1;

        for (uint step = 0u; step < stepsPerSlice; ++step) {
            float stepNoise = fract(noise.y + float(slice + step * stepsPerSlice) *
                                    0.6180339887498948482f);
            float s = pow((float(step) + stepNoise) / float(stepsPerSlice),
                          gtao.sampleDistributionPower) + minSample;
            float2 sampleOffset = round(s * omega) * gtao.viewportPixelSize;
            float sampleLength = length(s * omega);
            float mip = clamp(log2(max(sampleLength, 1.0f)) - gtao.depthMIPSamplingOffset,
                              0.0f, 4.0f);
            float2 screen0 = screen + sampleOffset;
            float2 screen1 = screen - sampleOffset;
            float depth0 = viewDepth.sample(gtaoDepthSampler, screen0, level(mip)).r;
            float depth1 = viewDepth.sample(gtaoDepthSampler, screen1, level(mip)).r;
            float3 delta0 = GTAOViewPosition(screen0, depth0, gtao) - center;
            float3 delta1 = GTAOViewPosition(screen1, depth1, gtao) - center;
            float distance0 = max(length(delta0), 1e-6f);
            float distance1 = max(length(delta1), 1e-6f);
            float weight0 = GTAOSaturate(distance0 * falloffMul + falloffAdd);
            float weight1 = GTAOSaturate(distance1 * falloffMul + falloffAdd);
            horizon0 = max(horizon0, mix(low0, dot(delta0 / distance0, view), weight0));
            horizon1 = max(horizon1, mix(low1, dot(delta1 / distance1, view), weight1));
        }

        projectedLength = mix(projectedLength, 1.0f, 0.05f);
        float h0 = -GTAOFastAcos(horizon1);
        float h1 = GTAOFastAcos(horizon0);
        float arc0 = (cosNormal + 2.0f * h0 * sin(n) - cos(2.0f * h0 - n)) * 0.25f;
        float arc1 = (cosNormal + 2.0f * h1 * sin(n) - cos(2.0f * h1 - n)) * 0.25f;
        visibility += projectedLength * (arc0 + arc1);
    }
    visibility /= float(sliceCount);
    return max(0.03f, pow(max(visibility, 0.0f), gtao.finalValuePower));
}

kernel void gtaoMain(constant GTAOConstants& gtao [[buffer(0)]],
                     constant GTAOSamples& samples [[buffer(1)]],
                     texture2d<float, access::sample> viewDepth [[texture(0)]],
                     texture2d<uint, access::read> packedNormals [[texture(1)]],
                     texture2d<uint, access::write> outWorkingAO [[texture(2)]],
                     texture2d<float, access::write> outEdges [[texture(3)]],
                     uint2 pixel [[thread_position_in_grid]])
{
    if (any(pixel >= uint2(gtao.viewportSize)))
        return;
    float center = viewDepth.read(pixel, 0).r;
    if (center >= 65500.0f) {
        outWorkingAO.write(uint4(170u), pixel);
        outEdges.write(float4(1.0f), pixel);
        return;
    }
    int2 p = int2(pixel);
    int2 maxPixel = gtao.viewportSize - 1;
    float left = viewDepth.read(uint2(clamp(p + int2(-1, 0), int2(0), maxPixel)), 0).r;
    float right = viewDepth.read(uint2(clamp(p + int2(1, 0), int2(0), maxPixel)), 0).r;
    float top = viewDepth.read(uint2(clamp(p + int2(0, -1), int2(0), maxPixel)), 0).r;
    float bottom = viewDepth.read(uint2(clamp(p + int2(0, 1), int2(0), maxPixel)), 0).r;
    outEdges.write(float4(GTAOPackEdges(
        GTAOCalculateEdges(center, left, right, top, bottom))), pixel);

    uint index = GTAOHilbertIndex(pixel & 63u);
    float2 noise = fract(0.5f + float(index) *
                         float2(0.75487766624669276005f, 0.56984029099805326591f));
    float3 normal = GTAOUnpackNormal(packedNormals.read(pixel).r);
    float visibility = GTAOVisibility(p, samples.sliceCount, samples.stepsPerSlice,
                                      noise, normal, viewDepth, gtao);
    outWorkingAO.write(uint4(uint(clamp(visibility / GTAO_OCCLUSION_SCALE, 0.0f, 1.0f) *
                                  255.0f + 0.5f)), pixel);
}

inline float GTAOEdgeWeight(float4 centerEdges, int2 offset)
{
    float weight = 1.0f;
    if (offset.x < 0) weight *= centerEdges.x;
    if (offset.x > 0) weight *= centerEdges.y;
    if (offset.y < 0) weight *= centerEdges.z;
    if (offset.y > 0) weight *= centerEdges.w;
    return weight;
}

kernel void gtaoDenoise(constant GTAOConstants& gtao [[buffer(0)]],
                        texture2d<uint, access::read> workingAO [[texture(0)]],
                        texture2d<float, access::read> workingEdges [[texture(1)]],
                        texture2d<uint, access::write> outFinalAO [[texture(2)]],
                        uint2 pixel [[thread_position_in_grid]])
{
    if (any(pixel >= uint2(gtao.viewportSize)))
        return;
    int2 p = int2(pixel);
    int2 maxPixel = gtao.viewportSize - 1;
    float4 centerEdges = GTAOUnpackEdges(workingEdges.read(pixel).r);
    float sum = 0.0f;
    float sumWeight = 0.0f;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            int2 offset = int2(x, y);
            uint2 samplePixel = uint2(clamp(p + offset, int2(0), maxPixel));
            float spatial = (x == 0 && y == 0) ? 1.0f :
                            ((x == 0 || y == 0) ? 0.75f : 0.425f);
            float weight = spatial * GTAOEdgeWeight(centerEdges, offset);
            sum += float(workingAO.read(samplePixel).r) * weight;
            sumWeight += weight;
        }
    }
    float visibility = sum / max(sumWeight, 1e-6f) / 255.0f * GTAO_OCCLUSION_SCALE;
    outFinalAO.write(uint4(uint(clamp(visibility, 0.0f, 1.0f) * 255.0f + 0.5f)), pixel);
}

struct GTAOCompositeVertexOut
{
    float4 position [[position]];
};

vertex GTAOCompositeVertexOut gtaoCompositeVertex(uint vertexID [[vertex_id]])
{
    float2 corner = float2((vertexID << 1u) & 2u, vertexID & 2u);
    GTAOCompositeVertexOut out;
    out.position = float4(corner * 2.0f - 1.0f, 0.0f, 1.0f);
    return out;
}

fragment half4 gtaoCompositeFragment(GTAOCompositeVertexOut in [[stage_in]],
                                     texture2d<uint, access::read> finalAO [[texture(0)]])
{
    float visibility = float(finalAO.read(uint2(in.position.xy)).r) / 255.0f;
    return half4(half3(visibility), 1.0h);
}
