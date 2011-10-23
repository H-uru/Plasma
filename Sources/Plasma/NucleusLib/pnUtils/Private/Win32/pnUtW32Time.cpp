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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Time.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void FormatTime (
    qword       time,
    wchar const dateFmt[],
    wchar const timeFmt[],
    unsigned    chars,
    wchar *     buffer
) {
    COMPILER_ASSERT(sizeof(FILETIME) == sizeof(qword));

    SYSTEMTIME sysTime;
    FileTimeToSystemTime((FILETIME *)&time, &sysTime);

    unsigned offset = GetDateFormatW(
        LOCALE_SYSTEM_DEFAULT,
        0,
        &sysTime,
        dateFmt,
        buffer,
        chars
    );

    if (timeFmt) {
        // if we printed any characters, move offset back to overwrite the string terminator
        if (offset)
        --offset;

        offset += GetTimeFormatW(
            LOCALE_SYSTEM_DEFAULT,
            0,
            &sysTime,
            timeFmt,
            buffer + offset,
            chars - offset
        );
    }

    // if we didn't print any characters, NULL terminate the buffer
    if (!offset && chars)
        buffer[0] = 0;
}


/*****************************************************************************
*
*   Exported functions
*
***/

#ifdef HS_BUILD_FOR_WIN32

//===========================================================================
void TimeGetDesc (
    qword       time,
    TimeDesc *  desc
) {
    ASSERT(desc);

    SYSTEMTIME sysTime;
    COMPILER_ASSERT(sizeof(qword) == sizeof(FILETIME));
    FileTimeToSystemTime((FILETIME *) &time, &sysTime);

    desc->year      = sysTime.wYear;
    desc->month     = sysTime.wMonth;
    desc->day       = sysTime.wDay;
    desc->dayOfWeek = sysTime.wDayOfWeek;
    desc->hour      = sysTime.wHour;
    desc->minute    = sysTime.wMinute;
    desc->second    = sysTime.wSecond;
}

//============================================================================
qword TimeGetTime () {
    qword time;
    COMPILER_ASSERT(sizeof(qword) == sizeof(FILETIME));
    GetSystemTimeAsFileTime((FILETIME *) &time);
    return time;
}

//============================================================================
qword TimeGetLocalTime () {
    qword time;
    COMPILER_ASSERT(sizeof(qword) == sizeof(FILETIME));
    GetSystemTimeAsFileTime((FILETIME *) &time);
    FileTimeToLocalFileTime((FILETIME *) &time, (FILETIME *) &time);
    return time;
}

//============================================================================
void TimePrettyPrint (
    qword       time,
    unsigned    chars,
    wchar *     buffer
) {
    FormatTime(
        time,
        L"ddd MMM dd',' yyyy ",
        L"hh':'mm':'ss tt",
        chars,
        buffer
    );
}


#endif // HS_BUILD_FOR_WIN32
