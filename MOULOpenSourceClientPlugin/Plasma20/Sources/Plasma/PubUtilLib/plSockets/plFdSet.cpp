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
#include "plFdSet.h"
#include "plSocket.h"

#if HS_BUILD_FOR_UNIX
#include <sys/time.h>
#endif

plFdSet::plFdSet(void)
{
	ZeroFds();
}


void plFdSet::SetForSocket(plSocket & in)
{
	SOCKET sck = in.GetSocket();
	hsAssert(sck>=0, "plFdSet::SetForSocket: socket<0");
	if ( sck<0 )
		return;
	FD_SET(sck, &fFds);
	FD_SET(sck, &fErrFds);
	if(fMaxFd < sck)
		fMaxFd = sck;
}

void plFdSet::ClearForSocket(plSocket & in)
{
	SOCKET sck = in.GetSocket();
	hsAssert(sck>=0, "plFdSet::ClearForSocket: socket<0");
	if ( sck<0 )
		return;
	FD_CLR(sck, &fFds);
	FD_CLR(sck, &fErrFds);
}


bool plFdSet::IsSetFor(plSocket & in)
{
	SOCKET sck = in.GetSocket();
	hsAssert(sck>=0, "plFdSet::IsSetFor: socket<0");
	if ( sck<0 )
		return false;
	return (FD_ISSET(sck, &fFds) != 0);
}

bool plFdSet::IsErrFor(plSocket & in)
{
	SOCKET sck = in.GetSocket();
	hsAssert(sck>=0, "plFdSet::IsErrFor: socket<0");
	if ( sck<0 )
		return false;
	return (FD_ISSET(sck, &fErrFds) != 0);
}


int plFdSet::WaitForRead(bool shouldZeroFds, unsigned long timeoutMillis)
{        
	int ret_val = 0;        

	if(timeoutMillis == kInfinite)                
	{
		ret_val = select(fMaxFd+1,&fFds,NULL,&fErrFds,NULL);
	}
	else
	{
		struct timeval tv;
		tv.tv_sec = timeoutMillis  / 1000;
		tv.tv_usec = (timeoutMillis % 1000) * 1000;
		
		ret_val = select(fMaxFd+1,&fFds,NULL,&fErrFds,&tv);
	}
	if (shouldZeroFds) 
		ZeroFds();
	
	return ret_val;
}


int plFdSet::WaitForWrite(bool shouldZeroFds, unsigned long timeoutMillis)
{        
	int ret_val = 0;        

	if(timeoutMillis == kInfinite)                
	{
		ret_val = select(fMaxFd+1,NULL,&fFds,&fErrFds,NULL);
	}
	else
	{
		timeval tv;
		tv.tv_sec = timeoutMillis  / 1000;
		tv.tv_usec = (timeoutMillis % 1000) * 1000;
		
		ret_val = select(fMaxFd+1,NULL,&fFds,&fErrFds,&tv);
	}
	if (shouldZeroFds) 
		ZeroFds();
	
	return ret_val;
}


int plFdSet::WaitForError(bool shouldZeroFds, unsigned long timeoutMillis)
{        
	int ret_val = 0;        

	if(timeoutMillis == kInfinite)                
	{
		ret_val = select(fMaxFd+1,NULL,NULL,&fErrFds,NULL);
	}
	else
	{
		timeval tv;
		tv.tv_sec = timeoutMillis  / 1000;
		tv.tv_usec = (timeoutMillis % 1000) * 1000;
		
		ret_val = select(fMaxFd+1,NULL,NULL,&fErrFds,&tv);
	}
	if (shouldZeroFds) 
		ZeroFds();
	
	return ret_val;
}


void plFdSet::ZeroFds(void)
{
	fMaxFd = 0;
	FD_ZERO(&fFds);
	FD_ZERO(&fErrFds);
}


