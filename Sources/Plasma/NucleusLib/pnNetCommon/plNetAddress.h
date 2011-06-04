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

#ifndef plNetAddress_h_inc
#define plNetAddress_h_inc

#include "hsUtils.h"
#include "hsStlUtils.h"
#include "hsStream.h"

#if defined(HS_BUILD_FOR_WIN32)
#include "hsWindows.h"
#elif defined( HS_BUILD_FOR_UNIX )
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#error  "Must Include net stuff for this OS here"
#endif


#ifdef SetPort
#undef SetPort
#endif

typedef sockaddr_in  AddressType;

class plNetAddress
{
    // fAddr must be first field
	AddressType     fAddr;

public:
	plNetAddress();
	plNetAddress(UInt32 addr, int port);
	plNetAddress(const char * addr, int port);
	virtual ~plNetAddress(){}

	void Clear();
	bool SetAnyAddr();
	bool SetAnyPort();
	bool SetPort(int port);
	bool SetHost(const char * hostname);
	bool SetHost(UInt32 ip4addr);
	int GetPort() const;
	std::string GetHostString() const;
	UInt32 GetHost() const;
	std::string GetHostWithPort() const; 
	const AddressType & GetAddressInfo() const { return fAddr; }
	AddressType & GetAddressInfo() { return fAddr; }

	std::string AsString() const;

	void Read(hsStream * stream);
	void Write(hsStream * stream);
};



#endif // plNetAddress_h_inc
#endif // SERVER
