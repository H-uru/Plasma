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
#include "plNet.h"

////////////////////////////////////////////////////

plNet	plNet::_;

////////////////////////////////////////////////////
// Windows socket interface
#if HS_BUILD_FOR_WIN32

plNet::plNet()
{        
	static struct WSAData wsa;
	WSAStartup(0x0101, &wsa);
}

plNet::~plNet()
{
	WSACleanup();
}

SOCKET plNet::NewUDP()
{
	return ::socket(AF_INET, SOCK_DGRAM, 0);
}

SOCKET plNet::NewTCP()
{
	SOCKET x = ::socket(AF_INET, SOCK_STREAM, 0);
	unsigned int timeoutval;
	timeoutval = kDefaultSocketTimeout;
	setsockopt(x, SOL_SOCKET, (int)SO_RCVTIMEO,(const char*)&timeoutval,sizeof(timeoutval));
	timeoutval = kDefaultSocketTimeout;
	setsockopt(x, SOL_SOCKET, (int)SO_SNDTIMEO,(const char*)&timeoutval,sizeof(timeoutval));
	return x;
}

int plNet::GetError()
{
	return WSAGetLastError();
}

int plNet::Read(const SOCKET sck, char * buf, const int size)
{
	return ::recv(sck,buf,size,0);
}

int plNet::Write(const SOCKET sck, const char * buf, const int len)
{
	return ::send(sck,buf,len,0);
}

int plNet::ReadFrom(const SOCKET sck, char * buf, int len, sockaddr_in * addr)
{
	int addrlen = sizeof(sockaddr);
	return ::recvfrom(sck,buf,len,0,reinterpret_cast<sockaddr*>(addr),&addrlen);
}

int plNet::WriteTo(const SOCKET sck, const char * buf, const int len, sockaddr_in * addr)
{
	return ::sendto(sck,buf,len,0,reinterpret_cast<const sockaddr*>(addr),sizeof(sockaddr));
}

int plNet::Connect(const SOCKET sck, const sockaddr_in * addr)
{
	return ::connect(sck, reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr));
}

int plNet::Close(const SOCKET sck)
{
	return ::closesocket(sck);
}

int plNet::Bind(const SOCKET sck, const sockaddr_in * addr)
{
	return ::bind(sck,reinterpret_cast<const sockaddr*>(addr),sizeof(sockaddr));
}

int plNet::Listen(const SOCKET sck, const int qsize)
{
	return ::listen(sck,qsize);
}

int plNet::Accept(const SOCKET sck, sockaddr_in * addr)
{
	int addrlen = sizeof(sockaddr);
	return ::accept(sck,reinterpret_cast<sockaddr*>(addr),&addrlen);
}

int plNet::Ioctl(const SOCKET sck, const long flags, unsigned long * val)
{
	return ::ioctlsocket(sck,flags,val);
}

