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
#include "hsTypes.h"

#include "SceneWatcher.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxComponent/plComponent.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../pnKeyedObject/plKey.h"

SceneWatcher::SceneWatcher() : fDirty(false)
{
	RegisterNotification(INotify, this, NOTIFY_NODE_CREATED);
	IAddNodeRecur((plMaxNode*)GetCOREInterface()->GetRootNode());
}

SceneWatcher::~SceneWatcher()
{
	DeleteAllRefsFromMe();
	UnRegisterNotification(INotify, this, NOTIFY_NODE_CREATED);
}

int SceneWatcher::NumRefs()
{
	return fNodes.size();
}

RefTargetHandle SceneWatcher::GetReference(int i)
{
	if (i < fNodes.size())
		return fNodes[i];

	hsAssert(0, "Index out of range");
	return nil;
}

void SceneWatcher::SetReference(int i, RefTargetHandle rtarg)
{
	if (i < fNodes.size())
		fNodes[i] = (plMaxNode*)rtarg;
	else
		hsAssert(0, "Index out of range");
}

//
// Something in the scene has changed.
//
RefResult SceneWatcher::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
						   PartID& partID, RefMessage message)
{
	plMaxNode *node = (plMaxNode *)hTarget;

#ifdef HS_DEBUGGING
	char *tmp = node->GetName();
#endif

	if (message == REFMSG_CHANGE)
	{
		// If the message is from a component, and was not generated locally (ie.
		// it came from a ref parameter), ignore it.  There is no way to tell if
		// one of these messages is an actual change to the component or just a
		// change to the referenced object.  We'll catch the real changes with
		// REFMSG_USER_COMP_REF_CHANGED below.
		if (node->IsComponent())
		{
			plComponentBase *comp = node->ConvertToComponent();
			if (!comp->IsCurMsgLocal())
				return REF_SUCCEED;
		}

		// If this is a static light, ignore it
		Object *obj = node->GetObjectRef();
		if (obj && obj->SuperClassID() == LIGHT_CLASS_ID && !node->GetRunTimeLight())
			return REF_SUCCEED;

		node->SetDirty(plMaxNode::kGeomDirty, true);
		ISetDirty();
	}
	else if (message == REFMSG_TARGET_DELETED)
	{
		// If the deleted node was a component, dirty everyone it was attached to
		if (node->IsComponent())
		{
			plComponentBase *comp = node->ConvertToComponent();
			for (UInt32 i = 0; i < comp->NumTargets(); i++)
			{
				if (comp->GetTarget(i))
					comp->GetTarget(i)->SetDirty(plMaxNode::kGeomDirty, true);
			}
		}

		fDeleted.push_back(node->GetKey());

		IRemoveRef(node);
		ISetDirty();
	}
	else if (message == REFMSG_NODE_MATERIAL_CHANGED ||
			message == REFMSG_USER_MAT)
	{
		if (!node->IsComponent())
		{
			node->SetDirty(plMaxNode::kMatDirty, true);
			ISetDirty();
		}
	}
	// A node was added to the components target list
	else if (message == REFMSG_USER_TARGET_ADD)
	{
		plMaxNode *target = (plMaxNode*)partID;
		target->SetDirty(plMaxNode::kGeomDirty, true);
		ISetDirty();
	}
	// A node was deleted from the components target list
	else if (message == REFMSG_USER_TARGET_DELETE)
	{
		plMaxNode *target = (plMaxNode*)partID;
		target->SetDirty(plMaxNode::kGeomDirty, true);
		ISetDirty();
	}
	// A ref maintained by a component PB changed (not just a propagated message
	// from the ref)
	else if (message == REFMSG_USER_COMP_REF_CHANGED)
	{
		node->SetDirty(plMaxNode::kGeomDirty, true);
		ISetDirty();
	}

	return REF_SUCCEED;
}

bool SceneWatcher::AnyDeleted()
{
	return !fDeleted.empty();
}

SceneWatcher::KeyList& SceneWatcher::GetDeleted()
{
	return fDeleted;
}

const SceneWatcher::NodeList& SceneWatcher::GetWatchNodes()
{
	return fNodes;
}

bool SceneWatcher::AnyDirty()
{
	return fDirty;
}

void SceneWatcher::GetDirty(NodeSet& dirtyNodes)
{
	int size = fNodes.size();
	for (int i = 0; i < size; i++)
	{
		plMaxNode *node = fNodes[i];

#ifdef HS_DEBUGGING
		const char *tmp = node ? node->GetName() : nil;
#endif
		// If any dirty flags are set, add to dirty list
		if (node && node->GetDirty(plMaxNode::kAllDirty))
			IGetDependents(node, dirtyNodes);
	}

	fDirty = false;
}

