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
#include "hsUtils.h"
#if HS_BUILD_FOR_MAC
#include <Gestalt.h>
#endif
#if HS_BUILD_FOR_WIN32
extern "C" {
#endif
#include <stdio.h>
#include <stdarg.h>
#if HS_BUILD_FOR_WIN32
};
#endif
#if __MWERKS__
#include <ctype.h>
#endif
#if HS_BUILD_FOR_PS2
#include <ctype.h>
#include "eekernel.h"
#include "sifdev.h"
#endif

#if HS_BUILD_FOR_WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "hsStlUtils.h"
#include "hsTemplates.h"


char * hsFormatStr(const char * fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	char * result = hsFormatStrV(fmt,args);
	va_end(args);
	return result;
}

char * hsFormatStrV(const char * fmt, va_list args)
{
	std::string buf;
	xtl::formatv(buf,fmt,args);
	return hsStrcpy(buf.c_str());
}

static char hsStrBuf[100];

char *hsScalarToStr(hsScalar s)
{
#if !(HS_BUILD_FOR_REFERENCE)
	if (s == hsIntToScalar(hsScalarToInt(s)))
		sprintf(hsStrBuf, "%d", hsScalarToInt(s));
	else
	#if HS_CAN_USE_FLOAT
		sprintf(hsStrBuf, "%f", hsScalarToFloat(s));
	#else
		sprintf(hsStrBuf, "%d:%lu", hsFixedToInt(s), (UInt16)s);
	#endif
#endif
	return hsStrBuf;
}

bool hsMessageBox_SuppressPrompts = false;

int hsMessageBoxWithOwner(void * owner, const char message[], const char caption[], int kind, int icon)
{
	if (hsMessageBox_SuppressPrompts)
		return hsMBoxOk;

#if HS_BUILD_FOR_WIN32
	UInt32 flags = 0;

	if (kind == hsMessageBoxNormal)
		flags |= MB_OK;
	else if (kind == hsMessageBoxAbortRetyIgnore)
		flags |= MB_ABORTRETRYIGNORE;
	else if (kind == hsMessageBoxOkCancel)
		flags |= MB_OKCANCEL;
	else if (kind == hsMessageBoxRetryCancel)
		flags |= MB_RETRYCANCEL;
	else if (kind == hsMessageBoxYesNo)
		flags |= MB_YESNO;
	else if (kind == hsMessageBoxYesNoCancel)
		flags |= MB_YESNOCANCEL;
	else
		flags |= MB_OK;

	if (icon == hsMessageBoxIconError)
		flags |= MB_ICONERROR;
	else if (icon == hsMessageBoxIconQuestion)
		flags |= MB_ICONQUESTION;
	else if (icon == hsMessageBoxIconExclamation)
		flags |= MB_ICONEXCLAMATION;
	else if (icon == hsMessageBoxIconAsterisk)
		flags |= MB_ICONASTERISK;
	else
		flags |= MB_ICONERROR;

#ifdef CLIENT
	ErrorMinimizeAppWindow();
#endif
	int ans = MessageBox((HWND)owner, message, caption, flags);

	switch (ans)
	{
	case IDOK:			return hsMBoxOk;
	case IDCANCEL:		return hsMBoxCancel;
	case IDABORT:		return hsMBoxAbort;
	case IDRETRY:		return hsMBoxRetry;
	case IDIGNORE:		return hsMBoxIgnore;
	case IDYES:			return hsMBoxYes;
	case IDNO:			return hsMBoxNo;
	default:			return hsMBoxCancel;
	}

#endif
#if HS_BUILD_FOR_MACPPC
	DebugStr(message);
#endif
#if HS_BUILD_FOR_PS2
	printf("Cap:%s Message:%s\n",caption, message);
#endif
}

