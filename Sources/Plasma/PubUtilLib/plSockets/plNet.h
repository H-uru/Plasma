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
#ifndef plNet_h_inc
#define plNet_h_inc


#include "HeadSpin.h"


////////////////////////////////////////////////////
// Windows net types
#if HS_BUILD_FOR_WIN32

const int   kBlockingError          = WSAEWOULDBLOCK;
const int   kTimeoutError           = WSAETIMEDOUT;    
const SOCKET    kBadSocket          = 0xffffffff;
typedef int socklen_t;


////////////////////////////////////////////////////
// UNIX net types
#else
#ifdef HS_BUILD_FOR_FREEBSD
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

typedef int SOCKET;
const int   kBlockingError          = EWOULDBLOCK;
const int   kTimeoutError           = ETIMEDOUT;    
const SOCKET    kBadSocket          = -1;

// must #define BSDBLOCK if compiling on BSD

#endif

const unsigned int kDefaultSocketTimeout = 5*60*1000;  // 5 mins in millis

////////////////////////////////////////////////////
// OS socket interface wrapper
struct plNet
{
    static SOCKET NewUDP();
    static SOCKET NewTCP();
    static int GetError();
    static int Read(const SOCKET sck, char * buf, const int size);
    static int Write(const SOCKET sck, const char * buf, const int len);
    static int ReadFrom(const SOCKET sck, char * buf, int len, sockaddr_in * addr);
    static int WriteTo(const SOCKET sck, const char * buf, const int len, sockaddr_in * addr);
    static int Connect(const SOCKET sck, const sockaddr_in * addr);
    static int Close(const SOCKET sck);
    static int Bind(const SOCKET sck, const sockaddr_in * addr);
    static int Listen(const SOCKET sck, const int qsize);
    static int Accept(const SOCKET sck, sockaddr_in * addr);
    static int Ioctl(const SOCKET sck, const long flags, unsigned long * val);
    static const char * GetErrorMsg(int error);
    // TODO: Add get/setsockopt() here
    ~plNet();
private:
    static plNet _;
    plNet();
    // not impl
    plNet(const plNet &);
    plNet & operator=(const plNet &);
};





#endif // plNet_h_inc
