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
#include "plComponent.h"
#include "plComponentReg.h"
#include "../plMath/plRandom.h"
#include "plObjectFlockerComponent.h"
#include "../pnKeyedObject/plUoid.h"
#include "../MaxMain/plMaxNode.h"

#include "plPickNode.h"
#include "../pfAnimation/pfObjectFlocker.h"
#include "../MaxMain/plPluginResManager.h"
#include "../pnSceneObject/plSceneObject.h"

void DummyCodeIncludeFuncObjectFlocker()
{
}

class ObjectFlockerDlgProc : public ParamMap2UserDlgProc
{
public:
	ObjectFlockerDlgProc() {}
	~ObjectFlockerDlgProc() {}

	void IUpdateNode(TimeValue t, IParamBlock2* pb, HWND hWnd, ParamID buttonID, int button)
	{
		INode* node = pb->GetINode(buttonID, t);
		HWND hButton = GetDlgItem(hWnd, button);

		if (node)
			SetWindowText(hButton, node->GetName());
		else
			SetWindowText(hButton, "<none>");
	}

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();

		switch (msg)
		{
		case WM_INITDIALOG:
			IUpdateNode(t, pb, hWnd, plObjectFlockerComponent::kBoidObject, IDC_OBJ_FLOCKER_BOID_BUTTON);

			// Disable stuff that the artists shouldn't have to touch
			EnableWindow(GetDlgItem(hWnd, IDC_OBJ_FLOCKER_SEP_RADIUS), false);
			EnableWindow(GetDlgItem(hWnd, IDC_OBJ_FLOCKER_SEP_RADIUS_SPIN), false);
			SetDlgItemText(hWnd, IDC_OBJ_FLOCKER_SEP_RADIUS, "5.0");

			EnableWindow(GetDlgItem(hWnd, IDC_OBJ_FLOCKER_COH_RADIUS), false);
			EnableWindow(GetDlgItem(hWnd, IDC_OBJ_FLOCKER_COH_RADIUS_SPIN), false);
			SetDlgItemText(hWnd, IDC_OBJ_FLOCKER_COH_RADIUS, "9.0");

			return TRUE;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static ObjectFlockerDlgProc gObjectFlockerDlgProc;

CLASS_DESC(plObjectFlockerComponent, gObjectFlockerDesc, "Object Flocker",  "Object Flocker", COMP_TYPE_MISC, OBJECT_FLOCKER_COMPONENT_CLASS_ID)

ParamBlockDesc2 gObjectFlockerBk
(
 	plComponent::kBlkComp, _T("ObjectFlocker"), 0, &gObjectFlockerDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_OBJ_FLOCKER, IDS_COMP_OBJ_FLOCKER, 0, 0, &gObjectFlockerDlgProc,

	plObjectFlockerComponent::kBoidObject, _T("BoidObject"), TYPE_INODE, 0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_OBJ_FLOCKER_BOID_BUTTON,
		//p_sclassID,	GEOMOBJECT_CLASS_ID,
		end,

	plObjectFlockerComponent::kNumBoids,	_T("NumBoids"),	TYPE_INT, 0, 0,
		p_default, 5,
		p_range, 2, 30,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,
		IDC_OBJ_FLOCKER_NUM_BOIDS, IDC_OBJ_FLOCKER_NUM_BOIDS_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kGoalStrength,	_T("GoalStrength"),	TYPE_FLOAT, 0, 0,
		p_default, 8.0,
		p_range, 00.0, 50.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_GOAL_STRENGTH, IDC_OBJ_FLOCKER_GOAL_STRENGTH_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kWanderStrength,	_T("WanderStrength"),	TYPE_FLOAT, 0, 0,
		p_default, 12.0,
		p_range, 00.0, 50.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_WANDER_STRENGTH, IDC_OBJ_FLOCKER_WANDER_STRENGTH_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kSepStrength,	_T("SeparationStrength"),	TYPE_FLOAT, 0, 0,
		p_default, 12.0,
		p_range, 00.0, 50.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_SEP_STRENGTH, IDC_OBJ_FLOCKER_SEP_STRENGTH_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kSepRadius,	_T("SeparationRadius"),	TYPE_FLOAT, 0, 0,
		p_default, 05.0,
		p_range, 00.0, 50.0,
		/*p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_SEP_RADIUS, IDC_OBJ_FLOCKER_SEP_RADIUS_SPIN, 1.0,*/ // Commented out so Max doesn't auto-enable these
		end,

	plObjectFlockerComponent::kCohStrength, _T("CohesionStrength"),	TYPE_FLOAT, 0, 0,
		p_default, 08.0,
		p_range, 00.0, 50.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_COH_STRENGTH, IDC_OBJ_FLOCKER_COH_STRENGTH_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kCohRadius,	_T("CohesionRadius"),	TYPE_FLOAT, 0, 0,
		p_default, 09.0,
		p_range, 00.0, 50.0,
		/*p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_COH_RADIUS, IDC_OBJ_FLOCKER_COH_RADIUS_SPIN, 1.0,*/ // Commented out so Max doesn't auto-enable these
		end,

	plObjectFlockerComponent::kMaxForce, _T("MaxForce"),	TYPE_FLOAT, 0, 0,
		p_default, 10.0,
		p_range, 00.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_MAX_FORCE, IDC_OBJ_FLOCKER_MAX_FORCE_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kMaxSpeed,	_T("MaxSpeed"),	TYPE_FLOAT, 0, 0,
		p_default, 05.0,
		p_range, 00.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_SLIMIT_MAX, IDC_OBJ_FLOCKER_SLIMIT_MAX_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kMinSpeed,	_T("MinSpeed"),	TYPE_FLOAT, 0, 0,
		p_default, 04.0,
		p_range, 00.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_OBJ_FLOCKER_SLIMIT_MIN, IDC_OBJ_FLOCKER_SLIMIT_MIN_SPIN, 1.0,
		end,

	plObjectFlockerComponent::kUseTargetRotation,	_T("UseTargetRotation"),	TYPE_BOOL, 0, 0,
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_OBJ_FLOCKER_USE_TARGET_ROTATION,
		end,

	plObjectFlockerComponent::kRandomAnimStart,	_T("RandomAnimStart"),	TYPE_BOOL, 0, 0,
		p_default, TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_OBJ_FLOCKER_RANDOM_ANIM_START,
		end,

	plObjectFlockerComponent::kHideTarget,	_T("HideTarget"),	TYPE_BOOL, 0, 0,
		p_default, TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_OBJ_FLOCKER_HIDE_TARGET,
		end,

	end
);

plObjectFlockerComponent::plObjectFlockerComponent()
{
	fFlocker = nil;
	fClassDesc = &gObjectFlockerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plObjectFlockerComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetDrawable(!fCompPB->GetInt(ParamID(kHideTarget)));
	node->SetForceLocal(true);

	plMaxNode* targNode = (plMaxNode*)fCompPB->GetINode(kBoidObject);
	if (targNode)
		targNode->SetForceLocal(true);

	return true;
}

hsBool plObjectFlockerComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (fFlocker)
		delete fFlocker;

	fFlocker = TRACKED_NEW pfObjectFlocker;
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fFlocker, node->GetLocation(), node->GetLoadMask());

