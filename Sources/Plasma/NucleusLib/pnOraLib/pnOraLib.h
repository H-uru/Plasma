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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
    OraEnv *            oraEnv;
    occi::Connection *  occiConn;
    wchar_t               tag[128];
};


/*****************************************************************************
*
*   OraLib
*
***/

void OraLogError (const wchar_t sql[], const exception & e);

void OraGetShaDigest (
    occi::Statement *   oraStmt,
    unsigned            index,
    ShaDigest *         digest
);

void OraSetShaDigest (
    occi::Statement *       oraStmt,
    unsigned                index,
    const ShaDigest &       digest
);

void OraBindString (
    occi::Statement *   oraStmt,
    unsigned            index,
    wchar_t *             buffer,
    unsigned            chars,
    ub2 *               length,
    sb2 *               indicator
);

void OraBindString (
    occi::ResultSet *   rs,
    unsigned            index,
    wchar_t *             buffer,
    unsigned            chars,
    ub2 *               length,
    sb2 *               indicator
);

void OraGetUuid (
    occi::Statement *   oraStmt,
    unsigned            index,
    Uuid *              uuid
);

void OraSetUuid (
    occi::Statement *   oraStmt,
    unsigned            index,
    const Uuid &        uuid
);

OraConn * OraGetConn (const wchar_t tag[] = nil);

void OraFreeConn (OraConn *& oraConn);

occi::Statement * OraGetStmt (
    OraConn *       oraConn,
    const wchar_t     sql[]
);

void OraFreeStmt (occi::Statement *& oraStmt);

void OraInitialize (
    const wchar_t username[], 
    const wchar_t password[], 
    const wchar_t connectString[],
    unsigned stmtCacheSize
);
void OraShutdown ();
void OraDestroy ();

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNORALIB_PNORALIB_H