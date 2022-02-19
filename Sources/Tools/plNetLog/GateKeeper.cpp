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

bool GateKeeper_Factory(QTreeWidget* logger, QString timeFmt, int direction,
                        ChunkBuffer& buffer)
{
    unsigned short msgId = buffer.read<unsigned short>();

    if (direction == kCli2Srv) {
        switch (msgId) {
        case kCli2GateKeeper_PingRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2GateKeeper_PingRequest").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Ping Time: %1 ms").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                size_t payloadSize = buffer.read<unsigned>();
                if (payloadSize > 0) {
                    new QTreeWidgetItem(top, QStringList()
                        << QString("Payload: %1 bytes").arg(payloadSize));
                    buffer.skip(payloadSize);
                    QFont bold = top->font(0);
                    bold.setBold(true);
                    top->setFont(0, bold);
                }
                break;
            }
        case kCli2GateKeeper_FileSrvIpAddressRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2GateKeeper_FileSrvIpAddressRequest").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Patcher: %1").arg(buffer.read<bool>() ? "True" : "False"));
                break;
            }
        case kCli2GateKeeper_AuthSrvIpAddressRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2GateKeeper_AuthSrvIpAddressRequest").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Invalid Cli2GateKeeper message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                OutputDebugStringA(QString("Invalid Cli2GateKeeper message (%1)\n").arg(msgId).toUtf8().data());
                return false;
            }
        }
    } else {
        switch (msgId) {
        case kGateKeeper2Cli_PingReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- GateKeeper2Cli_PingReply").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Ping Time: %1 ms").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                size_t payloadSize = buffer.read<unsigned>();
                if (payloadSize > 0) {
                    new QTreeWidgetItem(top, QStringList()
                        << QString("Payload: %1 bytes").arg(payloadSize));
                    buffer.skip(payloadSize);
                    QFont bold = top->font(0);
                    bold.setBold(true);
                    top->setFont(0, bold);
                }
                break;
            }
        case kGateKeeper2Cli_FileSrvIpAddressReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- GateKeeper2Cli_FileSrvIpAddressReply").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Address: %1").arg(buffer.readString()));
                break;
            }
        case kGateKeeper2Cli_AuthSrvIpAddressReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- GateKeeper2Cli_AuthSrvIpAddressReply").arg(timeFmt));
                top->setForeground(0, kColorGateKeeper);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Address: %1").arg(buffer.readString()));
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Invalid GateKeeper2Cli message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                OutputDebugStringA(QString("Invalid GateKeeper2Cli message (%1)\n").arg(msgId).toUtf8().data());
                return false;
            }
        }
    }

    return true;
}
