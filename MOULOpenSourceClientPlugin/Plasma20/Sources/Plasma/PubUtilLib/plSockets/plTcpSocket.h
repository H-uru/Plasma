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
#ifndef plTcpSocket_h_inc
#define plTcpSocket_h_inc

#include "plSocket.h"

class plNetAddress;


class plTcpSocket : public plSocket
{
public:
	plTcpSocket();
	plTcpSocket(SOCKET sck);
	bool operator==(const plTcpSocket & rhs);
	int SetNoDelay();
	int SetLinger(int intervalSecs=0);
	int SetSendBufferSize(int size);
	bool ActiveOpen(plNetAddress & addr);
	bool ActiveOpenNonBlocking(plNetAddress & addr);
	int SendData(const char * data, int size);
	int RecvData(char * data, int size);
	friend bool operator<(const plTcpSocket & lhs, const plTcpSocket & rhs);
	friend bool operator==(const plTcpSocket & lhs, const plTcpSocket & rhs);
};


inline bool operator<(const plTcpSocket & lhs, const plTcpSocket & rhs)
{
	return lhs.fSocket<rhs.fSocket;
}

inline bool operator==(const plTcpSocket & lhs, const plTcpSocket & rhs)
{
	return lhs.fSocket==rhs.fSocket;
}


#endif //plTcpSocket_h_inc