void SceneWatcher::IAddRef(plMaxNode *node)
{
	// Ensure that we don't already ref this node
	if (FindRef(node) != -1)
		return;

	// Make a ref
	int size = fNodes.size();
	fNodes.resize(size+1);
	MakeRefByID(FOREVER, size, node);
}

void SceneWatcher::IRemoveRef(plMaxNode *node)
{
	// Delete the reference if it's in our list
	int i = FindRef(node);

	if (i != -1)
	{
		// Clear the node data, in case it is undeleted later
		// (when it will be invalid)
		node->ClearData(nil, nil);
		node->SetDirty(plMaxNode::kAllDirty, true);

		// We can never really delete this reference because the user may "undo"
		// and cause it to come back, which just sets the reference again.
		fNodes[i] = nil;
	}
}

void SceneWatcher::IAddNodeRecur(plMaxNode *node)
{
	IAddRef(node);

	// If a node is dirty, make sure to set the dirty flag (since nodes may have
	// been dirtied in a previous run but not reconverted yet).
	if (node->GetDirty(plMaxNode::kAllDirty))
		fDirty = true;

	for (int i = 0; i < node->NumberOfChildren(); ++i)
	{
		plMaxNode *childNode = (plMaxNode*)node->GetChildNode(i);
		IAddNodeRecur(childNode);
	}
}

void SceneWatcher::INotify(void *param, NotifyInfo *info)
{
	SceneWatcher *inst = (SceneWatcher*)param;

	int code = info->intcode;

	// New node was added to the scene, add it to our refs
	if (code == NOTIFY_NODE_CREATED)
	{
		plMaxNode *node = (plMaxNode*)info->callParam;

		// Add a ref to the node and set it to dirty
		inst->IAddRef(node);
		node->SetDirty(plMaxNodeBase::kAllDirty, true);
		inst->ISetDirty();
	}
}

void SceneWatcher::ISetDirty()
{
	fDirty = true;
}

#include "../MaxComponent/plMiscComponents.h"

#include "../MaxExport/plExportErrorMsg.h"
#include "../MaxExport/plExportProgressBar.h"
#include "../MaxComponent/plResponderComponent.h"	// Just need the CID

void SceneWatcher::IGetLogicDependents(plMaxNode *node, NodeSet& nodes)
{
	int attached = node->NumAttachedComponents();
	for (int i = 0; i < attached; i++)
	{
		plComponentBase *comp = node->GetAttachedComponent(i);
/*
		if (comp->ClassID() == ACTIVATOR_CID)
		{
			plMaxNodeBase *activatorNode = comp->GetINode();
			int numResponders = activatorNode->NumAttachedComponents(true);
			for (int i = 0; i < numResponders; i++)
			{
				plComponentBase *responderComp = activatorNode->GetAttachedComponent(i, true);
				if (responderComp->ClassID() == RESPONDER_CID)
				{
					for (int j = 0; j < responderComp->NumTargets(); j++)
					{
						plMaxNode *targ = (plMaxNode*)responderComp->GetTarget(j);
						if (targ && nodes.find(targ) == nodes.end())
						{
							nodes.insert(targ);
							IGetLogicDependents(targ, nodes);
						}
					}
				}
			}
		}
		else
*/
		if (comp->ClassID() == RESPONDER_CID)
		{
			int activatorCnt = ResponderGetActivatorCount(comp);
			for (int i = 0; i < activatorCnt; i++)
			{
				plComponentBase *activator = ResponderGetActivator(comp, i);

				for (int j = 0; j < activator->NumTargets(); j++)
				{
					plMaxNode *targ = (plMaxNode*)activator->GetTarget(j);
					if (targ && nodes.find(targ) == nodes.end())
					{
						nodes.insert(targ);
						IGetLogicDependents(targ, nodes);
					}
				}
			}
		}
	}
}

void SceneWatcher::IGetDependents(plMaxNode *node, NodeSet& nodes)
{
	NodeSet dependents;

	if (node->IsComponent())
	{
		plComponentBase *comp = node->ConvertToComponent();

		if (comp->ClassID() == ROOM_CID || comp->ClassID() == PAGEINFO_CID)
			return;

		for (int i = 0; i < comp->NumTargets(); i++)
		{
			plMaxNode *targ = (plMaxNode*)comp->GetTarget(i);
			if (targ)
			{
				dependents.insert(targ);
				IGetLogicDependents(targ, dependents);
			}
		}
	}
	else
	{
		dependents.insert(node);

		IGetLogicDependents(node, dependents);
	}

//	Bug in VC++?
//	nodes.insert(dependents.begin(), dependents.end());
	for (NodeSet::iterator i = dependents.begin(); i != dependents.end(); i++)
		nodes.insert(*i);
}