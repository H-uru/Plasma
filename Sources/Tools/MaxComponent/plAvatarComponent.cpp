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
#include <windowsx.h>

#include "plComponentProcBase.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../MaxConvert/hsConverterUtils.h"

#include "../pnSceneObject/plSceneObject.h"

#include "plgDispatch.h"
#include "../plScene/plSceneNode.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plBitmapCreator.h"
#include "hsStringTokenizer.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnKeyedObject/plKey.h"

#include "hsResMgr.h"

#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../plMessage/plMatRefMsg.h"
#include "../plMessage/plLayRefMsg.h"

#include "plMaxAnimUtils.h"
#include "../plInterp/plController.h"
#include "../plPhysical/plSimDefs.h"
#include "plPhysicsGroups.h"
#include "../plAudible/plWinAudible.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plSurface/plLayerAnimation.h"
#include "../plSurface/hsGMaterial.h"
#include "../plAudio/plWin32StaticSound.h"
#include "../plAudioCore/plSoundBuffer.h"
#include "plAudioComponents.h"

#include "../MaxMain/plPlasmaRefMsgs.h"			

#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plAvBrainCritter.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plArmatureEffects.h"
#include "../plGImage/plMipmap.h"
#include "../plGImage/plLODMipmap.h"

// Auto generation of shadows here.
#include "plShadowComponents.h"
#include "../plGLight/plShadowCaster.h"

#include "plAvatarComponent.h"
#include "../../tools/MaxComponent/plPhysicalComponents.h"

#include "../MaxMain/plPhysicalProps.h"
//#include <vector>
//#include <string>

#include "plPickNode.h"
#include "plPickMaterialMap.h"
#include "../MaxMain/plMtlCollector.h"

//#define BOB_SORT_AVATAR_FACES


// CONSTANTS
float kStdRestitution = 0.5f;
float kStdFriction = 0.1f;


void DummyCodeIncludeAvatarFunc() {}

// PROTOTYPES
class plAvatarComponent;
class plCritterComponent;
class plArmatureComponent;
class plCritterCommands;

plArmatureMod* plArmatureComponent::IGenerateMyArmMod(plHKPhysical* myHKPhys, plMaxNode* node)
{
	plArmatureMod *avMod = TRACKED_NEW plArmatureMod();
	avMod->SetRootName(node->GetKey()->GetName());
	return avMod;
}

void plArmatureComponent::ISetupAvatarRenderPropsRecurse(plMaxNode *node)
{
	node->SetNoSpanSort(true);
	node->SetNoFaceSort(true);
	node->SetNoDeferDraw(true);

	int i;
	for (i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode *pChild = (plMaxNode *)node->GetChildNode(i);
		ISetupAvatarRenderPropsRecurse(pChild);
	}
}

//SETUPPROPERTIES
// Tests if IPB2 pointers are healthy and sets up the MaxNode Data
hsBool plArmatureComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	// Global issues
	node->SetMovable(true);
	node->SetForceLocal(true);

#ifndef BOB_SORT_AVATAR_FACES
	ISetupAvatarRenderPropsRecurse(node);
#endif


// 	float mass, friction, restitution;
// 	if(ClassID() == AVATAR_CLASS_ID || ClassID() == LOD_AVATAR_CLASS_ID)
// 	{
// 		mass = 12;	// *** good number from old physical player
// 		restitution = 0.5f;
// 		friction = 0.1f;
// 	}
	plMaxNode *animRoot = nil;

	if(ClassID() == AVATAR_CLASS_ID)
		animRoot = (plMaxNode *)fCompPB->GetINode(plAvatarComponent::kRootNode);
	else if (ClassID() == LOD_AVATAR_CLASS_ID)
		animRoot = (plMaxNode *)fCompPB->GetINode(plLODAvatarComponent::kRootNodeAddBtn);

	if(animRoot)
	{
		const char *nodeName = animRoot->GetName();
		animRoot->SetDrawable(false);		// make sure our root bone is invisible
	}

	// Ignore all of the old physicals, so old scenes can export
	if (ClassID() == AVATAR_CLASS_ID || ClassID() == LOD_AVATAR_CLASS_ID)
	{
		bool isLOD = ((ClassID() == LOD_AVATAR_CLASS_ID) != 0);

		plMaxNode* ignoreNode = (plMaxNode*)fCompPB->GetINode(isLOD ? plLODAvatarComponent::kPhysicsProxyFeet_DEAD : plAvatarComponent::kPhysicsProxyFeet_DEAD);
		if (ignoreNode)
			ignoreNode->SetCanConvert(false);

		ignoreNode = (plMaxNode*)fCompPB->GetINode(isLOD ? plLODAvatarComponent::kPhysicsProxyTorso_DEAD : plAvatarComponent::kPhysicsProxyTorso_DEAD);
		if (ignoreNode)
			ignoreNode->SetCanConvert(false);

		ignoreNode = (plMaxNode*)fCompPB->GetINode(isLOD ? plLODAvatarComponent::kPhysicsProxyHead_DEAD : plAvatarComponent::kPhysicsProxyHead_DEAD);
		if (ignoreNode)
			ignoreNode->SetCanConvert(false);
	}

	return true;
}

//// ISETARMATURESORECURSE
// Do some strange magic to make sure that we know when all the parts of the avatar are loaded.
void plArmatureComponent::ISetArmatureSORecurse(plMaxNode *node, plSceneObject *so)
{
	if (node->CanConvert())
		node->SetAvatarSO(so);

	int i;
	for (i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode *pChild = (plMaxNode *)node->GetChildNode(i);
		ISetArmatureSORecurse(pChild, so);
	}
}


