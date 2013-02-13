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
#include "HeadSpin.h"
#include "hsWindows.h"
#include <wchar.h>

#ifdef _MSC_VER
#   include <crtdbg.h>
#endif
#pragma hdrstop

#include "hsTemplates.h"
#include "plString.h"


///////////////////////////////////////////////////////////////////////////
/////////////////// For Status Messages ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////
hsDebugMessageProc gHSStatusProc = nil;

hsDebugMessageProc hsSetStatusMessageProc(hsDebugMessageProc newProc)
{
    hsDebugMessageProc oldProc = gHSStatusProc;

    gHSStatusProc = newProc;

    return oldProc;
}

//////////////////////////////////////////////////////////////////////////

hsDebugMessageProc gHSDebugProc = nil;

hsDebugMessageProc hsSetDebugMessageProc(hsDebugMessageProc newProc)
{
    hsDebugMessageProc oldProc = gHSDebugProc;

    gHSDebugProc = newProc;

    return oldProc;
}

#ifdef HS_DEBUGGING
void hsDebugMessage (const char message[], long val)
{
    char    s[1024];

    if (val)
        s[0] = snprintf(&s[1], 1022, "%s: %ld", message, val);
    else
        s[0] = snprintf(&s[1], 1022, "%s", message);

    if (gHSDebugProc)
        gHSDebugProc(&s[1]);
    else
#if HS_BUILD_FOR_WIN32
    {   OutputDebugString(&s[1]);
        OutputDebugString("\n");
    }
#elif HS_BUILD_FOR_UNIX
    {   fprintf(stderr, "%s\n", &s[1]);
//      hsThrow(&s[1]);
    }
#else
    hsThrow(&s[1]);
#endif
}
#endif

static bool s_GuiAsserts = true;
void ErrorEnableGui(bool enabled)
{
    s_GuiAsserts = enabled;
}

void ErrorAssert(int line, const char file[], const char fmt[], ...)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
#ifdef HS_DEBUGGING
    if (s_GuiAsserts)
    {
        if(_CrtDbgReport(_CRT_ASSERT, file, line, NULL, msg))
            DebugBreak();
    } else
#endif // HS_DEBUGGING
      if (DebugIsDebuggerPresent()) {
        char str[] = "-------\nASSERTION FAILED:\nFile: %s   Line: %i\nMessage: %s\n-------";
        DebugMsg(str, file, line, msg);
    }
#else
    DebugBreakIfDebuggerPresent();
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

bool DebugIsDebuggerPresent()
{
#ifdef _MSC_VER
    return IsDebuggerPresent();
#else
    // FIXME
    return false;
#endif
}

void DebugBreakIfDebuggerPresent()
{
#ifdef _MSC_VER
    __try
    {
        __debugbreak();
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Debugger not present or some such shwiz.
        // Whatever. Don't crash here.
    }
#endif // _MSC_VER
}

void DebugMsg(const char fmt[], ...)
{
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);

    if (DebugIsDebuggerPresent())
    {
#ifdef _MSC_VER
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
#endif
    } else {
        fprintf(stderr, "%s\n", msg);
    }
}

////////////////////////////////////////////////////////////////////////////

#ifndef PLASMA_EXTERNAL_RELEASE

void hsStatusMessage(const char message[])
{
  if (gHSStatusProc) {
    gHSStatusProc(message);
  } else {
#if HS_BUILD_FOR_UNIX
    printf("%s",message);
    int len = strlen(message);
    if (len>0 && message[len-1]!='\n')
        printf("\n");
#elif HS_BUILD_FOR_WIN32
    OutputDebugString(message);
    int len = strlen(message);
    if (len>0 && message[len-1]!='\n')
        OutputDebugString("\n");
#endif
  }
}

void hsStatusMessageV(const char * fmt, va_list args)
{
    char  buffer[2000];
    vsprintf(buffer, fmt, args);
    hsStatusMessage(buffer);
}

void hsStatusMessageF(const char * fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    hsStatusMessageV(fmt,args);
    va_end(args);
}

#endif

// TODO: Deprecate these in favor of plString
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
    plString buf = plString::IFormat(fmt, args);
    return hsStrcpy(buf.c_str());
}

static char hsStrBuf[100];

char *hsScalarToStr(float s)
{
    sprintf(hsStrBuf, "%f", s);
    return hsStrBuf;
}

class hsMinimizeClientGuard
{
#ifdef CLIENT
    hsWindowHndl fWnd;

public:
    hsMinimizeClientGuard()
    {
#ifdef HS_BUILD_FOR_WIN32
        fWnd = GetActiveWindow();
        // If the application's topmost window is fullscreen, minimize it before displaying an error
        if ((GetWindowLong(fWnd, GWL_STYLE) & WS_POPUP) != 0)
            ShowWindow(fWnd, SW_MINIMIZE);
#endif // HS_BUILD_FOR_WIN32
    }

