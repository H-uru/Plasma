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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtMisc.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTMISC_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtMisc.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTMISC_H


/*****************************************************************************
*
*   Constants
*
***/

const wchar UNICODE_BOM = 0xfeff;  // Unicode byte-order mark
const char  UTF8_BOM[]  = "\xef\xbb\xbf";


/*****************************************************************************
*
*   Module instance functions
*
***/

void ModuleSetInstance (void * instance);
void * ModuleGetInstance ();


/*****************************************************************************
*
*   Command line functions
*
***/

const wchar * AppGetCommandLine ();


/*****************************************************************************
*
*   System info functions
*
***/
void MachineGetName (wchar * computerName, unsigned length = 32);


/*****************************************************************************
*
*   Misc types
*
***/

// used to dump the internal state of a module
typedef void (__cdecl * FStateDump)(
    void *      param,
    const wchar fmt[],
    ...
);


/*****************************************************************************
*
*   Dll initialization
*
***/

#if HS_BUILD_FOR_WIN32
#define SRV_MODULE_PRE_INIT()                                               \
    extern BOOL WINAPI PreDllMain (HANDLE handle, DWORD reason, LPVOID);    \
    extern "C" BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID) = PreDllMain
#endif


/*****************************************************************************
*
*   System status
*
***/

struct MemoryStatus {
    unsigned totalPhysMB;       // total physical memory 
    unsigned availPhysMB;       // free physical memory
    unsigned totalPageFileMB;   // total page file size
    unsigned availPageFileMB;   // free page file size
    unsigned totalVirtualMB;    // total virtual address space for calling process
    unsigned availVirtualMB;    // available virtual address space for calling process
    unsigned memoryLoad;        // 0..100
};
void MemoryGetStatus (MemoryStatus * status);

struct DiskStatus {
    wchar       name[16];
    unsigned    totalSpaceMB;
    unsigned    freeSpaceMB;
};

void DiskGetStatus (ARRAY(DiskStatus) * disks);


void CpuGetInfo (
	word *	cpuCaps,
	dword *	cpuVendor,
	word *	cpuSignature
);

// CPU capability flags
const unsigned kCpuCap3dNow	= 1<<0;
const unsigned kCpuCapCmov	= 1<<1;	// conditional move
const unsigned kCpuCapEst	= 1<<2;	// enhanced speed step
const unsigned kCpuCapHtt	= 1<<3;	// hyperthreading
const unsigned kCpuCapMmx	= 1<<4;	// multimedia extensions
const unsigned kCpuCapPsn	= 1<<5;	// processor serial number
const unsigned kCpuCapSse	= 1<<6; // streaming SIMD extensions
const unsigned kCpuCapSse2	= 1<<7;
const unsigned kCpuCapSse3	= 1<<8;
const unsigned kCpuCapTsc	= 1<<9;	// time stamp counter

// Macros for packing and unpacking CPU signature
#define CPU_SIGNATURE(family, model, stepping)	((stepping) | (model << 4) | (family << 8))
#define CPU_SIGNATURE_FAMILY(sig)				((sig >> 8) & 0xf)
#define CPU_SIGNATURE_MODEL(sig)				((sig >> 4) & 0xf)
#define CPU_SIGNATURE_STEPPING(sig)				(sig & 0xf)
