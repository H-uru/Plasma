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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Misc.cpp
*   
***/

#include "../pnUtils.h"

/*****************************************************************************
*
*   Private
*
***/
static MEMORYSTATUSEX s_memstatus;

/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
const wchar_t * AppGetCommandLine () {
    return GetCommandLineW();
}

//============================================================================
void MachineGetName (wchar_t *computerName, unsigned int length) {
    DWORD len = length;
    GetComputerNameW(computerName, &len);
}

/*****************************************************************************
*
*   System status
*
***/

//============================================================================
void MemoryGetStatus (MemoryStatus * status) {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    const uint64_t BYTES_PER_MB = 1024 * 1024;
    status->totalPhysMB         = unsigned(mem.ullTotalPhys     / BYTES_PER_MB);
    status->availPhysMB         = unsigned(mem.ullAvailPhys     / BYTES_PER_MB);
    status->totalPageFileMB     = unsigned(mem.ullTotalPageFile / BYTES_PER_MB);
    status->availPageFileMB     = unsigned(mem.ullAvailPageFile / BYTES_PER_MB);
    status->totalVirtualMB      = unsigned(mem.ullTotalVirtual  / BYTES_PER_MB);   
    status->availVirtualMB      = unsigned(mem.ullAvailVirtual  / BYTES_PER_MB);
    status->memoryLoad          = mem.dwMemoryLoad;
}

//============================================================================
void DiskGetStatus (ARRAY(DiskStatus) * disks) {
    DWORD length = GetLogicalDriveStrings(0, NULL);
    if (!length || length > 2048)
        return;

    wchar_t* buffer = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    wchar_t* origbuf = buffer;
    if (!GetLogicalDriveStringsW(length, buffer))
    {
        free(buffer);
        return;
    }

    for (; *buffer; buffer += StrLen(buffer) + 1) {
        UINT driveType = GetDriveTypeW(buffer);
        if (driveType != DRIVE_FIXED)
            continue;

        ULARGE_INTEGER freeBytes;
        ULARGE_INTEGER totalBytes;
        if (!GetDiskFreeSpaceExW(buffer, &freeBytes, &totalBytes, NULL))
            continue;

        DiskStatus status;
        StrCopy(status.name, buffer, arrsize(status.name));

        const uint64_t BYTES_PER_MB = 1024 * 1024;
        status.totalSpaceMB = unsigned(totalBytes.QuadPart / BYTES_PER_MB);
        status.freeSpaceMB  = unsigned(freeBytes.QuadPart  / BYTES_PER_MB);

        disks->Add(status);
    }
    free(origbuf);
}

//============================================================================
// Loosely taken from MS's cpuid code sample
void CpuGetInfo (
    uint16_t *  cpuCaps,
    uint32_t * cpuVendor,
    uint16_t *  cpuSignature
) {
    uint32_t signature = 0;
    uint32_t extended  = 0;
    uint32_t flags[2]  = { 0, 0 };
    cpuVendor[0]    = 0;

    _asm {
        // Detect if cpuid instruction is supported by attempting
        // to change the ID bit of EFLAGS
        pushfd
        pop eax             // get EFLAGS
        mov ecx, eax        // store copy of original EFLAGS
        xor eax, 0x200000   // flip ID bit 
        push eax
        popfd               // replace EFLAGS
        pushfd              // get EFLAGS
        pop eax
        xor eax, ecx
        je DONE

        // Get processor id (GenuineIntel, AuthenticAMD, etc)
        xor eax, eax
        cpuid
        mov edi, cpuVendor
        mov [edi + 0], ebx
        mov [edi + 4], edx
        mov [edi + 8], ecx

        // Check if capability flags are supported
        cmp eax, 1
        jl DONE

        // Get processor capability flags and signature
        mov eax, 1
        cpuid
        mov signature, eax
        mov [flags + 0], edx
        mov [flags + 4], ecx

        // Check for extended capabilities
        mov eax, 0x80000000
        cpuid
        cmp eax, 0x80000001
        jl DONE

        // Get extended capabilities
        mov eax, 0x80000001
        cpuid
        mov extended, edx

DONE:
    }

    // Decode capability flags
    const static struct CpuCap {
        uint16_t    cpuFlag;
        uint8_t    field;
        uint8_t    bit;
    } s_caps[] = {
        // feature      field   bit
        // -------      -----   ---
        { kCpuCapCmov,  0,      15  },
        { kCpuCapEst,   1,      7   },
        { kCpuCapHtt,   0,      28  },
        { kCpuCapMmx,   0,      23  },
        { kCpuCapPsn,   0,      18  },
        { kCpuCapSse,   0,      25  },
        { kCpuCapSse2,  0,      26  },
        { kCpuCapSse3,  1,      0   },
        { kCpuCapTsc,   0,      4   },
    };
    for (unsigned i = 0; i < arrsize(s_caps); ++i) {
        const CpuCap & cap = s_caps[i];
        if (flags[cap.field] & (1 << cap.bit))
            *cpuCaps |= cap.cpuFlag;
    }

    // Copy signature
    *cpuSignature = uint16_t(signature & 0xfff);

    // If this is an AMD CPU, check for 3DNow support
    const char * vendorAmd = "AuthenticAMD";
    if (!memcmp(vendorAmd, cpuVendor, 12)) {
        if (extended & (1 << 31))
            *cpuCaps |= kCpuCap3dNow;
    }
}
