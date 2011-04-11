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
void DummyCodeIncludeFuncPhysConst()
{
}


#include "HeadSpin.h"
#include "plComponent.h"
#include "plComponentReg.h"
#define PHYS_CONST_HINGE_CID	Class_ID(0x790b1637, 0x32c94144)
#define PHYS_CONST_WHEEL_CID	Class_ID(0x6e2958dc, 0x62e86e87)
#define PHYS_CONST_SS_CID		Class_ID(0x14843886, 0x62a24e94)
#define PHYS_CONST_BRIDGE_CID	Class_ID(0x5f99392f, 0x3e5a5807)

OBSOLETE_CLASS(plPhysHingeConstraintComponent, gPhysHingeConstDesc, "(ex)Hinge Constraint",  "(ex)Hinge Constraint", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_HINGE_CID)
OBSOLETE_CLASS(plPhysBridgeComponent, gPhysBridgeConstDesc, "(ex)Bridge", "Bridge", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_BRIDGE_CID)
OBSOLETE_CLASS(plStrongSpringConstraintComponent, gPhysStrongSpringConstDesc, "(ex)StrongSpring Constraint",  "(ex)StrongSpring Constraint", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_SS_CID)

#if 0



#include <hkdynamics/defs.h>
#include <hkdynamics/entity/rigidcollection.h>
#include <hkdynamics/util/units.h>

//#include "../plHavok1/plHKConstraintSolver.h"		//Got Havok Messiness...  Must go before all plasma directories...
//#include "../plHavok1/plHavokConstraintTools.h"

#include "plPhysicalComponents.h"
#include "plComponentProcBase.h"
#include "../plPhysical/plPhysicsGroups.h"
#include "../MaxMain/plPhysicalProps.h"
//#include "max.h"								//Max Dependencies, reffed in plEventGroupDefs

#include "resource.h"							//Resource Dependencies
#include "hsResMgr.h"		//	Ibid

#include "plComponent.h"						//Component Dependencies
#include "plComponentReg.h"						//	Ibid
#include "../pnSceneObject/plSceneObject.h"		//  Ibid
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plScene/plSceneNode.h"				//  Ibid
#include "../pnKeyedObject/plKey.h"				//	Ibid
#include "../MaxMain/plMaxNode.h"				//  Ibid

#include "../MaxConvert/hsConverterUtils.h"		//Conversion Dependencies
#include "../MaxConvert/hsControlConverter.h"	//	Ibid

#include "../plHavok1/plHKPhysical.h"			//Havok Dependencies
#include "../MaxMain/plPhysicalProps.h"

#include "plgDispatch.h"						//Message Dependencies
#include "../pnMessage/plObjRefMsg.h"			//	Ibid
#include "../pnMessage/plIntRefMsg.h"			//	Ibid	
#include "../pnMessage/plNodeRefMsg.h"			//	Ibid
#include "../MaxMain/plPlasmaRefMsgs.h"			//	Ibid
#include "../plModifier/plAliasModifier.h"

//
//	DummyCodeIncludeFuncPhys Function.
//		Necessary to keep the compiler from throwing away this file.
//		No functions within are inherently called otherwise....
//
//

class plPhysConstraintAccessor : public PBAccessor
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
		plComponentBase *comp = (plComponentBase*)owner;
		comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);

	}
};



//! Global variable.
plPhysConstraintAccessor gPhysConstraintAccessor;

//!  Physical Hinge Constraint Component Class

/*!
	This Class offers the very limited functionality, specifically wheel constraint
	Physical properties.  It relies on the Core's Convert functionality.  
	The function GetParamVals is processed uniquely, to offer transparency into the Core 
	internal states found in its PhysicalStats member function, fUserInput.
	  
		member functions:
			
			GetParamVals(plMaxNode *pNode,plErrorMsg *pErrMsg)
			PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
		
		    \sa plPhysDetectorComponent(), DeleteThis(), GetParamVals(), PreConvert(), Convert(), MaybeMakeLocal() and FixUpPhysical()
*/

class plPhysHingeConstraintComponent : plComponent
{
public:
		enum
		{
			kPositionPtr,
			kParent,
			kParentPinnedBool,
			kFriction,
			kUpperAngle,
			kLowerAngle,
			kRebound,
			kStrength,
			kUseParentBool,
			kChildPinnedBool,
		};
	
