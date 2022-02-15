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

#include "hsSystemInfo.h"

#include "hsConfig.h"
#include "plFileSystem.h"
#include "hsStream.h"
#include "hsWindows.h"

#include <cstring>
#include <iterator>
#include <optional>
#include <string_theory/string>
#include <stdio.h>

#ifdef HAVE_SYSINFO
#    include <sys/sysinfo.h>
#    include <sys/utsname.h>
#elif defined(HAVE_SYSCTL)
#    include <sys/types.h>
#    include <sys/sysctl.h>
#endif

#ifdef HS_BUILD_FOR_UNIX
#    include <sys/utsname.h>
#endif

#ifdef HS_BUILD_FOR_APPLE
#    include <CoreFoundation/CoreFoundation.h>

    extern "C" {
        CF_EXPORT const CFStringRef _kCFSystemVersionProductNameKey;
        CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionKey;

        CF_EXPORT CFDictionaryRef _CFCopyServerVersionDictionary();
        CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary();
    }
#endif

#ifdef HAVE_CPUID
#    if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#        include <intrin.h>
#        define MSC_COMPATIBLE
#        define cpuid_t int
#    elif defined(__GNUC__)
#        include <cpuid.h>
#        define GCC_COMPATIBLE
#        define cpuid_t unsigned int
#    else
#        define cpuid_t int
#    endif
#endif

ST::string hsSystemInfo::AsString()
{
    ST::string_stream ss;
    ss << "OS: " << GetOperatingSystem() << '\n';
    ss << "CPU: " << GetCPUBrand() << '\n';
    ss << "RAM: " << GetRAM() << " MiB";
    return ss.to_string();
}

static inline void ICPUID(cpuid_t* info, int function_id)
{
#if defined(MSC_COMPATIBLE)
    __cpuid(info, function_id);
#elif defined(GCC_COMPATIBLE)
    __get_cpuid(function_id, &info[0], &info[1], &info[2], &info[3]);
#elif !defined(HAVE_CPUID)
    memset(info, 0, sizeof(info));
#endif
}

ST::string hsSystemInfo::GetCPUBrand()
{
    cpuid_t cpuInfo[4]{};
    ICPUID(cpuInfo, 0x80000000);
    unsigned int nExIds = (unsigned int)cpuInfo[0];
    const unsigned int brandIds[]{ 0x80000002, 0x80000003, 0x80000004 };

    char str[std::size(brandIds) * sizeof(cpuInfo)]{};
    char* strp = str;
    for (unsigned int i : brandIds) {
        if (i <= nExIds) {
            ICPUID(reinterpret_cast<cpuid_t*>(strp), i);
            strp += sizeof(cpuInfo);
        }
    }

    ST::string result(str);
    if (result.empty())
        result = ST_LITERAL("Unknown");
    return result;
}

#ifdef HS_BUILD_FOR_APPLE
static inline bool IGetAppleVersion(ST::string& system)
{
    CFDictionaryRef dict = _CFCopyServerVersionDictionary();
    if (dict == nullptr)
        dict = _CFCopySystemVersionDictionary();

    if (dict != nullptr) {
        CFStringRef name = (CFStringRef)CFDictionaryGetValue(dict, _kCFSystemVersionProductNameKey);
        CFStringRef version = (CFStringRef)CFDictionaryGetValue(dict, _kCFSystemVersionProductVersionKey);
        CFStringRef info = CFStringCreateWithFormat(nullptr, nullptr, CFSTR("%@ %@"), name, version);

        CFIndex infoLen = CFStringGetLength(info);
        CFIndex infoBufSz = 0;
        CFStringGetBytes(info, CFRangeMake(0, infoLen), kCFStringEncodingUTF8, 0, false, nullptr, 0, &infoBufSz);
        ST::char_buffer systemBuf;
        systemBuf.allocate(infoBufSz);
        CFStringGetBytes(info, CFRangeMake(0, infoLen), kCFStringEncodingUTF8, 0, false, (UInt8*)systemBuf.data(), infoLen, nullptr);
        system = ST::string(systemBuf);

        CFRelease(info);
        CFRelease(version);
        CFRelease(name);
        CFRelease(dict);

        return true;
    }

    return false;
}
#endif

