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
#include "plClickableComponent.h"
#include "resource.h"
#include "plComponentReg.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plKey.h"

#include "../plPhysical/plCollisionDetector.h"	// MM
#include "../plModifier/plLogicModifier.h"
#include "../../NucleusLib/pnModifier/plConditionalObject.h"
#include "../plPhysical/plPickingDetector.h"
#include "../pfConditional/plActivatorConditionalObject.h"
#include "../pfConditional/plFacingConditionalObject.h"
#include "../pfConditional/plObjectInBoxConditionalObject.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plCursorChangeMsg.h"

#include "hsResMgr.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxConvert/plConvert.h"
#include "../MaxMain/plPhysicalProps.h"
#include "../plPhysical/plSimDefs.h"

#include "plResponderComponent.h"

#include "../MaxMain/plPhysicalProps.h"


void DummyCodeIncludeFuncClickable() {}

CLASS_DESC(plClickableComponent, gClickableDesc, "Clickable",  "Clickable", COMP_TYPE_DETECTOR, CLICKABLE_CID)

enum
{
	kClickableDirectional,
	kClickableDegrees,
	kClickableUseProxy,
	kClickableProxy,
	kClickableUseRegion,
	kClickableProxyRegion,
	kClickableToggle_DEAD,
	kClickableOneShot,
	kClickableBoundsType,
	kClickableEnabled,
	kClickablePhysical,
	kClickableIgnoreProxyRegion,
	kClickableFriction,
};

