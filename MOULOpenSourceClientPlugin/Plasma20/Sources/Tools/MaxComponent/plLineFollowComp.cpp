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

#include "../MaxMain/plMaxNode.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../plInterp/plController.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnKeyedObject/plKey.h"

#include "plgDispatch.h"
#include "../MaxMain/plPluginResManager.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

// Swivel related
#include "../pfAnimation/plViewFaceModifier.h" // ViewFace Comp

// Line Follow related
#include "../plInterp/plAnimPath.h"
#include "../pfAnimation/plLineFollowMod.h"
#include "../pfAnimation/plFollowMod.h"

#include "../pnMessage/plRefMsg.h"

// Stereizer
#include "../pfAnimation/plStereizer.h"

const Class_ID STEREIZE_COMP_CID(0x15066ec7, 0x64ea7381);
const Class_ID LINEFOLLOW_COMP_CID(0x64ec57f6, 0x292d47f6);
const Class_ID SWIVEL_COMP_CID(0x106a466b, 0x1c1700f7);

void DummyCodeIncludeFuncLineFollow()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LineFollow Component
//
//

enum	
{
	kFollowModeRadio,
	kPathObjectSel,
	kFollowObjectSel,
	kOffsetActive,
	kOffsetDegrees,
	kOffsetClampActive,
	kOffsetClamp,
	kForceToLine,
	kSpeedClampActive,
	kSpeedClamp
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plLineObjAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if( (id == kFollowObjectSel) || (id == kPathObjectSel) )
		{
			plComponentBase* comp = (plComponentBase*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plLineObjAccessor gLineObjAccessor;


class plLineFollowComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
					IParamBlock2* pb = map->GetParamBlock();
					map->SetTooltip(kPathObjectSel, TRUE, "Press the button, & select the path source object in one of the Viewports" );
					map->SetTooltip(kFollowObjectSel, TRUE, "Press the button, & select the object to follow in one of the Viewports" );
					map->SetTooltip(kOffsetDegrees, TRUE, "Positive angle to right, negative to left." );
					if( pb->GetInt(kFollowModeRadio) == plLineFollowMod::kFollowObject )
						map->Enable(kFollowObjectSel, TRUE);
					else
						map->Enable(kFollowObjectSel, FALSE);
			}
			return true;

//////////////////
		case WM_COMMAND:
			{
				if( (LOWORD(wParam) == IDC_RADIO_LISTENER)
					|| (LOWORD(wParam) == IDC_RADIO_CAMERA)
					|| (LOWORD(wParam) == IDC_RADIO_OBJECT) )
				{
					IParamBlock2* pb = map->GetParamBlock();
					if( pb->GetInt(kFollowModeRadio) == plLineFollowMod::kFollowObject )
						map->Enable(kFollowObjectSel, TRUE);
					else
						map->Enable(kFollowObjectSel, FALSE);
					
					return true;
				}
			}
			
		}

		return false;
	}
	void DeleteThis() {}
};
static plLineFollowComponentProc gLineFollowProc;


//Class that accesses the paramblock below.
class plLineFollowComponent : public plComponent
{
private:
	hsBool			fValid;

	plLineFollowMod*	fLineMod;

	hsBool		IMakeLineMod(plMaxNode* pNode, plErrorMsg* pErrMsg);

public:
	plLineFollowComponent();

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	plLineFollowMod*	GetLineMod(plErrorMsg* pErrMsg);
};



CLASS_DESC(plLineFollowComponent, gLineFollowDesc, "LineFollow",  "LineFollow", COMP_TYPE_GRAPHICS, LINEFOLLOW_COMP_CID)