hsBool plArmatureComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	// add audio interface and record/playback component
	pl2WayWinAudible* pAudible = TRACKED_NEW pl2WayWinAudible;

	// Add a key for it
	plKey key = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pAudible, node->GetLocation() );

	plAudioInterface* ai = TRACKED_NEW plAudioInterface;
	plKey pAiKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), (hsKeyedObject*)ai,node->GetLocation());
		
	hsgResMgr::ResMgr()->AddViaNotify(pAiKey, TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
		
	plIntRefMsg* pMsg = TRACKED_NEW plIntRefMsg(node->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kAudible);
	hsgResMgr::ResMgr()->AddViaNotify(pAudible->GetKey(), pMsg, plRefFlags::kActiveRef );

	ISetArmatureSORecurse(node, node->GetSceneObject());
	
	// Uncomment this line to enable a single bone pallete for the entire avatar.
	node->SetupBoneHierarchyPalette();
	return true;

}

// this is a little gross...the armature component shouldn't know that the subclasses
// actually exist....it's a hard-to-detect implementation detail that breaks new subclasses....
hsBool plArmatureComponent::Convert(plMaxNode* node, plErrorMsg *pErrMsg)
{
//	plHKPhysical *physical = plHKPhysical::ConvertToPhysical(node->GetSceneObject());
 //	physical->SetProperty(plSimulationInterface::kUpright, true);

	IAttachModifiers(node, pErrMsg);
	ISetupClothes(node, fArmMod, pErrMsg);

	// ArmatureEffects
	plArmatureEffectsMgr *effects = TRACKED_NEW plArmatureEffectsMgr();
	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effects, node->GetLocation());
	plGenRefMsg *msg = TRACKED_NEW plGenRefMsg(fArmMod->GetKey(), plRefMsg::kOnCreate, -1, -1);
	hsgResMgr::ResMgr()->AddViaNotify(effects->GetKey(), msg, plRefFlags::kActiveRef); // Attach effects

	plSceneObject *obj = node->GetSceneObject();
	node->MakeCharacterHierarchy(pErrMsg);

	const plAGModifier *temp = static_cast<const plAGModifier *>(FindModifierByClass(obj, plAGModifier::Index()));
	plAGModifier *agMod = const_cast<plAGModifier *>(temp);

	hsAssert(agMod, "Armature root didn't get a agmod. I'll make one for you.");
	if( ! agMod)
	{
		// MakeCharacterHierarchy will attach agmodifiers to all the bones in the hierarchy;
		// have to manually add any for non-bone objects...
		agMod = TRACKED_NEW plAGModifier("Handle");		// the player root is known as the handle
		node->AddModifier(agMod, IGetUniqueName(node));
	}

	agMod->SetChannelName("Handle");

	// Get the position and radius of the head and torso physicals
	if (ClassID() == AVATAR_CLASS_ID || ClassID() == LOD_AVATAR_CLASS_ID)
	{
		bool isLOD = ((ClassID() == LOD_AVATAR_CLASS_ID) != 0);
		float height = fCompPB->GetFloat(isLOD ? plLODAvatarComponent::kPhysicsHeight : plAvatarComponent::kPhysicsHeight);
		float width = fCompPB->GetFloat(isLOD ? plLODAvatarComponent::kPhysicsWidth : plAvatarComponent::kPhysicsWidth);
		fArmMod->SetPhysicalDims(height, width);
	}

//	node->SetupBonesAliasesRecur(node->GetKey()->GetName());

	return true;
}


hsBool plArmatureComponent::IVerifyUsedNode(INode* thisNode, plErrorMsg* pErrMsg, hsBool IsHull)
{
	if(thisNode != NULL)
	{
		if(((plMaxNode*)thisNode)->CanConvert())
		{
			if(IsHull)
				((plMaxNode*)thisNode)->SetDrawable(false);

		}else
		{
			pErrMsg->Set(true, "Ignored Node Selection", "The object that Node Ptr %s refs was set to be Ignored. Avatar Component failure.", thisNode->GetName());				
			pErrMsg->Set(false);
			return false;
		}
	}else{
	
		pErrMsg->Set(true, "Empty Node in Avatar Component", "It is imperative that all the node pickers have real values.\n Avatar Component failure.").Show();
		pErrMsg->Set(false);
		return false;
	}
	return true;
}

void plArmatureComponent::IAttachShadowCastModifiersRecur(plMaxNode* node, plShadowCaster* caster)
{
	if( !node || !caster )
		return;

	// Add test here for whether we want this guy to cast a shadow.

	plShadowCastComponent::AddShadowCastModifier(node, caster);

	int i; 
	for( i = 0; i < node->NumberOfChildren(); i++ )
		IAttachShadowCastModifiersRecur((plMaxNode*)node->GetChildNode(i), caster);	
}



//class AvatarStats
//{
//public:
//	float fFriction;
//	float fMaxVel;
//	float fAccel;
//	float fTurnForce;
//	hsBool fUseNewMovement;
//
//	AvatarStats(float a, float b, float c, float d, hsBool newMove)
//		: fFriction(a), fMaxVel(b), fAccel(c), fTurnForce(d), fUseNewMovement(newMove) {};
//	AvatarStats() : fFriction(-1.0), fMaxVel(-1.0), fAccel(-1.0), fTurnForce(-1.0) {};
//};

enum
{
	kArmMain,
	kArmBounce,
	kArmReport,
};

CLASS_DESC(plAvatarComponent, gAvatarCompDesc, "Avatar",  "Avatar", COMP_TYPE_AVATAR, AVATAR_CLASS_ID)

