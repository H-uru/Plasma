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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrBuildType.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
unsigned BuildType () {
    return BUILD_TYPE;
}

//============================================================================
const wchar * BuildTypeString () {

#if BUILD_TYPE == BUILD_TYPE_DEV
	return L"Dev";
#elif BUILD_TYPE == BUILD_TYPE_QA
	return L"QA";
#elif BUILD_TYPE == BUILD_TYPE_TEST
	return L"Test";
#elif BUILD_TYPE == BUILD_TYPE_BETA
	return L"Beta";
#elif BUILD_TYPE == BUILD_TYPE_LIVE
	return L"Live";
#else
# error "Unknown build type"
#endif

}

//============================================================================
const wchar *BuildTypeServerStatusPath () {
		
	#if BUILD_TYPE == BUILD_TYPE_DEV
		return nil;
	#elif BUILD_TYPE == BUILD_TYPE_QA
		return nil;
	#elif BUILD_TYPE == BUILD_TYPE_TEST
		return nil;
	#elif BUILD_TYPE == BUILD_TYPE_BETA
		return L"/serverstatus/moulbeta.php";
	#elif BUILD_TYPE == BUILD_TYPE_LIVE
		return L"/serverstatus/moullive.php";
	#else
	# error "Unknown build type"
	#endif

}
