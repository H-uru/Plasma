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
#include <windowsx.h>

#include "max.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../pnSceneObject/plSceneObject.h"

#include "../pfCamera/plInterestingModifier.h"
#include "../plModifier/plSpawnModifier.h"
#include "plgDispatch.h"



#include "hsResMgr.h"

#include "../plScene/plSceneNode.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxMain/plMaxNode.h"
#include "hsGeometry3.h"
#include "../plPhysical/plSimDefs.h"

#include "../pnSceneObject/plCoordinateInterface.h"

//Necessary Empty function.  Otherwise Linker throws the Paramblock away as extraneous.
void DummyCodeIncludeFuncTypes()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	StartPoint Component
//
//

//Class that accesses the paramblock below.
class plStartingPointComponent : public plComponent
{
public:
	plStartingPointComponent();
	void DeleteThis() { delete this; }
	hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	//hsBool IsValidNodeType(plMaxNode *pNode);
};

//Max desc stuff necessary.
CLASS_DESC(plStartingPointComponent, gStartPtDesc, "Starting Point",  "StartPoint", COMP_TYPE_TYPE, Class_ID(0x2a127b68, 0xdc7367a))

//The MAX paramblock stuff below
ParamBlockDesc2 gStartPtBk
(
	1, _T(""), 0, &gStartPtDesc, P_AUTO_CONSTRUCT, 0,

	end
);

plStartingPointComponent::plStartingPointComponent()
{
	fClassDesc = &gStartPtDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plStartingPointComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	return true;
}
hsBool plStartingPointComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plSpawnModifier* pSpawn = TRACKED_NEW plSpawnModifier;
	node->AddModifier(pSpawn, IGetUniqueName(node));
	return true;
}

hsBool plStartingPointComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "plPickNode.h"

class plVehicleModifier;

class plVehicleComponent : public plComponent
{
protected:
	plVehicleModifier* fMod;

	bool IIsValid();

public:
	plVehicleComponent();

	hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

CLASS_DESC(plVehicleComponent, gVehicleDesc, "(ex)Vehicle", "Vehicle", COMP_TYPE_MISC, Class_ID(0x75903e2, 0x50ac210b))

enum
{
	kVehicleChassis,
	kVehicleWheelFR,
	kVehicleWheelFL,
	kVehicleWheelRR,
	kVehicleWheelRL,
	kVehicleHardpointFR,
	kVehicleHardpointFL,
	kVehicleHardpointRR,
	kVehicleHardpointRL,
	kVehicleDriveDet,
};

class plVehicleComponentProc : public ParamMap2UserDlgProc
{
protected:
	void IUpdateButtonText(HWND hWnd, IParamBlock2 *pb)
	{
		INode *node = pb->GetINode(kVehicleDriveDet);
		SetWindowText(GetDlgItem(hWnd, IDC_DRIVE), node ? node->GetName() : "(none)");
	}

public:
	BOOL DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			IUpdateButtonText(hWnd, pm->GetParamBlock());
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (LOWORD(wParam) == IDC_DRIVE)
				{
					// Adding an activator.  Set it and refresh the UI to show it in our list.
					plPick::Activator(pm->GetParamBlock(), kVehicleDriveDet, true);
					IUpdateButtonText(hWnd, pm->GetParamBlock());
					return TRUE;
				}
			}
			break;
		}

		return FALSE;
	}
	void DeleteThis() {}
};
static plVehicleComponentProc gVehicleComponentProc;

