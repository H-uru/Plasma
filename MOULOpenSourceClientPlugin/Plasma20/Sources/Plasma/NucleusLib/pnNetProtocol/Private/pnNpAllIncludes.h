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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpAllIncludes.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPALLINCLUDES_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/pnNpAllIncludes.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PNNPALLINCLUDES_H

#if defined(USES_PROTOCOL_CLI2AUTH) || defined(USES_PROTOCOL_CLI2GAME) || defined(USES_PROTOCOL_CLI2CSR) || defined(USES_PROTOCOL_CLI2GATEKEEPER)
# define USES_NETCLI
#endif

#if defined(USES_PROTOCOL_SRV2VAULT) || defined(USES_PROTOCOL_SRV2DB) || defined(USES_PROTOCOL_SRV2MCP) || defined(USES_PROTOCOL_SRV2STATE) || defined(USES_PROTOCOL_SRV2LOG) || defined(USES_PROTOCOL_SRV2SCORE)
# define USES_NETSRV
#endif

#include "pnNpCommon.h"



#ifdef USES_PROTOCOL_CLI2FILE
# include "Protocols/Cli2File/pnNpCli2File.h"
#endif


#ifdef USES_NETCLI
# ifdef USES_PROTOCOL_CLI2AUTH
#  include "Protocols/Cli2Auth/pnNpCli2Auth.h"
# endif

# ifdef USES_PROTOCOL_CLI2GAME
#  include "Protocols/Cli2Game/pnNpCli2Game.h"
# endif

# ifdef USES_PROTOCOL_CLI2CSR
#  include "Protocols/Cli2Csr/pnNpCli2Csr.h"
# endif

# ifdef USES_PROTOCOL_CLI2GATEKEEPER
#  include "Protocols/Cli2GateKeeper/pnNpCli2GateKeeper.h"
# endif
#endif // def USES_NETCLI


#ifdef SERVER
# ifdef USES_NETSRV
// for SrvMsgHeader definition
#  include "../../NucleusLib/pnIni/pnIni.h"	// psSrvConn needs ini types
#  include "../../ServerLib/psUtils/psUtils.h"

#  ifdef USES_PROTOCOL_SRV2VAULT
#   include "Protocols/Srv2Vault/pnNpSrv2Vault.h"
#  endif

#  ifdef USES_PROTOCOL_SRV2DB
#   include "Protocols/Srv2Db/pnNpSrv2Db.h"
#  endif

#  ifdef USES_PROTOCOL_SRV2MCP
#   include "Protocols/Srv2Mcp/pnNpSrv2Mcp.h"
#  endif

#  ifdef USES_PROTOCOL_SRV2STATE
#   include "Protocols/Srv2State/pnNpSrv2State.h"
#  endif

#  ifdef USES_PROTOCOL_SRV2SCORE
#   include "Protocols/Srv2Score/pnNpSrv2Score.h"
#  endif

#  ifdef USES_PROTOCOL_SRV2LOG
#   include "Protocols/Srv2Log/pnNpSrv2Log.h"
#  endif

# endif	// def USES_NETSRV
#endif // def SERVER
