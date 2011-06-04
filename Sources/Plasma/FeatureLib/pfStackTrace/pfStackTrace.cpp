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
#include "pfStackTrace.h"
#include "pfMapFile.h"
#include "pfMapFileEntry.h"
#include <string.h>
#include <stdio.h>

#pragma optimize( "y", off )

//-----------------------------------------------------------------------------

#define MAX_DEPTH 32

//-----------------------------------------------------------------------------

namespace dev
{

static long getCallerFromStack( unsigned long stackPtr, int index )
{
#if /*defined(_DEBUG) && */defined(_MSC_VER) && defined(_M_IX86)

	long caller = 0;
	__asm
	{
		mov ebx, stackPtr
		mov ecx, index
		inc ecx			
		xor eax, eax
StackTrace_getCaller_next:
		mov eax, [ebx+4]
		mov ebx, [ebx]
		test eax,eax
		jz StackTrace_getCallerFromStack_done
		dec ecx
		jnz StackTrace_getCaller_next
StackTrace_getCallerFromStack_done:
		mov caller, eax
	}
	return caller;

#else

	return 0;

#endif
}

static long getCaller( int index )
{
#if /*defined(_DEBUG) && */defined(_MSC_VER) && defined(_M_IX86)

	long caller = 0;
	__asm
	{
		mov ebx, ebp
		mov ecx, index
		inc ecx
		xor eax, eax
StackTrace_getCaller_next:
		mov eax, [ebx+4]
		mov ebx, [ebx]
		test eax,eax
		jz StackTrace_getCaller_done
		dec ecx
		jnz StackTrace_getCaller_next
StackTrace_getCaller_done:
		mov caller, eax
	}
	return caller;

#else

	return 0;

#endif
}

int StackTrace::printStackTrace( MapFile** map, int maps,
	int initLevel, int maxDepth,
	char* buffer, int bufferSize, unsigned long stackPtr, unsigned long opPtr )
{
	if ( maxDepth > MAX_DEPTH )
		maxDepth = MAX_DEPTH;
	bool	sucks = false;

	// list callers
	long callersAddr[MAX_DEPTH];
	int callers = 0;
	int i;
	for ( i = initLevel ; i < maxDepth ; ++i )
	{
		long addr;
		if( stackPtr != 0 )
		{
			if( i == initLevel )
				addr = opPtr;
			else
				addr = getCallerFromStack( stackPtr, i - initLevel - 1 );
		}
		else
			addr = getCaller( i );
		callersAddr[callers++] = addr;

		// end tracing here if the entry is not in a map file
		if( map != 0 )
		{
			int entry = -1;
			for ( int j = 0 ; j < maps ; ++j )
			{
				entry = map[j]->findEntry( addr );
				if ( -1 != entry )
					break;
			}
			if ( -1 == entry )
			{
				sucks = true;
				break;
			}
		}
	}


	int needed = 0;
	if ( bufferSize > 0 )
		*buffer = 0;

	sprintf( buffer, "Call stack (%d levels%s):\r\n", callers - initLevel, sucks ? ", truncated" : "" );
	needed = strlen( buffer );

	// output call stack
	for ( i = initLevel ; i < callers ; ++i )
	{
		long addr = callersAddr[callers-i-1];

		// find entry info
		int entry = -1;
		const MapFile* entryMap = 0;
		for ( int j = 0 ; j < maps ; ++j )
		{
			entry = map[j]->findEntry( addr );
			if ( -1 != entry )
			{
				entryMap = map[j];
				break;
			}
		}

		// format entry to tempory buf
		char buf[MapFileEntry::MAX_NAME+MAX_DEPTH+20];	// name + margin + hex number
		buf[0] = 0;
		for ( int k = initLevel-1 ; k < i ; ++k )
			strcat( buf, " " );
		if ( !entryMap )
			sprintf( buf+strlen(buf), "0x%08X\r\n", addr );
		else
		{
			const MapFileEntry &en = entryMap->getEntry( entry );
			long entryAddr = entryMap->loadAddress() + (en.section() << 12) + en.offset();

			sprintf( buf+strlen(buf), "%s (0x%08X + 0x%08X)\r\n", en.name(), entryAddr, addr - entryAddr );
		}

		// append temporary buf to output buffer if space left
		needed += strlen( buf );
		if ( needed < bufferSize )
			strcat( buffer, buf );
	}

	// terminate output buffer
	if ( needed < bufferSize )
		buffer[needed] = 0;
	else if ( bufferSize > 0 )
		buffer[bufferSize-1] = 0;
	return needed;
}


} // dev
/*
 * Copyright (c) 2001 Jani Kajala
 *
 * Permission to use, copy, modify, distribute and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Jani Kajala makes no representations about the suitability 
 * of this software for any purpose. It is provided "as is" 
 * without express or implied warranty.
 */
