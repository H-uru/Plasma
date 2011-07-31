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

#include "Auth.h"

bool Auth_Factory(QTreeWidget* logger, QString timeFmt, int direction,
                  ChunkBuffer& buffer)
{
    unsigned short msgId = buffer.read<unsigned short>();

    if (direction == kCli2Srv) {
        switch (msgId) {
        case kCli2Auth_PingRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_PingRequest").arg(timeFmt));
                top->setForeground(0, kColorAuth);
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
        case kCli2Auth_ClientRegisterRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_ClientRegisterRequest").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Build ID: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kCli2Auth_AcctLoginRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_AcctLoginRequest").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Challenge: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Account Name: %1").arg(buffer.readString()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Challenge Hash: %1").arg(buffer.readHex<20>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Auth Token: %1").arg(buffer.readString()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("OS: %1").arg(buffer.readString()));
                break;
            }
        case kCli2Auth_AcctSetPlayerRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_AcctSetPlayerRequest").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Player ID: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kCli2Auth_VaultFetchNodeRefs:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_VaultFetchNodeRefs").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Node ID: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kCli2Auth_LogPythonTraceback:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Auth_LogPythonTraceback").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList() << buffer.readString());
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Invalid Cli2Auth message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                return false;
            }
        }
    } else {
        switch (msgId) {
        case kAuth2Cli_PingReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_PingReply").arg(timeFmt));
                top->setForeground(0, kColorAuth);
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
        case kAuth2Cli_ServerAddr:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_ServerAddr").arg(timeFmt));
                unsigned addr = buffer.read<unsigned>();
                new QTreeWidgetItem(top, QStringList()
                    << QString("Address: %1.%2.%3.%4").arg(addr & 0xFF)
                       .arg((addr >> 8) & 0xFF).arg((addr >> 16) & 0xFF)
                       .arg((addr >> 24) & 0xFF));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Token: %1").arg(buffer.readUuid()));
                top->setForeground(0, kColorAuth);
                break;
            }
        case kAuth2Cli_ClientRegisterReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_ClientRegisterReply").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Challenge: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kAuth2Cli_AcctLoginReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_AcctLoginReply").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Result: %1").arg(buffer.readResultCode()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Account ID: %1").arg(buffer.readUuid()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Account Flags: 0x%1").arg(buffer.read<unsigned>(), 8, 16, QChar('0')));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Billing Type: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Encryption Key: (0x%1, 0x%2, 0x%3, 0x%4)")
                       .arg(buffer.read<unsigned>(), 8, 16, QChar('0'))
                       .arg(buffer.read<unsigned>(), 8, 16, QChar('0'))
                       .arg(buffer.read<unsigned>(), 8, 16, QChar('0'))
                       .arg(buffer.read<unsigned>(), 8, 16, QChar('0')));
                break;
            }
        case kAuth2Cli_AcctPlayerInfo:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_AcctPlayerInfo").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Player ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Player Name: %1").arg(buffer.readString()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Avatar Shape: %1").arg(buffer.readString()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Explorer: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kAuth2Cli_AcctSetPlayerReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_AcctSetPlayerReply").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Result: %1").arg(buffer.readResultCode()));
                break;
            }
        case kAuth2Cli_VaultNodeRefsFetched:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Auth2Cli_VaultNodeRefsFetched").arg(timeFmt));
                top->setForeground(0, kColorAuth);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Result: %1").arg(buffer.readResultCode()));
                unsigned count = buffer.read<unsigned>();
                QTreeWidgetItem* refs = new QTreeWidgetItem(top, QStringList() << "Refs");
                for (unsigned i=0; i<count; ++i) {
                    new QTreeWidgetItem(refs, QStringList()
                        << QString("Parent: %1;  Child: %2;  Owner: %3;  [%4]")
                           .arg(buffer.read<unsigned>())
                           .arg(buffer.read<unsigned>())
                           .arg(buffer.read<unsigned>())
                           .arg(buffer.read<bool>() ? "SEEN" : ""));
                }
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Invalid Auth2Cli message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                return false;
            }
        }
    }

    return true;
}
