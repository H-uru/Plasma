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
#include "plInventoryObjComponent.h"							//Inventory Dependencies

#include "resource.h"											//Resource Dependencies
#include "hsResMgr.h"						// Ibid
	
#include "plComponent.h"										//Component Dependencies
#include "plComponentReg.h"										// Ibid
#include "../pnSceneObject/plSceneObject.h"						// Ibid
#include "../pnKeyedObject/hsKeyedObject.h"						// Ibid
#include "../MaxMain/plMaxNode.h"								// Ibid
#include "plResponderComponent.h"


#include "../plPhysical/plCollisionDetector.h"					//Modifiers Dependencies
#include "../plModifier/plLogicModifier.h"						// Ibid
#include "../plModifier/plAxisAnimModifier.h"					// Ibid
#include "../../NucleusLib/pnModifier/plConditionalObject.h"	// Ibid
#include "../plPhysical/plPickingDetector.h"					// Ibid
#include "../pfConditional/plActivatorConditionalObject.h"		// Ibid
#include "../pfConditional/plFacingConditionalObject.h"			// Ibid
#include "../pfConditional/plObjectInBoxConditionalObject.h"	// Ibid

#include "../pnMessage/plObjRefMsg.h"							//Message Dependencies
#include "../pnMessage/plNotifyMsg.h"							// Ibid
#include "../pnMessage/plCursorChangeMsg.h"						// Ibid


#include "../MaxConvert/plConvert.h"


//
//	DummyCodeIncludeFuncInventStuff Function.
//		Necessary to keep the compiler from throwing away this file.
//		No functions within are inherently called otherwise....
//
//


void DummyCodeIncludeFuncInventStuff() {}




CLASS_DESC(plInventoryObjComponent, gInventoryObjDesc, "(ex)InventoryObj",  "(ex)InventoryObj", COMP_TYPE_LOGIC, INVENTORYOBJCOMP_CID)


enum
{
	kClickDragDirectional,
	kClickDragDegrees,
	kClickDragUseProxy,
	kClickDragProxy,
	kClickDragUseRegion,
	kClickDragProxyRegion,
	kClickDragUseX,
	kClickDragUseY,
	kClickDragAnimX,
	kClickDragAnimY,
	kClickDragUseLoopX,
	kClickDragUseLoopY,
	kClickDragLoopAnimX,
	kClickDragLoopAnimY,
	kAgeSpecificCheckBx,		//Added in v1
	kRespawnAfterLostCheckBx,	//Added in v1
	kConsumableCheckbx,			//Added in v1
	kLifeSpan					//Added in v1
};


class plInventoryObjComponentProc;
extern plInventoryObjComponentProc gInventoryObjComponentProc;

ParamBlockDesc2 gInventoryObjBlock
(
	plComponent::kBlkComp, _T("ClickDragComp"), 0, &gInventoryObjDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_INV_OBJECT, IDS_COMP_INV_OBJECTS, 0, 0, NULL, //&gInventoryObjComponentProc,

	kAgeSpecificCheckBx,  _T("AgeSpecificObject"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_enable_ctrls, 1, kRespawnAfterLostCheckBx,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_ITINERANTBOOL,
		end,

	kRespawnAfterLostCheckBx,  _T("RespawnAtSPObject"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_RESPAWNBOOL,
		end,
	
	kConsumableCheckbx , _T("TemporaryObject"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_enable_ctrls, 1, kLifeSpan,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_INV_OBJECT_CONSUMABLE,
		end,	

	kLifeSpan,	_T("LifeSpan"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 100000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_INV_OBJECT_LIFE_EDIT, IDC_COMP_INV_OBJECT_LIFE_SPIN, 1.0f,
		end,



	end
);

plInventoryObjComponent::plInventoryObjComponent()
{
	fClassDesc = &gInventoryObjDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

const plInventoryObjComponent::LogicKeys& plInventoryObjComponent::GetLogicKeys()
{
	return fLogicModKeys;
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plInventoryObjComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fLogicModKeys.clear();
	fReceivers.Reset();
	return true;
}

plKey plInventoryObjComponent::GetLogicKey(plMaxNode* node)
{
	LogicKeys::const_iterator it;
	
	for (it = fLogicModKeys.begin(); it != fLogicModKeys.end(); it++)
	{
		if (node == it->first)
			return(it->second);
	}
	return nil;
}


hsBool plInventoryObjComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plLocation loc = node->GetLocation();
	plSceneObject *obj = node->GetSceneObject();

	// Create and register the ClickDrag's logic component
	plLogicModifier *logic = TRACKED_NEW plLogicModifier;
	char tmpName[256];
	sprintf(tmpName, "%s_%s_LogicModifier", obj->GetKeyName(), GetINode()->GetName());
	plKey logicKey = hsgResMgr::ResMgr()->NewKey(tmpName, logic, node->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(logicKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	fLogicModKeys[node] = logicKey;



return true;
}

void plInventoryObjComponent::AddReceiverKey(plKey key, plMaxNode* node)
{
	fReceivers.Append(key);
}

hsBool plInventoryObjComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

#include "plNoteTrackDlgComp.h"

class plInventoryObjComponentProc : public ParamMap2UserDlgProc
{
protected:
	plComponentNoteTrackDlg fNoteTrackDlgX;
	plComponentNoteTrackDlg fNoteTrackDlgY;
	IParamBlock2 *fPB;

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
//			fPB = map->GetParamBlock();
/*			
//			fNoteTrackDlgX.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX),
//								nil,
///								kClickDragAnimX,
/								nil,
								fPB,
								fPB->GetOwner());

			fNoteTrackDlgX.Load();

			EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX), true);
			
			fNoteTrackDlgY.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y),
								nil,
								kClickDragAnimY,
								nil,
								fPB,
								fPB->GetOwner());

			fNoteTrackDlgY.Load();

			EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y), true);
*/
		return TRUE;
/*
		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIMX)
			{
				fNoteTrackDlgX.AnimChanged();
				return TRUE;
			}
			if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIM_Y)
			{
				fNoteTrackDlgY.AnimChanged();
				return TRUE;
			}
			break;
			*/
		}

		return false;	
	}

	void DeleteThis()
	{
//		fNoteTrackDlgX.DeleteCache();
//		fNoteTrackDlgY.DeleteCache();
	}
};
static plInventoryObjComponentProc gInventoryObjComponentProc;