//Max paramblock2 stuff below.
ParamBlockDesc2 gPAvatarBk
(	
	plComponent::kBlkComp, _T("Avatar"), 0, &gAvatarCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1,
	kArmMain, IDD_COMP_AVATAR, IDS_COMP_AVATARS, 0, 0, &gAvatarCompDlgProc,
//	kArmBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
//	kArmReport, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_REPORT, 0, APPENDROLL_CLOSED, &gReportGroupProc,

	// params
 	plAvatarComponent::kPhysicsProxyFeet_DEAD, _T("ProxyFeet"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKB,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,

	plAvatarComponent::kFriction,	_T("Friction"),	TYPE_FLOAT,	0, 0,
		p_ui,	kArmMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_COMP_AVATAR_FRICTION_EDIT, IDC_COMP_AVATAR_FRICTION_SPIN, 0.1f,
		p_default, 0.9f,
		end,

	plAvatarComponent::kRootNode, _T("RootNode"),	TYPE_INODE,		0, 0,
		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_ROOT_PICKB,
		p_prompt, IDS_COMP_AVATAR_PROXYS,
		end,
	
	plAvatarComponent::kMeshNode, _T("MeshNode"),	TYPE_INODE,		0, 0,
		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_MESH_PICKB,
		//p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_AVATAR_PROXYS,
		end,
		
	plAvatarComponent::kClothingGroup,	_T("ClothingGroup"),	TYPE_INT,	0, 0,
		p_default, plClothingMgr::kClothingBaseMale,
		end,

	plArmatureComponent::kBounceGroups, _T("bounceGroups"), TYPE_INT,	0,0,
		p_default,  plPhysicsGroups_DEAD::kCreatures |
					plPhysicsGroups_DEAD::kStaticSimulated |
					plPhysicsGroups_DEAD::kDynamicSimulated |
					plPhysicsGroups_DEAD::kAnimated,
		end,

	plArmatureComponent::kReportGroups, _T("reportGroups"), TYPE_INT,	0,0,
		end,

	plAvatarComponent::kBrainType, _T("Brain"), TYPE_INT, 0, 0,
		p_default, plAvatarComponent::kBrainHuman,
		end,
		
	plAvatarComponent::kSkeleton, _T("Skeleton"),	TYPE_INT, 0, 0,
		p_default, plArmatureMod::kBoneBaseMale,
		end,

 	plAvatarComponent::kPhysicsProxyTorso_DEAD, _T("ProxyTorso"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKT,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,
 
 	plAvatarComponent::kPhysicsProxyHead_DEAD, _T("ProxyHead"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKH,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,

	plAvatarComponent::kPhysicsHeight,	_T("physHeight"), TYPE_FLOAT,	0,	0,
		p_range, 0.1f, 50.0f,
		p_default, 5.f,
		p_ui,	kArmMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_PHYS_HEIGHT_EDIT, IDC_PHYS_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	plAvatarComponent::kPhysicsWidth,	_T("physWidth"), TYPE_FLOAT,	0,	0,
		p_range, 0.1f, 50.0f,
		p_default, 2.5f,
		p_ui,	kArmMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_PHYS_WIDTH_EDIT, IDC_PHYS_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	plAvatarComponent::kBodyFootstepSoundPage,	_T("bodyFootstepPage"), TYPE_STRING,	0,	0,
		p_default, "Audio",
		p_ui,	kArmMain, TYPE_EDITBOX, IDC_BODYFOOTSTEPPAGE_EDIT,
		end,

	plAvatarComponent::kAnimationPrefix,_T("animationPrefix"), TYPE_STRING,	0,	0,
		p_default, "Male",
		p_ui,	kArmMain, TYPE_EDITBOX, IDC_ANIMATIONPREFIX_EDIT,
		end,

	end
);

plAvatarComponent::plAvatarComponent()
{
	fClassDesc = &gAvatarCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


void plAvatarComponent::IAttachModifiers(plMaxNode *node, plErrorMsg *pErrMsg)
{
	const char *name = node->GetKey()->GetName();

	plMaxNode *meshNode = (plMaxNode *)fCompPB->GetINode(plAvatarComponent::kMeshNode);
	plKey meshKey = meshNode->GetSceneObject()->GetKey();
	plMaxNode *animRootNode = (plMaxNode *)fCompPB->GetINode(plAvatarComponent::kRootNode);
	plKey animRootKey = animRootNode->GetSceneObject()->GetKey();
			
	plSceneObject * bodySO = node->GetSceneObject();

	plArmatureMod* avMod = TRACKED_NEW plArmatureMod();
	avMod->SetRootName(name);
	avMod->AppendMeshKey(meshKey);
	int skeletonType = fCompPB->GetInt(ParamID(kSkeleton));
	avMod->SetBodyType( skeletonType );

	// only make a human brain if we're a human
	if (skeletonType == plArmatureMod::kBoneBaseCritter)
		avMod->PushBrain(TRACKED_NEW plAvBrainCritter());
	else
		avMod->PushBrain(TRACKED_NEW plAvBrainHuman(skeletonType == plArmatureMod::kBoneBaseActor));

	avMod->SetBodyAgeName(node->GetAgeName());
	avMod->SetBodyFootstepSoundPage(fCompPB->GetStr(ParamID(kBodyFootstepSoundPage)));
	avMod->SetAnimationPrefix(fCompPB->GetStr(ParamID(kAnimationPrefix)));

	//AddLinkSound(node, node->GetSceneObject()->GetKey(), pErrMsg );

	plKey avKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), avMod, node->GetLocation());
	plObjRefMsg *objRefMsg = TRACKED_NEW plObjRefMsg(bodySO->GetKey(), plRefMsg::kOnCreate,-1, plObjRefMsg::kModifier);
	hsgResMgr::ResMgr()->AddViaNotify(avKey, objRefMsg, plRefFlags::kActiveRef);

	fArmMod = avMod;
}

// Helper function for the Avatar and LOD Avatar components
void AddClothingToMod(plMaxNode *node, plArmatureMod *mod, int group, hsGMaterial *mat, plErrorMsg *pErrMsg)
{
	plGenRefMsg *msg;
	char keyName[256];
	TSTR sdata;
	hsStringTokenizer toker;

	if (mod == nil)
	{
		hsAssert(false, "Adding clothes to a nil armatureMod.");
		return;
	}

	plClothingBase *base = TRACKED_NEW plClothingBase();
	if (node->GetUserPropString("layout", sdata))
	{
		toker.Reset(sdata, hsConverterUtils::fTagSeps);
		base->SetLayoutName(toker.next());
	}
	else 
		base->SetLayoutName("BasicHuman");
	sprintf(keyName, "%s_ClothingBase", node->GetName());
	hsgResMgr::ResMgr()->NewKey(keyName, base, node->GetLocation());
	plClothingOutfit *outfit = TRACKED_NEW plClothingOutfit();
	outfit->fGroup = group;
	sprintf(keyName, "%s_outfit", mod->GetKey()->GetName());
	hsgResMgr::ResMgr()->NewKey(keyName, outfit, node->GetLocation());
	
	msg = TRACKED_NEW plGenRefMsg(outfit->GetKey(), plRefMsg::kOnCreate, -1, -1);
	hsgResMgr::ResMgr()->AddViaNotify(base->GetKey(), msg, plRefFlags::kActiveRef); // Add clothing base to outfit
	msg = TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnCreate, -1, -1);
	hsgResMgr::ResMgr()->AddViaNotify(outfit->GetKey(), msg, plRefFlags::kActiveRef); // Attach outfit
	
	
	plMipmap *baseTex = nil;
	plLayerInterface *li = nil;

	if (mat != nil)
	{
		li = mat->GetLayer(0)->BottomOfStack();
		baseTex = plMipmap::ConvertNoRef(li->GetTexture());
		hsAssert(li->GetTexture() == baseTex, "Base texture mismatch on avatar construction?");
	}
	if (mat != nil && baseTex != nil)
	{
		// Let's fix up these bitmap flags. Normally, they are set on convert based on
		// the contents of the bitmap. But here our contents are subject to change at runtime.
		// Fortunately, we've got a pretty good idea what to expect.
		// I'm making a subjective decision here on forcing 32bit even when we run in 16 bit,
		// because an A4R4G4B4 makes the avatar look like it has a rare skin disease.
		baseTex->SetFlags(plMipmap::kAlphaChannelFlag | plMipmap::kForce32Bit | plMipmap::kDontThrowAwayImage);
		
		// The tex is what the outfit and the material hold onto. It's what
		// gets rendered. The base will hang onto the original baseTex.
		msg = TRACKED_NEW plGenRefMsg(base->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->SendRef(baseTex->GetKey(), msg, plRefFlags::kActiveRef); // Set base texture of avatar
		plLayRefMsg* layRef = TRACKED_NEW plLayRefMsg(li->GetKey(), plRefMsg::kOnRemove, 0, plLayRefMsg::kTexture);
		hsgResMgr::ResMgr()->SendRef(baseTex->GetKey(), layRef, plRefFlags::kActiveRef); // Remove it from the material

		msg = TRACKED_NEW plGenRefMsg(outfit->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->AddViaNotify(li->GetKey(), msg, plRefFlags::kActiveRef); // Set outfit's target layer interface
		msg = TRACKED_NEW plGenRefMsg(outfit->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->AddViaNotify(mat->GetKey(), msg, plRefFlags::kActiveRef); // Outfit needs the material
	}
	else
	{
		pErrMsg->Set(outfit->fGroup != plClothingMgr::kClothingBaseNoOptions, node->GetName(), 
					 "This avatar expects clothing options, but has no material on which to place them. "
					 "It will not be visable at runtime.").CheckAskOrCancel();
		outfit->fGroup = plClothingMgr::kClothingBaseNoOptions;
	}
}

void plAvatarComponent::ISetupClothes(plMaxNode *node, plArmatureMod *mod, plErrorMsg *pErrMsg)
{
	AddClothingToMod(node, mod, fCompPB->GetInt(kClothingGroup), nil, pErrMsg);
}

hsBool plAvatarComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetItinerant(true);

// 	if(!IVerifyUsedNode(fCompPB->GetINode(plAvatarComponent::kPhysicsProxyFeet), pErrMsg, true))
// 		return false;
// 	if(!IVerifyUsedNode(fCompPB->GetINode(plAvatarComponent::kPhysicsProxyTorso), pErrMsg, true))
// 		return false;
// 	if(!IVerifyUsedNode(fCompPB->GetINode(plAvatarComponent::kPhysicsProxyHead), pErrMsg, true))
// 		return false;
	if(!IVerifyUsedNode(fCompPB->GetINode(plAvatarComponent::kMeshNode), pErrMsg, false))
		return false;
	if(!IVerifyUsedNode(fCompPB->GetINode(plAvatarComponent::kRootNode), pErrMsg, false))
		return false;

	plMaxNode *meshNode = (plMaxNode *)fCompPB->GetINode(plAvatarComponent::kMeshNode);
	if (meshNode)
	{
		if (meshNode->GetObjectRef()->ClassID() == Class_ID(DUMMY_CLASS_ID, 0))
		{
			meshNode->SetSwappableGeomTarget(plArmatureMod::kSwapTargetShadow);
		}
	}

	return plArmatureComponent::SetupProperties(node, pErrMsg);
}

class AvatarCompDlgProc : public ParamMap2UserDlgProc
{
public:
	AvatarCompDlgProc() {}
	~AvatarCompDlgProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		HWND cbox = NULL;
		char* buffer = NULL;

		int selection;
		switch (msg)
		{
		case WM_INITDIALOG:
			int j;
			for (j = 0; j < plClothingMgr::kMaxGroup; j++)
			{
				cbox = GetDlgItem(hWnd, IDC_COMP_AVATAR_CLOTHING_GROUP);
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plClothingMgr::GroupStrings[j]);
			}
			selection = pb->GetInt(ParamID(plAvatarComponent::kClothingGroup));
			SendMessage(cbox, CB_SETCURSEL, selection, 0);

			for (j = 0; j < plArmatureMod::kMaxBoneBase; j++)
			{
				cbox = GetDlgItem(hWnd, IDC_COMP_AVATAR_SKELETON);
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plArmatureMod::BoneStrings[j]);
			}
			selection = pb->GetInt(ParamID(plAvatarComponent::kSkeleton));
			SendMessage(cbox, CB_SETCURSEL, selection, 0);
			
			return TRUE;

		case WM_COMMAND:  
        	if (id == IDC_COMP_AVATAR_CLOTHING_GROUP)
			{
				selection = SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0);
				pb->SetValue(plAvatarComponent::kClothingGroup, t, selection);
				return TRUE;
			}
        	if (id == IDC_COMP_AVATAR_SKELETON)
			{
				selection = SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0);
				pb->SetValue(plAvatarComponent::kSkeleton, t, selection);
				return TRUE;
			}

			break;
		}	
		return FALSE;
	}
	void DeleteThis() {}
};
static AvatarCompDlgProc gAvatarCompDlgProc;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class plCompoundCtrlComponent : plComponent
{
public:
	plCompoundCtrlComponent();
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode* node,plErrorMsg *pErrMsg);
};

