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
#ifndef PN_ADDR_INFO_H
#define PN_ADDR_INFO_H

#ifndef SERVER

// A wrapper library for getaddrinfo
//  made to support Windows 98, Me, 2k
//  Unix and XP use native code

#if HS_BUILD_FOR_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#elif HS_BUILD_FOR_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#error "pnAddrInfo Not Implemented!"
#endif
#include "hsTypes.h"
#include "hsStlUtils.h"


class pnAddrInfo
{
public:
	typedef std::list<std::string> List;

	pnAddrInfo();
	~pnAddrInfo();

	static const pnAddrInfo* GetInterface();

	static struct addrinfo* GetAddrByNameSimple(const char* name, const int family = PF_INET);
	static int Get(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res);
	static const char* GetErrorString(int errcode);
	static void Free(struct addrinfo* res);

	static void GetLocalAddrs(List& addrslist, bool incLoopBack = false);
};

#endif // SERVER

#endif  // PN_ADDR_INFO_H
