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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrBuildString.cpp
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
const wchar * ProductBuildString () {
	// This string is replaced by plMarkBuild.exe with the following string:
	// "date:YYMMDD time:HHMMSS user:%USERNAME%"
	static const wchar PRODUCT_BUILD_STRING[64] = {
		L"PRODUCT_BUILD_STRING                                           "
	};
	return PRODUCT_BUILD_STRING;
}
