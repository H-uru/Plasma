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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetDiag/pnNetSys.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

typedef DWORD (PASCAL FAR * FGetAdaptersInfo)(
	PIP_ADAPTER_INFO	pAdapterInfo,
	PULONG				pOutBufLen
);



/*****************************************************************************
*
*   Local data
*
***/

static FGetAdaptersInfo		GetAdaptersInfo;


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void SysStartup () {

	if (g_lib) {
		GetAdaptersInfo = (FGetAdaptersInfo)GetProcAddress(g_lib, "GetAdaptersInfo");
	}
}

//============================================================================
void SysShutdown () {

	GetAdaptersInfo = nil;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetDiagSys (
	NetDiag *				diag,
	FNetDiagDumpProc		dump,
	FNetDiagTestCallback	callback,
	void *					param
) {
	ASSERT(diag);
	ASSERT(dump);
	ASSERT(callback);

	{ // Timestamp
		wchar str[256];
		qword time = TimeGetTime();
		TimePrettyPrint(time, arrsize(str), str);
		dump(L"[SYS] Time: %s UTC", str);
	}
		
	{ // Command line
		dump(L"[SYS] Cmdline: %s", AppGetCommandLine());
	}
	
	{ // Product
		wchar product[128];
		ProductString(product, arrsize(product));
		dump(L"[SYS] Product: %s", product);
	}
	
	{ // pnNetDiag version
		dump(L"[SYS] Cognomen: '%s'", g_version);
	}
	
	{ // OS
		OSVERSIONINFOEX info;
		info.dwOSVersionInfoSize = sizeof(info);
		GetVersionEx((OSVERSIONINFO*)&info);
		dump(L"[SYS] OS Version: %u.%u", info.dwMajorVersion, info.dwMinorVersion);
		dump(L"[SYS] OS Patch: %u.%u (%S)", info.wServicePackMajor, info.wServicePackMinor, info.szCSDVersion);
	}
	
	{ // System
		word	cpuCaps;
		dword	cpuVendor[3];
		word	cpuSignature;
		CpuGetInfo(&cpuCaps, cpuVendor,	&cpuSignature);
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		dump(L"[SYS] CPU Count: %u", info.dwNumberOfProcessors);
		dump(L"[SYS] CPU Vendor: %.*S", sizeof(cpuVendor), cpuVendor);
	}
	
	{ // Adapters
		if (!GetAdaptersInfo) {
			dump(L"[SYS] Failed to load IP helper API");
			callback(diag, kNetProtocolNil, kNetErrNotSupported, param);
			return;
		}

		ULONG ulOutBufLen = 0;
		GetAdaptersInfo(nil, &ulOutBufLen);
		PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)ALLOC(ulOutBufLen);
		PIP_ADAPTER_INFO pAdapter;
		if (GetAdaptersInfo(pInfo, &ulOutBufLen) == NO_ERROR) {
			pAdapter = pInfo;
			while (pAdapter) {
				dump(L"[SYS] NIC: %S", pAdapter->Description);
				pAdapter = pAdapter->Next;
			}
			callback(diag, kNetProtocolNil, kNetSuccess, param);
		}
		else {
			dump(L"[SYS] Error getting adaper list");
			callback(diag, kNetProtocolNil, kNetErrFileNotFound, param);
		}
		FREE(pInfo);
	}
}
