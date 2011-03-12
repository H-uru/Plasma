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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCrashExe/Win32/pnCreMail.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


//============================================================================
namespace Crash {
//============================================================================


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static bool CreateInheritablePipe (
	HANDLE *	read, 
	HANDLE *	write,
	bool		inheritRead
) {
	// create pipe for std error read/write
	if (!CreatePipe(read, write, (LPSECURITY_ATTRIBUTES) NULL, 0))
		return false;

	// make the appropriate handle inheritable
	HANDLE hProcess = GetCurrentProcess();
	HANDLE * inherit = inheritRead ? read : write;
	bool result = DuplicateHandle(
		hProcess,
		*inherit, 
		hProcess,
		inherit,
		0,
		true,   // make inheritable
		DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
	);
	if (!result)
		return false;

    return true;
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
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
) {
	enum {
		IN_CHILD,
		IN_PARENT,
		NUM_PIPES
	};

	unsigned i;
	
	HANDLE pipes[NUM_PIPES];
	for (i = 0; i < arrsize(pipes); ++i)
		pipes[i] = INVALID_HANDLE_VALUE;


	for (;;) {
		// create pipes for Server -> StdIn -> Client
		if (!CreateInheritablePipe(&pipes[IN_CHILD], &pipes[IN_PARENT], true))
			break;
	
		char subject[512];
		StrPrintf(
			subject,
			arrsize(subject),
			"\"[Crash] %s, %s\"",
			programName,
			errorType
		);

		char cmdLine[512];
		StrPrintf(
			cmdLine,
			arrsize(cmdLine),
			"plMail.exe -smtp %s -sender %s",
			smtp,
			sender
		);
		
		if (replyTo && replyTo[0]) {
			StrPack(cmdLine, " -replyTo ", arrsize(cmdLine));
			StrPack(cmdLine, replyTo, arrsize(cmdLine));
		}
		
		if (username && username[0]) {
			StrPack(cmdLine, " -u ", arrsize(cmdLine));
			StrPack(cmdLine, username, arrsize(cmdLine));
		}

		if (password && password[0]) {
			StrPack(cmdLine, " -p ", arrsize(cmdLine));
			StrPack(cmdLine, password, arrsize(cmdLine));
		}
		
		StrPack(cmdLine, " ", arrsize(cmdLine));
		StrPack(cmdLine, recipients, arrsize(cmdLine));

		StrPack(cmdLine, " ", arrsize(cmdLine));
		StrPack(cmdLine, subject, arrsize(cmdLine));

		// create process
		STARTUPINFO si;
		ZERO(si);
		si.cb           = sizeof(si);
		si.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow  = SW_HIDE;
		si.hStdInput    = pipes[IN_CHILD];
		si.hStdOutput   = INVALID_HANDLE_VALUE;
		si.hStdError    = INVALID_HANDLE_VALUE;
		PROCESS_INFORMATION pi;
		BOOL result = CreateProcess(
			NULL,
			cmdLine,
			(LPSECURITY_ATTRIBUTES) NULL,
			(LPSECURITY_ATTRIBUTES) NULL,
			true,      // => inherit handles
			NORMAL_PRIORITY_CLASS,
			NULL,
			NULL,
			&si,
			&pi
		);
		if (!result)
			break;
			
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Write output data
		DWORD written, length = StrLen(logBuffer);
		WriteFile(pipes[IN_PARENT], logBuffer, length, &written, NULL);

		// complete
		break;
    }

    // cleanup pipes
    for (i = 0; i < arrsize(pipes); ++i) {
		if (pipes[i] != INVALID_HANDLE_VALUE)
			CloseHandle(pipes[i]);
    }
}



//============================================================================
} // namespace Crash
//============================================================================
