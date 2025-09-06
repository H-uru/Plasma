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

#include <string_theory/format>

#include "plNetAddress.h"

ST::string plNetAddress::GetIPv4AddressAsString(uint32_t ip4addr)
{
    static_assert(sizeof(ip4addr) == 4*sizeof(uint8_t));
    const auto* bytes = reinterpret_cast<const uint8_t*>(&ip4addr);
    return ST::format("{}.{}.{}.{}", bytes[0], bytes[1], bytes[2], bytes[3]);
}

ST::string plNetAddress::GetHostString() const
{
    return GetIPv4AddressAsString(fHost);
}

ST::string plNetAddress::GetHostWithPort() const
{
    return ST::format("{}:{}", GetIPv4AddressAsString(fHost), fPort);
}

std::array<uint8_t, 4> plNetAddress::GetHostBytes() const
{
    std::array<uint8_t, 4> bytes;
    static_assert(sizeof(fHost) == sizeof(bytes));
    memcpy(bytes.data(), &fHost, sizeof(fHost));
    return bytes;
}

void plNetAddress::SetHost(const std::array<uint8_t, 4>& addr)
{
    static_assert(sizeof(fHost) == sizeof(addr));
    memcpy(&fHost, addr.data(), addr.size());
}

ST::string plNetAddress::AsString() const
{
    return ST::format("IP:{}:{}", GetIPv4AddressAsString(fHost), fPort);
}

void plNetAddress::Read(hsStream * s)
{
    // No endianness conversion here - fHost is always big-endian in memory and in the stream!
    s->Read(sizeof(fHost), &fHost);
    fPort = s->ReadLE16();

    // Family is always kInet
    (void) s->ReadLE16();
}

void plNetAddress::Write(hsStream * s)
{
    // No endianness conversion here - fHost is always big-endian in memory and in the stream!
    s->Write(sizeof(fHost), &fHost);
    s->WriteLE16(fPort);

    s->WriteLE16(static_cast<uint16_t>(Family::kInet));
}
