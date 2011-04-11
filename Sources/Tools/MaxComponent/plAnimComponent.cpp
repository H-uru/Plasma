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

#include "resource.h"
#include "hsUtils.h"
#include "plAnimComponent.h"
#include "plComponentProcBase.h"
#include "plPhysicalComponents.h"
#include "plMiscComponents.h"
#include "../MaxMain/plPhysicalProps.h"

#include "../pnSceneObject/plSceneObject.h"

#include "../plInterp/plController.h"
#include "plNotetrackAnim.h"
#include "hsResMgr.h"
#include "../plAvatar/plAGModifier.h"
#include "../plAvatar/plAGChannel.h"
#include "../plAvatar/plAGAnim.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plAvatar/plMatrixChannel.h"
#include "../plAvatar/plPointChannel.h"
#include "../plAvatar/plScalarChannel.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlBase.h"

#include "../pnKeyedObject/plUoid.h"
#include "plMaxAnimUtils.h"

#include "../MaxPlasmaLights/plRealTimeLightBase.h"
#include "../pfAnimation/plLightModifier.h"
#include "../pnKeyedObject/plMsgForwarder.h"

#include "../plSDL/plSDL.h"
#include "../plSDL/plSDLDescriptor.h"

#include "plPickNodeBase.h"


// For material animations
#include "../MaxPlasmaMtls/Materials/plAnimStealthNode.h"

// So that the linker won't throw this code away, since it doesn't appear to be used
void DummyCodeIncludeFunc() {}

bool HasPhysicalComponent(plMaxNodeBase *node, bool searchChildren)
{
	int i;
	for (i = 0; i < node->NumAttachedComponents(); i++)
	{
		if (node->GetAttachedComponent(i)->CanConvertToType(PHYSICS_BASE_CID))
			return true;
	}

	if (searchChildren)
	{
		for (i = 0; i < node->NumberOfChildren(); i++)
			if (HasPhysicalComponent((plMaxNodeBase *)node->GetChildNode(i), searchChildren))
				return true;
	}
	return false;
}

bool HasPhysicalComponent(plComponentBase *comp)
{
	int i;
	for (i = 0; i < comp->NumTargets(); i++)
	{
		plMaxNodeBase *node = comp->GetTarget(i);
		if (node && HasPhysicalComponent(node, true))
			return true;
	}
	return false;
}

bool	plAnimComponentBase::GetAnimKey( plMaxNode *node, hsTArray<plKey> &outKeys )
{
	plComponentBase *comp = node->ConvertToComponent();
	if( comp != nil )
	{
		if( IsAnimComponent( comp ) )
		{
			plAnimComponentBase *base = (plAnimComponentBase *)comp;
			// Grab this guy's key
		}
	}
//	else if( )
	{
	}
	return true;
}

plAnimObjInterface	*plAnimComponentBase::GetAnimInterface( INode *inode )
{
	if( inode == nil )
		return nil;

	plMaxNode *node = (plMaxNode *)inode;
	plComponentBase *comp = node->ConvertToComponent();
	if( comp != nil )
	{
		if( IsAnimComponent( comp ) )
		{
			plAnimComponentBase *base = (plAnimComponentBase *)comp;
			return (plAnimObjInterface *)base;
		}
	}
	else
	{
		plAnimStealthNode *stealth = plAnimStealthNode::ConvertToStealth( node );
		if( stealth != nil )
			return (plAnimObjInterface *)stealth;
	}

	return nil;
}

//This enum is necessary and can only be appended.
//This is used in the ParamBlock2Desc.
enum
{
	kAnimRadio_DEAD,
		kAnimAutoStart,			// Start the Animation on load  (V2)
		kAnimLoop,				// Start Looping at Begin Location
		kAnimBegin_DEAD,
		kAnimEnd_DEAD,
		kAnimLoopSegCkBx_DEAD,
		kAnimLoopSegBeg_DEAD,
		kAnimLoopSegEnd_DEAD,
		kAnimName,				// Name of the notetrack animation to play
		kAnimLoopSegBegBox_DEAD,
		kAnimLoopSegEndBox_DEAD,
		kAnimUseGlobal,
		kAnimGlobalName,
		kAnimLoopName,			// Name of the notetrack specified loop
		kAnimEaseInType,
		kAnimEaseOutType,
		kAnimEaseInLength,
		kAnimEaseOutLength,
		kAnimEaseInMin,
		kAnimEaseInMax,
		kAnimEaseOutMin,
		kAnimEaseOutMax,
		kAnimPhysAnim,
};

void plAnimComponentProc::EnableGlobal(HWND hWnd, hsBool enable)
{
	ComboBox_Enable(GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST), enable);
	ComboBox_Enable(GetDlgItem(hWnd, IDC_ANIM_NAMES), !enable);
	ComboBox_Enable(GetDlgItem(hWnd, IDC_LOOP_NAMES), !enable);
	Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_AUTOSTART_CKBX), !enable);
	Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_LOOP_CKBX), !enable);
}	

