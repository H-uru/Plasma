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
#ifndef plActivatorBaseComponent_inc
#define plActivatorBaseComponent_inc

#include "plComponent.h"
#include "plPhysicalComponents.h"
#include <map>
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

class plMaxNode;

#define ACTIVATOR_BASE_CID Class_ID(0x23915577, 0x2d0f4cdd)

class plActivatorBaseComponent : public plPhysicCoreComponent
{
public:
	typedef std::map<plMaxNode*, plKey> LogicKeys;
protected:
	LogicKeys fLogicModKeys;
	typedef std::multimap<plMaxNode*, plKey> ReceiverKeys;
	typedef std::pair<plMaxNode*, plKey> ReceiverKey;
	ReceiverKeys fReceivers;
//	hsTArray<plKey> fReceivers;

	void IGetReceivers(plMaxNode* node, hsTArray<plKey>& receivers);

public:
	// Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual	hsBool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

	const LogicKeys& GetLogicKeys() { return fLogicModKeys; }
	virtual plKey GetLogicKey(plMaxNode* node);
	virtual void AddReceiverKey(plKey pKey, plMaxNode* node=nil);
	virtual bool HasLogicOut() { return false; }

	int CanConvertToType(Class_ID obtype)
	{ return (obtype == ACTIVATOR_BASE_CID) ? 1 : plComponent::CanConvertToType(obtype); }
};

#endif // plActivatorBaseComponent_inc