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

#include <cstring>
#include <gtest/gtest.h>

#include "HeadSpin.h"
#include "hsBitVector.h"

TEST(hsBitVector, empty_vector)
{
    hsBitVector bv;

    EXPECT_EQ(bv.Empty(), true);
    EXPECT_EQ(bv.GetSize(), 0);
    EXPECT_EQ(bv.GetNumBitVectors(), 0);
}

TEST(hsBitVector, bit_setting)
{
    hsBitVector bv;

    bv.SetBit(1, true);

    EXPECT_EQ(bv.Empty(), false);
    EXPECT_EQ(bv.GetSize(), 32);
    EXPECT_EQ(bv.GetNumBitVectors(), 1);
    EXPECT_NE(bv.GetBitVector(0), 0);

    EXPECT_EQ(bv.IsBitSet(1), true);
    EXPECT_EQ(bv.IsBitSet(2), false);
    EXPECT_EQ(bv[1], true);
    EXPECT_EQ(bv[2], false);
}

TEST(hsBitVector, bit_setting_larger)
{
    hsBitVector bv;

    bv.SetBit(48, true);

    EXPECT_EQ(bv.Empty(), false);
    EXPECT_EQ(bv.GetSize(), 64);
    EXPECT_EQ(bv.GetNumBitVectors(), 2);
    EXPECT_EQ(bv.GetBitVector(0), 0);
    EXPECT_NE(bv.GetBitVector(1), 0);

    EXPECT_EQ(bv.IsBitSet(48), true);
    EXPECT_EQ(bv[48], true);
}

TEST(hsBitVector, bit_toggle)
{
    hsBitVector bv;

    EXPECT_EQ(bv.Empty(), true);
    EXPECT_EQ(bv.GetSize(), 0);
    EXPECT_EQ(bv.IsBitSet(1), false);

    bv.ToggleBit(1);

    EXPECT_EQ(bv.Empty(), false);
    EXPECT_EQ(bv.GetSize(), 32);
    EXPECT_EQ(bv.IsBitSet(1), true);

    bv.ToggleBit(1);

    EXPECT_EQ(bv.Empty(), true);
    EXPECT_EQ(bv.GetSize(), 32);
    EXPECT_EQ(bv.IsBitSet(1), false);
}

TEST(hsBitVector, bit_clearing)
{
    hsBitVector bv;

    bv.SetBit(1);
    bv.SetBit(2);
    bv.SetBit(3);

    EXPECT_EQ(bv.IsBitSet(1), true);
    EXPECT_EQ(bv.IsBitSet(2), true);
    EXPECT_EQ(bv.IsBitSet(3), true);

    bv.ClearBit(2);

    EXPECT_EQ(bv.IsBitSet(1), true);
    EXPECT_EQ(bv.IsBitSet(2), false);
    EXPECT_EQ(bv.IsBitSet(3), true);

    bv.Clear();

    EXPECT_EQ(bv.IsBitSet(1), false);
    EXPECT_EQ(bv.IsBitSet(2), false);
    EXPECT_EQ(bv.IsBitSet(3), false);
    EXPECT_EQ(bv.Empty(), true);
    EXPECT_EQ(bv.GetSize(), 32);

    bv.Reset();

    EXPECT_EQ(bv.IsBitSet(1), false);
    EXPECT_EQ(bv.IsBitSet(2), false);
    EXPECT_EQ(bv.IsBitSet(3), false);
    EXPECT_EQ(bv.Empty(), true);
    EXPECT_EQ(bv.GetSize(), 0);
}

TEST(hsBitVector, overlap)
{
    hsBitVector bv1, bv2;

    bv1.SetBit(1);
    bv1.SetBit(2);

    bv2.SetBit(1);
    bv2.SetBit(3);

    EXPECT_EQ(bv1.Overlap(bv2), true);

    bv2.ClearBit(1);

    EXPECT_EQ(bv1.Overlap(bv2), false);
}

TEST(hsBitVector, bitwise_and)
{
    hsBitVector bv1, bv2;

    bv1.SetBit(1);
    bv1.SetBit(2);

    bv2.SetBit(1);
    bv2.SetBit(3);

    hsBitVector bv = bv1 & bv2;

    EXPECT_EQ(bv[1], true);
    EXPECT_EQ(bv[2], false);
    EXPECT_EQ(bv[3], false);
}

TEST(hsBitVector, bitwise_or)
{
    hsBitVector bv1, bv2;

    bv1.SetBit(1);
    bv1.SetBit(2);

    bv2.SetBit(1);
    bv2.SetBit(3);

    hsBitVector bv = bv1 | bv2;

    EXPECT_EQ(bv[1], true);
    EXPECT_EQ(bv[2], true);
    EXPECT_EQ(bv[3], true);
}

TEST(hsBitVector, bitwise_xor)
{
    hsBitVector bv1, bv2;

    bv1.SetBit(1);
    bv1.SetBit(2);

    bv2.SetBit(1);
    bv2.SetBit(3);

    hsBitVector bv = bv1 ^ bv2;

    EXPECT_EQ(bv[1], false);
    EXPECT_EQ(bv[2], true);
    EXPECT_EQ(bv[3], true);
}

TEST(hsBitVector, subtraction)
{
    hsBitVector bv1, bv2;

    bv1.SetBit(1);
    bv1.SetBit(2);

    bv2.SetBit(1);
    bv2.SetBit(3);

    hsBitVector bv = bv1 - bv2;

    EXPECT_EQ(bv[1], false);
    EXPECT_EQ(bv[2], true);
    EXPECT_EQ(bv[3], false);
}

TEST(hsBitVector, compacting)
{
    hsBitVector bv;

    bv.SetBit(24, true);
    bv.SetBit(48, true);

    EXPECT_EQ(bv.Empty(), false);
    EXPECT_EQ(bv.GetSize(), 64);
    EXPECT_EQ(bv.GetNumBitVectors(), 2);

    bv.ClearBit(48);

    EXPECT_EQ(bv.GetSize(), 64);
    EXPECT_EQ(bv.GetNumBitVectors(), 2);

    bv.Compact();

    EXPECT_EQ(bv.GetSize(), 32);
    EXPECT_EQ(bv.GetNumBitVectors(), 1);

    bv.ClearBit(24);
    bv.Compact();

    EXPECT_EQ(bv.GetSize(), 0);
    EXPECT_EQ(bv.GetNumBitVectors(), 0);
}