ParamBlockDesc2 gLineFollowBk
(
	plComponent::kBlkComp, _T("LineFollow"), 0, &gLineFollowDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_LINEFOLLOW, IDS_COMP_LINEFOLLOWS,  0, 0, &gLineFollowProc,

	kFollowModeRadio, _T("FollowMode"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 3,	IDC_RADIO_LISTENER,					IDC_RADIO_CAMERA,				IDC_RADIO_OBJECT,	
		p_vals,						plLineFollowMod::kFollowListener,	plLineFollowMod::kFollowCamera,	plLineFollowMod::kFollowObject,		
		p_default, plLineFollowMod::kFollowListener,
		end,

	kPathObjectSel, _T("PathObjectChoice"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_LINE_CHOOSE_PATH,
		p_prompt, IDS_COMP_LINE_CHOSE_PATH,
		p_accessor, &gLineObjAccessor,
		end,

	kFollowObjectSel, _T("FollowObjectChoice"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_LINE_CHOOSE_OBJECT,
		p_prompt, IDS_COMP_LINE_CHOSE_OBJECT,
		p_accessor, &gLineObjAccessor,
		end,

	kOffsetActive,  _T("OffsetActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LINE_OFFSETACTIVE,
		p_enable_ctrls,		4, kOffsetDegrees, kOffsetClampActive, kOffsetClamp, kForceToLine,
		end,

	kOffsetDegrees, _T("OffsetDegrees"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -85.0, 85.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_LINE_OFFSETDEGREES, IDC_COMP_LINE_OFFSETDEGREES_SPIN, 1.0,
		end,	
	
	kOffsetClampActive,  _T("OffsetClampActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LINE_OFFSETCLAMPACTIVE,
		p_enable_ctrls,		1, kOffsetClamp,
		end,

	kOffsetClamp, _T("OffsetClamp"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LINE_OFFSETCLAMP, IDC_COMP_LINE_OFFSETCLAMP_SPIN, 1.0,
		end,	
	
	kForceToLine,  _T("ForceToLine"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LINE_FORCETOLINE,
		end,

	kSpeedClampActive,  _T("SpeedClampActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LINE_SPEEDCLAMPACTIVE,
		p_enable_ctrls,		1, kSpeedClamp,
		end,

	kSpeedClamp, _T("SpeedClamp"), TYPE_FLOAT, 	0, 0,	
		p_default, 30.0,
		p_range, 3.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LINE_SPEEDCLAMP, IDC_COMP_LINE_SPEEDCLAMP_SPIN, 1.0,
		end,	
	
	end

);

plLineFollowComponent::plLineFollowComponent()
:	fValid(false),
	fLineMod(nil)
{
	fClassDesc = &gLineFollowDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plLineFollowComponent::IMakeLineMod(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
	plLineFollowMod::FollowMode mode = plLineFollowMod::FollowMode(fCompPB->GetInt(kFollowModeRadio));

	plLineFollowMod* lineMod = TRACKED_NEW plLineFollowMod;
	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), lineMod, pNode->GetLocation());

	if( plLineFollowMod::kFollowObject == mode )
	{
		if(fCompPB->GetINode(kFollowObjectSel) != NULL)


		{
			plMaxNode* targNode = (plMaxNode*)fCompPB->GetINode(kFollowObjectSel);
			//plMaxNodeData* pMD = targNode->GetMaxNodeData();
			if( targNode->CanConvert() )
			{
				plSceneObject* targObj = targNode->GetSceneObject();
				if( targObj )
				{
					plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(lineMod->GetKey(), plRefMsg::kOnCreate, 0, plLineFollowMod::kRefObject);
					hsgResMgr::ResMgr()->AddViaNotify(targObj->GetKey(), refMsg, plRefFlags::kPassiveRef);

					lineMod->SetFollowMode(plLineFollowMod::kFollowObject);
				}
			}
		}
	}
	else
	{
		lineMod->SetFollowMode(mode);
	}

	plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kPathObjectSel);
	if(!pathNode)
	{
		pErrMsg->Set(true, "Path Node Failure", "Path Node %s was set to be Ignored or empty. Path Component ignored.", ((INode*)pathNode)->GetName()).Show();
		pErrMsg->Set(false);
		return false;
	}
	//hsAssert(pathNode, "If valid is true, this must be set");
	Control* maxTm = pathNode->GetTMController();

	plCompoundController* tmc = hsControlConverter::Instance().MakeTransformController(maxTm, pathNode);

	Matrix3 w2p(true);
	Matrix3 l2w = pathNode->GetNodeTM(TimeValue(0));
	if( !pathNode->GetParentNode()->IsRootNode() )
		w2p = Inverse(pathNode->GetParentNode()->GetNodeTM(TimeValue(0)));
	hsMatrix44 loc2Par = pathNode->Matrix3ToMatrix44(l2w * w2p);

	gemAffineParts ap;
	decomp_affine(loc2Par.fMap, &ap); 
	hsAffineParts initParts;
	AP_SET(initParts, ap);

	plAnimPath* animPath = TRACKED_NEW plAnimPath;
	animPath->SetController(tmc);
	animPath->InitParts(initParts);

	lineMod->SetPath(animPath);

	if( !pathNode->GetParentNode()->IsRootNode() )
	{
		plMaxNode* parNode = (plMaxNode*)pathNode->GetParentNode();
		plSceneObject* parObj = parNode->GetSceneObject();
		if( parObj )
		{
			plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(lineMod->GetKey(), plRefMsg::kOnCreate, 0, plLineFollowMod::kRefParent);
			hsgResMgr::ResMgr()->AddViaNotify(parObj->GetKey(), refMsg, plRefFlags::kPassiveRef);
		}
	}

	if( fCompPB->GetInt(kOffsetActive) )
	{
		lineMod->SetOffsetDegrees(fCompPB->GetFloat(kOffsetDegrees));
		
		if( fCompPB->GetInt(kOffsetClampActive) )
		{
			lineMod->SetOffsetClamp(fCompPB->GetFloat(kOffsetClamp));
		}

		lineMod->SetForceToLine(fCompPB->GetInt(kForceToLine));
	}

	if( fCompPB->GetInt(kSpeedClampActive) )
	{
		lineMod->SetSpeedClamp(fCompPB->GetFloat(kSpeedClamp));
	}

	fLineMod = lineMod;

	return true;
}

plLineFollowMod* plLineFollowComponent::GetLineMod(plErrorMsg* pErrMsg)
{
	if( !fValid )
		return nil;
	if( !fLineMod )
	{
		int i;
		for( i = 0; i < NumTargets(); i++ )
		{
			plMaxNode* targ = (plMaxNode*)GetTarget(i);
			IMakeLineMod(targ, pErrMsg);
			if( fLineMod )
				break;
		}
	}
	return fLineMod;
}

hsBool plLineFollowComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	if( !fLineMod )
	{
		if( !IMakeLineMod(node, pErrMsg) )
		{
			fValid = false;
			return true;
		}
	}

	node->AddModifier(fLineMod, IGetUniqueName(node));

	return true;
}

hsBool plLineFollowComponent::SetupProperties(plMaxNode* pNode,  plErrorMsg* pErrMsg)
{
	fValid = false;
	fLineMod = nil;

	if( !fCompPB->GetINode(kPathObjectSel) )
	{
		return true;
	}
	plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kPathObjectSel);
	if( !pathNode )
	{
		return true;
	}
	if( !pathNode->IsTMAnimated() )
	{
		return true;
	}
	pathNode->SetCanConvert(false);

	if( plLineFollowMod::kFollowObject == fCompPB->GetInt(kFollowModeRadio) )
	{
		if( !fCompPB->GetINode(kFollowObjectSel) )
		{
			return true;
		}
	}
	fValid = true;
	pNode->SetForceLocal(true);
	pNode->SetMovable(true);
	return true;
}

hsBool plLineFollowComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;
	fValid = false;

	if( plLineFollowMod::kFollowObject == fCompPB->GetInt(kFollowModeRadio) )
	{
		plMaxNode* followNode = (plMaxNode*)fCompPB->GetINode(kFollowObjectSel);
		if( !followNode->CanConvert() )
		{
			return true;
		}
	}
	plMaxNode* pathNode = (plMaxNode*)fCompPB->GetINode(kPathObjectSel);
	if( !pathNode->GetParentNode()->IsRootNode() )
	{
		if( !((plMaxNode*)pathNode->GetParentNode())->CanConvert() )
		{
			return true;
		}
	}
	fValid = true;


	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The stereizer component is sort of related to LineFollow. Really.

class plStereizeComp : public plComponent
{
public:
	enum
	{
		kLeft,
		kAmbientDist,
		kTransition,
		kSepAngle,
		kMaxDist,
		kMinDist
	};

	plLineFollowMod*	ISetMaster(plStereizer* stereo, plMaxNode* node, plErrorMsg* pErrMsg);

public:
	plStereizeComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	hsBool	Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg);

};



CLASS_DESC(plStereizeComp, gStereizeDesc, "Stereo-ize",  "Stereize", COMP_TYPE_AUDIO, STEREIZE_COMP_CID)

ParamBlockDesc2 gStereizeBk
(	
	plComponent::kBlkComp, _T("Stereize"), 0, &gStereizeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_STEREIZE, IDS_COMP_STEREIZE, 0, 0, NULL,

	plStereizeComp::kLeft,	_T("Left"),	TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_STEREIZE_LEFT,
		end,

	plStereizeComp::kAmbientDist, _T("AmbientDist"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_STEREIZE_AMB, IDC_COMP_STEREIZE_AMB_SPIN, 1.0,
		end,	

	plStereizeComp::kTransition, _T("Transition"), TYPE_FLOAT, 	0, 0,	
		p_default, 25.0,
		p_range, 1.0, 500.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_STEREIZE_TRANS, IDC_COMP_STEREIZE_TRANS_SPIN, 1.0,
		end,	

	plStereizeComp::kSepAngle, _T("SepAngle"), TYPE_FLOAT, 	0, 0,	
		p_default, 30.0,
		p_range, 1.0, 80.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_STEREIZE_ANG, IDC_COMP_STEREIZE_ANG_SPIN, 1.0,
		end,	

	plStereizeComp::kMaxDist, _T("MaxDist"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 1.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_STEREIZE_MAXDIST, IDC_COMP_STEREIZE_MAXDIST_SPIN, 1.0,
		end,	

	plStereizeComp::kMinDist, _T("MinDist"), TYPE_FLOAT, 	0, 0,	
		p_default, 5.0,
		p_range, 1.0, 50.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_STEREIZE_MINDIST, IDC_COMP_STEREIZE_MINDIST_SPIN, 1.0,
		end,	

	end
);

plStereizeComp::plStereizeComp()
{
	fClassDesc = &gStereizeDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plStereizeComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	if( !node->GetParentNode()->IsRootNode() )
		((plMaxNode*)node->GetParentNode())->SetForceLocal(true);

	return true;
}

hsBool plStereizeComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plStereizeComp::Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg)
{
	pErrMsg->Set(true, node->GetName(), msg).CheckAndAsk();
	pErrMsg->Set(false);
	return true;
}

hsBool plStereizeComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plStereizer* stereo = TRACKED_NEW plStereizer;

	stereo->SetAmbientDist(fCompPB->GetFloat(kAmbientDist));
	stereo->SetTransition(fCompPB->GetFloat(kTransition));

	hsScalar ang = fCompPB->GetFloat(kSepAngle);
	if( ang > 80.f )
		ang = 80.f;
	stereo->SetSepAngle(hsScalarDegToRad(ang));

	stereo->SetMaxSepDist(fCompPB->GetFloat(kMaxDist));
	stereo->SetMinSepDist(fCompPB->GetFloat(kMinDist));

	stereo->SetParentInitPos(node->GetLocalToParent44().GetTranslate());

	stereo->SetAsLeftChannel(fCompPB->GetInt(kLeft));

	node->AddModifier(stereo, IGetUniqueName(node));

	// Do this after AddModifier, cuz that's when stereo gets a plKey.
	ISetMaster(stereo, node, pErrMsg);

	return true;
}

plLineFollowMod* plStereizeComp::ISetMaster(plStereizer* stereo, plMaxNode* node, plErrorMsg* pErrMsg)
{
	int numComp = node->NumAttachedComponents(false);

	int i;
	for( i = 0; i < numComp; i++ )
	{
		plComponentBase *comp = node->GetAttachedComponent(i);
		if( comp && (comp->ClassID() == LINEFOLLOW_COMP_CID) )
		{
			plLineFollowComponent* lineComp = (plLineFollowComponent*)comp;
			plLineFollowMod* lineMod = lineComp->GetLineMod(pErrMsg);
			if( lineMod )
			{
				lineMod->AddStereizer(stereo->GetKey());
				return lineMod;
			}
		}
	}
	return nil;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The swivel component is also sort of related to LineFollow. And stereizer. Really.

class plSwivelComp : public plComponent
{
public:
	enum
	{
		kFaceTypeRadio,
		kFaceObjectSel,
		kPivotY,
		kOffsetActive,
		kOffsetX,
		kOffsetY,
		kOffsetZ,
		kOffsetLocal
	};
	enum
	{
		kFaceCamera,
		kFaceListener,
		kFacePlayer,
		kFaceObject
	};

public:
	plSwivelComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	hsBool	Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg);

};


class plSwivelComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
					IParamBlock2* pb = map->GetParamBlock();
					if( pb->GetInt(plSwivelComp::kFaceTypeRadio) == plSwivelComp::kFaceObject )
						map->Enable(plSwivelComp::kFaceObjectSel, TRUE);
					else
						map->Enable(plSwivelComp::kFaceObjectSel, FALSE);
			}
			return true;

//////////////////
		case WM_COMMAND:
			{
				if( (LOWORD(wParam) == IDC_RADIO_LISTENER)
					|| (LOWORD(wParam) == IDC_RADIO_PLAYER)
					|| (LOWORD(wParam) == IDC_RADIO_CAMERA)
					|| (LOWORD(wParam) == IDC_RADIO_OBJECT) )
				{
					IParamBlock2* pb = map->GetParamBlock();
					if( pb->GetInt(plSwivelComp::kFaceTypeRadio) == plSwivelComp::kFaceObject )
						map->Enable(plSwivelComp::kFaceObjectSel, TRUE);
					else
						map->Enable(plSwivelComp::kFaceObjectSel, FALSE);
					
					return true;
				}
			}
			
		}

		return false;
	}
	void DeleteThis() {}
};
static plSwivelComponentProc gSwivelProc;


