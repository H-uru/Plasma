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
#include "plActivatorBaseComponent.h"

#include "../pnKeyedObject/plKey.h"
#include "../MaxMain/plMaxNode.h"

#include "../plModifier/plLogicModifier.h"
#include "../pnSceneObject/plSceneObject.h"
#include "hsResMgr.h"
#include "../pnMessage/plObjRefMsg.h"

void plActivatorBaseComponent::AddReceiverKey(plKey pKey, plMaxNode* node)
{
	fReceivers.insert(ReceiverKey(node, pKey));
}

plKey plActivatorBaseComponent::GetLogicKey(plMaxNode* node)
{
	LogicKeys::const_iterator it = fLogicModKeys.find(node);
	if (it != fLogicModKeys.end())
		return it->second;

	return nil;
}

void plActivatorBaseComponent::IGetReceivers(plMaxNode* node, hsTArray<plKey>& receivers)
{
	// Add the guys who want to be notified by all instances
	ReceiverKeys::iterator lowIt = fReceivers.lower_bound(nil);
	ReceiverKeys::iterator highIt = fReceivers.upper_bound(nil);
	for (; lowIt != highIt; lowIt++)
		receivers.Append(lowIt->second);

	// Add the ones for just this instance
	lowIt = fReceivers.lower_bound(node);
	highIt = fReceivers.upper_bound(node);
	for (; lowIt != highIt; lowIt++)
		receivers.Append(lowIt->second);
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plActivatorBaseComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fLogicModKeys.clear();
	fReceivers.clear();
	return true;
}

hsBool plActivatorBaseComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetForceLocal(true);

	plLocation loc = node->GetLocation();
	plSceneObject *obj = node->GetSceneObject();

	// Create and register the VolumeGadget's logic component
	plLogicModifier *logic = TRACKED_NEW plLogicModifier;
	plKey logicKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), logic, node->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(logicKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	fLogicModKeys[node] = logicKey;

	return true;
}

hsBool plActivatorBaseComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
	fReceivers.clear();
	fLogicModKeys.clear();
	return plPhysicCoreComponent::DeInit( node, pErrMsg ); 
}