	fFlocker->SetGoalWeight(fCompPB->GetFloat(ParamID(kGoalStrength)));
	fFlocker->SetWanderWeight(fCompPB->GetFloat(ParamID(kWanderStrength)));

	fFlocker->SetSeparationWeight(fCompPB->GetFloat(ParamID(kSepStrength)));
	fFlocker->SetSeparationRadius(fCompPB->GetFloat(ParamID(kSepRadius)));

	fFlocker->SetCohesionWeight(fCompPB->GetFloat(ParamID(kCohStrength)));
	fFlocker->SetCohesionRadius(fCompPB->GetFloat(ParamID(kCohRadius)));

	fFlocker->SetMaxForce(fCompPB->GetFloat(ParamID(kMaxForce)));
	fFlocker->SetMaxSpeed(fCompPB->GetFloat(ParamID(kMaxSpeed)));
	fFlocker->SetMinSpeed(fCompPB->GetFloat(ParamID(kMinSpeed)));

	fFlocker->SetUseTargetRotation(fCompPB->GetInt(ParamID(kUseTargetRotation)) != 0);
	fFlocker->SetRandomizeAnimStart(fCompPB->GetInt(ParamID(kRandomAnimStart)) != 0);

	fFlocker->SetNumBoids(fCompPB->GetInt(ParamID(kNumBoids)));

	plKey boidKey = nil;
	plMaxNode* targNode = (plMaxNode*)fCompPB->GetINode(kBoidObject);

	if( targNode->CanConvert() )
	{
		plSceneObject* targObj = targNode->GetSceneObject();
		if( targObj )
		{
			boidKey = targObj->GetKey();
		}
	}
	fFlocker->SetBoidKey(boidKey);

	// Add a ref to the flocker.
	fFlocker->GetKey()->RefObject();

	return true;
}

hsBool plObjectFlockerComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->AddModifier(fFlocker, nil);

	return true;
}

hsBool plObjectFlockerComponent::DeInit(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( fFlocker )
		fFlocker->GetKey()->UnRefObject();
	fFlocker = nil;

	return true;
}

