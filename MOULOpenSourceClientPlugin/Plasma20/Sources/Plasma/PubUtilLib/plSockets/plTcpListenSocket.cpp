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
#include "plTcpListenSocket.h"
#include "../pnNetCommon/plNetAddress.h"


// Initialize a socket for listening
bool plTcpListenSocket::OpenForListen(plNetAddress & inAddess, int backlogSize)
{        
	ErrorClose();
	SetSocket(plNet::NewTCP());
	
	SetReuseAddress();
	
	if(plNet::Bind(fSocket, &inAddess.GetAddressInfo()) != 0)
		return ErrorClose();
	
	if(plNet::Listen(fSocket, backlogSize) != 0)
		return ErrorClose();
	
	return true;
}


// Initialize a socket for listening. non blocking version
bool plTcpListenSocket::OpenForListenNonBlocking(plNetAddress & inAddess, int backlogSize)
{        
	ErrorClose();
	SetSocket(plNet::NewTCP());
	
	SetReuseAddress();
	SetBlocking(false);	// so GetIncomingConnection() won't block.
	
	if(plNet::Bind(fSocket, &inAddess.GetAddressInfo()) != 0)
		return ErrorClose();
	
	if(plNet::Listen(fSocket, backlogSize) != 0)
		return ErrorClose();
	
	return true;
}


// Accept an incoming connection
bool  plTcpListenSocket::GetIncomingConnection(SOCKET & outNewSession, plNetAddress & outAddress)
{        
	outNewSession = plNet::Accept(fSocket, &outAddress.GetAddressInfo());
	if(outNewSession == kBadSocket)
		return false;
	return true;
}

