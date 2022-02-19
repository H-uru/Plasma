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

#include "Game.h"
#include "GameMsg/Factory.h"

bool Game_Factory(QTreeWidget* logger, QString timeFmt, int direction,
                  ChunkBuffer& buffer)
{
    unsigned short msgId = buffer.read<unsigned short>();

    if (direction == kCli2Srv) {
        switch (msgId) {
        case kCli2Game_PingRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Game_PingRequest").arg(timeFmt));
                top->setForeground(0, kColorGame);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Ping Time: %1 ms").arg(buffer.read<unsigned>()));
                break;
            }
        case kCli2Game_JoinAgeRequest:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Game_JoinAgeRequest").arg(timeFmt));
                top->setForeground(0, kColorGame);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Age MCP: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Account UUID: %1").arg(buffer.readUuid()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Player ID: %1").arg(buffer.read<unsigned>()));
                break;
            }
        case kCli2Game_PropagateBuffer:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Cli2Game_PropagateBuffer").arg(timeFmt));
                top->setForeground(0, kColorGame);
                unsigned typeId = buffer.read<unsigned>();
                new QTreeWidgetItem(top, QStringList()
                    << QString("Type: %1").arg(typeId, 4, 16, QChar('0')));
                QTreeWidgetItem* creatable = new QTreeWidgetItem(top, QStringList() << Factory_Name(typeId));
                Factory_Create(creatable, buffer, buffer.read<unsigned>());
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 --> Invalid Cli2Game message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                OutputDebugStringA(QString("Invalid Cli2Game message (%1)\n").arg(msgId).toUtf8().data());
                return false;
            }
        }
    } else {
        switch (msgId) {
        case kGame2Cli_PingReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Game2Cli_PingReply").arg(timeFmt));
                top->setForeground(0, kColorGame);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Ping Time: %1 ms").arg(buffer.read<unsigned>()));
                break;
            }
        case kGame2Cli_JoinAgeReply:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Game2Cli_JoinAgeReply").arg(timeFmt));
                top->setForeground(0, kColorGame);
                new QTreeWidgetItem(top, QStringList()
                    << QString("Trans ID: %1").arg(buffer.read<unsigned>()));
                new QTreeWidgetItem(top, QStringList()
                    << QString("Result: %1").arg(buffer.readResultCode()));
                break;
            }
        case kGame2Cli_PropagateBuffer:
            {
                QTreeWidgetItem* top = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Game2Cli_PropagateBuffer").arg(timeFmt));
                top->setForeground(0, kColorGame);
                unsigned typeId = buffer.read<unsigned>();
                new QTreeWidgetItem(top, QStringList()
                    << QString("Type: %1").arg(typeId, 4, 16, QChar('0')));
                QTreeWidgetItem* creatable = new QTreeWidgetItem(top, QStringList() << Factory_Name(typeId));
                Factory_Create(creatable, buffer, buffer.read<unsigned>());
                break;
            }
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(logger, QStringList()
                    << QString("%1 <-- Invalid Game2Cli message (%2)").arg(timeFmt).arg(msgId));
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                OutputDebugStringA(QString("Invalid Game2Cli message (%1)\n").arg(msgId).toUtf8().data());
                return false;
            }
        }
    }

    return true;
}
