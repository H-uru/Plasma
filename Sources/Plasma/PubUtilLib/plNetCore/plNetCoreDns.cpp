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

#include "HeadSpin.h"
#include "plNetCoreDns.h"

#ifdef HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#else
#   include <netdb.h>
#endif

#include <ares.h>
#include "hsThread.h"
#include "pnNetCommon/plNetAddress.h"


/**
 * plResolver
 *
 * A thread that resolves a DNS query.
 */
class plResolver : public hsThread
{
private:
    ares_channel                        fChannel;
    plNetCoreDns::plResolveCallback     fCallback;
    const plString&                     fHostname;
    uint16_t                            fPort;
    void*                               fCBData;

    static void ares_callback(void* arg, int status, int timeout, struct hostent* host)
    {
        plResolver* self = reinterpret_cast<plResolver*>(arg);

        if (host == nullptr || status != ARES_SUCCESS)
        {
            // TODO: Figure out error handling?
            self->fCallback(0, nullptr, self->fCBData);
            return;
        }

        in_addr const * const * const inAddr = (in_addr**)host->h_addr_list;

        size_t count = arrsize(inAddr);
        plNetAddress* addrs = new plNetAddress[count];

        for (size_t i = 0; i < count; i++)
        {
            addrs[i].SetHost(inAddr[i]->s_addr);

            if (self->fPort != 0) {
                addrs[i].SetPort(self->fPort);
            } else {
                addrs[i].SetAnyPort();
            }
        }

        self->fCallback(count, addrs, self->fCBData);
    }

public:
    plResolver(plNetCoreDns::plResolveCallback cb, void* data, const plString& host, uint16_t port=0)
        : hsThread(0), fCallback(cb), fCBData(data), fHostname(host), fPort(port) {}

    virtual hsError Run()
    {
        int status = ares_init(&fChannel);
        hsAssert(status == ARES_SUCCESS, "Failed to initialize C-Ares channel");

        ares_gethostbyname(fChannel, fHostname.c_str(), AF_INET, plResolver::ares_callback, this);

        for (;;) {
            struct timeval *tvp, tv;
            fd_set read_fds, write_fds;
            int nfds;

            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            nfds = ares_fds(fChannel, &read_fds, &write_fds);
            if (nfds == 0) {
                break;
            }

            tvp = ares_timeout(fChannel, nullptr, &tv);
            select(nfds, &read_fds, &write_fds, nullptr, tvp);
            ares_process(fChannel, &read_fds, &write_fds);
        }

        ares_destroy(fChannel);

        return hsOK;
    }
};


bool plNetCoreDns::fInitialized;

void plNetCoreDns::Initialize()
{
    int status = ares_library_init(ARES_LIB_INIT_ALL);

    hsAssert(status == ARES_SUCCESS, "Failed to initialize C-Ares library!");

    fInitialized = true;
}

void plNetCoreDns::Shutdown()
{
    fInitialized = false;

    ares_library_cleanup();
}


void plNetCoreDns::Resolve(const plString& hostname, plResolveCallback cb, void* data)
{
    hsAssert(fInitialized, "plNetCoreDns is not initialized!");

    plResolver* resolver = new plResolver(cb, data, hostname);
    resolver->Start();
}

void plNetCoreDns::Resolve(const plString& hostname, uint16_t port, plResolveCallback cb, void* data)
{
    hsAssert(fInitialized, "plNetCoreDns is not initialized!");

    plResolver* resolver = new plResolver(cb, data, hostname, port);
    resolver->Start();
}
