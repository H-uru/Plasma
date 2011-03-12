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
#include "hsTypes.h"

#include <ddraw.h>

#include "hsGDDrawDllLoad.h"

static hsGDDrawDllLoad staticDllLoad;

hsGDDrawDllLoad::hsGDDrawDllLoad()
{
	hsAssert(!staticDllLoad.fD3DDll, "Don't make instances of this class, just use GetDDrawDll func");

	fD3DDll = LoadLibrary( "D3D9.DLL" );
	if (fD3DDll)
		hsStatusMessage( "--- D3D9.DLL loaded successfully.\n" );
	else
		hsStatusMessage( "--- Unable to load D3D9.DLL successfully.\n" );
}

hsGDDrawDllLoad::~hsGDDrawDllLoad()
{
	if (fD3DDll != nil)
	{
		hsStatusMessage( "--- Unloading D3D.DLL.\n" );
		FreeLibrary(fD3DDll);
	}
}

HMODULE hsGDDrawDllLoad::GetD3DDll()
{
	return staticDllLoad.fD3DDll;
}