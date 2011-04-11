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
#ifndef plClickDragComponent_inc
#define plClickDragComponent_inc

#include "plClickableComponent.h"

#define CLICK_DRAG_CID Class_ID(0x61880560, 0x348d72d3)

class plClickDragComponent : public plClickableComponent
{
protected:
	LogicKeys fAxisKeys;

public:
	plClickDragComponent();

	// Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool DeInit(plMaxNode *node, plErrorMsg* pErrMsg);

	virtual plKey GetAxisKey(plMaxNode* node);
	const LogicKeys& GetAxisKeys();

	virtual void CollectNonDrawables(INodeTab& nonDrawables);

};

#endif // plClickDragComponent_inc