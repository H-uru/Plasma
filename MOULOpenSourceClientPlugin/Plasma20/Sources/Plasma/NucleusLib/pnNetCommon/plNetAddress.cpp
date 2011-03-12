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


plNetAddress::plNetAddress(UInt32 addr, int port)
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

UInt32 plNetAddress::GetHost() const
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

bool plNetAddress::SetHost(UInt32 addr) 
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
	s->ReadSwap((UInt32*)&fAddr.sin_addr.s_addr);
	s->ReadSwap(&fAddr.sin_port);
	s->ReadSwap(&fAddr.sin_family);
}

void plNetAddress::Write(hsStream * s)
{
	s->WriteSwap((UInt32)fAddr.sin_addr.s_addr);
	s->WriteSwap(fAddr.sin_port);
	s->WriteSwap(fAddr.sin_family);
}


#endif