static inline bool IGetLinuxVersion(const plFileName& osVersionPath, ST::string& system)
{
    plFileInfo info(osVersionPath);
    if (info.Exists() && info.IsFile()) {
        hsUNIXStream s;
        if (s.Open(osVersionPath, "r")) {
            char token[128];
            while (s.GetToken(token, sizeof(token))) {
                if (strcmp(token, "PRETTY_NAME") == 0) {
                    if (s.ReadLn(token, sizeof(token))) {
                        system = ST::string::from_utf8(token);
                        // chop off the quotes
                        system = system.substr(1, system.size() - 2);
                        s.Close();
                        return true;
                    }
                }
            }
            s.Close();
        }
    }

    return false;
}

#ifdef HS_BUILD_FOR_WIN32
static inline bool IGetWindowsVersion(const RTL_OSVERSIONINFOEXW& info, ST::string& system, std::optional<bool> server = std::nullopt)
{
    if (!server.has_value())
        server = info.wProductType == VER_NT_SERVER;

    ST::string_stream ss;
    ss << "Windows ";
    if (info.dwMajorVersion == 5 && info.dwMinorVersion == 0)
        ss << "2000 "; // This is not actually supported.
    else if (info.dwMajorVersion == 5 && info.dwMinorVersion == 1)
        ss << "XP "; // This is not actually supported.
    else if (info.dwMajorVersion == 5 && info.dwMinorVersion == 2 && server.value())
        ss << "2003 "; // This is not actually supported.
    else if (info.dwMajorVersion == 5 && info.dwMinorVersion == 2 && !server.value())
        ss << "XP (64-bit) "; // This is not actually supported.
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 0 && !server.value())
        ss << "Vista "; // Marginally supported.
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 0 && server.value())
        ss << "Server 2008 "; // Marginally supported.
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 1 && !server.value())
        ss << "7 ";
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 1 && server.value())
        ss << "Server 2008 R2 ";
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 2 && !server.value())
        ss << "8 ";
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 2 && server.value())
        ss << "Server 2012 ";
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 3 && !server.value())
        ss << "8.1 ";
    else if (info.dwMajorVersion == 6 && info.dwMinorVersion == 3 && server.value())
        ss << "Server 2012 R2 ";
    else if (info.dwMajorVersion == 10 && info.dwBuildNumber < 22000 && !server.value())
        ss << "10 ";
    else if (info.dwMajorVersion == 10 && info.dwBuildNumber == 14393 && server.value())
        ss << "Server 2016 ";
    else if (info.dwMajorVersion == 10 && info.dwBuildNumber == 17763 && server.value())
        ss << "Server 2019 ";
    else if (info.dwMajorVersion == 10 && info.dwBuildNumber >= 22000 && !server.value())
        ss << "11 ";
    else if (info.dwPlatformId & VER_PLATFORM_WIN32_NT)
        ss << "NT " << info.dwMajorVersion << '.' << info.dwMinorVersion << ' ';
    else
        ss << info.dwMajorVersion << '.' << info.dwMinorVersion << ' ';

    if (info.wSuiteMask & VER_SUITE_BLADE)
        ss << "Web Edition ";
    else if (info.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
        ss << "Compute Cluster Edition ";
    else if (info.wSuiteMask & VER_SUITE_DATACENTER)
        ss << "Datacenter Edition ";
    else if (info.wSuiteMask & VER_SUITE_ENTERPRISE)
        ss << "Enterprise Edition ";
    else if (info.wSuiteMask & VER_SUITE_EMBEDDEDNT)
        ss << "Embedded "; // ORLY?
    else if (info.wSuiteMask & VER_SUITE_PERSONAL)
        ss << "Home Edition ";

    if (info.wProductType == VER_NT_WORKSTATION)
        ss << "Professional ";

    if (info.szCSDVersion && *info.szCSDVersion)
        ss << info.szCSDVersion << ' ';

    ss << "(Build " << info.dwBuildNumber << ')';
    system = ss.to_string();
    return true;
}
#endif

