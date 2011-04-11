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
#include "plBufferedSocketWriter.h"
#include "plTcpSocket.h"

plBufferedSocketWriter::plBufferedSocketWriter(int size, int bytesPerFlush, bool blockOnSend, int flushPoint)
:	plRingBuffer(size)
,	fFlushPoint(flushPoint)
,	fBlockOnSend(blockOnSend)
,	fBytesPerFlush(bytesPerFlush)
{}


int plBufferedSocketWriter::WriteBlock(const char * data, int len, plTcpSocket & sck)
{
	int ans = kSuccessNoDataSent;
	
	if(len > BufferAvailable())
		ans = Flush(sck);
	
	if(ans >= 0)
		ans = WriteBlock(data,len);
	
	if(ans >= 0 && fFlushPoint >= 0 && fFlushPoint < AmountBuffered())
		ans = Flush(sck);
			
	return ans;
}

int plBufferedSocketWriter::WriteBlock(const char * data, int len)
{
	int ans = kSuccessNoDataSent;
	if(!Put(data,len))
		ans = kFailedNoBufferSpace;
	return ans;
}

int plBufferedSocketWriter::Flush(plTcpSocket & sck)	// this is where things get ugly.
{
	int ans = kSuccessNoDataSent;

	int writeSize = MIN(FastAmountBuffered(),fBytesPerFlush);
	
	if(writeSize > 0)
	{
		int nBytesWritten = 0;
//		int nBytesWritten = sck.SendData(FastGetBufferStart(), writeSize);

//		if (nBytesWritten<0 && fBlockOnSend
//			&& plNet::GetError()==kBlockingError)
		{
			bool wasBlockingOrNot = sck.IsBlocking();
			sck.SetBlocking(fBlockOnSend);
			nBytesWritten = sck.SendData(FastGetBufferStart(), writeSize);        
			sck.SetBlocking(wasBlockingOrNot);
		}

		if (nBytesWritten > 0)
		{
			fStartPos += nBytesWritten;
			FullCompress();
			ans = kSuccessDataSent;
		}
		else if (nBytesWritten < 0)
		{
			if (plNet::GetError()!=kBlockingError)
				ans = kFailedWriteError;
		}
		else
		{
			ans = kFailedSocketClosed;
		}
	}        
	return ans;
}

bool plBufferedSocketWriter::IsEmpty()
{
	return !(FastAmountBuffered() > 0);
}

void plBufferedSocketWriter::Reset()
{
	plRingBuffer::Reset();
}

