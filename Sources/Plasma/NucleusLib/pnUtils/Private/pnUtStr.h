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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtStr.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSTR_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtStr.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSTR_H


/*****************************************************************************
*
*   String functions
*
***/

inline char  CharLowerFast (char  ch) { return ((ch >=  'A') && (ch <=  'Z')) ? (char )(ch +  'a' -  'A') : ch; }
inline wchar CharLowerFast (wchar ch) { return ((ch >= L'A') && (ch <= L'Z')) ? (wchar)(ch + L'a' - L'A') : ch; }

inline char  CharUpperFast (char  ch) { return ((ch >=  'a') && (ch <=  'z')) ? (char )(ch +  'A' -  'a') : ch; }
inline wchar CharUpperFast (wchar ch) { return ((ch >= L'a') && (ch <= L'z')) ? (wchar)(ch + L'A' - L'a') : ch; }

unsigned StrBytes (const char str[]);   // includes space for terminator
unsigned StrBytes (const wchar str[]);  // includes space for terminator

char * StrChr (char * str, char ch, unsigned chars = (unsigned)-1);
wchar * StrChr (wchar * str, wchar ch, unsigned chars = (unsigned)-1);
const char * StrChr (const char str[], char ch, unsigned chars = (unsigned)-1);
const wchar * StrChr (const wchar str[], wchar ch, unsigned chars = (unsigned)-1);

unsigned StrPrintf (char * dest, unsigned count, const char format[], ...);
unsigned StrPrintf (wchar * dest, unsigned count, const wchar format[], ...);

unsigned StrPrintfV (char * dest, unsigned count, const char format[], va_list args);
unsigned StrPrintfV (wchar * dest, unsigned count, const wchar format[], va_list args);

unsigned StrLen (const char str[]);
unsigned StrLen (const wchar str[]);

char * StrDup (const char str[]);
wchar * StrDup (const wchar str[]);

char * StrDupLen (const char str[], unsigned chars);
wchar * StrDupLen (const wchar str[], unsigned chars);

wchar * StrDupToUnicode (const char str[]);
char * StrDupToAnsi (const wchar str[]);

int StrCmp (const char str1[], const char str2[], unsigned chars = (unsigned)-1);
int StrCmp (const wchar str1[], const wchar str2[], unsigned chars = (unsigned)-1);

int StrCmpI (const char str1[], const char str2[], unsigned chars = (unsigned)-1);
int StrCmpI (const wchar str1[], const wchar str2[], unsigned chars = (unsigned)-1);

char * StrStr (char * source, const char match[]);
const char * StrStr (const char source[], const char match[]);
wchar * StrStr (wchar * source, const wchar match[]);
const wchar * StrStr (const wchar source[], const wchar match[]);

char * StrStrI (char * source, const char match[]);
const char * StrStrI (const char source[], const char match[]);
wchar * StrStrI (wchar * source, const wchar match[]);
const wchar * StrStrI (const wchar source[], const wchar match[]);

char * StrChrR (char * str, char ch);
wchar * StrChrR (wchar * str, wchar ch);
const char * StrChrR (const char str[], char ch);
const wchar * StrChrR (const wchar str[], wchar ch);

void StrCopy (char * dest, const char source[], unsigned chars);
void StrCopy (wchar * dest, const wchar source[], unsigned chars);

unsigned StrCopyLen (char * dest, const char source[], unsigned chars);
unsigned StrCopyLen (wchar * dest, const wchar source[], unsigned chars);

void StrPack (char * dest, const char source[], unsigned chars);
void StrPack (wchar * dest, const wchar source[], unsigned chars);

unsigned StrToAnsi (char * dest, const wchar source[], unsigned destChars);
unsigned StrToAnsi (char * dest, const wchar source[], unsigned destChars, unsigned codePage);

unsigned StrToUnicode (wchar * dest, const char source[], unsigned destChars);
unsigned StrToUnicode (wchar * dest, const char source[], unsigned destChars, unsigned codePage);

unsigned StrUnicodeToUtf8 (char * dest, const wchar source[], unsigned destChars);
unsigned StrUtf8ToUnicode (wchar * dest, const char source[], unsigned destChars);

float StrToFloat (const char source[], const char ** endptr);
float StrToFloat (const wchar source[], const wchar ** endptr);

int StrToInt (const char source[], const char ** endptr);
int StrToInt (const wchar source[], const wchar ** endptr);

unsigned StrToUnsigned (char source[], char ** endptr, int radix);
unsigned StrToUnsigned (wchar source[], wchar ** endptr, int radix);
unsigned StrToUnsigned (const char source[], const char ** endptr, int radix);
unsigned StrToUnsigned (const wchar source[], const wchar ** endptr, int radix);

void StrLower (char * dest, unsigned chars = (unsigned) -1);
void StrLower (wchar * dest, unsigned chars = (unsigned) -1);
void StrLower (char * dest, const char source[], unsigned chars);
void StrLower (wchar * dest, const wchar source[], unsigned chars);

dword StrHash (const char str[], unsigned chars = (unsigned)-1);
dword StrHash (const wchar str[], unsigned chars = (unsigned)-1);

dword StrHashI (const char str[], unsigned chars = (unsigned)-1);
dword StrHashI (const wchar str[], unsigned chars = (unsigned)-1);

bool StrTokenize (const char * source[], char * dest, unsigned chars, const char whitespace[], unsigned maxWhitespaceSkipCount = (unsigned)-1);
bool StrTokenize (const wchar * source[], wchar * dest, unsigned chars, const wchar whitespace[], unsigned maxWhitespaceSkipCount = (unsigned)-1);
bool StrTokenize (const char * source[], ARRAY(char) * destArray, const char whitespace[], unsigned maxWhitespaceSkipCount = (unsigned)-1);
bool StrTokenize (const wchar * source[], ARRAY(wchar) * destArray, const wchar whitespace[], unsigned maxWhitespaceSkipCount = (unsigned)-1);
