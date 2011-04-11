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
//Resource related
#include "resource.h"

//Max related
#include "plComponent.h"
#include "plComponentReg.h"

//Messages related
#include "plgDispatch.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

//Scene related
#include "../plScene/plSceneNode.h"
#include "../plInterp/plController.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxMain/plMaxNodeData.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "hsResMgr.h"

//Conversion related
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"

//Avatar related
#include "../plAvatar/plAGAnim.h"
#include "../plAvatar/plMatrixChannel.h"
#include "BipedKiller.h"

//Anim related
#include "plNotetrackAnim.h"


//
//	DummyCodeIncludeFuncAGComp Function
//		Necessary to keep the compiler from tossing this file.
//		No functions herein are directly called, excepting this
//		one.
//
//
void DummyCodeIncludeFuncAGComp()
{
}

enum	{
		kShareableBool,			//Added in v1
		kGlobalBool,			//Added in v1
		};

//////////////////////////////////////////////////////////////
//
//	AnimAvatar Component
//
//
//
class plAnimAvatarComponent : public plComponent
{
//protected:
//	static plAGAnimMgr *fManager;
public:
		plAnimAvatarComponent();
		virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);

		virtual hsBool Convert(plMaxNode* node, plErrorMsg *pErrMsg);

		virtual plATCAnim * NewAnimation(const char *name, double begin, double end);

		hsBool ConvertNode(plMaxNode *node, plErrorMsg *pErrMsg);
		hsBool ConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *mod, plErrorMsg *pErrMsg);
		hsBool MakePersistent(plMaxNode *node, plAGAnim *anim, const char *animName, plErrorMsg *pErrMsg);

		virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }

		void DeleteThis() { delete this; }
};

//plAGAnimMgr * plAnimAvatarComponent::fManager = nil;

CLASS_DESC(plAnimAvatarComponent, gAnimAvatarDesc, "Compound Animation",  "Compound Animation", COMP_TYPE_AVATAR, Class_ID(0x3192253d, 0x60c4178c))

//
//	Anim Avatar ParamBlock2
//
//
ParamBlockDesc2 gAnimAvatarBk
(	
	plComponent::kBlkComp, _T("CompoundAnim"), 0, &gAnimAvatarDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_ANIM_AVATAR, IDS_COMP_ANIM_AVATARS, 0, 0, NULL,

	// params
	kShareableBool, _T("ShareableBool"), TYPE_BOOL, 0, 0, 	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AVATAR_SHAREBOOL,
		end,

	kGlobalBool, _T("ShareableBool"), TYPE_BOOL, 0, 0, 	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AVATAR_GLOBALBOOL,
		end,

	//kBoundCondRadio, _T("BoundingConditions"),		TYPE_INT, 		0, 0,
	//	p_ui,		TYPE_RADIO, 2, IDC_COMP_PHYS_DETECTOR_RAD1, IDC_COMP_PHYS_DETECTOR_RAD2,
	//	end,

	end
);


//
//	Anim Avatar CONSTRUCTOR
//
//
plAnimAvatarComponent::plAnimAvatarComponent()
{
	fClassDesc = &gAnimAvatarDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


//
//	Anim Avatar PRECONVERT
//
//
hsBool plAnimAvatarComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if(node->GetMaxNodeData())
	{
		node->SetMovable(true);
		node->SetForceLocal(true);
		node->SetDrawable(false);
	}
	
	int childCount = node->NumberOfChildren();
	for (int i = 0; i < childCount; i++)
	{
		SetupProperties((plMaxNode *)node->GetChildNode(i), pErrMsg);
	}

	return true; 
}

//
//
// CONVERT
// top level conversion: recursive descent on the node and children
// for each node, search for segments to convert...
//
//
hsBool plAnimAvatarComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	Interface *theInterface = node->GetInterface();
	RemoveBiped(node, theInterface);

	ConvertNode(node, pErrMsg);

	((plSceneNode *)node->GetRoomKey()->GetObjectPtr())->SetFilterGenericsOnly(true);
	
	return true;
}