void plAnimComponentProc::FillAgeGlobalComboBox(HWND box, char *varName)
{		
	plStateDescriptor *sd = plSDLMgr::GetInstance()->FindDescriptor(plPageInfoComponent::GetCurrExportAgeName(), plSDL::kLatestVersion);
	if (sd)
	{
		int i;
		for (i = 0; i < sd->GetNumVars(); i++)
		{
			plVarDescriptor *var = sd->GetVar(i);
			if (var->GetType() == plVarDescriptor::kFloat ||
				var->GetType() == plVarDescriptor::kDouble ||
				var->GetType() == plVarDescriptor::kTime ||
				var->GetType() == plVarDescriptor::kAgeTimeOfDay)
			{
				ComboBox_AddString(box, var->GetName());
			}
		}
	}
	ComboBox_AddString(box, "(none)");
}

void plAnimComponentProc::SetBoxToAgeGlobal(HWND box, char *varName)
{
	char buff[512];
	if (!varName || !strcmp(varName, ""))
		varName = "(none)";

	ComboBox_SelectString(box, 0, varName);
	ComboBox_GetLBText(box, ComboBox_GetCurSel(box), buff);
	if (strcmp(varName, buff))
	{
		// Didn't find our variable in the age SDL file... 
		// Probably just missing the sdl file,
		// so we'll force it in there. It'll export fine.
		ComboBox_AddString(box, varName);
		ComboBox_SelectString(box, 0, varName);
	}
}	

BOOL plAnimComponentProc::DlgProc(TimeValue t, IParamMap2 *pMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND gWnd = GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST);
	char buff[512];
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			fPB = pMap->GetParamBlock();
			
			fNoteTrackDlg.Init(GetDlgItem(hWnd, IDC_ANIM_NAMES),
				GetDlgItem(hWnd, IDC_LOOP_NAMES),
				kAnimName,
				kAnimLoopName,
				fPB,
				fPB->GetOwner());
			fNoteTrackDlg.Load();
			
			EnableWindow(GetDlgItem(hWnd, IDC_LOOP_NAMES), fPB->GetInt(kAnimLoop));
			
			FillAgeGlobalComboBox(gWnd, fPB->GetStr(ParamID(kAnimGlobalName)));
			SetBoxToAgeGlobal(gWnd, fPB->GetStr(ParamID(kAnimGlobalName)));
			EnableGlobal(hWnd, fPB->GetInt(ParamID(kAnimUseGlobal)));
			Button_Enable(GetDlgItem(hWnd, IDC_COMP_ANIM_PHYSANIM), 
				HasPhysicalComponent((plComponentBase*)fPB->GetOwner()));
		}
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_ANIM_NAMES)
		{
			fNoteTrackDlg.AnimChanged();
			return TRUE;
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LOOP_NAMES)
		{
			// Get the new loop name
			fNoteTrackDlg.LoopChanged();
			return TRUE;
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_ANIM_GLOBAL_LIST)
		{
			ComboBox_GetLBText(gWnd, ComboBox_GetCurSel(gWnd), buff);
			fPB->SetValue(ParamID(kAnimGlobalName), 0, _T(buff));
		}
		// Catch loop button updates
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ANIM_LOOP_CKBX)
			EnableWindow(GetDlgItem(hWnd, IDC_LOOP_NAMES), fPB->GetInt(kAnimLoop));
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ANIM_USE_GLOBAL)
		{
			EnableGlobal(hWnd, fPB->GetInt(ParamID(kAnimUseGlobal)));
		}
		break;
	}
	return false;	
}

void plAnimComponentProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
	HWND hWnd = pmap->GetHWnd();
	IParamBlock2 *pb = pmap->GetParamBlock();
	SetBoxToAgeGlobal(GetDlgItem(hWnd, IDC_ANIM_GLOBAL_LIST), pb->GetStr(ParamID(kAnimGlobalName))); 	
}	

void plAnimComponentProc::DeleteThis()
{
	fNoteTrackDlg.DeleteCache();
}	

//  For the paramblock below.
static plAnimComponentProc gAnimCompProc;

#define WM_ROLLOUT_OPEN WM_USER+1

class plAnimEaseComponentProc : public ParamMap2UserDlgProc
{
protected:
	void EnableStopPoints(IParamMap2 *pm, bool enable)
	{
		pm->Enable(kAnimEaseInMin, enable);
		pm->Enable(kAnimEaseInMax, enable);
		pm->Enable(kAnimEaseOutMin, enable);
		pm->Enable(kAnimEaseOutMax, enable);
	}

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
				IParamBlock2 *pb = map->GetParamBlock();

				// Enable the min and max controls (that are only valid with stop points)
				// if at least one of the targets has a stop point
				plAnimComponent *comp = (plAnimComponent*)pb->GetOwner();
				int num = comp->NumTargets();
				bool stopPoints = false;
				for (int i = 0; i < num; i++)
				{
					if (DoesHaveStopPoints(comp->GetTarget(i)))
					{
						stopPoints = true;
						break;
					}
				}
				EnableStopPoints(map, stopPoints);

