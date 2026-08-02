/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.

*==LICENSE==*/

#ifndef plGTAO_inc
#define plGTAO_inc

#include <algorithm>
#include <cstdint>

/** Runtime quality presets from Intel's XeGTAO reference implementation. */
enum class plGTAOQuality : uint8_t
{
    kLow = 0,
    kMedium,
    kHigh,
    kUltra,
};

struct plGTAOSettings
{
    bool fEnabled = true;
    plGTAOQuality fQuality = plGTAOQuality::kUltra;
    float fRadius = 0.5f;
    float fPower = 3.0f;
};

/** Shared CPU/GPU constant layout used by both the Vulkan and Metal ports. */
struct plGTAOConstants
{
    int32_t fViewportSize[2];
    float fViewportPixelSize[2];
    float fDepthUnpackConsts[2];
    float fCameraTanHalfFOV[2];
    float fNDCToViewMul[2];
    float fNDCToViewAdd[2];
    float fNDCToViewMulPixelSize[2];
    float fEffectRadius;
    float fEffectFalloffRange;
    float fRadiusMultiplier;
    float fPadding0;
    float fFinalValuePower;
    float fDenoiseBlurBeta;
    float fSampleDistributionPower;
    float fThinOccluderCompensation;
    float fDepthMIPSamplingOffset;
    int32_t fNoiseIndex;
};

static_assert(sizeof(plGTAOConstants) == 96);

inline plGTAOSettings plClampGTAOSettings(plGTAOSettings settings)
{
    settings.fQuality = static_cast<plGTAOQuality>(std::clamp(
        static_cast<int>(settings.fQuality),
        static_cast<int>(plGTAOQuality::kLow),
        static_cast<int>(plGTAOQuality::kUltra)));
    settings.fRadius = std::clamp(settings.fRadius, 0.01f, 100.f);
    settings.fPower = std::clamp(settings.fPower, 0.5f, 5.f);
    return settings;
}

inline void plGTAOQualitySamples(plGTAOQuality quality, uint32_t& slices, uint32_t& steps)
{
    constexpr uint32_t kSlices[] = { 1, 2, 3, 9 };
    constexpr uint32_t kSteps[] = { 2, 2, 3, 3 };
    const uint32_t index = std::clamp(static_cast<uint32_t>(quality), 0u, 3u);
    slices = kSlices[index];
    steps = kSteps[index];
}

/** From XeGTAO's 64x64 Hilbert/R2 spatial-noise path. */
inline uint32_t plGTAOHilbertIndex(uint32_t x, uint32_t y)
{
    constexpr uint32_t kWidth = 64;
    uint32_t index = 0;
    for (uint32_t level = kWidth / 2; level > 0; level /= 2) {
        const uint32_t regionX = (x & level) != 0;
        const uint32_t regionY = (y & level) != 0;
        index += level * level * ((3u * regionX) ^ regionY);
        if (regionY == 0) {
            if (regionX == 1) {
                x = (kWidth - 1) - x;
                y = (kWidth - 1) - y;
            }
            std::swap(x, y);
        }
    }
    return index;
}

/**
 * Updates the constants from Plasma's row-vector projection matrix.
 *
 * This is the upstream XeGTAO constant setup with temporal noise deliberately
 * fixed at zero because Plasma does not currently have TAA.
 */
inline bool plUpdateGTAOConstants(plGTAOConstants& out, uint32_t width, uint32_t height,
                                  const plGTAOSettings& rawSettings,
                                  const float projection[16])
{
    if (width == 0 || height == 0 || projection[0] == 0.f || projection[5] == 0.f)
        return false;

    const plGTAOSettings settings = plClampGTAOSettings(rawSettings);
    out.fViewportSize[0] = static_cast<int32_t>(width);
    out.fViewportSize[1] = static_cast<int32_t>(height);
    out.fViewportPixelSize[0] = 1.f / static_cast<float>(width);
    out.fViewportPixelSize[1] = 1.f / static_cast<float>(height);

    // Plasma copies hsMatrix44's row-major projection directly into the shader
    // matrix storage. The perspective marker is therefore at [14], while the
    // view-depth numerator (cameraToNDC[2][3]) is at [11]. XeGTAO's upstream
    // column-vector extraction uses [14] for the latter, which is not valid for
    // Plasma's row-vector shader convention.
    if (projection[14] == 0.f)
        return false;

    float depthMul = -projection[11];
    float depthAdd = projection[10];
    if (depthMul * depthAdd < 0.f)
        depthAdd = -depthAdd;
    if (depthMul == 0.f)
        return false;

    out.fDepthUnpackConsts[0] = depthMul;
    out.fDepthUnpackConsts[1] = depthAdd;
    out.fCameraTanHalfFOV[0] = 1.f / projection[0];
    out.fCameraTanHalfFOV[1] = 1.f / projection[5];
    out.fNDCToViewMul[0] = out.fCameraTanHalfFOV[0] * 2.f;
    out.fNDCToViewMul[1] = out.fCameraTanHalfFOV[1] * -2.f;
    out.fNDCToViewAdd[0] = -out.fCameraTanHalfFOV[0];
    out.fNDCToViewAdd[1] = out.fCameraTanHalfFOV[1];
    out.fNDCToViewMulPixelSize[0] = out.fNDCToViewMul[0] * out.fViewportPixelSize[0];
    out.fNDCToViewMulPixelSize[1] = out.fNDCToViewMul[1] * out.fViewportPixelSize[1];

    out.fEffectRadius = settings.fRadius;
    out.fEffectFalloffRange = 0.615f;
    out.fRadiusMultiplier = 1.457f;
    out.fPadding0 = 0.f;
    out.fFinalValuePower = settings.fPower;
    out.fDenoiseBlurBeta = 1.2f;
    out.fSampleDistributionPower = 2.f;
    out.fThinOccluderCompensation = 0.f;
    out.fDepthMIPSamplingOffset = 3.30f;
    out.fNoiseIndex = 0;
    return true;
}

#endif // plGTAO_inc