ParamBlockDesc2 gClickableBlock
(
	plComponent::kBlkComp, _T("clickable"), 0, &gClickableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DETECTOR_CLICKABLE, IDS_COMP_DETECTOR_CLICKABLE, 0, 0, NULL,

	kClickableDirectional,		_T("directional"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_OMNI,
		end,

	kClickableDegrees, _T("degrees"),	TYPE_INT,	P_ANIMATABLE, 0,	
		p_range, 1, 180,
		p_default, 180,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT, 
		IDC_COMP_CLICK_DEG,	IDC_COMP_CLICK_DEGSPIN, SPIN_AUTOSCALE,
		end,

	kClickableUseProxy,		_T("useProxy"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COMP_CLICK_USEPROXY,
		p_enable_ctrls, 1, kClickableProxy,
		end,
		

	kClickableProxy, _T("proxyPrimitave"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_PROXY,
//		p_sclassID,	 GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,

	kClickableProxyRegion, _T("proxyRegion"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_PROXYREGION,
//		p_sclassID,	 GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		end,
	
	kClickableOneShot,		_T("oneshot"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_ONESHOT,
		end,

	kClickableBoundsType,	_T("BoundingConditions"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
		p_vals,						plSimDefs::kSphereBounds,		plSimDefs::kBoxBounds,		plSimDefs::kHullBounds,		plSimDefs::kProxyBounds,
		p_default, plSimDefs::kHullBounds,
		end,

	kClickableEnabled,		_T("enabled"),		TYPE_BOOL,			0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_ENABLED,
		p_default, TRUE,
		end,

	kClickablePhysical,		_T("physical"),		TYPE_BOOL,				0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_COLLIDABLE_CHECK,
		p_default, TRUE,
		end,

	kClickableIgnoreProxyRegion, _T("ignoreProxyRegion"), TYPE_BOOL,	0, 0,
		p_ui,				TYPE_SINGLECHEKBOX,	IDC_IGNORE_REGION_CHECK,
		p_default, FALSE,
		end,

	kClickableFriction, _T("friction"),	TYPE_FLOAT,	0, 0,	
		p_range, 0.0f, FLT_MAX,
		p_default, 0.0f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT, 
		IDC_COMP_CLICKABLE_FRIC_EDIT1,	IDC_COMP_CLICKABLE_FRIC_SPIN1, SPIN_AUTOSCALE,
		end,
	end
);

plClickableComponent::plClickableComponent()
{
	fClassDesc = &gClickableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plClickableComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
	if( fCompPB->GetInt(kClickableUseProxy) )
	{
		INode* clickNode = fCompPB->GetINode(kClickableProxy);
		if( clickNode )
			nonDrawables.Append(1, &clickNode);
	}
	INode* detectNode = fCompPB->GetINode(kClickableProxyRegion);
	if( detectNode )
		nonDrawables.Append(1, &detectNode);

}

hsBool plClickableComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plActivatorBaseComponent::SetupProperties(node, pErrMsg);

	bool physical = (fCompPB->GetInt(kClickablePhysical) != 0);

	//
	// Phys Props for the Clickable itself.
	//
	plMaxNode *clickNode = node;
	if (fCompPB->GetInt(kClickableUseProxy))
	{
		clickNode = (plMaxNode*)fCompPB->GetINode(kClickableProxy);
		if (clickNode)
			clickNode->SetDrawable(false);
		else
			clickNode = node;
	}

	if (clickNode)
	{
		plPhysicalProps *physProps = clickNode->GetPhysicalProps();
		physProps->SetLOSUIItem(true, clickNode, pErrMsg);
		if (physical)
		{
			physProps->SetGroup(plSimDefs::kGroupStatic, clickNode, pErrMsg);
			// only if movable will it have mass (then it will keep track of movements in PhysX)
			if ( clickNode->IsMovable() || clickNode->IsTMAnimatedRecur() )
				physProps->SetMass(1.0, clickNode, pErrMsg);
			physProps->SetFriction(fCompPB->GetFloat(kClickableFriction),clickNode,pErrMsg);
		}
		else
		{
			physProps->SetGroup(plSimDefs::kGroupLOSOnly, clickNode, pErrMsg);
			if(clickNode->IsMovable() || clickNode->IsTMAnimatedRecur())
			{
				physProps->SetMass(1.0, clickNode, pErrMsg);

			}
		}
		physProps->SetBoundsType(fCompPB->GetInt(kClickableBoundsType), clickNode, pErrMsg);
	}

	//
	// Phys Properties for the auto-generated Detector Region...
	//
	plMaxNode* detectNode = (plMaxNode*)fCompPB->GetINode(kClickableProxyRegion);
	if (detectNode)
	{
		plPhysicalProps *physPropsDetector = detectNode->GetPhysicalProps();
//		physPropsDetector->SetAllowLOS(true, detectNode, pErrMsg);
		physPropsDetector->SetProxyNode(detectNode, node, pErrMsg);
		physPropsDetector->SetBoundsType(plSimDefs::kHullBounds, detectNode, pErrMsg);
		// only if movable will it have mass (then it will keep track of movements in PhysX)
		if ( detectNode->IsMovable() || detectNode->IsTMAnimatedRecur() )
			physPropsDetector->SetMass(1.0, detectNode, pErrMsg);

		physPropsDetector->SetGroup(plSimDefs::kGroupDetector, detectNode, pErrMsg );
		physPropsDetector->SetReportGroup(1<<plSimDefs::kGroupAvatar, detectNode, pErrMsg );
	}

	return true;
}

hsBool plClickableComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{

	plMaxNode *clickNode = node;
	if (fCompPB->GetInt(kClickableUseProxy))
	{
		clickNode = (plMaxNode*)fCompPB->GetINode(kClickableProxy);
		if (clickNode)
			clickNode->SetDrawable(false);
		else
			clickNode = node;
	}

	clickNode->SetForceLocal(true);

	plLocation loc = clickNode->GetLocation();
	plSceneObject *obj = clickNode->GetSceneObject();

	// Create and register the VolumeGadget's logic component
	plLogicModifier *logic = TRACKED_NEW plLogicModifier;
	plKey logicKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), logic, clickNode->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(logicKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	fLogicModKeys[clickNode] = logicKey;


	return true;
}


hsBool plClickableComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	bool ignoreProxyRegion = (fCompPB->GetInt(kClickableIgnoreProxyRegion) != 0);

	//
	// Error checking
	//
	plMaxNode* clickProxyNode = node;
	if (fCompPB->GetInt(kClickableUseProxy))
	{
		clickProxyNode = (plMaxNode*)fCompPB->GetINode(kClickableProxy);
		if (!clickProxyNode || !clickProxyNode->CanConvert())
		{
			pErrMsg->Set(true,
						"Clickable Error",
						"The Clickable '%s' on node '%s' is set to use a proxy but doesn't have one, or it didn't convert.\n"
						"The node the Clickable is attached to will be used instead.",
						GetINode()->GetName(), node->GetName()).Show();
			pErrMsg->Set(false);

			clickProxyNode = node;
		}
	}

	plMaxNode* detectNode = (plMaxNode*)fCompPB->GetINode(kClickableProxyRegion);
	if ((!detectNode || !detectNode->CanConvert()) && (!ignoreProxyRegion))
	{
		pErrMsg->Set(true,
					"Clickable Error",
					"The Clickable '%s' on node '%s' has a required region that is missing, or didn't convert.\n"
					"The export will be aborted.",
					GetINode()->GetName(), node->GetName()).Show();
		return false;
	}


	plLocation loc = clickProxyNode->GetLocation();
	plSceneObject *obj = clickProxyNode->GetSceneObject();
	
	plKey logicKey = fLogicModKeys[clickProxyNode];
	plLogicModifier *logic = plLogicModifier::ConvertNoRef(logicKey->GetObjectPtr());
	logic->fMyCursor = plCursorChangeMsg::kCursorPoised;
	
	if (fCompPB->GetInt(kClickableOneShot))
		logic->SetFlag(plLogicModBase::kOneShot);

	hsTArray<plKey> receivers;
	IGetReceivers(node, receivers);
	for (int i = 0; i < receivers.Count(); i++)
		logic->AddNotifyReceiver(receivers[i]);

		// Create the detector
	plDetectorModifier *detector = nil;
	detector = TRACKED_NEW plPickingDetector;

	// Register the detector
	plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
	hsgResMgr::ResMgr()->AddViaNotify(detectorKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	// create and register the CONDITIONS for the DETECTOR's Logic Modifier
	plActivatorConditionalObject* activatorCond = TRACKED_NEW plActivatorConditionalObject;
	plKey activatorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), activatorCond, loc);
	
	//
	// Create required region
	//
	// need a player in box condition here...
	// first a detector-any for the box
	if (!ignoreProxyRegion)
	{
		plObjectInVolumeDetector* pCDet = TRACKED_NEW plObjectInVolumeDetector(plCollisionDetector::kTypeAny);
		
		plKey pCDetKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pCDet, loc);
		hsgResMgr::ResMgr()->AddViaNotify(pCDetKey, TRACKED_NEW plObjRefMsg(detectNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
		pCDet->AddLogicObj(logicKey);

		// then an object-in-box condition for the logic mod
		plObjectInBoxConditionalObject* boxCond = TRACKED_NEW plObjectInBoxConditionalObject;
		plKey boxCondKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), boxCond, loc);
		logic->AddCondition(boxCond);
	}

	//
	// How do we feel about player facing
	//
	plFacingConditionalObject* facingCond = TRACKED_NEW plFacingConditionalObject;
	facingCond->SetDirectional(fCompPB->GetInt(kClickableDirectional));
	int deg = fCompPB->GetInt(kClickableDegrees);
	if (deg > 180)
		deg = 180;
	hsScalar rad = hsScalarDegToRad(deg);
	facingCond->SetTolerance(hsCosine(rad));
	plKey facingKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), facingCond, loc);
	
	detector->AddLogicObj(logicKey);	 // send messages to this logic component
	activatorCond->SetActivatorKey(detectorKey); // Tells the activator condition to look for stimulus from the detector

	logic->AddCondition(activatorCond); // add this activator condition
	logic->AddCondition(facingCond);

	logic->SetDisabled(!fCompPB->GetInt(kClickableEnabled));

	// If this is for the SceneViewer, set the local only flag since the read function will never be called
	if (plConvert::Instance().IsForSceneViewer())
		logic->SetLocalOnly(true);

	return true;
}



//
// special physical you can walk through and click with mouse
//

class plNoBlkClickableComponent : public plComponent
{
public:
	plNoBlkClickableComponent();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; } 

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }
};

OBSOLETE_CLASS_DESC(plNoBlkClickableComponent, gNoBlkClickableDesc, "(ex)Non Physical Clickable Proxy",  "(ex)Non Physical Clickable Proxy", COMP_TYPE_PHYSICAL, Class_ID(0x66325afc, 0x253a3760))

ParamBlockDesc2 gNoBlkClickableBlock
(
	plComponent::kBlkComp, _T("NonPhysicalClickableProxy"), 0, &gNoBlkClickableDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

	end
);

plNoBlkClickableComponent::plNoBlkClickableComponent()
{
	fClassDesc = &gNoBlkClickableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plNoBlkClickableComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return true;
}

