/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "HeadSpin.h"

#include "max.h"

#include "plNodeLock.h"

void plNodeLock::Lock(BOOL on)
{
	Interface *ip = GetCOREInterface();
	ISetLockRecur(ip->GetRootNode(), on);
}

void plNodeLock::ISetLockRecur(INode* node, BOOL on)
{
	BOOL isSel = node->Selected();
	if( isSel )
	{
		node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_X, on);
		node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_Y, on);
		node->SetTransformLock(INODE_LOCKPOS, INODE_LOCK_Z, on);

		node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_X, on);
		node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_Y, on);
		node->SetTransformLock(INODE_LOCKROT, INODE_LOCK_Z, on);

		node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_X, on);
		node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_Y, on);
		node->SetTransformLock(INODE_LOCKSCL, INODE_LOCK_Z, on);
	}
	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
		ISetLockRecur(node->GetChildNode(i), on);
}
