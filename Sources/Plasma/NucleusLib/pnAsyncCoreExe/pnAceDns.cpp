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

#include "Pch.h"

using tcp = asio::ip::tcp;

struct DnsResolver
{
    asio::io_context                                           fContext;
    asio::executor_work_guard<asio::io_context::executor_type> fWorkGuard;
    tcp::resolver                                              fResolver;
    AsyncThreadRef                                             fLookupThread;

    DnsResolver() : fResolver(fContext),
                    fWorkGuard(fContext.get_executor())
    {
        // Start the resolver thread
        fLookupThread = AsyncThreadCreate([this] {
            hsThread::SetThisThreadName(ST_LITERAL("AceDnsResolver"));
            fContext.run();
        });
    }

    void Destroy(unsigned exitThreadWaitMs)
    {
        fWorkGuard.reset();
        fResolver.cancel();
        AsyncThreadTimedJoin(fLookupThread, exitThreadWaitMs);
    }
};

struct DnsResolveData
{
    ST::string       fName;
    FAsyncLookupProc fLookupProc;
    void*            fParam;
};

static std::recursive_mutex s_critsect;
static DnsResolver*         s_resolver = nullptr;

void DnsDestroy(unsigned exitThreadWaitMs)
{
    if (s_resolver) {
        s_resolver->Destroy(exitThreadWaitMs);
        delete s_resolver;
        s_resolver = nullptr;
    }
}

static void AddressResolved(const asio::error_code&            err,
                            const tcp::resolver::results_type& results,
                            const DnsResolveData&              data)
{
    if (err || results.empty()) {
        if (err)
            LogMsg(kLogFatal, "DNS: Failed to resolve {}: {}", data.fName, err.message());
        data.fLookupProc(data.fParam, data.fName, {});
        return;
    }

    // This can be cleaned up when the rest of the netcode uses ASIO instead
    // of other (Win32/NT) backends...
    std::vector<plNetAddress> addrs;
    addrs.reserve(results.size());
    for (const auto& result : results) {
        const auto& endpoint = result.endpoint();
        if (!endpoint.address().is_v4()) {
            // Due to limitations in the net protocol, we can only support IPv4 for now :(
            continue;
        }
        const auto ipv4_addr = endpoint.address().to_v4();
        addrs.emplace_back(ipv4_addr.to_bytes(), endpoint.port());
    }

    data.fLookupProc(data.fParam, data.fName, addrs);

    PerfSubCounter(kAsyncPerfNameLookupAttemptsCurr, 1);
}

void AsyncAddressLookupName(FAsyncLookupProc lookupProc,

                            const ST::string& name, unsigned port, void* param)
{
    ASSERT(lookupProc);

    PerfAddCounter(kAsyncPerfNameLookupAttemptsCurr, 1);
    PerfAddCounter(kAsyncPerfNameLookupAttemptsTotal, 1);

    std::string_view hostStr, portStr;
    char             portBuffer[12];
    ST_ssize_t       colon = name.find(':');
    if (colon >= 0) {
        hostStr = name.view(0, colon);
        portStr = name.view(colon + 1);
    } else {
        snprintf(portBuffer, std::size(portBuffer), "%u", port);
        hostStr = name.view();
        portStr = std::string_view(portBuffer);
    }

    DnsResolveData data;
    data.fName = name;
    data.fLookupProc = std::move(lookupProc);
    data.fParam = param;

    hsLockGuard(s_critsect);

    if (!s_resolver)
        s_resolver = new DnsResolver;

    s_resolver->fResolver.async_resolve(hostStr, portStr, tcp::resolver::address_configured,
            [data = std::move(data)](const asio::error_code& err,
                                     const tcp::resolver::results_type& results)
            {
                AddressResolved(err, results, data);
            });
}
