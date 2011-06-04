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
#include "plTcpSocket.h"
#include "plFdSet.h"
#include "../pnNetCommon/plNetAddress.h"

#if HS_BUILD_FOR_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif


plTcpSocket::plTcpSocket()
{
}

plTcpSocket::plTcpSocket(SOCKET sck)
:	plSocket(sck)
{

}

bool plTcpSocket::operator==(const plTcpSocket & rhs)
{
	return fSocket==rhs.fSocket;
}

// Disable Nagle algorithm.
int plTcpSocket::SetNoDelay(void)
{
	int  nodel = 1;
	int ret1;    
	ret1 = setsockopt(fSocket, IPPROTO_TCP, TCP_NODELAY,(char *)&nodel, sizeof(nodel));
	
	if(ret1 != 0)
		return -1;
	
	return 0;
}


// Control the behavior of SO_LINGER
int plTcpSocket::SetLinger(int intervalSecs)
{
	linger  ll;        
	ll.l_linger = intervalSecs;
	ll.l_onoff = intervalSecs?1:0;
	int ret1 = setsockopt(fSocket, SOL_SOCKET, SO_LINGER, (const char *)&ll,sizeof(linger));     
	if(ret1 != 0)
		return -1;        
	return 0;
}


int plTcpSocket::SetSendBufferSize(int insize)
{
	if (setsockopt(fSocket, (int) SOL_SOCKET, (int) SO_SNDBUF, (char *) &insize, sizeof(int)))
		return -1;    
	return 0;
}

bool plTcpSocket::ActiveOpen(plNetAddress & addr)
{
	SetSocket(plNet::NewTCP());
	if(fSocket == kBadSocket)
		return false;
	
	if(plNet::Connect(fSocket, &addr.GetAddressInfo()) != 0)
		return ErrorClose();
	
	return true;
}


bool plTcpSocket::ActiveOpenNonBlocking(plNetAddress & addr)
{
	SetSocket(plNet::NewTCP());
	if(fSocket == kBadSocket)
		return false;

	SetBlocking(false);

	if(plNet::Connect(fSocket, &addr.GetAddressInfo()) != 0)
		return ErrorClose();
	
	return true;
}


// Returns:
//	- if error
//	0 if socket closed or size=0
//	+ bytes written
int plTcpSocket::SendData(const char * data, int size)
{
	return plNet::Write(fSocket,data,size);
}


// Returns:
//	- if error
//	0 if socket closed or size=0
//	+ bytes read
int plTcpSocket::RecvData(char * data, int len)
{
	plFdSet fdset;
	fdset.SetForSocket(*this);
	fdset.WaitForRead(false,fRecvTimeOut);
	if (fdset.IsErrFor(*this))
		return -1;
	if (fdset.IsSetFor(*this))
		return plNet::Read(fSocket,data,len);	// returns -1 on error
	else
		return -2;
};

