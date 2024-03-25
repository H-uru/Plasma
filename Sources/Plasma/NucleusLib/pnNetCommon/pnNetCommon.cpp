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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include <string>
#include "pnNetCommon.h"

#if defined(HS_BUILD_FOR_UNIX)
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#elif defined(HS_BUILD_FOR_WIN32)
#include "hsWindows.h"
#include <ws2tcpip.h>
#else
#error "Not implemented for this platform"
#endif

namespace pnNetCommon
{

// NOTE: On Win32, WSAStartup() must be called before GetTextAddr() will work.
ST::string GetTextAddr(uint32_t binAddr)
{
    in_addr in;
    in.s_addr = binAddr;

    char text_addr[INET_ADDRSTRLEN];
    return ST::string::from_utf8(inet_ntop(AF_INET, &in, text_addr, sizeof(text_addr)));
}

// NOTE: On Win32, WSAStartup() must be called before GetBinAddr() will work.
uint32_t GetBinAddr(const ST::string& textAddr)
{
    uint32_t addr = 0;
    if (textAddr.empty())
        return addr;

    in_addr in;
    int result = inet_pton(AF_INET, textAddr.c_str(), &in);
    hsAssert(result >= 0, "inet_pton failed");
    if (result) {
        addr = in.s_addr;
    } else {
        struct addrinfo* ai = nullptr;
        struct addrinfo hints = {};
        hints.ai_family = PF_INET;
        hints.ai_flags  = AI_CANONNAME;
        if (getaddrinfo(textAddr.c_str(), nullptr, &hints, &ai) != 0)
        {
            hsAssert(false, "getaddrinfo failed");
            return addr;
        }

        addr = reinterpret_cast<sockaddr_in *>(ai->ai_addr)->sin_addr.s_addr;
        freeaddrinfo(ai);
    }

    return addr;
}

} // pnNetCommon namespace



////////////////////////////////////////////////////////////////////

void plCreatableStream::Write( hsStream* stream, hsResMgr* mgr )
{
    fStream.Rewind();

    uint32_t len = fStream.GetEOF();
    stream->WriteLE32(len);

    uint8_t* buf = new uint8_t[len];
    fStream.Read(len, (void*)buf);
    stream->Write(len, (const void*)buf);

    fStream.Rewind();
    delete[] buf;
}

void plCreatableStream::Read( hsStream* stream, hsResMgr* mgr )
{
    fStream.Rewind();

    uint32_t len = stream->ReadLE32();

    uint8_t* buf = new uint8_t[len];
    stream->Read(len, buf);
    fStream.Write(len, buf);

    fStream.Rewind();
    delete[] buf;
}

////////////////////////////////////////////////////////////////////
// End.