CLASS_DESC(plSwivelComp, gSwivelDesc, "Swivel",  "Swivel", COMP_TYPE_GRAPHICS, SWIVEL_COMP_CID)

ParamBlockDesc2 gSwivelBk
(	
	plComponent::kBlkComp, _T("Swivel"), 0, &gSwivelDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SWIVEL, IDS_COMP_SWIVEL, 0, 0, &gSwivelProc,

	plSwivelComp::kFaceTypeRadio, _T("FaceType"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_CAMERA, IDC_RADIO_LISTENER, IDC_RADIO_PLAYER, IDC_RADIO_OBJECT,	
		p_vals,						plSwivelComp::kFaceCamera, plSwivelComp::kFaceListener, plSwivelComp::kFacePlayer, plSwivelComp::kFaceObject,
		p_default, plSwivelComp::kFacePlayer,
		end,

	plSwivelComp::kFaceObjectSel, _T("FaceObjectChoice"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CHOOSE_OBJECT,
		p_prompt, IDS_COMP_CHOOSE_OBJECT,
		end,

	plSwivelComp::kPivotY,  _T("PivotY"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PIVOTY,
		end,

	plSwivelComp::kOffsetActive,  _T("OffsetActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_OFFSETACTIVE,
		p_enable_ctrls,		4, plSwivelComp::kOffsetX, plSwivelComp::kOffsetY, plSwivelComp::kOffsetZ, plSwivelComp::kOffsetLocal,
		end,

	plSwivelComp::kOffsetX, _T("X"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_OFFSETX, IDC_COMP_OFFSETX_SPIN, 1.0,
		end,	

	plSwivelComp::kOffsetY, _T("Y"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_OFFSETY, IDC_COMP_OFFSETY_SPIN, 1.0,
		end,	

	plSwivelComp::kOffsetZ, _T("Z"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_OFFSETZ, IDC_COMP_OFFSETZ_SPIN, 1.0,
		end,	

	plSwivelComp::kOffsetLocal,  _T("OffsetLocal"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_OFFSETLOCAL,
		end,

	end
);

plSwivelComp::plSwivelComp()
{
	fClassDesc = &gSwivelDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSwivelComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	node->SetMovable(true);

	return true;
}

hsBool plSwivelComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plSwivelComp::Bail(plMaxNode* node, const char* msg, plErrorMsg* pErrMsg)
{
	pErrMsg->Set(true, node->GetName(), msg).CheckAndAsk();
	pErrMsg->Set(false);
	return true;
}

hsBool plSwivelComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plViewFaceModifier* pMod = TRACKED_NEW plViewFaceModifier;
	pMod->SetOrigTransform(node->GetLocalToParent44(), node->GetParentToLocal44());
	node->AddModifier(pMod, IGetUniqueName(node));

	if( fCompPB->GetInt(kPivotY) )
		pMod->SetFlag(plViewFaceModifier::kPivotY);
	else
		pMod->SetFlag(plViewFaceModifier::kPivotFace);

	switch( fCompPB->GetInt(kFaceTypeRadio) )
	{
	case kFaceCamera:
		pMod->SetFollowMode(plViewFaceModifier::kFollowCamera);
		break;

	case kFaceListener:
		pMod->SetFollowMode(plViewFaceModifier::kFollowListener);
		break;

	case kFacePlayer:
		pMod->SetFollowMode(plViewFaceModifier::kFollowPlayer);
		break;

	case kFaceObject:
		{
			plMaxNode* targNode = (plMaxNode*)fCompPB->GetINode(kFaceObjectSel);
			if( targNode && targNode->CanConvert() )
			{
				pMod->SetFollowMode(plViewFaceModifier::kFollowObject, targNode->GetKey());
			}
			else
			{
				pErrMsg->Set(true, node->GetName(), "Swivel to look at component %s has no Plasma object to look at.", GetINode()->GetName()).Show();
				pErrMsg->Set(false);
				pMod->SetFollowMode(plViewFaceModifier::kFollowCamera);
			}
		}
		break;

	}

	pMod->SetOffsetActive(fCompPB->GetInt(kOffsetActive));
	if( fCompPB->GetInt(kOffsetActive) )
	{
		hsVector3 off(fCompPB->GetFloat(kOffsetX), fCompPB->GetFloat(kOffsetY), fCompPB->GetFloat(kOffsetZ));
		pMod->SetOffset(off, fCompPB->GetInt(kOffsetLocal));
	}

	return true;
}

