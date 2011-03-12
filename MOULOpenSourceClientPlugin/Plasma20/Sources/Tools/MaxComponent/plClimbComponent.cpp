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
// max includes
#include "plPickNode.h"
#include "resource.h"

// local
#include "plClimbComponent.h"
#include "plPhysicalComponents.h"	// so we can pick terrains
#include "plComponentReg.h"

// other
#include "../MaxMain/plMaxNode.h"
#include "../plMessage/plClimbMsg.h"
#include "../plPhysical/plCollisionDetector.h"
#include "../MaxMain/plPhysicalProps.h"
#include "../plPhysical/plSimDefs.h"
#include "../pnSceneObject/plSceneObject.h"

// stl
#include <map>

/////////////////////////////////////////////////////////////////
//
// THE DUMMY
//
/////////////////////////////////////////////////////////////////
void DummyCodeIncludeFuncClimbTrigger() {}

/////////////////////////////////////////////////////////////////
//
// SOME ENUMS
//
/////////////////////////////////////////////////////////////////
// CLIMBCOMMANDS
enum Commands
{
	kMount				= 0,
	kEnableDismount		= 1,
	kDisableDismount	= 2,
	kEnableClimb		= 3,
	kDisableClimb		= 4,
	kFallOff			= 5,
	kRelease			= 6,
	kMaxCommands		= 7
};

// these synchronized ^^^^VVVVV
const char * fCommandStrs[] =
{
	"Start Climbing",
	"Enable Dismount",
	"Disable Dismount",
	"Enable Climb",
	"Disable Climb",
	"Fall Off",
	"Let Go"
};

enum Directions
{
	kUp		= 0,
	kDown	= 1,
	kLeft	= 2,
	kRight  = 3,
	kMaxDirections
};

const char * fDirectionStrs[] =
{
	"Up",
	"Down",
	"Left",
	"Right"
};


/////////////////////////////////////////////////////////////////
//
// CLASS DESCRIPTOR AND PARAM BLOCK
//
/////////////////////////////////////////////////////////////////

// CLASS DESCRIPTOR
CLASS_DESC(plClimbTriggerComponent, gClimbTriggerDesc, "Avatar ClimbTrigger",  "AvatarClimbTrigger", COMP_TYPE_AVATAR, CLIMB_TRIGGER_COMPONENT_CLASS_ID)

// FORWARD REFERENCE OT THE COMPONENT DIALOG PROC
static plClimbTriggerComponentProc gClimbTriggerComponentProc;

// PARAM BLOCK
ParamBlockDesc2 gClimbTriggerBk
(	

	plComponent::kBlkComp, _T("ClimbTrigger"), 0, &gClimbTriggerDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_CLIMB_TRIGGER, IDS_COMP_CLIMB_TRIGGER, 0, 0, &gClimbTriggerComponentProc,

	plClimbTriggerComponent::kCommand, _T("Command"),	TYPE_INT, 0, 0,
		p_default, kMount,
		end,

	plClimbTriggerComponent::kDirection, _T("Direction"),	TYPE_INT, 0, 0,
		p_default, kUp,
		end,

	plClimbTriggerComponent::kWallPicker, _T("WallPicker"),	TYPE_INODE,		0, 0,
		end,

	end
);

/////////////////////////////////////////////////////////////////
//
// PLCLIMBTRIGGER IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////

