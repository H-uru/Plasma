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
#include "plClickDragComponent.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "plAnimComponent.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plKey.h"

#include "../plPhysical/plCollisionDetector.h"	// MM
#include "../plModifier/plLogicModifier.h"
#include "../plModifier/plAxisAnimModifier.h"
#include "../../NucleusLib/pnModifier/plConditionalObject.h"
#include "../plPhysical/plPickingDetector.h"
#include "../pfConditional/plActivatorConditionalObject.h"
#include "../pfConditional/plFacingConditionalObject.h"
#include "../pfConditional/plObjectInBoxConditionalObject.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plCursorChangeMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../plMessage/plAnimCmdMsg.h"

#include "hsResMgr.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxConvert/plConvert.h"

#include "plResponderComponent.h"
#include "plgDispatch.h"

#include "../MaxMain/plPhysicalProps.h"
#include "plNotetrackAnim.h"

#include "../plPhysical/plSimDefs.h"

void DummyCodeIncludeFuncClickDrag() {}

CLASS_DESC(plClickDragComponent, gClickDragDesc, "Dragable",  "Dragable", COMP_TYPE_DETECTOR, CLICK_DRAG_CID)


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
	kClickDragToggle,
	kClickDragAllOrNothing,
	kClickDragOneShot,
	kClikDragBoundsType,
	kClikDragEnabled,
};


class plClickDragComponentProc;
extern plClickDragComponentProc gClickDragComponentProc;