CLASS_DESC(plCompoundCtrlComponent, gCompoundCtrlCompDesc, "Compound Controller", "CompoundCtrl", COMP_TYPE_AVATAR, Class_ID(0x3f2a790f, 0x30354673))

plCompoundCtrlComponent::plCompoundCtrlComponent()
{
	fClassDesc = &gCompoundCtrlCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plCompoundCtrlComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetMovable(true);
	node->SetForceLocal(true);
	node->SetItinerant(true);
	return true;
}

hsBool plCompoundCtrlComponent::Convert(plMaxNode* node, plErrorMsg *pErrMsg)
{
	const char *name = node->GetKey()->GetName();

	node->MakeCharacterHierarchy(pErrMsg);
	node->SetupBonesAliasesRecur(name);


	// create and register the player modifier
	plAGMasterMod *agMaster = TRACKED_NEW plAGMasterMod();
	node->AddModifier(agMaster, IGetUniqueName(node));

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//
//	LOD Avatar Stuff below


class plLODAvatarComponentProc : public plVSBaseComponentProc
{
protected:
	IParamBlock2 *fPB;
	plLODAvatarComponent* fComp;
	HWND fMstrDlg;

public:
	plLODAvatarComponentProc() : fComp(nil), fPB(nil) {}

	void UpdateBoneDisplay(IParamMap2 *pm)
	{
		HWND hWnd = pm->GetHWnd();
		HWND hList = GetDlgItem(hWnd, IDC_COMP_LOD_AVATAR_BONELIST);
		IParamBlock2 *pb = pm->GetParamBlock();
			
		ListBox_ResetContent(hList);
		int group = fComp->GetCurGroupIdx();
		int startIdx = fComp->GetStartIndex(group);
		int endIdx = fComp->GetEndIndex(group);

		while (startIdx < endIdx)
		{
			INode *curNode = pb->GetINode(ParamID(plLODAvatarComponent::kBoneList), 0, startIdx);
			if (curNode == nil)
			{
				fComp->RemoveBone(startIdx);
				endIdx--;
				continue;
			}
			ListBox_AddString(hList, curNode->GetName());
			startIdx++;
		}
	}

	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) { UpdateBoneDisplay(pmap); }

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int selection;
		HWND cbox = NULL;
		HWND hList = GetDlgItem(hWnd, IDC_COMP_LOD_AVATAR_BONELIST);

		switch (msg)
		{
		case WM_INITDIALOG:
			{
				int LodBeginState	= map->GetParamBlock()->GetInt(plLODAvatarComponent::kLODState);

				HWND LODCombo = GetDlgItem(hWnd, IDC_COMP_LOD_AVATAR_STATE);
	
				fMstrDlg = hWnd;
			
				fPB = map->GetParamBlock();
				fComp = (plLODAvatarComponent*) fPB->GetOwner();

				VCharArray Nilptr;
				ILoadComboBox(LODCombo, fComp->fLODLevels);
				SendMessage(LODCombo, CB_SETCURSEL, LodBeginState,	0);	// select the right one

				int i;
				for (i = 0; i < plClothingMgr::kMaxGroup; i++)
				{
					cbox = GetDlgItem(hWnd, IDC_COMP_AVATAR_CLOTHING_GROUP);
					SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plClothingMgr::GroupStrings[i]);
				}
				selection = fPB->GetInt(ParamID(plLODAvatarComponent::kClothingGroup));
				SendMessage(cbox, CB_SETCURSEL, selection, 0);

				for (i = 0; i < plArmatureMod::kMaxBoneBase; i++)
				{
					cbox = GetDlgItem(hWnd, IDC_COMP_AVATAR_SKELETON);
					SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plArmatureMod::BoneStrings[i]);
				}
				selection = fPB->GetInt(ParamID(plLODAvatarComponent::kSkeleton));
				SendMessage(cbox, CB_SETCURSEL, selection, 0);

				Mtl *mat = fPB->GetMtl(plLODAvatarComponent::kMaterial);
				Button_SetText(GetDlgItem(hWnd, IDC_COMP_LOD_AVATAR_MTL), (mat ? mat->GetName() : "(none)"));

				UpdateBoneDisplay(map);
				return true;
			}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_COMP_AVATAR_CLOTHING_GROUP)
			{
				selection = SendMessage(GetDlgItem(hWnd, IDC_COMP_AVATAR_CLOTHING_GROUP), CB_GETCURSEL, 0, 0);
				fPB->SetValue(ParamID(plLODAvatarComponent::kClothingGroup), t, selection);
				return TRUE;
			}
        	else if (LOWORD(wParam) == IDC_COMP_AVATAR_SKELETON)
			{
				selection = SendMessage(GetDlgItem(hWnd, IDC_COMP_AVATAR_SKELETON), CB_GETCURSEL, 0, 0);
				fPB->SetValue(ParamID(plLODAvatarComponent::kSkeleton), t, selection);
				return TRUE;
			}
			else if (HIWORD(wParam) == BN_CLICKED)
			{
				if (LOWORD(wParam) == IDC_COMP_LOD_AVATAR_BONE_ADD)
				{
					std::vector<Class_ID> cids;
					cids.push_back(Class_ID(TRIOBJ_CLASS_ID, 0));
					cids.push_back(Class_ID(EDITTRIOBJ_CLASS_ID, 0));
					if (plPick::NodeRefKludge(fPB, plLODAvatarComponent::kLastPick, &cids, true, false))			
						fComp->AddSelectedBone();

					return TRUE;
				}
				// Remove the currently selected material
				else if (LOWORD(wParam) == IDC_COMP_LOD_AVATAR_BONE_REMOVE)
				{
					int curSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
					if (curSel >= 0)
						fComp->RemoveBone(curSel);

					return TRUE;
				}
				else if (LOWORD(wParam) == IDC_COMP_LOD_AVATAR_MTL)
				{
					Mtl *pickedMtl = plPickMaterialMap::PickMaterial(plMtlCollector::kPlasmaOnly);
					fPB->SetValue(plLODAvatarComponent::kMaterial, 0, pickedMtl);
					Button_SetText(GetDlgItem(hWnd, IDC_COMP_LOD_AVATAR_MTL), (pickedMtl ? pickedMtl->GetName() : "(none)"));

					return TRUE;
				}
			}
			else 
			{
				int LodBeginState	= map->GetParamBlock()->GetInt(plLODAvatarComponent::kLODState);
				
				if(fPB->GetINode(plLODAvatarComponent::kMeshNodeAddBtn,t))
					fPB->SetValue(plLODAvatarComponent::kMeshNodeTab, t, fPB->GetINode(plLODAvatarComponent::kMeshNodeAddBtn,t), LodBeginState);
					
				if(LOWORD(wParam) == IDC_COMP_LOD_AVATAR_STATE && HIWORD(wParam) == CBN_SELCHANGE)
				{
					int idx = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					fPB->SetValue(plLODAvatarComponent::kLODState, 0, idx);
					
					if(fPB->GetINode(plLODAvatarComponent::kMeshNodeTab, t, idx))
						fPB->SetValue(plLODAvatarComponent::kMeshNodeAddBtn, t, fPB->GetINode(plLODAvatarComponent::kMeshNodeTab,t, idx));
					else
						fPB->Reset(plLODAvatarComponent::kMeshNodeAddBtn);
					return true;
				}
			}

			break;

		case WM_CLOSE:
			{
				int LodBeginState	= map->GetParamBlock()->GetInt(plLODAvatarComponent::kLODState);

				if(fPB->GetINode(plLODAvatarComponent::kMeshNodeAddBtn,t))
					fPB->SetValue(plLODAvatarComponent::kMeshNodeTab, t, fPB->GetINode(plLODAvatarComponent::kMeshNodeAddBtn,t), LodBeginState);

				return false;
			}
		}
			
		return false;
	}

	void DeleteThis() {}
};




