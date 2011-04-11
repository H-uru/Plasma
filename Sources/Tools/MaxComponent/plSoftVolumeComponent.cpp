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
#include "dummy.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "plMiscComponents.h"
#include "plSoftVolumeComponent.h"

#include "../MaxMain/plPlasmaRefMsgs.h"
#include "../MaxMain/plMaxNode.h"

#include "../plIntersect/plSoftVolumeTypes.h"
#include "../plIntersect/plVolumeIsect.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plObjRefMsg.h"
#include "hsResMgr.h"

#include "../plGLight/plLightInfo.h"
#include "../plScene/plOccluder.h"

#include "../pnSceneObject/plDrawInterface.h"
#include "../plScene/plVisRegion.h"
#include "../plScene/plRelevanceRegion.h"

void DummyCodeIncludeFuncSoftVolume() {}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
class plVolumeHitCallback : public HitByNameDlgCallback
{
protected:
	INode*			fOwner;
	IParamBlock2*	fPB;
	ParamID			fNodeListID;
	BOOL			fSingleSel;
	TCHAR			fTitle[ 128 ];

public:
	plVolumeHitCallback(INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title = nil, BOOL singleSel=false );

	virtual TCHAR *dialogTitle() { return fTitle; }
	virtual TCHAR *buttonText() { return "OK"; }
	virtual int filter(INode *node);
	virtual void proc(INodeTab &nodeTab);
	virtual BOOL showHiddenAndFrozen() { return TRUE; }
	virtual BOOL singleSelect() { return fSingleSel; }
};

plVolumeHitCallback::plVolumeHitCallback(INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title, BOOL singleSel)
:	fOwner(owner),
	fPB(pb),
	fNodeListID(nodeListID),
	fSingleSel(singleSel)
{
	strcpy( fTitle, title );
}

int plVolumeHitCallback::filter(INode *node)
{
	if( node == fOwner )
		return FALSE;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();

	// If this is an activator type component
	if( comp )
	{
		if( (comp->ClassID() == SOFTVOLUME_CID)
			|| (comp->ClassID() == SOFTVOLUME_UNION_CID)
			|| (comp->ClassID() == SOFTVOLUME_ISECT_CID)
			|| (comp->ClassID() == SOFTVOLUME_NEGATE_CID) )
		{

			if( !fSingleSel )
			{
				// And we don't already reference it
				int i;
				for( i = 0; i < fPB->Count(fNodeListID); i++ )
				{
					if( fPB->GetINode(fNodeListID, 0, i) == node )
						return FALSE;
				}
			}

			// And this wouldn't create a cyclical reference (Max doesn't like those)
			if (comp->TestForLoop(FOREVER, fPB) == REF_FAIL)
				return FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

void plVolumeHitCallback::proc(INodeTab &nodeTab)
{
	if( fSingleSel )
	{
		if( nodeTab.Count() )
			fPB->SetValue(fNodeListID, TimeValue(0), nodeTab[0]);
		else
			fPB->SetValue(fNodeListID, TimeValue(0), (INode*)nil);
	}
	else
		fPB->Append(fNodeListID, nodeTab.Count(), &nodeTab[0]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plSingleCompSelProc Functions //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

plSingleCompSelProc::plSingleCompSelProc(ParamID nodeID, int dlgItem, TCHAR *title) : fNodeID(nodeID), fDlgItem(dlgItem) 
{
	strcpy( fTitle, title );
}

BOOL plSingleCompSelProc::DlgProc(TimeValue t, IParamMap2 *paramMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			IParamBlock2 *pb = paramMap->GetParamBlock();
			INode* node = pb->GetINode(fNodeID);
			TSTR newName(node ? node->GetName() : "Pick");
			::SetWindowText(::GetDlgItem(hWnd, fDlgItem), newName);
		}
		return true;

	case WM_COMMAND:
		if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == fDlgItem) )
		{
			// Adding a volume.  Set it and refresh the UI to show it in our list.
			IParamBlock2 *pb = paramMap->GetParamBlock();
			plVolumeHitCallback hitCB((INode*)pb->GetOwner(), pb, fNodeID, fTitle, true );
			GetCOREInterface()->DoHitByNameDialog(&hitCB);
			INode* node = pb->GetINode(fNodeID);
			TSTR newName(node ? node->GetName() : "Pick");
			::SetWindowText(::GetDlgItem(hWnd, fDlgItem), newName);
			paramMap->Invalidate(fNodeID);
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(hWnd, SW_SHOW);

			return false;
		}
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// Moved class declaration to .h -mcn


hsBool plSoftVolBaseComponent::SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg) 
{ 
	fSoftKey = nil; 
	fValid = false; 
	return true; 
}

plKey plSoftVolBaseComponent::GetSoftVolume()
{ 
	if( !fSoftKey )
		ICreateSoftVolume(); 
	return fSoftKey; 
}

plSoftVolBaseComponent* plSoftVolBaseComponent::GetSoftComponent(INode* node)
{
	if( node == nil )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( comp == nil )
		return nil;

	return GetSoftComponent( comp );
}

plSoftVolBaseComponent* plSoftVolBaseComponent::GetSoftComponent(plComponentBase *comp)
{
	if( comp != nil && 
		   (comp->ClassID() == SOFTVOLUME_CID)
		|| (comp->ClassID() == SOFTVOLUME_UNION_CID)
		|| (comp->ClassID() == SOFTVOLUME_ISECT_CID)
		|| (comp->ClassID() == SOFTVOLUME_NEGATE_CID) )
	{
		return (plSoftVolBaseComponent*)comp;
	}
	return nil;
}

void plSoftVolBaseComponent::IAddSubVolume(plKey masterKey, plKey subKey)
{
	if( masterKey && subKey )
		hsgResMgr::ResMgr()->AddViaNotify(subKey, TRACKED_NEW plGenRefMsg(masterKey, plRefMsg::kOnCreate, 0, plSoftVolume::kSubVolume), plRefFlags::kActiveRef);
}

plKey plSoftVolBaseComponent::ISetVolumeKey(plSoftVolume* vol)
{
	int i;
	for( i = 0; i < NumTargets(); i++ )
	{
		if( GetTarget(i) )
			break;
	}
	hsAssert(i < NumTargets(), "We're not attached to anything?");
	plKey key = hsgResMgr::ResMgr()->NewKey(GetINode()->GetName(), vol, GetTarget(i)->GetLocation());

	return key;
}

plKey plSoftVolBaseComponent::IInvertVolume(plKey subKey)
{
	if( !subKey )
		return nil;

	plSoftVolumeInvert* invert = TRACKED_NEW plSoftVolumeInvert;
	plKey invertKey = ISetVolumeKey(invert);

	IAddSubVolume(invertKey, subKey);

	return invertKey;
}

hsBool plSoftVolBaseComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fSoftKey = nil;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoftVolume Component
//
//Class that accesses the paramblock below.
class plSoftVolComponent : public plSoftVolBaseComponent
{
public:
	enum {
		kSoftDistance,
		kPartialEnabled,
		kInsidePower,
		kOutsidePower
	};

private:

	plKey				ISetFromIsect(plMaxNodeBase* pNode, plVolumeIsect* isect);
	plKey				ICreateSoftVolume();
	plKey				ICreateFromNode(plMaxNodeBase* pNode);
	plKey				ICreateFromDummyObject(plMaxNodeBase* pNode, Object* obj);
	plKey				ICreateFromTriObject(plMaxNodeBase* pNode, Object* obj);

protected:
	void				ICreateVolume();
public:
	plSoftVolComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

//
// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plSoftVolObjAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
	}
};
plSoftVolObjAccessor gSoftVolObjAccessor;


class plSoftVolComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
					IParamBlock2 *pb = map->GetParamBlock();
					map->SetTooltip(plSoftVolComponent::kSoftDistance, TRUE, "Distance effect fades in and out." );
			}
			return true;

		}

		return false;
	}
	void DeleteThis() {}
};
static plSoftVolComponentProc gSoftVolProc;



