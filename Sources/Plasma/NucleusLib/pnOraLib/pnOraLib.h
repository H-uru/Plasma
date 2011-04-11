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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnOraLib/pnOraLib.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNORALIB_PNORALIB_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNORALIB_PNORALIB_H

// OCCI
#define WIN32COMMON
#include "occi.h"
using namespace oracle;
struct OraEnv;

enum {
	kOcciPerfOpenConns,
	kOcciPerfBusyConns,
	kOcciNumPerf
};

long OraLibGetOcciPerf (unsigned index);


struct OraConn {
// treat all fields as read-only
	OraEnv *			oraEnv;
	occi::Connection *	occiConn;
	wchar				tag[128];
};


/*****************************************************************************
*
*   OraLib
*
***/

void OraLogError (const wchar sql[], const exception & e);

void OraGetShaDigest (
	occi::Statement *	oraStmt,
	unsigned			index,
	ShaDigest *			digest
);

void OraSetShaDigest (
	occi::Statement *		oraStmt,
	unsigned				index,
	const ShaDigest &		digest
);

void OraBindString (
	occi::Statement *	oraStmt,
	unsigned			index,
	wchar *				buffer,
	unsigned			chars,
	ub2 *				length,
	sb2 *				indicator
);

void OraBindString (
	occi::ResultSet *	rs,
	unsigned			index,
	wchar *				buffer,
	unsigned			chars,
	ub2 *				length,
	sb2 *				indicator
);

void OraGetUuid (
	occi::Statement *	oraStmt,
	unsigned			index,
	Uuid *				uuid
);

void OraSetUuid (
	occi::Statement *	oraStmt,
	unsigned			index,
	const Uuid &		uuid
);

OraConn * OraGetConn (const wchar tag[] = nil);

void OraFreeConn (OraConn *& oraConn);

occi::Statement * OraGetStmt (
	OraConn *		oraConn,
	const wchar		sql[]
);

void OraFreeStmt (occi::Statement *& oraStmt);

void OraInitialize (
	const wchar username[], 
	const wchar password[], 
	const wchar connectString[],
	unsigned stmtCacheSize
);
void OraShutdown ();
void OraDestroy ();

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNORALIB_PNORALIB_H