//! A static variable.
/*! 
	A static instance used in the ParamBlock2 processing of
	the physical Dialog UIs.

	\sa plPhysCoreComponentProc()
*/
static plLODAvatarComponentProc gLODAvComponentProc;

class plLODAvAccessor : public PBAccessor
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
		if (id == plLODAvatarComponent::kMeshNodeAddBtn)
		{
			plLODAvatarComponent *comp = (plLODAvatarComponent *)owner;
			IParamBlock2 *pb = comp->GetParamBlockByID(plLODAvatarComponent::kBlkComp);
			int LodBeginState = pb->GetInt(plLODAvatarComponent::kLODState);
				
			INode *node = pb->GetINode(plLODAvatarComponent::kMeshNodeAddBtn, t);
			if (node)
				pb->SetValue(plLODAvatarComponent::kMeshNodeTab, t, node, LodBeginState);
		}
	}
};

plLODAvAccessor gLODAvatarAccessor;

/////////////////////////////////////////////////////////////////////////////////////////
///////
//		PLLODAVATARCOMPONENT

CLASS_DESC(plLODAvatarComponent, gLODAvatarCompDesc, "LODAvatar",  "LODAvatar", COMP_TYPE_AVATAR, LOD_AVATAR_CLASS_ID)

//Max paramblock2 stuff below.
ParamBlockDesc2 gPLODAvatarBk
(	
	plComponent::kBlkComp, _T("Avatar"), 0, &gLODAvatarCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	//Roll out
	1,
	kArmMain, IDD_COMP_LOD_AVATAR, IDS_COMP_LOD_AVATARS, 0, 0, &gLODAvComponentProc,
// 	kArmBounce, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_BOUNCE, 0, APPENDROLL_CLOSED, &gBounceGroupProc,
// 	kArmReport, IDD_COMP_PHYS_CORE_GROUP, IDS_COMP_PHYS_REPORT, 0, APPENDROLL_CLOSED, &gReportGroupProc,

	// params
 	plLODAvatarComponent::kPhysicsProxyFeet_DEAD, _T("ProxyFeet"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKB,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,

	plLODAvatarComponent::kFriction,	_T("Friction"),	TYPE_FLOAT,	0, 0,
		p_ui,	kArmMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,
		IDC_COMP_AVATAR_FRICTION_EDIT, IDC_COMP_AVATAR_FRICTION_SPIN, 0.1f,
		p_default, 0.9f,
		end,

	plLODAvatarComponent::kLODState, _T("LODState"),		TYPE_INT,	0, 0,
		p_range, 1, plLODAvatarComponent::kMaxNumLODLevels,
		end,

	plLODAvatarComponent::kMeshNodeTab,	_T("MeshObject"),	TYPE_INODE_TAB, plLODAvatarComponent::kMaxNumLODLevels,		0, 0,
		p_accessor,		&gLODAvatarAccessor,
		end,

	plLODAvatarComponent::kRootNodeAddBtn, _T("RtNodePicker"),	TYPE_INODE,		0, 0,
		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_LOD_AVATAR_ROOT_PICKB,
		p_prompt, IDS_COMP_AVATAR_PROXYS,
		end,

	plLODAvatarComponent::kMeshNodeAddBtn, _T("MshNodePicker"),	TYPE_INODE,		0, 0,
		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_LOD_AVATAR_MESH_PICKB,
		//p_sclassID,	GEOMOBJECT_CLASS_ID,
		p_prompt, IDS_COMP_AVATAR_PROXYS,
		end,

	plLODAvatarComponent::kClothingGroup,	_T("ClothingGroup"),	TYPE_INT,	0, 0,
		p_default, plClothingMgr::kClothingBaseMale,
		end,

	plArmatureComponent::kBounceGroups, _T("bounceGroups"), TYPE_INT,	0,0,
		p_default,  plPhysicsGroups_DEAD::kCreatures |
					plPhysicsGroups_DEAD::kStaticSimulated |
					plPhysicsGroups_DEAD::kDynamicSimulated |
					plPhysicsGroups_DEAD::kAnimated,
		end,

	plArmatureComponent::kReportGroups, _T("reportGroups"), TYPE_INT,	0,0,
		end,

	plLODAvatarComponent::kBrainType, _T("Brain"), TYPE_INT, 0, 0,
		p_default, plLODAvatarComponent::kBrainHuman,
		end,
		
	plLODAvatarComponent::kGroupIdx,	_T("GroupIndex"),	TYPE_INT,	0, 0,
		p_default, 0,
		p_range, 0, plLODAvatarComponent::kMaxNumLODLevels - 1,
		p_ui, kArmMain, TYPE_SPINNER,	EDITTYPE_INT,
		IDC_COMP_LOD_AVATAR_GROUP, IDC_COMP_LOD_AVATAR_GROUP_SPIN, 1.f,
		end,

	plLODAvatarComponent::kBoneList, _T("Bones"), TYPE_INODE_TAB, 0,	0, 0,
		end,

	plLODAvatarComponent::kGroupTotals, _T("Totals"), TYPE_INT_TAB, plLODAvatarComponent::kMaxNumLODLevels,	0, 0,
		p_default, 0,
		end,

	plLODAvatarComponent::kLastPick, _T("LastPick"), TYPE_INODE,	0, 0, // Temp storage space for the bone picker
		end,

	plLODAvatarComponent::kSkeleton, _T("Skeleton"),	TYPE_INT, 0, 0,
		p_default, plArmatureMod::kBoneBaseMale,
		end,

	plLODAvatarComponent::kMaterial, _T("Material"),	TYPE_MTL, 0, 0,
		end,

 	plLODAvatarComponent::kPhysicsProxyTorso_DEAD, _T("ProxyTorso"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKT,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,
// 
 	plLODAvatarComponent::kPhysicsProxyHead_DEAD, _T("ProxyHead"),	TYPE_INODE,		0, 0,
// 		p_ui,	kArmMain, TYPE_PICKNODEBUTTON, IDC_COMP_AVATAR_PROXY_PICKH,
// 		p_sclassID,	GEOMOBJECT_CLASS_ID,
// 		p_prompt, IDS_COMP_AVATAR_PROXYS,
 		end,

	plLODAvatarComponent::kPhysicsHeight,	_T("physHeight"), TYPE_FLOAT,	0,	0,
		p_range, 0.1f, 50.0f,
		p_default, 5.f,
		p_ui,	kArmMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_PHYS_HEIGHT_EDIT, IDC_PHYS_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	plLODAvatarComponent::kPhysicsWidth,	_T("physWidth"), TYPE_FLOAT,	0,	0,
		p_range, 0.1f, 50.0f,
		p_default, 2.5f,
		p_ui,	kArmMain, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_PHYS_WIDTH_EDIT, IDC_PHYS_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	plLODAvatarComponent::kBodyFootstepSoundPage,	_T("bodyFootstepPage"), TYPE_STRING,	0,	0,
		p_default, "Audio",
		p_ui,	kArmMain, TYPE_EDITBOX, IDC_BODYFOOTSTEPPAGE_EDIT,
		end,

	plLODAvatarComponent::kAnimationPrefix,_T("animationPrefix"), TYPE_STRING,	0,	0,
		p_default, "Male",
		p_ui,	kArmMain, TYPE_EDITBOX, IDC_ANIMATIONPREFIX_EDIT,
		end,

	end
);

plLODAvatarComponent::plLODAvatarComponent() : fMaterial(nil)
{
	fClassDesc = &gLODAvatarCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);

	fLODLevels.push_back("High");
	fLODLevels.push_back("Medium");
	fLODLevels.push_back("Low");

}

void plLODAvatarComponent::IAttachModifiers(	plMaxNode *node, plErrorMsg *pErrMsg)
{
	const char *avatarName = node->GetKey()->GetName();
	plMaxNode *animRoot = (plMaxNode *)fCompPB->GetINode(plLODAvatarComponent::kRootNodeAddBtn);
	plKey animRootKey = animRoot->GetSceneObject()->GetKey();
	plArmatureLODMod* avMod = TRACKED_NEW plArmatureLODMod(avatarName);

	int skeletonType = fCompPB->GetInt(ParamID(kSkeleton));
	avMod->SetBodyType( skeletonType );
	if (skeletonType == plArmatureLODMod::kBoneBaseCritter)
		avMod->PushBrain(TRACKED_NEW plAvBrainCritter());
	else
		avMod->PushBrain(TRACKED_NEW plAvBrainHuman(skeletonType == plArmatureMod::kBoneBaseActor));

	avMod->SetBodyAgeName(node->GetAgeName());
	avMod->SetBodyFootstepSoundPage(fCompPB->GetStr(ParamID(kBodyFootstepSoundPage)));
	avMod->SetAnimationPrefix(fCompPB->GetStr(ParamID(kAnimationPrefix)));

	int iLODCount = fCompPB->Count(plLODAvatarComponent::kMeshNodeTab);
	for (int i = 0; i < iLODCount; i++)
	{
		plMaxNode *meshNode = (plMaxNode *)fCompPB->GetINode(plLODAvatarComponent::kMeshNodeTab, 0, i);
		plKey meshKey = meshNode->GetSceneObject()->GetKey();
		avMod->AppendMeshKey(meshKey);
	}

	node->AddModifier(avMod, IGetUniqueName(node));
	fArmMod = avMod;
	IAttachShadowCastToLODs(node);
}

hsBool plLODAvatarComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetItinerant(true);	
	
// 	if(!IVerifyUsedNode(fCompPB->GetINode(plLODAvatarComponent::kPhysicsProxyFeet), pErrMsg, true))
// 		return false;
// 	if(!IVerifyUsedNode(fCompPB->GetINode(plLODAvatarComponent::kPhysicsProxyTorso), pErrMsg, true))
// 		return false;
// 	if(!IVerifyUsedNode(fCompPB->GetINode(plLODAvatarComponent::kPhysicsProxyHead), pErrMsg, true))
// 		return false;
	if(!IVerifyUsedNode(fCompPB->GetINode(plLODAvatarComponent::kRootNodeAddBtn), pErrMsg, false))
		return false;
	for(int i = 0; i < plLODAvatarComponent::kMaxNumLODLevels; i++)
	{
		plMaxNode *meshNode = (plMaxNode *)fCompPB->GetINode(plLODAvatarComponent::kMeshNodeTab, 0, i);
		if (!IVerifyUsedNode(meshNode, pErrMsg, false))
		{
			return false;
		}
		else
		{
			if (meshNode->GetObjectRef()->ClassID() == Class_ID(DUMMY_CLASS_ID, 0))
			{
				meshNode->SetSwappableGeomTarget(plArmatureMod::kSwapTargetShadow);
			}
		}
	}

	return plArmatureComponent::SetupProperties(node, pErrMsg);
}

