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
#include "pnNetCommon.h"
#include "../pnAddrInfo/pnAddrInfo.h"
#ifdef HS_BUILD_FOR_WIN32
# include "hsWindows.h"
#elif HS_BUILD_FOR_UNIX
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <string.h>  // for memcpy
#else
#error "Not implemented for this platform"
#endif

namespace pnNetCommon
{

#ifndef SERVER

// NOTE: On Win32, WSAStartup() must be called before GetTextAddr() will work.
const char * GetTextAddr(UInt32 binAddr)
{
	in_addr in;
	memcpy(&in,&binAddr,sizeof(binAddr));
	return inet_ntoa(in);
}

// NOTE: On Win32, WSAStartup() must be called before GetBinAddr() will work.
UInt32 GetBinAddr(const char * textAddr)
{
	UInt32 addr = 0;
	
	if (!textAddr)
		return addr;
	
    struct addrinfo * ai = NULL;
    
	addr = inet_addr(textAddr);
	if(addr == INADDR_NONE)
	{
		ai = pnAddrInfo::GetAddrByNameSimple(textAddr);
		if(ai!= NULL)
			memcpy(&addr,(void*)(&(((sockaddr_in*)(ai->ai_addr))->sin_addr)),sizeof(addr));
		pnAddrInfo::Free(ai);
	}
	
	return addr;
}

#endif

} // pnNetCommon namespace



////////////////////////////////////////////////////////////////////

void plCreatableStream::Write( hsStream* stream, hsResMgr* mgr )
{
	fStream.Rewind();
	std::string buf;
	UInt32 len = fStream.GetEOF();
	stream->WriteSwap( len );
	buf.resize( len );
	fStream.Read( len, (void*)buf.data() );
	stream->Write( len, (const void*)buf.data() );
	fStream.Rewind();
}

void plCreatableStream::Read( hsStream* stream, hsResMgr* mgr )
{
	fStream.Rewind();
	std::string buf;
	UInt32 len;
	stream->LogReadSwap( &len,"CreatableStream Len");
	buf.resize( len );
	stream->LogRead( len, (void*)buf.data(),"CreatableStream Data");
	fStream.Write( len, (const void*)buf.data() );
	fStream.Rewind();
}

////////////////////////////////////////////////////////////////////
// End.
