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
    // Can't pre-allocate anything, because we don't know the string length yet...
    std::vector<char16_t> utf16Buffer;
    for (size_t i = 0; i < bufferSize / sizeof(char16_t); i++) {
        char16_t c = byteBuffer[2*i] | byteBuffer[2*i + 1] << 8;
        if (c == 0) {
            break;
        }
        utf16Buffer.emplace_back(c);
    }
    // consumedSize includes the terminator, which is not stored in utf16Buffer
    consumedSize = (utf16Buffer.size() + 1) * sizeof(char16_t);
    return ST::string::from_utf16(utf16Buffer.data(), utf16Buffer.size());
}

std::vector<uint8_t> hsSTStringToUTF16LE(const ST::string& string)
{
    ST::utf16_buffer utf16Buffer = string.to_utf16();
    std::vector<uint8_t> buffer;
    buffer.resize(utf16Buffer.size() * 2);
    for (size_t i = 0; i < utf16Buffer.size(); i++) {
        char16_t c = utf16Buffer[i];
        buffer[2*i] = c & 0xff;
        buffer[2*i + 1] = c >> 8 & 0xff;
    }
    return buffer;
}
