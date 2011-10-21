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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCrashExe/Intern.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCRASHEXE_INTERN_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnCrashExe/Intern.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCRASHEXE_INTERN_H


namespace Crash {


/*****************************************************************************
*
*   Mail
*
***/

void CrashSendEmail (
    const char smtp[],
    const char sender[],
    const char recipients[],
    const char replyTo[],
    const char username[],
    const char password[],
    const char programName[],
    const char errorType[],
    const char logBuffer[]
);



/*****************************************************************************
*
*   ImgHlp
*
***/

// make our own IMAGEAPI type because the real definition uses DECLSPEC_IMPORT
#define MYIMAGEAPI __stdcall

class CImageHelp {
private:
    HMODULE m_libInst;
    HANDLE  m_process;
    char *  m_appName;
    char    m_appPath[MAX_PATH];

public:
    CImageHelp (HINSTANCE instance);
    ~CImageHelp ();

    inline HANDLE Process () const {
        return m_process;
    }

    inline const char * GetProgramPath () const {
        return m_appPath;
    }

    inline const char * GetProgramName () const {
        return m_appName;
    }

    BOOL (MYIMAGEAPI * SymGetModuleInfo)(
        HANDLE hProcess,
        DWORD dwAddr,
        IMAGEHLP_MODULE * ModuleInfo
    );

    BOOL (MYIMAGEAPI * StackWalk)(
        DWORD MachineType,                                          
        HANDLE hProcess,                                            
        HANDLE hThread,                                             
        LPSTACKFRAME StackFrame,                                
        LPVOID ContextRecord,                                   
        PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,             
        PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,  
        PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,              
        PTRANSLATE_ADDRESS_ROUTINE TranslateAddress                 
    );

    LPVOID (MYIMAGEAPI * SymFunctionTableAccess)(
        HANDLE hProcess,  
        DWORD AddrBase    
    );

    DWORD (MYIMAGEAPI * SymGetModuleBase)(
        HANDLE hProcess,  
        DWORD dwAddr      
    );

    BOOL (MYIMAGEAPI * SymGetSymFromAddr)(
        HANDLE hProcess,             
        DWORD Address,               
        LPDWORD Displacement,       
        PIMAGEHLP_SYMBOL Symbol  
    );
};



} using namespace Crash;