		enum
		{
			kYAxis,
			kXAxis,
			kZAxis,
		};
	
		//! Constructor function for class
		/*!
			Herein the ClassDesc2 object that is used extensively by the ParamBlock2
			has gained accessibiltiy.  Auto-Creation of the UI is done here as well.
					
			\sa DeleteThis(), GetParamVals(), PreConvert(), Convert(), MaybeMakeLocal() and FixUpPhysical()

		*/

		plPhysHingeConstraintComponent();

		//! Detector GetParamVals function for class, with 1 input.
		/*!
			Partial Transparency.  The following Internal states are accessible:
				
				  -Bounce
				  -Friction
				  -Collision Reporting
				  -Bounding Surface State
				  -Disabling LOS
				  -Disable Ghost

			\param pNode a plMaxNode ptr, is the only formal parameter.
			\sa DeleteThis(), plPhysicalCoreComponent(), Convert(), PreConvert(), MaybeMakeLocal() and FixUpPhysical()

		*/

		hsBool GetParamVals(plMaxNode *pNode, plErrorMsg *pErrMsg);
		
		//! Detector PreConvert, takes in two variables and return a hsBool.
		/*! 
			Calls the function MaybeMakeLocal() and Sets Drawable to false.

			Takes in two variables, being:
			\param node a plMaxNode ptr.
			\param pErrMsg a pErrMsg ptr.

			\return A hsBool expressing the success of the operation.
			\sa DeleteThis(), plPhysicalCoreComponent(), Convert(), GetParamVals(), MaybeMakeLocal() and FixUpPhysical()
		*/

		hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);
		hsBool PreConvert(plMaxNode* node, plErrorMsg* plErrorMsg) { return true;}

		hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//
//	plPhysHingeConstraint Component MacroConstructor
//


CLASS_DESC(plPhysHingeConstraintComponent, gPhysHingeConstDesc, "(ex)Hinge Constraint",  "(ex)Hinge Constraint", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_HINGE_CID)


enum { HackVal = 1};


