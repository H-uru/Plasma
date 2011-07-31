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

#include "GateKeeper.h"

#include <QDateTime>

bool GateKeeper_Factory(QTreeWidget* logger, QString timeFmt, int direction,
                        const unsigned char*& data, size_t& size)
{
    unsigned short msgId = chompBuffer<unsigned short>(data, size);

    if (direction == kCli2Srv) {
        switch (msgId) {
        case kCli2GateKeeper_PingRequest:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - Cli2GateKeeper_PingRequest").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Ping Time: %1").arg(QDateTime::fromTime_t(chompBuffer<uint>(data, size)).toString()));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            size_t payloadSize = chompBuffer<unsigned>(data, size);
            if (payloadSize > 0) {
                new QTreeWidgetItem(top, QStringList()
                    << QString("Payload: %1 bytes").arg(payloadSize));
                data += payloadSize;
                size -= payloadSize;
                QFont bold = top->font(0);
                bold.setBold(true);
                top->setFont(0, bold);
            }
            break;
        }
        case kCli2GateKeeper_FileSrvIpAddressRequest:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - Cli2GateKeeper_FileSrvIpAddressRequest").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            new QTreeWidgetItem(top, QStringList()
                << QString("Patcher: %1").arg(chompBuffer<unsigned char>(data, size) == 0 ? "False" : "True"));
            break;
        }
        case kCli2GateKeeper_AuthSrvIpAddressRequest:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - Cli2GateKeeper_AuthSrvIpAddressRequest").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            break;
        }
        default:
            new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - Invalid Cli2GateKeeper message (%2)").arg(timeFmt).arg(msgId));
            return false;
        }
    } else {
        switch (msgId) {
        case kGateKeeper2Cli_PingReply:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - GateKeeper2Cli_PingReply").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Ping Time: %1").arg(QDateTime::fromTime_t(chompBuffer<uint>(data, size)).toString()));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            size_t payloadSize = chompBuffer<unsigned>(data, size);
            if (payloadSize > 0) {
                new QTreeWidgetItem(top, QStringList()
                    << QString("Payload: %1 bytes").arg(payloadSize));
                data += payloadSize;
                size -= payloadSize;
                QFont bold = top->font(0);
                bold.setBold(true);
                top->setFont(0, bold);
            }
            break;
        }
        case kGateKeeper2Cli_FileSrvIpAddressReply:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - GateKeeper2Cli_FileSrvIpAddressReply").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            new QTreeWidgetItem(top, QStringList()
                << QString("Address: %1").arg(chompString(data, size)));
            break;
        }
        case kGateKeeper2Cli_AuthSrvIpAddressReply:
        {
            QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - GateKeeper2Cli_AuthSrvIpAddressReply").arg(timeFmt));
            new QTreeWidgetItem(top, QStringList()
                << QString("Trans ID: %1").arg(chompBuffer<unsigned>(data, size)));
            new QTreeWidgetItem(top, QStringList()
                << QString("Address: %1").arg(chompString(data, size)));
            break;
        }
        default:
            new QTreeWidgetItem(logger, QStringList()
                << QString("%1 - Invalid GateKeeper2Cli message (%2)").arg(timeFmt).arg(msgId));
            return false;
        }
    }

    return true;
}
