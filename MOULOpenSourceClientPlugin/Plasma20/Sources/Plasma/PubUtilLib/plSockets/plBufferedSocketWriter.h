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
#ifndef plBufferedSocketWriter_h_inc
#define plBufferedSocketWriter_h_inc

#include "plRingBuffer.h"

class plTcpSocket;


class plBufferedSocketWriter : protected plRingBuffer
{
protected:
	bool	fBlockOnSend;
	int		fFlushPoint;
	int		fBytesPerFlush;

public:
	enum WriteResult
	{
		kSuccessNoDataSent		= 0,
		kSuccessDataSent		= 1,
		kFailedWriteError		=-1,
		kFailedSocketClosed		=-2,
		kFailedNoBufferSpace	=-3,
	};

public:
	plBufferedSocketWriter(int size=8192, int bytesPerFlush=4096, bool blockOnSend=true, int flushPoint=-1);
	int WriteBlock(const char * data, int len, plTcpSocket & sck);
	int WriteBlock(const char * data, int len);
	int Flush(plTcpSocket & sck);
	bool IsEmpty();
	void Reset();
};



#endif // plBufferedSocketWriter_h_inc
