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

void Create_LoadCloneMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    Key(parent, "Clone Key", buffer);
    Key(parent, "Requestor", buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Origin Player ID: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("User Data: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Message Valid: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Loading: %1").arg(buffer.read<bool>() ? "True" : "False"));

    QTreeWidgetItem* message = new QTreeWidgetItem(parent, QStringList());
    QString msgType = Factory_Create(message, buffer, 0);
    message->setText(0, QString("Trigger Message: %1").arg(msgType));
}

void Create_AvatarInputStateMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_stateNames[] = {
        "Move Forward", "Move Backward", "Rotate Left", "Rotate Right",
        "Strafe Left", "Strafe Right", "Always Run", "Jump",
        "Consumable Jump", "Fast Modifier", "Strafe Modifier", "Ladder Inverted",
        "(1<<12)", "(1<<13)", "(1<<14)", "(1<<15)"
    };

    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    FlagField(parent, "State", buffer.read<unsigned short>(), s_stateNames);
}

void Create_AvBrainGenericMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_types[] = {
        "kNextStage", "kPrevStage", "kGotoStage", "kSetLoopCount"
    };

    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    unsigned type = buffer.read<unsigned>();
    if (type < (sizeof(s_types) / sizeof(s_types[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1").arg(s_types[type]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1").arg(type));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("Stage: %1").arg(buffer.read<int>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Set Time: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("New Time: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Set Direction: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("New Direction: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Transition Time: %1").arg(buffer.read<float>()));
}

void Create_AvTaskMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    if (buffer.read<bool>()) {
        QTreeWidgetItem* task = new QTreeWidgetItem(parent, QStringList());
        QString taskType = Factory_Create(task, buffer, 0);
        task->setText(0, QString("Task: %1").arg(taskType));
    }
}

void Create_InputIfaceMgrMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_commands[] = {
        "kAddInterface", "kRemoveInterface", "kEnableClickables", "kDisableClickables",
        "kSetOfferBookMode", "kClearOfferBookMode", "kNotifyOfferAccepted",
        "kNotifyOfferRejected", "kNotifyOfferCompleted", "kDisableAvatarClickable",
        "kEnableAvatarClickable", "kGUIDisableAvatarClickable",
        "kGUIEnableAvatarClickable", "kSetShareSpawnPoint", "kSetShareAgeInstanceGuid",
    };

    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    unsigned char command = buffer.read<unsigned char>();
    if (command < (sizeof(s_commands) / sizeof(s_commands[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Command: %1").arg(s_commands[command]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Command: %1").arg(command));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("Page ID: 0x%1").arg(buffer.read<unsigned>(), 8, 16, QChar('0')));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Age Name: %1").arg(buffer.readSafeString()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Age Filename: %1").arg(buffer.readSafeString()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Spawn Point: %1").arg(buffer.readSafeString()));
    Key(parent, "Avatar", buffer);
}

void Create_KIMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_commands[] = {
        "kHACKChatMsg", "kEnterChatMode", "kSetChatFadeDelay", "kSetTextChatAdminMode",
        "kDisableKIandBB", "kEnableKIandBB", "kYesNoDialog", "kAddPlayerDevice",
        "kRemovePlayerDevice", "kUpgradeKILevel", "kDowngradeKILevel", "kRateIt",
        "kSetPrivateChatChannel", "kUnsetPrivateChatChannel", "kStartBookAlert",
        "kMiniBigKIToggle", "kKIPutAway", "kChatAreaPageUp", "kChatAreaPageDown",
        "kChatAreaGoToBegin", "kChatAreaGoToEnd", "kKITakePicture", "kKICreateJournalNote",
        "kKIToggleFade", "kKIToggleFadeEnable", "kKIChatStatusMsg", "kKILocalChatStatusMsg",
        "kKIUpSizeFont", "kKIDownSizeFont", "kKIOpenYeehsaBook", "kKIOpenKI",
        "kKIShowCCRHelp", "kKICreateMarker", "kKICreateMarkerFolder",
        "kKILocalChatErrorMsg", "kKIPhasedAllOn", "kKIPhasedAllOff", "kKIOKDialog",
        "kDisableYeeshaBook", "kEnableYeeshaBook", "kQuitDialog", "kTempDisableKIandBB",
        "kTempEnableKIandBB", "kDisableEntireYeeshaBook", "kEnableEntireYeeshaBook",
        "kKIOKDialogNoQuit", "kGZUpdated", "kGZInRange", "kGZOutRange",
        "kUpgradeKIMarkerLevel", "kKIShowMiniKI", "kGZFlashUpdate", "kStartJournalAlert",
        "kAddJournalBook", "kRemoveJournalBook", "kKIOpenJournalBook", "kMGStartCGZGame",
        "kMGStopCGZGame", "kKICreateMarkerNode", "kStartKIAlert", "kUpdatePelletScore",
        "kFriendInviteSent", "kRegisterImager", "kNoCommand"
    };

    static const char* s_flagNames[] = {
        "kPrivateMsg", "kAdminMsg", "kDead", "kUNUSED1",
        "kStatusMsg", "kNeighborMsg", "(1<<6)", "(1<<7)",
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        "(1<<16)", "(1<<17)", "(1<<18)", "(1<<19)",
        "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)",
        "(1<<24)", "(1<<25)", "(1<<26)", "(1<<27)",
        "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)",
    };

    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    unsigned char command = buffer.read<unsigned char>();
    if (command < (sizeof(s_commands) / sizeof(s_commands[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Command: %1").arg(s_commands[command]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Command: %1").arg(command));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("User: %1").arg(buffer.readSafeString()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Player ID: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("String: %1").arg(buffer.readSafeWString()));

    unsigned flags = buffer.read<unsigned>();
    FlagField(parent, "Flags", flags & 0xFFFF00FF, s_flagNames);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Channel: %1").arg((flags >> 8) & 0xFF));

    new QTreeWidgetItem(parent, QStringList()
        << QString("Delay: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Value: %1").arg(buffer.read<int>()));
}

void Create_LinkEffectsTriggerMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    new QTreeWidgetItem(parent, QStringList()
        << QString("Invisibility Level: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Leaving: %1").arg(buffer.read<bool>() ? "True" : "False"));
    Key(parent, "Link Key", buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Effects (Ignored): 0x%1").arg(buffer.read<unsigned>(), 8, 16, QChar('0')));
    Key(parent, "Animation Key", buffer);
}

void Create_LoadAvatarMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_LoadCloneMsg(new QTreeWidgetItem(parent, QStringList() << "<plLoadCloneMsg>"), buffer);

    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Player: %1").arg(buffer.read<bool>() ? "True" : "False"));
    Key(parent, "Spawn Point", buffer);

    if (buffer.read<bool>()) {
        QTreeWidgetItem* initTask = new QTreeWidgetItem(parent, QStringList());
        QString msgType = Factory_Create(initTask, buffer, 0);
        initTask->setText(0, QString("Initial Task: %1").arg(msgType));
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("User String: %1").arg(buffer.readSafeString()));
}

void Create_NotifyMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_typeNames[] = {
        "Activator", "Var Notification", "Notify Self", "Responder Fast Fwd",
        "Responder State Change"
    };

    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

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

void Create_ServerReplyMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_Message(new QTreeWidgetItem(parent, QStringList() << "<plMessage>"), buffer);

    int reply = buffer.read<int>();
    switch (reply) {
    case -1:
        new QTreeWidgetItem(parent, QStringList() << "Reply: Invalid");
        break;
    case 0:
        new QTreeWidgetItem(parent, QStringList() << "Reply: Deny");
        break;
    case 1:
        new QTreeWidgetItem(parent, QStringList() << "Reply: Affirm");
        break;
    default:
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent,
                QStringList() << QString("Reply: %1").arg(reply));
            item->setForeground(0, Qt::red);
        }
    }
}
