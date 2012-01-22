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
*   $/Plasma20/Sources/Plasma/CoreLibExe/hsExeMalloc.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Local constants
*
***/

#if defined(NO_MEM_TRACKER) || !defined(HS_FIND_MEM_LEAKS) || !defined(HS_BUILD_FOR_WIN32) || !defined(_MSC_VER)
 // no mem debugging
#else
# undef MEM_DEBUG
# define MEM_DEBUG
#endif

#ifndef MEM_DEBUG

# define _malloc_dbg(s, t, f, l)        malloc(s)
# define _calloc_dbg(c, s, t, f, l)     calloc(c, s)
# define _realloc_dbg(p, s, t, f, l)    realloc(p, s)
# define _expand_dbg(p, s, t, f, l)     _expand(p, s)
# define _free_dbg(p, t)                free(p)
# define _msize_dbg(p, t)               _msize(p)

# ifndef _CLIENT_BLOCK
#  define _CLIENT_BLOCK  0
# endif

# ifndef _IGNORE_BLOCK
#  define _IGNORE_BLOCK  0
# endif

# ifndef _CRTDBG_ALLOC_MEM_DF
#  define _CRTDBG_ALLOC_MEM_DF  0
# endif

# define  SET_CRT_DEBUG_FIELD(a)
# define  CLEAR_CRT_DEBUG_FIELD(a)

#endif  // !MEM_DEBUG


/*****************************************************************************
*
*   Private data
*
***/

#ifdef MEM_DEBUG
    #define  SET_CRT_DEBUG_FIELD(a) \
                _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
    #define  CLEAR_CRT_DEBUG_FIELD(a) \
                _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

    // From dbgint.h
    #define nNoMansLandSize 4
    typedef struct _CrtMemBlockHeader
    {
            struct _CrtMemBlockHeader * pBlockHeaderNext;
            struct _CrtMemBlockHeader * pBlockHeaderPrev;
            char *                      szFileName;
            int                         nLine;
    #ifdef _WIN64
            /* These items are reversed on Win64 to eliminate gaps in the struct
            * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
            * maintained in the debug heap.
            */
            int                         nBlockUse;
            size_t                      nDataSize;
    #else  /* _WIN64 */
            size_t                      nDataSize;
            int                         nBlockUse;
    #endif  /* _WIN64 */
            long                        lRequest;
            unsigned char               gap[nNoMansLandSize];
            /* followed by:
            *  unsigned char           data[nDataSize];
            *  unsigned char           anotherGap[nNoMansLandSize];
            */
    } _CrtMemBlockHeader;
    #define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
    #define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)


    enum EMemFile {
        kMemErr,
        kMemLeaks,
        kMemUsage,
        kMemAllocs,
        kNumMemFiles
    };

    static char *   s_memFilename[kNumMemFiles] = {
        "MemErr.log",
        "MemLeaks.log",
        "MemUsage.log",
        "MemAllocs.log",
    };

    static char *   s_memDlgTitle[kNumMemFiles] = {
        "Memory error",
        "Memory leak",
        nil,
        nil,
    };

    static HANDLE   s_memFile[kNumMemFiles] = {
        INVALID_HANDLE_VALUE,
        INVALID_HANDLE_VALUE,
        INVALID_HANDLE_VALUE,
        INVALID_HANDLE_VALUE,
    };

    struct MemDumpParam {
        EMemFile    file;
        bool        showDialog;
    };
    
    static unsigned s_memColor;
    static unsigned s_memCheckOff;

    static CCritSect * s_critsect;

#endif // MEM_DEBUG



