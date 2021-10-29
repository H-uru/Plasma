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

#include "plStackWalker.h"

#include "hsWindows.h"
#include <DbgHelp.h>

#include <string_theory/stdio>
#include <tuple>

// ===================================================

constexpr DWORD kMachineType = sizeof(void*) == 8 ? IMAGE_FILE_MACHINE_AMD64 : IMAGE_FILE_MACHINE_I386;

// ===================================================

plStackWalker::plStackWalker(plStackWalker::Process process, plStackWalker::Thread thread, void* context, size_t levels)
    : fProcess(process), fThread(thread), fContext(context), fNumLevels(levels)
{
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitializeW(fProcess, nullptr, TRUE);
}

plStackWalker::~plStackWalker()
{
    SymCleanup(fProcess);
}

// ===================================================

struct plWindowsStackFrame : plStackFrame
{
    STACKFRAME64 fFrame;

    plWindowsStackFrame() = delete;
    plWindowsStackFrame(const LPCONTEXT ctx)
        : fFrame()
    {
        fFrame.AddrStack.Mode = AddrModeFlat;
        fFrame.AddrFrame.Mode = AddrModeFlat;
        fFrame.AddrPC.Mode = AddrModeFlat;
#ifdef _M_X64
        fFrame.AddrPC.Offset = ctx->Rip;
        fFrame.AddrStack.Offset = ctx->Rsp;
        fFrame.AddrFrame.Offset = ctx->Rbp;
#else
        fFrame.AddrPC.Offset = ctx->Eip;
        fFrame.AddrStack.Offset = ctx->Esp;
        fFrame.AddrFrame.Offset = ctx->Ebp;
#endif
    }

    static STACKFRAME64* Get(const std::shared_ptr<plStackFrame>& frame)
    {
        return &(static_cast<plWindowsStackFrame*>(frame.get())->fFrame);
    }
};

// ===================================================

plStackWalker::iterator plStackWalker::cbegin() const
{
    LPCONTEXT ctx = reinterpret_cast<LPCONTEXT>(fContext);
    plStackWalker::iterator it(this, std::make_shared<plWindowsStackFrame>(ctx), 0);
    if (it.next())
        return it;
    return cend();
}

bool plStackWalker::iterator::next()
{
    if (!(fFrame && fLevelIdx < fThis->fNumLevels))
        return false;

    STACKFRAME64* frame = plWindowsStackFrame::Get(fFrame);

    BOOL result = StackWalk64(
        kMachineType,
        fThis->fProcess,
        fThis->fThread,
        frame,
        fThis->fContext,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    // StackWalk64 doesn't set the last error code, so just bail on falsey returns.
    if (result == FALSE || frame->AddrReturn.Offset == 0)
        return false;
    return true;
}

// ===================================================

class plStackWalkError : public std::runtime_error
{
public:
    plStackWalkError() = default;
    explicit plStackWalkError(const char* msg) : std::runtime_error(msg) { }
    explicit plStackWalkError(const ST::string& msg) : std::runtime_error(msg.c_str()) { }
};

class plStackWalkBufferTooSmall : public std::exception
{
    size_t fHint;

public:
    plStackWalkBufferTooSmall() = delete;
    explicit plStackWalkBufferTooSmall(size_t hint)
        : fHint(hint)
    { }

    size_t Hint() const { return fHint; }
};

// ===================================================

static inline ST::string IGetLastErrorString()
{
    wchar_t* msg = nullptr;
    auto result = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msg, 0, nullptr);
    if (result && msg) {
        ST::char_buffer utf8 = ST::string::from_wchar(msg, result, ST::assume_valid).to_utf8();
        LocalFree(msg);
        return utf8;
    } else {
        return ST::format("unknown error 0x{8X}", GetLastError());
    }
}

static inline ST::string IGetModuleName(HANDLE process, DWORD64 address)
{
    // Microsoft warns that this structure has changed over the years and that
    // this call could fail due to the DbgHelp.dll being older than the Windows SDK
    // used. As of Windows 10 21H1, the struct hasn't changed since "15-Jul-2009" -
    // Windows 7 was RTM one week later. So, we'll accept the risk. Worst case scenario,
    // we just don't have module names in our stack walks. Seems fine to me.
    IMAGEHLP_MODULEW64 info{};
    info.SizeOfStruct = sizeof(info);
    if (!SymGetModuleInfoW64(process, address, &info))
        throw plStackWalkError(ST::format("failed to get module name: {}", IGetLastErrorString()));
    return ST::string::from_wchar(info.ModuleName);
}

