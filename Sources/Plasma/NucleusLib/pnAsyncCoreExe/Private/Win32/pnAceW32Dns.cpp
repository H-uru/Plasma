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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Win32/pnAceW32Dns.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

enum {
    WM_LOOKUP_EXIT = WM_APP,
    WM_LOOKUP_FOUND_HOST
};

const unsigned kMaxLookupName = 128;

struct Lookup {
    LINK(Lookup)        link;
    AsyncCancelId       cancelId;
    HANDLE              cancelHandle;
    FAsyncLookupProc    lookupProc;
    unsigned            port;
    void *              param;
    wchar               name[kMaxLookupName];
    char                buffer[MAXGETHOSTSTRUCT];
};

static CCritSect                s_critsect;
static LISTDECL(Lookup, link)   s_lookupList;
static HANDLE                   s_lookupThread;
static HWND                     s_lookupWindow;
static unsigned                 s_nextLookupCancelId = 1;


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static void LookupProcess (Lookup * lookup, unsigned error) {
    unsigned count      = 0;
    NetAddress * addrs  = nil;
    for (ONCE) {
        if (error)
            break;

        const HOSTENT & host = * (HOSTENT *) lookup->buffer;
        if (host.h_addrtype != AF_INET)
            break;
        if (host.h_length != sizeof(in_addr))
            break;
        if (!host.h_addr_list)
            break;

        in_addr const * const * const inAddr = (in_addr **) host.h_addr_list;

        // count the number of addresses
        while (inAddr[count])
            ++count;

        // allocate a buffer large enough to hold all the addresses
        addrs = (NetAddress *) _alloca(sizeof(*addrs) * count);
        MemZero(addrs, sizeof(*addrs) * count);

        // fill in address data
        const word port = htons((word) lookup->port);
        for (unsigned i = 0; i < count; ++i) {
            sockaddr_in * inetaddr = (sockaddr_in *) &addrs[i];
            inetaddr->sin_family    = AF_INET;
            inetaddr->sin_addr      = *inAddr[i];
            inetaddr->sin_port      = port;
        }

        if (host.h_name && host.h_name[0])
            StrToUnicode(lookup->name, host.h_name, arrsize(lookup->name));
    }

    if (lookup->lookupProc)
        lookup->lookupProc(lookup->param, lookup->name, count, addrs);

    // we can delete the operation outside an IoConn critical
    // section because it isn't linked into an ioConn opList
    // and because connection attempts are not waitable
    ASSERT(!lookup->link.IsLinked());
    DEL(lookup);
    PerfSubCounter(kAsyncPerfNameLookupAttemptsCurr, 1);
}

//===========================================================================
static void LookupFindAndProcess (HANDLE cancelHandle, unsigned error) {
    // find the operation for this cancel handle
    Lookup * lookup;
    s_critsect.Enter();
    for (lookup = s_lookupList.Head(); lookup; lookup = s_lookupList.Next(lookup)) {
        if (lookup->cancelHandle == cancelHandle) {
            lookup->cancelHandle = nil;
            s_lookupList.Unlink(lookup);
            break;
        }
    }
    s_critsect.Leave();
    if (lookup)
        LookupProcess(lookup, error);
}

