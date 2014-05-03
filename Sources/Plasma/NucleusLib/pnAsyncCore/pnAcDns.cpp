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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcDns.cpp
*   
***/

#include "pnAcDns.h"
#include "pnAcInt.h"
#include "hsThread.h"
#include <sstream>


/*****************************************************************************
*
*   dns lookup functions 
*   
***/

struct AsyncDns::P : hsThread, IWorkerThreads::Operation {
    struct Name;
    struct Addr;
    static P *  last;
    static std::mutex   listLock;
    
    bool        cancel;
    FProc       lookupProc;
    void *      param;
    bool        error;
    P *         next;
    P *         prev;
    
    P () : cancel(false), prev(nullptr) {}
    
    void Callback ();
};
AsyncDns::P *   AsyncDns::P::last = nullptr;
std::mutex      AsyncDns::P::listLock;

//===========================================================================
void AsyncDns::P::Callback () {
    Stop();
    
    std::lock_guard<std::mutex> lock(P::listLock);
    if (prev)
        prev->next = next;
    else
        last = next;
    if (next)
        next->prev = prev;
}

//===========================================================================
bool AsyncDns::Cancel::LookupCancel () {
    std::lock_guard<std::mutex> lock(P::listLock);
    for (P * op = P::last; op; op = op->next) {
        if (op == ptr) {
            op->cancel = true;
            ptr = nullptr;
            return true;
        }
    }
    ptr = nullptr;
    return false;
}

//===========================================================================
void AsyncDns::LookupCancel (FProc lookupProc) {
    std::lock_guard<std::mutex> lock(P::listLock);
    for (P * op = P::last; op; op = op->next) {
        if (op->lookupProc == lookupProc)
            op->cancel = true;
    }
}


/*****************************************************************************
*
*   lookup name
*   
***/

struct AsyncDns::P::Name : AsyncDns::P {
    const char *    name;
    unsigned        port;
    addrinfo *      result;
    
    hsError Run ();
    void Callback ();
};

//===========================================================================
hsError AsyncDns::P::Name::Run () {
    std::stringstream sstr;
    sstr << port;
    addrinfo hints = { 0 };
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_protocol = IPPROTO_TCP; // tcp
    hints.ai_addrlen = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    error = getaddrinfo(name, sstr.str().c_str(), &hints, &result);
    IWorkerThreads::Add(this);
    
    return 0;
}

//===========================================================================
void AsyncDns::P::Name::Callback () {
    P::Callback();
    
    if (error) {
        delete this;
        return;
    }
    if (cancel) {
        freeaddrinfo(result);
        delete this;
        return;
    }
    
    unsigned count = 0;
    for (addrinfo * it = result; it; it = it->ai_next)
        count++;
    plNetAddress * addrs = new plNetAddress[count], * it2 = addrs;
    for (addrinfo * it = result; it; it->ai_next, it2++)
        // IPv4 only: cast must be safe!
        *it2 = plNetAddress(((sockaddr_in*)it->ai_addr)->sin_addr.s_addr, port);
    freeaddrinfo(result);
    
    lookupProc(param, name, count, addrs);
    delete[] addrs;
    delete this;
}

//===========================================================================
AsyncDns::Cancel AsyncDns::LookupName (
    const char      name[],
    unsigned        port,
    FProc           lookupProc,
    void *          param
) {
    P::Name * op = new P::Name;
    {
        std::lock_guard<std::mutex> lock(P::listLock);
        op->next = P::last;
        P::last = P::last->prev = op;
    }
    op->name =          name;
    op->port =          port;
    op->lookupProc =    lookupProc;
    op->param =         param;
    
    op->Start();
    return Cancel(op);
}


/*****************************************************************************
*
*   lookup address
*   
***/

struct AsyncDns::P::Addr : AsyncDns::P {
    static const unsigned       kMaxLookupName = 128;
    
    plNetAddress    addr;
    char            result[kMaxLookupName];
    
    hsError Run ();
    void Callback ();
};

//===========================================================================
hsError AsyncDns::P::Addr::Run () {
    error = getnameinfo(
        (const sockaddr*)&addr.GetAddressInfo(),
        sizeof(AddressType),
        result, kMaxLookupName,
        nullptr, 0,
        0
    );
    IWorkerThreads::Add(this);
    
    return 0;
}

//===========================================================================
void AsyncDns::P::Addr::Callback () {
    P::Callback();
    
    if (cancel || error) {
        delete this;
        return;
    }
    
    lookupProc(param, result, 1, &addr);
    delete this;
}

//===========================================================================
AsyncDns::Cancel AsyncDns::LookupAddr (
    const plNetAddress& address,
    FProc               lookupProc,
    void *              param
) {
    P::Addr * op = new P::Addr;
    {
        std::lock_guard<std::mutex> lock(P::listLock);
        op->next = P::last;
        P::last = P::last->prev = op;
    }
    op->addr =          address;
    op->lookupProc =    lookupProc;
    op->param =         param;
    
    op->Start();
    return Cancel(op);
}

//===========================================================================
void DnsDestroy (unsigned exitThreadWaitMs) {
    std::lock_guard<std::mutex> lock(AsyncDns::P::listLock);
    
    for (AsyncDns::P * op = AsyncDns::P::last; op; op = op->next)
        op->cancel = true;
}