				// If we're doing an ease, set the ease rollup to open
				if (pb->GetInt(kAnimEaseInType) != plAnimEaseTypes::kNoEase ||
					pb->GetInt(kAnimEaseOutType) != plAnimEaseTypes::kNoEase)
					PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);
			}
			return TRUE;

		// Max doesn't know about the rollup until after WM_CREATE, so we get
		// around it by posting a message
		case WM_ROLLOUT_OPEN:
			{
				IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
				int idx = rollup->GetPanelIndex(hWnd);
				rollup->SetPanelOpen(idx, TRUE);
			}
			return TRUE;
		}
		return FALSE;
	}

	void DeleteThis() {}
};	
//  For the paramblock below.
static plAnimEaseComponentProc gAnimEaseCompProc;

/*
// Make sure min is less than normal, which is less than max
class EaseAccessor : public PBAccessor
{
protected:
	bool fDoingUpdate;

	void AdjustMin(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
	{
		if (value > pb->GetFloat(normalID))
		{
			pb->SetValue(normalID, 0, value);
			if (value > pb->GetFloat(maxID))
				pb->SetValue(maxID, 0, value);
		}
	}
	void AdjustNormal(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
	{
		if (value < pb->GetFloat(minID))
			pb->SetValue(minID, 0, value);
		if (value > pb->GetFloat(maxID))
			pb->SetValue(maxID, 0, value);
	}
	void AdjustMax(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
	{
		if (value < pb->GetFloat(normalID))
		{
			pb->SetValue(normalID, 0, value);
			if (value < pb->GetFloat(minID))
				pb->SetValue(minID, 0, value);
		}
	}

public:
	EaseAccessor() : fDoingUpdate(false) {}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (fDoingUpdate)
			return;
		fDoingUpdate = true;

		plAnimComponent *comp = (plAnimComponent*)owner;
		IParamBlock2 *pb = comp->GetParamBlockByID(plComponentBase::kBlkComp);

		if (id == kAnimEaseInMin)
			AdjustMin(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMin, v.f);
		else if (id == kAnimEaseInLength)
			AdjustNormal(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMax, v.f);
		else if (id == kAnimEaseInMax)
			AdjustMax(pb, kAnimEaseInMin, kAnimEaseInLength, kAnimEaseInMax, v.f);
		else if (id == kAnimEaseOutMin)
			AdjustMin(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);
		else if (id == kAnimEaseOutLength)
			AdjustNormal(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);
		else if (id == kAnimEaseOutMax)
			AdjustMax(pb, kAnimEaseOutMin, kAnimEaseOutLength, kAnimEaseOutMax, v.f);

		fDoingUpdate = false;
	}
};
*/
static plEaseAccessor gAnimCompEaseAccessor(plComponentBase::kBlkComp, 
											kAnimEaseInMin, kAnimEaseInMax, kAnimEaseInLength,
											kAnimEaseOutMin, kAnimEaseOutMax, kAnimEaseOutLength);

CLASS_DESC(plAnimComponent, gAnimDesc, "Animation",  "Animation", COMP_TYPE_MISC, ANIM_COMP_CID)
CLASS_DESC(plAnimGroupedComponent, gAnimGroupedDesc, "Animation Grouped",  "AnimGrouped", COMP_TYPE_MISC, ANIM_GROUP_COMP_CID)

plAnimComponentBase::IsAnimComponent(plComponentBase *comp)
{
	return (comp->ClassID() == ANIM_COMP_CID ||
			comp->ClassID() == ANIM_GROUP_COMP_CID);
}
		
enum { kAnimMain, kAnimEase };