static inline std::tuple<ST::string, ST::wchar_buffer, DWORD64> IGetMangledSymbol(HANDLE process, DWORD64 addr, void* buf, size_t bufsz)
{
    if (bufsz < sizeof(SYMBOL_INFOW))
        throw plStackWalkError("symbol info buffer too small");

    SYMBOL_INFOW* info = reinterpret_cast<SYMBOL_INFOW*>(buf);
    memset(info, 0, bufsz);
    info->SizeOfStruct = sizeof(SYMBOL_INFOW);
    // The first character of the symbol name is in the SYMBOL_INFOW struct.
    info->MaxNameLen = (ULONG)((bufsz - sizeof(SYMBOL_INFOW)) / sizeof(wchar_t)) + 1;

    DWORD64 displacement = 0;
    if (!SymFromAddrW(process, addr, &displacement, info))
        throw plStackWalkError(ST::format("failed to lookup symbol: {}", IGetLastErrorString()));

    // Need to do it again on the heap to get the full name.
    // This should hopefully never happen.
    if (info->NameLen > info->MaxNameLen)
        throw plStackWalkBufferTooSmall(info->NameLen);

    return std::make_tuple(
        IGetModuleName(process, info->Address),
        ST::wchar_buffer(info->Name, info->NameLen),
        displacement
    );
}

static inline std::tuple<ST::string, ST::wchar_buffer, DWORD64> IGetMangledSymbol(HANDLE process, DWORD64 addr)
{
    constexpr size_t NAME_EXTRA_SIZE = 1023 * sizeof(wchar_t);
    constexpr size_t STACK_BUF_SIZE = sizeof(SYMBOL_INFOW) + NAME_EXTRA_SIZE;
    uint8_t symbolBuf[STACK_BUF_SIZE];
    try {
        return IGetMangledSymbol(process, addr, symbolBuf, STACK_BUF_SIZE);
    } catch (const plStackWalkBufferTooSmall& ex) {
        size_t heapBufsz = sizeof(SYMBOL_INFOW) + (ex.Hint() * sizeof(wchar_t));
        auto heapBuf = std::make_unique<uint8_t[]>(heapBufsz);
        return IGetMangledSymbol(process, addr, heapBuf.get(), heapBufsz);
    }
}

