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
#include "plRealTimeLightBase.h"
#include "plRealTimeLights.h"
#include "plRTProjDirLightClassDesc.h"

#include "../pnMessage/pnMessageCreatable.h"
#include "../pnKeyedObject/pnKeyedObjectCreatable.h"
#include "../pnNetCommon/pnNetCommonCreatable.h"

#include "../plSurface/plLayerInterface.h"
REGISTER_NONCREATABLE( plLayerInterface );


HINSTANCE hInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit)
	{
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}

__declspec(dllexport) const TCHAR* LibDescription()
{
	return NULL;
}

__declspec(dllexport) int LibNumberClasses()
{
	return 4;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	switch (i)
	{
		case 0: return (ClassDesc*)plRTSpotLightDesc::GetDesc();
		case 1: return (ClassDesc*)plRTOmniLightDesc::GetDesc();
		case 2: return (ClassDesc*)plRTDirLightDesc::GetDesc();
		case 3: return (ClassDesc*)plRTProjDirLightDesc::GetDesc();
		default: return 0;
	}
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