// static
const char * plNet::GetErrorMsg(int error)
{
	switch(error)
	{
		case WSAEINTR:              return "Interrupted system call"; break;
		case WSAEBADF:              return "Bad file number"; break;
		case WSAEACCES:             return "Permission denied"; break;
		case WSAEFAULT:             return "Bad address"; break;
		case WSAEINVAL:             return "Invalid argument"; break;
		case WSAEMFILE:             return "Too many open sockets"; break;
		case WSAEWOULDBLOCK:        return "Operation would block"; break;
		case WSAEINPROGRESS:        return "Operation now in progress"; break;
		case WSAEALREADY:           return "Operation already in progress"; break;
		case WSAENOTSOCK:           return "Socket operation on non-socket"; break;
		case WSAEDESTADDRREQ:       return "Destination address required"; break;
		case WSAEMSGSIZE:           return "Message too long"; break;
		case WSAEPROTOTYPE:         return "Protocol wrong type for socket"; break;
		case WSAENOPROTOOPT:        return "Bad protocol option"; break;
		case WSAEPROTONOSUPPORT:    return "Protocol not supported"; break;
		case WSAESOCKTNOSUPPORT:    return "Socket type not supported"; break;
		case WSAEOPNOTSUPP:         return "Operation not supported on socket"; break;
		case WSAEPFNOSUPPORT:       return "Protocol family not supported"; break;
		case WSAEAFNOSUPPORT:       return "Address family not supported"; break;
		case WSAEADDRINUSE:         return "Address already in use"; break;
		case WSAEADDRNOTAVAIL:      return "Can't assign requested address"; break;
		case WSAENETDOWN:           return "Network is down"; break;
		case WSAENETUNREACH:        return "Network is unreachable"; break;
		case WSAENETRESET:          return "Net connection reset"; break;
		case WSAECONNABORTED:       return "Software caused connection abort"; break;
		case WSAECONNRESET:         return "Connection reset by peer"; break;
		case WSAENOBUFS:            return "No buffer space available"; break;
		case WSAEISCONN:            return "Socket is already connected"; break;
		case WSAENOTCONN:           return "Socket is not connected"; break;
		case WSAESHUTDOWN:          return "Can't send after socket shutdown"; break;
		case WSAETOOMANYREFS:       return "Too many references, can't splice"; break;
		case WSAETIMEDOUT:          return "Connection timed out"; break;
		case WSAECONNREFUSED:       return "Connection refused"; break;
		case WSAELOOP:              return "Too many levels of symbolic links"; break;
		case WSAENAMETOOLONG:       return "File name too long"; break;
		case WSAEHOSTDOWN:          return "Host is down"; break;
		case WSAEHOSTUNREACH:       return "No route to host"; break;
		case WSAENOTEMPTY:          return "Directory not empty"; break;
		case WSAEPROCLIM:           return "Too many processes"; break;
		case WSAEUSERS:             return "Too many users"; break;
		case WSAEDQUOT:             return "Disc quota exceeded"; break;
		case WSAESTALE:             return "Stale NFS file handle"; break;
		case WSAEREMOTE:            return "Too many levels of remote in path"; break;
		case WSASYSNOTREADY:        return "Network subsystem is unavailable"; break;
		case WSAVERNOTSUPPORTED:    return "Winsock version not supported"; break;
		case WSANOTINITIALISED:     return "Winsock not yet initialized"; break;
		case WSAHOST_NOT_FOUND:     return "Host not found"; break;
		case WSATRY_AGAIN:          return "Non-authoritative host not found"; break;
		case WSANO_RECOVERY:        return "Non-recoverable errors"; break;
		case WSANO_DATA:            return "Valid name, no data record of requested type"; break;
		case WSAEDISCON:            return "Graceful disconnect in progress"; break;
		case WSASYSCALLFAILURE:     return "System call failure"; break;
		case WSA_NOT_ENOUGH_MEMORY: return "Insufficient memory available"; break;
		case WSA_OPERATION_ABORTED: return "Overlapped operation aborted"; break;
		case WSA_IO_INCOMPLETE:  	 return "Overlapped I/O object not signalled"; break;
		case WSA_IO_PENDING:        return "Overlapped I/O will complete later"; break;
		//case WSAINVALIDPROCTABLE:   return "Invalid proc. table from service provider"; break;
		//case WSAINVALIDPROVIDER:    return "Invalid service provider version number"; break;
		//case WSAPROVIDERFAILEDINIT, return "Unable to init service provider"; break;
		case WSA_INVALID_PARAMETER: return "One or more parameters are invalid"; break;
		case WSA_INVALID_HANDLE:    return "Event object handle not valid"; break;
	};
	return "\0";
}

////////////////////////////////////////////////////
// UNIX socket interface
#elif HS_BUILD_FOR_UNIX

#include <unistd.h>

plNet::plNet()
{ }

plNet::~plNet()
{ }

SOCKET plNet::NewUDP()
{
	return ::socket(AF_INET, SOCK_DGRAM, 0);
}

SOCKET plNet::NewTCP()
{
	SOCKET x = ::socket(AF_INET, SOCK_STREAM, 0);
	unsigned int timeoutval;
	timeoutval = kDefaultSocketTimeout;
	setsockopt(x, SOL_SOCKET, (int)SO_RCVTIMEO,(const char*)&timeoutval,sizeof(timeoutval));
	timeoutval = kDefaultSocketTimeout;
	setsockopt(x, SOL_SOCKET, (int)SO_SNDTIMEO,(const char*)&timeoutval,sizeof(timeoutval));
	return x;
}

int plNet::GetError()
{
	return errno;
}

int plNet::Read(const SOCKET sck, char * buf, const int size)
{
	return ::recv(sck,buf,size,0);
}

int plNet::Write(const SOCKET sck, const char * buf, const int len)
{
	return ::send(sck,buf,len,0);
}

int plNet::ReadFrom(const SOCKET sck, char * buf, int len, sockaddr_in * addr)
{
	unsigned addrlen = sizeof(sockaddr);
	return ::recvfrom(sck,buf,len,0,reinterpret_cast<sockaddr*>(addr),&addrlen);
}

int plNet::WriteTo(const SOCKET sck, const char * buf, const int len, sockaddr_in * addr)
{
	return ::sendto(sck,buf,len,0,reinterpret_cast<const sockaddr*>(addr),sizeof(sockaddr));
}

int plNet::Connect(const SOCKET sck, const sockaddr_in * addr)
{
	return ::connect(sck, reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr));
}

int plNet::Close(const SOCKET sck)
{
	return ::close(sck);
}

int plNet::Bind(const SOCKET sck, const sockaddr_in * addr)
{
	return ::bind(sck,reinterpret_cast<const sockaddr*>(addr),sizeof(sockaddr));
}

int plNet::Listen(const SOCKET sck, const int qsize)
{
	return ::listen(sck,qsize);
}

int plNet::Accept(const SOCKET sck, sockaddr_in * addr)
{
	unsigned addrlen = sizeof(sockaddr);
	return ::accept(sck,reinterpret_cast<sockaddr*>(addr),&addrlen);
}

int plNet::Ioctl(const SOCKET sck, const long flags, unsigned long * val)
{
	return ::ioctl(sck,flags,val);
}

#else
#error "plNet: Sockets not ported"
#endif