#ifdef HS_BUILD_FOR_WIN32
static inline bool IGetWINEVersion(const RTL_OSVERSIONINFOEXW& info, ST::string& system)
{
    typedef const char* (CDECL* wine_get_version)();
    typedef void (CDECL* wine_get_host_version)(const char** sysname, const char** release);

    HMODULE ntdll = GetModuleHandleW(L"ntdll");
    if (ntdll) {
        wine_get_version wine_target_version = (wine_get_version)GetProcAddress(ntdll, "wine_get_version");
        wine_get_host_version wine_host_version = (wine_get_host_version)GetProcAddress(ntdll, "wine_get_host_version");
        if (wine_target_version && wine_host_version) {
            // WINE for some reason likes to pretend to be Windows Server. Debunk that crap
            // because we really only want to get "Windows Server 2008 R2" if they are really
            // on that OS, not because WINE is saying it's a Windows 7 server. Ugh.
            ST::string windowsVersion;
            IGetWindowsVersion(info, windowsVersion, false);

            // Ideally, we would try to parse Z:\etc\os-release to get the Linux distro name,
            // but that seems to crash WINE, unfortunately. So, we'll just naively use whatever
            // WINE returns (the kernel name and version).
            const char* sysname;
            const char* release;
            wine_host_version(&sysname, &release);
            const char* wineVersion = wine_target_version();

            system = ST::format("{} [Emulated on {} ({}) by WINE {}]", windowsVersion, sysname, release, wineVersion);
            return true;
        }
    }

    return false;
}
#endif

ST::string hsSystemInfo::GetOperatingSystem()
{
    ST::string system = ST_LITERAL("Unknown");

#if defined(HS_BUILD_FOR_WIN32)
    const RTL_OSVERSIONINFOEXW& info = hsGetWindowsVersion();
    if (IGetWINEVersion(info, system))
        return system;
    if (IGetWindowsVersion(info, system))
        return system;
#elif defined(HS_BUILD_FOR_APPLE)
    if (IGetAppleVersion(system))
        return system;
#elif defined(HS_BUILD_FOR_LINUX)
    if (IGetLinuxVersion("/etc/os-release", system))
        return system;
#endif

#ifdef HS_BUILD_FOR_UNIX
    // Should work for everything else.
    utsname sysinfo;
    uname(&sysinfo);
    system = ST::format("{} {} ({})", sysinfo.sysname, sysinfo.release, sysinfo.machine);
#endif

    return system;
}

uint64_t hsSystemInfo::GetRAM()
{
#if defined(HS_BUILD_FOR_WIN32)
    ULONGLONG memoryInKB = 0;
    GetPhysicallyInstalledSystemMemory(&memoryInKB);
    hsAssert((memoryInKB % 1024) == 0, "Odd amount of memory?");
    return (uint64_t)(memoryInKB / 1024ULL);
#elif defined(HAVE_SYSINFO)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        uint64_t desiredUnit = 1024 * 1024;
        if (info.mem_unit != desiredUnit)
            return ((uint64_t)info.totalram * (uint64_t)info.mem_unit) / desiredUnit;
        return info.totalram;
    }
#elif defined(HAVE_SYSCTL)
    int mib[2]{ CTL_HW, HW_MEMSIZE };
    uint64_t memory = 0;
    size_t length = sizeof(memory);
    if (sysctl(mib, std::size(mib), &memory, &length, nullptr, 0) != 0)
        hsAssert(0, "shit");
    // This will probably subtract some amount of kernel memory, so don't assert on
    // offset amounts of memory.
    return memory / (uint64_t)(1024 * 1024);
#else
    static_assert(false, "hsSystemInfo::GetRAM() not implemented for this platform");
#endif
    return 0; // Yikes
}
