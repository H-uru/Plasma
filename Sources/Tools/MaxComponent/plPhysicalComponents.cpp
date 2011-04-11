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

// singular
#include "plPhysicalComponents.h"

// local
#include "plComponentReg.h"
#include "resource.h"

// global
#include "hsResMgr.h"

// other
#include "../plPhysical/plSimDefs.h"
#include "plPhysicsGroups.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxMain/plPhysicalProps.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../plPhysical/plCollisionDetector.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../plMessage/plSwimMsg.h"
#include "../plAvatar/plSwimRegion.h"
#include "../plMessage/plRideAnimatedPhysMsg.h"

/////////////////////////////////////////////////////////////////
//
// THE DUMMY
//
/////////////////////////////////////////////////////////////////

// Necessary to keep the compiler from throwing away this file.
void DummyCodeIncludeFuncPhys()
{
}

/////////////////////////////////////////////////////////////////
//
// plPhysicCoreComponent
//
/////////////////////////////////////////////////////////////////

// SetupProperties -----------------------------------------------------------------
// ----------------
hsBool plPhysicCoreComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

// PreConvert ----------------------------------------------------------------
// -----------
hsBool plPhysicCoreComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

// IFixBounds --------------------------
// -----------
void plPhysicCoreComponent::IFixBounds()
{
	if (fCompPB->GetInt(kBoundCondRadio) == 0)
	{
		// zero is a bad value left over from an old version of the GUI: upgrade in place
		fCompPB->Reset(kBoundCondRadio);									// reset to default
	}
}

// IGetProxy ----------------------------------------------------------------
// ----------
hsBool plPhysicCoreComponent::IGetProxy(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (fCompPB->GetInt(kCustomBoundField))
	{
		plMaxNode *boundNode = (plMaxNode*)fCompPB->GetINode(kCustomBoundListStuff);
		if (boundNode && boundNode->CanConvert())
			return node->GetPhysicalProps()->SetProxyNode(boundNode, node, pErrMsg);
	}

	return true;
}

// Convert ----------------------------------------------------------------
// --------
hsBool plPhysicCoreComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}


//! Global Accessor Class, used in ParamBlock2 processing.
/*!

 When one of our parameters that is a ref changes, send out the component ref
 changed message.  Normally, messages from component refs are ignored since
 they pass along all the messages of the ref, which generates a lot of false
 converts.
*/

class plPhysCoreAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plPhysicCoreComponent::kCustomBoundListStuff)
		{
			plComponentBase *comp = (plComponentBase*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plPhysCoreAccessor gPhysCoreAccessor;
/*
//!  Physics Debug Component Class
class plPhysDebugComponent : public plPhysicCoreComponent
{
public:
	plPhysDebugComponent();
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 

	virtual void CollectNonDrawables(INodeTab& nonDrawables);
};
*/
OBSOLETE_CLASS(plPhysDebugComponent, gPhysDebugDesc, "Physics Debug", "PhysDebug", COMP_TYPE_PHYSICAL, PHYSICS_DEBUG_CID)

enum
{
	kPhysMain,
	kPhysMember,
	kPhysBounce,
	kPhysReport,
};
/*
ParamBlockDesc2 gPhysicalBk
(	
	plComponent::kBlkComp, _T("physicsDebug"), 0, &gPhysDebugDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	4, 
	kPhysMain, IDD_COMP_PHYSICAL, IDS_COMP_PHYS_DEBUG, 0, 0, NULL,
	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
	kPhysReport, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_REPORT, 0, APPENDROLL_CLOSED, &gReportGroupProc,

	// params
	plPhysicCoreComponent::kMass,		_T("Mass"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 500.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYSICAL_EDIT1, IDC_COMP_PHYSICAL_SPIN1, 1.0,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYSICAL_EDIT2, IDC_COMP_PHYSICAL_SPIN2, 0.1,
		end,

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYSICAL_EDIT3, IDC_COMP_PHYSICAL_SPIN3, 0.1,
		end,
	
	plPhysicCoreComponent::kStartForceX, _T("StartForceX"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_PHYSICAL_P3SF_EDIT1,IDC_COMP_PHYSICAL_P3SF_SPIN1, 0.1f,
		end,

	plPhysicCoreComponent::kStartForceY, _T("StartForceY"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_PHYSICAL_P3SF_EDIT2,IDC_COMP_PHYSICAL_P3SF_SPIN2, 0.1f,
		end,
	
	plPhysicCoreComponent::kStartForceZ, _T("StartForceZ"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_PHYSICAL_P3SF_EDIT3,IDC_COMP_PHYSICAL_P3SF_SPIN3, 0.1f,
		end,

	plPhysicCoreComponent::kStartTorqueX, _T("StartTorqueX"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_COMP_PHYSICAL_P3ST_EDIT1,IDC_COMP_PHYSICAL_P3ST_SPIN1, 0.1f,
		end,

	plPhysicCoreComponent::kStartTorqueY, _T("StartTorqueY"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_COMP_PHYSICAL_P3ST_EDIT2,IDC_COMP_PHYSICAL_P3ST_SPIN2, 0.1f,
		end,

	plPhysicCoreComponent::kStartTorqueZ, _T("StartTorqueZ"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0f,
		p_ui,	kPhysMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_COMP_PHYSICAL_P3ST_EDIT3,IDC_COMP_PHYSICAL_P3ST_SPIN3, 0.1f,
		end,

	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4,	IDC_RADIO_BSPHERE,	IDC_RADIO_BBOX, IDC_RADIO_BHULL,	IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kCustomBoundListStuff, _T("UserBoundChoice"),	TYPE_INODE,		0, 0,
		p_ui,	kPhysMain, TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PICKSTATE_BASE,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		p_accessor, &gPhysCoreAccessor,
		end,

	plPhysicCoreComponent::kCustomBoundField, _T("UserBoundCheckBx"),		TYPE_BOOL,		0,		0,
		p_default, false,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_CUSTOMCHK,
		p_enable_ctrls, 1, plPhysicCoreComponent::kCustomBoundListStuff,
		end,

	plPhysicCoreComponent::kLOSChkBx, _T("LOSChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_CHECK_LOS,
		end,

	plPhysicCoreComponent::kAlignProxyShape, _T("AlignShapeChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_ALIGN_SHAPE_BOOL,
		end,

	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kReportGroups, _T("memberGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kGroup, _T("group"), TYPE_INT,	0,0,
		end,

	end
);

plPhysDebugComponent::plPhysDebugComponent()
{
	fClassDesc = &gPhysDebugDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plPhysDebugComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	if (fCompPB->GetInt(kCustomBoundField))
	{
		INode* boundsNode = fCompPB->GetINode(kCustomBoundListStuff);
		if( boundsNode )
			nonDrawables.Append(1, &boundsNode);
	}
}


hsBool plPhysDebugComponent::SetupProperties(plMaxNode *node, plErrorMsg *errMsg)
{
	IFixBounds();

	plPhysicalProps *physProps = node->GetPhysicalProps();

	physProps->SetMass(fCompPB->GetFloat(kMass), node, errMsg);

	physProps->SetRestitution(fCompPB->GetFloat(kBounce), node, errMsg);
	physProps->SetFriction(fCompPB->GetFloat(kFriction), node, errMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), node, errMsg);
	if(fCompPB->GetInt(kLOSChkBx))
		physProps->SetLOSBlockCamera(true, node, errMsg);		// *** we really need to have four check boxes now....
	physProps->SetPinned(false, node, errMsg);
	physProps->SetAlignToOwner(fCompPB->GetInt(kAlignProxyShape) != 0, node, errMsg);


	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD), node, errMsg);
	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups_DEAD), node, errMsg);
	physProps->SetReportGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kReportGroups), node, errMsg);

	return IGetProxy(node, errMsg);
}
*/

/////////////////////////////////////////////////////////////////////////////////////////
//
// plPhysTerrainComponent
//
/////////////////////////////////////////////////////////////////////////////////////////

class plPhysTerrainComponent : public plPhysicCoreComponent
{
public:
	plPhysTerrainComponent();
	hsBool SetupProperties(plMaxNode* node,plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables);
};


CLASS_DESC(plPhysTerrainComponent, gPhysTerrainDesc, "Terrain", "Terrain", COMP_TYPE_PHYSICAL, PHYSICS_TERRAIN_CID)


ParamBlockDesc2 gPhysTerrainBk
(	
	plComponent::kBlkComp, _T("Terrain"), 0, &gPhysTerrainDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_TERRAIN, IDS_COMP_PHYS_TERRAIN, 0, 0, NULL,
//	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
//	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,

	// params

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,
	
	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT2, IDC_COMP_PHYS_TERRAIN_SPIN2, 0.0001f,
		end,
	
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kCustomBoundListStuff, _T("UserBoundChoice"),	TYPE_INODE,		0, 0,
		p_ui,	kPhysMain, TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PICKSTATE_TERRAIN,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_TERRAIN,
		p_accessor, &gPhysCoreAccessor,
		end,

	plPhysicCoreComponent::kCustomBoundField, _T("UserBoundCheckBx"),		TYPE_BOOL,		0,		0,
		p_default, false,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_CUSTOMCHK,
		p_enable_ctrls, 1, plPhysicCoreComponent::kCustomBoundListStuff,
		end,

	plPhysicCoreComponent::kAlignProxyShape, _T("AlignShapeChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_ALIGN_SHAPE_BOOL,
		end,

	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,

	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plPhysTerrainComponent::plPhysTerrainComponent()
{
	fClassDesc = &gPhysTerrainDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plPhysTerrainComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	if (fCompPB->GetInt(kCustomBoundField))
	{
		INode* boundsNode = fCompPB->GetINode(kCustomBoundListStuff);
		if( boundsNode )
			nonDrawables.Append(1, &boundsNode);
	}
}

void ValidateGroups(IParamBlock2* pb, int memberID, int bounceID, plComponent* comp, plErrorMsg* pErrMsg)
{
	UInt32 defMember = pb->GetParamDef(memberID).def.i;
	UInt32 member = pb->GetInt(memberID);

	UInt32 defCollide = pb->GetParamDef(bounceID).def.i;
	UInt32 collide = pb->GetInt(bounceID);

	if (defMember != member || defCollide != collide)
	{
		pErrMsg->Set(true,
			"Physics Conflict",
			"The legacy physical component \"%s\" has non-default member or collide groups.\nPlease recreate it.",
			comp->GetINode()->GetName()).Show();
		pErrMsg->Set(false);
	}
}

hsBool plPhysTerrainComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);

	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);
	physProps->SetRestitution(fCompPB->GetFloat(kBounce), pNode, pErrMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), pNode, pErrMsg);
	physProps->SetAlignToOwner(fCompPB->GetInt(kAlignProxyShape) != 0, pNode, pErrMsg);

	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupStatic, pNode, pErrMsg);

	physProps->SetLOSBlockCamera(true, pNode, pErrMsg);
	physProps->SetLOSBlockUI(true, pNode, pErrMsg);
	physProps->SetLOSAvatarWalkable(true, pNode, pErrMsg);

	return IGetProxy(pNode, pErrMsg);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plProxyTerrainComponent
