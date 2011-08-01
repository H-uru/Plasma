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

#include "plNetMessage.h"
#include "Factory.h"

#include <zlib.h>

static ChunkBuffer* NetMessageStream(ChunkBuffer& buffer)
{
    unsigned uncompressedSize = buffer.read<unsigned>();
    unsigned char compression = buffer.read<unsigned char>();
    unsigned streamSize = buffer.read<unsigned>();
    unsigned char* stream = new unsigned char[streamSize];
    buffer.chomp(stream, streamSize);

    ChunkBuffer* chunk = new ChunkBuffer();
    if (compression == 2) {
        unsigned char* ustream = new unsigned char[uncompressedSize];
        ustream[0] = stream[0];
        ustream[1] = stream[1];
        uLongf zlength = uncompressedSize - 2;
        uncompress(ustream + 2, &zlength, stream + 2, streamSize - 2);
        chunk->append(ustream, uncompressedSize, 0);
        delete[] stream;
    } else {
        chunk->append(stream, streamSize, 0);
    }

    return chunk;
}

enum NetMsg_ContentFlags
{
    kHasTimeSent               = (1<<0),
    kHasGameMsgReceivers       = (1<<1),
    kEchoBackToSender          = (1<<2),
    kRequestP2P                = (1<<3),
    kAllowTimeOut              = (1<<4),
    kIndirectMember            = (1<<5),
    kPublicIPClient            = (1<<6),
    kHasContext                = (1<<7),
    kAskVaultForGameState      = (1<<8),
    kHasTransactionID          = (1<<9),
    kNewSDLState               = (1<<10),
    kInitialAgeStateRequest    = (1<<11),
    kHasPlayerID               = (1<<12),
    kUseRelevanceRegions       = (1<<13),
    kHasAcctUuid               = (1<<14),
    kInterAgeRouting           = (1<<15),
    kHasVersion                = (1<<16),
    kIsSystemMessage           = (1<<17),
    kNeedsReliableSend         = (1<<18),
    kRouteToAllPlayers         = (1<<19),
};

void Create_NetMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_flagNames[] = {
        "kHasTimeSent", "kHasGameMsgReceivers", "kEchoBackToSender", "kRequestP2P",
        "kAllowTimeOut", "kIndirectMember", "kPublicIPClient", "kHasContext",
        "kAskVaultForGameState", "kHasTransactionID", "kNewSDLState", "kInitialAgeStateRequest",
        "kHasPlayerID", "kUseRelevanceRegions", "kHasAcctUuid", "kInterAgeRouting",
        "kHasVersion", "kIsSystemMessage", "kNeedsReliableSend", "kRouteToAllPlayers",
        "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)", "(1<<24)", "(1<<25)",
        "(1<<26)", "(1<<27)", "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)"
    };

    unsigned contentFlags = buffer.read<unsigned>();
    FlagField(parent, "Contents", contentFlags, s_flagNames);

    if (contentFlags & kHasVersion) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Protocol Version: %1.%2")
               .arg(buffer.read<unsigned char>())
               .arg(buffer.read<unsigned char>()));
    }
    if (contentFlags & kHasTimeSent) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Timestamp: %1.%2")
               .arg(buffer.read<unsigned>())
               .arg(buffer.read<unsigned>()));
    }
    if (contentFlags & kHasContext) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Context: %1").arg(buffer.read<unsigned>()));
    }
    if (contentFlags & kHasTransactionID) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Transaction ID: %1").arg(buffer.read<unsigned>()));
    }
    if (contentFlags & kHasPlayerID) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Player ID: %1").arg(buffer.read<unsigned>()));
    }
    if (contentFlags & kHasAcctUuid) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Account ID: %1").arg(buffer.readUuid()));
    }
}

void Create_NetMsgGameMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    ChunkBuffer* subStream = NetMessageStream(buffer);
    QTreeWidgetItem* message = new QTreeWidgetItem(parent, QStringList());
    QString msgType = Factory_Create(message, *subStream, subStream->size());
    message->setText(0, QString("Game Message: %1").arg(msgType));
    delete subStream;

    if (buffer.read<bool>()) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Delivery Time: %1.%2")
               .arg(buffer.read<unsigned>())
               .arg(buffer.read<unsigned>()));
    }
}

void Create_NetMsgGroupOwner(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_flagNames[] = {
        "kNetGroupConstant", "kNetGroupLocal", "(1<<2)", "(1<<3)",
        "(1<<4)", "(1<<5)", "(1<<6)", "(1<<7)"
    };

    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    unsigned count = buffer.read<unsigned>();
    for (unsigned i = 0; i < count; ++i) {
        QTreeWidgetItem* group = new QTreeWidgetItem(parent, QStringList()
            << QString("Group %1").arg(i));
        Location(group, "Location", buffer);
        FlagField(group, "Flags", buffer.read<unsigned char>(), s_flagNames);
        new QTreeWidgetItem(group, QStringList()
            << QString("Owned: %1").arg(buffer.read<bool>() ? "True" : "False"));
    }
}

void Create_NetMsgLoadClone(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMsgGameMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMsgGameMessage>"), buffer);

    Uoid(parent, "Clone Key", buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Player: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Loading: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Initial State: %1").arg(buffer.read<bool>() ? "True" : "False"));
}

void Create_NetMsgPlayerPage(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    new QTreeWidgetItem(parent, QStringList()
        << QString("Unload: %1").arg(buffer.read<bool>() ? "True" : "False"));
    Uoid(parent, "Player Key", buffer);
}

void Create_NetMsgRelevanceRegions(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    BitVector(parent, "Regions I Care About", buffer);
    BitVector(parent, "Regions I Am In", buffer);
}

void Create_NetMsgRoomsList(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    unsigned count = buffer.read<unsigned>();
    for (unsigned i = 0; i < count; ++i) {
        QTreeWidgetItem* room = new QTreeWidgetItem(parent, QStringList()
            << QString("Room %1").arg(i));
        Location(room, "Location", buffer);
        new QTreeWidgetItem(room, QStringList()
            << QString("Name: %1").arg(buffer.readString()));
    }
}
