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
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxExport/plExportProgressBar.h"

#include "plXImposter.h"

#include "../pfAnimation/plFilterCoordInterface.h"

#include "../pnSceneObject/plSimulationInterface.h"
#include "plPhysical.h"

const Class_ID FILTERINHERIT_COMP_CID(0x263928d8, 0x548456da);

void DummyCodeIncludeFuncXImposter()
{

}

//Class that accesses the paramblock below.
//////////////////////////////////////////////////////////////////////////////////////////////////////

//Max desc stuff necessary below.
CLASS_DESC(plXImposterComp, gXImposterDesc, "X-Form",  "X-Form", COMP_TYPE_DISTRIBUTOR, XIMPOSTER_COMP_CID)

ParamBlockDesc2 gXImposterBk
(	
	plComponent::kBlkComp, _T("X-Form"), 0, &gXImposterDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_XFORM, IDS_COMP_XFORMS, 0, 0, NULL,

	end
);

plXImposterComp::plXImposterComp()
{
	fClassDesc = &gXImposterDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plXImposterComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetRadiateNorms(true);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
const Class_ID FORCE_CTT_COMP_CID(0x30ee73b7, 0x4cdd551b);

class plForceCTTComp : public plComponent
{
public:
	plForceCTTComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};

//Max desc stuff necessary below.
CLASS_DESC(plForceCTTComp, gForceCTTDesc, "ForceClick2Turn",  "ForceCTT", COMP_TYPE_MISC, FORCE_CTT_COMP_CID)

ParamBlockDesc2 gForceCTTBk
(	
	plComponent::kBlkComp, _T("ForceCTT"), 0, &gForceCTTDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FORCE_CTT, IDS_COMP_FORCE_CTT, 0, 0, NULL,

	end
);

plForceCTTComp::plForceCTTComp()
{
	fClassDesc = &gForceCTTDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plForceCTTComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceVisLOS(true);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The filter inheritance component doesn't have anything to do with the XImposter, but there's
// plenty of space here, so blow me.

class plFilterInheritComp : public plComponent
{
public:
	enum
	{
		kActive,
		kNoX,
		kNoY,
		kNoZ
	};
public:
	plFilterInheritComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	hsBool	SetMaxInherit();
	hsBool	SetMaxInherit(plMaxNodeBase* targ);
	hsBool	KillMaxInherit();
	hsBool	KillMaxInherit(plMaxNodeBase* targ);

	hsBool	Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg);

	virtual void AddTarget(plMaxNodeBase *target);
	virtual void DeleteTarget(plMaxNodeBase *target);
	virtual void DeleteAllTargets();
};

class FilterInheritCompDlgProc : public ParamMap2UserDlgProc
{
protected:
	void ISetTransEnable(IParamMap2* map)
	{
		IParamBlock2* pb = map->GetParamBlock();
		if( !pb->GetInt(plFilterInheritComp::kActive) )
		{
			map->Enable(plFilterInheritComp::kNoX, FALSE);
			map->Enable(plFilterInheritComp::kNoY, FALSE);
			map->Enable(plFilterInheritComp::kNoZ, FALSE);
		}
		else
		{
			map->Enable(plFilterInheritComp::kNoX, TRUE);
			map->Enable(plFilterInheritComp::kNoY, TRUE);
			map->Enable(plFilterInheritComp::kNoZ, TRUE);
		}
		plFilterInheritComp* comp = (plFilterInheritComp*)map->GetParamBlock()->GetOwner();
		comp->SetMaxInherit();
	}
public:
	FilterInheritCompDlgProc() {}
	~FilterInheritCompDlgProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ISetTransEnable(map);
			break;
		case WM_COMMAND: 
			ISetTransEnable(map);
			break;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static FilterInheritCompDlgProc gFilterInheritCompDlgProc;



CLASS_DESC(plFilterInheritComp, gFilterInheritDesc, "Filter Inherit",  "FiltInherit", COMP_TYPE_MISC, FILTERINHERIT_COMP_CID)

ParamBlockDesc2 gFilterInheritBk
(	
	plComponent::kBlkComp, _T("FilterInherit"), 0, &gFilterInheritDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FILTERINHERIT, IDS_COMP_FILTER, 0, 0, &gFilterInheritCompDlgProc,

	plFilterInheritComp::kActive,	_T("Active"),	TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_FILTER_ACTIVE,
		end,

	plFilterInheritComp::kNoX,	_T("NoX"),	TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_FILTER_NOX,
		end,

	plFilterInheritComp::kNoY,	_T("NoY"),	TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_FILTER_NOY,
		end,

	plFilterInheritComp::kNoZ,	_T("NoZ"),	TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_FILTER_NOZ,
		end,

	end
);

plFilterInheritComp::plFilterInheritComp()
{
	fClassDesc = &gFilterInheritDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plFilterInheritComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fCompPB->GetInt(kActive) )
		return true;

	if( node->GetParentNode()->IsRootNode() )
		return true;

	node->SetFilterInherit(true);
	node->SetForceLocal(true);
	plMaxNode* parent = (plMaxNode*)node->GetParentNode();
	parent->SetForceLocal(true);

	// Okay, everything works fine as long as you set up your heirarchy, and THEN
	// add this component. But if you put this component on the as yet unlinked child,
	// and THEN link the child to the parent, Max starts reporting bogus local TM info.
	// If you turn off the component (uncheck the disable rotation checkbox), and turn
	// it back on, Max is happy again.
	// However, if I just turn off the component and turn it right back on again here,
	// (a KillMaxInherit() followed by a SetMaxInherit()), apparently Max hasn't had
	// enough time to think in between, and so stays confused.
	// Soooo, here at the last minute before conversion, we kill all those inherit
	// checkboxes for this node, then after we've finished converting, we turn them
	// back on. Note that we don't currently give a rat's patooties whether the check
	// boxes are set or not, we just want to turn them off and turn them back on
	// sometime before export, with enough time in between for Max to get on to the fact.
	// See the matching SetMaxInherit() at the end of Convert().
	KillMaxInherit(node);

	return true;
}

hsBool plFilterInheritComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plFilterInheritComp::Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg)
{
	pErrMsg->Set(true, node->GetName(), msg).CheckAndAsk();
	pErrMsg->Set(false);
	return true;
}

