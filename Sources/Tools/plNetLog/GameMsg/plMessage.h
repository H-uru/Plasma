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

void Create_Message(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_LoadCloneMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);

void Create_AvatarInputStateMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainGenericMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvTaskMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_InputIfaceMgrMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_KIMessage(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_LinkEffectsTriggerMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_LoadAvatarMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_NotifyMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_ServerReplyMsg(QTreeWidgetItem* parent, ChunkBuffer& buffer);
