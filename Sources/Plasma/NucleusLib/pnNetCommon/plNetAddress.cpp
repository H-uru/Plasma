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
#ifndef SERVER

#include "plNetAddress.h"
#include "pnNetCommon.h"


plNetAddress::plNetAddress()
{
    Clear();
}

plNetAddress::plNetAddress(uint32_t addr, uint16_t port)
{
    Clear();
    SetHost(addr);
    SetPort(port);
}

plNetAddress::plNetAddress(const ST::string& addr, uint16_t port)
{
    Clear();
    SetHost(addr);
    SetPort(port);
}

bool plNetAddress::SetAnyAddr()
{
    fAddr.sin_family = AF_INET;
    fAddr.sin_addr.s_addr = INADDR_ANY;
    return true;
}

bool plNetAddress::SetAnyPort()
{
    fAddr.sin_family = AF_INET;
    fAddr.sin_port = htons(0);
    return true;
}

bool plNetAddress::SetPort(uint16_t port)
{
    fAddr.sin_family = AF_INET;
    fAddr.sin_port = htons(port);
    return true;
}

void plNetAddress::Clear()
{
    memset(&fAddr,0,sizeof(fAddr));
    fAddr.sin_family = AF_INET;
    fAddr.sin_addr.s_addr = INADDR_ANY;
}

uint16_t plNetAddress::GetPort() const
{
    return ntohs(fAddr.sin_port);
}

ST::string plNetAddress::GetHostString() const
{
    return pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr);
}

uint32_t plNetAddress::GetHost() const
{
    return fAddr.sin_addr.s_addr;
}

ST::string plNetAddress::GetHostWithPort() const
{
    ST::string_stream ss;
    ss << pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr) << ":" << GetPort();
    return ss.to_string();
}

bool plNetAddress::SetHost(const ST::string& hostname)
{
    fAddr.sin_addr.s_addr = pnNetCommon::GetBinAddr(hostname);
    fAddr.sin_family = AF_INET;
    return true;
}

bool plNetAddress::SetHost(uint32_t addr)
{
    memcpy(&fAddr.sin_addr, &addr,sizeof(addr));
    fAddr.sin_family = AF_INET;
    return true;
}

ST::string plNetAddress::AsString() const
{
    ST::string_stream ss;
    ss << "IP:" << pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr);
    ss << ":" << GetPort();

    return ss.to_string();
}

void plNetAddress::Read(hsStream * s)
{
    s->ReadLE((uint32_t*)&fAddr.sin_addr.s_addr);
    s->ReadLE(&fAddr.sin_port);
    s->ReadLE(&fAddr.sin_family);
}

void plNetAddress::Write(hsStream * s)
{
    s->WriteLE((uint32_t)fAddr.sin_addr.s_addr);
    s->WriteLE(fAddr.sin_port);
    s->WriteLE(fAddr.sin_family);
}


#endif
