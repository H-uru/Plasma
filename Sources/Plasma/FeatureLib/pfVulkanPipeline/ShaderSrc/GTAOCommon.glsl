/*
 * Copyright (C) 2016-2021, Intel Corporation
 * SPDX-License-Identifier: MIT
 *
 * Native GLSL port of the production, non-bent-normal XeGTAO path from
 * GameTechDev/XeGTAO commit a5b1686c7ea37788eeb3576b5be47f7c03db532c.
 */

#extension GL_EXT_scalar_block_layout : require

const float GTAO_PI = 3.14159265358979323846;
const float GTAO_HALF_PI = 1.57079632679489661923;
const float GTAO_OCCLUSION_SCALE = 1.5;

layout(scalar, set = 0, binding = 0) uniform GTAOConstantBlock
{
    ivec2 viewportSize;
    vec2 viewportPixelSize;
    vec2 depthUnpackConsts;
    vec2 cameraTanHalfFOV;
    vec2 ndcToViewMul;
    vec2 ndcToViewAdd;
    vec2 ndcToViewMulPixelSize;
    float effectRadius;
    float effectFalloffRange;
    float radiusMultiplier;
    float skyViewDepth;
    float finalValuePower;
    float denoiseBlurBeta;
    float sampleDistributionPower;
    float thinOccluderCompensation;
    float depthMIPSamplingOffset;
    int noiseIndex;
} gtao;

float GTAOSaturate(float value) { return clamp(value, 0.0, 1.0); }
vec4 GTAOSaturate(vec4 value) { return clamp(value, vec4(0.0), vec4(1.0)); }

float GTAOLinearizeDepth(float screenDepth)
{
    float denominator = gtao.depthUnpackConsts.y - screenDepth;
    if (abs(denominator) < 1e-7)
        return 65504.0;
    return clamp(gtao.depthUnpackConsts.x / denominator, 0.0, 65504.0);
}

vec3 GTAOViewPosition(vec2 screenPos, float viewDepth)
{
    return vec3((gtao.ndcToViewMul * screenPos + gtao.ndcToViewAdd) * viewDepth,
                viewDepth);
}

vec4 GTAOCalculateEdges(float center, float left, float right, float top, float bottom)
{
    vec4 edges = vec4(left, right, top, bottom) - center;
    float slopeLR = (edges.y - edges.x) * 0.5;
    float slopeTB = (edges.w - edges.z) * 0.5;
    vec4 adjusted = edges + vec4(slopeLR, -slopeLR, slopeTB, -slopeTB);
    edges = min(abs(edges), abs(adjusted));
    return GTAOSaturate(vec4(1.25) - edges / max(center * 0.011, 1e-6));
}

float GTAOPackEdges(vec4 edges)
{
    edges = round(GTAOSaturate(edges) * 2.9);
    return dot(edges, vec4(64.0, 16.0, 4.0, 1.0) / 255.0);
}

vec4 GTAOUnpackEdges(float packed)
{
    uint value = uint(packed * 255.5);
    return vec4(float((value >> 6) & 3u), float((value >> 4) & 3u),
                float((value >> 2) & 3u), float(value & 3u)) / 3.0;
}

vec3 GTAOCalculateNormal(vec4 edges, vec3 center, vec3 left, vec3 right,
                         vec3 top, vec3 bottom)
{
    vec4 accepted = GTAOSaturate(vec4(edges.x * edges.z, edges.z * edges.y,
                                       edges.y * edges.w, edges.w * edges.x) + 0.01);
    left = normalize(left - center);
    right = normalize(right - center);
    top = normalize(top - center);
    bottom = normalize(bottom - center);
    return normalize(accepted.x * cross(left, top) +
                     accepted.y * cross(top, right) +
                     accepted.z * cross(right, bottom) +
                     accepted.w * cross(bottom, left));
}

