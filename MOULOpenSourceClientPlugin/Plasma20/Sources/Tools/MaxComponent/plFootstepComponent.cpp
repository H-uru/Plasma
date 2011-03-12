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
#include "resource.h"
#include "hsConfig.h"
#include <map>
#include "hsResMgr.h"
#include "../MaxMain/plPlasmaRefMsgs.h"
#include "../MaxMain/plMaxNode.h"

#include "plFootstepComponent.h"
#include "plAudioComponents.h"
#include "plComponentReg.h"

#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plArmatureEffects.h"
#include "../plAvatar/plArmatureMod.h"
#include "../pfAudio/plRandomSoundMod.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "plPickNode.h"

void DummyCodeIncludeFuncFootstepSound()
{
}

CLASS_DESC(plFootstepSoundComponent, gFootstepSoundDesc, "Avatar FootstepSound",  "AvatarFootstepSound", COMP_TYPE_AVATAR, FOOTSTEP_SOUND_COMPONENT_CLASS_ID)

static plFootstepSoundComponentProc gFootstepSoundComponentProc;

ParamBlockDesc2 gFootstepSoundBk
(	

	plComponent::kBlkComp, _T("FootstepSound"), 0, &gFootstepSoundDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_FOOTSTEP_SOUND, IDS_COMP_FOOTSTEP_SOUND, 0, 0, &gFootstepSoundComponentProc,

	plFootstepSoundComponent::kSurface, _T("Surface"),		TYPE_INT,	0, 0,
		p_default, plArmatureEffectsMgr::kFootDirt,
		end,

	plFootstepSoundComponent::kSurfaceList,	_T("SoundGroups"),	TYPE_INODE_TAB, plArmatureEffectsMgr::kMaxSurface,		0, 0,
		end,

	plFootstepSoundComponent::kNodePicker, _T("NodePicker"),	TYPE_INODE,		0, 0,
		end,

	end
);

plFootstepSoundComponent::plFootstepSoundComponent()
{
	fClassDesc = &gFootstepSoundDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

extern const plArmatureMod * FindArmatureMod(const plSceneObject *obj);
/*{
	int count = obj->GetNumModifiers();

	for (int i = 0; i < count; i++)
	{
		const plModifier *mod = obj->GetModifier(i);
		const plArmatureMod * avMod = plArmatureMod::ConvertNoRef(mod);
		if(avMod)
			return avMod;
	}
	return nil;
}*/

hsBool plFootstepSoundComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plGenRefMsg *msg;
	plArmatureEffectFootSound *effect = TRACKED_NEW plArmatureEffectFootSound();
	
	// Note: MUST be a hard-coded keyname, since we search for same name in plArmatureMod.cpp
	hsgResMgr::ResMgr()->NewKey( "FootstepSounds", effect, node->GetLocation());

	int i;
	for (i = 0; i < plArmatureEffectsMgr::kMaxSurface; i++)
	{
		plMaxNode *compNode = (plMaxNode*)fCompPB->GetINode(ParamID(kSurfaceList), 0, i);
		if (compNode)
		{
			plRandomSoundComponent *rsComp = (plRandomSoundComponent *)compNode->ConvertToComponent();
			if (rsComp)
			{
				plRandomSoundMod *mod = rsComp->fSoundMods[node];
				if (mod != nil)
				{
					msg = TRACKED_NEW plGenRefMsg(effect->GetKey(), plRefMsg::kOnCreate, i, -1);
					hsgResMgr::ResMgr()->AddViaNotify(mod->GetKey(), msg, plRefFlags::kActiveRef);
				}
			}
		}
	}

	// Add it to the scene node's generic list, so that all avatars can access it.
	plNodeRefMsg* nodeRefMsg = TRACKED_NEW plNodeRefMsg(node->GetRoomKey(),	plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kGeneric);
	hsgResMgr::ResMgr()->AddViaNotify(effect->GetKey(), nodeRefMsg, plRefFlags::kActiveRef);
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

BOOL plFootstepSoundComponentProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2 *pb = pm->GetParamBlock();
	HWND hSurface = GetDlgItem(hWnd, IDC_COMP_FOOTSTEP_SOUND_SURFACE);
	HWND hPick = GetDlgItem(hWnd, IDC_COMP_FOOTSTEP_SOUND_PICK);
	INode *curPick = nil;
	int curSurface = 0;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			int i;
			for (i = 0; i < plArmatureEffectsMgr::kMaxSurface; i++)
				ComboBox_AddString(hSurface, plArmatureEffectsMgr::SurfaceStrings[i]);

			curSurface = pb->GetInt(ParamID(plFootstepSoundComponent::kSurface));
			ComboBox_SetCurSel(hSurface, curSurface);

			curPick = pb->GetINode(ParamID(plFootstepSoundComponent::kSurfaceList), 0, curSurface);
			Button_SetText(hPick, (curPick == nil ? "None" : curPick->GetName()));
		}
		return TRUE;


	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDC_COMP_FOOTSTEP_SOUND_PICK)
			{
				std::vector<Class_ID> cids;
				cids.push_back(RANDOM_SOUND_COMPONENT_ID);
				if (plPick::NodeRefKludge(pb, plFootstepSoundComponent::kNodePicker, &cids, true, false))			
				{
					curPick = pb->GetINode(ParamID(plFootstepSoundComponent::kNodePicker));
					curSurface = pb->GetInt(ParamID(plFootstepSoundComponent::kSurface));
					pb->SetValue(ParamID(plFootstepSoundComponent::kSurfaceList), 0, curPick, curSurface); 
					Button_SetText(hPick, (curPick == nil ? "None" : curPick->GetName()));
				}
			
				return TRUE;
			}
		}
		else if (LOWORD(wParam) == IDC_COMP_FOOTSTEP_SOUND_SURFACE)
		{
			curSurface = ComboBox_GetCurSel(hSurface);
			curPick = pb->GetINode(ParamID(plFootstepSoundComponent::kSurfaceList), 0, curSurface);
			pb->SetValue(ParamID(plFootstepSoundComponent::kSurface), 0, curSurface);
			Button_SetText(hPick, (curPick == nil ? "None" : curPick->GetName()));
		}
	}

	return FALSE;
}