static inline std::tuple<ST::string, ST::string> IGetStackAddress(HANDLE process, DWORD64 addr)
{
    class plModuleHeaders
    {
        HANDLE fFile;
        HANDLE fMapping;
        LPVOID fImageBase;
        PIMAGE_NT_HEADERS fHeaders;

    public:
        plModuleHeaders() = delete;
        plModuleHeaders(const plModuleHeaders&) = delete;
        plModuleHeaders(plModuleHeaders&&) = delete;

        plModuleHeaders(HANDLE process, const IMAGEHLP_MODULEW64& info)
            : fFile(), fMapping(), fImageBase(), fHeaders()
        {
            if (process == GetCurrentProcess()) {
                fImageBase = (LPVOID)info.BaseOfImage;
            } else {
                // This is an unlikely code path, so I see no need to prematurely optimize it.
                fFile = CreateFileW(
                    info.ImageName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr
                );
                if (fFile == INVALID_HANDLE_VALUE) {
                    throw plStackWalkError(
                        ST::format(
                            "couldn't CreateFile for module {}: {}",
                            info.ImageName,
                            IGetLastErrorString()
                        )
                    );
                }

                fMapping = CreateFileMappingW(
                    fFile,
                    nullptr,
                    PAGE_READONLY,
                    0,
                    0,
                    nullptr
                );
                if (fMapping == nullptr) {
                    throw plStackWalkError(
                        ST::format(
                            "couldn't CreateFileMapping for module {}: {}",
                            info.ImageName,
                            IGetLastErrorString()
                        )
                    );
                }

                fImageBase = MapViewOfFile(
                    fMapping,
                    FILE_MAP_READ,
                    0,
                    0,
                    0
                );
                if (fImageBase == nullptr) {
                    throw plStackWalkError(
                        ST::format(
                            "couldn't MapViewOfFile for module {}: {}",
                            info.ImageName,
                            IGetLastErrorString()
                        )
                    );
                }
            }

            fHeaders = ImageNtHeader(fImageBase);
            if (fHeaders == nullptr) {
                throw plStackWalkError(
                    ST::format(
                        "couldn't get image header for module {}: {}",
                        info.ImageName,
                        IGetLastErrorString()
                    )
                );
            }
        }

        ~plModuleHeaders()
        {
            if (fMapping) {
                UnmapViewOfFile(fImageBase);
                CloseHandle(fMapping);
            }
            if (fFile && fFile != INVALID_HANDLE_VALUE)
                CloseHandle(fFile);
        }

        const IMAGE_NT_HEADERS* operator ->() const { return fHeaders; }
    };

    // See caveat in IGetModuleName().
    IMAGEHLP_MODULEW64 info{};
    info.SizeOfStruct = sizeof(info);
    if (SymGetModuleInfoW64(process, addr, &info)) {
        // Toss the ASLRandomized image base and add the desired base so these addresses
        // can be thrown at something like windbg or IDA without requiring mental gymnastics.
        addr -= info.BaseOfImage;
        try {
            plModuleHeaders headers(process, info);
            addr += headers->OptionalHeader.ImageBase;
            return std::make_tuple(
                ST::string::from_wchar(info.ModuleName),
                ST::format("0x{08X}", addr)
            );
        } catch (const plStackWalkError& ex) {
            ST::printf(stderr, "unable to virtualize address: {}", ex.what());
            return std::make_tuple(
                ST::string::from_wchar(info.ModuleName),
                ST::format("0x{08X} (offset)", addr)
            );
        }
    } else {
        ST::printf(stderr, "failed to get module info: {}", IGetLastErrorString());
    }

    return std::make_tuple(
        ST::string::from_wchar(info.ModuleName),
        ST::format("0x{08X} (process space)", addr)
    );
}

static inline ST::string IUnmangleSymbol(const ST::wchar_buffer& name)
{
    wchar_t stackBuf[1024];
    DWORD result = UnDecorateSymbolNameW(name.data(), stackBuf, std::size(stackBuf), UNDNAME_COMPLETE);
    if (result == 0)
        throw plStackWalkError(IGetLastErrorString());
    return ST::string::from_wchar(stackBuf, result);
}

static inline std::optional<std::tuple<plFileName, DWORD>> IGetLineInfo(HANDLE process, DWORD64 addr)
{
    std::optional<std::tuple<plFileName, DWORD>> retval;

    IMAGEHLP_LINEW64 line{};
    line.SizeOfStruct = sizeof(line);
    DWORD displacement;
    if (SymGetLineFromAddrW64(process, addr, &displacement, &line))
        retval = std::make_tuple(ST::string::from_wchar(line.FileName), line.LineNumber);
    return retval;
}

plStackEntry plStackWalker::iterator::get() const
{
    plStackEntry entry;
    if (!fFrame)
        return entry;

    const STACKFRAME64* frame = plWindowsStackFrame::Get(fFrame);

    ST::wchar_buffer symbol;
    try {
        std::tie(entry.fModuleName, symbol, entry.fOffset) = IGetMangledSymbol(fThis->fProcess, frame->AddrPC.Offset);
    } catch (const plStackWalkError& ex) {
        ST::printf(stderr, "Failed to get symbol for level {}: {}", fLevelIdx, ex.what());
        std::tie(entry.fModuleName, entry.fFunctionName) = IGetStackAddress(fThis->fProcess, frame->AddrPC.Offset);
        return entry;
    }

    try {
        entry.fFunctionName = IUnmangleSymbol(symbol);
    } catch (const plStackWalkError& ex) {
        ST::printf(stderr, "Failed to unmangle symbol {} on level {}: {}", symbol, fLevelIdx, ex.what());
        entry.fFunctionName = symbol;
    }

    // The PDB may not contain this information, so don't even complain if it's missing.
    auto lineInfo = IGetLineInfo(fThis->fProcess, frame->AddrPC.Offset);
    if (lineInfo.has_value())
        std::tie(entry.fFileName, entry.fLine) = lineInfo.value();

    return entry;
}