ParamBlockDesc2 gClickDragBlock
(
	
	plComponent::kBlkComp, _T("ClickDragComp"), 0, &gClickDragDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DETECTOR_CLICK_DRAGGABLE, IDS_COMP_DETECTOR_CLICK_DRAGGABLE, 0, 0, &gClickDragComponentProc,

	kClickDragDirectional,		_T("directional"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_OMNI,
		end,

	kClickDragDegrees, _T("degrees"),	TYPE_INT,	P_ANIMATABLE, 0,	
		p_range, 1, 180,
		p_default, 180,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT, 
		IDC_COMP_CLICK_DRAG_DEG,	IDC_COMP_CLICK_DRAG_DEGSPIN, SPIN_AUTOSCALE,
		end,

	kClickDragUseProxy,		_T("useProxy"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_USEPROXYPHYS,
		p_enable_ctrls, 1, kClickDragProxy,
		end,

	kClickDragProxy, _T("proxyPrimitave"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_DRAG_PROXY,
		p_sclassID,	 GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,

	kClickDragProxyRegion, _T("proxyRegion"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_DRAG_PROXYREGION,
		p_sclassID,	 GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,


	kClickDragUseX,		_T("useXAnim"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_USEX,
		end,
	
	kClickDragAnimX,	_T("XanimName"),		TYPE_STRING,	0, 0,
		end,

	kClickDragUseY,		_T("useYAnim"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_USEYANIM,
		end,
	
	kClickDragAnimY,	_T("YanimName"),		TYPE_STRING,	0, 0,
		end,

	kClickDragUseLoopX,		_T("useLoopXAnim"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_USE_LOOPX,
		end,
	
	kClickDragUseLoopY,		_T("useLoopYAnim"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_USE_LOOPY,
		end,

	kClickDragAllOrNothing,		_T("allOrNot"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_DRAG_ALLORNOT,
		end,

	kClickDragOneShot,		_T("oneshot"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_ONESHOT,
		end,

	kClikDragBoundsType,	_T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	kClikDragEnabled,		_T("enabled"),		TYPE_BOOL,			0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_ENABLE,
		p_default, TRUE,
		end,

	end
);

plClickDragComponent::plClickDragComponent()
{
	fClassDesc = &gClickDragDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

const plClickDragComponent::LogicKeys& plClickDragComponent::GetAxisKeys()
{
	return fAxisKeys;
}

plKey plClickDragComponent::GetAxisKey(plMaxNode* node)
{
	LogicKeys::const_iterator it = fAxisKeys.find(node);
	if (it != fAxisKeys.end())
		return(it->second);

	return nil;
}

void plClickDragComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	INode* boundsNode = fCompPB->GetINode(kClickDragProxy);
	if(boundsNode && fCompPB->GetInt(kClickDragUseProxy))
		nonDrawables.Append(1, &boundsNode);

	boundsNode = fCompPB->GetINode(kClickDragProxyRegion);
	if(boundsNode )
		nonDrawables.Append(1, &boundsNode);

}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plClickDragComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{

	plActivatorBaseComponent::SetupProperties(node, pErrMsg);

	// Phys Props for the Clickable itself.
	plMaxNode *boundsNode = nil;
	boundsNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxy);
	if(boundsNode && fCompPB->GetInt(kClickDragUseProxy))
		if(boundsNode->CanConvert())
		{
			boundsNode->SetDrawable(false);
			plPhysicalProps *physProps = boundsNode->GetPhysicalProps();
			// only if movable will it have mass (then it will keep track of movements in PhysX)
			if ( boundsNode->IsMovable() || boundsNode->IsTMAnimatedRecur() )
				physProps->SetMass(1.0, boundsNode, pErrMsg);
			physProps->SetGroup( plSimDefs::kGroupStatic, boundsNode, pErrMsg);
//			physProps->SetReportGroup( plPhysicsGroups::kLocalAvatars, boundsNode, pErrMsg);
			physProps->SetBoundsType(fCompPB->GetInt(kClikDragBoundsType), boundsNode, pErrMsg);
		}
		else
		{
				pErrMsg->Set(true, "Clickable Sensor Warning", "The Clickable %s has a Proxy Surface %s that was Ignored.\nThe Sensors geometry will be used instead.", node->GetName(), boundsNode->GetName()).Show();
				pErrMsg->Set(false);
		}
	else
	{	
		
		plPhysicalProps *physProps = node->GetPhysicalProps();
		// only if movable will it have mass (then it will keep track of movements in PhysX)
		if ( node->IsMovable() || node->IsTMAnimatedRecur() )
			physProps->SetMass(1.0, node, pErrMsg);
		physProps->SetGroup( plSimDefs::kGroupStatic, node, pErrMsg);
//		physProps->SetReportGroup( plPhysicsGroups::kLocalAvatars, node, pErrMsg);
		//node->GetPhysicalProps()->SetAllCollideGroups(0);
		physProps->SetBoundsType(fCompPB->GetInt(kClikDragBoundsType), node, pErrMsg);
	}
	// Phys Properties for the auto-generated Detector Region...
	boundsNode = nil;
	boundsNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxyRegion);
	if(boundsNode)
	{
		if(boundsNode->CanConvert())
		{
			plPhysicalProps *physPropsDetector = boundsNode->GetPhysicalProps();
//			physPropsDetector->SetAllowLOS(true, boundsNode, pErrMsg);
			physPropsDetector->SetProxyNode(boundsNode, node, pErrMsg);
			physPropsDetector->SetBoundsType(plSimDefs::kHullBounds, boundsNode, pErrMsg);
			// only if movable will it have mass (then it will keep track of movements in PhysX)
			if ( boundsNode->IsMovable() || boundsNode->IsTMAnimated() )
				physPropsDetector->SetMass(1.0, boundsNode, pErrMsg);

			physPropsDetector->SetGroup( plSimDefs::kGroupDetector, boundsNode, pErrMsg );
			//boundsNode->GetPhysicalProps()->SetAllCollideGroups(0);
			physPropsDetector->SetReportGroup( 1<<plSimDefs::kGroupAvatar, boundsNode, pErrMsg );
		}
	}
	else
	{
		pErrMsg->Set(true, "Clickable Sensor Error", "The Clickable Sensor %s has a Required Region that is missing.\nThe Export will be aborted", node->GetName()).Show();
//		pErrMsg->Set(false);
		return false;
	}
	
	fAxisKeys.clear();
	return true;
}

hsBool plClickDragComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plActivatorBaseComponent::PreConvert(node, pErrMsg);
	plLogicModifier *logic = plLogicModifier::ConvertNoRef(fLogicModKeys[node]->GetObjectPtr());

	if (fCompPB->GetInt(kClickDragOneShot))
		logic->SetFlag(plLogicModBase::kOneShot);

	plLocation loc = node->GetLocation();
	plSceneObject *obj = node->GetSceneObject();

	// do the same thing for axis animation controllers.
	plAxisAnimModifier* pAxis = TRACKED_NEW plAxisAnimModifier;
	plKey axisKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pAxis, loc);
	hsgResMgr::ResMgr()->AddViaNotify(axisKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
	logic->AddNotifyReceiver(axisKey);
	
	fAxisKeys[node] = axisKey;

	return true;
}

hsBool plClickDragComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plLocation loc = node->GetLocation();
	plSceneObject *obj = node->GetSceneObject();
	
	plKey logicKey = fLogicModKeys[node];
	plLogicModifier *logic = plLogicModifier::ConvertNoRef(logicKey->GetObjectPtr());
	logic->fMyCursor = plCursorChangeMsg::kCursorOpen;

		// Create the detector
	plDetectorModifier *detector = nil;
	detector = TRACKED_NEW plPickingDetector;

	// Register the detector
	plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
	hsgResMgr::ResMgr()->AddViaNotify(detectorKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	// set up the axis anim controller
	
	plKey axisKey = fAxisKeys[node];
	plAxisAnimModifier* pAxis = plAxisAnimModifier::ConvertNoRef(axisKey->GetObjectPtr());
	// attach the animation controller to the animation objects:
	// find an animation controller:
	
	hsTArray<plKey> receivers;
	IGetReceivers(node, receivers);
	
	int i;
	for (i = 0; i < receivers.Count(); i++)
		pAxis->GetNotify()->AddReceiver(receivers[i]);

	pAxis->SetNotificationKey(logicKey);
	UInt32 count = node->NumAttachedComponents();
	hsBool bHasAnim = false;
	plAnimComponentBase* pAnim = nil;

	for (i = 0; i < count; i++)
	{
		plComponentBase *comp = node->GetAttachedComponent(i);
		if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
		{
			pAnim = (plAnimComponentBase*)comp;
			break;
		}
	}
	if (!pAnim)
	{
		pErrMsg->Set(true, "WARNING", "Object %s has click-drag component attached but NO animation component!", ((INode*)node)->GetName()).Show();
		pErrMsg->Set(false);
	}
	else
	{
		if (fCompPB->GetInt(kClickDragUseX))
		{
			pAxis->SetXAnim( pAnim->GetModKey(node) );
		}	
		else // take out this else when we support multiple channels
		if (fCompPB->GetInt(kClickDragUseY))
		{
			pAxis->SetYAnim( pAnim->GetModKey(node) );
		}
		pAxis->SetAllOrNothing(fCompPB->GetInt(kClickDragAllOrNothing));

		// add callbacks for beginning and end of animation
		plEventCallbackMsg* pCall1 = TRACKED_NEW plEventCallbackMsg;
		pCall1->fEvent = kBegin;
		pCall1->fRepeats = -1;
		pCall1->AddReceiver(axisKey);
		
		plEventCallbackMsg* pCall2 = TRACKED_NEW plEventCallbackMsg;
		pCall2->fEvent = kEnd;
		pCall2->fRepeats = -1;
		pCall2->AddReceiver(axisKey);

		plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
		const char *tempAnimName = pAnim->GetAnimName();
		if (tempAnimName == nil)
		{
			//pMsg->SetAnimName(ENTIRE_ANIMATION_NAME);
			pMsg->SetAnimName(pAnim->GetModKey(node)->GetName());
			pAxis->SetAnimLabel(ENTIRE_ANIMATION_NAME);
		}
		else
		{
			//pMsg->SetAnimName(tempAnimName);
			pMsg->SetAnimName(pAnim->GetModKey(node)->GetName());
			pAxis->SetAnimLabel(tempAnimName);
		}
		

		pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
		pMsg->AddCallback(pCall1);
		pMsg->AddCallback(pCall2);

		hsRefCnt_SafeUnRef( pCall1 );
		hsRefCnt_SafeUnRef( pCall2 );

		pMsg->AddReceiver( pAnim->GetModKey(node) );
		plgDispatch::MsgSend(pMsg);
	}


	// is this a using a proxy primitive?
	plPickingDetector* det2 = nil;
	plKey det2Key  = nil;
	plMaxNode* pProxyNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxy);
	
	if (pProxyNode && fCompPB->GetInt(kClickDragUseProxy))
	{
		
		// verify that there is a physical proxy attached to this scene object:
		UInt32 count = ((plMaxNodeBase*)pProxyNode)->NumAttachedComponents();
		hsBool bHasPhys = false;
//		for (UInt32 i = 0; i < count; i++)
		//		{
		//			plComponentBase *comp = ((plMaxNodeBase*)pProxyNode)->GetAttachedComponent(i);
		//			if (comp->ClassID() == Class_ID(0x11e81ee4, 0x36b81450))
		//			{
		//				bHasPhys = true;
		//				break;
		//			}
		//		}
		//		if (!bHasPhys)
		//		{
		//			pErrMsg->Set(true, "WARNING", "Object %s listed as draggable component proxy physical for %s but has NO physical component.\n  Please attach a proxyTerrain componet!\n Export will continue but this gadget will not function",pProxyNode->GetName(), ((INode*)node)->GetName()).Show();
		//			pErrMsg->Set(false);
		//		}
		

		if(pProxyNode->CanConvert())
		{
			det2 = TRACKED_NEW plPickingDetector;
			// Register the detector
			det2Key = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), det2, loc);
			hsgResMgr::ResMgr()->AddViaNotify(det2Key, TRACKED_NEW plObjRefMsg(((plMaxNode*)pProxyNode)->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
			hsgResMgr::ResMgr()->AddViaNotify(logicKey, TRACKED_NEW plObjRefMsg( det2Key, plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
			det2->SetProxyKey(node->GetSceneObject()->GetKey());
		}
		else
		{
			pErrMsg->Set(true, "Unknown Error", "Invalid proxy physical detector set for draggable %s.", ((INode*)pProxyNode)->GetName()).Show();
			pErrMsg->Set(false);
			return false;
		}
	
	}



	// create and register the CONDITIONS for the DETECTOR's Logic Modifier
	plActivatorConditionalObject* activatorCond = TRACKED_NEW plActivatorConditionalObject;
	plKey activatorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), activatorCond, loc);

	// do we have a required region?
	plMaxNode* pProxyRegNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxyRegion);
	if (pProxyRegNode)
	{
		// verify that there is a physical detector attached to this scene object:
		UInt32 count = ((plMaxNodeBase*)pProxyRegNode)->NumAttachedComponents();
		hsBool bHasPhys = false;
//		for (UInt32 i = 0; i < count; i++)
		//		{
		//			plComponentBase *comp = ((plMaxNodeBase*)pProxyRegNode)->GetAttachedComponent(i);
		//			if (comp->ClassID() == Class_ID(0x33b60376, 0x7e5163e0))
		//			{
		//				bHasPhys = true;
		//				break;
		//			}
		//		}
		//		if (!bHasPhys)
		//		{
		//			pErrMsg->Set(true, "WARNING", "Object %s listed as draggable component detector region for %s but has NO physical detector component!\n  Please attach a detector componet.\n Export will continue but this gadget will not function",((INode*)pProxyRegNode)->GetName(), ((INode*)node)->GetName()).Show();
		//			pErrMsg->Set(false);
		//		}
		

		if(pProxyRegNode->CanConvert())
		{
			// need a player in box condition here...
			// first a detector-any for the box
			plObjectInVolumeDetector* pCDet = TRACKED_NEW plObjectInVolumeDetector(plCollisionDetector::kTypeAny);
			plKey cDetKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pCDet, loc);
			hsgResMgr::ResMgr()->AddViaNotify(cDetKey, TRACKED_NEW plObjRefMsg(((plMaxNode*)pProxyRegNode)->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
			pCDet->AddLogicObj(logicKey);
			// then an object-in-box condition for the logic mod
			plObjectInBoxConditionalObject* boxCond = TRACKED_NEW plObjectInBoxConditionalObject;
			plKey boxCondKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), boxCond, loc);
			logic->AddCondition(boxCond);
		}
		else
		{		
			pErrMsg->Set(true, "Problem with region", "Can't convert region component on  %s.  This component will not be exported.\n", ((INode*)pProxyRegNode)->GetName()).Show();
			pErrMsg->Set(false);
			return false;
		}
	}
	else
	{
		pErrMsg->Set(true, "Must specify trigger region", "No required trigger region specified for click-drag component on %s.  This component will not be exported.\n", ((INode*)node)->GetName()).Show();
		pErrMsg->Set(false);
		return false;
	}


	// How do we feel about player facing
	plFacingConditionalObject* facingCond = TRACKED_NEW plFacingConditionalObject;
	facingCond->SetDirectional(fCompPB->GetInt(kClickDragDirectional));
	int deg = fCompPB->GetInt(kClickDragDegrees);
	if (deg > 180)
		deg = 180;
	hsScalar rad = hsScalarDegToRad(deg);
	facingCond->SetTolerance(hsCosine(rad));
	plKey facingKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), facingCond, loc);
	
	
	// link everything up:
	if (det2) // set up the remote detector (if any)
	{
		activatorCond->SetActivatorKey(det2Key);
		det2->AddLogicObj(logicKey);
	}
	else
	{
		detector->AddLogicObj(logicKey);	 // send messages to this logic component
		activatorCond->SetActivatorKey(detectorKey); // Tells the activator condition to look for stimulus from the detector
	}
	logic->AddCondition(activatorCond); // add this activator condition
	logic->AddCondition(facingCond);
	logic->SetDisabled(fCompPB->GetInt(kClikDragEnabled) == 0);

	
	// If this is for the SceneViewer, set the local only flag since the read function will never be called
	if (plConvert::Instance().IsForSceneViewer())
		logic->SetLocalOnly(true);

	return true;
}

hsBool plClickDragComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
	fAxisKeys.clear();
	return plActivatorBaseComponent::DeInit( node, pErrMsg ); 
}

#include "plNoteTrackDlgComp.h"

class plClickDragComponentProc : public ParamMap2UserDlgProc
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
			fPB = map->GetParamBlock();
			
			fNoteTrackDlgX.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX),
								nil,
								kClickDragAnimX,
								-1,
								fPB,
								fPB->GetOwner());

			fNoteTrackDlgX.Load();

			EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX), true);
			
			fNoteTrackDlgY.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y),
								nil,
								kClickDragAnimY,
								-1,
								fPB,
								fPB->GetOwner());

			fNoteTrackDlgY.Load();

			EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y), true);
			return TRUE;

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
		}
		return false;	
	}

	void DeleteThis()
	{
		fNoteTrackDlgX.DeleteCache();
		fNoteTrackDlgY.DeleteCache();
	}
};
static plClickDragComponentProc gClickDragComponentProc;