ParamBlockDesc2 gVehicleBlock
(
	plComponent::kBlkComp, _T("vehicleComp"), 0, &gVehicleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_VEHICLE, IDS_COMP_VEHICLE, 0, 0, &gVehicleComponentProc,

	kVehicleChassis,	_T("chassis"),		TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_CHASSIS,
		end,

	kVehicleWheelFR,	_T("wheelFR"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_FR_WHEEL,
		end,
	kVehicleWheelFL,	_T("wheelFL"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_FL_WHEEL,
		end,
	kVehicleWheelRR,	_T("wheelRR"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_RR_WHEEL,
		end,
	kVehicleWheelRL,	_T("wheelRL"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_RL_WHEEL,
		end,

	kVehicleHardpointFR,	_T("hardpointFR"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_FR_HARDPOINT,
		end,
	kVehicleHardpointFL,	_T("hardpointFL"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_FL_HARDPOINT,
		end,
	kVehicleHardpointRR,	_T("hardpointRR"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_RR_HARDPOINT,
		end,
	kVehicleHardpointRL,	_T("hardpointRL"),	TYPE_INODE,		0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_RL_HARDPOINT,
		end,

	kVehicleDriveDet,	_T("driveDet"),	TYPE_INODE,		0, 0,
		end,

	end
);

plVehicleComponent::plVehicleComponent() : fMod(nil)
{
	fClassDesc = &gVehicleDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

#include "../MaxMain/plPhysicalProps.h"
#include "../pnSceneObject/plSimulationInterface.h"
//#include "../plHavok1/plVehicleModifier.h"
//#include "../plHavok1/plPhysicsGroups.h"
/*
void SetupVehiclePhys(plMaxNode* physNode, plMaxNode* node, plErrorMsg* pErrMsg, bool chassis=false)
{
	plPhysicalProps* physProps = physNode->GetPhysicalProps();
	physProps->SetMass(1.0, node, pErrMsg);
	physProps->SetRestitution(0.0, node, pErrMsg);
	physProps->SetFriction(0.5, node, pErrMsg);
	physProps->SetBoundsType(plSimDefs::kHullBounds, node, pErrMsg);

	if (chassis)
	{
		physProps->SetMemberGroup(plPhysicsGroups::kDynamicSimulated, node, pErrMsg);
		physProps->SetBounceGroup(	plPhysicsGroups::kStaticSimulated |
									plPhysicsGroups::kDynamicSimulated |
									plPhysicsGroups::kAnimated,
									node, pErrMsg);
		physProps->SetPinned(true, node, pErrMsg);
	}

	physNode->SetMovable(true);
	physNode->SetForceLocal(true);
}
*/
bool plVehicleComponent::IIsValid()
{
	return
		(
		fCompPB->GetINode(kVehicleChassis) &&
		fCompPB->GetINode(kVehicleWheelFR) &&
		fCompPB->GetINode(kVehicleWheelFL) &&
		fCompPB->GetINode(kVehicleWheelRR) &&
		fCompPB->GetINode(kVehicleWheelRL) &&
		fCompPB->GetINode(kVehicleHardpointFR) &&
		fCompPB->GetINode(kVehicleHardpointFL) &&
		fCompPB->GetINode(kVehicleHardpointRR) &&
		fCompPB->GetINode(kVehicleHardpointRL) &&
		fCompPB->GetINode(kVehicleDriveDet)
		);
}

hsBool plVehicleComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return false;
#if 0
	if (!IIsValid())
		return false;

	plMaxNode* chassis = (plMaxNode*)fCompPB->GetINode(kVehicleChassis);
	plMaxNode* wheelFR = (plMaxNode*)fCompPB->GetINode(kVehicleWheelFR);
	plMaxNode* wheelFL = (plMaxNode*)fCompPB->GetINode(kVehicleWheelFL);
	plMaxNode* wheelRR = (plMaxNode*)fCompPB->GetINode(kVehicleWheelRR);
	plMaxNode* wheelRL = (plMaxNode*)fCompPB->GetINode(kVehicleWheelRL);

	plMaxNode* hardpointFR = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointFR);
	plMaxNode* hardpointFL = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointFL);
	plMaxNode* hardpointRR = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointRR);
	plMaxNode* hardpointRL = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointRL);

	chassis->SetDrawable(false);

	hardpointFR->SetForceLocal(true);
	hardpointFL->SetForceLocal(true);
	hardpointRR->SetForceLocal(true);
	hardpointRL->SetForceLocal(true);

	SetupVehiclePhys(chassis, node, pErrMsg, true);
	SetupVehiclePhys(wheelFR, node, pErrMsg);
	SetupVehiclePhys(wheelFL, node, pErrMsg);
	SetupVehiclePhys(wheelRR, node, pErrMsg);
	SetupVehiclePhys(wheelRL, node, pErrMsg);

	return true;
#endif
}

hsBool plVehicleComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return false;
#if 0
	if (!IIsValid())
		return false;

	fMod = TRACKED_NEW plVehicleModifier;
	plKey modKey = pNode->AddModifier(fMod, IGetUniqueName(pNode));

	plMaxNode* detectorNode = (plMaxNode*)fCompPB->GetINode(kVehicleDriveDet);
	plComponentBase* comp = detectorNode ? detectorNode->ConvertToComponent() : nil;
	if (comp)
		comp->AddReceiverKey(modKey);

	return true;
#endif
}

#if 0

void GetSuspensionProps(plMaxNode* hardPoint, plMaxNode* wheel, hsPoint3& chassisPoint,
						plVehicleModifier::WheelProps& props)
{
	props.wheelKey = wheel->GetKey();

	hsPoint3 hardPos = hardPoint->GetLocalToWorld44().GetTranslate();
	hsPoint3 wheelPos = wheel->GetLocalToWorld44().GetTranslate();

	// Get position of the hardpoint relative to the chassis
	props.pos = hardPos - chassisPoint;

	// Get a vector from the hardpoint to the wheel
	hsVector3 dir(wheelPos - hardPos);

	// Get the length of the suspension (hardpoint to wheel)
	props.len = hsPoint3(dir).Magnitude();

	// Get the direction of the suspension
	dir.Normalize();
	props.dir = dir;
}

#endif

hsBool plVehicleComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return false;
#if 0
	if (!IIsValid())
		return false;

	plMaxNode* chassis = (plMaxNode*)fCompPB->GetINode(kVehicleChassis);
	plMaxNode* wheelFR = (plMaxNode*)fCompPB->GetINode(kVehicleWheelFR);
	plMaxNode* wheelFL = (plMaxNode*)fCompPB->GetINode(kVehicleWheelFL);
	plMaxNode* wheelRR = (plMaxNode*)fCompPB->GetINode(kVehicleWheelRR);
	plMaxNode* wheelRL = (plMaxNode*)fCompPB->GetINode(kVehicleWheelRL);

	plMaxNode* hardpointFR = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointFR);
	plMaxNode* hardpointFL = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointFL);
	plMaxNode* hardpointRR = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointRR);
	plMaxNode* hardpointRL = (plMaxNode*)fCompPB->GetINode(kVehicleHardpointRL);

	hsPoint3 chassisPos = chassis->GetLocalToWorld44().GetTranslate();

	plVehicleModifier::WheelProps wheelPropsFR, wheelPropsFL, wheelPropsRR, wheelPropsRL;

	GetSuspensionProps(hardpointFR, wheelFR, chassisPos, wheelPropsFR);
	GetSuspensionProps(hardpointFL, wheelFL, chassisPos, wheelPropsFL);
	GetSuspensionProps(hardpointRR, wheelRR, chassisPos, wheelPropsRR);
	GetSuspensionProps(hardpointRL, wheelRL, chassisPos, wheelPropsRL);

	fMod->Setup(chassis->GetKey(),
				wheelPropsFR,
				wheelPropsFL,
				wheelPropsRR,
				wheelPropsRL);

	return true;
