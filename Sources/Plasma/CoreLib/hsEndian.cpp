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

#include "hsEndian.h"

#include <string_theory/string>

ST::string hsSTStringFromUTF16LE(const void* buffer, size_t char16Count)
{
    auto byteBuffer = static_cast<const uint8_t*>(buffer);
    ST::utf16_buffer utf16Buffer;
    utf16Buffer.allocate(char16Count);
    for (size_t i = 0; i < char16Count; i++) {
        utf16Buffer[i] = byteBuffer[2*i] | byteBuffer[2*i + 1] << 8;
    }
    return ST::string::from_utf16(utf16Buffer);
}

ST::string hsSTStringFromTerminatedUTF16LE(const void* buffer, size_t bufferSize, size_t& consumedSize)
{
    auto byteBuffer = static_cast<const uint8_t*>(buffer);
    // Count how many char16_ts there are in the string.
    // char16Count only counts the actual string contents,
    // consumedChar16Count also counts the terminator, if present
    // (there is no terminator if the end of the buffer was reached unexpectedly).
    size_t consumedChar16Count = 0;
    size_t char16Count = 0;
    for (size_t i = 0; i < bufferSize / sizeof(char16_t); i++) {
        consumedChar16Count++;
        if (byteBuffer[2*i] == 0 && byteBuffer[2*i + 1] == 0) {
            break;
        }
        char16Count++;
    }
    consumedSize = consumedChar16Count * sizeof(char16_t);
    return hsSTStringFromUTF16LE(buffer, char16Count);
}

static void ICopyUTF16BufferToLE(const char16_t* utf16Buffer, size_t char16Count, uint8_t* utf16LEBuffer)
{
    for (size_t i = 0; i < char16Count; i++) {
        char16_t c = utf16Buffer[i];
        utf16LEBuffer[2*i] = c & 0xff;
        utf16LEBuffer[2*i + 1] = c >> 8 & 0xff;
    }
}

std::vector<uint8_t> hsSTStringToUTF16LE(const ST::string& string)
{
    ST::utf16_buffer utf16Buffer = string.to_utf16();
    std::vector<uint8_t> buffer;
    buffer.resize(utf16Buffer.size() * 2);
    ICopyUTF16BufferToLE(utf16Buffer.data(), utf16Buffer.size(), buffer.data());
    return buffer;
}

bool hsSTStringToFixedSizeUTF16LE(const ST::string& string, void* buffer, size_t bufferSize)
{
    ST::utf16_buffer utf16Buffer = string.to_utf16();

    // Calculate how many char16_ts fit into the provided buffer
    // while still leaving room for a terminating zero char16_t
    // (except if the buffer is shorter than 2 bytes, obviously).
    size_t char16CountToCopy;
    bool bufferIsBigEnough;
    if (bufferSize < (utf16Buffer.size() + 1) * 2) {
        char16CountToCopy = bufferSize / 2;
        if (char16CountToCopy > 0) {
            char16CountToCopy--;
        }
        bufferIsBigEnough = false;
    } else {
        char16CountToCopy = utf16Buffer.size();
        bufferIsBigEnough = true;
    }

    auto byteBuffer = static_cast<uint8_t*>(buffer);

    // Copy and convert the characters.
    ICopyUTF16BufferToLE(utf16Buffer.data(), char16CountToCopy, byteBuffer);

    // Fill the rest of the buffer with zeroes.
    // This terminates the string and ensures that no uninitialized data
    // is sent over the network, written to disk, or similar.
    for (size_t i = char16CountToCopy * 2; i < bufferSize; i++) {
        byteBuffer[i] = 0;
    }

    return bufferIsBigEnough;
}
