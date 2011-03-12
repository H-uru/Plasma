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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrProductId.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNPRODUCT_PRIVATE_PNPRPRODUCTID_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrProductId.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNPRODUCT_PRIVATE_PNPRPRODUCTID_H


/*****************************************************************************
*
*   Global data
*
***/

//============================================================================
// Build productIds
//
// These values must never be stored on disk or sent over the network,
// use the value returned from ProductId() for that purpose.
//============================================================================
#define BUILD_PRODUCT_ID_URU    1
// @@@ Add your build product id above this line



//============================================================================
// Default to Uru for lack of a better idea
//============================================================================
#ifndef BUILD_PRODUCT_ID
# define BUILD_PRODUCT_ID   BUILD_PRODUCT_ID_URU
#endif



/*****************************************************************************
*
*   Product functions
*
***/

const Uuid &  ProductId ();
const wchar * ProductCoreName ();	// e.g: L"Uru"
const wchar * ProductShortName ();	// e.g: L"Uru"						(filename/registry friendly)
const wchar * ProductLongName ();	// e.g: L"Uru: Ages Beyond Myst"	(human friendly)


// Returns: "<ProductCoreName>.<BuildTypeString>.<BranchId>.<BuildId>"
// Example: "Uru.Beta.3.204"
void ProductString (wchar * dest, unsigned destChars);