hsBool plLODAvatarComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)			
{ 
	hsBool result = plArmatureComponent::PreConvert(node, pErrMsg); 

	hsTArray<hsGMaterial*> mats;
	Mtl *mtl = fCompPB->GetMtl(kMaterial);
	if (mtl)
	{
		hsMaterialConverter::Instance().GetMaterialArray(mtl, node, mats);
		fMaterial = (mats.GetCount() > 0 ? mats[0] : nil);
	}
	return result;
}

hsBool plLODAvatarComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plArmatureComponent::Convert(node, pErrMsg);

	// Bone LOD stuff
	int numSoFar = 0;
	int i;
	for (i = 0; i < kMaxNumLODLevels; i++)
	{
		int numBones = fCompPB->GetInt(ParamID(kGroupTotals), 0, i);
		plKeyVector *keyVec = TRACKED_NEW plKeyVector;

		int j;
		for (j = 0; j < numBones; j++)
		{
			plMaxNode *compNode = (plMaxNode*)fCompPB->GetINode(ParamID(kBoneList), 0, numSoFar + j);
			if (compNode)
			{
				plAGModifier *agMod = compNode->HasAGMod();
				keyVec->push_back(agMod ? agMod->GetKey() : nil);
			}
		}
		plArmatureLODMod::ConvertNoRef(fArmMod)->AppendBoneVec(keyVec);

		numSoFar += numBones;
	}

	return true;
}

