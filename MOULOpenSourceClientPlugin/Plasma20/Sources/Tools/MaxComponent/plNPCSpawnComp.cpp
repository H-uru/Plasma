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
#include "plNPCSpawnComp.h"

#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"

#include "plActivatorBaseComponent.h"

#include "../MaxMain/plMaxNode.h"

#include "../plAvatar/plNPCSpawnMod.h"

#include "../pnMessage/plNotifyMsg.h"

#include <map>


// Keep from getting dead-code-stripped
void DummyCodeIncludFuncNPCSpawn() {}

// DON'T delete from this enum. Just add _DEAD to the end of the name
enum
{
	kModelName,			// v1
	kAccountName,		// v1
	kAutoSpawn,			// v1
};

/** \class plNPCSpawnComp
	Simply creates a plNPCSpawnMod and applies it to the scene node.
*/
class plNPCSpawnComp : public plActivatorBaseComponent
{
public:
	plNPCSpawnComp();

	plKey GetNPCSpawnKey(plMaxNode *node);

	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode* node,plErrorMsg *pErrMsg);

private:
	// per-instance registry of all the modifiers that were created by this component
	typedef std::map<plMaxNode*, plNPCSpawnMod*> modmap;
	modmap fMods;
	bool IIsValid();
};

// Max class descriptor.
CLASS_DESC(plNPCSpawnComp, gNPCSpawnDesc, "NPC Spawner", "NPC Spawner", COMP_TYPE_AVATAR, NPC_SPAWN_CLASS_ID )

// GETNPCSPAWNKEY
// The idea here is to allow access to all the modifiers that
// were created by a given npc spawner component.
plKey GetNPCSpawnModKey(plComponentBase *npcSpawnComp, plMaxNodeBase *target)
{
	if (npcSpawnComp->ClassID() == NPC_SPAWN_CLASS_ID)
	{
		plNPCSpawnComp *comp = (plNPCSpawnComp*)npcSpawnComp;
		return comp->GetNPCSpawnKey((plMaxNode*)target);
	}

	return nil;
}

// GNPCSPAWNBLOCK
ParamBlockDesc2 gNPCSpawnBlock
(
	plComponent::kBlkComp, _T("(ex)One Shot Comp"), 0, &gNPCSpawnDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Rollout data
	IDD_COMP_NPC_SPAWN, IDS_COMP_NPC_SPAWNER, 0, 0, NULL,

	//params
	kModelName,	_T("ModelName"),	TYPE_STRING,	0, 0,
		p_ui,	TYPE_EDITBOX, IDC_NPC_SPAWN_MODEL_TEXT_BOX,
		end,

	//params
	kAccountName,	_T("AccountName"),	TYPE_STRING,	0, 0,
		p_ui,	TYPE_EDITBOX, IDC_NPC_SPAWN_ACCOUNT_TEXT_BOX,
		end,

	kAutoSpawn, _T("AutoSpawn"), TYPE_BOOL, 0,	0,
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_NPC_SPAWN_AUTOSPAWN_BOOL,
		end,

	end
);

plNPCSpawnComp::plNPCSpawnComp()
{
	fClassDesc = &gNPCSpawnDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// GETNPCSPAWNKEY
plKey plNPCSpawnComp::GetNPCSpawnKey(plMaxNode *node)
{
	if (fMods.find(node) != fMods.end())
		return fMods[node]->GetKey();

	return nil;
}

// ISVALID
bool plNPCSpawnComp::IIsValid()
{
	const char *modelName = fCompPB->GetStr(kModelName);
	// account name is optional (for the moment)
	//const char *account = fCompPB->GetStr(kAccountName);
	return (modelName && *modelName != '\0');
}

// SETUPPROPERTIES
hsBool plNPCSpawnComp::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fMods.clear();			// clear out our cache of modifiers (used for interfacing to python & responder)

	if (IIsValid())
	{
		node->SetForceLocal(true);
		return true;
	}
	else
	{
		if (pErrMsg->Set(true, "NPC Spawner", "NPC Spawn component on '%s' has no animation name, and will not be included in the export. Abort this export?", node->GetName()).Ask())
			pErrMsg->Set(true, "", "");
		else
			pErrMsg->Set(false); // Don't want to abort
		return false;
	}
}

// PRECONVERT
// We actually do the convert here so we're around for responder & python fixup
hsBool plNPCSpawnComp::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (IIsValid())
	{
		const char *modelName = fCompPB->GetStr(kModelName);
		const char *accountName = fCompPB->GetStr(kAccountName);
		bool autoSpawn = fCompPB->GetInt(kAutoSpawn) ? true : false;

		plNPCSpawnMod *mod = TRACKED_NEW plNPCSpawnMod(modelName, accountName, autoSpawn);
		fMods[node] = mod;

		// this is used by the python file modifier to figure out which component we're coming from
		plKey modKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), mod, node->GetLocation());
		fLogicModKeys[node] = modKey;
	}

	return true;
}

// CONVERT
// We just attach to the scene object here. Not sure why we didn't do it at convert time;
// I'm cribbing from Colin's modifications to the one shot component.
hsBool plNPCSpawnComp::Convert(plMaxNode* node, plErrorMsg *pErrMsg)
{
	modmap::iterator i = fMods.find(node);

	if (i != fMods.end())
	{
		plNPCSpawnMod *mod = ((*i).second);

		// let's make a notification message that we'll use to notify interested parties
		// when we actually do our spawn.
		plNotifyMsg *notify = TRACKED_NEW plNotifyMsg();
		hsTArray<plKey> receivers;
		IGetReceivers(node, receivers);
		notify->SetSender(mod->GetKey());
		notify->SetState(1.0f);
		for (int i = 0; i < receivers.Count(); i++)
			notify->AddReceiver(receivers[i]);

		mod->SetNotify(notify);

		node->AddModifier(mod, IGetUniqueName(node));

		return true;
	}

	return false;
}