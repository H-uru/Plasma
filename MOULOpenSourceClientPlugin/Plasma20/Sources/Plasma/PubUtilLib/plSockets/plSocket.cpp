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
#include "plSocket.h"

#if HS_BUILD_FOR_UNIX
#include <sys/ioctl.h>
#endif

/////////////////////////////////////////////////

plSocket::plSocket()
{
	fCloseOnDestroy	= false;
	fCloseBeforeSet	= false;
	fTimeoutsSet = 0;
	fSocket = kBadSocket;
}


plSocket::plSocket(SOCKET sck)
{
	fCloseOnDestroy	= false;
	fCloseBeforeSet	= false;
	fTimeoutsSet = 0;
	SetSocket(sck);
}

plSocket::~plSocket()
{
	if (fCloseOnDestroy)
		Close();
}

bool plSocket::operator==(const plSocket & rhs)
{
	return fSocket == rhs.fSocket;
}

bool plSocket::ErrorClose()
{
	if(Active())
		plNet::Close(fSocket);
	fSocket = kBadSocket;
	return false;
}


bool plSocket::Active() 
{ 
	return (fSocket != kBadSocket); 
}


void plSocket::Close()
{
	if(Active())
		plNet::Close(fSocket);
	fSocket = kBadSocket;
}


int plSocket::GetLastError()
{
	return plNet::GetError();
}


void plSocket::SetSocket(SOCKET sck)
{
	if (fSocket!=sck)
	{
		if (fCloseBeforeSet)
			Close();
		fSocket = sck;
		if (fTimeoutsSet & kRecvTimeoutSet)
			setsockopt(fSocket, SOL_SOCKET, (int)SO_RCVTIMEO,(const char*)&fRecvTimeOut,sizeof(fRecvTimeOut));

		if (fTimeoutsSet & kSendTimeoutSet)
			setsockopt(fSocket, SOL_SOCKET, (int)SO_SNDTIMEO,(const char*)&fSendTimeOut,sizeof(fSendTimeOut));

		IGetTimeOutsFromSocket();
	}
}


int plSocket::SetRecvBufferSize(int insize)
{
	if (setsockopt(fSocket, (int)SOL_SOCKET, (int)SO_RCVBUF, (char *)&insize, sizeof(int)))
		return 0;
	
	return -1;
}


void plSocket::SetBlocking(bool value)
{
	unsigned long val = value?0:1;
#ifdef BSDBLOCK
// NOTE: Bsd does this a little differently, using fcntl()
#error "BSDBLOCK: This code needs to be verified right now. Don't assume it will work at all."
	int flags = fcntl(fSocket,F_GETFL,val);
	flags = flags |O_NONBLOCK;
	fcntl(fSocket,F_SETFL,flags);        
#else
	plNet::Ioctl(fSocket,FIONBIO,&val);
#endif	
}


bool plSocket::IsBlocking()
{
#ifdef BSDBLOCK
#error "BSDBLOCK: TODO: IsBlocking() for bsd."
#else
	int ans = plNet::Ioctl(fSocket,FIONBIO,NULL);
	return (ans!=0);
#endif
}


void plSocket::SetReuseAddress()
{
	int opt = 1; 
	setsockopt(fSocket, SOL_SOCKET, SO_REUSEADDR,  (const char *)&opt, sizeof(opt));
}

int plSocket::SetSendTimeOut(unsigned int milliSecs/* =0 */)
{
	fTimeoutsSet |= kSendTimeoutSet;
	fSendTimeOut = milliSecs;

	if (fSocket != kBadSocket)
	{
		if (setsockopt(fSocket, SOL_SOCKET, (int)SO_SNDTIMEO,(const char*)&fSendTimeOut,sizeof(fSendTimeOut)) != 0)
			return -1;
	}

	return 0;
}

int plSocket::SetRecvTimeOut(unsigned int milliSecs/* =0 */)
{
	fTimeoutsSet |= kRecvTimeoutSet;
	fRecvTimeOut = milliSecs;

	if (fSocket != kBadSocket)
	{
		if (setsockopt(fSocket, SOL_SOCKET, (int)SO_RCVTIMEO,(const char*)&fRecvTimeOut,sizeof(fRecvTimeOut)) != 0)
			return -1;
	}

	return 0;
}

int plSocket::IGetTimeOutsFromSocket()
{
	unsigned int rtimeoutval, stimeoutval;

	if (fSocket != kBadSocket)
	{
		socklen_t sizeval;
		sizeval = sizeof(rtimeoutval);
		if (getsockopt(fSocket, SOL_SOCKET, (int)SO_RCVTIMEO,(char*)&rtimeoutval,&sizeval) != 0)
			return -1;
		sizeval = sizeof(stimeoutval);
		if (getsockopt(fSocket, SOL_SOCKET, (int)SO_SNDTIMEO,(char*)&stimeoutval,&sizeval) != 0)
			return -1;
		fRecvTimeOut = rtimeoutval;
		fSendTimeOut = stimeoutval;
	}
	return 0;
}