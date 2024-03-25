/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include "plNetMessage.h"
#include "Factory.h"

#include <memory>
#include <zlib.h>

static ChunkBuffer* NetMessageStream(ChunkBuffer& buffer)
{
    unsigned uncompressedSize = buffer.read<unsigned>();
    unsigned char compression = buffer.read<unsigned char>();
    unsigned streamSize = buffer.read<unsigned>();
    std::vector<unsigned char> stream(streamSize);
    buffer.chomp(stream.data(), streamSize);

    ChunkBuffer* chunk = new ChunkBuffer();
    if (compression == 2) {
        std::vector<unsigned char> ustream(uncompressedSize);
        ustream[0] = stream[0];
        ustream[1] = stream[1];
        uLongf zlength = uncompressedSize - 2;
        uncompress(ustream.data() + 2, &zlength, stream.data() + 2, streamSize - 2);
        chunk->append(ustream, 0);
    } else {
        chunk->append(stream, 0);
    }

    return chunk;
}

enum ClientGuidContents
{
    kHasAcctUuid       = (1<<0),
    kHasPlayerId       = (1<<1),
    kHasTempPlayerId   = (1<<2),
    kHasCCRLevel       = (1<<3),
    kHasProtectedLogin = (1<<4),
    kHasBuildType      = (1<<5),
    kHasPlayerName     = (1<<6),
    kHasSrcAddr        = (1<<7),
    kHasSrcPort        = (1<<8),
    kHasReserved       = (1<<9),
    kHasClientKey      = (1<<10),
};

static void ClientGuid(QTreeWidgetItem* parent, const QString& title, ChunkBuffer& buffer)
{
    static const char* s_flagNames[] = {
        "kHasAcctUuid", "kHasPlayerId", "kHasTempPlayerId", "kHasCCRLevel",
        "kHasProtectedLogin", "kHasBuildType", "kHasPlayerName", "kHasSrcAddr",
        "kHasSrcPort", "kHasReserved", "kHasClientKey", "(1<<11)",
        "(1<<12)", "(1<<13)", "(1<<14)", "(1<<15)"
    };

    QTreeWidgetItem* top = new QTreeWidgetItem(parent, QStringList{ title });

    unsigned short contents = buffer.read<unsigned short>();
    FlagField(top, "Contents", contents, s_flagNames);

    if (contents & kHasAcctUuid) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Account UUID: %1").arg(buffer.readUuid()));
    }
    if ((contents & kHasPlayerId) || (contents & kHasTempPlayerId)) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Player ID: %1").arg(buffer.read<unsigned>()));
    }
    if (contents & kHasPlayerName) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Player Name: %1").arg(buffer.readPString<unsigned short>()));
    }
    if (contents & kHasCCRLevel) {
        new QTreeWidgetItem(top, QStringList()
            << QString("CCR Level: %1").arg(buffer.read<unsigned char>()));
    }
    if (contents & kHasProtectedLogin) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Protected Login: %1").arg(buffer.read<bool>() ? "True" : "False"));
    }
    if (contents & kHasBuildType) {
        unsigned char buildType = buffer.read<unsigned char>();
        switch (buildType) {
        case 0:
            new QTreeWidgetItem(top, QStringList() << "Build Type: Unknown");
            break;
        case 1:
            new QTreeWidgetItem(top, QStringList() << "Build Type: Debug");
            break;
        case 2:
            new QTreeWidgetItem(top, QStringList() << "Build Type: Internal");
            break;
        case 3:
            new QTreeWidgetItem(top, QStringList() << "Build Type: External");
            break;
        default:
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(top, QStringList()
                    << QString("Build Type: %1").arg(buildType));
                item->setForeground(0, Qt::red);
            }
        }
    }
    if (contents & kHasSrcAddr) {
        unsigned addr = buffer.read<unsigned>();
        new QTreeWidgetItem(top, QStringList()
            << QString("Source Address: %1.%2.%3.%4").arg(addr & 0xFF)
               .arg((addr >> 8) & 0xFF).arg((addr >> 16) & 0xFF)
               .arg((addr >> 24) & 0xFF));
    }
    if (contents & kHasSrcPort) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Source Port: %1").arg(buffer.read<unsigned short>()));
    }
    if (contents & kHasReserved) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Reserved: %1").arg(buffer.read<unsigned short>()));
    }
    if (contents & kHasClientKey) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Client Key: %1").arg(buffer.readPString<unsigned short>()));
    }
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
    kHasAcctUuid_1             = (1<<14),
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
    if (contentFlags & kHasAcctUuid_1) {
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
    if (subStream->size() != 0)
        throw std::runtime_error("Substream parse incomplete");
    delete subStream;

    if (buffer.read<bool>()) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Delivery Time: %1.%2")
               .arg(buffer.read<unsigned>())
               .arg(buffer.read<unsigned>()));
    }
}