//===========================================================================
static unsigned THREADCALL LookupThreadProc (AsyncThread * thread) {
    static const char WINDOW_CLASS[] = "AsyncLookupWnd";
    WNDCLASS wc;
    ZERO(wc);
    wc.lpfnWndProc      = DefWindowProc;
    wc.hInstance        = GetModuleHandle(0);
    wc.lpszClassName    = WINDOW_CLASS;
    RegisterClass(&wc);

    s_lookupWindow = CreateWindow(
        WINDOW_CLASS,
        WINDOW_CLASS,
        WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        (HWND)0,
        (HMENU) 0,
        wc.hInstance,
        0
    );
    if (!s_lookupWindow)
        ErrorFatal(__LINE__, __FILE__, "CreateWindow %#x", GetLastError());

    HANDLE lookupStartEvent = (HANDLE) thread->argument;
    SetEvent(lookupStartEvent);

    MSG msg;
    while (GetMessage(&msg, s_lookupWindow, 0, 0)) {
        if (msg.message == WM_LOOKUP_FOUND_HOST)
            LookupFindAndProcess((HANDLE) msg.wParam, HIWORD(msg.lParam));
        else if (msg.message == WM_LOOKUP_EXIT)
            break;
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // fail all pending name lookups
    for (;;) {
        s_critsect.Enter();
        Lookup * lookup = s_lookupList.Head();
        if (lookup) {
            WSACancelAsyncRequest(lookup->cancelHandle);
            lookup->cancelHandle = nil;
            s_lookupList.Unlink(lookup);
        }
        s_critsect.Leave();
        if (!lookup)
            break;

        LookupProcess(lookup, (unsigned) -1);
    }

    // cleanup
    DestroyWindow(s_lookupWindow);
    s_lookupWindow = nil;

    return 0;
}

//===========================================================================
static void StartLookupThread () {
    if (s_lookupThread)
        return;

    // create a shutdown event
    HANDLE lookupStartEvent = CreateEvent(
        (LPSECURITY_ATTRIBUTES) 0,
        true,           // manual reset
        false,          // initial state off
        (LPCTSTR) 0     // name
    );
    if (!lookupStartEvent)
        ErrorFatal(__LINE__, __FILE__, "CreateEvent %#x", GetLastError());

    // create a thread to perform lookups
    s_lookupThread = (HANDLE) AsyncThreadCreate(
        LookupThreadProc,
        lookupStartEvent,
        L"AsyncLookupThread"
    );

    WaitForSingleObject(lookupStartEvent, INFINITE);
    CloseHandle(lookupStartEvent);
    ASSERT(s_lookupWindow);
}


/*****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void DnsDestroy (unsigned exitThreadWaitMs) {
    if (s_lookupThread) {
        PostMessage(s_lookupWindow, WM_LOOKUP_EXIT, 0, 0);
        WaitForSingleObject(s_lookupThread, exitThreadWaitMs);
        CloseHandle(s_lookupThread);
        s_lookupThread = nil;
        ASSERT(!s_lookupWindow);
    }
}


/*****************************************************************************
*
*   Public functions
*
***/

//===========================================================================
void AsyncAddressLookupName (
    AsyncCancelId *     cancelId,   // out
    FAsyncLookupProc    lookupProc,
    const wchar         name[], 
    unsigned            port, 
    void *              param
) {
    ASSERT(lookupProc);
    ASSERT(name);

    PerfAddCounter(kAsyncPerfNameLookupAttemptsCurr, 1);
    PerfAddCounter(kAsyncPerfNameLookupAttemptsTotal, 1);

    // Get name/port
    char ansiName[kMaxLookupName];
    StrToAnsi(ansiName, name, arrsize(ansiName));
    if (char * portStr = StrChr(ansiName, ':')) {
        if (unsigned newPort = StrToUnsigned(portStr + 1, nil, 10))
            port = newPort;
        *portStr = 0;
    }

    // Initialize lookup
    Lookup * lookup         = NEW(Lookup);
    lookup->lookupProc      = lookupProc;
    lookup->port            = port;
    lookup->param           = param;
    StrCopy(lookup->name, name, arrsize(lookup->name));

    s_critsect.Enter();
    {
        // Start the lookup thread if it wasn't started already
        StartLookupThread();
        s_lookupList.Link(lookup);

        // get cancel id; we can avoid checking for zero by always using an odd number
        ASSERT(s_nextLookupCancelId & 1);
        s_nextLookupCancelId += 2;
        *cancelId = lookup->cancelId = (AsyncCancelId) s_nextLookupCancelId;

        // Perform async lookup
        lookup->cancelHandle = WSAAsyncGetHostByName(
            s_lookupWindow, 
            WM_LOOKUP_FOUND_HOST,
            ansiName,
            &lookup->buffer[0],
            sizeof(lookup->buffer)
        );
        if (!lookup->cancelHandle) {
            PostMessage(s_lookupWindow, WM_LOOKUP_FOUND_HOST, nil, (unsigned) -1);
        }
    }
    s_critsect.Leave();
}

//===========================================================================
void AsyncAddressLookupAddr (
    AsyncCancelId *     cancelId,   // out
    FAsyncLookupProc    lookupProc,
    const NetAddress &  address,
    void *              param
) {
    ASSERT(lookupProc);

    PerfAddCounter(kAsyncPerfNameLookupAttemptsCurr, 1);
    PerfAddCounter(kAsyncPerfNameLookupAttemptsTotal, 1);

    // Initialize lookup
    Lookup * lookup         = NEW(Lookup);
    lookup->lookupProc      = lookupProc;
    lookup->port            = 1;
    lookup->param           = param;
    NetAddressToString(
        address,
        lookup->name,
        arrsize(lookup->name),
        kNetAddressFormatNodeNumber
    );
        
    s_critsect.Enter();
    {
        // Start the lookup thread if it wasn't started already
        StartLookupThread();
        s_lookupList.Link(lookup);

        // get cancel id; we can avoid checking for zero by always using an odd number
        ASSERT(s_nextLookupCancelId & 1);
        s_nextLookupCancelId += 2;
        *cancelId = lookup->cancelId = (AsyncCancelId) s_nextLookupCancelId;

        // Perform async lookup
        u_long addr = ((const sockaddr_in *) &address)->sin_addr.S_un.S_addr;
        lookup->cancelHandle = WSAAsyncGetHostByAddr(
            s_lookupWindow, 
            WM_LOOKUP_FOUND_HOST,
            (const char *) &addr,
            sizeof(addr),
            AF_INET,
            &lookup->buffer[0],
            sizeof(lookup->buffer)
        );
        if (!lookup->cancelHandle) {
            PostMessage(s_lookupWindow, WM_LOOKUP_FOUND_HOST, nil, (unsigned) -1);
        }
    }
    s_critsect.Leave();
}

//===========================================================================
void AsyncAddressLookupCancel (
    FAsyncLookupProc    lookupProc,
    AsyncCancelId       cancelId        // nil = cancel all with specified lookupProc
) {
    s_critsect.Enter();
    for (Lookup * lookup = s_lookupList.Head(); lookup; lookup = s_lookupList.Next(lookup)) {
        if (lookup->lookupProc && (lookup->lookupProc != lookupProc))
            continue;
        if (cancelId && (lookup->cancelId != cancelId))
            continue;
        if (!lookup->cancelHandle)
            continue;

        // cancel this request
        WSACancelAsyncRequest(lookup->cancelHandle);
        lookup->cancelHandle = nil;

        // initiate user callback
        PostMessage(s_lookupWindow, WM_LOOKUP_FOUND_HOST, nil, (unsigned) -1);
    }
    s_critsect.Leave();
}