//
/////////////////////////////////////////////////////////////////////////////////////////

class plProxyTerrainComponent : public plPhysicCoreComponent
{
public:
	plProxyTerrainComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plProxyTerrainComponent, gProxyTerrainDesc, "Proxy Terrain", "ProxyTerrain", COMP_TYPE_PHYSICAL, PHYSICS_INVISIBLE_CID)

ParamBlockDesc2 gPhysInvisibleBk
(	
	plComponent::kBlkComp, _T("ProxyTerrain"), 0, &gProxyTerrainDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_PROXY_TERRAIN, IDS_COMP_PHYS_PROXY_TERRAIN, 0, 0, NULL,
// 	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
// 	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,

	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kUILOSChkBx, _T("UILOSChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_CHECK_LOS,
		end,

	plPhysicCoreComponent::kCamLOSChkBx, _T("CamLOSChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_CHECK_LOS2,
		end,

	plPhysicCoreComponent::kCamAvoidChkBx, _T("CamLOSChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_CHECK_LOS3,
		end,
		
	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_INVIS_BOUNCE_EDIT, IDC_COMP_PHYS_INVIS_BOUNCE_SPIN, 0.0001f,
		end,

	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plProxyTerrainComponent::plProxyTerrainComponent()
{
	fClassDesc = &gProxyTerrainDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plProxyTerrainComponent::SetupProperties(plMaxNode *node, plErrorMsg *errMsg)
{
	IFixBounds();
	node->SetDrawable(false);

	plPhysicalProps *physProps = node->GetPhysicalProps();

	physProps->SetMass(0, node, errMsg);
	
	if (fCompPB->GetInt(kUILOSChkBx) || fCompPB->GetInt(kCamLOSChkBx))
	{
// XXX		physProps->SetAllowLOS(false, pNode, pErrMsg);

	
		if (fCompPB->GetInt(kUILOSChkBx))
			physProps->SetLOSBlockUI(true, node, errMsg);
		if (fCompPB->GetInt(kCamLOSChkBx))
			physProps->SetLOSBlockCamera(true, node, errMsg);
		if (fCompPB->GetInt(kCamAvoidChkBx))
			physProps->SetCameraAvoidFlag(true, node, errMsg);
	}
	
	physProps->SetLOSAvatarWalkable(true, node, errMsg);
	
	physProps->SetFriction(fCompPB->GetFloat(kFriction), node, errMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), node, errMsg);
	
	physProps->SetGroup(plSimDefs::kGroupStatic, node, errMsg);
	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, errMsg);
// 	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), node, errMsg);
// 	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), node, errMsg);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plCameraBlockerComponent
//
/////////////////////////////////////////////////////////////////////////////////////////
class plCameraBlockerComponent : public plPhysicCoreComponent
{
public:
	plCameraBlockerComponent();

	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plCameraBlockerComponent, gCameraBlockerDesc, "(ex) Camera Blocker", "CameraBlocker", COMP_TYPE_PHYSICAL, PHYS_CAMERA_BLOCK_CID)

ParamBlockDesc2 gCameraOccludeBlock
(
	plComponent::kBlkComp, _T("cameraBlocker"), 0, &gCameraBlockerDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_CAMERA_OCCLUDER, IDS_CAMERA_OCCLUDERS, 0, 0, NULL,
// 	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
// 	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,

	plPhysicCoreComponent::kBoundCondRadio, _T("BlockerBounds"),	TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,		plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plCameraBlockerComponent::plCameraBlockerComponent()
{
	fClassDesc = &gCameraBlockerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plCameraBlockerComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);
	physProps->SetFriction(0, pNode, pErrMsg);
//	physProps->SetAllowLOS(false, pNode, pErrMsg);
	physProps->SetLOSBlockCamera(true, pNode, pErrMsg);
//	physProps->SetUILOSFlag(true, pNode, pErrMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), pNode, pErrMsg);

	physProps->SetGroup(plSimDefs::kGroupLOSOnly, pNode, pErrMsg);
	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
// 	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
// 	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plPhysSimpleComponent
//
/////////////////////////////////////////////////////////////////////////////////////////
class plPhysSimpleComponent : public plPhysicCoreComponent
{
public:
	plPhysSimpleComponent();
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables);
};

CLASS_DESC(plPhysSimpleComponent, gPhysSimpleDesc, "Simple", "Simple", COMP_TYPE_PHYSICAL, PHYSICS_SIMPLE_CID)

ParamBlockDesc2 gPhysSimpleBk
(	
	plComponent::kBlkComp, _T("Simple"), 0, &gPhysSimpleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_SIMPLE, IDS_COMP_PHYS_SIMPLE, 0, 0, NULL,
// 	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
// 	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,

	// params
	plPhysicCoreComponent::kMass,		_T("Mass"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 1.0,
		p_range, 0.001, 500.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_SIMP_EDIT1, IDC_COMP_PHYS_SIMP_SPIN1, 1.0,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_SIMP_EDIT2, IDC_COMP_PHYS_SIMP_SPIN2, 0.1,
		end,

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_SIMP_EDIT3, IDC_COMP_PHYS_SIMP_SPIN3, 0.1,
		end,

	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kCustomBoundListStuff, _T("UserBoundChoice"),	TYPE_INODE,		0, 0,
		p_ui,	kPhysMain, TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PICKSTATE_SIMP,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_SIMP,
		p_accessor, &gPhysCoreAccessor,
		end,

	plPhysicCoreComponent::kCustomBoundField, _T("UserBoundCheckBx"),		TYPE_BOOL,		0,		0,
		p_default, false,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_CUSTOMCHK,
		p_enable_ctrls, 1, plPhysicCoreComponent::kCustomBoundListStuff,
		end,

	plPhysicCoreComponent::kAlignProxyShape, _T("AlignShapeChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_ALIGN_SHAPE_BOOL,
		end,
		
	plPhysicCoreComponent::kNoSynchronize, _T("DontSynchronizeChkBx"),	TYPE_BOOL,	0,	0,
		p_default, false,
		p_ui,	kPhysMain,	TYPE_SINGLECHEKBOX, IDC_PH_NO_SYNC_CHK,
		end,

	plPhysicCoreComponent::kStartInactive, _T("StartInactiveChkBx"),	TYPE_BOOL, 0, 0,
		p_default, true,
		p_ui,	kPhysMain,	TYPE_SINGLECHEKBOX, IDC_PH_INACTIVE_CHK,
		end,
		
	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kDynamicSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
 		p_default,  plPhysicsGroups_DEAD::kStaticSimulated |
 					plPhysicsGroups_DEAD::kDynamicSimulated |
 					plPhysicsGroups_DEAD::kAnimated,
		end,

	plPhysicCoreComponent::kAvAnimPushable, _T("AvAnimPushable"),	TYPE_BOOL,	0,	0,
		p_default, false,
		p_ui,	kPhysMain,	TYPE_SINGLECHEKBOX, IDC_PH_AVANIMPUSHABLE,
		end,
		
	end
);	

plPhysSimpleComponent::plPhysSimpleComponent()
{
	fClassDesc = &gPhysSimpleDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plPhysSimpleComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	if (fCompPB->GetInt(kCustomBoundField))
	{
		INode* boundsNode = fCompPB->GetINode(kCustomBoundListStuff);
		if( boundsNode )
			nonDrawables.Append(1, &boundsNode);
	}
}

hsBool plPhysSimpleComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(fCompPB->GetFloat(kMass), pNode, pErrMsg);
	physProps->SetRestitution(fCompPB->GetFloat(kBounce), pNode, pErrMsg);
	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), pNode, pErrMsg);
	physProps->SetAlignToOwner(fCompPB->GetInt(kAlignProxyShape) != 0, pNode, pErrMsg);

	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupDynamic, pNode, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);

	physProps->SetNoSynchronize(fCompPB->GetInt(kNoSynchronize));
	physProps->SetStartInactive(fCompPB->GetInt(kStartInactive));
	physProps->SetAvAnimPushable(fCompPB->GetInt(kAvAnimPushable));

	if( !pNode->IsAnimated() )
		pNode->SetItinerant(true);

	return IGetProxy(pNode, pErrMsg);
}


////////////////////////////////////////////////////////////////////////////////
//!  Invisible Blocker Component Class

class plPhysBlockerComponent : public plPhysicCoreComponent
{
public:
	plPhysBlockerComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plPhysBlockerComponent, gPhysBlockerDesc, "(ex) Invisible Blocker",  "InvisBlocker", COMP_TYPE_PHYSICAL, PHYSICS_BLOCKER_CID)

ParamBlockDesc2 gPhysBlockerBk
(	
	plComponent::kBlkComp, _T("invisibleBlocker"), 0, &gPhysBlockerDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_INVISIBLE, IDS_COMP_PHYS_INVISIBLE, 0, 0, NULL,
// 	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
// 	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,

	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kLOSChkBx, _T("LOSChkBx"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	kPhysMain, TYPE_SINGLECHEKBOX, IDC_COMP_CHECK_LOS,
		end,

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_INVIS_BOUNCE_EDIT, IDC_COMP_PHYS_INVIS_BOUNCE_SPIN, 0.0001f,
		end,

	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kGroup, _T("group"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 2, IDC_RADIO_BLOCK_AVATAR, IDC_RADIO_BLOCK_DYNAMIC,
		p_vals,		plSimDefs::kGroupAvatarBlocker,		plSimDefs::kGroupDynamicBlocker,
		p_default, plSimDefs::kGroupAvatarBlocker,
		end,

	end
);

plPhysBlockerComponent::plPhysBlockerComponent()
{
	fClassDesc = &gPhysBlockerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysBlockerComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);

	if(fCompPB->GetInt(kLOSChkBx))
	{
		physProps->SetLOSBlockUI(true, pNode, pErrMsg);
		physProps->SetLOSBlockCamera(true, pNode, pErrMsg);
	}

	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);

	physProps->SetGroup(fCompPB->GetInt(kGroup), pNode, pErrMsg);
	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// Walkable

class plPhysWalkableComponent : public plPhysicCoreComponent
{
public:
	plPhysWalkableComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plPhysWalkableComponent, gPhysWalkableDesc, "Walkable", "Walkable", COMP_TYPE_PHYS_TERRAINS, PHYS_WALKABLE_CID)

ParamBlockDesc2 gPhysWalkableBk
(	
	plComponent::kBlkComp, _T("Walkable"), 0, &gPhysWalkableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_INVISIBLE, IDS_COMP_PHYS_WALKABLE, 0, 0, NULL,
//	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
//	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
	
	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,


	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_INVIS_BOUNCE_EDIT, IDC_COMP_PHYS_INVIS_BOUNCE_SPIN, 0.0001f,
		end,
	
	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plPhysWalkableComponent::plPhysWalkableComponent()
{
	fClassDesc = &gPhysWalkableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysWalkableComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);
	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);
	physProps->SetRestitution(fCompPB->GetFloat(kBounce), pNode, pErrMsg);

	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupStatic, pNode, pErrMsg);
//	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, pNode, pErrMsg);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Climable
class plPhysClimbableComponent : public plPhysicCoreComponent
{
public:
	plPhysClimbableComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plPhysClimbableComponent, gPhysClimbableDesc, "Climbable", "Climbable", COMP_TYPE_PHYS_TERRAINS, PHYS_CLIMBABLE_CID)

ParamBlockDesc2 gPhysClimbableBk
(	
	plComponent::kBlkComp, _T("Climbable"), 0, &gPhysClimbableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_CLIMBABLE, IDS_COMP_PHYS_CLIMBABLE, 0, 0, NULL,
//	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
//	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
	
	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_INVIS_BOUNCE_EDIT, IDC_COMP_PHYS_INVIS_BOUNCE_SPIN, 0.0001f,
		end,
	
	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plPhysClimbableComponent::plPhysClimbableComponent()
{
	fClassDesc = &gPhysClimbableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysClimbableComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);
	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);
	physProps->SetRestitution(fCompPB->GetFloat(kBounce), pNode, pErrMsg);

	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupStatic, pNode, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);
//	physProps->SetReportGroup(1<<plSimDefs::kAvatars, pNode, pErrMsg);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plSwim2DComponent
//
/////////////////////////////////////////////////////////////////////////////////////////

class Swim2DDlgProc : public ParamMap2UserDlgProc
{
public:
	Swim2DDlgProc() {}
	~Swim2DDlgProc() {}
	
	void IValidateSpinners(TimeValue t, IParamBlock2 *pb, IParamMap2 *map, UInt32 id)
	{
		UInt32 minIndex, maxIndex;
		hsBool adjustMin;
		switch(id)
		{
		case IDC_SWIM_CURRENT_PULL_NEAR_DIST:
		case IDC_SWIM_CURRENT_PULL_NEAR_DIST_SPIN:
			minIndex = plPhysicCoreComponent::kSwimCurrentPullNearDist; maxIndex = plPhysicCoreComponent::kSwimCurrentPullFarDist; adjustMin = false;
			break;

		case IDC_SWIM_CURRENT_PULL_FAR_DIST:
		case IDC_SWIM_CURRENT_PULL_FAR_DIST_SPIN:
			minIndex = plPhysicCoreComponent::kSwimCurrentPullNearDist; maxIndex = plPhysicCoreComponent::kSwimCurrentPullFarDist; adjustMin = true;
			break;

		case IDC_SWIM_CURRENT_STRAIGHT_NEAR_DIST:
		case IDC_SWIM_CURRENT_STRAIGHT_NEAR_DIST_SPIN:
			minIndex = plPhysicCoreComponent::kSwimCurrentStraightNearDist; maxIndex = plPhysicCoreComponent::kSwimCurrentStraightFarDist; adjustMin = false;
			break;

		case IDC_SWIM_CURRENT_STRAIGHT_FAR_DIST:
		case IDC_SWIM_CURRENT_STRAIGHT_FAR_DIST_SPIN:
			minIndex = plPhysicCoreComponent::kSwimCurrentStraightNearDist; maxIndex = plPhysicCoreComponent::kSwimCurrentStraightFarDist; adjustMin = true;
			break;
			
		default:
			return;
		}
		
		float min, max;
		min = pb->GetFloat(minIndex, t);
		max = pb->GetFloat(maxIndex, t);
		
		if (min > max)
		{
			if (adjustMin)
				pb->SetValue(minIndex, t, max);
			else
				pb->SetValue(maxIndex, t, min);
			
			map->Invalidate(minIndex);
			map->Invalidate(maxIndex);
		}
	}
	
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		
		IParamBlock2 *pb = map->GetParamBlock();
		
		switch (msg)
		{
		case WM_COMMAND:  
		case CC_SPINNER_CHANGE:
			IValidateSpinners(t, pb, map, id);
			return TRUE;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static Swim2DDlgProc gSwim2DDlgProc;

CLASS_DESC(plSwim2DComponent, gPhysSwimSurfaceDesc, "Swim 2D", "Swim 2D", COMP_TYPE_PHYS_TERRAINS, PHYS_SWIMSURFACE_CID)

ParamBlockDesc2 gPhysSwimSurfaceBk
(	
	plComponent::kBlkComp, _T("Swim 2D"), 0, &gPhysSwimSurfaceDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_SWIMSURF, IDS_COMP_PHYS_SWIMSURF, 0, 0, &gSwim2DDlgProc,
//	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
//	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
	
	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,		plSimDefs::kSphereBounds,	plSimDefs::kBoxBounds,	plSimDefs::kHullBounds,	plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,
		
	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	plPhysicCoreComponent::kSwimCurrentType, _T("CurrentType"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 3, IDC_SWIM_CURRENT_NONE, IDC_SWIM_CURRENT_SPIRAL, IDC_SWIM_CURRENT_STRAIGHT,
		p_vals,		plSwim2DComponent::kCurrentNone,	plSwim2DComponent::kCurrentSpiral, plSwim2DComponent::kCurrentStraight,
		p_default,	plSwim2DComponent::kCurrentNone,
		end,

	plPhysicCoreComponent::kSwimCurrentRotation, _T("SwimRotation"), TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, -100.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_ROTATION, IDC_SWIM_CURRENT_ROTATION_SPIN, 1.0,
		end,
		
	plPhysicCoreComponent::kSwimDetectorNode, _T("swimDetector"),	TYPE_INODE,		0, 0,
		p_ui,		kPhysMain, TYPE_PICKNODEBUTTON, IDC_SWIM_DETECTOR_NODE,
		p_sclassID,	 GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_SWIM_DETECTOR_NODE,
		end,
		
	plPhysicCoreComponent::kSwimCurrentNode, _T("swimCurrentNode"),	TYPE_INODE,		0, 0,
		p_ui,		kPhysMain, TYPE_PICKNODEBUTTON, IDC_SWIM_CURRENT_NODE,
		//p_sclassID,	 DUMMY_CLASS_ID,
		p_prompt, IDS_SWIM_CURRENT_NODE,
		end,
		
	plPhysicCoreComponent::kSwimBuoyancyDown, _T("BuoyancyDown"), TYPE_FLOAT, 0, 0,	
		p_default, 3.0,
		p_range, 0.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_SWIM_BUOYANCY_DOWN, IDC_SWIM_BUOYANCY_DOWN_SPIN, 0.1,
		end,

	plPhysicCoreComponent::kSwimBuoyancyUp, _T("BuoyancyUp"), TYPE_FLOAT, 0, 0,	
		p_default, 0.05,
		p_range, 0.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_SWIM_BUOYANCY_UP, IDC_SWIM_BUOYANCY_UP_SPIN, 0.1,
		end,
	
	plPhysicCoreComponent::kSwimMaxUpVel, _T("MaxUpVel"), TYPE_FLOAT, 0, 0,	
		p_default, 3.0,
		p_range, 0.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_SWIM_MAX_UP_VEL, IDC_SWIM_MAX_UP_VEL_SPIN, 0.1,
		end,		

	plPhysicCoreComponent::kSwimCurrentPullNearDist, _T("PullNearDist"), TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 10000.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_SWIM_CURRENT_PULL_NEAR_DIST, IDC_SWIM_CURRENT_PULL_NEAR_DIST_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentPullNearVel, _T("PullNearVel"), TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, -100.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_PULL_NEAR_VEL, IDC_SWIM_CURRENT_PULL_NEAR_VEL_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentPullFarDist, _T("PullFarDist"), TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 10000.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_SWIM_CURRENT_PULL_FAR_DIST, IDC_SWIM_CURRENT_PULL_FAR_DIST_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentPullFarVel, _T("PullFarVel"), TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, -100.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_PULL_FAR_VEL, IDC_SWIM_CURRENT_PULL_FAR_VEL_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentStraightNearDist, _T("StraightNearDist"), TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, -10000.0, 10000.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_STRAIGHT_NEAR_DIST, IDC_SWIM_CURRENT_STRAIGHT_NEAR_DIST_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentStraightNearVel, _T("StraightNearVel"), TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, -100.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_STRAIGHT_NEAR_VEL, IDC_SWIM_CURRENT_STRAIGHT_NEAR_VEL_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentStraightFarDist, _T("StraightFarDist"), TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, -10000.0, 10000.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_STRAIGHT_FAR_DIST, IDC_SWIM_CURRENT_STRAIGHT_FAR_DIST_SPIN, 1.0,
		end,

	plPhysicCoreComponent::kSwimCurrentStraightFarVel, _T("StraightFarVel"), TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, -100.0, 100.0,
		p_ui,		kPhysMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_SWIM_CURRENT_STRAIGHT_FAR_VEL, IDC_SWIM_CURRENT_STRAIGHT_FAR_VEL_SPIN, 1.0,
		end,
		
	end
	);

// plSwim2DComponent -----------------
// ------------------
plSwim2DComponent::plSwim2DComponent()
{
	fClassDesc = &gPhysSwimSurfaceDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetUpProperties -----------------------------------------------------------
// ----------------
hsBool plSwim2DComponent::SetupProperties(plMaxNode *node, plErrorMsg *errMsg)
{
	IFixBounds();
	plPhysicalProps *physProps = nil;
	plMaxNode *detectorNode = (plMaxNode *)fCompPB->GetINode(kSwimDetectorNode);
	if (detectorNode)
	{
		detectorNode->SetDrawable(false);
		physProps = detectorNode->GetPhysicalProps();
		physProps->SetMass(0, detectorNode, errMsg);
		ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, errMsg);
		physProps->SetGroup(plSimDefs::kGroupDetector, node, errMsg);
// 		physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), detectorNode, errMsg);
// 		physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), detectorNode, errMsg);
		physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, detectorNode, errMsg);
		physProps->SetBoundsType(plSimDefs::kBoxBounds, detectorNode, errMsg);
	}
	
	physProps = node->GetPhysicalProps();
	physProps->SetMass(0, node, errMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), node, errMsg);
	physProps->SetLOSSwimRegion(true, node, errMsg);
	
	plMaxNode *currentNode = (plMaxNode *)fCompPB->GetINode(kSwimCurrentNode);
	if (currentNode)
		currentNode->SetForceLocal(true);
	
	return true;
}

hsBool plSwim2DComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plSwimRegionInterface *swimInt = nil;
	int type = fCompPB->GetInt(ParamID(kSwimCurrentType));
	if (type != kCurrentNone && fCompPB->GetINode(ParamID(kSwimCurrentNode)) == nil)
	{
		pErrMsg->Set(true, node->GetName(), "No dummy box set to define current. Forcing current to \"none\"").Show();
		type = kCurrentNone;
	}
	
	switch (type)
	{
	case kCurrentSpiral:
		{
			fSwimRegions[node] = TRACKED_NEW plSwimCircularCurrentRegion();
			hsgResMgr::ResMgr()->NewKey(node->GetName(), fSwimRegions[node], node->GetLocation(), node->GetLoadMask());	
			break;
		}
	case kCurrentStraight:
		{
			fSwimRegions[node] = TRACKED_NEW plSwimStraightCurrentRegion();
			hsgResMgr::ResMgr()->NewKey(node->GetName(), fSwimRegions[node], node->GetLocation(), node->GetLoadMask());
			break;
		}
	default:
		{
			fSwimRegions[node] = TRACKED_NEW plSwimRegionInterface();
			hsgResMgr::ResMgr()->NewKey(node->GetName(), fSwimRegions[node], node->GetLocation(), node->GetLoadMask());
			break;
		}
	}
	return true;
}
	
// Convert ------------------------------------------------------------
// --------
hsBool plSwim2DComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plMaxNode *detectorNode = (plMaxNode *)fCompPB->GetINode(kSwimDetectorNode);
	if (detectorNode && detectorNode->GetSceneObject())
	{
		if (!detectorNode->GetSceneObject()->GetModifierByType(plSwimDetector::Index()))
		{
			plKey nilKey;
			plSwimMsg *enterMsg = TRACKED_NEW plSwimMsg(detectorNode->GetKey(), nilKey, true, nil);
			plSwimMsg *exitMsg = TRACKED_NEW plSwimMsg(detectorNode->GetKey(), nilKey, false, nil);
			enterMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
			exitMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
			plSwimDetector *swimMod = TRACKED_NEW plSwimDetector(enterMsg, exitMsg);
			detectorNode->AddModifier(swimMod, IGetUniqueName(node));
			
			// the mod doesn't have a valid key until AddModifier is called, so this comes last.
			enterMsg->fSwimRegionKey = swimMod->GetKey();
			exitMsg->fSwimRegionKey = swimMod->GetKey();
		}
	}
	
	int type = fCompPB->GetInt(ParamID(kSwimCurrentType));
	switch (type)
	{
	case kCurrentSpiral:
		{
			plSwimCircularCurrentRegion *circInt = plSwimCircularCurrentRegion::ConvertNoRef(fSwimRegions[node]);
			circInt->fRotation = fCompPB->GetFloat(ParamID(kSwimCurrentRotation));
			circInt->fPullNearDistSq = fCompPB->GetFloat(ParamID(kSwimCurrentPullNearDist));
			circInt->fPullNearDistSq *= circInt->fPullNearDistSq;
			circInt->fPullNearVel = fCompPB->GetFloat(ParamID(kSwimCurrentPullNearVel));
			circInt->fPullFarDistSq = fCompPB->GetFloat(ParamID(kSwimCurrentPullFarDist));
			circInt->fPullFarDistSq *= circInt->fPullFarDistSq;
			circInt->fPullFarVel = fCompPB->GetFloat(ParamID(kSwimCurrentPullFarVel));
		
			plMaxNode *currentNode = (plMaxNode *)fCompPB->GetINode(ParamID(kSwimCurrentNode));
			if (currentNode)
			{
				plGenRefMsg *msg= TRACKED_NEW plGenRefMsg(circInt->GetKey(), plRefMsg::kOnCreate, -1, -1);
				hsgResMgr::ResMgr()->AddViaNotify(currentNode->GetSceneObject()->GetKey(), msg, plRefFlags::kActiveRef); 
			}
			break;
		}
	case kCurrentStraight:
		{
			plSwimStraightCurrentRegion *strInt = plSwimStraightCurrentRegion::ConvertNoRef(fSwimRegions[node]);
			strInt->fNearDist = fCompPB->GetFloat(ParamID(kSwimCurrentStraightNearDist));
			strInt->fNearVel = fCompPB->GetFloat(ParamID(kSwimCurrentStraightNearVel));
			strInt->fFarDist = fCompPB->GetFloat(ParamID(kSwimCurrentStraightFarDist));
			strInt->fFarVel = fCompPB->GetFloat(ParamID(kSwimCurrentStraightFarVel));
			
			plMaxNode *currentNode = (plMaxNode *)fCompPB->GetINode(ParamID(kSwimCurrentNode));
			if (currentNode)
			{
				plGenRefMsg *msg= TRACKED_NEW plGenRefMsg(strInt->GetKey(), plRefMsg::kOnCreate, -1, -1);
				hsgResMgr::ResMgr()->AddViaNotify(currentNode->GetSceneObject()->GetKey(), msg, plRefFlags::kActiveRef); 
			}
			break;
		}
	default:
		{
			// Already done all the work in PreConvert
			break;
		}
	}

	fSwimRegions[node]->fDownBuoyancy = fCompPB->GetFloat(ParamID(kSwimBuoyancyDown)) + 1;
	fSwimRegions[node]->fUpBuoyancy = fCompPB->GetFloat(ParamID(kSwimBuoyancyUp)) + 1;
	fSwimRegions[node]->fMaxUpwardVel = fCompPB->GetFloat(ParamID(kSwimMaxUpVel));
	
	hsgResMgr::ResMgr()->AddViaNotify(fSwimRegions[node]->GetKey(), TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// Swim 3D

class plPhysSwim3DComponent : public plPhysicCoreComponent
{
public:
	plPhysSwim3DComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plPhysSwim3DComponent, gPhysSwim3DDesc, "Swim 3D", "Swim 3D", COMP_TYPE_PHYS_TERRAINS, PHYS_SWIM3D_CID)

ParamBlockDesc2 gPhysSwim3DBk
(	
	plComponent::kBlkComp, _T("Swim 3D"), 0, &gPhysSwim3DDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1, 
	kPhysMain, IDD_COMP_PHYS_SWIM3D, IDS_COMP_PHYS_SWIM3D, 0, 0, NULL,
//	kPhysMember, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_MEMBER, 0, APPENDROLL_CLOSED, &gMemberGroupProc,
//	kPhysBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
	
	// params
	plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		kPhysMain, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	plPhysicCoreComponent::kFriction,	_T("Friction"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TERRAIN_EDIT1, IDC_COMP_PHYS_TERRAIN_SPIN1, 0.0001f,
		end,

	plPhysicCoreComponent::kBounce,	_T("Bounce"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	kPhysMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_INVIS_BOUNCE_EDIT, IDC_COMP_PHYS_INVIS_BOUNCE_SPIN, 0.0001f,
		end,
	
	// Event Groups
	plPhysicCoreComponent::kMemberGroups_DEAD, _T("memberGroups"), TYPE_INT,	0,0,
		p_default, plPhysicsGroups_DEAD::kStaticSimulated,
		end,
	plPhysicCoreComponent::kBounceGroups_DEAD, _T("bounceGroups"), TYPE_INT,	0,0,
		end,

	end
);

plPhysSwim3DComponent::plPhysSwim3DComponent()
{
	fClassDesc = &gPhysSwim3DDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysSwim3DComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	IFixBounds();

	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(0, pNode, pErrMsg);
	physProps->SetFriction(fCompPB->GetFloat(kFriction), pNode, pErrMsg);
	physProps->SetRestitution(fCompPB->GetFloat(kBounce), pNode, pErrMsg);

	ValidateGroups(fCompPB, plPhysicCoreComponent::kMemberGroups_DEAD, plPhysicCoreComponent::kBounceGroups_DEAD, this, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupStatic, pNode, pErrMsg);
//	physProps->SetMemberGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kMemberGroups), pNode, pErrMsg);
//	physProps->SetBounceGroup(plEventGroupProc::GetGroups(fCompPB, plPhysicCoreComponent::kBounceGroups), pNode, pErrMsg);
//	physProps->SetReportGroup(plPhysicsGroups_DEAD::kAvatars, pNode, pErrMsg);

	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
// DEAD
//
/////////////////////////////////////////////////////////////////////////////////////////
class plPhysPlayerComponent : public plPhysicCoreComponent
{
public:
	plPhysPlayerComponent();
};

OBSOLETE_CLASS_DESC(plPhysPlayerComponent, gPhysPlayerDesc, "Player", "Player", COMP_TYPE_PHYSICAL, PHYSICS_PLAYER_CID)

ParamBlockDesc2 gPhysPlayerBk
(	
	plComponent::kBlkComp, _T("Player"), 0, &gPhysPlayerDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

	end
);

plPhysPlayerComponent::plPhysPlayerComponent()
{
	fClassDesc = &gPhysPlayerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////

class plPhysBoundBlockerComponent : public plPhysicCoreComponent
{
public:
	plPhysBoundBlockerComponent();
};

OBSOLETE_CLASS_DESC(plPhysBoundBlockerComponent, gPhysBoundBlockerDesc, "(ex) Boundary Blocker",  "BoundaryBlocker", COMP_TYPE_PHYSICAL, PHYSICS_BOUND_BLOCKER_CID)

ParamBlockDesc2 gPhysBoundBlockerBk
(	
	plComponent::kBlkComp, _T("(ex)Boundary Blocker"), 0, &gPhysBoundBlockerDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

	end
);

plPhysBoundBlockerComponent::plPhysBoundBlockerComponent()
{
	fClassDesc = &gPhysBoundBlockerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////

class plPhysDetectorComponent : public plPhysicCoreComponent
{
public:
	plPhysDetectorComponent();
};

OBSOLETE_CLASS_DESC(plPhysDetectorComponent, gPhysDetectorDesc, "Detector", "Detector", COMP_TYPE_PHYSICAL, PHYSICS_DETECTOR_CID)

ParamBlockDesc2 gPhysDetectorBk
(	
	plComponent::kBlkComp, _T("Detector"), 0, &gPhysDetectorDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

	end
);

plPhysDetectorComponent::plPhysDetectorComponent()
{
	fClassDesc = &gPhysDetectorDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

////////////////////////////////////////////////////////////////////////////////
//!  Physical Simple Component Class 

class plPhysSubWorldComponent : public plPhysicCoreComponent
{
public:
	plPhysSubWorldComponent();
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	void IAddChildren(plMaxNode *node, plMaxNode* worldKey);
};

CLASS_DESC(plPhysSubWorldComponent, gPhysSubWorldDesc, "Subworld", "Subworld", COMP_TYPE_PHYSICAL, PHYS_SUBWORLD_CID)

ParamBlockDesc2 gPhysSubWorldBk
(	
	plComponent::kBlkComp, _T("Subworld	"), 0, &gPhysSubWorldDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PHYS_SUBWORLD, IDS_COMP_PHYS_SUBWORLD, 0, 0, NULL,

	end
);	

plPhysSubWorldComponent::plPhysSubWorldComponent()
{
	fClassDesc = &gPhysSubWorldDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysSubWorldComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	IAddChildren(node, node);

	node->SetForceLocal(true);
	node->SetMovable(true);

	return true;
}

hsBool plPhysSubWorldComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

void plPhysSubWorldComponent::IAddChildren(plMaxNode* node, plMaxNode* subworld)
{
	int numChildren = node->NumberOfChildren();
	for (int i = 0; i < numChildren; i++)
	{
		plMaxNode* child = (plMaxNode*)node->GetChildNode(i);
		const char* childName = child->GetName();

		bool hasSubworld = false;

		UInt32 numComps = child->NumAttachedComponents();
		for (int j = 0; j < numComps; j++)
		{
			plComponentBase* comp = child->GetAttachedComponent(j);
			if (comp && comp->ClassID() == PHYS_SUBWORLD_CID)
			{
				hasSubworld = true;
				break;
			}
		}

		// If we've reached another subworld, terminate our descent so we don't file
		// children in our subworld instead.
		// This has to happen *after* we've converted this node, because it may be a physical
		// in the parent subworld.
		if (!hasSubworld)
		{
			plPhysicalProps* props = child->GetPhysicalProps();
			if (props)
				props->SetSubworld(subworld);
			IAddChildren(child, subworld);
		}
	}
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// the detector for triggering subworld changes
///////////////////////////////////////////////////////

class plSubworldDetectorComponent : public plPhysicCoreComponent
{
public:
	plSubworldDetectorComponent();

	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};



CLASS_DESC(plSubworldDetectorComponent, gSubworldDetectorDesc, "Subworld Region",  "SubworldRegion", COMP_TYPE_PHYSICAL, SUBWORLD_REGION_CID)

enum
{
	kSubworldTarget,
	kInclusive_DEAD,
	kSubworldTriggerOn,
};

enum
{
	kSubTriggerOnEnter,
	kSubTriggerOnExit,
};

ParamBlockDesc2 gSubworldRegionBlock
(
	plComponent::kBlkComp, _T("subworldRegion"), 0, &gSubworldDetectorDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SUBWORLD_REGION, IDS_COMP_SUBWORLD_REGION, 0, 0, NULL,

	kSubworldTarget, _T("SubworldTarget"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CAMERARGN_PICKSTATE_BASE,
		p_sclassID,	 HELPER_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,

	kSubworldTriggerOn, _T("triggerOn"),	TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 2, IDC_RADIO_ENTER, IDC_RADIO_EXIT,
		p_vals,		kSubTriggerOnEnter, kSubTriggerOnExit,
		p_default,	kSubTriggerOnEnter,
		end,

	end
);

plSubworldDetectorComponent::plSubworldDetectorComponent()
{
	fClassDesc = &gSubworldDetectorDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSubworldDetectorComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	pNode->SetForceLocal(true);
 	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(1.0, pNode, pErrMsg);
	physProps->SetFriction(0.0, pNode, pErrMsg);
	physProps->SetRestitution(0.0, pNode, pErrMsg);
	//physProps->SetBoundsType(plSimDefs::kExplicitBounds, pNode, pErrMsg);
	physProps->SetBoundsType(plSimDefs::kHullBounds, pNode, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupDetector, pNode, pErrMsg);
	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, pNode, pErrMsg);

	return true;
}

hsBool plSubworldDetectorComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plSubworldDetectorComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plSceneObject *obj = node->GetSceneObject();
	plLocation loc = node->GetLocation();
	
	plSubworldRegionDetector *detector = TRACKED_NEW plSubworldRegionDetector;
	
	// Register the detector
	plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
	hsgResMgr::ResMgr()->AddViaNotify(detectorKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	// need to get the key for the camera here...
	plMaxNode* pSubNode = (plMaxNode*)fCompPB->GetINode(kSubworldTarget);
	if (pSubNode)
	{	
		if(pSubNode->CanConvert())
		{
			detector->SetSubworldKey(pSubNode->GetKey());
		}
	}
	else
	{
		detector->SetSubworldKey(nil);
	}

	bool onExit = (fCompPB->GetInt(kSubworldTriggerOn) == kSubTriggerOnExit);
	detector->SetTriggerOnExit(onExit);

	return true;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// the detector for triggering panic links
///////////////////////////////////////////////////////

class plPanicLinkDetectorComponent : public plPhysicCoreComponent
{
public:
	enum
	{
		kPlayAnim,
	};

	plPanicLinkDetectorComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};


CLASS_DESC(plPanicLinkDetectorComponent, gPanicLinkDetectorDesc, "Panic Link Region",  "PanicLinkRegion", COMP_TYPE_PHYSICAL, PANIC_LINK_REGION_CID)

ParamBlockDesc2 gPanicLinkRegionBlock
(
	plComponent::kBlkComp, _T("panicLinkRegion"), 0, &gPanicLinkDetectorDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_PANIC, IDS_COMP_PANIC, 0, 0, NULL,
	
	plPanicLinkDetectorComponent::kPlayAnim, _T("PlayAnim"),		TYPE_BOOL,		0,		0,
	p_default, true,
	p_ui, TYPE_SINGLECHEKBOX, IDC_COMP_PANIC_ANIM,
	end,
	
	end
);


plPanicLinkDetectorComponent::plPanicLinkDetectorComponent()
{
	fClassDesc = &gPanicLinkDetectorDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPanicLinkDetectorComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	pNode->SetForceLocal(true);
	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(1.0, pNode, pErrMsg);
	physProps->SetFriction(0.0, pNode, pErrMsg);
	physProps->SetRestitution(0.0, pNode, pErrMsg);
	physProps->SetBoundsType(plSimDefs::kHullBounds, pNode, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupDetector, pNode, pErrMsg);
	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, pNode, pErrMsg);

	return true;
}

hsBool plPanicLinkDetectorComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plPanicLinkDetectorComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plSceneObject *obj = node->GetSceneObject();
	plLocation loc = node->GetLocation();
	
	plPanicLinkRegion *detector = TRACKED_NEW plPanicLinkRegion;
	detector->fPlayLinkOutAnim = fCompPB->GetInt(ParamID(kPlayAnim));
	
	// Register the detector
	plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
	hsgResMgr::ResMgr()->AddViaNotify(detectorKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Shootable COMPONENT
//
/////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plShootableComponent : public plComponent
{
public:
	plShootableComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plShootableComponent, gShootableDesc, "Shootable",  "Shootable", COMP_TYPE_PHYSICAL, Class_ID(0x77b94f4a, 0x6d3851fc))

ParamBlockDesc2 gShootableBk
(
//	1, _T(""), 0, &gShootableDesc, P_AUTO_CONSTRUCT, 0,
	plComponent::kBlkComp, _T("Shootable"), 0, &gShootableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SHOOTABLE, IDS_COMP_SHOOTABLE, 0, 0, NULL,

	end
);

plShootableComponent::plShootableComponent()
{
	fClassDesc = &gShootableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plShootableComponent::SetupProperties(plMaxNode *node,  plErrorMsg *errMsg)
{
	plPhysicalProps *props = node->GetPhysicalProps();

	props->SetMass(0.0, node, errMsg);
	props->SetFriction(0.0, node, errMsg);
	props->SetRestitution(0.0, node, errMsg);
	props->SetBoundsType(plSimDefs::kExplicitBounds, node, errMsg);
	props->SetGroup(plSimDefs::kGroupLOSOnly, node, errMsg);
	props->SetPinned(true, node, errMsg);
	props->SetLOSShootable(true, node, errMsg);

	return true;
}

hsBool plShootableComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}
class plRideAnimatedPhysicalComponent : public plPhysicCoreComponent
{
public:
	plRideAnimatedPhysicalComponent();
	void DeleteThis(){delete this;}		
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};
CLASS_DESC(plRideAnimatedPhysicalComponent , gRideAnimatedPhysicalComponent, "RideAnimPhysReg",  "RideAnimatedPhysicalRegion", COMP_TYPE_PHYSICAL, Class_ID(0xaf305963, 0x63a246df));
ParamBlockDesc2 gSRideAnimatedPhysBk
(
plComponent::kBlkComp, _T("rideAnimated"), 0, &gRideAnimatedPhysicalComponent, P_AUTO_CONSTRUCT + P_AUTO_UI , plComponent::kRefComp,
//Roll out

IDD_COMP_RIDE_ANIMATED_PHYS, IDS_COMP_RIDE_ANIMATED_PHYS, 0, 0, NULL,

plPhysicCoreComponent::kBoundCondRadio, _T("BoundingConditions"), TYPE_INT, 0, 0,
	p_ui, TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
	p_vals,	plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kExplicitBounds,
	p_default, plSimDefs::kHullBounds,	
	end,
	end
);
plRideAnimatedPhysicalComponent::plRideAnimatedPhysicalComponent()
{
	fClassDesc = &gRideAnimatedPhysicalComponent;
	fClassDesc->MakeAutoParamBlocks(this);
}
hsBool plRideAnimatedPhysicalComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	pNode->SetForceLocal(true);
	pNode->SetDrawable(false);

	plPhysicalProps *physProps = pNode->GetPhysicalProps();

	physProps->SetMass(1.0, pNode, pErrMsg);
	physProps->SetFriction(0.0, pNode, pErrMsg);
	physProps->SetRestitution(0.0, pNode, pErrMsg);
	physProps->SetGroup(plSimDefs::kGroupDetector, pNode, pErrMsg);
	physProps->SetReportGroup(1<<plSimDefs::kGroupAvatar, pNode, pErrMsg);
	physProps->SetBoundsType(fCompPB->GetInt(kBoundCondRadio), pNode, pErrMsg);
	return true;
}
hsBool plRideAnimatedPhysicalComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plRideAnimatedPhysicalComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plSceneObject *obj = node->GetSceneObject();
	plLocation loc = node->GetLocation();
	plRideAnimatedPhysMsg* enter = TRACKED_NEW plRideAnimatedPhysMsg(obj->GetKey(), nil, true, nil);
	enter->SetBCastFlag(plMessage::kPropagateToModifiers);
	plRideAnimatedPhysMsg* exit = TRACKED_NEW plRideAnimatedPhysMsg(obj->GetKey(), nil, false, nil);
	exit->SetBCastFlag(plMessage::kPropagateToModifiers);
	plRidingAnimatedPhysicalDetector *detector = TRACKED_NEW plRidingAnimatedPhysicalDetector(enter, exit);
	// Register the detector
	//node->AddModifier(detector, IGetUniqueName(node));
	plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
	hsgResMgr::ResMgr()->AddViaNotify(detectorKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
	return true;
}