void Create_NetMsgRoomsList(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    unsigned count = buffer.read<unsigned>();
    QTreeWidgetItem* rooms = new QTreeWidgetItem(parent, QStringList() << "Rooms");
    for (unsigned i = 0; i < count; ++i) {
        QTreeWidgetItem* room = new QTreeWidgetItem(rooms, QStringList()
            << QString("Room %1").arg(i));
        Location(room, "Location", buffer);
        new QTreeWidgetItem(room, QStringList()
            << QString("Name: %1").arg(buffer.readPString<unsigned short>()));
    }
}

void Create_NetMsgGameMessageDirected(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMsgGameMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMsgGameMessage>"), buffer);

    unsigned char count = buffer.read<unsigned char>();
    QTreeWidgetItem* receivers = new QTreeWidgetItem(parent, QStringList() << "Receivers");
    for (unsigned char i = 0; i < count; ++i) {
        new QTreeWidgetItem(receivers, QStringList()
            << QString("%1").arg(buffer.read<unsigned>()));
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

void Create_NetMsgInitialAgeStateSent(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Num States: %1").arg(buffer.read<unsigned>()));
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

static const char* s_memberFlagNames[] = {
    "kWaitingForLinkQuery", "kIndirectMember", "kRequestP2P",
    "kWaitingForChallengeResponse", "kIsServer", "kAllowTimeOut",
    "(1<<6)", "(1<<7)", "(1<<8)", "(1<<9)", "(1<<10)", "(1<<11)",
    "(1<<12)", "(1<<13)", "(1<<14)", "(1<<15)",
    "(1<<16)", "(1<<17)", "(1<<18)", "(1<<19)",
    "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)",
    "(1<<24)", "(1<<25)", "(1<<26)", "(1<<27)",
    "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)",
};

void Create_NetMsgMembersList(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    unsigned short count = buffer.read<unsigned short>();
    QTreeWidgetItem* members = new QTreeWidgetItem(parent, QStringList() << "Members");
    for (unsigned short i = 0; i < count; ++i) {
        QTreeWidgetItem* member = new QTreeWidgetItem(members, QStringList()
            << QString("Member %1").arg(i));
        FlagField(member, "Flags", buffer.read<unsigned>(), s_memberFlagNames);
        ClientGuid(member, "Client GUID", buffer);
        Uoid(member, "Avatar Key", buffer);
    }
}

void Create_NetMsgMembersListReq(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);
}

void Create_NetMsgMemberUpdate(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);

    QTreeWidgetItem* member = new QTreeWidgetItem(parent, QStringList() << "Member");
    FlagField(member, "Flags", buffer.read<unsigned>(), s_memberFlagNames);
    ClientGuid(member, "Client GUID", buffer);
    Uoid(member, "Avatar Key", buffer);

    new QTreeWidgetItem(parent, QStringList()
        << QString("Add Member: %1").arg(buffer.read<bool>() ? "True" : "False"));
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

void Create_NetMsgPagingRoom(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_flagNames[] = {
        "kPagingOut", "kResetList", "kRequestState", "kFinalRoomInAge",
        "(1<<4)", "(1<<5)", "(1<<6)", "(1<<7)",
        "(1<<8)", "(1<<9)", "(1<<10)", "(1<<11)",
        "(1<<12)", "(1<<13)", "(1<<14)", "(1<<15)",
        "(1<<16)", "(1<<17)", "(1<<18)", "(1<<19)",
        "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)",
        "(1<<24)", "(1<<25)", "(1<<26)", "(1<<27)",
        "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)",
    };

    Create_NetMsgRoomsList(new QTreeWidgetItem(parent, QStringList() << "<plNetMessageRoomsList>"), buffer);
    FlagField(parent, "Paging Flags", buffer.read<unsigned char>(), s_flagNames);
}

void Create_NetMsgSDLState(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);
    Uoid(parent, "Object", buffer);

    ChunkBuffer* state = NetMessageStream(buffer);
    // TODO:  Add SDL parsing...  I really hate SDL
    unsigned size = state->size();
    QString contents;
    while (size) {
        if (size > 16) {
            contents.append(QString("%1 %2 %3 %4 %5 %6 %7 %8  %9 %10 %11 %12 %13 %14 %15 %16%17")
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                            .arg(size == 16 ? "" : "\n"));
            size -= 16;
        } else {
            for (unsigned i = 0; i < size; ++i)
                contents.append(QString("%1%2")
                                .arg(state->read<unsigned char>(), 2, 16, QChar('0'))
                                .arg(i == size ? "" : i == 8 ? "  " : " "));
            size = 0;
        }
    }
    QTreeWidgetItem* blob = new QTreeWidgetItem(parent, QStringList() << "SDL Blob");
    QTreeWidgetItem* sdl = new QTreeWidgetItem(blob, QStringList() << contents.toUpper());
    sdl->setFont(0, QFont("Courier New", sdl->font(0).pointSize()));
    delete state;

    new QTreeWidgetItem(parent, QStringList()
        << QString("Initial State: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Persist on Server: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Avatar: %1").arg(buffer.read<bool>() ? "True" : "False"));
}

enum GenericVarType
{
    kTypeInt, kTypeFloat, kTypeBool, kTypeString, kTypeByte,
    kTypeAny, kTypeUint, kTypeDouble, kTypeNone = 0xFF,
};

void Create_NetMsgSharedState(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_NetMessage(new QTreeWidgetItem(parent, QStringList() << "<plNetMessage>"), buffer);
    Uoid(parent, "Object", buffer);

    ChunkBuffer* state = NetMessageStream(buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("State Name: %1").arg(state->readPString<unsigned short>()));

    unsigned count = state->read<unsigned>();
    new QTreeWidgetItem(parent, QStringList()
        << QString("Server May Delete: %1").arg(state->read<bool>() ? "True" : "False"));
    QTreeWidgetItem* vars = new QTreeWidgetItem(parent, QStringList() << "Variables");
    for (unsigned i = 0; i < count; ++i) {
        QString varName = state->readSafeString();

        unsigned char type = state->read<unsigned char>();
        switch (type) {
        case kTypeInt:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Int) %2").arg(varName).arg(state->read<int>()));
            break;
        case kTypeUint:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (UInt) %2").arg(varName).arg(state->read<unsigned>()));
            break;
        case kTypeString:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (String) \"%2\"").arg(varName).arg(state->readSafeString()));
            break;
        case kTypeAny:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Any) \"%2\"").arg(varName).arg(state->readSafeString()));
            break;
        case kTypeFloat:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Float) %2").arg(varName).arg(state->read<float>()));
            break;
        case kTypeBool:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Bool) %2").arg(varName)
                   .arg(state->read<bool>() ? "True" : "False"));
            break;
        case kTypeByte:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Byte) %2").arg(varName).arg(state->read<unsigned char>()));
            break;
        case kTypeDouble:
            new QTreeWidgetItem(vars, QStringList()
                << QString("%1: (Double) %2").arg(varName).arg(state->read<double>()));
            break;
        default:
            {
                new QTreeWidgetItem(parent, QStringList()
                    << QString("Unsupported GenericVar (%1)").arg(type, 4, 16, QChar('0')));

                QTreeWidgetItem* item = parent;
                while (item->parent())
                    item = item->parent();
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);

                OutputDebugStringW(QString("Unsupported GenericVar (%1)\n")
                                   .arg(type, 4, 16, QChar('0')).toStdWString().c_str());
                i = count;
            }
        }
    }

    if (state->size() != 0)
        throw std::runtime_error("SharedState parse incomplete\n");
    delete state;

    new QTreeWidgetItem(parent, QStringList()
        << QString("Lock Request: %1").arg(buffer.read<bool>() ? "True" : "False"));
}