#endif
}	
/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	 Maintainers Marker Component
//
//

#include "../plModifier/plMaintainersMarkerModifier.h"

enum
{
	kCalibrated,
};
//Class that accesses the paramblock below.
class plMaintainersMarkerComponent : public plComponent
{
public:
	plMaintainersMarkerComponent();
	void DeleteThis() { delete this; }
	hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary.
CLASS_DESC(plMaintainersMarkerComponent, gMaintainersDesc, "Maintainers Marker",  "MaintainersMarker", COMP_TYPE_TYPE, Class_ID(0x7d7f1f72, 0x405355f5))

//The MAX paramblock stuff below
ParamBlockDesc2 gMaintainersBk
(
	
	1, _T("maintainersMarker"), 0, &gMaintainersDesc, P_AUTO_CONSTRUCT  + P_AUTO_UI, plComponent::kRefComp,
	IDD_COMP_MAINTAINERS_MARKER, IDS_COMP_MAINTAINERS_MARKER, 0, 0, NULL,

	kCalibrated, _T("Calibrated"),		TYPE_INT, 		0, 0,
		p_ui, TYPE_RADIO, 3,	IDC_RADIO_BROKEN, IDC_RADIO_REPAIRED, IDC_RADIO_CALIBRATED,
		p_vals,	plMaintainersMarkerModifier::kBroken, plMaintainersMarkerModifier::kRepaired, plMaintainersMarkerModifier::kCalibrated,
		end,
	end
);

plMaintainersMarkerComponent::plMaintainersMarkerComponent()
{
	fClassDesc = &gMaintainersDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plMaintainersMarkerComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	return true;
}
hsBool plMaintainersMarkerComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plMaintainersMarkerModifier* pSpawn = TRACKED_NEW plMaintainersMarkerModifier;
	pSpawn->SetCalibrated(fCompPB->GetInt(kCalibrated));
	node->AddModifier(pSpawn, IGetUniqueName(node));
	return true;
}

