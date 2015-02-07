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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtStr.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSTR_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSTR_H

#include "Pch.h"
#include "pnUtArray.h"
#include <wchar.h>

// Got Damn eap...
// Duplicate Symbols in shlwapi!
#ifdef _INC_SHLWAPI
#   undef StrDup
#endif // _INC_SHLWAPI

/*****************************************************************************
*
*   String functions
*
***/

inline char  CharLowerFast (char  ch) { return ((ch >=  'A') && (ch <=  'Z')) ? (char )(ch +  'a' -  'A') : ch; }
inline wchar_t CharLowerFast (wchar_t ch) { return ((ch >= L'A') && (ch <= L'Z')) ? (wchar_t)(ch + L'a' - L'A') : ch; }

unsigned StrPrintf (char * dest, unsigned count, const char format[], ...);
unsigned StrPrintf (wchar_t * dest, unsigned count, const wchar_t format[], ...);

unsigned StrPrintfV (char * dest, unsigned count, const char format[], va_list args);
unsigned StrPrintfV (wchar_t * dest, unsigned count, const wchar_t format[], va_list args);

unsigned StrLen (const char str[]);
unsigned StrLen (const wchar_t str[]);

char * StrDup (const char str[]);
wchar_t * StrDup (const wchar_t str[]);

int StrCmp (const char str1[], const char str2[], unsigned chars = (unsigned)-1);
int StrCmp (const wchar_t str1[], const wchar_t str2[], unsigned chars = (unsigned)-1);

int StrCmpI (const char str1[], const char str2[], unsigned chars = (unsigned)-1);
int StrCmpI (const wchar_t str1[], const wchar_t str2[], unsigned chars = (unsigned)-1);

void StrCopy (char * dest, const char source[], unsigned chars);
void StrCopy (wchar_t * dest, const wchar_t source[], unsigned chars);

uint32_t StrHash (const char str[], unsigned chars = (unsigned)-1);
uint32_t StrHash (const wchar_t str[], unsigned chars = (unsigned)-1);

uint32_t StrHashI (const char str[], unsigned chars = (unsigned)-1);
uint32_t StrHashI (const wchar_t str[], unsigned chars = (unsigned)-1);
#endif