ParamBlockDesc2 gAnimBlock
(
	plComponent::kBlkComp, _T("animation"), 0, &gAnimDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	// map rollups
	2, 
	kAnimMain, IDD_COMP_ANIM, IDS_COMP_ANIM, 0, 0, &gAnimCompProc,
	kAnimEase, IDD_COMP_ANIM_EASE, IDS_COMP_ANIM_EASE, 0, APPENDROLL_CLOSED, &gAnimEaseCompProc,

	// Anim Main rollout
	kAnimAutoStart, _T("autoStart"),	TYPE_BOOL,		0, 0,
		p_ui,		kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AUTOSTART_CKBX,
		p_default,	FALSE,
		end,
	kAnimLoop,		_T("loop"),			TYPE_BOOL,		0, 0,
		p_ui,		kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_LOOP_CKBX,
		p_default,	FALSE,
		end,
	kAnimName,		_T("animName"),		TYPE_STRING,	0, 0,
		end,
	kAnimUseGlobal,		_T("UseGlobal"),	TYPE_BOOL,	0, 0,
		p_default,	FALSE,
		p_ui,	kAnimMain, TYPE_SINGLECHEKBOX,	IDC_COMP_ANIM_USE_GLOBAL,
		end,
	kAnimGlobalName,	_T("GlobalName"),	TYPE_STRING,	0,	0,
		p_default, _T(""),
		end,
	kAnimLoopName,	_T("loopName"),		TYPE_STRING,	0, 0,
		end,
	kAnimPhysAnim,	_T("PhysAnim"),		TYPE_BOOL,	0, 0,
		p_default, TRUE,
		p_ui,	kAnimMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_PHYSANIM,
		end,

	// Anim Ease rollout
	kAnimEaseInType,	_T("easeInType"),	TYPE_INT,		0, 0,
		p_ui,		kAnimEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_IN_NONE, IDC_COMP_ANIM_EASE_IN_CONST_ACCEL, IDC_COMP_ANIM_EASE_IN_SPLINE,
		p_vals,		plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
		p_default,	plAnimEaseTypes::kNoEase,
		end,
	kAnimEaseInLength,	_T("easeInLength"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_TIME, IDC_COMP_ANIM_EASE_IN_TIME_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	kAnimEaseInMin,		_T("easeInMin"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_MIN, IDC_COMP_ANIM_EASE_IN_MIN_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	kAnimEaseInMax,	_T("easeInMax"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_MAX, IDC_COMP_ANIM_EASE_IN_MAX_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,

	kAnimEaseOutType,	_T("easeOutType"),	TYPE_INT,		0, 0,
		p_ui,		kAnimEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_OUT_NONE, IDC_COMP_ANIM_EASE_OUT_CONST_ACCEL, IDC_COMP_ANIM_EASE_OUT_SPLINE,
		p_vals,		plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
		p_default,	plAnimEaseTypes::kNoEase,
		end,
	kAnimEaseOutLength,	_T("easeOutLength"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_TIME, IDC_COMP_ANIM_EASE_OUT_TIME_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	kAnimEaseOutMin,		_T("easeOutMin"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_MIN, IDC_COMP_ANIM_EASE_OUT_MIN_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	kAnimEaseOutMax,	_T("easeOutMax"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kAnimEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_MAX, IDC_COMP_ANIM_EASE_OUT_MAX_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,

	end
);

ParamBlockDesc2 gAnimGroupedBlock
(
	plComponent::kBlkComp, _T("animGrouped"), 0, &gAnimGroupedDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	// map rollups
	2, 
	kAnimMain, IDD_COMP_ANIM, IDS_COMP_ANIM_GROUPED, 0, 0, &gAnimCompProc,
	kAnimEase, IDD_COMP_ANIM_EASE, IDS_COMP_ANIM_EASE, 0, APPENDROLL_CLOSED, &gAnimEaseCompProc,

	// use params from existing descriptor
	&gAnimBlock,

	end
);

plAnimComponent::plAnimComponent()
{
	fClassDesc = &gAnimDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plKey plAnimComponent::GetModKey(plMaxNode *node)
{
	if (fMods.find(node) != fMods.end())
		return fMods[node]->GetKey();

	return nil;
}

hsBool	plAnimComponent::GetKeyList( INode *restrictedNode, hsTArray<plKey> &outKeys )
{
	if( restrictedNode != nil )
	{
		if( fMods.find( (plMaxNode *)restrictedNode ) != fMods.end() )
		{
			outKeys.Append( fMods[ (plMaxNode *)restrictedNode ]->GetKey() );
			return true;
		}
		return false;
	}
	else
	{
		hsAssert( false, "DO SOMETHING!" );
		return false;
	}
}

plAnimGroupedComponent::plAnimGroupedComponent() : fForward(nil)
{
	fClassDesc = &gAnimGroupedDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plKey plAnimGroupedComponent::GetModKey(plMaxNode *node)
{
	if( fForward )
		return fForward->GetKey();
	return nil;
}

hsBool	plAnimGroupedComponent::GetKeyList( INode *restrictedNode, hsTArray<plKey> &outKeys )
{
	if( fForward )
	{
		outKeys.Append( fForward->GetKey() );
		return true;
	}
	return false;
}


#include "../pnMessage/plNodeRefMsg.h"

hsBool plAnimGroupedComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	bool needSetMaster = fNeedReset;
	if (fNeedReset)
	{
		fForward = TRACKED_NEW plMsgForwarder;
		plKey forwardKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), fForward, node->GetLocation());

		plNodeRefMsg *refMsg = TRACKED_NEW plNodeRefMsg(node->GetRoomKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric);
		hsgResMgr::ResMgr()->AddViaNotify(forwardKey, refMsg, plRefFlags::kActiveRef);
	}

	hsBool ret = plAnimComponentBase::PreConvert(node, pErrMsg);

	plAGMasterMod *mod = fMods[node];

	if (needSetMaster)
		mod->SetIsGroupMaster(true, fForward);
	mod->SetIsGrouped(true);

	fForward->AddForwardKey(mod->GetKey());

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////

plAnimComponentBase::plAnimComponentBase() : fNeedReset(true)
{
}


const char *plAnimComponentBase::GetAnimName()
{
	const char *name = fCompPB->GetStr(kAnimName);
	if (!name || name[0] == '\0')
		return nil;
	return name;
}

bool IsSubworld(plMaxNode* node)
{
	UInt32 numComps = node->NumAttachedComponents();
	for (int i = 0; i < numComps; i++)
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if (comp && comp->ClassID() == PHYS_SUBWORLD_CID)
			return true;
	}

	return false;
}

void SetPhysAnimRecurse(plMaxNode *node, plErrorMsg *pErrMsg)
{
	// If we hit a subworld, stop.  The subworld may be animated, but the
	// physicals in it aren't.
	if (IsSubworld(node))
		return;

	if (HasPhysicalComponent(node, false))
	{	char* debugName = node->GetName();
		node->GetPhysicalProps()->SetPhysAnim(true, node, pErrMsg);
	}
	int i;
	for (i = 0; i < node->NumberOfChildren(); i++)
		SetPhysAnimRecurse((plMaxNode *)node->GetChildNode(i), pErrMsg);
}

hsBool plAnimComponentBase::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	
{
	if (node->IsTMAnimated())
	{
		node->SetMovable(true);
		node->SetForceLocal(true);
	
		//
		// forceLocal on our parent (since keys work in local space)
		//
		plMaxNode *parent = (plMaxNode *)node->GetParentNode();
		if (!parent->IsRootNode())
		{
			parent->SetForceLocal(true);

			//char str[512];
			//sprintf(str, "Forcing local on '%s' because of animated child '%s'\n",parent->GetName(),node->GetName() );
			//OutputDebugString(str);
		}
	}

	if (fCompPB->GetInt(ParamID(kAnimPhysAnim)))
		SetPhysAnimRecurse(node, pErrMsg);
	
	/*
	int childCount = node->NumberOfChildren();
	for (int i = 0; i < childCount; i++)
	{
		SetupProperties((plMaxNode *)node->GetChildNode(i), pErrMsg);
	}
	*/
	return true; 
}

hsBool plAnimComponentBase::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	// If this is the first time in the preconvert, reset the map
	if (fNeedReset)
	{
		fNeedReset = false;
	}

	// If this node is animated, create it's modifier and key now so we can give
	// it out to anyone that needs it
//	if (node->IsTMAnimated() || node->IsAnimatedLight())
//	{
		const char *name = node->GetName();

		plAGMasterMod *mod = node->GetAGMasterMod();
		if (mod == nil)
		{
			if (!node->HasAGMod()) // Need to add this before the MasterMod, if it doesn't have one already.
			{
				node->AddModifier(new plAGModifier(node->GetName()), IGetUniqueName(node));
			}
			mod = TRACKED_NEW plAGMasterMod();

			plKey modKey = node->AddModifier(mod, IGetUniqueName(node));
		}
		fMods[node] = mod;
//	}


	// Small change here. We're setting up the timing specs on the
	// plAGAnim object during preconvert, so that the info is available
	// when actually converting the anim (and for other components
	// that need it, but may or may not actually convert before us.)

	// Note: if the component uses the "(Entire Animation)" segment for
	// the main start/end, the start/end times won't be valid until
	// we've added all keys during convert. Some cleanup might
	// be necessary in this case.

	char *animName = fCompPB->GetStr(kAnimName);
	if (animName == nil || !strcmp(animName, ""))
		animName = ENTIRE_ANIMATION_NAME;

	if (fCompPB->GetInt(ParamID(kAnimUseGlobal)))
	{
		plAgeGlobalAnim *ageAnim = TRACKED_NEW plAgeGlobalAnim(animName, 0, 0);
		ageAnim->SetGlobalVarName(fCompPB->GetStr(ParamID(kAnimGlobalName)));

		fAnims[node] = ageAnim;
	}
	else
	{
		plATCAnim *ATCAnim = TRACKED_NEW plATCAnim(animName, 0, 0);
		plNotetrackAnim noteAnim(node, pErrMsg);
		plAnimInfo info = noteAnim.GetAnimInfo(animName);
		ATCAnim->SetAutoStart(fCompPB->GetInt(kAnimAutoStart));

		hsScalar start = info.GetAnimStart();
		hsScalar end = info.GetAnimEnd();
		hsScalar initial = info.GetAnimInitial();
		if (start != -1)
			ATCAnim->SetStart(start);
		if (end != -1)
			ATCAnim->SetEnd(end);
		if (initial != -1)
			ATCAnim->SetInitial(initial);
	
		if (fCompPB->GetInt(kAnimLoop))
		{
			ATCAnim->SetLoop(true);
			char *loopName = fCompPB->GetStr(kAnimLoopName);
			hsScalar loopStart = info.GetLoopStart(loopName);
			hsScalar loopEnd = info.GetLoopEnd(loopName);

			ATCAnim->SetLoopStart(loopStart == -1 ? ATCAnim->GetStart() : loopStart);
			ATCAnim->SetLoopEnd(loopEnd == -1 ? ATCAnim->GetEnd() : loopEnd);
		}
	
		while (const char *loop = info.GetNextLoopName())
			ATCAnim->AddLoop(loop, info.GetLoopStart(loop), info.GetLoopEnd(loop));

		while (const char *marker = info.GetNextMarkerName())
			ATCAnim->AddMarker(marker, info.GetMarkerTime(marker));

		float stopPoint = -1;
		while ((stopPoint = info.GetNextStopPoint()) != -1)
			ATCAnim->AddStopPoint(stopPoint);

		ATCAnim->SetEaseInType(fCompPB->GetInt(kAnimEaseInType));
		ATCAnim->SetEaseOutType(fCompPB->GetInt(kAnimEaseOutType));
		ATCAnim->SetEaseInLength(fCompPB->GetFloat(kAnimEaseInLength));
		ATCAnim->SetEaseInMin(fCompPB->GetFloat(kAnimEaseInMin));
		ATCAnim->SetEaseInMax(fCompPB->GetFloat(kAnimEaseInMax));
		ATCAnim->SetEaseOutLength(fCompPB->GetFloat(kAnimEaseOutLength));
		ATCAnim->SetEaseOutMin(fCompPB->GetFloat(kAnimEaseOutMin));
		ATCAnim->SetEaseOutMax(fCompPB->GetFloat(kAnimEaseOutMax));

		fAnims[node] = ATCAnim;
	}
	return true;
}


hsBool plAnimComponentBase::IAddTMToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
	hsBool result = false;

	// Get the affine parts and the TM Controller
	plSceneObject *obj = node->GetSceneObject();
	hsAffineParts * parts = TRACKED_NEW hsAffineParts;
	plController* tmc;

	if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
		tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts);
	else 
		tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts, anim->GetStart(), anim->GetEnd());

	if (tmc)
	{
		plMatrixChannelApplicator *app = TRACKED_NEW plMatrixChannelApplicator();
			app->SetChannelName(node->GetName());
		plMatrixControllerChannel *channel = TRACKED_NEW plMatrixControllerChannel(tmc, parts);
		app->SetChannel(channel);
		anim->AddApplicator(app);
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			anim->ExtendToLength(tmc->GetLength());
		result = true;
	}

	delete parts;	// We copy this over, so no need to keep it around
	return result;
}

hsBool plAnimComponentBase::IAddLightToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
	if (!node->IsAnimatedLight())
		return false;

	Object *obj = node->GetObjectRef();
	Class_ID cid = obj->ClassID();

	IParamBlock2 *pb = nil;
	if (cid == RTSPOT_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkSpotLight);
	else if (cid == RTOMNI_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);
	else if (cid == RTDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);
	else if (cid == RTPDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkMain);

	node->GetRTLightColAnim(pb, anim);

	if (cid == RTSPOT_LIGHT_CLASSID || cid == RTOMNI_LIGHT_CLASSID)
		node->GetRTLightAttenAnim(pb, anim);

	if (cid == RTSPOT_LIGHT_CLASSID)
		node->GetRTConeAnim(pb, anim);

	return true;
}

