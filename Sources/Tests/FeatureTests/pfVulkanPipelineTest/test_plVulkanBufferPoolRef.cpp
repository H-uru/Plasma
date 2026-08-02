/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

*==LICENSE==*/

#include "pfVulkanPipeline/plVulkanDeviceRef.h"

#include <gtest/gtest.h>

namespace
{
class TestVulkanBufferPoolRef : public plVulkanBufferPoolRef
{
public:
    static void Advance(uint32_t frameTime, uint32_t& lastWriteFrameTime,
                        uint32_t& currentFrame, uint32_t& currentPass)
    {
        IAdvanceWriteCursor(frameTime, lastWriteFrameTime, currentFrame, currentPass);
    }
};
}

TEST(plVulkanBufferPoolRef, RotatesFrameAndResetsPass)
{
    uint32_t lastWriteFrameTime = 0;
    uint32_t currentFrame = 0;
    uint32_t currentPass = 0;

    TestVulkanBufferPoolRef::Advance(100, lastWriteFrameTime, currentFrame, currentPass);
    EXPECT_EQ(currentFrame, 1u);
    EXPECT_EQ(currentPass, 0u);

    TestVulkanBufferPoolRef::Advance(100, lastWriteFrameTime, currentFrame, currentPass);
    EXPECT_EQ(currentFrame, 1u);
    EXPECT_EQ(currentPass, 1u);

    TestVulkanBufferPoolRef::Advance(101, lastWriteFrameTime, currentFrame, currentPass);
    EXPECT_EQ(currentFrame, 2u);
    EXPECT_EQ(currentPass, 0u);

    TestVulkanBufferPoolRef::Advance(102, lastWriteFrameTime, currentFrame, currentPass);
    EXPECT_EQ(currentFrame, 0u);
    EXPECT_EQ(currentPass, 0u);
}
