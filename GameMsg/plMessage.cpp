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

#include "plMessage.h"
#include "Factory.h"
#include "proEventData.h"

void Create_Message(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_flagNames[] = {
        "kBcastByType", "kUnused", "kPropagateToChildren", "kByExactType",
        "kPropagateToModifiers", "kClearAfterBcast", "kNetPropagate", "kNetSent",
        "kNetUseRelevanceRegions", "kNetForce", "kNetNonLocal", "kLocalPropagate",
        "kMsgWatch", "kNetStartCascade", "kNetAllowInterAge", "kNetSendUnreliable",
        "kCCRSendToAllPlayers", "kNetCreatedRemotely", "(1<<18)", "(1<<19)",
        "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)", "(1<<24)", "(1<<25)",
        "(1<<26)", "(1<<27)", "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)"
    };

    Key(parent, "Sender", buffer);

    QTreeWidgetItem* receivers = new QTreeWidgetItem(parent, QStringList() << "Receivers");
    unsigned count = buffer.read<unsigned>();
    for (unsigned i = 0; i < count; ++i)
        Key(receivers, QString("Receiver %1").arg(i), buffer);

    new QTreeWidgetItem(parent, QStringList()
        << QString("Timestamp: %1").arg(buffer.read<double>()));
    FlagField(parent, "Bcast Flags", buffer.read<unsigned>(), s_flagNames);
}

void Create_NotifyMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_typeNames[] = {
        "Activator", "Var Notification", "Notify Self", "Responder Fast Fwd",
        "Responder State Change"
    };

    Create_Message(parent, buffer);

    unsigned type = buffer.read<unsigned>();
    if (type < (sizeof(s_typeNames) / sizeof(s_typeNames[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1").arg(s_typeNames[type]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1 (Unknown)").arg(type));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("State: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("ID: %1").arg(buffer.read<unsigned>()));

    QTreeWidgetItem* events = new QTreeWidgetItem(parent, QStringList() << "Events");
    unsigned count = buffer.read<unsigned>();
    for (unsigned i =0; i < count; ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem(events, QStringList()
            << QString("Event %1").arg(i));
        EventData_Factory(item, buffer);
    }
}