namespace ExeMalloc {
/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
#ifdef MEM_DEBUG
static void ConvertFilename (
    const char  src[],
    unsigned    chars,
    char *      dst
) {

    // Because the filename field may point into a DLL that has been
    // unloaded, this code validates and converts the string into a
    // reasonable value
    __try {
        unsigned pos = 0;
        for (;;) {
            // If the file name is too long then assume it is bogus
            if (pos >= chars) {
                pos = 0;
                break;
            }

            // Get the next character
            unsigned chr = src[pos];
            if (!chr)
                break;

            // If the character isn't valid low-ASCII
            // then assume that the name is bogus
            if ((chr < 32) || (chr > 127)) {
                pos = 0;
                break;
            }

            // Store character
            dst[pos++] = (char) chr;
        }

        // Ensure that name is terminated
        dst[pos] = 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        dst[0] = 0;
    }

    // Print the address of the filename; it may be of some
    // use given the load address and the map file of the DLL
    if (!dst[0])
        snprintf(dst, chars, "@%p", src);
}
#endif  // MEM_DEBUG

//===========================================================================
#ifdef MEM_DEBUG
static void OpenErrorFile (EMemFile file) {
    ASSERT(INVALID_HANDLE_VALUE == s_memFile[file]);
    s_memFile[file] = CreateFile(
        s_memFilename[file],
        GENERIC_WRITE,
        FILE_SHARE_READ,
        (LPSECURITY_ATTRIBUTES) NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
}
#endif  // MEM_DEBUG

//===========================================================================
#ifdef MEM_DEBUG
static void __cdecl ReportMem (EMemFile file, bool showDialog, const char fmt[], ...) {

    if (s_memFile[file] == INVALID_HANDLE_VALUE) {
        DebugBreakIfDebuggerPresent();
        OpenErrorFile(file);
        ErrorMinimizeAppWindow();
    }

    char buffer[512];
    va_list args;
    va_start(args, fmt);
    DWORD length = hsVsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);

    #ifdef HS_BUILD_FOR_WIN32

        OutputDebugStringA(buffer);

        if (s_memFile[file] != INVALID_HANDLE_VALUE)
            WriteFile(s_memFile[file], buffer, length, &length, NULL);

        static bool s_skip;
        if (showDialog && !s_skip && !ErrorGetOption(kErrOptNonGuiAsserts)) {
            s_skip = IDOK != MessageBox(
                NULL,
                buffer,
                s_memDlgTitle[file],
                MB_ICONSTOP | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL
            );
        }

    #else

        fputs(buffer, stderr);

    #endif
}
#endif

//============================================================================
#ifdef MEM_DEBUG
static void __cdecl MemDumpCallback (void * mem, void * param) {
    const _CrtMemBlockHeader * pHead = pHdr(mem);
    MemDumpParam * dumpParam = (MemDumpParam *) param;
    
    char filename[MAX_PATH];
    ConvertFilename(
        pHead->szFileName,
        arrsize(filename),
        filename
    );

    // HACK: Don't report array memory leaks since these underly the hash
    // table type and may not be cleaned up until after the mem leak
    // checker runs.  =(    
    if (strstr(filename, "pnUtArray"))
        return;
        
    ReportMem(
        dumpParam->file,
        dumpParam->showDialog,
        "Offset %p size %u at %s:%d\r\n",
        mem,
        pHead->nDataSize,
        filename,
        pHead->nLine
    );
}
#endif // MEM_DEBUG

//============================================================================
#ifdef MEM_DEBUG
static void __cdecl OnExitMemDumpCallback (void * mem, size_t) {
    static MemDumpParam param = { kMemLeaks, true };
    if (!ErrorGetOption(kErrOptDisableMemLeakChecking))
        MemDumpCallback(mem, &param);
}
#endif // MEM_DEBUG

//===========================================================================
#ifdef MEM_DEBUG
static void __cdecl CheckLeaksOnExit () {
    if (!ErrorGetOption(kErrOptDisableMemLeakChecking)) {
        MemDumpParam param;
        param.file          = kMemLeaks;
        param.showDialog    = true;
        _CrtDoForAllClientObjects(MemDumpCallback, &param);
    }
}
#endif // MEM_DEBUG

//============================================================================
#ifdef MEM_DEBUG
static int __cdecl CrtAllocHook (
    int                     method,
    void *                  pUserData,
    size_t                  nSize,
    int                     nBlockUse,
    long                    lRequest,
    const unsigned char *   szFileName,
    int                     nLine
) {
    if (nBlockUse == _NORMAL_BLOCK) {
        int xx = 0;
    }
    
    return 1;
}
#endif // MEM_DEBUG

//===========================================================================
#ifdef MEM_DEBUG
AUTO_INIT_FUNC(hsExeMallocInit) {
    // The critical section has to be initialized
    // before program startup and never freed
    static uint8_t rawMemory[sizeof CCritSect];
    s_critsect = new(rawMemory) CCritSect;
    SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
    _CrtSetAllocHook(CrtAllocHook);
    _CrtSetDumpClient(OnExitMemDumpCallback);
//    atexit(CheckLeaksOnExit);
}
#endif // MEM_DEBUG


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void MemSetLeakChecking (bool on) {
    if (on)
        SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
    else
        CLEAR_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
}


} using namespace ExeMalloc;
/****************************************************************************
*
*   Exports
*
***/

//============================================================================
void MemDumpAllocReport () {
#ifdef MEM_DEBUG
    MemDumpParam param;
    param.file          = kMemAllocs;
    param.showDialog    = true;
    _CrtDoForAllClientObjects(MemDumpCallback, &param);
#endif // MEM_DEBUG
}

//============================================================================
void MemDumpUsageReport () {
#ifdef MEM_DEBUG
#endif // MEM_DEBUG
}

//============================================================================
void MemValidateNow () {
#ifdef MEM_DEBUG
#endif // MEM_DEBUG
}

//============================================================================
void MemSetValidation (unsigned on) {
#ifdef MEM_DEBUG
#endif // MEM_DEBUG
}

//============================================================================
void MemPushDisableTracking () {

#ifdef MEM_DEBUG
    ++s_memCheckOff;
#endif // MEM_DEBUG
}

//============================================================================
void MemPopDisableTracking () {

#ifdef MEM_DEBUG
    ASSERT(s_memCheckOff);
    --s_memCheckOff;
#endif // MEM_DEBUG
}

//============================================================================
void MemSetColor (unsigned short color) {
#ifdef MEM_DEBUG
    s_memColor = color & 0xFFFF;
#endif // MEM_DEBUG
}

//============================================================================
unsigned MemSize (void * ptr) {
    ASSERT(ptr);
    unsigned result = 0;

#ifdef MEM_DEBUG
    const _CrtMemBlockHeader * pHead = pHdr(ptr);
    unsigned block = pHead->nBlockUse;
#endif

#if HS_BUILD_FOR_WIN32
    result = (unsigned)_msize_dbg(ptr, block);
#endif
    return result;
}