void plLODAvatarComponent::ISetupClothes(plMaxNode *node, plArmatureMod *mod, plErrorMsg* pErrMsg)
{
	AddClothingToMod(node, mod, fCompPB->GetInt(kClothingGroup), fMaterial, pErrMsg);
}

void plLODAvatarComponent::IAttachShadowCastModifiersRecur(plMaxNode* node, plShadowCaster* caster)
{
	if( !node || !caster )
		return;

	if( !node->GetSwappableGeom() && node->GetObjectRef()->ClassID() != Class_ID(DUMMY_CLASS_ID, 0))
		plShadowCastComponent::AddShadowCastModifier(node, caster);

	int i; 
	for( i = 0; i < node->NumberOfChildren(); i++ )
		IAttachShadowCastModifiersRecur((plMaxNode*)node->GetChildNode(i), caster);	
}

void plLODAvatarComponent::IAttachShadowCastToLODs(plMaxNode* rootNode)
{
	plShadowCaster* caster = TRACKED_NEW plShadowCaster;
	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(rootNode), caster, rootNode->GetLocation());
	caster->SetSelfShadow(true);

	int iLODCount = fCompPB->Count(plLODAvatarComponent::kMeshNodeTab);

	for (int i = 0; i < iLODCount; i++)
	{
		plMaxNode *meshNode = (plMaxNode *)fCompPB->GetINode(plLODAvatarComponent::kMeshNodeTab, 0, i);
		if( meshNode )
		{
			plShadowCastComponent::AddShadowCastModifier(meshNode, caster); // The LOD roots are a special case.
			IAttachShadowCastModifiersRecur(meshNode, caster);
		}
	}
}