int hsMessageBoxWithOwner(void * owner, const wchar_t message[], const wchar_t caption[], int kind, int icon)
{
	if (hsMessageBox_SuppressPrompts)
		return hsMBoxOk;

#if HS_BUILD_FOR_WIN32
	UInt32 flags = 0;
	
	if (kind == hsMessageBoxNormal)
		flags |= MB_OK;
	else if (kind == hsMessageBoxAbortRetyIgnore)
		flags |= MB_ABORTRETRYIGNORE;
	else if (kind == hsMessageBoxOkCancel)
		flags |= MB_OKCANCEL;
	else if (kind == hsMessageBoxRetryCancel)
		flags |= MB_RETRYCANCEL;
	else if (kind == hsMessageBoxYesNo)
		flags |= MB_YESNO;
	else if (kind == hsMessageBoxYesNoCancel)
		flags |= MB_YESNOCANCEL;
	else
		flags |= MB_OK;
	
	if (icon == hsMessageBoxIconError)
		flags |= MB_ICONERROR;
	else if (icon == hsMessageBoxIconQuestion)
		flags |= MB_ICONQUESTION;
	else if (icon == hsMessageBoxIconExclamation)
		flags |= MB_ICONEXCLAMATION;
	else if (icon == hsMessageBoxIconAsterisk)
		flags |= MB_ICONASTERISK;
	else
		flags |= MB_ICONERROR;
	
#ifdef CLIENT
	ErrorMinimizeAppWindow();
#endif
	int ans = MessageBoxW((HWND)owner, message, caption, flags);
	
	switch (ans)
	{
	case IDOK:			return hsMBoxOk;
	case IDCANCEL:		return hsMBoxCancel;
	case IDABORT:		return hsMBoxAbort;
	case IDRETRY:		return hsMBoxRetry;
	case IDIGNORE:		return hsMBoxIgnore;
	case IDYES:			return hsMBoxYes;
	case IDNO:			return hsMBoxNo;
	default:			return hsMBoxCancel;
	}
	
#endif
#if HS_BUILD_FOR_MACPPC
	DebugStr(message);
#endif
#if HS_BUILD_FOR_PS2
	printf("Cap:%s Message:%s\n",caption, message);
#endif
}

int hsMessageBox(const char message[], const char caption[], int kind, int icon)
{
#if HS_BUILD_FOR_WIN32
	return hsMessageBoxWithOwner(nil/*GetActiveWindow()*/,message,caption,kind,icon);
#else
	return hsMessageBoxWithOwner(nil,message,caption,kind,icon);
#endif
}

int hsMessageBox(const wchar_t message[], const wchar_t caption[], int kind, int icon)
{
#if HS_BUILD_FOR_WIN32
	return hsMessageBoxWithOwner(nil/*GetActiveWindow()*/,message,caption,kind,icon);
#else
	return hsMessageBoxWithOwner(nil,message,caption,kind,icon);
#endif
}


/* Generic psuedo RNG used in ANSI C. */ 
static unsigned long SEED = 1;
int hsRand()
{
	register int temp;
	SEED = SEED * 1103515245 + 12345;
	temp = (int)((SEED/65536)&32767);
	return (temp);
}

void hsRandSeed(int seed)
{
	SEED = seed;
}
/**************************************/
int hsStrlen(const char src[])
{
	if (src==nil)
		return 0;

	int i = 0;
	while (src[i])
		i++;
	return i;
}

char* hsStrcpy(char dst[], const char src[])
{
	if (src)
	{
		if (dst == nil)
		{
			int	count = hsStrlen(src);
			dst = (char *)ALLOC(count + 1);
			memcpy(dst, src, count);
			dst[count] = 0;
			return dst;
		}

		Int32 i;
		for (i = 0; src[i] != 0; i++)
			dst[i] = src[i];
		dst[i] = 0;
	}

	return dst;
}

hsBool hsStrEQ(const char s1[], const char s2[])
{
	if (s1 && s2)
	{
		while (*s1)
			if(*s1++ != *s2++)
				return false;
		return *s2 == 0;
	}

	return (!s1 && !s2);
}

hsBool hsStrCaseEQ(const char* s1, const char* s2)
{
	if (s1 && s2)
	{
		while (*s1)
			if(tolower(*s1++) != tolower(*s2++))
				return false;
		return *s2 == 0;
	}

	return (!s1 && !s2);
}

void hsStrcat(char dst[], const char src[])
{
	if (src && dst)
	{
		dst += hsStrlen(dst);
		while(*src)
			*dst++ = *src++;
		*dst = 0;
	}
}

void hsStrLower(char *s)
{
	if (s)
	{
		int i;
		for (i = 0; i < hsStrlen(s); i++)
			s[i] = tolower(s[i]); 
	}
}

char* hsP2CString(const UInt8 pstring[], char cstring[])
{
	char*	     cstr = cstring;
	const UInt8* stop = &pstring[1] + pstring[0];
	
	pstring += 1;	//	skip length byte
	while (pstring < stop)
		*cstr++ = *pstring++;
	*cstr = 0;
	return cstring;
}

UInt8* hsC2PString(const char cstring[], UInt8 pstring[])
{
	int	i;

	for (i = 1; *cstring; i++)
		pstring[i] = *cstring++;
	pstring[0] = i - 1;
	return pstring;
}

//// IStringToWString /////////////////////////////////////////////////////////
// Converts a char * string to a wchar_t * string

wchar_t	*hsStringToWString( const char *str )
{
	// convert the char string to a wchar_t string
	int len = strlen(str);
	wchar_t *wideString = TRACKED_NEW wchar_t[len+1];
	for (int i=0; i<len; i++)
		wideString[i] = btowc(str[i]);
	wideString[len] = L'\0';
	return wideString;
}

void	hsStringToWString( wchar_t *dst, const char *src )
{
	if (dst)
		delete [] dst;
	dst = hsStringToWString(src);
}

