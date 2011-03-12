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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrProductId.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private data
*
***/

namespace Product {

namespace Uru {
    static const wchar kCoreName[]	= L"UruLive";
    static const wchar kShortName[]	= L"UruLive";
    static const wchar kLongName[]	= L"Uru Live";
    static const Uuid  kId(L"ea489821-6c35-4bd0-9dae-bb17c585e680");
}

// @@@: add your product namespace here

}

/*****************************************************************************
*
*   Private definitions
*
***/

#if BUILD_PRODUCT_ID == BUILD_PRODUCT_ID_URU

#define PRODUCT_CORE_NAME	Product::Uru::kCoreName
#define PRODUCT_SHORT_NAME  Product::Uru::kShortName
#define PRODUCT_LONG_NAME   Product::Uru::kLongName
#define PRODUCT_ID          Product::Uru::kId

#else

#error "No product id defined"

#endif



/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
const Uuid & ProductId () {
    return PRODUCT_ID;
}

//============================================================================
const wchar * ProductCoreName () {
    return PRODUCT_CORE_NAME;
}

//============================================================================
const wchar * ProductShortName () {
    return PRODUCT_SHORT_NAME;
}

//============================================================================
const wchar * ProductLongName () {
    return PRODUCT_LONG_NAME;
}


//============================================================================
void ProductString (wchar * dest, unsigned destChars) {
	// Example: "UruLive.Beta.2.214 - External.Release"
	StrPrintf(
		dest,
		destChars,
		L"%s.%s.%u.%u - %s.%s",
		ProductCoreName(),
		BuildTypeString(),
		BranchId(),
		BuildId(),
		#ifdef PLASMA_EXTERNAL_RELEASE
			L"External",
		#else
			L"Internal",
		#endif
		#ifdef HS_DEBUGGING
			L"Debug"
		#else
			L"Release"
		#endif
	);
}
