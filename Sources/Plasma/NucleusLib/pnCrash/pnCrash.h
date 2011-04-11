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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCrash/pnCrash.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCRASH_PNCRASH_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCRASH_PNCRASH_H


/*****************************************************************************
*
*   Crash API
*
***/

void CrashExceptionDump (const char occasion[], void * info);

void CrashSetEmailParameters (
    const char      smtp[],
    const char      sender[],
    const char      recipients[],   // separate multiple recipients with semicolons
    const char      username[],     // optional (see auth notes in pnMail.h)
    const char      password[],     // optional (see auth notes in pnMail.h)
    const char      replyTo[]		// optional
);

void * CrashAddModule (
	unsigned_ptr	address,
	unsigned		buildId,
	unsigned		branchId,
	const wchar		name[],
	const wchar		buildString[]
);

void CrashRemoveModule (
	void *			module
);


/*****************************************************************************
*
*   Deadlock detection (server only)
*
***/

#ifdef SERVER
void * CrashAddDeadlockCheck (void * thread, const wchar debugstr[] );
void CrashRemoveDeadlockCheck (void * check);
void CrashSetDeadlockCheckTimes (unsigned emailSec, unsigned terminateSec);
// returns previous setting
bool CrashEnableDeadlockChecking (bool enable);
void CrashDeadlockCheckNow ();
void CrashSendThreadReport ();
#endif


#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCRASH_PNCRASH_H
