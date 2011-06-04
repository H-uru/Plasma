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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrBuildType.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNPRODUCT_PRIVATE_PNPRBUILDTYPE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnProduct/Private/pnPrBuildType.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNPRODUCT_PRIVATE_PNPRBUILDTYPE_H


/*****************************************************************************
*
*   BuildType definitions
*
***/
#ifndef BUILD_TYPE 
#define BUILD_TYPE BUILD_TYPE_DEV 
#endif  

#define BUILD_TYPE_DEV		10
#define BUILD_TYPE_QA		20
#define BUILD_TYPE_TEST		30
#define BUILD_TYPE_BETA		40
#define BUILD_TYPE_LIVE		50


/*****************************************************************************
*
*   BuildType functions
*
***/

unsigned BuildType ();
const wchar * BuildTypeString ();
const wchar * BuildTypeServerStatusPath ();
