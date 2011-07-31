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

#include "Factory.h"

QString Factory_Name(unsigned type)
{
    switch (type) {
    case kNetMsgPagingRoom:
        return "plNetMsgPagingRoom";
    case kLoadCloneMsg:
        return "plLoadCloneMsg";
    case kNetMsgGroupOwner:
        return "plNetMsgGroupOwner";
    case kNetMsgGameStateRequest:
        return "plNetMsgGameStateRequest";
    case kNetMsgGameMessage:
        return "plNetMsgGameMessage";
    case kServerReplyMsg:
        return "plServerReplyMsg";
    case kNetMsgVoice:
        return "plNetMsgVoice";
    case kNetMsgTestAndSet:
        return "plNetMsgTestAndSet";
    case kAvTaskMsg:
        return "plAvTaskMsg";
    case kAvSeekMsg:
        return "plAvSeekMsg";
    case kAvOneShotMsg:
        return "plAvOneShotMsg";
    case kNetMsgMembersListReq:
        return "plNetMsgMembersListReq";
    case kNetMsgMembersList:
        return "plNetMsgMembersList";
    case kNetMsgMemberUpdate:
        return "plNetMsgMemberUpdate";
    case kNetMsgInitialAgeStateSent:
        return "plNetMsgInitialAgeStateSent";
    case kAvTaskSeekDoneMsg:
        return "plAvTaskSeekDoneMsg";
    case kNetMsgSDLState:
        return "plNetMsgSDLState";
    case kNotifyMsg:
        return "plNotifyMsg";
    case kLinkEffectsTriggerMsg:
        return "plLinkEffectsTriggerMsg";
    case kNetMsgSDLStateBCast:
        return "plNetMsgSDLStateBCast";
    case kNetMsgGameMessageDirected:
        return "plNetMsgGameMessageDirected";
    case kAvatarInputStateMsg:
        return "plAvatarInputStateMsg";
    case kLinkingMgrMsg:
        return "plLinkingMgrMsg";
    case kClothingMsg:
        return "plClothingMsg";
    case kAvBrainHuman:
        return "plAvBrainHuman";
    case kAvBrainCritter:
        return "plAvBrainCritter";
    case kAvBrainDrive:
        return "plAvBrainDrive";
    case kAvBrainGeneric:
        return "plAvBrainGeneric";
    case kInputIfaceMgrMsg:
        return "plInputIfaceMgrMsg";
    case kKIMessage:
        return "plKIMessage";
    case kAvPushBrainMsg:
        return "plAvPushBrainMsg";
    case kAvPopBrainMsg:
        return "plAvPopBrainMsg";
    case kAvAnimTask:
        return "plAvAnimTask";
    case kAvSeekTask:
        return "plAvSeekTask";
    case kAvOneShotTask:
        return "plAvOneShotTask";
    case kAvTaskBrain:
        return "plAvTaskBrain";
    case kAnimStage:
        return "plAnimStage";
    case kAvBrainGenericMsg:
        return "plAvBrainGenericMsg";
    case kAvTaskSeek:
        return "plAvTaskSeek";
    case kNetMsgRelevanceRegions:
        return "plNetMsgRelevanceRegions";
    case kLoadAvatarMsg:
        return "plLoadAvatarMsg";
    case kNetMsgLoadClone:
        return "plNetMsgLoadClone";
    case kNetMsgPlayerPage:
        return "plNetMsgPlayerPage";
    case kAvBrainSwim:
        return "plAvBrainSwim";
    case kAvBrainClimb:
        return "plAvBrainClimb";
    case kAvOneShotLinkTask:
        return "plAvOneShotLinkTask";
    case kAvBrainRideAnimatedPhysical:
        return "plAvBrainRideAnimatedPhysical";
    default:
        OutputDebugStringA(QString("Unknown class ID (%1)\n").arg(type).toUtf8().data());
        return QString("Unknown class ID (%1)").arg(type);
    }
}

void Factory_Create(QTreeWidgetItem* parent, ChunkBuffer& buffer, size_t size)
{
    unsigned short type = buffer.read<unsigned short>();

    switch (type) {
    default:
        {
            new QTreeWidgetItem(parent, QStringList()
                << QString("Unsupported creatable (%1)").arg(type));

            QTreeWidgetItem* item = parent;
            while (item->parent())
                item = item->parent();
            QFont warnFont = item->font(0);
            warnFont.setBold(true);
            item->setFont(0, warnFont);
            item->setForeground(0, Qt::red);

            OutputDebugStringA(QString("Unsupported creatable (%1)\n").arg(type).toUtf8().data());
            if (size)
                buffer.skip(size - sizeof(unsigned short));
        }
    }
}