uint GTAOPackNormal(vec3 normal)
{
    vec3 value = clamp(normal * 0.5 + 0.5, vec3(0.0), vec3(1.0));
    return uint(value.x * 2047.0 + 0.5) |
           (uint(value.y * 2047.0 + 0.5) << 11) |
           (uint(value.z * 1023.0 + 0.5) << 22);
}

vec3 GTAOUnpackNormal(uint packed)
{
    vec3 value = vec3(float(packed & 0x7ffu) / 2047.0,
                      float((packed >> 11) & 0x7ffu) / 2047.0,
                      float((packed >> 22) & 0x3ffu) / 1023.0);
    return normalize(value * 2.0 - 1.0);
}

float GTAOFastSqrt(float value)
{
    return uintBitsToFloat(0x1fbd1df5u + (floatBitsToUint(value) >> 1));
}

float GTAOFastAcos(float value)
{
    float x = abs(value);
    float result = (-0.156583 * x + GTAO_HALF_PI) * GTAOFastSqrt(max(0.0, 1.0 - x));
    return value >= 0.0 ? result : GTAO_PI - result;
}

float GTAODepthMipFilter(float d0, float d1, float d2, float d3)
{
    float maxDepth = max(max(d0, d1), max(d2, d3));
    float radius = 0.75 * gtao.effectRadius * gtao.radiusMultiplier;
    float range = gtao.effectFalloffRange * radius;
    float falloffFrom = radius * (1.0 - gtao.effectFalloffRange);
    float mul = -1.0 / max(range, 1e-6);
    float add = falloffFrom / max(range, 1e-6) + 1.0;
    vec4 weights = clamp((maxDepth - vec4(d0, d1, d2, d3)) * mul + add,
                         vec4(0.0), vec4(1.0));
    return dot(weights, vec4(d0, d1, d2, d3)) / max(dot(weights, vec4(1.0)), 1e-6);
}

uint GTAOHilbertIndex(uvec2 pixel)
{
    const uint width = 64u;
    uint index = 0u;
    for (uint level = width / 2u; level > 0u; level /= 2u) {
        uint regionX = (pixel.x & level) != 0u ? 1u : 0u;
        uint regionY = (pixel.y & level) != 0u ? 1u : 0u;
        index += level * level * ((3u * regionX) ^ regionY);
        if (regionY == 0u) {
            if (regionX == 1u)
                pixel = (width - 1u) - pixel;
            pixel = pixel.yx;
        }
    }
    return index;
}

// A clamp-to-edge sampler turns a tap past the frame border into a duplicate of
// the border texel. That reads back as real geometry and fabricates a horizon,
// which shows up as a dark band hugging all four edges of the screen. Nothing is
// known about off-screen geometry, so such a tap must contribute no occlusion.
bool GTAOOnScreen(vec2 screen)
{
    return all(greaterThanEqual(screen, vec2(0.0))) &&
           all(lessThanEqual(screen, vec2(1.0)));
}