// CTOR
plClimbTriggerComponent::plClimbTriggerComponent()
{
	fClassDesc = &gClimbTriggerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

extern const plArmatureMod * FindArmatureMod(const plSceneObject *obj);

// CONVERT
hsBool plClimbTriggerComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plClimbMsg::Command enterCommand;		// when entering the region
	plClimbMsg::Command exitCommand;		// run this command when exiting the region
	hsBool enterStatus = false;
	hsBool exitStatus = false;
	plClimbMsg::Direction direction;		// direction is assumed the same for both enter and exit commands
											// i.e. enable up, disable up

	int iCommand = fCompPB->GetInt(plClimbTriggerComponent::kCommand);
	int iDirection = fCompPB->GetInt(plClimbTriggerComponent::kDirection);

	switch(iCommand)
	{
	case kMount:
		enterCommand = plClimbMsg::kStartClimbing;
		exitCommand = plClimbMsg::kNoCommand;
		break;
	case kEnableClimb:
		enterCommand = plClimbMsg::kEnableClimb;
		enterStatus = true;
		exitCommand = plClimbMsg::kEnableClimb;
		exitStatus = false;
		break;
	case kDisableClimb:
		enterCommand = plClimbMsg::kEnableClimb;
		enterStatus = false;
		exitCommand = plClimbMsg::kEnableClimb;
		exitStatus = true;
		break;
	case kEnableDismount:
		enterCommand = plClimbMsg::kEnableDismount;
		enterStatus = true;
		exitCommand = plClimbMsg::kEnableDismount;
		exitStatus = false;
		break;
	case kDisableDismount:
		enterCommand = plClimbMsg::kEnableDismount;
		enterStatus = false;
		exitCommand = plClimbMsg::kEnableDismount;
		exitStatus = true;
		break;
	case kFallOff:
		enterCommand = plClimbMsg::kFallOff;
		exitCommand = plClimbMsg::kNoCommand;
		break;
	case kRelease:
		enterCommand = plClimbMsg::kRelease;
		exitCommand = plClimbMsg::kNoCommand;
		break;
	}

	switch(iDirection)
	{
	case kUp:
		direction = plClimbMsg::kUp;
		break;
	case kDown:
		direction = plClimbMsg::kDown;
		break;
	case kLeft:
		direction = plClimbMsg::kLeft;
		break;
	case kRight:
		direction = plClimbMsg::kRight;
		break;
	}

	plKey nilKey = nil;
	plKey target = node->GetSceneObject()->GetKey();
	plClimbMsg *enterMsg = nil;
	if(enterCommand != plClimbMsg::kNoCommand)
	{
		enterMsg = TRACKED_NEW plClimbMsg(nilKey, nilKey, enterCommand, direction, enterStatus, target);
		enterMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		enterMsg->SetBCastFlag(plMessage::kNetPropagate);
		enterMsg->SetBCastFlag(plMessage::kNetForce);
	}

	plClimbMsg *exitMsg = nil;
	if(exitCommand != nil)
	{
		exitMsg = TRACKED_NEW plClimbMsg(nilKey, nilKey, exitCommand, direction, exitStatus, target);
		exitMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		exitMsg->SetBCastFlag(plMessage::kNetPropagate);
		exitMsg->SetBCastFlag(plMessage::kNetForce);
	}

	plSimpleRegionSensor *sensMod = TRACKED_NEW plSimpleRegionSensor(enterMsg, exitMsg);
	node->AddModifier(sensMod, IGetUniqueName(node));
	
	return true;
}

hsBool plClimbTriggerComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
 	node->SetForceLocal(true);
	node->SetDrawable(false);

	plPhysicalProps *props = node->GetPhysicalProps();

	// only if movable will it have mass (then it will keep track of movements in PhysX)
	if ( node->IsMovable() || node->IsTMAnimatedRecur() )
 		props->SetMass(1.0, node, pErrMsg);
 	props->SetFriction(0.0, node, pErrMsg);
 	props->SetRestitution(0.0, node, pErrMsg);
	props->SetBoundsType(plSimDefs::kExplicitBounds, node, pErrMsg);
	props->SetGroup(plSimDefs::kGroupDetector, node, pErrMsg);
	props->SetReportGroup(1<<plSimDefs::kGroupAvatar, node, pErrMsg);
//	props->SetPinned(true, node, pErrMsg);

	return true;
}