ParamBlockDesc2 gPhysHingeConstraintBk
(	
	plComponent::kBlkComp, _T("Hinge Constraint"), 0, &gPhysHingeConstDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PHYS_HINGE_CONSTRAINT, IDS_COMP_PHYS_HINGE_CONSTRAINTS, 0, 0, NULL,//&gPhysCoreComponentProc,
	
	// params


	plPhysHingeConstraintComponent::kPositionPtr, 	_T("Rotation Axis Conditions"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 3, IDC_COMP_PHYS_HINGE_AXIS_RADIO, IDC_COMP_PHYS_HINGE_AXIS_RADIO2, IDC_COMP_PHYS_HINGE_AXIS_RADIO3,
		p_vals,						plPhysHingeConstraintComponent::kXAxis,		plPhysHingeConstraintComponent::kYAxis,		plPhysHingeConstraintComponent::kZAxis,
		p_default, plPhysHingeConstraintComponent::kZAxis,
		end,

	plPhysHingeConstraintComponent::kUseParentBool, _T("UseParentChkBx"),	TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_USE_PARENT_BOOL,
		p_default, TRUE,
		p_enable_ctrls, 2, plPhysHingeConstraintComponent::kParent, plPhysHingeConstraintComponent::kParentPinnedBool,
		end,

	
	plPhysHingeConstraintComponent::kParent,  _T("Parent"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PARENT,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		p_accessor, &gPhysConstraintAccessor,
		end,

	
	plPhysHingeConstraintComponent::kParentPinnedBool, _T("ParentPinnedChkBx"),	TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_PINNED_STATE_BOOL,
		end,


	plPhysHingeConstraintComponent::kFriction,	_T("Friction"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 1.0,
		p_range, 0.0, 50000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_FRICTION_SLIDER, IDC_COMP_PHYS_FRICTION_SPIN1, 1.0,
		end,

	plPhysHingeConstraintComponent::kUpperAngle,	_T("UpperLimit"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, -360.0, 360.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PHYS_OPENANGLE_EDIT, IDC_COMP_PHYS_OPENANGLE_SPIN, 1.0,
		end,

	plPhysHingeConstraintComponent::kLowerAngle,	_T("LowerLimit"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 360.0,
		p_range, -360.0, 360.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PHYS_CLOSEANGLE_EDIT, IDC_COMP_PHYS_CLOSEANGLE_SPIN, 1.0,
		end,


			
	plPhysHingeConstraintComponent::kRebound,	_T("Rebound"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.1,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_TAU_SLIDER, IDC_COMP_PHYS_TAU_SPIN, .01f,
		end,

	plPhysHingeConstraintComponent::kStrength,	_T("Strength"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_STRENGTH_SLIDER, IDC_COMP_PHYS_STRENGTH_SPIN, .01f,
		end,

	plPhysHingeConstraintComponent::kChildPinnedBool, _T("ChildPinnedChkBx"),	TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_PINNED_STATE_BOOL2,
		end,


	end
);





//
//	PhysHingeConstraint Component Constructor
//
//


plPhysHingeConstraintComponent::plPhysHingeConstraintComponent()
{
	fClassDesc = &gPhysHingeConstDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

//
//	plPhysHingeConstraintComponent GetParamVals Function
//
//


hsBool plPhysHingeConstraintComponent::GetParamVals(plMaxNode *node,plErrorMsg *pErrMsg)
{
	return true;
}


//
//	Physical Wheel Constraint Component PreConvert Function
//
//


hsBool plPhysHingeConstraintComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{

	/*
	plMaxNodeData *pMD = pNode->GetMaxNodeData();
	pMD->SetMovable(true);
	pMD->SetForceLocal(true);


	plMaxNode* ParentNode = (plMaxNode*)fCompPB->GetINode(kParent);
	plMaxNodeData *pMD2 = ParentNode->GetMaxNodeData();
	pMD2->SetMovable(true);
	pMD2->SetForceLocal(true);


*/
/*
		hsBool HasPhysFlag = false;
		for(int i = 0; i < pNode->NumAttachedComponents(); i++)
		{
			plComponentBase* thisComp = pNode->GetAttachedComponent(i);
			if(thisComp->ClassID() == PHYSICS_DEBUG_CID && !HasPhysFlag) HasPhysFlag = true;
			if(thisComp->ClassID() == PHYSICS_TERRAIN_CID && !HasPhysFlag) HasPhysFlag = true;
			if(thisComp->ClassID() == PHYSICS_DETECTOR_CID && !HasPhysFlag) HasPhysFlag = true;
			if(thisComp->ClassID() == PHYSICS_INVISIBLE_CID && !HasPhysFlag) HasPhysFlag = true;
			if(thisComp->ClassID() == PHYSICS_PLAYER_CID && !HasPhysFlag) HasPhysFlag = true;
			if(thisComp->ClassID() == PHYSICS_SIMPLE_CID && !HasPhysFlag) HasPhysFlag = true;
		if(HasPhysFlag) break;
		}*/
	



	GetParamVals(pNode, pErrMsg);




		if(fCompPB->GetINode(kParent) && fCompPB->GetInt(kUseParentBool))
			if(!((plMaxNode*)fCompPB->GetINode(kParent))->CanConvert())
			{
				pErrMsg->Set(true, "Ignored Parent Value", "Parent %s was set to be Ignored. No Constraint was used.", (fCompPB->GetINode(kParent)->GetName())).Show();
				pErrMsg->Set(false);
				return false;
			}			




	return true;
}


extern PlasmaToHavokQuat(Havok::Quaternion &a, hsQuat &b);



hsBool plPhysHingeConstraintComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plHingeConstraintMod* HMod = TRACKED_NEW plHingeConstraintMod;


		plMaxNode* ParentNode = (plMaxNode*)fCompPB->GetINode(kParent);
		if(ParentNode)
		{
			if(!ParentNode->GetPhysicalProps()->IsUsed())
			{
				pErrMsg->Set(true, "Need a Physical Component", "Object %s has a Physical Constraint but no Physical Component. No Constraint was used.", (node->GetName())).Show();
				pErrMsg->Set(false);
			}


		}

		if(!node->GetPhysicalProps()->IsUsed())
		{
			pErrMsg->Set(true, "Need a Physical Component", "Object %s has a Physical Constraint but no Physical Component. No Constraint was used.", (node->GetName())).Show();
			pErrMsg->Set(false);
		}
		
		


		// Using the MotorTorque field, just to keep from creating yet another field in our bloated base class.
		if(fCompPB->GetInt(kParentPinnedBool))
			HMod->SetParentPin(true);

		if(fCompPB->GetInt(kChildPinnedBool))
			HMod->SetChildPin(true);


		if(fCompPB->GetFloat(kFriction))
			HMod->SetHCFriction(0,fCompPB->GetFloat(kFriction));

		//Grab the pivot point from the child translate
		hsPoint3 PP = node->GetLocalToWorld44().GetTranslate();
		hsVector3 PPVector;
		PPVector.Set(PP.fX, PP.fY, PP.fZ);
		HMod->SetPP(PPVector);



		plKey ParentKey  = nil;
		if(fCompPB->GetINode(kParent) && fCompPB->GetInt(kUseParentBool))
			if(((plMaxNode*)fCompPB->GetINode(kParent))->CanConvert())
			{
				plMaxNode* ParentNode = (plMaxNode*)fCompPB->GetINode(kParent);
				ParentKey = ParentNode->GetKey();

			}
			else
			{
				pErrMsg->Set(true, "Ignored Position Value", "Position %s was set to be Ignored. No Physical Proxy selected.", (fCompPB->GetINode(kPositionPtr)->GetName()));
				pErrMsg->Set(false);
				return false;
			}			

		hsVector3 HingeVector;
		if(fCompPB->GetInt(kPositionPtr) == kZAxis)
		{
			HingeVector = node->GetLocalToWorld44().GetAxis(hsMatrix44::kUp);
			HMod->SetHCLimits(kZAxis, 1, fCompPB->GetFloat(kUpperAngle));
			HMod->SetHCLimits(kZAxis, 0, fCompPB->GetFloat(kLowerAngle));


		}		
		else if(fCompPB->GetInt(kPositionPtr) == kYAxis)
		{
			HingeVector = node->GetLocalToWorld44().GetAxis(hsMatrix44::kView);
			HMod->SetHCLimits(kYAxis, 0, -1*fCompPB->GetFloat(kUpperAngle));
			HMod->SetHCLimits(kYAxis, 1, -1*fCompPB->GetFloat(kLowerAngle));

		}
		else
		{
			HingeVector = node->GetLocalToWorld44().GetAxis(hsMatrix44::kRight);
			HMod->SetHCLimits(kXAxis, 0, -1*fCompPB->GetFloat(kUpperAngle));
			HMod->SetHCLimits(kXAxis, 1, -1*fCompPB->GetFloat(kLowerAngle));

		}
		HMod->SetRotationAxis(-1*HingeVector);

		HMod->SetRR(fCompPB->GetFloat(kRebound));
		HMod->SetDamp(fCompPB->GetFloat(kStrength));
		
		node->AddModifier(HMod, IGetUniqueName(node));
		if(ParentKey)
			hsgResMgr::ResMgr()->AddViaNotify( ParentKey, TRACKED_NEW plGenRefMsg( HMod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kParentIdx, 0 ), plRefFlags::kPassiveRef );
		hsgResMgr::ResMgr()->AddViaNotify( node->GetKey(), TRACKED_NEW plGenRefMsg( HMod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kChildIdx, 0 ), plRefFlags::kPassiveRef );









	return true;

}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class plPhysBridgeComponent : public plComponent
{
protected:
	// Only used during convert
	bool fIsValidated;

public:
	enum
	{
		kSections,
		kUpperAngle,
		kLowerAngle,
		kStiffness,
		kStrength,
	};

	plPhysBridgeComponent();

	void SetDefaultTau();
	void ValidateSections();

	hsBool SetupProperties(plMaxNode* node, plErrorMsg* errMsg);
	hsBool PreConvert(plMaxNode* node, plErrorMsg* errMsg);
	hsBool Convert(plMaxNode* node, plErrorMsg* errMsg);
};

class plBridgeProc : public ParamMap2UserDlgProc
{
protected:
	void ILoadList(IParamBlock2* pb, HWND hDlg);
	void IMoveListSel(bool up, IParamBlock2* pb, HWND hDlg);

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};
static plBridgeProc gBridgeComponentProc;

CLASS_DESC(plPhysBridgeComponent, gPhysBridgeConstDesc, "(ex)Bridge", "Bridge", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_BRIDGE_CID)

ParamBlockDesc2 gPhysBridgeConstraintBk
(	
	plComponent::kBlkComp, _T("Bridge"), 0, &gPhysBridgeConstDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_PHYS_BRIDGE, IDS_COMP_PHYS_BRIDGE, 0, 0, &gBridgeComponentProc,
	
	plPhysBridgeComponent::kSections,  _T("Sections"),	TYPE_INODE_TAB, 0,		0, 0,
		end,

	plPhysBridgeComponent::kUpperAngle,	_T("UpperLimit"),		TYPE_FLOAT, 	0, 0,	
		p_default, 15.0,
		p_range, 0.0, 360.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PHYS_OPENANGLE_EDIT, IDC_COMP_PHYS_OPENANGLE_SPIN, 1.0,
		end,

	plPhysBridgeComponent::kLowerAngle,	_T("LowerLimit"),		TYPE_FLOAT, 	0, 0,	
		p_default, -15.0,
		p_range, -360.0, 0.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PHYS_CLOSEANGLE_EDIT, IDC_COMP_PHYS_CLOSEANGLE_SPIN, 1.0,
		end,

	plPhysBridgeComponent::kStiffness,	_T("Stiffness"),		TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,
		IDC_STIFFNESS_EDIT, IDC_STIFFNESS_SPIN, 0.01,
		end,

	plPhysBridgeComponent::kStrength,	_T("Strength"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.4,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,
		IDC_STRENGTH_EDIT, IDC_STRENGTH_SPIN, 0.01,
		end,
		
	end
);

plPhysBridgeComponent::plPhysBridgeComponent() : fIsValidated(false)
{
	fClassDesc = &gPhysBridgeConstDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plPhysBridgeComponent::SetDefaultTau()
{
	int count = fCompPB->Count(kSections);
	float tau = 1.f / sqrt(float(count));
	fCompPB->SetValue(kStrength, 0, tau);
}

void plPhysBridgeComponent::ValidateSections()
{
	int i;

	// Make sure everything we're attached to is in our list of sections
	for (i = 0; i < NumTargets(); i++)
	{
		INode* node = (INode*)GetTarget(i);

		bool found = false;
		int count = fCompPB->Count(kSections);
		for (int j = 0; j < count; j++)
		{
			if (node == fCompPB->GetINode(kSections, 0, j))
			{
				found = true;
				break;
			}
		}

		if (!found)
			fCompPB->Append(kSections, 1, &node);
	}

	// Make sure we're still attached to everything in our list of sections
	for (i = fCompPB->Count(kSections)-1; i >= 0; i--)
	{
		plMaxNodeBase* node = (plMaxNodeBase*)fCompPB->GetINode(kSections, 0, i);

		if (!IsTarget(node))
			fCompPB->Delete(kSections, i, 1);
	}
}

hsBool plPhysBridgeComponent::SetupProperties(plMaxNode* node, plErrorMsg* errMsg)
{
	if (!fIsValidated)
	{
		ValidateSections();
		fIsValidated = true;
	}

	plPhysicalProps* physProps = node->GetPhysicalProps();
	physProps->SetMass(100.f, node, errMsg);
	physProps->SetBoundsType(plSimDefs::kHullBounds, node, errMsg);

	// If the parent is the start or end anchor for the bridge, pin it
	int numSections = fCompPB->Count(kSections);
	if (node == fCompPB->GetINode(kSections, 0, 0) ||
		(numSections > 0 && node == fCompPB->GetINode(kSections, 0, numSections-1)))
	{
		physProps->SetPinned(true, node, errMsg);
		physProps->SetMemberGroup(plHKPhysicsGroups::kStaticSimulated, node, errMsg);
	}
	else
	{
		physProps->SetMemberGroup(plHKPhysicsGroups::kDynamicSimulated, node, errMsg);
	}

	return true;
}

hsBool plPhysBridgeComponent::PreConvert(plMaxNode* node, plErrorMsg* errMsg)
{
	fIsValidated = false;

	return true;
}

hsBool plPhysBridgeComponent::Convert(plMaxNode* node, plErrorMsg* errMsg)
{
	plMaxNode* parent = nil;

	// Find the parent for this section
	int count = fCompPB->Count(kSections);
	for (int i = 0; i < count; i++)
	{
		plMaxNode* curNode = (plMaxNode*)fCompPB->GetINode(kSections, 0, i);
		if (curNode == node)
		{
			if (i < count-1)
				parent = (plMaxNode*)fCompPB->GetINode(kSections, 0, i+1);
			break;
		}
	}

	// No parent, must be the end anchor for the bridge
	if (!parent)
		return false;

	plHingeConstraintMod* mod = TRACKED_NEW plHingeConstraintMod;

//	mod->SetHCFriction(0, 1.f);
	mod->SetRR(fCompPB->GetFloat(kStrength));//1.f / sqrt(float(count)));
	mod->SetDamp(fCompPB->GetFloat(kStiffness));

	// Grab the pivot point from the child translate
	hsPoint3 pivot = node->GetLocalToWorld44().GetTranslate();
	hsVector3 pivotVec;
	pivotVec.Set(pivot.fX, pivot.fY, pivot.fZ);
	mod->SetPP(pivotVec);

	// Cut'n'Paste
	enum
	{
		kYAxis,
		kXAxis,
		kZAxis,
	};

	hsVector3 hingeVector;
	hingeVector = node->GetLocalToWorld44().GetAxis(hsMatrix44::kRight);
//	mod->SetHCLimits(kXAxis, 0, -1*fCompPB->GetFloat(kUpperAngle));
//	mod->SetHCLimits(kXAxis, 1, -1*fCompPB->GetFloat(kLowerAngle));

	mod->SetRotationAxis(-1*hingeVector);

	node->AddModifier(mod, IGetUniqueName(node));
	hsgResMgr::ResMgr()->AddViaNotify(parent->GetKey(), TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kParentIdx, 0), plRefFlags::kPassiveRef);
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kChildIdx, 0), plRefFlags::kPassiveRef);

	return true;
}

///

void plBridgeProc::ILoadList(IParamBlock2* pb, HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_SECT_LIST);
	for (int i = 0; i < pb->Count(plPhysBridgeComponent::kSections); i++)
	{
		INode* node = pb->GetINode(plPhysBridgeComponent::kSections, 0, i);
		ListBox_AddString(hList, node->GetName());
	}
}

void plBridgeProc::IMoveListSel(bool up, IParamBlock2* pb, HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_SECT_LIST);
	int idx = ListBox_GetCurSel(hList);
	int count = ListBox_GetCount(hList);

	int newIdx = 0;
	if (up && idx > 0)
		newIdx = idx-1;
	else if (!up && idx < count-1)
		newIdx = idx+1;
	else
		return;

	INode* node = pb->GetINode(plPhysBridgeComponent::kSections, 0, idx);
	INode* swapNode = pb->GetINode(plPhysBridgeComponent::kSections, 0, newIdx);

	pb->SetValue(plPhysBridgeComponent::kSections, 0, node, newIdx);
	pb->SetValue(plPhysBridgeComponent::kSections, 0, swapNode, idx);

	ListBox_ResetContent(hList);
	ILoadList(pb, hDlg);
	ListBox_SetCurSel(hList, newIdx);
}

BOOL plBridgeProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			plPhysBridgeComponent* comp = (plPhysBridgeComponent*)pm->GetParamBlock()->GetOwner();
			comp->ValidateSections();

			ILoadList(pm->GetParamBlock(), hWnd);
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (LOWORD(wParam) == IDC_UP_BUTTON || LOWORD(wParam) == IDC_DN_BUTTON)
			{
				IMoveListSel((LOWORD(wParam) == IDC_UP_BUTTON), pm->GetParamBlock(), hWnd);
				return TRUE;
			}
			else if (LOWORD(wParam) == IDC_DEF_STR_BUTTON)
			{
				plPhysBridgeComponent* comp = (plPhysBridgeComponent*)pm->GetParamBlock()->GetOwner();
				comp->SetDefaultTau();
				pm->Invalidate(plPhysBridgeComponent::kStrength);
				return TRUE;
			}
			break;
		case LBN_SELCHANGE:
			{
				bool hasSelection = (ListBox_GetCurSel(HWND(lParam)) != LB_ERR);
				EnableWindow(GetDlgItem(hWnd, IDC_UP_BUTTON), hasSelection);
				EnableWindow(GetDlgItem(hWnd, IDC_DN_BUTTON), hasSelection);
			}
			break;
		}
		break;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class plStrongSpringConstraintComponent : plComponent
{
public:

		enum
		{
			kLength = 0,
			kRebound,
			kStrength,
			kParent,
			kParentPinnedBool,
		};
	
	
		//! Constructor function for class
		/*!
			Herein the ClassDesc2 object that is used extensively by the ParamBlock2
			has gained accessibiltiy.  Auto-Creation of the UI is done here as well.
					
			\sa DeleteThis(), GetParamVals(), PreConvert(), Convert(), MaybeMakeLocal() and FixUpPhysical()

		*/

		plStrongSpringConstraintComponent();

		//! Detector GetParamVals function for class, with 1 input.
		/*!
			Partial Transparency.  The following Internal states are accessible:
				
				  -Bounce
				  -Friction
				  -Collision Reporting
				  -Bounding Surface State
				  -Disabling LOS
				  -Disable Ghost

			\param pNode a plMaxNode ptr, is the only formal parameter.
			\sa DeleteThis(), plPhysicalCoreComponent(), Convert(), PreConvert(), MaybeMakeLocal() and FixUpPhysical()

		*/

		hsBool GetParamVals(plMaxNode *pNode, plErrorMsg *pErrMsg);
		
		//! Detector PreConvert, takes in two variables and return a hsBool.
		/*! 
			Calls the function MaybeMakeLocal() and Sets Drawable to false.

			Takes in two variables, being:
			\param node a plMaxNode ptr.
			\param pErrMsg a pErrMsg ptr.

			\return A hsBool expressing the success of the operation.
			\sa DeleteThis(), plPhysicalCoreComponent(), Convert(), GetParamVals(), MaybeMakeLocal() and FixUpPhysical()
		*/

		hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);
		hsBool PreConvert(plMaxNode* node, plErrorMsg* plErrorMsg) { return true;}

		hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

};


//
//	Hack! Hack!  Don't talk back!
//


//
//	Detector Component MacroConstructor
//


CLASS_DESC(plStrongSpringConstraintComponent, gPhysStrongSpringConstDesc, "(ex)StrongSpring Constraint",  "(ex)StrongSpring Constraint", COMP_TYPE_PHYS_CONSTRAINTS, PHYS_CONST_SS_CID)


//
//	Wheel Constraint ParamBlock2
//
//


ParamBlockDesc2 gPhysSSConstraintBk
(	
	plComponent::kBlkComp, _T("Strong Spring Constraint"), 0, &gPhysStrongSpringConstDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PHYS_SS_CONSTRAINT, IDS_COMP_PHYS_SS_CONSTRAINTS, 0, 0, NULL,//&gPhysCoreComponentProc,
	
	// params

	plStrongSpringConstraintComponent::kLength,		_T("Length"),		TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 500.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHY_SS_LENGTH_EDIT, IDC_COMP_PHY_SS_LENGTH_SPIN, 1.0,
		end,

	plStrongSpringConstraintComponent::kRebound,	_T("Rebound"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.0,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_SS_REBOUND_EDIT, IDC_COMP_PHYS_SS_REBOUND_SPIN, 0.1,
		end,

	plStrongSpringConstraintComponent::kStrength,	_T("Strength"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.5,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PHYS_SS_STRENGTH_EDIT, IDC_COMP_PHYS_SS_STRENGTH_SPIN, 0.1,
		end,
	
	plStrongSpringConstraintComponent::kParent,  _T("Parent"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_PHYS_PARENT,
		//p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
		p_accessor, &gPhysConstraintAccessor,
		end,

	plStrongSpringConstraintComponent::kParentPinnedBool, _T("PinnedChkBx"),	TYPE_BOOL, 	0, 0,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PHYS_USE_PARENT_BOOL,
		end,



	end
);


//
//	Detector Component Constructor
//
//


plStrongSpringConstraintComponent::plStrongSpringConstraintComponent()
{
	fClassDesc = &gPhysStrongSpringConstDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

//
//	Detector Component GetParamVals Function
//
//


extern PlasmaToHavokQuat(Havok::Quaternion &a, hsQuat &b);


hsBool plStrongSpringConstraintComponent::GetParamVals(plMaxNode *node,plErrorMsg *pErrMsg)
{


	return true;
}


//
//	Physical Wheel Constraint Component PreConvert Function
//
//


hsBool plStrongSpringConstraintComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{

	GetParamVals(pNode, pErrMsg);

		if(fCompPB->GetINode(kParent))
			if(!((plMaxNode*)fCompPB->GetINode(kParent))->CanConvert())
			{
				pErrMsg->Set(true, "Ignored Parent Value", "Parent %s was set to be Ignored. No Constraint was used.", (fCompPB->GetINode(kParent)->GetName()));
				pErrMsg->Set(false);
				return false;
			}			
		else
		{
			pErrMsg->Set(true, "Bad Parent Value", " Parent %s wasn't selected, all are currently necessary.", (fCompPB->GetINode(kParent)->GetName()));
			pErrMsg->Set(false);
			return false;
		}

		if(500 >= fCompPB->GetFloat(kLength) && fCompPB->GetFloat(kLength) >= 0  )
		{
		}
		else
		{
			pErrMsg->Set(true, "Bad Spring Length Value", " Strong Spring Constraint was out of range.  Bailing out!.");
			pErrMsg->Set(false);
			return false;

		}

		if(fCompPB->GetFloat(kRebound) >= 0 && fCompPB->GetFloat(kRebound) <= 1)
		{
		}
		else
		{
			pErrMsg->Set(true, "Bad Spring Rebound Value", " Strong Spring Constraint was lost.  Bailing out!.");
			pErrMsg->Set(false);
			return false;

		}

		if(fCompPB->GetFloat(kStrength) >= 0 && fCompPB->GetFloat(kStrength) <= 1)
		{
		}
		else
		{
			pErrMsg->Set(true, "Bad Spring Strength Value", " Strong Spring Constraint was lost.  Bailing out!.");
			pErrMsg->Set(false);
			return false;

		}




	return true;
}



hsBool plStrongSpringConstraintComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plStrongSpringConstraintMod* HMod = TRACKED_NEW plStrongSpringConstraintMod;

	HMod->SetDamp(fCompPB->GetFloat(kStrength));
	HMod->SetRR(fCompPB->GetFloat(kRebound));

	//No MaximumTorque here, might as well use that field for the Pinned state for the Parent...
	HMod->SetParentPin(fCompPB->GetInt(kParentPinnedBool));


		Object *obj = fCompPB->GetINode(kParent)->EvalWorldState(0/*hsConverterUtils::Instance().GetTime(GetInterface())*/).obj;

		plKey ParentKey  = nil;
		if(fCompPB->GetINode(kParent))
			if(((plMaxNode*)fCompPB->GetINode(kParent))->CanConvert() && (obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) || obj->SuperClassID() == GEOMOBJECT_CLASS_ID ))
			{
				plMaxNode* ParentNode = (plMaxNode*)fCompPB->GetINode(kParent);
				ParentKey = ParentNode->GetKey();
			}
			else
			{
				pErrMsg->Set(true, "Ignored Parent Node", "Parent Node %s was set to be Ignored. Bad! Bad!.", (fCompPB->GetINode(kParent)->GetName()));
				pErrMsg->Set(false);
				return false;
			}			
		else
		{

//			pErrMsg->Set(true, "Bad Parent Node", " Parent Node %s wasn't selected. Strong Spring Constraint failed.", (fCompPB->GetINode(kParent)->GetName()));
//			pErrMsg->Set(false);
//			return false;
		}


	//No motor Angle here, might as well use the field for Length Storage...
	HMod->SetFixedLength(fCompPB->GetFloat(kLength));



	node->AddModifier(HMod, IGetUniqueName(node));
	hsgResMgr::ResMgr()->AddViaNotify( ParentKey, TRACKED_NEW plGenRefMsg( HMod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kParentIdx, 0 ), plRefFlags::kPassiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( node->GetKey(), TRACKED_NEW plGenRefMsg( HMod->GetKey(), plRefMsg::kOnCreate, plHavokConstraintsMod::kChildIdx, 0 ), plRefFlags::kPassiveRef );









	return true;

}










#endif