    ~hsMinimizeClientGuard()
    {
#ifdef HS_BUILD_FOR_WIN32
        ShowWindow(fWnd, SW_RESTORE);
#endif // HS_BUILD_FOR_WIN32
    }
#endif // CLIENT
};

bool hsMessageBox_SuppressPrompts = false;

int hsMessageBoxWithOwner(hsWindowHndl owner, const char message[], const char caption[], int kind, int icon)
{
    if (hsMessageBox_SuppressPrompts)
        return hsMBoxOk;

#if HS_BUILD_FOR_WIN32
    uint32_t flags = 0;

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

    hsMinimizeClientGuard guard;
    int ans = MessageBox(owner, message, caption, flags);

    switch (ans)
    {
    case IDOK:          return hsMBoxOk;
    case IDCANCEL:      return hsMBoxCancel;
    case IDABORT:       return hsMBoxAbort;
    case IDRETRY:       return hsMBoxRetry;
    case IDIGNORE:      return hsMBoxIgnore;
    case IDYES:         return hsMBoxYes;
    case IDNO:          return hsMBoxNo;
    default:            return hsMBoxCancel;
    }

#endif
    return hsMBoxCancel;
}

int hsMessageBoxWithOwner(hsWindowHndl owner, const wchar_t message[], const wchar_t caption[], int kind, int icon)
{
    if (hsMessageBox_SuppressPrompts)
        return hsMBoxOk;

#if HS_BUILD_FOR_WIN32
    uint32_t flags = 0;

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

    hsMinimizeClientGuard guard;
    int ans = MessageBoxW(owner, message, caption, flags);

    switch (ans)
    {
    case IDOK:          return hsMBoxOk;
    case IDCANCEL:      return hsMBoxCancel;
    case IDABORT:       return hsMBoxAbort;
    case IDRETRY:       return hsMBoxRetry;
    case IDIGNORE:      return hsMBoxIgnore;
    case IDYES:         return hsMBoxYes;
    case IDNO:          return hsMBoxNo;
    default:            return hsMBoxCancel;
    }

#endif
    return hsMBoxCancel;
}

int hsMessageBox(const char message[], const char caption[], int kind, int icon)
{
    return hsMessageBoxWithOwner((hsWindowHndl)nil,message,caption,kind,icon);
}

int hsMessageBox(const wchar_t message[], const wchar_t caption[], int kind, int icon)
{
    return hsMessageBoxWithOwner((hsWindowHndl)nil,message,caption,kind,icon);
}

/**************************************/
char* hsStrcpy(char dst[], const char src[])
{
    if (src)
    {
        if (dst == nil)
        {
            int count = strlen(src);
            dst = (char *)malloc(count + 1);
            memcpy(dst, src, count);
            dst[count] = 0;
            return dst;
        }

        int32_t i;
        for (i = 0; src[i] != 0; i++)
            dst[i] = src[i];
        dst[i] = 0;
    }

    return dst;
}

void hsStrLower(char *s)
{
    if (s)
    {
        int i;
        for (i = 0; i < strlen(s); i++)
            s[i] = tolower(s[i]);
    }
}

//// IStringToWString /////////////////////////////////////////////////////////
// Converts a char * string to a wchar_t * string

wchar_t *hsStringToWString( const char *str )
{
    // convert the char string to a wchar_t string
    int len = strlen(str);
    wchar_t *wideString = new wchar_t[len+1];
    for (int i=0; i<len; i++)
        wideString[i] = btowc(str[i]);
    wideString[len] = L'\0';
    return wideString;
}

//// IWStringToString /////////////////////////////////////////////////////////
// Converts a wchar_t * string to a char * string

char    *hsWStringToString( const wchar_t *str )
{
    // convert the wchar_t string to a char string
    int len = wcslen(str);
    char *sStr = new char[len+1];

    int i;
    for (i = 0; i < len; i++)
    {
        char temp = wctob(str[i]);
        if (temp == EOF)
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

//
// Microsoft SAMPLE CODE
// returns array of allocated version info strings or nil
//
char** DisplaySystemVersion()
{
    // TODO:  I so want to std::vector<plString> this, but that requires
    //        including more headers in HeadSpin.h :(
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

        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
            versionStrs.Append(hsStrcpy ("Microsoft Windows Vista "));

        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
            versionStrs.Append(hsStrcpy ("Microsoft Windows 7 "));

        if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2 )
            versionStrs.Append(hsStrcpy ("Microsoft Windows 8 "));

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
            versionStrs.Append(hsStrcpy (plString::Format("version %d.%d %s (Build %d)\n",
                osvi.dwMajorVersion,
                osvi.dwMinorVersion,
                osvi.szCSDVersion,
                osvi.dwBuildNumber & 0xFFFF).c_str()));
        }
        else
        {
            versionStrs.Append(hsStrcpy (plString::Format("%s (Build %d)\n",
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

    versionStrs.Append(nil);    // terminator

    return versionStrs.DetachArray();
#else
    return nil;
#endif
}