hsBool plMaintainersMarkerComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	 Game Marker Component
//

#include "../plModifier/plGameMarkerModifier.h"
#include "plNotetrackAnim.h"
#include "plPickMaterialMap.h"
#include "../MaxMain/plMtlCollector.h"
#include "plResponderMtl.h"
#include "plResponderGetComp.h"
#include "plAnimComponent.h"
#include "plAudioComponents.h"

class plGameMarkerComponent : public plComponent
{
protected:
	plKey IGetMtlAnimKey(int paramID, plMaxNode* node);
	plKey IGetAnimKey(int nodeID, int compID);

public:
	plGameMarkerComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

CLASS_DESC(plGameMarkerComponent, gGameMarkerDesc, "Game Marker",  "GameMarker", COMP_TYPE_TYPE, Class_ID(0x4a15029a, 0x350f7258))

enum
{
	kMarkerPhys,
	kMarkerMtl,
	kMarkerGreenAnim,
	kMarkerRedAnim,
	kMarkerOpenAnim,
	kMarkerEditAnim_DEAD,
	kMarkerBounceNode,
	kMarkerBounceComp,
	kMarkerMtlNode,
	kMarkerSndPlace,
	kMarkerSndHit,
};

class plGameMarkerComponentProc : public ParamMap2UserDlgProc
{
protected:
	void IComboChanged(HWND hWnd, IParamBlock2 *pb, int id)
	{
		char buf[256];
		GetDlgItemText(hWnd, id, buf, sizeof(buf));
		int paramID = 0;
		switch (id)
		{
		case IDC_ANIM_RED_COMBO:	paramID = kMarkerRedAnim;	break;
		case IDC_ANIM_GREEN_COMBO:	paramID = kMarkerGreenAnim;	break;
		case IDC_ANIM_OPEN_COMBO:	paramID = kMarkerOpenAnim;	break;
		}

		pb->SetValue(paramID, 0, buf);
	}

	void ILoadCombo(HWND hWnd, int ctrlID, int paramID, IParamBlock2* pb, plNotetrackAnim& anim)
	{
		const char* savedName = pb->GetStr(paramID);
		HWND hCombo = GetDlgItem(hWnd, ctrlID);
		ComboBox_ResetContent(hCombo);

		while (const char* animName = anim.GetNextAnimName())
		{
			int sel = ComboBox_AddString(hCombo, animName);
			if (hsStrEQ(animName, savedName))
				ComboBox_SetCurSel(hCombo, sel);
		}
	}

