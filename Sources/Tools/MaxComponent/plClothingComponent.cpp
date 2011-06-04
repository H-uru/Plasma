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
//#include "hsUtils.h"
#include "hsTemplates.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plPageInfo.h"
#include "hsResMgr.h"
#include "../MaxMain/plMaxNode.h"
#include "plClothingComponent.h"
#include "plComponentReg.h"
#include "../MaxPlasmaMtls/Materials/plClothingMtl.h"
#include "../pnMessage/plRefMsg.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plMeshConverter.h"
#include "plPickMaterialMap.h"
#include "../MaxMain/plMtlCollector.h"
#include "plAvatarComponent.h"
#include "../MaxMain/plPlasmaRefMsgs.h"			
#include "../plDrawable/plSharedMesh.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plMorphSequence.h"
#include "../plScene/plSceneNode.h"
#include "../plDrawable/plGeometrySpan.h"

void DummyCodeIncludeFuncClothing()
{
}

CLASS_DESC(plClothingComponent, gClothingDesc, "Avatar Clothing",  "AvatarClothing", COMP_TYPE_MISC, CLOTHING_COMPONENT_CLASS_ID)

static plClothingComponentProc gClothingComponentProc;

class plClothingAccessor : public PBAccessor
{
public:

	//! Public Accessor Class, used in ParamBlock2 processing.
	/*!
	Workhorse for this Accessor Class (derived from Max's PBAccessor).

	When one of our parameters that is a ref changes, send out the component ref
	changed message.  Normally, messages from component refs are ignored since
	they pass along all the messages of the ref, which generates a lot of false
	converts.
	*/

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plClothingComponent::kMeshNodeAddBtn)
		{
			plClothingComponent *comp = (plClothingComponent *)owner;
			IParamBlock2 *pb = comp->GetParamBlockByID(plClothingComponent::kBlkComp);
			int state = pb->GetInt(plClothingComponent::kLODState);
				
			INode *node = pb->GetINode(plClothingComponent::kMeshNodeAddBtn);
			if (node)
				pb->SetValue(plClothingComponent::kMeshNodeTab, 0, node, state);
		}
			
		if (id == plClothingComponent::kMeshNodeTab)
		{
			plComponentBase *comp = (plComponentBase*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};

plClothingAccessor gClothingAccessor;

ParamBlockDesc2 gClothingBk
(	

	plComponent::kBlkComp, _T("Clothing"), 0, &gClothingDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_CLOTHING, IDS_COMP_CLOTHING, 0, 0, &gClothingComponentProc,

	plClothingComponent::kMaterials,	_T("ClothingMaterials"),	TYPE_MTL_TAB, 0,	0, 0,	
		end,
		
	plClothingComponent::kGroup,	_T("ClothingGroup"),	TYPE_INT, 0, 0,
		end,
		
	plClothingComponent::kType,		_T("ClothingType"),		TYPE_INT, 0, 0,
		end,

	plClothingComponent::kLODState, _T("LODState"),		TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plClothingComponent::kMeshNodeTab,	_T("MeshObject"),	TYPE_INODE_TAB, plLODAvatarComponent::kMaxNumLODLevels,		0, 0,
		p_accessor,		&gClothingAccessor,
		end,

	plClothingComponent::kMeshNodeAddBtn, _T("MshNodePicker"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_LOD_CLOTHING_MESH_PICKB,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_AVATAR_PROXYS,
		p_accessor, &gClothingAccessor,
		end,

	end
);

plClothingComponent::plClothingComponent()
{
	fClassDesc = &gClothingDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plClothingComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	int i;
	for (i = 0; i < fCompPB->Count(kMeshNodeTab); i++)
	{
		plMaxNode *LODNode = (plMaxNode *)fCompPB->GetINode(kMeshNodeTab, 0, i);
		if (LODNode != nil)
		{
			char *dbgNodeName = LODNode->GetName();
			//LODNode->SetCanConvert(false);
			LODNode->SetDrawable(false);
			LODNode->SetForceShadow(true);
			LODNode->SetForceMatShade(true);
			if (!LODNode->GetSwappableGeom())
				LODNode->SetSwappableGeom(new plSharedMesh);

			//UInt32 targetID = fCompPB->GetInt(kType);
			//((plMaxNode *)LODNode->GetParentNode())->SetSwappableGeomTarget(targetID);
		}
	}
	
	return true;
}

hsBool plClothingComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetupBoneHierarchyPalette();

	return true;
}

hsBool plClothingComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	int i, j;
	hsTArray<plGeometrySpan*> spanArray;
	hsTArray<plKey> keys;
	plMaxNode *LODNode = nil;
	plMaxNode *locationNode = nil;


	if (fCompPB->Count(plClothingComponent::kMaterials) <= 0)
		return true;

	for (i = 0; i < fCompPB->Count(kMeshNodeTab); i++)
	{
		spanArray.Reset();
		//plSharedMesh *mesh = TRACKED_NEW plSharedMesh;
		LODNode = (plMaxNode *)fCompPB->GetINode(kMeshNodeTab, 0, i);
		if (LODNode != nil)
		{
			char *dbgNodeName = LODNode->GetName();
			keys.Append(LODNode->GetSwappableGeom()->GetKey());
			locationNode = LODNode;

			if (fCompPB->GetInt(ParamID(kType)) != plClothingMgr::kTypeFace)
			{
				// We only save state for the face node.
				LODNode->GetSwappableGeom()->fFlags |= plSharedMesh::kDontSaveMorphState;
			}
			else
			{
				// The face's weight for the first layer (0) is to be applied to all
				// meshes on that node.
				LODNode->GetSwappableGeom()->fFlags |= plSharedMesh::kLayer0GlobalToMod;
			}
		}
		else
		{
			keys.Append(nil);
			//delete mesh;
		}
	}

	const plPageInfo* thisInfo = plKeyFinder::Instance().GetLocationInfo(locationNode ? locationNode->GetLocation() : node->GetLocation());
	const plLocation &loc = plKeyFinder::Instance().FindLocation("GlobalClothing", thisInfo->GetPage());

	for (i = 0; i < fCompPB->Count(plClothingComponent::kMaterials); i++)
	{
		plClothingMtl *mtl = (plClothingMtl *)fCompPB->GetMtl(ParamID(plClothingComponent::kMaterials), 0, i);
		plClothingItem *cloth = hsMaterialConverter::Instance().GenerateClothingItem(mtl, loc);
		cloth->fGroup = fCompPB->GetInt(ParamID(kGroup));
		cloth->fType = fCompPB->GetInt(ParamID(kType));

		plGenRefMsg *refMsg;
		for (j = 0; j < keys.GetCount(); j++)
		{
			if (keys[j] != nil)
			{
				refMsg = TRACKED_NEW plGenRefMsg(cloth->GetKey(), plRefMsg::kOnCreate, j, -1);
				hsgResMgr::ResMgr()->AddViaNotify(keys[j], refMsg, plRefFlags::kActiveRef);
			}
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////

BOOL plClothingComponentProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2 *pb = pm->GetParamBlock();
	HWND hList = GetDlgItem(hWnd, IDC_CLOTHING_LIST);
	HWND hGroup = GetDlgItem(hWnd, IDC_CLOTHING_GROUP);
	HWND hType = GetDlgItem(hWnd, IDC_CLOTHING_TYPE);
	HWND hLOD = GetDlgItem(hWnd, IDC_COMP_LOD_CLOTHING_STATE);
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			ListBox_ResetContent(hList);
			int i;
			for (i = 0; i < pb->Count(plClothingComponent::kMaterials); i++)
				ListBox_AddString(hList, pb->GetMtl(ParamID(plClothingComponent::kMaterials), 0, i)->GetName());

			ListBox_SetCurSel(hList, -1);

			for (i = 0; i < plClothingMgr::kMaxGroup; i++)
				ComboBox_AddString(hGroup, plClothingMgr::GroupStrings[i]);
			ComboBox_SetCurSel(hGroup, pb->GetInt(plClothingComponent::kGroup));

			for (i = 0; i < plClothingMgr::kMaxType; i++)
				ComboBox_AddString(hType, plClothingMgr::TypeStrings[i]);
			ComboBox_SetCurSel(hType, pb->GetInt(plClothingComponent::kType));

			ComboBox_AddString(hLOD, "High");
			ComboBox_AddString(hLOD, "Medium");
			ComboBox_AddString(hLOD, "Low");
			ComboBox_SetCurSel(hLOD, pb->GetInt(plClothingComponent::kLODState));
		}
		return TRUE;


	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDC_CLOTHING_ADD)
			{
				Mtl *pickedMtl = plPickMaterialMap::PickMaterial(plMtlCollector::kClothingMtlOnly);
				if (pickedMtl != nil)
				{
					LRESULT stringIdx = ListBox_FindStringExact(hList, -1, pickedMtl->GetName());
					if (stringIdx == LB_ERR) // It's not already there, go and add it
					{
						pb->Append(ParamID(plClothingComponent::kMaterials), 1, &pickedMtl, 0);
						ListBox_AddString(hList, pickedMtl->GetName());
					}
				}
				return TRUE;
			}
			// Remove the currently selected material
			else if (LOWORD(wParam) == IDC_CLOTHING_REMOVE)
			{
				int sel = ListBox_GetCurSel(hList);
				if (sel != LB_ERR)
				{
					pb->Delete(plClothingComponent::kMaterials, sel, 1);
					ListBox_DeleteString(hList, sel);
				}
				return TRUE;
			}
			else if( LOWORD( wParam ) == IDC_CLOTHING_CLEARMESH )
			{
				int state = pb->GetInt(plClothingComponent::kLODState);
				pb->SetValue(plClothingComponent::kMeshNodeTab, 0, (INode*)nil, state );
				pb->Reset(plClothingComponent::kMeshNodeAddBtn);
			}
		}
		else if (LOWORD(wParam) == IDC_CLOTHING_GROUP)
		{
			int setIdx = ComboBox_GetCurSel(hGroup);
			pb->SetValue(plClothingComponent::kGroup, 0, setIdx);

			return TRUE;
		}
		else if (LOWORD(wParam) == IDC_CLOTHING_TYPE)
		{
			int setIdx = ComboBox_GetCurSel(hType);
			pb->SetValue(plClothingComponent::kType, 0, setIdx);

			return TRUE;
		}
		else
		{
			int state = pb->GetInt(plClothingComponent::kLODState);
				
			INode *node = pb->GetINode(plClothingComponent::kMeshNodeAddBtn);
			if (node)
				pb->SetValue(plClothingComponent::kMeshNodeTab, 0, node, state);

			if(LOWORD(wParam) == IDC_COMP_LOD_CLOTHING_STATE && HIWORD(wParam) == CBN_SELCHANGE)
			{
				int idx = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				pb->SetValue(plClothingComponent::kLODState, 0, idx);
			
				node = pb->GetINode(plClothingComponent::kMeshNodeTab, 0, idx);
				if (node)
					pb->SetValue(plClothingComponent::kMeshNodeAddBtn, 0, node);
				else
					pb->Reset(plClothingComponent::kMeshNodeAddBtn);

				return TRUE;
			}
		}
	}

	return FALSE;
}