/////////////////////////////////////////////////////////////////
//
// DIALOG PROC IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////
BOOL plClimbTriggerComponentProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2 *pb = pm->GetParamBlock();
	HWND hCommandMenu = GetDlgItem(hWnd, IDC_COMP_CLIMB_COMMAND);
	HWND hDirectionMenu = GetDlgItem(hWnd, IDC_COMP_CLIMB_DIRECTION);
	HWND hPick = GetDlgItem(hWnd, IDC_COMP_WALL_PICK);
	INode *curPick = nil;
	int curSurface = 0;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			int i = 0;
			// fill out the command menu
			for (i = 0; i < kMaxCommands; i++)
				ComboBox_AddString(hCommandMenu, fCommandStrs[i]);
			// reflect the current selection
			ComboBox_SetCurSel(hCommandMenu, pb->GetInt(ParamID(plClimbTriggerComponent::kCommand)));

			// fill out the direction menu
			for (i = 0; i < kMaxDirections; i++)
				ComboBox_AddString(hDirectionMenu, fDirectionStrs[i]);
			// reflect the current selection
			ComboBox_SetCurSel(hDirectionMenu, pb->GetInt(ParamID(plClimbTriggerComponent::kDirection)));

			// show the name of the currently picked item
			curPick = pb->GetINode(ParamID(plClimbTriggerComponent::kWallPicker));
			Button_SetText(hPick, (curPick == nil ? "None" : curPick->GetName()));
		}
		return TRUE;
		break;


	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDC_COMP_WALL_PICK)
			{
				// we're picking a new climbing wall
				std::vector<Class_ID> pickableClasses;
				pickableClasses.push_back(PHYSICS_TERRAIN_CID);		// allow picking terrains
				pickableClasses.push_back(PHYS_CLIMBABLE_CID);		// and climbables
				if (plPick::NodeRefKludge(pb, plClimbTriggerComponent::kWallPicker, &pickableClasses, true, false))			
				{
					curPick = pb->GetINode(ParamID(plClimbTriggerComponent::kWallPicker));
					Button_SetText(hPick, (curPick == nil ? "None" : curPick->GetName()));
				}
			
				return TRUE;
			}
		}
		else if (LOWORD(wParam) == IDC_COMP_CLIMB_COMMAND)
		{
			HWND hSurface = GetDlgItem(hWnd, IDC_COMP_CLIMB_COMMAND);
			curSurface = ComboBox_GetCurSel(hSurface);
			pb->SetValue(ParamID(plClimbTriggerComponent::kCommand), 0, curSurface);
		}
		else if (LOWORD(wParam) == IDC_COMP_CLIMB_DIRECTION)
		{
			HWND hSurface = GetDlgItem(hWnd, IDC_COMP_CLIMB_DIRECTION);
			curSurface = ComboBox_GetCurSel(hSurface);
			pb->SetValue(ParamID(plClimbTriggerComponent::kDirection), 0, curSurface);
		}
	}

	return FALSE;
}




/////////////////////////////////////////////////////////////////////////////////////////
//
//	CLIMB BLOCKER COMPONENT
//
/////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plClimbBlockerComponent : public plComponent
{
public:
	plClimbBlockerComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plClimbBlockerComponent, gClimbBlockDesc, "Climb Blocker",  "ClimbBlocker", COMP_TYPE_AVATAR, Class_ID(0x170000ac, 0x3cee02c5))

ParamBlockDesc2 gClimbBlockBk
(
	plComponent::kBlkComp, _T("Climb Blocker"), 0, &gClimbBlockDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_CLIMB_BLOCK, IDS_COMP_CLIMB_BLOCKER, 0, 0, NULL,

	end
);

plClimbBlockerComponent::plClimbBlockerComponent()
{
	fClassDesc = &gClimbBlockDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plClimbBlockerComponent::SetupProperties(plMaxNode *node,  plErrorMsg *errMsg)
{
	node->SetDrawable(false);

	plPhysicalProps *props = node->GetPhysicalProps();

// 	props->SetMass(0.0, node, errMsg);
// 	props->SetFriction(0.0, node, errMsg);
// 	props->SetRestitution(0.0, node, errMsg);
	props->SetBoundsType(plSimDefs::kExplicitBounds, node, errMsg);
//	props->SetPinned(true, node, errMsg);
	props->SetLOSBlockCustom(true, node, errMsg);

	return true;
}

hsBool plClimbBlockerComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

