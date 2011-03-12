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
#ifndef hsUtils_Defined
#define hsUtils_Defined

#include "HeadSpin.h"

#include <string.h>
#include <ctype.h>
#include <stdarg.h>

int		hsStrlen(const char src[]);
char*	hsStrcpy(char dstOrNil[], const char src[]);
void	hsStrcat(char dst[], const char src[]);
hsBool	hsStrEQ(const char s1[], const char s2[]);
hsBool	hsStrCaseEQ(const char* s1, const char* s2);
char*	hsScalarToStr(hsScalar);
int     hsRemove(const char* filename);
void 	hsCPathToMacPath(char* dst, char* fname);	
void 	hsStrLower(char *s);
char *	hsFormatStr(const char * fmt, ...);	// You are responsible for returned memory.
char *	hsFormatStrV(const char * fmt, va_list args);	// You are responsible for returned memory.


//	A pstring has a length byte at the beginning, and no trailing 0
char*	hsP2CString(const UInt8 pstring[], char cstring[]);
UInt8*	hsC2PString(const char cstring[], UInt8 pstring[]);

inline char* hsStrcpy(const char src[])
{
	return hsStrcpy(nil, src);
}

inline char *hsStrncpy(char *strDest, const char *strSource, size_t count)
{
    char *temp = strncpy(strDest, strSource, count-1);
    strDest[count-1] = 0;
    return temp;
}

wchar_t	*hsStringToWString( const char *str );
void	hsStringToWString( wchar_t *dst, const char *src );
char	*hsWStringToString( const wchar_t *str );
void	hsWStringToString( char *dst, const wchar_t *src);

enum {				// Kind of MessageBox...passed to hsMessageBox
	hsMessageBoxAbortRetyIgnore,
	hsMessageBoxNormal,				// Just Ok
	hsMessageBoxOkCancel,
	hsMessageBoxRetryCancel,
	hsMessageBoxYesNo,
	hsMessageBoxYesNoCancel,
};

enum {
	hsMessageBoxIconError,
	hsMessageBoxIconQuestion,
	hsMessageBoxIconExclamation,
	hsMessageBoxIconAsterisk,
};

enum {			// RETURN VALUES FROM hsMessageBox
	hsMBoxOk = 1,		// OK button was selected. 
	hsMBoxCancel,	// Cancel button was selected. 
	hsMBoxAbort,	// Abort button was selected. 
	hsMBoxRetry,	// Retry button was selected. 
	hsMBoxIgnore,	// Ignore button was selected. 
	hsMBoxYes,		// Yes button was selected. 
	hsMBoxNo		// No button was selected. 
};

extern bool hsMessageBox_SuppressPrompts;
int hsMessageBox(const char message[], const char caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBox(const wchar_t message[], const wchar_t caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBoxWithOwner(void* owner, const char message[], const char caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBoxWithOwner(void* owner, const wchar_t message[], const wchar_t caption[], int kind, int icon=hsMessageBoxIconAsterisk);

inline hsBool hsCompare(hsScalar a, hsScalar b, hsScalar delta=0.0001)
{
	return (fabs(a - b) < delta);
}

// flag testing / clearing
#define hsCheckBits(f,c) ((f & c)==c)
#define hsTestBits(f,b)  ( (f) & (b)   )
#define hsSetBits(f,b)   ( (f) |= (b)  )
#define hsClearBits(f,b) ( (f) &= ~(b) )
#define hsToggleBits(f,b) ( (f) ^= (b) )
#define hsChangeBits(f,b,t) ( t ? hsSetBits(f,b) : hsClearBits(f,b) )


#if HS_BUILD_FOR_WIN32
#define hsVsnprintf	_vsnprintf
#define hsVsnwprintf _vsnwprintf
#define snprintf _snprintf
#define snwprintf _snwprintf
#define hsSnprintf snprintf
#define hsSnwprintf snwprintf
#else
#define hsVsnprintf	vsnprintf
#define hsWvnwprintf vsnwprintf
#define hsSnprintf snprintf
#define hsSnwprintf snwprintf
#define _snprintf snprintf
#define _snwprintf snwprintf
#endif


#if HS_BUILD_FOR_UNIX || HS_BUILD_FOR_PS2

#define _stricmp(s1, s2)		strcasecmp(s1, s2)
#define _strnicmp(s1, s2, n)	strncasecmp(s1, s2, n)
#define stricmp(s1, s2)		strcasecmp(s1, s2)
#define strnicmp(s1, s2, n)	strncasecmp(s1, s2, n)

#define _fileno(n) fileno(n)


#elif HS_BUILD_FOR_MAC //  HS_BUILD_FOR_UNIX || HS_BUILD_FOR_PS2

int	hsStrcasecmp(const char s1[], const char s2[]);
int	hsStrncasecmp(const char s1[], const char s2[], int n);

#define _stricmp(s1, s2)		hsStrcasecmp(s1, s2)
#define _strnicmp(s1, s2, n)	hsStrncasecmp(s1, s2, n)

#endif  // HS_BUILD_FOR_UNIX || HS_BUILD_FOR_PS2

/////////////////////////////
// Physical memory functions
/////////////////////////////
enum MemSpec
{
	kBlows = 0,		// Less than 128
	kAcceptable,	// Less than 256
	kOptimal		// 256 or greater
};

UInt32 hsPhysicalMemory();
MemSpec hsMemorySpec();

inline int hsRandMax() { return 32767; }
inline float hsRandNorm() { return 1.f / 32767.f; } // multiply by hsRand to get randoms ranged [0..1]
int hsRand(void);
void hsRandSeed(int seed);


#if HS_BUILD_FOR_MAC
FILE* hsFopen(const char name[], const char mode[]);	// handles path names with /s

#elif HS_BUILD_FOR_PS2 // HS_BUILD_FOR_MAC

int hsPS2Open(const char name[], const char mode[]);
void hsPS2Close( int file );

#else // HS_BUILD_FOR_MAC

#define hsFopen(name, mode)	fopen(name, mode)

#endif // HS_BUILD_FOR_MAC

char** DisplaySystemVersion();

#endif // hsUtils_Defined

