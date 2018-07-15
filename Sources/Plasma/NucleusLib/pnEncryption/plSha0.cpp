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
#include "plSha0.h"

#include "HeadSpin.h"
#include <functional>
#include <cstring>

void plSha0::Start()
{
    fHash[0] = 0x67452301;
    fHash[1] = 0xefcdab89;
    fHash[2] = 0x98badcfe;
    fHash[3] = 0x10325476;
    fHash[4] = 0xc3d2e1f0;

    fIndex = 0;
    fSize = 0;
}

void plSha0::AddTo(size_t size, const uint8_t* buffer)
{
    while (size > 0) {
        size_t count = size;
        if (fIndex + count > sizeof(fData))
            count = sizeof(fData) - fIndex;
        memcpy(fData + fIndex, buffer, count);
        fIndex += count;
        fSize += count;
        buffer += count;
        size -= count;

        if (fIndex == sizeof(fData)) {
            ProcessChunk();
            fIndex = 0;
        }
    }
}

void plSha0::Finish(uint8_t* digest)
{
    fData[fIndex++] = 0x80;   // Append '1' bit to the end of the message
    if (fIndex == sizeof(fData)) {
        ProcessChunk();
        fIndex = 0;
    }

    if (fIndex + sizeof(uint64_t) > sizeof(fData)) {
        // Not enough space to write the size -- we need another chunk.
        memset(fData + fIndex, 0, sizeof(fData) - fIndex);
        ProcessChunk();
        fIndex = 0;
    }

    memset(fData + fIndex, 0, sizeof(fData) - fIndex);
    uint64_t msg_size_bits = hsToBE64(static_cast<uint64_t>(fSize) * 8);
    memcpy(fData + sizeof(fData) - sizeof(msg_size_bits),
           &msg_size_bits, sizeof(msg_size_bits));
    ProcessChunk();

    // Bring the output back to host endian
    for (size_t i = 0; i < 5; ++i)
        fHash[i] = hsToBE32(fHash[i]);

    memcpy(digest, fHash, sizeof(fHash));
}

uint32_t rol32(uint32_t value, unsigned int n)
{
    // Many compilers will optimize this to a single rol instruction on x86
    return (value << n) | (value >> (32 - n));
}

void plSha0::ProcessChunk()
{
    uint32_t work[80];
    memcpy(work, fData, sizeof(fData));

    for (size_t i = 0; i < 16; ++i)
        work[i] = hsToBE32(work[i]);

    for (size_t i = 16; i < 80; ++i) {
        // SHA-1 difference: no rol32(work[i], 1)
        work[i] = work[i-3] ^ work[i-8] ^ work[i-14] ^ work[i-16];
    }

    uint32_t hv[5];
    memcpy(hv, fHash, sizeof(hv));

    // Main SHA loop
    for (size_t i = 0; i < 20; ++i) {
        static const uint32_t K = 0x5a827999;
        const uint32_t f = (hv[1] & hv[2]) | (~hv[1] & hv[3]);
        const uint32_t temp = rol32(hv[0], 5) + f + hv[4] + K + work[i];
        hv[4] = hv[3];
        hv[3] = hv[2];
        hv[2] = rol32(hv[1], 30);
        hv[1] = hv[0];
        hv[0] = temp;
    }
    for (size_t i = 20; i < 40; ++i) {
        static const uint32_t K = 0x6ed9eba1;
        const uint32_t f = (hv[1] ^ hv[2] ^ hv[3]);
        const uint32_t temp = rol32(hv[0], 5) + f + hv[4] + K + work[i];
        hv[4] = hv[3];
        hv[3] = hv[2];
        hv[2] = rol32(hv[1], 30);
        hv[1] = hv[0];
        hv[0] = temp;
    }
    for (size_t i = 40; i < 60; ++i) {
        static const uint32_t K = 0x8f1bbcdc;
        const uint32_t f = (hv[1] & hv[2]) | (hv[1] & hv[3]) | (hv[2] & hv[3]);
        const uint32_t temp = rol32(hv[0], 5) + f + hv[4] + K + work[i];
        hv[4] = hv[3];
        hv[3] = hv[2];
        hv[2] = rol32(hv[1], 30);
        hv[1] = hv[0];
        hv[0] = temp;
    }
    for (size_t i = 60; i < 80; ++i) {
        static const uint32_t K = 0xca62c1d6;
        const uint32_t f = (hv[1] ^ hv[2] ^ hv[3]);
        const uint32_t temp = rol32(hv[0], 5) + f + hv[4] + K + work[i];
        hv[4] = hv[3];
        hv[3] = hv[2];
        hv[2] = rol32(hv[1], 30);
        hv[1] = hv[0];
        hv[0] = temp;
    }

    fHash[0] += hv[0];
    fHash[1] += hv[1];
    fHash[2] += hv[2];
    fHash[3] += hv[3];
    fHash[4] += hv[4];
}
