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
#include "resource.h"							//	Resource Dependencies

#include "../MaxMain/plPhysicalProps.h"

#include "plComponent.h"						//Component Dependencies
#include "plComponentReg.h"						//	Ibid
#include "../MaxMain/plMaxNode.h"				//	Ibid
#include "../pnKeyedObject/plKey.h"				//	Ibid
#include "plComponentProcBase.h"

#include "plNavigableComponents.h"
#include "plActivatorBaseComponent.h"
#include "plPhysicalComponents.h"

#include "../MaxConvert/hsConverterUtils.h"		//Conversion Dependencies
#include "../MaxConvert/hsControlConverter.h"	//	Ibid

#include "../plAvatar/plAvLadderModifier.h"		//Modifier Dependencies
#include "../plPhysical/plSimDefs.h"

#include "plgDispatch.h"						//Message Dependencies
#include "../pnMessage/plObjRefMsg.h"			//	Ibid
#include "../pnMessage/plIntRefMsg.h"			//	Ibid	
#include "../pnMessage/plNodeRefMsg.h"			//	Ibid
#include "../MaxMain/plPlasmaRefMsgs.h"			//	Ibid

void DummyCodeIncludeFuncNavigablesRegion() {}


CLASS_DESC(plAvLadderComponent, gAvLadderComponentDesc, "(ex)Ladder Component", "(ex)LadderComp", COMP_TYPE_PHYS_TERRAINS, NAV_LADDER_CID)

class plAvLadderComponentProc;
extern plAvLadderComponentProc gAvLadderComponentProc;

enum kAvLadderFields
{
	kTypeCombo,
	kLoopsInt,
	kTriggerNode,
	kDirectionBool,
	kBoundsType_DEAD,
	kEnabled,
	kLadderNode,
};

ParamBlockDesc2 gAvLadderComponentBlock
(
	plComponent::kBlkComp, _T("(ex)Ladder Component"), 0, &gAvLadderComponentDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_NAV_LADDER, IDS_COMP_NAV_LADDERS, 0, 0, &gAvLadderComponentProc,

	kTypeCombo, _T("Ladder Type"), TYPE_INT, 0,0,
		end,

	kDirectionBool, _T("Climbing Direction"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 2,	IDC_RADIO_UP,	IDC_RADIO_DOWN,
		p_vals,						true,			false,
		end,

	kLoopsInt,	_T("BigLadderNumLoop"),	TYPE_INT, 0, 0,	
		p_default, 0,
		p_range, 0, 500,
		p_ui,	TYPE_SPINNER,	EDITTYPE_INT,
		IDC_COMP_NAV_LADDER_LOOPS_EDIT, IDC_COMP_NAV_LADDER_LOOPS_SPIN, 0.4,
		end,

	kTriggerNode, _T("Trigger Node"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_NAV_TRIGGER,
		//p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		//p_accessor, &gPhysCoreAccessor,
		end,

	kLadderNode, _T("ladder"),			TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_NAV_LADDER,
		end,

	kEnabled,	_T("enabled"),		TYPE_BOOL,		0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_ENABLED,
		p_default, TRUE,
		end,

	end
);

plAvLadderComponent::plAvLadderComponent()
{
	fClassDesc = &gAvLadderComponentDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plAvLadderComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	INode* ladderNode = fCompPB->GetINode(kLadderNode);
	if( ladderNode )
		nonDrawables.Append(1, &ladderNode);

	INode* triggerNode = fCompPB->GetINode(kTriggerNode);
	if( triggerNode )
		nonDrawables.Append(1, &triggerNode);
	
	AddTargetsToList(nonDrawables);
}

hsBool plAvLadderComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fKeys.Reset();

	//
	// Create an invisible blocker for the ladder shape, so the avatar won't fall over the side
	//
	plMaxNode *ladderNode = (plMaxNode*)fCompPB->GetINode(kLadderNode);
	if (ladderNode)
	{
		plPhysicalProps* ladderPhys = ladderNode->GetPhysicalProps();
//		ladderPhys->SetMass(0, ladderNode, pErrMsg);
		ladderPhys->SetBoundsType(plSimDefs::kHullBounds, ladderNode, pErrMsg);
		ladderPhys->SetGroup(plSimDefs::kGroupStatic, ladderNode, pErrMsg);
///		ladderPhys->SetAllowLOS(true, ladderNode, pErrMsg);

		ladderNode->SetDrawable(false);
		ladderNode->SetForceLocal(true);	// Get a coord interface for facing calculations
	}
	else
	{
		pErrMsg->Set(true,
					"Ladder Warning",
					"Ladder component %s doesn't have the ladder node set",
					GetINode()->GetName()).Show();
		pErrMsg->Set(false);
		return false;
	}

	//
	// Create a detector region for the node we're attached to
	//
	plPhysicalProps *physProps = node->GetPhysicalProps();

	plMaxNode *triggerNode = (plMaxNode*)fCompPB->GetINode(kTriggerNode);
	if (triggerNode)
		physProps->SetProxyNode(triggerNode, node, pErrMsg);

	physProps->SetGroup(plSimDefs::kGroupDetector, node, pErrMsg);		// this is a detector
	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, node, pErrMsg);	// only fires on local avatars
	physProps->SetPinned(true, node, pErrMsg);
	// only if movable will it have mass (then it will keep track of movements in PhysX)
	if ( node->IsMovable() || node->IsTMAnimated() )
		physProps->SetMass(1.0f, node, pErrMsg);										// detectors don't move
	physProps->SetBoundsType(plSimDefs::kHullBounds, node, pErrMsg);

	node->SetForceLocal(true);	// force our seek point to be local
	node->SetDrawable(false);

	return true;
}

