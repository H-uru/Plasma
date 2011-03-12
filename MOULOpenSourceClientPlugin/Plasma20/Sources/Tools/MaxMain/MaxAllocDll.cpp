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
#include "max.h"
#include "MaxAllocDll.h"

typedef void* (*MAXMALLOC) (size_t size);
typedef void (*MAXFREE) (void *memblock);

static MAXMALLOC maxMalloc = NULL;
static MAXFREE maxFree = NULL;
static HINSTANCE maxAllocDll = NULL;

void LoadAllocDll()
{
	if (!maxAllocDll)
	{
		// Search through all the Max plugin paths for MaxAlloc.dll
		Interface *ip = GetCOREInterface();
		for (int i = 0; i < ip->GetPlugInEntryCount(); i++)
		{
			const char *dir = ip->GetPlugInDir(i);

			char path[MAX_PATH];
			sprintf(path, "%sMaxAlloc.dll", dir);

			maxAllocDll = LoadLibrary(path);

			if (maxAllocDll)
				return;
		}

		maxAllocDll = LoadLibrary("MaxAlloc.dll");

		if (!maxAllocDll)
		{
         ::MessageBox(NULL, "Couldn't load MaxAlloc.dll", "Error", MB_OK);
			exit(0);
		}
	}
}

void *plMaxMalloc(size_t size)
{
	if (!maxMalloc)
	{
		LoadAllocDll();
		maxMalloc = (MAXMALLOC)GetProcAddress(maxAllocDll, "MaxMalloc");

		if (!maxMalloc)
		{
			::MessageBox(NULL, "Couldn't find MaxMalloc in MaxAlloc.dll", "Error", MB_OK);
			exit(0);
		}
	}

	return maxMalloc(size);
}

void plMaxFree(void *memblock)
{
	if (!maxFree)
	{
		LoadAllocDll();
		maxFree = (MAXFREE)GetProcAddress(maxAllocDll, "MaxFree");

		if (!maxFree)
		{
         ::MessageBox(NULL, "Couldn't find MaxFree in MaxAlloc.dll", "Error", MB_OK);
			exit(0);
		}

	}

	maxFree(memblock);
}
