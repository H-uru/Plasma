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
#ifndef plSocket_h_inc
#define plSocket_h_inc

#include "plNet.h"

////////////////////////////////////////////////////
// Socket base class
//
//		plSocket
//			plTcpSocket
//			plTcpListenSocket
//			plIncomingUdpSocket		- TODO (simple to impl, no immediate need)
//			plOutgoingUdpSocket		- TODO ""
//			plConnectedOutgoingUdpSocket	- TODO ""
//		plFdSet
//

class plSocket
{
public:
	plSocket();
	plSocket(SOCKET sck);
	virtual ~plSocket();
	bool operator==(const plSocket & rhs);
	static int GetLastError();
	void Close();
	void SetBlocking(bool value);
	bool IsBlocking();
	void SetReuseAddress();
	bool Active();
	int SetRecvBufferSize(int size);
	void SetSocket(SOCKET sck);
	SOCKET GetSocket() const { return fSocket; }
	void CloseOnDestroy(bool value) { fCloseOnDestroy=value; }
	void CloseBeforeSet(bool value) { fCloseBeforeSet=value; }
	int SetSendTimeOut(unsigned int milliSecs=0);
	int SetRecvTimeOut(unsigned int milliSecs=0);

protected:
	enum TimeoutsSet
	{
		kRecvTimeoutSet = 1<<0,
		kSendTimeoutSet = 1<<1,
	};
	bool ErrorClose();
	SOCKET	fSocket;
	bool	fCloseOnDestroy;
	bool	fCloseBeforeSet;
	unsigned int fTimeoutsSet;
	unsigned int fRecvTimeOut;
	unsigned int fSendTimeOut;
	int IGetTimeOutsFromSocket();
};



#endif // plSocket_h_inc