//
// CONVERTNODE
// look for all the segments on this node and convert them
// recurse on children
//
//
hsBool plAnimAvatarComponent::ConvertNode(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plNotetrackAnim noteAnim(node, pErrMsg);
	// does this node have any segments specified?
	if (noteAnim.HasNotetracks())
	{
		// for each segment we found: 
		while (const char *animName = noteAnim.GetNextAnimName())
		{
			plAnimInfo info = noteAnim.GetAnimInfo(animName);

			plATCAnim *anim = NewAnimation(info.GetAnimName(), info.GetAnimStart(), info.GetAnimEnd());

			const char *loopName = info.GetNextLoopName();
			if (loopName)
			{
				anim->SetLoop(true);
				hsScalar loopStart = info.GetLoopStart(loopName);
				hsScalar loopEnd = info.GetLoopEnd(loopName);
				anim->SetLoopStart(loopStart == -1 ? anim->GetStart() : loopStart);
				anim->SetLoopEnd(loopEnd == -1 ? anim->GetEnd() : loopEnd);
			}
			while (const char *marker = info.GetNextMarkerName())
				anim->AddMarker(marker, info.GetMarkerTime(marker));

			ConvertNodeSegmentBranch(node, anim, pErrMsg);
			MakePersistent(node, anim, info.GetAnimName(), pErrMsg);
		}
	}

	// let's see if the children have any segments specified...
	int childCount = node->NumberOfChildren();
	for (int i = 0; i < childCount; i++)
		ConvertNode((plMaxNode *)(node->GetChildNode(i)), pErrMsg);

	return true;
}

// NewAnimation -------------------------------------------------------------------------
// -------------
plATCAnim * plAnimAvatarComponent::NewAnimation(const char *name, double begin, double end)
{
	return TRACKED_NEW plATCAnim(name, begin, end); 
}


//
// CONVERT NODE SEGMENT BRANCH
// we're now in the middle of converting a segment
// every node gets an animation channel for the time period in question
//
//
hsBool plAnimAvatarComponent::ConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *mod, plErrorMsg *pErrMsg)
{
	// Check for a suppression marker
	plNotetrackAnim noteAnim(node, pErrMsg);
	plAnimInfo info = noteAnim.GetAnimInfo(nil);
	hsBool suppressed = info.IsSuppressed(mod->GetName());

	// Get the affine parts and the TM Controller
	plSceneObject *obj = node->GetSceneObject();
	if(obj && !suppressed) {
		hsAffineParts parts;
		hsControlConverter::Instance().ReduceKeys(node->GetTMController(), node->GetKeyReduceThreshold());
		plController* tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, &parts, mod->GetStart(), mod->GetEnd());
		
		if (tmc)
		{
			plMatrixChannel *channel;
			hsMatrix44 constSetting;
			parts.ComposeMatrix(&constSetting);

			// If all our keys match, there's no point in keeping an animation controller
			// around. Just nuke it and replace it with a constant channel.
			if (tmc->PurgeRedundantSubcontrollers())
			{
				channel = TRACKED_NEW plMatrixConstant(constSetting);
				delete tmc;
				tmc = nil;
			}
			else
			{
				channel = TRACKED_NEW plMatrixControllerChannel(tmc, &parts);
			}
			plMatrixChannelApplicator *app = TRACKED_NEW plMatrixChannelApplicator();
			app->SetChannelName(node->GetKey()->GetName());
			app->SetChannel(channel);
			mod->AddApplicator(app);
		}

		// let's see if the children have any segments specified...
		int childCount = node->NumberOfChildren();
		for (int i = 0; i < childCount; i++)
			ConvertNodeSegmentBranch((plMaxNode *)(node->GetChildNode(i)), mod, pErrMsg);

		return true;
	} else {
		return false;
	}
}

