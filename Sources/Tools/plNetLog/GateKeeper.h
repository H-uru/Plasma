/******************************************************************************
 * This file is part of plNetLog.                                             *
 *                                                                            *
 * plNetLog is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * plNetLog is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with plNetLog.  If not, see <http://www.gnu.org/licenses/>.          *
 ******************************************************************************/

#include "plNetLog.h"

enum
{
    kCli2GateKeeper_PingRequest = 0,
    kCli2GateKeeper_FileSrvIpAddressRequest,
    kCli2GateKeeper_AuthSrvIpAddressRequest,

    // -------------------------------------------------------------- //

    kGateKeeper2Cli_PingReply = 0,
    kGateKeeper2Cli_FileSrvIpAddressReply,
    kGateKeeper2Cli_AuthSrvIpAddressReply,
};

bool GateKeeper_Factory(QTreeWidget* logger, QString timeFmt, int direction,
                        ChunkBuffer& buffer);
