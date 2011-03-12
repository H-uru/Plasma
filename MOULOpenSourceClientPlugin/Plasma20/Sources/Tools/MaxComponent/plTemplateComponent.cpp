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
#include "plComponent.h"
#include "plComponentReg.h"

#include "hsTemplates.h"
#include "../MaxMain/plPluginResManager.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/plKey.h"

#include "plMiscComponents.h"

#include "resource.h"

void DummyCodeIncludeFuncTemplate()
{
}

static const char *GetPBString(IParamBlock2 *pb, ParamID id)
{
	const char *str = pb->GetStr(id, 0);
	if (str && *str == '\0')
		return nil;
	return str;
}

class plTemplateComponent : public plComponent
{
protected:
	const char* IGetAgeName(plMaxNode *node);

public:
	plTemplateComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

CLASS_DESC(plTemplateComponent, gTemplateDesc, "Template", "CloneTemplate", "Clone", Class_ID(0x6742590b, 0x14fd2135))

enum
{
	kTemplateName
};

ParamBlockDesc2 gTemplateBlk
(
	plComponent::kBlkComp, _T("Template"), 0, &gTemplateDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

	end
);


plTemplateComponent::plTemplateComponent()
{
	fClassDesc = &gTemplateDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

const char* plTemplateComponent::IGetAgeName(plMaxNode *node)
{
	UInt32 numComps = node->NumAttachedComponents();
	for (UInt32 i = 0; i < numComps; i++)
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if (comp->ClassID() == PAGEINFO_CID)
		{
			plPageInfoComponent* pageInfo = (plPageInfoComponent*)comp;
			return pageInfo->GetAgeName();
		}
	}

	return nil;
}

#include "../MaxMain/plMaxNodeData.h"

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plTemplateComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	const char* ageName = IGetAgeName(node);
	if (!ageName)
		return false;

#if 0
	const char *templateName = node->GetName();
	plKey roomKey = plPluginResManager::ResMgr()->NameToLoc(ageName, "District", "BuiltIn", (UInt32)-1);

	// Set this object and all its children to be in the special template age
	node->SetRoomKey(roomKey);
	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode* childNode = (plMaxNode*)node->GetChildNode(i);
		childNode->SetRoomKey(roomKey);
	}
#endif

	// We need a coordinate interface so we can move to the clone position
	node->SetForceLocal(true);

	node->GetMaxNodeData()->SetItinerant(true);

	return true;
}

hsBool plTemplateComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (node->GetSceneObject())
		node->GetSceneObject()->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);

	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode* childNode = (plMaxNode*)node->GetChildNode(i);

		if (childNode->GetSceneObject())
			childNode->GetSceneObject()->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////

#include "../plModifier/plCloneSpawnModifier.h"

class plSpawnComponent : public plComponent
{
public:
	plSpawnComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

CLASS_DESC(plSpawnComponent, gSpawnDesc, "Instance", "CloneInst", "Clone", Class_ID(0x5702450d, 0x2c636131))

ParamBlockDesc2 gSpawnBlk
(
	plComponent::kBlkComp, _T("Spawn"), 0, &gSpawnDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_TEMPLATE, IDS_COMP_CLONE_INST, 0, 0, NULL,

	kTemplateName,	_T("name"),		TYPE_STRING,	0, 0,
		p_ui,		TYPE_EDITBOX, IDC_NAME,
		end,

	end
);


plSpawnComponent::plSpawnComponent()
{
	fClassDesc = &gSpawnDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plSpawnComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!GetPBString(fCompPB, kTemplateName))
	{
		pErrMsg->Set(true, "Clone Instance Component", "Clone Instance component on node %s can't convert because it doesn't have a name", node->GetName());
		pErrMsg->Set(false);
		return false;
	}
	
	// We need a coordinate interface to find the point to warp the clone to.
	node->SetForceLocal(true);

	return true;
}

hsBool plSpawnComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	const char *templateName = GetPBString(fCompPB, kTemplateName);
	if (!templateName)
		return false;

	plCloneSpawnModifier* mod = TRACKED_NEW plCloneSpawnModifier;
	mod->SetExportTime();
	mod->SetTemplateName(templateName);
	node->AddModifier(mod, IGetUniqueName(node));

	return true;
}