float GTAOVisibility(ivec2 pixel, uint sliceCount, uint stepsPerSlice,
                     vec2 noise, vec3 normal, sampler2D viewDepth)
{
    vec2 screen = (vec2(pixel) + 0.5) * gtao.viewportPixelSize;
    float centerDepth = texelFetch(viewDepth, pixel, 0).r;
    if (centerDepth >= gtao.skyViewDepth)
        return 1.0;

    // Nudge the center toward the camera to hide depth-buffer imprecision. The
    // working depth chain is 32 bit, so this is XeGTAO's FP32 constant, not its
    // much heavier FP16 one.
    vec3 center = GTAOViewPosition(screen, centerDepth * 0.99999);
    vec3 view = normalize(-center);
    float radius = gtao.effectRadius * gtao.radiusMultiplier;
    float falloffRange = gtao.effectFalloffRange * radius;
    float falloffFrom = radius * (1.0 - gtao.effectFalloffRange);
    float falloffMul = -1.0 / max(falloffRange, 1e-6);
    float falloffAdd = falloffFrom / max(falloffRange, 1e-6) + 1.0;
    // XeGTAO's thickness heuristic: stretching the view-space Z of the sample
    // delta discards occluders behind the center sooner. At the default
    // compensation of 0 this is exactly length(delta).
    float thickness = 1.0 + gtao.thinOccluderCompensation;
    float screenRadius = abs(radius / max(centerDepth * gtao.ndcToViewMulPixelSize.x, 1e-6));
    float visibility = GTAOSaturate((10.0 - screenRadius) / 100.0) * 0.5;
    float minSample = 1.3 / max(screenRadius, 1e-6);

    for (uint slice = 0u; slice < sliceCount; ++slice) {
        float phi = (float(slice) + noise.x) / float(sliceCount) * GTAO_PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        vec2 omega = vec2(cosPhi, -sinPhi) * screenRadius;
        vec3 direction = vec3(cosPhi, sinPhi, 0.0);
        vec3 ortho = direction - dot(direction, view) * view;
        vec3 axis = normalize(cross(ortho, view));
        vec3 projectedNormal = normal - axis * dot(normal, axis);
        float projectedLength = max(length(projectedNormal), 1e-6);
        float cosNormal = GTAOSaturate(dot(projectedNormal, view) / projectedLength);
        float n = sign(dot(ortho, projectedNormal)) * GTAOFastAcos(cosNormal);
        float low0 = cos(n + GTAO_HALF_PI);
        float low1 = cos(n - GTAO_HALF_PI);
        float horizon0 = low0;
        float horizon1 = low1;

        for (uint step = 0u; step < stepsPerSlice; ++step) {
            float stepNoise = fract(noise.y + float(slice + step * stepsPerSlice) *
                                    0.6180339887498948482);
            float s = pow((float(step) + stepNoise) / float(stepsPerSlice),
                          gtao.sampleDistributionPower) + minSample;
            vec2 sampleOffset = round(s * omega) * gtao.viewportPixelSize;
            float sampleLength = length(s * omega);
            float mip = clamp(log2(max(sampleLength, 1.0)) - gtao.depthMIPSamplingOffset,
                              0.0, 4.0);

            vec2 screen0 = screen + sampleOffset;
            vec2 screen1 = screen - sampleOffset;
            float depth0 = textureLod(viewDepth, screen0, mip).r;
            float depth1 = textureLod(viewDepth, screen1, mip).r;
            vec3 delta0 = GTAOViewPosition(screen0, depth0) - center;
            vec3 delta1 = GTAOViewPosition(screen1, depth1) - center;
            float distance0 = max(length(delta0), 1e-6);
            float distance1 = max(length(delta1), 1e-6);
            float falloffBase0 = length(vec3(delta0.xy, delta0.z * thickness));
            float falloffBase1 = length(vec3(delta1.xy, delta1.z * thickness));
            float weight0 = GTAOOnScreen(screen0)
                          ? GTAOSaturate(falloffBase0 * falloffMul + falloffAdd) : 0.0;
            float weight1 = GTAOOnScreen(screen1)
                          ? GTAOSaturate(falloffBase1 * falloffMul + falloffAdd) : 0.0;
            float horizonSample0 = dot(delta0 / distance0, view);
            float horizonSample1 = dot(delta1 / distance1, view);
            horizonSample0 = mix(low0, horizonSample0, weight0);
            horizonSample1 = mix(low1, horizonSample1, weight1);
            horizon0 = max(horizon0, horizonSample0);
            horizon1 = max(horizon1, horizonSample1);
        }

        projectedLength = mix(projectedLength, 1.0, 0.05);
        float h0 = -GTAOFastAcos(horizon1);
        float h1 = GTAOFastAcos(horizon0);
        float arc0 = (cosNormal + 2.0 * h0 * sin(n) - cos(2.0 * h0 - n)) * 0.25;
        float arc1 = (cosNormal + 2.0 * h1 * sin(n) - cos(2.0 * h1 - n)) * 0.25;
        visibility += projectedLength * (arc0 + arc1);
    }

    visibility /= float(sliceCount);
    return max(0.03, pow(max(visibility, 0.0), gtao.finalValuePower));
}