	void IInit(HWND hWnd, IParamBlock2* pb)
	{
		Mtl* mtl = pb->GetMtl(kMarkerMtl);

		if (mtl)
		{
			SetDlgItemText(hWnd, IDC_MTL_BUTTON, mtl->GetName());

			plNotetrackAnim anim(mtl, nil);
			ILoadCombo(hWnd, IDC_ANIM_RED_COMBO, kMarkerRedAnim, pb, anim);
			ILoadCombo(hWnd, IDC_ANIM_GREEN_COMBO, kMarkerGreenAnim, pb, anim);
			ILoadCombo(hWnd, IDC_ANIM_OPEN_COMBO, kMarkerOpenAnim, pb, anim);
		}

		if (pb->GetINode(kMarkerMtlNode))
			SetDlgItemText(hWnd, IDC_MTL_NODE_BUTTON, pb->GetINode(kMarkerMtlNode)->GetName());
		else
			SetDlgItemText(hWnd, IDC_MTL_NODE_BUTTON, "(none)");

		if (pb->GetINode(kMarkerBounceNode))
			SetDlgItemText(hWnd, IDC_BOUNCE_BUTTON, pb->GetINode(kMarkerBounceNode)->GetName());
		else
			SetDlgItemText(hWnd, IDC_BOUNCE_BUTTON, "(none)");

		if (pb->GetINode(kMarkerSndPlace))
			SetDlgItemText(hWnd, IDC_PLACE_BUTTON, pb->GetINode(kMarkerSndPlace)->GetName());
		else
			SetDlgItemText(hWnd, IDC_PLACE_BUTTON, "(none)");

		if (pb->GetINode(kMarkerSndHit))
			SetDlgItemText(hWnd, IDC_HIT_BUTTON, pb->GetINode(kMarkerSndHit)->GetName());
		else
			SetDlgItemText(hWnd, IDC_HIT_BUTTON, "(none)");
	}

public:
	BOOL DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			IInit(hWnd, pm->GetParamBlock());
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_MTL_BUTTON)
			{
				Mtl* pickedMtl = plPickMaterialMap::PickMaterial(plMtlCollector::kUsedOnly |
																plMtlCollector::kPlasmaOnly);
				if (pickedMtl)
				{
					pm->GetParamBlock()->SetValue(kMarkerMtl, 0, pickedMtl);
					IInit(hWnd, pm->GetParamBlock());
				}

				return TRUE;
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_MTL_NODE_BUTTON)
			{
				if (plPick::MtlNodes(pm->GetParamBlock(), kMarkerMtlNode, pm->GetParamBlock()->GetMtl(kMarkerMtl)))
					IInit(hWnd, pm->GetParamBlock());
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_BOUNCE_BUTTON)
			{
				plResponderGetComp::ClassIDs ids;
				ids.push_back(ANIM_COMP_CID);
				if (plResponderGetComp::Instance().GetComp(pm->GetParamBlock(), kMarkerBounceNode, kMarkerBounceComp, &ids))
					IInit(hWnd, pm->GetParamBlock());

				return TRUE;
			}
			else if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDC_PLACE_BUTTON || LOWORD(wParam) == IDC_HIT_BUTTON))
			{
				std::vector<Class_ID> cids;
				cids.push_back(SOUND_3D_COMPONENT_ID);

				int paramID = (LOWORD(wParam) == IDC_PLACE_BUTTON) ? kMarkerSndPlace : kMarkerSndHit;

				if (plPick::Node(pm->GetParamBlock(), paramID, &cids, true, false))
					IInit(hWnd, pm->GetParamBlock());
				return TRUE;
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				IComboChanged(hWnd, pm->GetParamBlock(), LOWORD(wParam));
				return TRUE;
			}
			break;
		}

		return FALSE;
	}
	void DeleteThis() {}
};
static plGameMarkerComponentProc gGameMarkerComponentProc;

ParamBlockDesc2 gGameMarkerBlk
(
	plComponent::kBlkComp, _T("gameMarker"), 0, &gGameMarkerDesc, P_AUTO_CONSTRUCT  + P_AUTO_UI, plComponent::kRefComp,
	IDD_COMP_MARKER, IDS_COMP_MARKER, 0, 0, &gGameMarkerComponentProc,

	kMarkerPhys,	_T("physical"),		TYPE_INODE,		0, 0,
		p_ui,		TYPE_PICKNODEBUTTON, IDC_MARKER_PHYS,
		end,

	kMarkerMtl,		_T("mtl"),			TYPE_MTL,		0, 0,
		end,

	kMarkerGreenAnim,	_T("green"),	TYPE_STRING,	0, 0,
		end,
	kMarkerRedAnim,		_T("red"),		TYPE_STRING,	0, 0,
		end,
	kMarkerOpenAnim,	_T("open"),		TYPE_STRING,	0, 0,
		end,

	kMarkerBounceNode,	_T("bounceNode"), TYPE_INODE, 0, 0,
		end,
	kMarkerBounceComp,	_T("bounceComp"), TYPE_INODE, 0, 0,
		end,

	kMarkerMtlNode,		_T("mtlNode"),	TYPE_INODE,		0, 0,
		end,

	kMarkerSndPlace,	_T("sndPlace"),	TYPE_INODE,		0, 0,
		end,

	kMarkerSndHit,		_T("sndHit"),	TYPE_INODE,		0, 0,
		end,

	end
);

plGameMarkerComponent::plGameMarkerComponent()
{
	fClassDesc = &gGameMarkerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plGameMarkerComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plMaxNode* proxy = (plMaxNode*)fCompPB->GetINode(kMarkerPhys);
	proxy->SetCanConvert(false);
	node->SetForceLocal(true);
	node->SetItinerant(true);

	plPhysicalProps* physProps = node->GetPhysicalProps();
	physProps->SetBoundsType(plSimDefs::kSphereBounds, node, pErrMsg);
	physProps->SetPinned(true, node, pErrMsg);
	// only if movable will it have mass (then it will keep track of movements in PhysX)
	if ( node->IsMovable() || node->IsTMAnimated() )
		physProps->SetMass(1.0, node, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupDetector, node, pErrMsg);
	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, node, pErrMsg);
	physProps->SetProxyNode(proxy, node, pErrMsg);

	return true;
}