int plLODAvatarComponent::GetCurGroupIdx()
{
	return fCompPB->GetInt(ParamID(kGroupIdx));
}

int plLODAvatarComponent::GetStartIndex(int group)
{
	int result = 0;
	int i;
	for (i = 0; i < group; i++)
		result += fCompPB->GetInt(ParamID(kGroupTotals), 0, i);

	return result;
}

int plLODAvatarComponent::GetEndIndex(int group)
{
	return GetStartIndex(group) + fCompPB->GetInt(ParamID(kGroupTotals), 0, group);
}

void plLODAvatarComponent::AddSelectedBone()
{
	int group = GetCurGroupIdx();
	int boneIdx = GetEndIndex(group);

	INode *node = fCompPB->GetINode(ParamID(kLastPick));
	fCompPB->Insert(ParamID(kBoneList), boneIdx, 1, &node);

	fCompPB->SetValue(ParamID(kGroupTotals), 0, fCompPB->GetInt(ParamID(kGroupTotals), 0, group) + 1, group);
}

void plLODAvatarComponent::RemoveBone(int index)
{
	int group = GetCurGroupIdx();
	int boneIdx = GetStartIndex(group) + index;
	
	fCompPB->Delete(ParamID(kBoneList), boneIdx, 1);
	fCompPB->SetValue(ParamID(kGroupTotals), 0, fCompPB->GetInt(ParamID(kGroupTotals), 0, group) - 1, group);
}