hsBool plAnimComponentBase::IConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
	hsBool madeAnim = false;
	int i;

	if (IAddTMToAnim(node, anim, pErrMsg))
		madeAnim = true;
	if (IAddLightToAnim(node, anim, pErrMsg))
		madeAnim = true;

	for (i = 0; i < node->NumAttachedComponents(); i++)
	{
		if (node->GetAttachedComponent(i)->AddToAnim(anim, node))
			madeAnim = true;
	}

	if (madeAnim)
	{
		// It has an animation, we're going to need a plAGMod when loading the anim
		if (!node->HasAGMod())
		{
			node->AddModifier(new plAGModifier(node->GetName()), IGetUniqueName(node));
		}
		madeAnim = true;
	}
/*
	// let's see if the children have any segments specified...
	int childCount = node->NumberOfChildren();
	for (int i = 0; i < childCount; i++)
	{
		if (IConvertNodeSegmentBranch((plMaxNode *)(node->GetChildNode(i)), anim, pErrMsg))
			madeAnim = true;
	}
*/
	return madeAnim;
}

hsBool plAnimComponentBase::IMakePersistent(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg)
{
	// anims made by this component are private to the specific AGMasterMod, so we attach them there.
	plAGMasterMod *mod = plAGMasterMod::ConvertNoRef(fMods[node]);
	hsAssert(mod != nil, "No MasterMod to make animation persistent!");

	char buffer[256];
	sprintf(buffer, "%s_%s_anim_%d", node->GetName(), anim->GetName(), mod->GetNumPrivateAnimations());
	plLocation nodeLoc = node->GetLocation();
	plKey animKey = hsgResMgr::ResMgr()->NewKey(buffer, anim, nodeLoc);

	plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnCreate, 0, 0);
	hsgResMgr::ResMgr()->AddViaNotify(animKey, refMsg, plRefFlags::kActiveRef);

	return true;
}