CLASS_DESC(plSoftVolComponent, gSoftVolDesc, "Soft Region",  "SoftRegion", COMP_TYPE_VOLUME, SOFTVOLUME_CID)



ParamBlockDesc2 gSoftVolBk
(
	plComponent::kBlkComp, _T("SoftRegion"), 0, &gSoftVolDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SOFTVOLUME, IDS_COMP_SOFTVOLUMES,  0, 0, &gSoftVolProc,

	plSoftVolComponent::kSoftDistance,		_T("Soft Distance"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 500.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTVOL_SOFT, IDC_COMP_SOFTVOL_SOFT_SPIN, 1.0,
		end,

	plSoftVolComponent::kPartialEnabled,  _T("EnablePartial"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SOFTVOL_ENABLEPARTIAL,
		p_enable_ctrls,		2, plSoftVolComponent::kInsidePower, plSoftVolComponent::kOutsidePower,
		end,

	plSoftVolComponent::kInsidePower,		_T("PowerInside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTVOL_INSIDE, IDC_COMP_SOFTVOL_INSIDE_SPIN, 1.0,
		end,

	plSoftVolComponent::kOutsidePower,		_T("PowerOutside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTVOL_OUTSIDE, IDC_COMP_SOFTVOL_OUTSIDE_SPIN, 1.0,
		end,

	end

);

plSoftVolComponent::plSoftVolComponent()
{
	fClassDesc = &gSoftVolDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSoftVolComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	if( !plSoftVolBaseComponent::SetupProperties(pNode, errMsg) )
		return false;

	int i;
	for( i = 0; i < pNode->NumAttachedComponents(); i++ )
	{
		plComponentBase* comp = pNode->GetAttachedComponent(i);
		if( comp 
			&& (comp != this)
			&& (comp->ClassID() == ClassID()) )
		{
			errMsg->Set(true, pNode->GetName(), "Multiple SoftRegion components attached, there can be only one").CheckAndAsk();
			errMsg->Set(false);

			fValid = false;
			return true;
		}
	}
	//
	// The node must either point to a Mesh or a dummy box, otherwise skip it.
	//
	Object *obj = pNode->EvalWorldState(TimeValue(0)).obj;
	if( !obj )
	{
		return true;
	}

	if( obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		fValid = true;
	}
	else if( obj->CanConvertToType(triObjectClassID) )
	{
		fValid = true;
	}
	pNode->SetDrawable(false);

	return true;
}

plKey plSoftVolComponent::ICreateSoftVolume()
{
	if( !fValid )
		return nil;

	if( !NumTargets() )
		return nil;

	if( NumTargets() < 2 )
	{
		return fSoftKey = ICreateFromNode(GetTarget(0));
	}

	plSoftVolumeUnion* compound = TRACKED_NEW plSoftVolumeUnion;
	fSoftKey = ISetVolumeKey(compound);

	int i;
	for( i = 0; i < NumTargets(); i++ )
	{
		if( GetTarget(i) )
		{
			plKey subKey = ICreateFromNode(GetTarget(i));
			IAddSubVolume(fSoftKey, subKey);
		}
	}
	if( !compound->GetNumSubs() )
	{
		delete compound;
		compound = nil;
		fSoftKey = nil;
	}

	return fSoftKey;
}

plKey plSoftVolComponent::ICreateFromNode(plMaxNodeBase* pNode)
{
	if( !pNode )
		return nil;

	if( !fValid )
		return nil;

	if( !pNode->GetSceneObject() )
		return nil;

	// Go ahead and make it here, so it'll be available for aggregaters in the Convert pass
	Object *obj = pNode->EvalWorldState(TimeValue(0)).obj;

	if( obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		return ICreateFromDummyObject(pNode, obj);
	}
	else if( obj->CanConvertToType(triObjectClassID) )
	{
		return ICreateFromTriObject(pNode, obj);
	}

	return nil;
}

plKey plSoftVolComponent::ICreateFromDummyObject(plMaxNodeBase* pNode, Object* obj)
{
	DummyObject* dummy = (DummyObject*)obj;
	Box3 bnd = dummy->GetBox();

	plParallelIsect* isect = TRACKED_NEW plParallelIsect;
	isect->SetNumPlanes(3);

	hsMatrix44 v2l = pNode->GetVertToLocal44();
	hsMatrix44 l2v = pNode->GetLocalToVert44();

	hsPoint3 corner(bnd.pmin.x, bnd.pmin.y, bnd.pmin.z);
	hsVector3 axis(bnd.pmax.x - bnd.pmin.x, bnd.pmax.y - bnd.pmin.y, bnd.pmax.z - bnd.pmin.z);

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsPoint3 bot = v2l * corner;
		hsPoint3 top = corner;
		top[i] += axis[i];
		top = v2l * top;

		isect->SetPlane(i, bot, top);
	}

	return ISetFromIsect(pNode, isect);
}

plKey plSoftVolComponent::ICreateFromTriObject(plMaxNodeBase* pNode, Object* obj)
{
	TriObject	*meshObj = (TriObject *)obj->ConvertToType(TimeValue(0), triObjectClassID);

	Mesh* mesh = &meshObj->mesh;

	hsMatrix44 v2l = pNode->GetVertToLocal44();
	hsMatrix44 l2v = pNode->GetLocalToVert44();

	plConvexIsect* isect = TRACKED_NEW plConvexIsect;
	int i;
	for( i = 0; i < mesh->getNumFaces(); i++ )
	{
		Face		*maxFace = &mesh->faces[ i ];

		Point3 v0 = mesh->verts[ maxFace->v[ 0 ] ];
		Point3 v1 = mesh->verts[ maxFace->v[ 1 ] ];
		Point3 v2 = mesh->verts[ maxFace->v[ 2 ] ];

		hsPoint3 p0(v0.x, v0.y, v0.z);
		hsPoint3 p1(v1.x, v1.y, v1.z);
		hsPoint3 p2(v2.x, v2.y, v2.z);

		p0 = v2l * p0;
		p1 = v2l * p1;
		p2 = v2l * p2;

		hsVector3 n = hsVector3(&p1, &p0) % hsVector3(&p2, &p0);

		isect->AddPlane(n, p0);
	}
	
	plKey retVal = ISetFromIsect(pNode, isect);

	if( meshObj != obj )
		meshObj->DeleteThis();

	return retVal;
}

plKey plSoftVolComponent::ISetFromIsect(plMaxNodeBase* pNode, plVolumeIsect* isect)
{
	isect->SetTransform(pNode->GetLocalToWorld44(), pNode->GetWorldToLocal44());

	plSoftVolumeSimple* simple = TRACKED_NEW plSoftVolumeSimple;
	simple->SetVolume(isect);
	simple->SetDistance(fCompPB->GetFloat(kSoftDistance));

	if( fCompPB->GetInt(kPartialEnabled) )
	{
		simple->SetInsideStrength(fCompPB->GetFloat(kInsidePower) * 0.01f);
		simple->SetOutsideStrength(fCompPB->GetFloat(kOutsidePower) * 0.01f);
	}

	plSceneObject* sceneObj = pNode->GetSceneObject();

	plKey retVal = ISetVolumeKey(simple);
	
	hsgResMgr::ResMgr()->AddViaNotify(simple->GetKey(), TRACKED_NEW plObjRefMsg(sceneObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Union of volumes
/////////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plSoftVolUnionComponent : public plSoftVolBaseComponent
{
public:
	enum {
		kSubVolumes,
		kPartialEnabled,
		kInsidePower,
		kOutsidePower
	};
protected:

	plKey				ICreateSoftVolume();

public:
	plSoftVolUnionComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plSoftVolUnionAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plSoftVolUnionComponent::kSubVolumes)
		{
			plSoftVolUnionComponent *comp = (plSoftVolUnionComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plSoftVolUnionAccessor gSoftVolUnionAccessor;


class plSoftVolUnionComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
					IParamBlock2 *pb = map->GetParamBlock();
					map->SetTooltip(plSoftVolUnionComponent::kSubVolumes, TRUE, "Select sub-volumes to combine into larger." );
			}
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_VOLUME)
			{
				// Adding a volume.  Set it and refresh the UI to show it in our list.
				plVolumeHitCallback hitCB((INode*)map->GetParamBlock()->GetOwner(), map->GetParamBlock(), plSoftVolUnionComponent::kSubVolumes, "Select sub-volumes");
				GetCOREInterface()->DoHitByNameDialog(&hitCB);
				map->Invalidate(plSoftVolUnionComponent::kSubVolumes);
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plSoftVolUnionComponentProc gSoftVolUnionProc;



CLASS_DESC(plSoftVolUnionComponent, gSoftVolUnionDesc, "Soft Region Union",  "SoftRegionUnion", COMP_TYPE_VOLUME, SOFTVOLUME_UNION_CID)



ParamBlockDesc2 gSoftVolUnionBk
(
	plComponent::kBlkComp, _T("SoftRegionUnion"), 0, &gSoftVolUnionDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SOFTVOLUME_UNION, IDS_COMP_SOFTVOLUME_UNION,  0, 0, &gSoftVolUnionProc,

	plSoftVolUnionComponent::kSubVolumes,	_T("SubRegions"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		p_classID,		SOFTVOLUME_BASE_CID,
		p_accessor,		&gSoftVolUnionAccessor,
		end,

	plSoftVolUnionComponent::kPartialEnabled,  _T("EnablePartial"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SOFTUNION_ENABLEPARTIAL,
		p_enable_ctrls,		2, plSoftVolUnionComponent::kInsidePower, plSoftVolUnionComponent::kOutsidePower,
		end,

	plSoftVolUnionComponent::kInsidePower,		_T("PowerInside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTUNION_INSIDE, IDC_COMP_SOFTUNION_INSIDE_SPIN, 1.0,
		end,

	plSoftVolComponent::kOutsidePower,		_T("PowerOutside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTUNION_OUTSIDE, IDC_COMP_SOFTUNION_OUTSIDE_SPIN, 1.0,
		end,

	end

);

plSoftVolUnionComponent::plSoftVolUnionComponent()
{
	fClassDesc = &gSoftVolUnionDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSoftVolUnionComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	return plSoftVolBaseComponent::SetupProperties(pNode, errMsg);
}

plKey plSoftVolUnionComponent::ICreateSoftVolume()
{
	int numSubs = fCompPB->Count(kSubVolumes);
	if( numSubs < 0 )
		return nil;

	if( numSubs < 2 )
		return fSoftKey = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSubVolumes, 0, 0))->GetSoftVolume();

	plSoftVolumeUnion* compound = TRACKED_NEW plSoftVolumeUnion;
	fSoftKey = ISetVolumeKey(compound);

	int i;
	for( i = 0; i < numSubs; i++ )
	{
		plSoftVolBaseComponent *comp = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSubVolumes, 0, i));
		if (comp)
		{
			plKey subKey = comp->GetSoftVolume();
			IAddSubVolume(fSoftKey, subKey);
		}
	}

	if( fCompPB->GetInt(kPartialEnabled) )
	{
		compound->SetInsideStrength(fCompPB->GetFloat(kInsidePower) * 0.01f);
		compound->SetOutsideStrength(fCompPB->GetFloat(kOutsidePower) * 0.01f);
	}

	return fSoftKey;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Intersection of volumes
/////////////////////////////////////////////////////////////////////////////////////////////////
//Class that accesses the paramblock below.
class plSoftVolIsectComponent : public plSoftVolBaseComponent
{
public:
	enum {
		kSubVolumes,
		kPartialEnabled,
		kInsidePower,
		kOutsidePower
	};
protected:

	plKey				ICreateSoftVolume();

public:
	plSoftVolIsectComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plSoftVolIsectAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plSoftVolIsectComponent::kSubVolumes)
		{
			plSoftVolIsectComponent *comp = (plSoftVolIsectComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plSoftVolIsectAccessor gSoftVolIsectAccessor;


class plSoftVolIsectComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
					IParamBlock2 *pb = map->GetParamBlock();
					map->SetTooltip(plSoftVolIsectComponent::kSubVolumes, TRUE, "Select sub-volumes to combine into larger." );
			}
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_VOLUME)
			{
				// Adding a volume.  Set it and refresh the UI to show it in our list.
				plVolumeHitCallback hitCB((INode*)map->GetParamBlock()->GetOwner(), map->GetParamBlock(), plSoftVolIsectComponent::kSubVolumes, "Select sub-volumes");
				GetCOREInterface()->DoHitByNameDialog(&hitCB);
				map->Invalidate(plSoftVolIsectComponent::kSubVolumes);
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plSoftVolIsectComponentProc gSoftVolIsectProc;



CLASS_DESC(plSoftVolIsectComponent, gSoftVolIsectDesc, "Soft Region Intersection",  "SoftRegionIsect", COMP_TYPE_VOLUME, SOFTVOLUME_ISECT_CID)



ParamBlockDesc2 gSoftVolIsectBk
(
	plComponent::kBlkComp, _T("SoftRegionIsect"), 0, &gSoftVolIsectDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SOFTVOLUME_ISECT, IDS_COMP_SOFTVOLUME_ISECT,  0, 0, &gSoftVolIsectProc,

	plSoftVolIsectComponent::kSubVolumes,	_T("SubRegions"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		p_classID,		SOFTVOLUME_BASE_CID,
		p_accessor,		&gSoftVolIsectAccessor,
		end,

	plSoftVolIsectComponent::kPartialEnabled,  _T("EnablePartial"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SOFTISECT_ENABLEPARTIAL,
		p_enable_ctrls,		2, plSoftVolIsectComponent::kInsidePower, plSoftVolIsectComponent::kOutsidePower,
		end,

	plSoftVolIsectComponent::kInsidePower,		_T("PowerInside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTISECT_INSIDE, IDC_COMP_SOFTISECT_INSIDE_SPIN, 1.0,
		end,

	plSoftVolIsectComponent::kOutsidePower,		_T("PowerOutside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTISECT_OUTSIDE, IDC_COMP_SOFTISECT_OUTSIDE_SPIN, 1.0,
		end,

	end

);

plSoftVolIsectComponent::plSoftVolIsectComponent()
{
	fClassDesc = &gSoftVolIsectDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSoftVolIsectComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	return plSoftVolBaseComponent::SetupProperties(pNode, errMsg);
}

plKey plSoftVolIsectComponent::ICreateSoftVolume()
{
	int numSubs = fCompPB->Count(kSubVolumes);
	if( numSubs < 0 )
		return nil;

	if( numSubs < 2 )
		return fSoftKey = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSubVolumes, 0, 0))->GetSoftVolume();

	plSoftVolumeIntersect* compound = TRACKED_NEW plSoftVolumeIntersect;
	fSoftKey = ISetVolumeKey(compound);

	int i;
	for( i = 0; i < numSubs; i++ )
	{
		plKey subKey = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSubVolumes, 0, i))->GetSoftVolume();
		IAddSubVolume(fSoftKey, subKey);
	}

	if( fCompPB->GetInt(kPartialEnabled) )
	{
		compound->SetInsideStrength(fCompPB->GetFloat(kInsidePower) * 0.01f);
		compound->SetOutsideStrength(fCompPB->GetFloat(kOutsidePower) * 0.01f);
	}

	return fSoftKey;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Inversion of a (possibly complex) volume
/////////////////////////////////////////////////////////////////////////////////////////////////
//Class that accesses the paramblock below.
class plSoftVolNegateComponent : public plSoftVolBaseComponent
{
public:
	enum {
		kSubVolume,
		kPartialEnabled,
		kInsidePower,
		kOutsidePower
	};
protected:

	plKey				ICreateSoftVolume();

public:
	plSoftVolNegateComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plSoftVolNegateAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plSoftVolNegateComponent::kSubVolume)
		{
			plSoftVolNegateComponent *comp = (plSoftVolNegateComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plSoftVolNegateAccessor gSoftVolNegateAccessor;


static plSingleCompSelProc gSoftVolNegateSingleSel(plSoftVolNegateComponent::kSubVolume, IDC_COMP_SOFTVOLUME_NEGATE_CHOOSE_SUB, "Select sub-volumes");



CLASS_DESC(plSoftVolNegateComponent, gSoftVolNegateDesc, "Soft Region Inverted",  "SoftRegionInvert", COMP_TYPE_VOLUME, SOFTVOLUME_NEGATE_CID)



ParamBlockDesc2 gSoftVolNegateBk
(
	plComponent::kBlkComp, _T("SoftRegionNegate"), 0, &gSoftVolNegateDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SOFTVOLUME_NEGATE, IDS_COMP_SOFTVOLUME_NEGATE,  0, 0, &gSoftVolNegateSingleSel, 

	plSoftVolNegateComponent::kSubVolume, _T("SubRegion"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_SOFTVOLUME_NEGATE,
		p_accessor, &gSoftVolNegateAccessor,
		end,

	plSoftVolNegateComponent::kPartialEnabled,  _T("EnablePartial"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SOFTNEGATE_ENABLEPARTIAL,
		p_enable_ctrls,		2, plSoftVolComponent::kInsidePower, plSoftVolComponent::kOutsidePower,
		end,

	plSoftVolNegateComponent::kInsidePower,		_T("PowerInside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTNEGATE_INSIDE, IDC_COMP_SOFTNEGATE_INSIDE_SPIN, 1.0,
		end,

	plSoftVolNegateComponent::kOutsidePower,		_T("PowerOutside"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_SOFTNEGATE_OUTSIDE, IDC_COMP_SOFTNEGATE_OUTSIDE_SPIN, 1.0,
		end,


	end

);

plSoftVolNegateComponent::plSoftVolNegateComponent()
{
	fClassDesc = &gSoftVolNegateDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSoftVolNegateComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	return plSoftVolBaseComponent::SetupProperties(pNode, errMsg);
}

plKey plSoftVolNegateComponent::ICreateSoftVolume()
{
	if( NumTargets() < 1 )
		return nil;

	INode* subNode = fCompPB->GetINode(kSubVolume);
	if( subNode )
	{
		plKey subKey = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSubVolume))->GetSoftVolume();
		if( subKey )
		{
			fSoftKey = IInvertVolume(subKey);

			if( fCompPB->GetInt(kPartialEnabled) )
			{
				plSoftVolumeInvert* invert = plSoftVolumeInvert::ConvertNoRef(fSoftKey->GetObjectPtr());
				if( invert )
				{
					invert->SetInsideStrength(fCompPB->GetFloat(kInsidePower) * 0.01f);
					invert->SetOutsideStrength(fCompPB->GetFloat(kOutsidePower) * 0.01f);
				}
			}
			return fSoftKey;
		}
	}
	return nil;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Soft Light Region
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plLightRegionComponent : public plComponent
{
public:
	enum {
		kSoftVolume
	};
public:
	plLightRegionComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *errMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *errMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg);
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plLightRegionAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plLightRegionComponent::kSoftVolume)
		{
			plLightRegionComponent *comp = (plLightRegionComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plLightRegionAccessor gLightRegionAccessor;


static plSingleCompSelProc gLightRegionSingleSel(plLightRegionComponent::kSoftVolume, IDC_COMP_LIGHTREGION_CHOOSE_VOLUME, "Select soft region for light");


//Max desc stuff necessary below.
CLASS_DESC(plLightRegionComponent, gLightRegionDesc, "Light Region",  "LightRegion", COMP_TYPE_VOLUME, LIGHTREGION_CID)


ParamBlockDesc2 gLightRegionBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("LightRegion"), 0, &gLightRegionDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_LIGHTREGION, IDS_COMP_LIGHTREGION, 0, 0, &gLightRegionSingleSel,

	plLightRegionComponent::kSoftVolume, _T("Region"),	TYPE_INODE,		0, 0,
		p_accessor,		&gLightRegionAccessor,
		end,
	end
);

plLightRegionComponent::plLightRegionComponent()
{
	fClassDesc = &gLightRegionDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plLightRegionComponent::Convert(plMaxNode *node, plErrorMsg *errMsg)
{
	if( !fCompPB->GetINode(kSoftVolume) )
		return true;

	plSceneObject* sceneObj = node->GetSceneObject();
	if( !sceneObj )
		return true;

	plLightInfo* li = plLightInfo::ConvertNoRef(sceneObj->GetGenericInterface(plLightInfo::Index()));
	if( !li )
		return true;

	plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSoftVolume));
	if( !softComp )
		return true;

	plKey softKey = softComp->GetSoftVolume();
	if( !softKey )
		return true;

	hsgResMgr::ResMgr()->AddViaNotify(softKey, TRACKED_NEW plGenRefMsg(li->GetKey(), plRefMsg::kOnCreate, 0, plLightInfo::kSoftVolume), plRefFlags::kActiveRef);

	return true;
}

hsBool plLightRegionComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *errMsg)
{

	return true;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plLightRegionComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Soft Visibility Region
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plVisRegionSingleSel : public plSingleCompSelProc
{
public:
	plVisRegionSingleSel(ParamID nodeID, int dlgItem, TCHAR *title)
		:	plSingleCompSelProc(nodeID, dlgItem, title)
	{
	}
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
				IParamBlock2 *pb = map->GetParamBlock();
				if( pb->GetInt(plVisRegionComponent::kExcludes) )
				{
					map->Enable(plVisRegionComponent::kDisableNormal, FALSE);
					map->Enable(plVisRegionComponent::kAffectDraw, TRUE);
					map->Enable(plVisRegionComponent::kAffectLight, TRUE);
					map->Enable(plVisRegionComponent::kAffectOcc, TRUE);
				}
				else
				{
					map->Enable(plVisRegionComponent::kDisableNormal, TRUE);
					if( pb->GetInt(plVisRegionComponent::kDisableNormal) )
					{
						map->Enable(plVisRegionComponent::kAffectDraw, FALSE);
						map->Enable(plVisRegionComponent::kAffectLight, FALSE);
						map->Enable(plVisRegionComponent::kAffectOcc, FALSE);
					}
					else
					{
						map->Enable(plVisRegionComponent::kAffectDraw, TRUE);
						map->Enable(plVisRegionComponent::kAffectLight, TRUE);
						map->Enable(plVisRegionComponent::kAffectOcc, TRUE);
					}
				}
			}
			break;
		case WM_COMMAND:
			{
				if( (LOWORD(wParam) == IDC_COMP_VISREGION_NOT) || (LOWORD(wParam) == IDC_COMP_VISREGION_DIS) )
				{
					IParamBlock2 *pb = map->GetParamBlock();
					if( pb->GetInt(plVisRegionComponent::kExcludes) )
					{
						map->Enable(plVisRegionComponent::kDisableNormal, FALSE);
						map->Enable(plVisRegionComponent::kAffectDraw, TRUE);
						map->Enable(plVisRegionComponent::kAffectLight, TRUE);
						map->Enable(plVisRegionComponent::kAffectOcc, TRUE);
					}
					else
					{
						map->Enable(plVisRegionComponent::kDisableNormal, TRUE);
						if( pb->GetInt(plVisRegionComponent::kDisableNormal) )
						{
							map->Enable(plVisRegionComponent::kAffectDraw, FALSE);
							map->Enable(plVisRegionComponent::kAffectLight, FALSE);
							map->Enable(plVisRegionComponent::kAffectOcc, FALSE);
						}
						else
						{
							map->Enable(plVisRegionComponent::kAffectDraw, TRUE);
							map->Enable(plVisRegionComponent::kAffectLight, TRUE);
							map->Enable(plVisRegionComponent::kAffectOcc, TRUE);
						}
					}
					return TRUE;
				}
			}
			break;
		}
		return plSingleCompSelProc::DlgProc(t, map, hWnd, msg, wParam, lParam);
	}
};

static plVisRegionSingleSel gVisRegionSingleSel(plVisRegionComponent::kSoftVolume, IDC_COMP_VISREGION_CHOOSE_VOLUME, "Select region for visibility");


//Max desc stuff necessary below.
CLASS_DESC(plVisRegionComponent, gVisRegionDesc, "Visibility Region",  "VisRegion", COMP_TYPE_VOLUME, VISREGION_CID)


ParamBlockDesc2 gVisRegionBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("VisRegion"), 0, &gVisRegionDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_VISREGION, IDS_COMP_VISREGION, 0, 0, &gVisRegionSingleSel,

	plVisRegionComponent::kSoftVolume, _T("Region"),	TYPE_INODE,		0, 0,
		p_accessor,		nil,
		end,

	plVisRegionComponent::kAffectDraw,  _T("AffectDraw"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_VISREGION_DRAW,
		end,

	plVisRegionComponent::kAffectLight,  _T("AffectLight"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_VISREGION_LIGHT,
		end,

	plVisRegionComponent::kAffectOcc,  _T("AffectOcc"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_VISREGION_OCC,
		end,

	plVisRegionComponent::kExcludes,  _T("Excludes"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_VISREGION_NOT,
		end,

	plVisRegionComponent::kDisableNormal,	_T("DisableNormal"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_VISREGION_DIS,
		end,

	end
);

plVisRegionComponent::plVisRegionComponent()
{
	fClassDesc = &gVisRegionDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plVisRegionComponent::ICheckVisRegion(const plLocation& loc)
{
	if( !fVisReg )
	{
		if( !fCompPB->GetINode(kSoftVolume) )
			return;

		plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSoftVolume));
		if( !softComp )
			return;

		plKey softKey = softComp->GetSoftVolume();
		if( !softKey )
			return;

		fVisReg = TRACKED_NEW plVisRegion;
		plKey key = hsgResMgr::ResMgr()->NewKey(GetINode()->GetName(), fVisReg, loc);


		hsBool excludes = fCompPB->GetInt(kExcludes);
		hsBool disableNormal = excludes ? false : fCompPB->GetInt(kDisableNormal);

		fVisReg->SetProperty(plVisRegion::kIsNot, excludes);
		fVisReg->SetProperty(plVisRegion::kReplaceNormal, true);
		fVisReg->SetProperty(plVisRegion::kDisableNormal, disableNormal);

		plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(fVisReg->GetKey(), plRefMsg::kOnCreate, 0, plVisRegion::kRefRegion);
		hsgResMgr::ResMgr()->SendRef(softKey, refMsg, plRefFlags::kActiveRef);
	}
}

hsBool plVisRegionComponent::Convert(plMaxNode *node, plErrorMsg *errMsg)
{
	const char* dbgNodeName = node->GetName();
	plSceneObject* obj = node->GetSceneObject();
	if( !obj )
		return true;

	hsBool excludes = fCompPB->GetInt(kExcludes);
	hsBool disableNormal = excludes ? false : fCompPB->GetInt(kDisableNormal);
	hsBool affectDraw = disableNormal ? true : fCompPB->GetInt(kAffectDraw);
	hsBool affectOcc = disableNormal ? true : fCompPB->GetInt(kAffectOcc);
	hsBool affectLight = disableNormal ? true : fCompPB->GetInt(kAffectLight);

	const plDrawInterface* di = affectDraw ? obj->GetDrawInterface() : nil;
	plOccluder* occ = affectOcc ? (plOccluder*)obj->GetGenericInterface(plOccluder::Index()) : nil;
	plLightInfo* li = affectLight ? (plLightInfo*)obj->GetGenericInterface(plLightInfo::Index()) : nil;
	if( !(disableNormal || di || occ || li) )
		return true;

	ICheckVisRegion(node->GetLocation());
	if( !fVisReg )
		return true;

	if( di )
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(di->GetKey(), plRefMsg::kOnCreate, 0, plDrawInterface::kRefVisRegion), plRefFlags::kActiveRef);

	if( occ )
	{
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(occ->GetKey(), plRefMsg::kOnCreate, 0, plOccluder::kRefVisRegion), plRefFlags::kActiveRef);
	}

	if( li )
	{
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(li->GetKey(), plRefMsg::kOnCreate, 0, plLightInfo::kVisRegion), plRefFlags::kActiveRef);
	}

	if( !(di || occ || li) )
	{
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	}

	return true;
}

hsBool plVisRegionComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	fVisReg = nil;

	return true;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plVisRegionComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{

	return true;
}

void plVisRegionComponent::CollectRegions(plMaxNode* node, hsTArray<plVisRegion*>& regions)
{
	int i;
	for( i = 0; i < node->NumAttachedComponents(); i++ )
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if( comp && comp->ClassID() == VISREGION_CID )
		{
			plVisRegionComponent* regComp = (plVisRegionComponent*)comp;
			if( regComp )
			{
				regComp->ICheckVisRegion(node->GetLocation());
				if( regComp->fVisReg )
					regions.Append(regComp->fVisReg);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Relevance Region
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plRelevanceRegionComponent : public plComponent
{
public:
	enum {
		kSoftVolume,
		kName,
	};
protected:
	plRelevanceRegion* fRegion;

public:
	plRelevanceRegionComponent();
	void DeleteThis() { delete this; }

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *errMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg);
};


static plSingleCompSelProc gRelevanceRegionSingleSel(plRelevanceRegionComponent::kSoftVolume, IDC_COMP_RELREGION_CHOOSE_VOLUME, "Select region");


//Max desc stuff necessary below.
CLASS_DESC(plRelevanceRegionComponent, gRelevanceRegionDesc, "Relevance Region",  "RelevanceRegion", COMP_TYPE_VOLUME, RELREGION_CID)


ParamBlockDesc2 gRelevanceRegionBk
(
	plComponent::kBlkComp, _T("RelevanceRegion"), 0, &gRelevanceRegionDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_RELREGION, IDS_COMP_RELREGION, 0, 0, &gRelevanceRegionSingleSel,

	plRelevanceRegionComponent::kSoftVolume, _T("Region"),	TYPE_INODE,		0, 0,
		p_accessor,		nil,
		end,
		
	end
);

plRelevanceRegionComponent::plRelevanceRegionComponent()
{
	fClassDesc = &gRelevanceRegionDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plRelevanceRegionComponent::Convert(plMaxNode *node, plErrorMsg *errMsg)
{
	const char* dbgNodeName = node->GetName();
	plSceneObject* obj = node->GetSceneObject();
	if( !obj )
		return true;

	if( !fRegion )
	{
		if( !fCompPB->GetINode(kSoftVolume) )
			return true;

		plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent(fCompPB->GetINode(kSoftVolume));
		if( !softComp )
			return true;

		plKey softKey = softComp->GetSoftVolume();
		if( !softKey )
			return true;

		fRegion = TRACKED_NEW plRelevanceRegion;
		plKey key = hsgResMgr::ResMgr()->NewKey(GetINode()->GetName(), fRegion, node->GetLocation());

		plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(fRegion->GetKey(), plRefMsg::kOnCreate, 0, 0);
		hsgResMgr::ResMgr()->SendRef(softKey, refMsg, plRefFlags::kActiveRef);
	}

	hsgResMgr::ResMgr()->AddViaNotify(fRegion->GetKey(), TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	
	return true;
}

hsBool plRelevanceRegionComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	fRegion = nil;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Special Effects Visibility Set
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



//Max desc stuff necessary below.
CLASS_DESC(plEffVisSetComponent, gEffVisSetDesc, "Effect Vis Set",  "EffVisSet", COMP_TYPE_VOLUME, EFFVISSET_CID)


ParamBlockDesc2 gEffVisSetBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("EffVisSet"), 0, &gEffVisSetDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_EFFVISSET, IDS_COMP_EFFVISSET, 0, 0, NULL,

	plEffVisSetComponent::kHideNormal,  _T("HideNormal"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_EFFVISSET_HIDENORMAL,
		end,

	end
);

plEffVisSetComponent::plEffVisSetComponent()
{
	fClassDesc = &gEffVisSetDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plEffVisSetComponent::Convert(plMaxNode *node, plErrorMsg *errMsg)
{
	const char* dbgNodeName = node->GetName();
	plSceneObject* obj = node->GetSceneObject();
	if( !obj )
		return true;

	const plDrawInterface* di = obj->GetDrawInterface();
	plOccluder* occ = (plOccluder*)obj->GetGenericInterface(plOccluder::Index());
	plLightInfo* li = (plLightInfo*)obj->GetGenericInterface(plLightInfo::Index());
	if( !(di || occ || li) )
		return true;

	if( !GetVisRegion(node) )
		return false;

	if( di )
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(di->GetKey(), plRefMsg::kOnCreate, 0, plDrawInterface::kRefVisRegion), plRefFlags::kActiveRef);

	if( occ )
	{
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(occ->GetKey(), plRefMsg::kOnCreate, 0, plOccluder::kRefVisRegion), plRefFlags::kActiveRef);
	}

	if( li )
	{
		hsgResMgr::ResMgr()->AddViaNotify(fVisReg->GetKey(), TRACKED_NEW plGenRefMsg(li->GetKey(), plRefMsg::kOnCreate, 0, plLightInfo::kVisRegion), plRefFlags::kActiveRef);
	}

	return true;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plEffVisSetComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{
	fVisReg = nil;

	return true;
}

plVisRegion* plEffVisSetComponent::GetVisRegion(plMaxNode* node)
{
	if( !fVisReg )
	{
		fVisReg = TRACKED_NEW plVisRegion;
		plKey key = hsgResMgr::ResMgr()->NewKey(GetINode()->GetName(), fVisReg, node->GetLocation());

		fVisReg->SetProperty(plVisRegion::kIsNot, false);
		fVisReg->SetProperty(plVisRegion::kReplaceNormal, fCompPB->GetInt(kHideNormal));
		fVisReg->SetProperty(plVisRegion::kDisable, true);
	}

	return fVisReg;
}


plEffVisSetComponent* plEffVisSetComponent::ConvertToEffVisSetComponent(plMaxNode* node)
{
	if( !node )
		return nil;

	plComponentBase *comp = node->ConvertToComponent();

	// If this is an activator type component
	if( comp )
	{
		if( comp->ClassID() == EFFVISSET_CID )
		{
			return (plEffVisSetComponent*)comp;
		}
	}
	return nil;
}

void plEffVisSetComponent::CollectRegions(plMaxNode* node, hsTArray<plVisRegion*>& regions)
{
	int i;
	for( i = 0; i < node->NumAttachedComponents(); i++ )
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if( comp && comp->ClassID() == EFFVISSET_CID )
		{
			plEffVisSetComponent* regComp = (plEffVisSetComponent*)comp;
			if( regComp )
			{
				if( regComp->fVisReg )
					regions.Append(regComp->fVisReg);
			}
		}
	}
}

