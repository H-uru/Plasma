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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCsrNet/pnCsrNet.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCSRNET_PNCSRNET_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCSRNET_PNCSRNET_H


#include "pnSimpleNet/pnSimpleNet.h"


/*****************************************************************************
*
*   CSR Client Automation - Types and Constants
*
***/

// Newer CSR game clients must remain compatible with older CSR tools,
// therefore these values may not change.  Only append to this enum.
enum {
	kCsrNet_ExecConsoleCmd,
};

//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>

#define CSRNET_MSG(a)	\
	CsrNet_##a () : SimpleNet_MsgHeader(kSimpleNetChannelCsr, kCsrNet_##a) { }

struct CsrNet_ExecConsoleCmd : SimpleNet_MsgHeader {
	CSRNET_MSG	(ExecConsoleCmd);

	char	cmd[1];	// null-terminated string
};

#undef CSRNET_MSG
//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNCSRNET_PNCSRNET_H
