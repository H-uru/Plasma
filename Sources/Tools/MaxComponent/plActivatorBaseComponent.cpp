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

#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"

#include "plActivatorBaseComponent.h"
#include "MaxMain/plMaxNode.h"

#include "plModifier/plLogicModifier.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plObjRefMsg.h"

void plActivatorBaseComponent::AddReceiverKey(plKey pKey, plMaxNode* node)
{
    fReceivers.insert(ReceiverKey(node, pKey));
}

plKey plActivatorBaseComponent::GetLogicKey(plMaxNode* node)
{
    LogicKeys::const_iterator it = fLogicModKeys.find(node);
    if (it != fLogicModKeys.end())
        return it->second;

    return nullptr;
}

void plActivatorBaseComponent::IGetReceivers(plMaxNode* node, std::vector<plKey>& receivers)
{
    // Add the guys who want to be notified by all instances
    ReceiverKeys::iterator lowIt = fReceivers.lower_bound(nullptr);
    ReceiverKeys::iterator highIt = fReceivers.upper_bound(nullptr);
    for (; lowIt != highIt; lowIt++)
        receivers.emplace_back(lowIt->second);

    // Add the ones for just this instance
    lowIt = fReceivers.lower_bound(node);
    highIt = fReceivers.upper_bound(node);
    for (; lowIt != highIt; lowIt++)
        receivers.emplace_back(lowIt->second);
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plActivatorBaseComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fLogicModKeys.clear();
    fReceivers.clear();
    return true;
}

bool plActivatorBaseComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    node->SetForceLocal(true);

    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();

    // Create and register the VolumeGadget's logic component
    plLogicModifier *logic = new plLogicModifier;
    plKey logicKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), logic, node->GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(logicKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    fLogicModKeys[node] = logicKey;

    return true;
}

bool plActivatorBaseComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
    fReceivers.clear();
    fLogicModKeys.clear();
    return plPhysicCoreComponent::DeInit( node, pErrMsg ); 
}