//// IWStringToString /////////////////////////////////////////////////////////
// Converts a wchar_t * string to a char * string

char	*hsWStringToString( const wchar_t *str )
{
	// convert the wchar_t string to a char string
	int len = wcslen(str);
	char *sStr = TRACKED_NEW char[len+1];

	int i;
	for (i = 0; i < len; i++)
	{
		char temp = wctob(str[i]);
		if (temp == WEOF)
		{
			sStr[i] = '\0';
			i = len;
		}
		else
			sStr[i] = temp;
	}
	sStr[len] = '\0';

	return sStr;
}

void	hsWStringToString( char *dst, const wchar_t *src )
{
	if (dst)
		delete [] dst;
	dst = hsWStringToString(src);
}

void hsCPathToMacPath(char* dst, char* fname)
{
	int i;
	
	int 	offset = 0;
	hsBool 	prefix = 1; 	// Assume its a relative path.
	
	// KLUDGE: this determines whether a PC path is 
	// relative or absolute. True if relative, therefore
	// we prefix the pathname with a colon.
	
	hsStrcpy(dst, "");

	if(strstr(fname, ":"))
	{
		prefix = 0;
	}
	else if(strstr(fname, "\\\\"))
	{
		prefix = 0;
		offset = 2; 		// copy fname from 2-bytes in. This removes 
							// the first two chars...
	}

	if(prefix)
	{
		hsStrcpy(dst, ":");
	}
	
	hsStrcat(dst, &fname[offset]);
	
	// No more slashes? We're done. (Optimization? Not really I guess.)
	if(!strstr(dst, "\\") && !strstr(dst, "/")) return; 
	
	for(i =0; i < hsStrlen(dst); i++)
	{
		if(dst[i] == '\\' || dst[i] == '/')
		{
			dst[i] = ':';
		}
	}
}

int hsRemove(const char * fname)
{
#if HS_BUILD_FOR_MACPPC
	char buf[500];
	hsStrcpy(buf,":");
	hsStrcat(buf,fname);
	int i;
	for(i =0; i < hsStrlen(buf); i++)
		if(buf[i] == '\\')
			buf[i] = ':';
	return remove(buf);
#endif
	return remove(fname);
	
}

UInt32 hsPhysicalMemory()
{
#define HS_ONE_MEGABYTE 1048576 // 1024 * 1024

#if HS_BUILD_FOR_WIN32
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	return (ms.dwTotalPhys / HS_ONE_MEGABYTE);
#elif HS_BUILD_FOR_MAC
	// Silver, figure out the physical memory here (in MB)	
	OSErr err;
	SInt32 TotPhysicalRAM;
	err = Gestalt(gestaltPhysicalRAMSize, &TotPhysicalRAM);
	return (TotPhysicalRAM / HS_ONE_MEGABYTE);
#endif
}

MemSpec hsMemorySpec()
{
	UInt32 mem = hsPhysicalMemory();

	// Currently adding a little margin of error here
	// due to the fact that Windows doesn't seem to
	// be totally accurate in it's calculations.
	if (mem < 127)
		return kBlows;
	else if (mem < 255)
		return kAcceptable;
	else
		return kOptimal;
}

#if HS_BUILD_FOR_MAC
FILE *hsFopen(const char *fname, const char *mode)
{
	char buf[500];
#if 0
	FILE *f;

	hsStrcpy(buf,":");
	hsStrcat(buf,fname);
	int i;
	for(i =0; i < hsStrlen(buf); i++)
		if(buf[i] == '\\')
			buf[i] = ':';
	
#endif
	hsCPathToMacPath(buf, (char*)fname);
	return fopen(buf,mode);
}

#endif
#if HS_BUILD_FOR_PS2
int hsPS2Open(const char name[], const char mode[])
{
  char buf[500];
  int newMode;
  int i;
  hsStrcpy(buf,"sim:");
//hsStrcpy(buf,"");
  hsStrcat(buf,name);
  for(i =0; i < hsStrlen(buf); i++)
	if(buf[i] == '\\')
		buf[i] = '/';
  printf("Opening File %s\n",buf);
  if(mode[0] == 'r')
    newMode = SCE_RDONLY;
  else if(mode[0] == 'w')
    newMode = SCE_WRONLY|SCE_CREAT;
  else
    hsAssert(0,"Bad mode in hsPS2Open\n");

  printf("Opening File %s mode =%d\n",buf,newMode);
   return  sceOpen(buf,newMode);
}

void hsPS2Close( int file )
{
	if( file != -1 )
		sceClose( file );
}