plKey FindSceneNode(plMaxNode *node)
{
	plSceneObject *obj = node->GetSceneObject();
	if(obj)
	{
		return obj->GetSceneNode();
	} else {
		plMaxNode *parent = (plMaxNode *)node->GetParentNode();

		if(parent)
		{
			return FindSceneNode(parent);
		} else {
			return nil;
		}
	}
}

//
// MAKE PERSISTENT
// Perform wizardry necessary to make the object save itself.
//
//
hsBool plAnimAvatarComponent::MakePersistent(plMaxNode *node, plAGAnim *anim, const char *animName, plErrorMsg *pErrMsg)
{
	// new approach: add to the generic pool on the scene node
	plLocation nodeLoc = node->GetLocation();
	plKey sceneNodeKey = FindSceneNode(node);
	if(sceneNodeKey)
	{
		plKey animKey = hsgResMgr::ResMgr()->NewKey(animName, anim, nodeLoc);

		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(sceneNodeKey, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kGeneric);

		hsgResMgr::ResMgr()->AddViaNotify(animKey, refMsg, plRefFlags::kActiveRef);
	}
	else
	{
		pErrMsg->Set(true, "Sorry", "Can't find node to save animation. Animation will not be saved.");
	}

	return true;
}




// plEmoteComponent ---------------------------------
// -----------------
class plEmoteComponent : public plAnimAvatarComponent
{
public:
	enum {
		kBodyUsage,
		kFadeIn,
		kFadeOut
	};
	
	enum {
		kBodyUnknown,
		kBodyUpper,
		kBodyFull
	};

	plEmoteComponent();
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual plATCAnim * NewAnimation(const char *name, double begin, double end);

protected:
	float fFadeIn;
	float fFadeOut;
	plEmoteAnim::BodyUsage fBodyUsage;
};


// gEmoteDesc ---------------------------------------------------------------------------------------------------------------------
// -----------
CLASS_DESC(plEmoteComponent, gEmoteDesc, "Emote Animation",  "Emote Animation", COMP_TYPE_AVATAR, Class_ID(0x383c55ba, 0x6f1d454c))


// gEmoteDesc ----------
// -----------
ParamBlockDesc2 gEmoteBk
(
	plComponent::kBlkComp, _T("EmoteAnim"), 0, &gEmoteDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_EMOTE, IDS_COMP_EMOTE, 0, 0, NULL,

	plEmoteComponent::kBodyUsage, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 3,	IDC_BODY_UNKNOWN, IDC_BODY_UPPER, IDC_BODY_FULL,
		p_vals,	plEmoteComponent::kBodyUnknown, plEmoteComponent::kBodyUpper, plEmoteComponent::kBodyFull,
		p_default, plEmoteComponent::kBodyUnknown,
		end,

	plEmoteComponent::kFadeIn, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 2.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_EMO_FADEIN, IDC_EMO_FADEIN_SPIN, 0.1,
		end,	

	plEmoteComponent::kFadeOut, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 2.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_EMO_FADEOUT, IDC_EMO_FADEOUT_SPIN, 0.1,
		end,	


	end
);


// plEmoteComponent ----------------
// -----------------
plEmoteComponent::plEmoteComponent()
{
	fClassDesc = &gEmoteDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


// Convert ------------------------------------------------------------
// --------
plEmoteComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	Interface *theInterface = node->GetInterface();
	RemoveBiped(node, theInterface);

	fFadeIn = fCompPB->GetFloat(kFadeIn);
	fFadeOut = fCompPB->GetFloat(kFadeOut);
	fBodyUsage = static_cast<plEmoteAnim::BodyUsage>(fCompPB->GetInt(kBodyUsage));

	ConvertNode(node, pErrMsg);
	((plSceneNode *)node->GetRoomKey()->GetObjectPtr())->SetFilterGenericsOnly(true);
	return true;
}


// NewAnimation ----------------------------------------------------------------------
// -------------
plATCAnim * plEmoteComponent::NewAnimation(const char *name, double begin, double end)
{
	return TRACKED_NEW plEmoteAnim(name, begin, end, fFadeIn, fFadeOut, fBodyUsage);
}
