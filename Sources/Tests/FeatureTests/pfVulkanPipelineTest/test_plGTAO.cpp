/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

*==LICENSE==*/

#include "plGTAO.h"

#include <gtest/gtest.h>

#include <array>

TEST(plGTAO, ClampsUserSettings)
{
    plGTAOSettings settings;
    settings.fQuality = static_cast<plGTAOQuality>(99);
    settings.fRadius = -10.f;
    settings.fPower = 20.f;

    settings = plClampGTAOSettings(settings);
    EXPECT_EQ(settings.fQuality, plGTAOQuality::kUltra);
    EXPECT_FLOAT_EQ(settings.fRadius, 0.01f);
    EXPECT_FLOAT_EQ(settings.fPower, 5.f);
}

TEST(plGTAO, MapsQualityPresetsToXeGTAOSamples)
{
    const uint32_t expectedSlices[] = { 1, 2, 3, 9 };
    const uint32_t expectedSteps[] = { 2, 2, 3, 3 };
    for (uint32_t quality = 0; quality < 4; ++quality) {
        uint32_t slices = 0;
        uint32_t steps = 0;
        plGTAOQualitySamples(static_cast<plGTAOQuality>(quality), slices, steps);
        EXPECT_EQ(slices, expectedSlices[quality]);
        EXPECT_EQ(steps, expectedSteps[quality]);
    }
}

TEST(plGTAO, HilbertNoiseTableIsAPermutation)
{
    std::array<bool, 64 * 64> seen{};
    for (uint32_t y = 0; y < 64; ++y) {
        for (uint32_t x = 0; x < 64; ++x) {
            const uint32_t index = plGTAOHilbertIndex(x, y);
            ASSERT_LT(index, seen.size());
            EXPECT_FALSE(seen[index]);
            seen[index] = true;
        }
    }
    for (bool value : seen)
        EXPECT_TRUE(value);
}

TEST(plGTAO, BuildsPerspectiveConstants)
{
    float projection[16]{};
    projection[0] = 1.f;
    projection[5] = 2.f;
    projection[10] = 1.001f;
    projection[11] = -1.001f;
    projection[14] = 1.f;

    plGTAOSettings settings;
    settings.fRadius = 0.75f;
    settings.fPower = 1.8f;
    plGTAOConstants constants{};
    ASSERT_TRUE(plUpdateGTAOConstants(constants, 1920, 1080, settings, projection));

    EXPECT_EQ(constants.fViewportSize[0], 1920);
    EXPECT_EQ(constants.fViewportSize[1], 1080);
    EXPECT_FLOAT_EQ(constants.fCameraTanHalfFOV[0], 1.f);
    EXPECT_FLOAT_EQ(constants.fCameraTanHalfFOV[1], 0.5f);
    EXPECT_FLOAT_EQ(constants.fEffectRadius, 0.75f);
    EXPECT_FLOAT_EQ(constants.fFinalValuePower, 1.8f);
    EXPECT_FLOAT_EQ(constants.fDepthUnpackConsts[0], 1.001f);
    EXPECT_FLOAT_EQ(constants.fDepthUnpackConsts[1], 1.001f);
    EXPECT_EQ(constants.fNoiseIndex, 0);
}

TEST(plGTAO, ReconstructsPlasmaViewDepth)
{
    constexpr float nearPlane = 1.f;
    constexpr float farPlane = 32768.f;
    constexpr float depthScale = farPlane / (farPlane - nearPlane);

    float projection[16]{};
    projection[0] = projection[5] = 1.f;
    projection[10] = depthScale;
    projection[11] = -depthScale * nearPlane;
    projection[14] = 1.f;

    plGTAOConstants constants{};
    ASSERT_TRUE(plUpdateGTAOConstants(constants, 1920, 1080, {}, projection));

    auto reconstruct = [&constants](float deviceDepth) {
        return constants.fDepthUnpackConsts[0] /
               (constants.fDepthUnpackConsts[1] - deviceDepth);
    };
    EXPECT_NEAR(reconstruct(0.f), nearPlane, 1e-5f);
    EXPECT_NEAR(reconstruct(1.f), farPlane, 64.f);
}

TEST(plGTAO, RejectsOrthographicProjection)
{
    float projection[16]{};
    projection[0] = projection[5] = projection[10] = projection[15] = 1.f;
    plGTAOConstants constants{};
    EXPECT_FALSE(plUpdateGTAOConstants(constants, 640, 480, {}, projection));
}