//FILE *hsFopen(const char *fname, const char *mode)
//{	
//	FILE *f;
//	char buf[500];
//	char newMode[10];
//	hsStrcpy(buf,"sim:");
//	//hsStrcpy(buf,"");
//	hsStrcat(buf,fname);
//	int i;
//	for(i =0; i < hsStrlen(buf); i++)
//		if(buf[i] == '\\')
//			buf[i] = '/';
//	printf("Opening File %s\n",buf);
//	if(!strcmp("rt",mode))
//	  {
//	    strcpy(newMode, "r");
//	  }
//	else
//	  strcpy(newMode, mode);
//
//	printf("Opening File %s mode =%s\n",buf,newMode);
//	f= fopen(buf,newMode);
//	if(f)
//		return f;
//	else
//		return nil;
//}
#endif

// Compare lexigraphically two strings

#if !(HS_BUILD_FOR_WIN32 || HS_BUILD_FOR_UNIX)

int hsStrcasecmp(const char *s1, const char *s2)
{
	if (s1 && s2)
	{
		char c1, c2;
		while (1)
		{
    		c1 = tolower(*s1++);
    		c2 = tolower(*s2++);
			if (c1 < c2) return -1;
			if (c1 > c2) return 1;
			if (c1 == '\0') return 0;
		}
	}
	return !s1 ? -1 : 1;
}

// Compare lexigraphically two strings up to a max length

int hsStrncasecmp(const char *s1, const char *s2, int n)
{
	if (s1 && s2)
	{
		int i;
		char c1, c2;
		for (i=0; i<n; i++)
		{
			c1 = tolower(*s1++);
			c2 = tolower(*s2++);
			if (c1 < c2) return -1;
			if (c1 > c2) return 1;
			if (!c1) return 0;
		}
		return 0;
	}
	return !s1 ? -1 : 1;
}
#endif

//
// Microsoft SAMPLE CODE
// returns array of allocated version info strings or nil
//
char** DisplaySystemVersion()
{
#if HS_BUILD_FOR_WIN32
#ifndef VER_SUITE_PERSONAL
#define VER_SUITE_PERSONAL 0x200
#endif
	hsTArray<char*> versionStrs;
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	
	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	//
	// If that fails, try using the OSVERSIONINFO structure.
	
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	
	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return FALSE;
	}
	
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		
		// Test for the product.
		
		if ( osvi.dwMajorVersion <= 4 )
            versionStrs.Append(hsStrcpy("Microsoft Windows NT "));
		
		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
            versionStrs.Append(hsStrcpy ("Microsoft Windows 2000 "));
		
		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
            versionStrs.Append(hsStrcpy ("Microsoft Windows XP "));
		
		// Test for product type.
		
		if( bOsVersionInfoEx )
		{
            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					versionStrs.Append(hsStrcpy ( "Personal " ));
				else
					versionStrs.Append(hsStrcpy ( "Professional " ));
            }
			
            else if ( osvi.wProductType == VER_NT_SERVER )
            {
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					versionStrs.Append(hsStrcpy ( "DataCenter Server " ));
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					versionStrs.Append(hsStrcpy ( "Advanced Server " ));
				else
					versionStrs.Append(hsStrcpy ( "Server " ));
            }
		}
		else
		{
            HKEY hKey;
            char szProductType[80];
            DWORD dwBufLen;
			
            RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );
            RegQueryValueEx( hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);
            RegCloseKey( hKey );
            if ( lstrcmpi( "WINNT", szProductType) == 0 )
				versionStrs.Append(hsStrcpy( "Professional " ));
            if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				versionStrs.Append(hsStrcpy( "Server " ));
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				versionStrs.Append(hsStrcpy( "Advanced Server " ));
		}
		
		// Display version, service pack (if any), and build number.
		
		if ( osvi.dwMajorVersion <= 4 )
		{
            versionStrs.Append(hsStrcpy (xtl::format("version %d.%d %s (Build %d)\n",
				osvi.dwMajorVersion,
				osvi.dwMinorVersion,
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF).c_str()));
		}
		else
		{ 
            versionStrs.Append(hsStrcpy (xtl::format("%s (Build %d)\n",
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF).c_str()));
		}
		break;
		
	case VER_PLATFORM_WIN32_WINDOWS:
		
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			versionStrs.Append(hsStrcpy ("Microsoft Windows 95 "));
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                versionStrs.Append(hsStrcpy("OSR2 " ));
		} 
		
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			versionStrs.Append(hsStrcpy ("Microsoft Windows 98 "));
			if ( osvi.szCSDVersion[1] == 'A' )
                versionStrs.Append(hsStrcpy("SE " ));
		} 
		
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			versionStrs.Append(hsStrcpy ("Microsoft Windows Me "));
		} 
		break;
		
	case VER_PLATFORM_WIN32s:
		
		versionStrs.Append(hsStrcpy ("Microsoft Win32s "));
		break;
	}
	
	versionStrs.Append(nil);	// terminator
	
	return versionStrs.DetachArray();
#else
	return nil;
#endif
}