hsBool plAnimComponentBase::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fNeedReset = true;

	if (!IConvertNodeSegmentBranch(node, fAnims[node], pErrMsg))
	{
		// Either we delete it here, or we make it persistent below and the resMgr handles it
		delete fAnims[node];
		fAnims[node] = nil;
		return false;
	}

	if (fCompPB->GetInt(ParamID(kAnimUseGlobal)))
	{
		((plAgeGlobalAnim *)fAnims[node])->SetGlobalVarName(fCompPB->GetStr(ParamID(kAnimGlobalName)));
	}
	else // It's an ATCAnim
	{
		// If we're on an "(Entire Animation)" segment. The loops won't know their lengths until
		// after the nodes have been converted and added. So we adjust them here if necessary.
		((plATCAnim *)fAnims[node])->CheckLoop();
	}
	
	IMakePersistent(node, fAnims[node], pErrMsg);
	return true;
}

hsBool plAnimComponentBase::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)		
{ 
	fMods.clear();
	fLightMods.clear();
	fAnims.clear();
	return true;
}

void plAnimComponentBase::SetupCtl( plAGAnim *anim, plController *ctl, plAGApplicator *app, plMaxNode *node )
{
	plScalarControllerChannel *channel = TRACKED_NEW plScalarControllerChannel(ctl);
	app->SetChannel(channel);
	anim->AddApplicator(app);
	if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
		anim->ExtendToLength(ctl->GetLength());
}

