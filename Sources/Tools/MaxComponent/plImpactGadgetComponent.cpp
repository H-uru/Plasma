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
#include "plImpactGadgetComponent.h"
#include "resource.h"
//#include "plComponent.h"
#include "plComponentReg.h"


#include "../pnSceneObject/plSceneObject.h"
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

#include "plResponderComponent.h"

#include "../MaxConvert/hsConverterUtils.h"		//Conversion Dependencies
#include "plPhysicalComponents.h"
#include "../pnMessage/plIntRefMsg.h"			//	Ibid
#include "plComponentProcBase.h"

#include "../MaxMain/plPhysicalProps.h"

void DummyCodeIncludeFuncImpactGadget() {}

// enum
// {
// 	kImpactObject_DEAD,
// 	kImpactOneShot,
// 	kUseImpactNode_DEAD,
// 	kImpactNode_DEAD,
// 	kImpactBoundsType_DEAD,
// 	kImpactBounceChoice_DEAD,
// 	kImpactBounceBoolTab_DEAD,
// 	kImpactReportChoice_DEAD,
// 	kImpactReportBoolTab_DEAD,
// 	kImpactEnabled,
// 	kImpactBounceGroups_DEAD,
// 	kImpactReportGroups,
// 	kImpactUseVelocity_DEAD,
// 	kImpactVelocity_DEAD,
// };
// 
// #include "plEventGroupUI.h"

// static plEventGroupProc gReportGroupProc(kImpactReportGroups, "Report collisions with these groups", false);

OBSOLETE_CLASS(plImpactGadget, gImpactGadgetDesc, "Collision Sensor",  "CollisionSensor", COMP_TYPE_DETECTOR, IMPACTGADGET_CID)

// enum
// {
// 	kImpactMain,
// 	kImpactBounce_DEAD,
// 	kImpactReport,
// };
// 
// ParamBlockDesc2 gImpactGadgetBlock
// (
// 	plComponent::kBlkComp, _T("ImpactGadgetComp"), 0, &gImpactGadgetDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,
// 
// 	2,
// 	kImpactMain, IDD_COMP_DETECTOR_COLLISION, IDS_COMP_DETECTOR_COLLISION, 0, 0, NULL,
// 	kImpactReport, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_REPORT, 0, APPENDROLL_CLOSED, &gReportGroupProc,
// 
// 	kImpactOneShot,		_T("oneshot"),		TYPE_BOOL,				0, 0,
// 		p_ui,			kImpactMain, 	TYPE_SINGLECHEKBOX,	IDC_ONESHOT,
// 		end,
// 
// 	kImpactReportGroups, _T("reportGroups"), TYPE_INT,	0,0,
// 		end,
// 
// 	kImpactEnabled,		_T("enabled"),		TYPE_BOOL,			0, 0,
// 		p_ui,	kImpactMain, TYPE_SINGLECHEKBOX, IDC_ENABLED,
// 		p_default, TRUE,
// 		end,
// 
// 	end
// );
// 
// 
// plImpactGadget::plImpactGadget()
// {
// 	fClassDesc = &gImpactGadgetDesc;
// 	fClassDesc->MakeAutoParamBlocks(this);
// }
// 
// hsBool plImpactGadget::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
// {
// 	plActivatorBaseComponent::SetupProperties(node, pErrMsg);
// 
// 	plPhysicalProps *physProps = node->GetPhysicalProps();
// 	physProps->SetReportGroup(plEventGroupProc::GetGroups(fCompPB, kImpactReportGroups), node, pErrMsg);
// 
// 	return true;
// }
// 
// hsBool plImpactGadget::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
// {
// 	plLocation loc = node->GetLocation();
// 	plSceneObject *obj = node->GetSceneObject();
// 	
// 	plKey logicKey = fLogicModKeys[node];
// 	plLogicModifier *logic = plLogicModifier::ConvertNoRef(logicKey->GetObjectPtr());
// 
// 	if (fCompPB->GetInt(kImpactOneShot))
// 		logic->SetFlag(plLogicModBase::kOneShot);
// 
// 	hsTArray<plKey> receivers;
// 	IGetReceivers(node, receivers);
// 	for (int i = 0; i < receivers.Count(); i++)
// 		logic->AddNotifyReceiver(receivers[i]);
// 
// 	// Get the physical node (where the mod is going to be put)
// 	plMaxNode* physNode = node->GetPhysicalProps()->GetProxyNode();
// 	if (!physNode)
// 		physNode = node;
// 
// 	// Create remote detector
// 	plCollisionDetector* det = TRACKED_NEW plCollisionDetector;
// 	det->SetType(plCollisionDetector::kTypeBump);
// 	
// 	// Register the detector
// 	plKey detKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), det, loc);
// 	hsgResMgr::ResMgr()->AddViaNotify(detKey, TRACKED_NEW plObjRefMsg(physNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
// 	
// 	// create and register the CONDITIONS for the DETECTOR's Logic Modifier
// 	plActivatorConditionalObject* activatorCond = TRACKED_NEW plActivatorConditionalObject;
// 	plKey activatorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), activatorCond, loc);
// 
// 	// link everything up:
// 	logic->AddCondition(activatorCond); // add this activator condition
// 	logic->SetDisabled(fCompPB->GetInt(kImpactEnabled) == 0);
// 
// 	// Set up the remote detector (if any)
// 	activatorCond->SetActivatorKey(detKey);
// 	det->AddLogicObj(logicKey);
// 	
// 	// If this is for the SceneViewer, set the local only flag since the read function will never be called
// 	if (plConvert::Instance().IsForSceneViewer())
// 		logic->SetLocalOnly(true);
// 
// 	return true;
// }
