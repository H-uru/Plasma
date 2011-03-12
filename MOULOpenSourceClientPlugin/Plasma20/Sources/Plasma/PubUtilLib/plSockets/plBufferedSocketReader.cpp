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
#include "plBufferedSocketReader.h"
#include "plTcpSocket.h"
#include <string.h>


plBufferedSocketReader::plBufferedSocketReader(int size)
:	plRingBuffer(size)
{    
}

int plBufferedSocketReader::ReadBlock(char * buf, int buflen, plTcpSocket & sck)
{
	if(GetBlock(buf, buflen))
		return kSuccessWithData;
	
	int ans = ReadFrom(sck);
	if(ans<=0)
		return ans;
	
	if(GetBlock(buf, buflen))
		return kSuccessWithData;
	
	return kSuccessNoData;
}

int plBufferedSocketReader::ReadString(char * buf, int buflen, char * termChars, plTcpSocket & sck)
{
	if(GetString(buf, buflen, termChars))
		return kSuccessWithData;
	
	int ans = kSuccessNoData;

	while ( ans>=0 )
	{
		ans = ReadFrom(sck);
		if(ans>0)
		{
			if ( GetString(buf, buflen, termChars) )
				return kSuccessWithData;
		}
	}
	
	return ans;
}

int plBufferedSocketReader::ReadStringInPlace(char ** buf, char * termChars, plTcpSocket & sck)
{
	if(GetStringInPlace(buf, termChars))
		return kSuccessWithData;

	int ans = kSuccessNoData;

	while ( ans>=0 )
	{
		ans = ReadFrom(sck);
		if(ans>0)
		{
			if ( GetStringInPlace(buf, termChars) )
				return kSuccessWithData;
		}
	}
	
	return ans;
}

void plBufferedSocketReader::Reset() 
{
	plRingBuffer::Reset();
}

int plBufferedSocketReader::ReadFrom(plTcpSocket & sck)	// this is where things get ugly.
{        
	int ans = kSuccessNoData;
	int readSize = BufferAvailable();
	
	if(readSize < 1)
	{
		Compress();
		readSize = BufferAvailable();
	}
	
	if(readSize > 0)
	{
		char * dst = GetBufferOpen();
		int nBytesRead = sck.RecvData(dst, readSize);
		if(nBytesRead < 0)
		{
			int err = plNet::GetError(); 
			if(err != kBlockingError)
			{
				ans = kFailedReadError;
			}
		}
		else if(nBytesRead > 0)
		{
			fEndPos += nBytesRead;
			ans = kSuccessWithData;
		}
		else
		{
			ans = kFailedSocketClosed;
		}
	}        
	else
	{
		ans = kFailedNoBufferSpace;
	}
	return ans;
}


bool plBufferedSocketReader::GetBlock(char * buf, int buflen)
{
	int dataAvailable = FastAmountBuffered();
	int maxRead = buflen;
	if(maxRead > dataAvailable)
		maxRead = dataAvailable;

	if (maxRead==0)
		return false;
	
	char * wrk = FastGetBufferStart();
	memcpy(buf,FastGetBufferStart(),maxRead);

	return true;
}

bool plBufferedSocketReader::GetString(char * buf, int buflen, char * termChars)
{
	bool ans = false;
	int dataAvailable = FastAmountBuffered();
	int maxRead = buflen;
	if(maxRead > dataAvailable)
		maxRead = dataAvailable;
	
	char * wrk = FastGetBufferStart();
	for(int i=0; i<maxRead; i++)
	{
		if(strchr(termChars,wrk[i])!=0)
		{
			memcpy(buf,wrk,i);
			buf[i] = '\0';
			fStartPos += i+1;                
			Compress();
			ans = true;
			break;
		}            
	}
	return ans;
}

bool plBufferedSocketReader::GetStringInPlace(char ** buf, char * termChars)
{
	bool ans = false;
	int dataAvailable = FastAmountBuffered();
	
	*buf = FastGetBufferStart();
	for(int i=0; i<dataAvailable; i++)
	{
		if(strchr(termChars,(*buf)[i])!=0)
		{
			(*buf)[i] = '\0';
			fStartPos += i+1;                
			Compress();
			ans = true;
			break;
		}            
	}
	return ans;
}