hsBool plGameMarkerComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

plKey plGameMarkerComponent::IGetMtlAnimKey(int paramID, plMaxNode* node)
{
	Mtl* mtl = fCompPB->GetMtl(kMarkerMtl);
	plMaxNode* mtlNode  = (plMaxNode*)fCompPB->GetINode(kMarkerMtlNode);
	hsTArray<plKey> keys;
	const char* anim = fCompPB->GetStr(paramID);
	GetMatAnimModKey(mtl, mtlNode, anim, keys);
	hsAssert(keys.Count() == 1, "Wrong number of keys");
	return keys[0];
}

plKey plGameMarkerComponent::IGetAnimKey(int nodeID, int compID)
{
	plMaxNode* animComp = (plMaxNode*)fCompPB->GetINode(compID);
	plMaxNode* animNode = (plMaxNode*)fCompPB->GetINode(nodeID);
	if (animComp && animNode)
	{
		plAnimComponent* comp = (plAnimComponent*)animComp->ConvertToComponent();
		return comp->GetModKey(animNode);
	}

	return nil;
}

hsBool plGameMarkerComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plGameMarkerModifier* markerMod = TRACKED_NEW plGameMarkerModifier;

	plKey greenKey	= IGetMtlAnimKey(kMarkerGreenAnim, node);
	plKey redKey	= IGetMtlAnimKey(kMarkerRedAnim, node);
	plKey openKey	= IGetMtlAnimKey(kMarkerOpenAnim, node);

	plKey bounceKey = IGetAnimKey(kMarkerBounceNode, kMarkerBounceComp);

	plMaxNode* sndPlaceComp = (plMaxNode*)fCompPB->GetINode(kMarkerSndPlace);
	int sndPlaceIdx = plAudioComp::GetSoundModIdx(sndPlaceComp->ConvertToComponent(), node);

	plMaxNode* sndHitComp = (plMaxNode*)fCompPB->GetINode(kMarkerSndHit);
	int sndHitIdx = plAudioComp::GetSoundModIdx(sndHitComp->ConvertToComponent(), node);

	markerMod->ExportInit(greenKey, redKey, openKey, bounceKey, sndPlaceIdx, sndHitIdx);

	node->AddModifier(markerMod, IGetUniqueName(node));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Camera Component
//
//


//Class that accesses the paramblock below.

class plCameraComponent : public plComponent
{
public:
	plCameraComponent();
	void DeleteThis() { delete this; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool IsValidNodeType(plMaxNode *pNode);

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

};

//Max desc stuff necessary.
OBSOLETE_CLASS_DESC(plCameraComponent, gCameraDesc, "Camera",  "Camera", COMP_TYPE_TYPE, Class_ID(0x75926577, 0x1cbe49b6))

//
// Block not necessary, kept for backwards compat.
//
enum
{
	kCamera,
	kCameraV2
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gCameraBk
(	
	1, _T("camera"), 0, &gCameraDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_CAMERA, IDS_COMP_CAMERAS,  0, 0, NULL,

	// params
	kCamera,	_T("Animation"),		TYPE_INT, 		0, 0,
		p_default, 0,
		p_ui,		TYPE_RADIO, 2, /*IDC_RADIO_DEFAULT,*/ IDC_RADIO_FIXEDCAM, IDC_RADIO_FIXEDPANCAM,
		end,
	//kCamera,	_T("CamType"),		TYPE_INT, 		0, 0,
	///	p_ui,		TYPE_RADIO, 2, IDC_RADIO_FIXEDCAM, IDC_RADIO_FIXEDPANCAM,
	//	end,



	end
);

plCameraComponent::plCameraComponent()
{
	fClassDesc = &gCameraDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plCameraComponent::SetupProperties(plMaxNode* pNode, plErrorMsg *pErrMsg)
{
	return true;
}


hsBool plCameraComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plCameraComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}


hsBool plCameraComponent::IsValidNodeType(plMaxNode *pNode)
{
		return false;
}