hsBool plFilterInheritComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fCompPB->GetInt(kActive) )
		return true;

	if( node->GetParentNode()->IsRootNode() )
		return true;

	plSceneObject* so = node->GetSceneObject();
	if( !so )
	{
		return Bail(node, "Error finding scene object for filtered inheritance", pErrMsg);
	}

	const plCoordinateInterface* co = so->GetCoordinateInterface();
	if( !co )
	{
		return Bail(node, "Error setting filtered inheritance - no coordinate interface", pErrMsg);
	}
	
	plFilterCoordInterface* filt = plFilterCoordInterface::ConvertNoRef(const_cast<plCoordinateInterface*>(co));
	if( !filt )
	{
		return Bail(node, "Error setting filtered inheritance - wrong coordinate interface", pErrMsg);
	}

	const plSimulationInterface* si = so->GetSimulationInterface();
	if (si)
	{
		plPhysical* phys = si->GetPhysical();
		// tell the physical not to send transforms back -- they'll be wrong if it tries to compose w/a subworld
		// this rules out using transform filters on dynamically simulated objects....
		phys->SetProperty(plSimulationInterface::kPassive, true);
	}

	UInt32 mask = plFilterCoordInterface::kNoRotation;
	if( fCompPB->GetInt(kNoX) )
		mask |= plFilterCoordInterface::kNoTransX;
	if( fCompPB->GetInt(kNoY) )
		mask |= plFilterCoordInterface::kNoTransY;
	if( fCompPB->GetInt(kNoZ) )
		mask |= plFilterCoordInterface::kNoTransZ;

	plMaxNode* parent = (plMaxNode*)node->GetParentNode();
	hsMatrix44 parL2W = parent->GetLocalToWorld44(TimeValue(0));

	filt->SetFilterMask(mask);
	filt->SetRefLocalToWorld(parL2W);

	// See the matching KillMaxInherit() in SetupProperties().
	SetMaxInherit(node);
	
	return true;
}

hsBool plFilterInheritComp::SetMaxInherit(plMaxNodeBase* targ)
{
	if( !fCompPB->GetInt(kActive) )
		return KillMaxInherit(targ);

	DWORD mask = INHERIT_ROT_X
		| INHERIT_ROT_Y
		| INHERIT_ROT_Z
		| INHERIT_SCL_X
		| INHERIT_SCL_Y
		| INHERIT_SCL_Z;

	if( fCompPB->GetInt(kNoX) )
		mask |= INHERIT_POS_X;
	if( fCompPB->GetInt(kNoY) )
		mask |= INHERIT_POS_Y;
	if( fCompPB->GetInt(kNoZ) )
		mask |= INHERIT_POS_Z;

	// Max documentation is a big fat liar
	mask = ~mask;

	if( targ )
	{
		targ->GetTMController()->SetInheritanceFlags(mask, true);
		targ->GetTMController()->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	}

	return true;
}

hsBool plFilterInheritComp::SetMaxInherit()
{
	if( !fCompPB->GetInt(kActive) )
		return KillMaxInherit();

	DWORD mask = INHERIT_ROT_X
		| INHERIT_ROT_Y
		| INHERIT_ROT_Z
		| INHERIT_SCL_X
		| INHERIT_SCL_Y
		| INHERIT_SCL_Z;

	if( fCompPB->GetInt(kNoX) )
		mask |= INHERIT_POS_X;
	if( fCompPB->GetInt(kNoY) )
		mask |= INHERIT_POS_Y;
	if( fCompPB->GetInt(kNoZ) )
		mask |= INHERIT_POS_Z;

	// Max documentation is a big fat liar
	mask = ~mask;

	int i;
	for( i = 0; i < NumTargets(); i++ )
	{
		plMaxNodeBase* targ = GetTarget(i);
		if( targ )
		{
			targ->GetTMController()->SetInheritanceFlags(mask, true);
			targ->GetTMController()->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		}
	}
	return true;
}

hsBool plFilterInheritComp::KillMaxInherit(plMaxNodeBase* targ)
{
	// Max documentation is a big fat liar
	DWORD mask = ~0;

	targ->GetTMController()->SetInheritanceFlags(mask, true);
	targ->GetTMController()->NotifyDependents(FOREVER,0,REFMSG_CHANGE);

	return true;
}

hsBool plFilterInheritComp::KillMaxInherit()
{
	int i;
	for( i = 0; i < NumTargets(); i++ )
	{
		plMaxNodeBase* targ = GetTarget(i);
		if( targ )
		{
			KillMaxInherit(targ);
		}
	}
	return true;
}

void plFilterInheritComp::AddTarget(plMaxNodeBase *target)
{
	plComponentBase::AddTarget(target);

	SetMaxInherit();
}

void plFilterInheritComp::DeleteTarget(plMaxNodeBase *target)
{
	plComponentBase::DeleteTarget(target);

	KillMaxInherit(target);
}

void plFilterInheritComp::DeleteAllTargets()
{
	KillMaxInherit();

	plComponentBase::DeleteAllTargets();
}


