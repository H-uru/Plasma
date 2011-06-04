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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/pnAceCore.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private data
*
***/

static long s_perf[kNumAsyncPerfCounters];


/****************************************************************************
*
*   Module data exports
*
***/

AsyncApi	g_api;
bool		s_transgaming;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void DoTransgamingCheck () {
#ifdef HS_BUILD_FOR_WIN32
#ifdef CLIENT

	HMODULE hMod = GetModuleHandle("ntdll");
	if (!hMod)
		return;

	s_transgaming = GetProcAddress(hMod, "IsTransgaming") != nil;

#endif
#endif
}

//===========================================================================
static void IAsyncInitUseW9x () {
	ref(IAsyncInitUseW9x);
#ifdef HS_BUILD_FOR_WIN32
	W9xGetApi(&g_api);
#else
	ErrorFatal("W9x I/O Not supported on this platform");
#endif
}

//===========================================================================
static void IAsyncInitUseNt () {
	ref(IAsyncInitUseNt);
#ifdef HS_BUILD_FOR_WIN32
	NtGetApi(&g_api);
#else
	ErrorFatal("Nt I/O Not supported on this platform");
#endif
}

//===========================================================================
static void IAsyncInitUseUnix () {
	ref(IAsyncInitUseUnix);
#ifdef HS_BUILD_FOR_UNIX
	#error Unix I/O not implemented yet
	UxGetApi(&g_api);
#else
	ErrorFatal(__LINE__, __FILE__, "Unix I/O Not supported on this platform");
#endif
}

//===========================================================================
static void IAsyncInitForClient () {
	ref(IAsyncInitForClient);
#ifdef HS_BUILD_FOR_WIN32
	DoTransgamingCheck();
	if (s_transgaming) {
		IAsyncInitUseW9x();
	}
	else {
		IAsyncInitUseNt();
	}
#elif HS_BUILD_FOR_UNIX
	IAsyncInitUseUnix();
#else
	ErrorFatal("AsyncCore: No default implementation for this platform");
#endif    
}

//===========================================================================
static void IAsyncInitForServer () {
	ref(IAsyncInitForServer);
#ifdef HS_BUILD_FOR_WIN32
	IAsyncInitUseNt();
#elif HS_BUILD_FOR_UNIX
	IAsyncInitUseUnix();
#else
	ErrorFatal("AsyncCore: No default implementation for this platform");
#endif    
}


/*****************************************************************************
*
*   Module exports
*
***/

//============================================================================
long PerfAddCounter (unsigned id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return AtomicAdd(&s_perf[id], n);
}

//============================================================================
long PerfSubCounter (unsigned id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return AtomicAdd(&s_perf[id], -(signed)n);
}

//============================================================================
long PerfSetCounter (unsigned id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return AtomicSet(&s_perf[id], n);
}


/*****************************************************************************
*
*   Public exports
*
***/

//===========================================================================
void AsyncCoreInitialize () {
    ASSERTMSG(!g_api.initialize, "AsyncCore already initialized");
    
#ifdef HS_BUILD_FOR_WIN32
    // Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(0x101, &wsaData))
        ErrorFatal(__LINE__, __FILE__, "WSA startup failed");
    if (wsaData.wVersion != 0x101)
        ErrorFatal(__LINE__, __FILE__, "WSA version failed");
#endif

#ifdef CLIENT
    IAsyncInitForClient();
#elif SERVER
    IAsyncInitForServer();
#else
# error "Neither CLIENT nor SERVER defined. Cannot configure AsyncCore for target"
#endif
    
    ASSERT(g_api.initialize);
    g_api.initialize();
}

//============================================================================
void AsyncCoreDestroy (unsigned waitMs) {
    if (g_api.destroy) {
        g_api.destroy(waitMs);
    }
    
    DnsDestroy(waitMs);
    TimerDestroy(waitMs);
    ThreadDestroy(waitMs);
	
	ZERO(g_api);
}

//============================================================================
void AsyncSignalShutdown () {
    ASSERT(g_api.signalShutdown);
    g_api.signalShutdown();
}

//============================================================================
void AsyncWaitForShutdown () {
    ASSERT(g_api.waitForShutdown);
    g_api.waitForShutdown();
}

//============================================================================
void AsyncSleep (unsigned sleepMs) {
    ASSERT(g_api.sleep);
    g_api.sleep(sleepMs);
}

//============================================================================
long AsyncPerfGetCounter (unsigned id) {
    COMPILER_ASSERT(arrsize(s_perf) == kNumAsyncPerfCounters);
    ASSERT(id < kNumAsyncPerfCounters);
    return s_perf[id];
}
