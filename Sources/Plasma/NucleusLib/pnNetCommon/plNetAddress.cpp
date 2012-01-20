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


plNetAddress::plNetAddress(uint32_t addr, int port)
{
    Clear();
    SetHost(addr);
    SetPort(port);
}


plNetAddress::plNetAddress(const char * addr, int port)
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


bool plNetAddress::SetPort(int port)
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


int plNetAddress::GetPort() const
{
    return ntohs(fAddr.sin_port);
}


std::string plNetAddress::GetHostString() const 
{
    return std::string(pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr));
}

uint32_t plNetAddress::GetHost() const
{
    return fAddr.sin_addr.s_addr;
}


std::string plNetAddress::GetHostWithPort() const 
{
    static const int buf_len = 1024;
    char buf[buf_len];
    sprintf(buf,"%s:%d",pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr),GetPort());
    return std::string(buf);
}


bool plNetAddress::SetHost(const char * hostname) 
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

std::string plNetAddress::AsString() const
{
    char buf[100] = "";
    sprintf(buf,"IP:%s:%d",pnNetCommon::GetTextAddr(fAddr.sin_addr.s_addr),GetPort());
    return std::string(buf);
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
