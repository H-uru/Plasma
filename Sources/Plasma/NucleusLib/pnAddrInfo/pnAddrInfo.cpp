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


#include "hsTypes.h"
#include "pnAddrInfo.h"
#include <string.h>
#if HS_BUILD_FOR_UNIX
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif

const pnAddrInfo* pnAddrInfo::GetInterface()
{
	static pnAddrInfo addrinfo;
	return &addrinfo;
}

struct addrinfo* pnAddrInfo::GetAddrByNameSimple(const char* name, const int family)
{
	struct addrinfo* info;
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = family;
	hints.ai_flags = AI_CANONNAME;
	if (Get(name,NULL,&hints,&info) != 0)
		info = NULL;
	return info;
};

int pnAddrInfo::Get(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
	return getaddrinfo(node,service,hints,res);
}

void pnAddrInfo::Free(struct addrinfo* res)
{
	freeaddrinfo(res);
}

const char* pnAddrInfo::GetErrorString(int errcode)
{
	return gai_strerror(errcode);
}

//////////////////////////////////
//// UNIX
//////////////////////////////////

#if HS_BUILD_FOR_UNIX
pnAddrInfo::pnAddrInfo()
{
}

pnAddrInfo::~pnAddrInfo()
{
}

void pnAddrInfo::GetLocalAddrs(List& addrslist, bool incLoopBack)
{
	struct ifconf info;
	struct ifreq infos[128];
	
	info.ifc_len = sizeof(infos);
	info.ifc_req = infos;
	
	int s = socket(AF_INET, SOCK_DGRAM, 0); 
	
	if (ioctl(s,SIOCGIFCONF,&info) == 0)
	{
		int i;
		int entries = info.ifc_len / sizeof(struct ifreq);
		
		for (i = 0; i < entries; i++)
		{
			struct in_addr addr = ((struct sockaddr_in *)&(infos[i].ifr_addr))->sin_addr;
			
			if (incLoopBack || (addr.s_addr != htonl(INADDR_LOOPBACK)))
			{
				addrslist.push_back(inet_ntoa(addr));
			}
		}
	}
	
	close(s);
}

#endif

////////////////////////////////////
//// Windows
////////////////////////////////////

#if HS_BUILD_FOR_WIN32
pnAddrInfo::pnAddrInfo()
{
	WSADATA data;
	WSAStartup(MAKEWORD( 2, 2 ), &data);
}

pnAddrInfo::~pnAddrInfo()
{
		WSACleanup();
}

void pnAddrInfo::GetLocalAddrs(List& addrslist, bool incLoopBack)
{
	INTERFACE_INFO infos[128];  // get at most 128 interfaces
	const unsigned int bufSize = sizeof(infos);
	DWORD retSize;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0); 

	if (WSAIoctl(s,SIO_GET_INTERFACE_LIST,NULL,NULL,(void*)infos,bufSize,&retSize,NULL,NULL) == 0)
	{
		unsigned int entries = retSize/sizeof(INTERFACE_INFO);
		int i;
		for (i = 0; i < entries; i++)
		{
			if (infos[i].iiFlags & IFF_UP)
			{
				struct in_addr addr = infos[i].iiAddress.AddressIn.sin_addr;
				if (incLoopBack || (addr.s_addr != htonl(INADDR_LOOPBACK)))
					addrslist.push_back(inet_ntoa(addr));
			}
		}
	}

	closesocket(s);
}
#endif


#endif // SERVER
