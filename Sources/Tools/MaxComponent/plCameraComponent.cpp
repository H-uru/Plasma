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
#include "plCameraComponent.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plKey.h"

#include "../plPhysical/plCollisionDetector.h"	// MM
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plCameraMsg.h"

#include "hsResMgr.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxConvert/plConvert.h"

#include "../MaxMain/plPhysicalProps.h"
#include "../plHavok1/plHKPhysicsGroups.h"

void DummyCodeIncludeFuncCamera() {}

OBSOLETE_CLASS_DESC(plCameraCmdComponent, gCameraCmdDesc, "(ex)Camera Command Region",  "CameraCmdRegion", COMP_TYPE_MISC, CAMERACMD_CID)

enum
{
	kCommand,
	kOffsetX,
	kOffsetY,
	kOffsetZ,
	kCustomBoundListStuff,
	kSmooth,
};

class plCameraCmdComponentProc;
extern plCameraCmdComponentProc gCameraCmdComponentProc;

enum
{
	kCommandSetOffset,
	kCommandSetFP,
	kCommandSetFixedCam,
};

ParamBlockDesc2 gCameraCmdBlock
(
	plComponent::kBlkComp, _T("cameraComp"), 0, &gCameraCmdDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_CAMERACMD, IDS_COMP_CAMERACMD, 0, 0, &gCameraCmdComponentProc,

	kCommand,		_T("Command"),		TYPE_INT,				0, 0,
		p_default, kCommandSetFixedCam,
		end,
		
	kOffsetX,	_T("X Offset"), TYPE_FLOAT,	P_ANIMATABLE,	0,
		p_range, 0.0f, 50.0f,
		p_default, 0.0f,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_CAMERACMD_OFFSETX, IDC_CAMERACMD_SPIN_OFFSETX, SPIN_AUTOSCALE,
		end,

	kOffsetY,	_T("Y Offset"), TYPE_FLOAT,	P_ANIMATABLE,	0,
		p_range, 0.0f, 50.0f,
		p_default, 10.0f,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_CAMERACMD_OFFSETY, IDC_CAMERACMD_SPIN_OFFSETY, SPIN_AUTOSCALE,
		end,

	kOffsetZ,	_T("Z Offset"), TYPE_FLOAT,	P_ANIMATABLE,	0,
		p_range, 0.0f, 50.0f,
		p_default, 3.0f,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_CAMERACMD_OFFSETZ, IDC_CAMERACMD_SPIN_OFFSETZ, SPIN_AUTOSCALE,
		end,

	kCustomBoundListStuff, _T("FixedCamera"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CAMERACMD_PICKSTATE_BASE,
		p_sclassID,	 CAMERA_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,

	kSmooth,	_T("useCut"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CAMERACMD_CUT,
		end,



	end
);

plCameraCmdComponent::plCameraCmdComponent()
{
	fClassDesc = &gCameraCmdDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plCameraCmdComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plCameraCmdComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plCameraCmdComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

class plCameraCmdComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}

	void DeleteThis() {}

protected:
	void IEnableControls(IParamMap2 *map, int type)
	{
	}

	void IAddComboItem(HWND hCombo, const char *name, int id)
	{
	}
	void ISetComboSel(HWND hCombo, int type)
	{
	}
};
static plCameraCmdComponentProc gCameraCmdComponentProc;
