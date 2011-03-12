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
#include "HeadSpin.h"
#include "hsRefCnt.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "hsExceptions.h"


#if HS_BUILD_FOR_MAC
	#include <Events.h>
	#include <ToolUtils.h>
	#include <Windows.h>
#endif

#if HS_BUILD_FOR_WIN32
# include <crtdbg.h>		/* for _RPT_BASE */
# define WIN32_LEAN_AND_MEAN
# define WIN32_EXTRA_LEAN
# include <windows.h>	// For OutputDebugString()
#endif


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
	char	s[1024];

#if HS_BUILD_FOR_WIN32
    #define strfmt _snprintf
#else
    #define strfmt snprintf
#endif

    if (val)
	    s[0] = strfmt(&s[1], 1022, "%s: %ld", message, val);
	else
	    s[0] = strfmt(&s[1], 1022, "%s", message);

	if (gHSDebugProc)
		gHSDebugProc(&s[1]);
	else
#if HS_BUILD_FOR_MAC
		DebugStr((unsigned char*)s);
#elif HS_BUILD_FOR_WIN32
	{	OutputDebugString(&s[1]);
		OutputDebugString("\n");
	}
#elif (HS_BUILD_FOR_BE || HS_BUILD_FOR_UNIX )
	{	fprintf(stderr, "%s\n", &s[1]);
//		hsThrow(&s[1]);
	}
#elif HS_BUILD_FOR_PS2
	fprintf(stderr, "%s\n", &s[1]);	
#else
	hsThrow(&s[1]);
#endif
}
#endif


///////////////////////////////////////////////////////////////////


hsRefCnt::~hsRefCnt()
{
	hsDebugCode(hsThrowIfFalse(fRefCnt == 1);)
}

void hsRefCnt::Ref()
{
	fRefCnt++;
}

void hsRefCnt::UnRef()
{
	hsDebugCode(hsThrowIfFalse(fRefCnt >= 1);)

	if (fRefCnt == 1)	// don't decrement if we call delete
		delete this;
	else
		--fRefCnt;
}


////////////////////////////////////////////////////////////////////////////

#ifndef PLASMA_EXTERNAL_RELEASE

void hsStatusMessage(const char message[])
{
  if (gHSStatusProc) {
    gHSStatusProc(message);
  } else {
#if HS_BUILD_FOR_PS2 || HS_BUILD_FOR_UNIX
    printf("%s",message);
	int len = strlen(message);
	if (len>0 && message[len-1]!='\n')
		printf("\n");
#elif HS_BUILD_FOR_WIN32
    OutputDebugString(message);
	int len = strlen(message);
	if (len>0 && message[len-1]!='\n')
		OutputDebugString("\n");
#endif  // MAC ??????  TODO
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

#endif // not PLASMA_EXTERNAL_RELEASE
