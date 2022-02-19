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

void Create_NetMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgGameMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgRoomsList(QTreeWidgetItem* parent, ChunkBuffer& buffer);

void Create_NetMsgGameMessageDirected(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgGroupOwner(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgInitialAgeStateSent(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgLoadClone(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgMembersList(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgMembersListReq(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgMemberUpdate(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgPagingRoom(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgPlayerPage(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgRelevanceRegions(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgSDLState(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NetMsgSharedState(QTreeWidgetItem* parent, ChunkBuffer& buffer);