//// Picker Dialog for Restricted Animation Components //////////////////////////////////////////

class plPickAnimCompNode : public plPickCompNode
{
protected:
	ParamID fTypeID;

	void IAddUserType(HWND hList)
	{
		int type = fPB->GetInt(fTypeID);

		int idx = ListBox_AddString(hList, kUseParamBlockNodeString);
		if (type == plAnimObjInterface::kUseParamBlockNode && !fPB->GetINode(fNodeParamID))
			ListBox_SetCurSel(hList, idx);


		idx = ListBox_AddString(hList, kUseOwnerNodeString);
		if (type == plAnimObjInterface::kUseOwnerNode)
			ListBox_SetCurSel(hList, idx);
	}

	void ISetUserType(plMaxNode* node, const char* userType)
	{
		if (hsStrEQ(userType, kUseParamBlockNodeString))
		{
			ISetNodeValue(nil);
			fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
		}
		else if (hsStrEQ(userType, kUseOwnerNodeString))
		{
			ISetNodeValue(nil);
			fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseOwnerNode);
		}
		else
			fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
	}

public:
	plPickAnimCompNode(IParamBlock2* pb, ParamID nodeParamID, ParamID typeID, plComponentBase *comp) :
	  plPickCompNode(pb, nodeParamID, comp), fTypeID(typeID)
	{
	}
};

void	plAnimComponentBase::PickTargetNode( IParamBlock2 *destPB, ParamID destParamID, ParamID destTypeID )
{
	plPickAnimCompNode pick( destPB, destParamID, destTypeID, (plComponentBase *)this );
	pick.DoPick();
}

const char	*plAnimComponentBase::GetIfaceSegmentName( hsBool allowNil )
{
	const char *name = GetAnimName();
	if( allowNil || name != nil )
		return name;
	return ENTIRE_ANIMATION_NAME;
}

//// Hit Callback for Animations /////////////////////////////////////////////

class plPlasmaAnimHitCallback : public HitByNameDlgCallback
{
protected:
	IParamBlock2*	fPB;
	ParamID			fParamID;
	TCHAR			fTitle[ 128 ];

public:
	plPlasmaAnimHitCallback( IParamBlock2 *pb, ParamID paramID, TCHAR *title = nil )
		: fPB( pb ), fParamID( paramID )

	{
		strcpy( fTitle, title );
	}

	virtual TCHAR *dialogTitle() { return fTitle; }
	virtual TCHAR *buttonText() { return "OK"; }

	virtual int filter( INode *node )
	{
		plComponentBase *comp = ( (plMaxNodeBase *)node )->ConvertToComponent();
		if( comp != nil && plAnimComponentBase::IsAnimComponent( comp ) )
		{
			// Make sure it won't create a cyclical reference (Max doesn't like those)
			if( comp->TestForLoop( FOREVER, fPB ) == REF_FAIL )
				return FALSE;

			return TRUE;
		}
		else
		{
			plAnimStealthNode *stealth = plAnimStealthNode::ConvertToStealth( node );
			if( stealth != nil )
			{
				if( stealth->TestForLoop( FOREVER, fPB ) == REF_FAIL )
					return FALSE;

				if( !stealth->IsParentUsedInScene() )
					return FALSE;

				return TRUE;
			}
		}

		return FALSE;
	}

	virtual void proc( INodeTab &nodeTab )
	{
		fPB->SetValue( fParamID, (TimeValue)0, nodeTab[ 0 ] );
	}

	virtual BOOL showHiddenAndFrozen() { return TRUE; }
	virtual BOOL singleSelect() { return TRUE; }
};

//// Dialog Proc For Anim Selection /////////////////////////////////////////////////////////////

plPlasmaAnimSelectDlgProc::plPlasmaAnimSelectDlgProc( ParamID paramID, int dlgItem, TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc )
{
	fParamID = paramID;
	fDlgItem = dlgItem;
	fUseNode = false;
	strcpy( fTitle, promptTitle );
	fChain = chainedDlgProc;
}

plPlasmaAnimSelectDlgProc::plPlasmaAnimSelectDlgProc( ParamID paramID, int dlgItem, ParamID nodeParamID, ParamID typeParamID, int nodeDlgItem, 
													  TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc )
{
	fParamID = paramID;
	fDlgItem = dlgItem;
	fUseNode = true;
	fNodeParamID = nodeParamID;
	fTypeParamID = typeParamID;
	fNodeDlgItem = nodeDlgItem;
	strcpy( fTitle, promptTitle );
	fChain = chainedDlgProc;
}

void	plPlasmaAnimSelectDlgProc::SetThing( ReferenceTarget *m )
{
	if( fChain != nil )
		fChain->SetThing( m );
}

void	plPlasmaAnimSelectDlgProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
	if( fChain != nil )
		fChain->Update( t, valid, pmap );
}

void	plPlasmaAnimSelectDlgProc::IUpdateNodeBtn( HWND hWnd, IParamBlock2 *pb )
{
	if( fUseNode )
	{
		int type = pb->GetInt( fTypeParamID );
		if( type == plAnimObjInterface::kUseOwnerNode )
			::SetWindowText( ::GetDlgItem( hWnd, fNodeDlgItem ), kUseOwnerNodeString );
		else
		{
			INode *node = pb->GetINode( fNodeParamID );
			TSTR newName( node ? node->GetName() : kUseParamBlockNodeString );
			::SetWindowText( ::GetDlgItem( hWnd, fNodeDlgItem ), newName );
		}

		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( pb->GetINode( fParamID ) );
		if( iface == nil || !iface->IsNodeRestricted() )
			::EnableWindow( ::GetDlgItem( hWnd, fNodeDlgItem ), false );
		else
		{
			::EnableWindow( ::GetDlgItem( hWnd, fNodeDlgItem ), true );
		}
	}
}

BOOL	plPlasmaAnimSelectDlgProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			{
				IParamBlock2 *pb = pmap->GetParamBlock();

				INode *node = pb->GetINode( fParamID );
				TSTR newName( node ? node->GetName() : "Pick" );
				::SetWindowText( ::GetDlgItem( hWnd, fDlgItem ), newName );

				IUpdateNodeBtn( hWnd, pb );
			}
			break;

		case WM_COMMAND:
			if( ( HIWORD( wParam ) == BN_CLICKED ) )
			{
				if( LOWORD( wParam ) == fDlgItem )
				{
					IParamBlock2 *pb = pmap->GetParamBlock();
					plPlasmaAnimHitCallback hitCB( pb, fParamID, fTitle );
					GetCOREInterface()->DoHitByNameDialog( &hitCB );

					INode *node = pb->GetINode( fParamID );
					TSTR newName( node ? node->GetName() : "Pick" );
					::SetWindowText( ::GetDlgItem(hWnd, fDlgItem ), newName );
					pmap->Invalidate( fParamID );
					::InvalidateRect( hWnd, NULL, TRUE );

					IUpdateNodeBtn( hWnd, pb );
					return true;
				}
				else if( fUseNode && LOWORD( wParam ) == fNodeDlgItem )
				{
					IParamBlock2 *pb = pmap->GetParamBlock();

					plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( pb->GetINode( fParamID ) );
					iface->PickTargetNode( pb, fNodeParamID, fTypeParamID );

					IUpdateNodeBtn( hWnd, pb );
					return true;
				}
			}
			break;
	}

	if( fChain != nil )
		return fChain->DlgProc( t, pmap, hWnd, msg, wParam, lParam );

	return false;
}

void plPlasmaAnimSelectDlgProc::DeleteThis() 
{ 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CLASS_DESC(plAnimCompressComp, gAnimCompressDesc, "Anim Compress",  "AnimCompress", COMP_TYPE_MISC, ANIM_COMPRESS_COMP_CID)

ParamBlockDesc2 gAnimCompressBk
(	
	plComponent::kBlkComp, _T("AnimCompress"), 0, &gAnimCompressDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,
	IDD_COMP_ANIM_COMPRESS, IDS_COMP_ANIM_COMPRESS_ROLL, 0, 0, nil,

	plAnimCompressComp::kAnimCompressLevel,	_T("compressLevel"),	TYPE_INT,		0, 0,
		p_ui,		TYPE_RADIO, 3, IDC_COMP_ANIM_COMPRESS_NONE, IDC_COMP_ANIM_COMPRESS_LOW, IDC_COMP_ANIM_COMPRESS_HIGH,
		p_vals,		plAnimCompressComp::kCompressionNone, plAnimCompressComp::kCompressionLow, plAnimCompressComp::kCompressionHigh,
		p_default,	plAnimCompressComp::kCompressionLow,
		end,

	plAnimCompressComp::kAnimCompressThreshold,	_T("Threshold"),	TYPE_FLOAT, 	0, 0,	
		p_default, 0.01,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_COMPRESS_THRESHOLD, IDC_COMP_ANIM_COMPRESS_THRESHOLD_SPIN, 0.001,
		end,

	end
);

plAnimCompressComp::plAnimCompressComp()
{
	fClassDesc = &gAnimCompressDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plAnimCompressComp::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	
{
	node->SetAnimCompress(fCompPB->GetInt(ParamID(kAnimCompressLevel)));

	// We use Max's key reduction code, which doesn't seem to match up with its own UI.
	// Manually using Max's "Reduce Keys" option with a threshold of .01 seems to give
	// approximately the same results as calling the function ApplyKeyReduction with
	// a threshold of .0002. I want the UI to appear consistent to the artist, so we
	// shrug our shoulders and scale down by 50.
	node->SetKeyReduceThreshold(fCompPB->GetFloat(ParamID(kAnimCompressThreshold)) / 50.f);
	return true;
}

hsBool plAnimCompressComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}