hsBool plAvLadderComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plMaxNode *ladderNode = (plMaxNode*)fCompPB->GetINode(kLadderNode);
	if (!ladderNode)
		return false;

	plMaxNode *triggerNode = (plMaxNode*)fCompPB->GetINode(kTriggerNode);
	if (!triggerNode)
		triggerNode = node;

	// Get a vector pointing from the ladder to the detector, for facing calculations
	Point3 ladderViewMax = ladderNode->GetNodeTM(0).GetTrans() - triggerNode->GetNodeTM(0).GetTrans();
	hsVector3 ladderView(ladderViewMax.x, ladderViewMax.y, ladderViewMax.z);
	ladderView.fZ = 0;
	ladderView.Normalize();

	bool goingUp = (fCompPB->GetInt(kDirectionBool) != 0);
	int loops = fCompPB->GetInt(kLoopsInt);
	int ladderType = fCompPB->GetInt(kTypeCombo);
	bool enabled = (fCompPB->GetInt(kEnabled) != 0);

	plAvLadderMod* ladMod = TRACKED_NEW plAvLadderMod(goingUp, ladderType, loops, enabled, ladderView);
	plKey modKey = node->AddModifier(ladMod, IGetUniqueName(node));
	fKeys.Append(modKey);

	return true;
}

hsBool plAvLadderComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plAvLadderComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fKeys.Reset();
	return true;
}

class plAvLadderComponentProc : public ParamMap2UserDlgProc
{
public:
	enum kLadderTypesEnums
	{
		kReallyBig,
		kFourFeet,
		kTwoFeet,
	};

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
				HWND hLadder = GetDlgItem(hWnd,IDC_COMP_NAV_LADDER_COMBO);

				ComboBox_AddString(hLadder, "Big");
				ComboBox_AddString(hLadder, "4 feet");
				ComboBox_AddString(hLadder, "2 feet");

				int type = map->GetParamBlock()->GetInt(kTypeCombo);
				ComboBox_SetCurSel(hLadder, type);
			}
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_COMP_NAV_LADDER_COMBO && HIWORD(wParam) == CBN_SELCHANGE)
			{
				//Util fcn found in plEventGroupRefs files in MaxMain
				HWND hLadder = GetDlgItem(hWnd,IDC_COMP_NAV_LADDER_COMBO);
				int idx = ComboBox_GetCurSel(hLadder);
				map->GetParamBlock()->SetValue(kTypeCombo, 0, idx);
/*
				if (idx == kReallyBig)
				{
					map->Enable(kLoopsInt, TRUE);
//					map->Invalidate(kLoopsInt);
				}
				else
				{
					map->Enable(kLoopsInt, TRUE);
//					map->Invalidate(kLoopsInt);
				}
*/
				return TRUE;
			}
			break;
		}
		return FALSE;
	}

	void DeleteThis() {}
};
static plAvLadderComponentProc gAvLadderComponentProc;


