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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCrashExe/pnCreTools.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


//============================================================================
namespace Crash {
//============================================================================


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static BOOL MYIMAGEAPI iSymGetModuleInfo (
    HANDLE,
    DWORD,
    IMAGEHLP_MODULE * ModuleInfo
) {
    memset(ModuleInfo, 0, sizeof(*ModuleInfo));
    return false;
}

//===========================================================================
static BOOL MYIMAGEAPI iStackWalk (
    DWORD,
    HANDLE,
    HANDLE,
    LPSTACKFRAME,
    LPVOID,
    PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE,
    PGET_MODULE_BASE_ROUTINE,
    PTRANSLATE_ADDRESS_ROUTINE
) {
    return false;
}

//===========================================================================
static LPVOID MYIMAGEAPI iSymFunctionTableAccess (
    HANDLE,
    DWORD
) {
    return nil;
}

//===========================================================================
static DWORD MYIMAGEAPI iSymGetModuleBase (
    HANDLE,  
    DWORD
) {
    return 0;
}

//===========================================================================
static BOOL MYIMAGEAPI iSymGetSymFromAddr (
    HANDLE,
    DWORD,
    LPDWORD Displacement,       
    PIMAGEHLP_SYMBOL Symbol  
) {
    *Displacement = 0;
    memset(Symbol, 0, sizeof(*Symbol));
    return false;
}


/*****************************************************************************
*
*   CImageHelp
*
***/

//============================================================================
CImageHelp::CImageHelp (HINSTANCE instance)
:   m_libInst(nil),
    m_process(nil)
{
    // get program directory and filename
    if (!GetModuleFileNameA(instance, m_appPath, arrsize(m_appPath)))
        m_appPath[0] = 0;
    if (nil !=  (m_appName = StrChrR(m_appPath, '\\')))
        *m_appName++ = '\0';
    else
        m_appName = m_appPath;

    // initialize symbols
    for (;;) {
        if (nil ==  (m_libInst = LoadLibrary("ImageHlp.dll")))
            break;

        // initialize symbols for this process
        BOOL (MYIMAGEAPI * SymInitialize)(HANDLE hProcess, char * path, BOOL fInvade);
        * (FARPROC *) &SymInitialize = GetProcAddress(m_libInst, "SymInitialize");
        if (!SymInitialize)
            break;

        HANDLE hProcess = GetCurrentProcess();
        if (!SymInitialize(hProcess, m_appPath, false))
            break;

        // import functions
        FARPROC proc;
        #define BIND(x) * (FARPROC *) &(x) = ((proc = GetProcAddress(m_libInst, #x)) != nil) ?  proc : (FARPROC) i##x
        BIND(SymGetModuleInfo);
        BIND(StackWalk);
        BIND(SymFunctionTableAccess);
        BIND(SymGetModuleBase);
        BIND(SymGetSymFromAddr);
        #undef BIND

        // success
        m_process = hProcess;
        return;
    }

    // failure!
    if (m_libInst) {
        FreeLibrary(m_libInst);
        m_libInst = nil;
    }

    #define INIT(x) x = i##x
    INIT(SymGetModuleInfo);
    INIT(StackWalk);
    INIT(SymFunctionTableAccess);
    INIT(SymGetModuleBase);
    INIT(SymGetSymFromAddr);
    #undef INIT
}

//============================================================================
CImageHelp::~CImageHelp () {
    if (m_process) {
        BOOL (MYIMAGEAPI * SymCleanup)(HANDLE hProcess);
        * (FARPROC *) &SymCleanup = GetProcAddress(m_libInst, "SymCleanup");
        if (SymCleanup)
            SymCleanup(m_process);
        m_process = 0;
    }

    if (m_libInst) {
        FreeLibrary(m_libInst);
        m_libInst = 0;
    }
}



//============================================================================
} // namespace Crash
//============================================================================
