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

void Create_AnimStage(QTreeWidgetItem* parent, ChunkBuffer& buffer);

void Create_ArmatureBrain(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainClimb(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainCritter(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainDrive(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainGeneric(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainHuman(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainRideAnimatedPhysical(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvBrainSwim(QTreeWidgetItem* parent, ChunkBuffer& buffer);

void Create_AvAnimTask(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvOneShotTask(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvOneShotLinkTask(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvSeekTask(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvTaskBrain(QTreeWidgetItem* parent, ChunkBuffer& buffer);
void Create_AvTaskSeek(QTreeWidgetItem* parent, ChunkBuffer& buffer);
