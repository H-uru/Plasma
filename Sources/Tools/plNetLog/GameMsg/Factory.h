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

enum ClassIDs
{
    kNetMsgPagingRoom = 0x0218,
    kLoadCloneMsg = 0x0253,
    kNetMsgGroupOwner = 0x0264,
    kNetMsgGameStateRequest = 0x0265,
    kNetMsgGameMessage = 0x026B,
    kServerReplyMsg = 0x026F,
    kNetMsgVoice = 0x0279,
    kNetMsgTestAndSet = 0x027D,
    kAvTaskMsg = 0x0298,
    kAvSeekMsg = 0x0299,
    kAvOneShotMsg = 0x029A,
    kNetMsgMembersListReq = 0x02AD,
    kNetMsgMembersList = 0x02AE,
    kNetMsgMemberUpdate = 0x02B1,
    kNetMsgInitialAgeStateSent = 0x02B8,
    kAvTaskSeekDoneMsg = 0x02C0,
    kNetMsgSDLState = 0x02CD,
    kNotifyMsg = 0x02ED,
    kLinkEffectsTriggerMsg = 0x0300,
    kNetMsgSDLStateBCast = 0x0329,
    kNetMsgGameMessageDirected = 0x032E,
    kAvatarInputStateMsg = 0x0347,
    kLinkingMgrMsg = 0x034B,
    kClothingMsg = 0x0357,
    kAvBrainHuman = 0x035C,
    kAvBrainCritter = 0x035D,
    kAvBrainDrive = 0x035E,
    kAvBrainGeneric = 0x0360,
    kInputIfaceMgrMsg = 0x0363,
    kKIMessage = 0x0364,
    kAvPushBrainMsg = 0x0367,
    kAvPopBrainMsg = 0x0368,
    kAvAnimTask = 0x036B,
    kAvSeekTask = 0x036C,
    kAvOneShotTask = 0x036E,
    kAvTaskBrain = 0x0370,
    kAnimStage = 0x0371,
    kAvBrainGenericMsg = 0x038F,
    kAvTaskSeek = 0x0390,
    kNetMsgRelevanceRegions = 0x03AC,
    kLoadAvatarMsg = 0x03B1,
    kNetMsgLoadClone = 0x03B3,
    kNetMsgPlayerPage = 0x03B4,
    kAvBrainSwim = 0x042D,
    kAvBrainClimb = 0x0453,
    kAvOneShotLinkTask = 0x0488,
    kAvBrainRideAnimatedPhysical = 0x049E,
};

QString Factory_Name(unsigned type);
QString Factory_Create(QTreeWidgetItem* parent, ChunkBuffer& buffer, size_t size);

void FlagField(QTreeWidgetItem* parent, QString title,
               unsigned flags, const char* names[]);
void Location(QTreeWidgetItem* parent, QString title, ChunkBuffer& buffer);
void Uoid(QTreeWidgetItem* parent, QString title, ChunkBuffer& buffer);
void Key(QTreeWidgetItem* parent, QString title, ChunkBuffer& buffer);
void BitVector(QTreeWidgetItem* parent, QString title, ChunkBuffer& buffer);
