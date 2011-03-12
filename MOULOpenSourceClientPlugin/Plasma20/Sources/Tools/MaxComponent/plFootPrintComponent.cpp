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
#include "../MaxMain/plPlasmaRefMsgs.h"
#include "plPickNode.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxExport/plExportProgressBar.h"
#include "../MaxConvert/hsMaterialConverter.h"

#include "hsTypes.h"
#include "plTweak.h"

#include "hsResMgr.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"

#include "../plDrawable/plDynaFootMgr.h"
#include "../plDrawable/plDynaRippleMgr.h"
#include "../plDrawable/plDynaRippleVSMgr.h"
#include "../plDrawable/plDynaBulletMgr.h"
#include "../plDrawable/plDynaPuddleMgr.h"
#include "../plDrawable/plDynaTorpedoMgr.h"
#include "../plDrawable/plDynaTorpedoVSMgr.h"
#include "../plDrawable/plDynaWakeMgr.h"
#include "../plDrawable/plCutter.h"
#include "../plModifier/plDecalEnableMod.h"
#include "../plDrawable/plPrintShape.h"
#include "../plDrawable/plActivePrintShape.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"

#include "plWaterComponent.h"
#include "../plDrawable/plWaveSetBase.h"

#include "plParticleComponents.h"

void DummyCodeIncludeFuncFootPrint()
{
}


const Class_ID FOOTPRINT_COMP_CID(0x5d553f57, 0x42344c5e);
const Class_ID RIPPLE_COMP_CID(0x750e1b2a, 0x4ad10f15);
const Class_ID PUDDLE_COMP_CID(0x2f800331, 0x640f4914);
const Class_ID WAKE_COMP_CID(0x714b6b6b, 0xa937ff4);
const Class_ID BULLET_COMP_CID(0x208c05d7, 0x41540df2);
const Class_ID TORPEDO_COMP_CID(0x72b85c86, 0xe2b0b40);

const Class_ID DIRTY_COMP_CID(0x500f1fd3, 0x508c4486);



class plFootPrintComponent : public plComponent
{
protected:
	plDynaDecalMgr*		fDecalMgr;
	bool				fValid;

	bool				fNotifiesSetup;

	hsBool				ISetupNotifies(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool				ISetupDecalMgr(plMaxNode* node, plErrorMsg* pErrMsg, plDynaDecalMgr* decalMgr);
	hsBool				ICreateDecalMaterials(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool				ISetupColorDecalMaterials(plMaxNode* node, plErrorMsg* pErrMsg);

	hsBool				ISetupParticles(plMaxNode* node, plErrorMsg* pErrMsg);

	static plParticleComponent* IGetParticleComp(INode* node);

	virtual void	IFakeParams();
public:
	enum 
	{
		kWidth,
		kLength,
		kFadeIn,
		kFadeOut,
		kLifeSpan,
		kLayer,
		kBlend,
		kDirtyTime,
		kNotifies,
		kIntensity,
		kParticles,
		kPartyTime
	};
	enum
	{
		kAlpha,
		kMADD,
		kAdd,
		kMult
	};
	
	plFootPrintComponent();
	virtual ~plFootPrintComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);


	static plDynaDecalMgr* GetDecalMgr(INode* node);
};


CLASS_DESC(plFootPrintComponent, gFootPrintCompDesc, "Foot Print",  "FootPrint", COMP_TYPE_FOOTPRINT, FOOTPRINT_COMP_CID)


class plWetProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
			}
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_NOTIFY)
			{
				std::vector<Class_ID> cids;
				cids.push_back(FOOTPRINT_COMP_CID);
				cids.push_back(RIPPLE_COMP_CID);
				cids.push_back(PUDDLE_COMP_CID);
				cids.push_back(WAKE_COMP_CID);
				IParamBlock2 *pb = map->GetParamBlock();
				plPick::Node(pb, plFootPrintComponent::kNotifies, &cids, false, false);

				map->Invalidate(plFootPrintComponent::kNotifies);
				return TRUE;
			}
			else
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_PARTICLE)
			{
				std::vector<Class_ID> cids;
				cids.push_back(PARTICLE_SYSTEM_COMPONENT_CLASS_ID);
				IParamBlock2 *pb = map->GetParamBlock();
				plPick::Node(pb, plFootPrintComponent::kParticles, &cids, false, false);

				map->Invalidate(plFootPrintComponent::kParticles);
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plWetProc gWetProc;

/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gFootPrintBk
(	
	plComponent::kBlkComp, _T("FootPrint"), 0, &gFootPrintCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_FOOTPRINT, IDS_COMP_FOOTPRINT, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 400.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_WIDTH, IDC_COMP_FP_WIDTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 400.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 10.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
		p_default, 30.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.1,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.25,
		p_range, 0.1, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

hsBool plFootPrintComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	// If we don't have a valid layer, we're screwed. Throw up a warning
	// and shutdown.
	if( !fCompPB->GetTexmap(kLayer) )
	{
		pErrMsg->Set(true, GetINode()->GetName(), "No layer setup. Ignoring Footprint generator").CheckAndAsk();
		pErrMsg->Set(false);
		fValid = false;
		return true;
	}

	fDecalMgr = nil;

	fValid = true;
	fNotifiesSetup = false;

	return true;
}

hsBool plFootPrintComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		ISetupDecalMgr(node, pErrMsg, TRACKED_NEW plDynaFootMgr);
	}

	return true;
}

hsBool plFootPrintComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	if( !fValid )
		return true;

	if( !fNotifiesSetup )
		ISetupNotifies(node, pErrMsg);

	// Add this node's object to our DynaDecalMgr.
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefTarget), plRefFlags::kActiveRef);

	return true; 
}

hsBool plFootPrintComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fDecalMgr = nil;
	return true;
}

plFootPrintComponent::plFootPrintComponent()
:	fDecalMgr(nil)
{
	fClassDesc = &gFootPrintCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plFootPrintComponent::~plFootPrintComponent()
{
}

void plFootPrintComponent::IFakeParams()
{
	fCompPB->SetValue(kFadeIn, TimeValue(0), 0.1f);
	fCompPB->SetValue(kFadeOut, TimeValue(0), fCompPB->GetFloat(kLifeSpan) * 0.25f);
}

hsBool plFootPrintComponent::ISetupDecalMgr(plMaxNode* node, plErrorMsg* pErrMsg, plDynaDecalMgr* decalMgr)
{
	IFakeParams();

	fDecalMgr = decalMgr;
	plKey mgrKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), fDecalMgr, node->GetLocation());

	float width = fCompPB->GetFloat(kWidth) * 1.e-2f;
	float length = fCompPB->GetFloat(kLength) * 1.e-2f;
	float wetTime = fCompPB->GetFloat(kDirtyTime);
	float fadeIn = fCompPB->GetFloat(kFadeIn);
	float fadeOut = fCompPB->GetFloat(kFadeOut);
	float lifeSpan = fCompPB->GetFloat(kLifeSpan);
	float intensity = fCompPB->GetFloat(kIntensity) * 1.e-2f;
	float partyTime = fCompPB->GetFloat(kPartyTime);

	const hsScalar kHeightHack = 1.f;
	fDecalMgr->SetScale(hsVector3(width, length, kHeightHack));

	const float kMinFadeOut = 1.e-2f;
	if( fadeOut > lifeSpan - kMinFadeOut)
		fadeOut = lifeSpan - kMinFadeOut;
	if( fadeIn > lifeSpan - fadeOut )
		fadeIn = lifeSpan - fadeOut;

	fDecalMgr->SetWetLength(wetTime);
	fDecalMgr->SetRampEnd(fadeIn);
	fDecalMgr->SetDecayStart(lifeSpan - fadeOut);
	fDecalMgr->SetLifeSpan(lifeSpan);
	fDecalMgr->SetIntensity(intensity);
	fDecalMgr->SetPartyTime(partyTime);

	if( !ICreateDecalMaterials(node, pErrMsg) )
	{
		delete fDecalMgr;
		fDecalMgr = nil;
		return fValid = false;
	}

	hsgResMgr::ResMgr()->AddViaNotify(mgrKey, TRACKED_NEW plNodeRefMsg(node->GetRoomKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric), plRefFlags::kActiveRef);

	ISetupParticles(node, pErrMsg);

	return true;
}

hsBool plFootPrintComponent::ISetupNotifies(plMaxNode* node, plErrorMsg* pErrMsg)
{
	int num = fCompPB->Count(kNotifies);
	int i;
	for( i = 0; i < num; i++ )
	{
		plDynaDecalMgr* slave = GetDecalMgr(fCompPB->GetINode(kNotifies, TimeValue(0), i));
		if( slave )
		{
			slave->SetWaitOnEnable(true);
			fDecalMgr->AddNotify(slave->GetKey());
		}
	}
	fNotifiesSetup = true;

	return true;
}

hsBool plFootPrintComponent::ISetupParticles(plMaxNode* node, plErrorMsg* pErrMsg)
{
	int num = fCompPB->Count(kParticles);
	if( !num )
		return true;

	int i;
	for( i = 0; i < num; i++ )
	{
		plParticleComponent* partyComp = IGetParticleComp(fCompPB->GetINode(kParticles, TimeValue(0), i));
		if( partyComp && partyComp->NumTargets() )
		{
			plMaxNodeBase* partyNode = nil;
			const int numTarg = partyComp->NumTargets();
			int j;
			for( j = 0; j < numTarg; j++ )
			{
				partyNode = partyComp->GetTarget(j);
				if( partyNode )
					break;
			}
			if( partyNode )
			{
				plSceneObject* obj = partyNode->GetSceneObject();
				if( obj )
				{
					hsgResMgr::ResMgr()->AddViaNotify(obj->GetKey(), 
						new plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefPartyObject), plRefFlags::kPassiveRef);
				}
			}
			plConst(int) kNumEmitters(3);
			partyComp->SetEmitterReserve(kNumEmitters);
		}
	}

	return true;
}

hsBool plFootPrintComponent::ICreateDecalMaterials(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( fCompPB->GetInt(kBlend) != kAlpha )
		return ISetupColorDecalMaterials(node, pErrMsg);

	hsGMaterial* matRTShade = hsMaterialConverter::Instance().NonAlphaHackPrint(node, fCompPB->GetTexmap(kLayer), hsGMatState::kBlendAlpha);

	if( !matRTShade )
		return fValid = false;

	hsgResMgr::ResMgr()->AddViaNotify(matRTShade->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefMatRTShade), plRefFlags::kActiveRef);

	hsGMaterial* matPreShade = hsMaterialConverter::Instance().AlphaHackPrint(node, fCompPB->GetTexmap(kLayer), hsGMatState::kBlendAlpha);

	hsgResMgr::ResMgr()->AddViaNotify(matPreShade->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefMatPreShade), plRefFlags::kActiveRef);

	return true;
}

hsBool plFootPrintComponent::ISetupColorDecalMaterials(plMaxNode* node, plErrorMsg* pErrMsg)
{
	UInt32 blendFlags = 0;
	switch( fCompPB->GetInt(kBlend) )
	{
	case kMADD:
		blendFlags = hsGMatState::kBlendMADD;
		break;
	case kAdd:
		blendFlags = hsGMatState::kBlendAdd;
		break;
	case kMult:
		blendFlags = hsGMatState::kBlendMult;
		break;
	default:
		hsAssert(false, "Unknown blend mode");
		blendFlags = hsGMatState::kBlendMADD; // cuz it's my fave.
		break;
	}
	hsGMaterial* matRTShade = hsMaterialConverter::Instance().NonAlphaHackPrint(node, fCompPB->GetTexmap(kLayer), blendFlags);

	if( blendFlags & hsGMatState::kBlendMult )
	{
		plLayer* layer = plLayer::ConvertNoRef(matRTShade->GetLayer(0)->BottomOfStack());
		if( !layer )
			return fValid = false;

		layer->SetBlendFlags(layer->GetBlendFlags() | hsGMatState::kBlendInvertFinalColor);
	}

	hsgResMgr::ResMgr()->AddViaNotify(matRTShade->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefMatRTShade), plRefFlags::kActiveRef);

	return true;
}

plParticleComponent* plFootPrintComponent::IGetParticleComp(INode* node)
{
	if( !node )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( comp == nil )
		return nil;

	if( comp->ClassID() == PARTICLE_SYSTEM_COMPONENT_CLASS_ID )
	{
		plParticleComponent* party = (plParticleComponent*)comp;
		return party;
	}

	return nil;
}

plDynaDecalMgr* plFootPrintComponent::GetDecalMgr(INode* node)
{
	if( !node )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( comp == nil )
		return nil;

	if( (comp->ClassID() == FOOTPRINT_COMP_CID)
		|| (comp->ClassID() == RIPPLE_COMP_CID) 
		|| (comp->ClassID() == PUDDLE_COMP_CID) 
		|| (comp->ClassID() == WAKE_COMP_CID)
		)
	{
		plFootPrintComponent* foot = (plFootPrintComponent*)comp;
		return foot->fDecalMgr;
	}

	return nil;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class plRippleComponent : public plFootPrintComponent
{
protected:
	virtual void	IFakeParams();
public:
	
	plRippleComponent();
	virtual ~plRippleComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plRippleComponent, gRippleCompDesc, "Ripple",  "Ripple", COMP_TYPE_FOOTPRINT, RIPPLE_COMP_CID)


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gRippleBk
(	
	plComponent::kBlkComp, _T("Ripple"), 0, &gRippleCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_RIPPLE, IDS_COMP_RIPPLE, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_WIDTH, IDC_COMP_FP_WIDTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
//		p_default, 1.0,
//		p_range, 0.25, 10.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 0.1,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 3.5,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
//		p_default, 5.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.25,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.25,
		p_range, 0.1, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

void plRippleComponent::IFakeParams()
{
	fCompPB->SetValue(kLength, TimeValue(0), fCompPB->GetFloat(kWidth));
	fCompPB->SetValue(kFadeIn, TimeValue(0), 0.25f);
	fCompPB->SetValue(kLifeSpan, TimeValue(0), fCompPB->GetFloat(kWidth) * 1.e-2f * 0.5f);
	fCompPB->SetValue(kFadeOut, TimeValue(0), fCompPB->GetFloat(kLifeSpan) * 0.80f);
}

hsBool plRippleComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return plFootPrintComponent::SetupProperties(node, pErrMsg);
}

hsBool plRippleComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		plDynaRippleMgr* ripple = nil;
		if( node->GetVS() || node->UserPropExists("XXXWaterColor") )
		{
			ripple = TRACKED_NEW plDynaRippleVSMgr;
		}
		else
		{
			ripple = TRACKED_NEW plDynaRippleMgr;
		}
		ISetupDecalMgr(node, pErrMsg, ripple);
		if( fValid )
		{
			ripple->SetUVWAnim(hsVector3(3.f, 3.f, 3.f), hsVector3(1.f, 1.f, 1.f));
		}
	}

	return true;
}

hsBool plRippleComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	if( !fValid )
		return true;

	if( !fNotifiesSetup )
	{
		plWaveSetBase* waveSet = plWaterComponent::GetWaveSetFromNode(node);
		if( waveSet )
		{
			plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaRippleVSMgr::kRefWaveSetBase);
			hsgResMgr::ResMgr()->AddViaNotify(waveSet->GetKey(), refMsg, plRefFlags::kPassiveRef);
		}

		ISetupNotifies(node, pErrMsg);
	}

	// Add this node's object to our DynaDecalMgr.
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefTarget), plRefFlags::kActiveRef);

	return true; 
}

plRippleComponent::plRippleComponent()
{
	fClassDesc = &gRippleCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plRippleComponent::~plRippleComponent()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


class plPuddleComponent : public plRippleComponent
{
protected:
public:
	
	plPuddleComponent();
	virtual ~plPuddleComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plPuddleComponent, gPuddleCompDesc, "Puddle",  "Puddle", COMP_TYPE_FOOTPRINT, PUDDLE_COMP_CID)


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gPuddleBk
(	
	plComponent::kBlkComp, _T("Puddle"), 0, &gPuddleCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_PUDDLE, IDS_COMP_PUDDLE, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_WIDTH, IDC_COMP_FP_WIDTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
//		p_default, 1.0,
//		p_range, 0.25, 10.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 0.1,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 3.5,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
//		p_default, 5.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.25,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.25,
		p_range, 0.1, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

hsBool plPuddleComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return plRippleComponent::SetupProperties(node, pErrMsg);
}

hsBool plPuddleComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		plDynaRippleMgr* puddle = TRACKED_NEW plDynaPuddleMgr;

		ISetupDecalMgr(node, pErrMsg, puddle);
		if( fValid )
		{
			puddle->SetUVWAnim(hsVector3(5.f, 5.f, 5.f), hsVector3(1.f, 1.f, 1.f));
		}
	}
	return true;
}

hsBool plPuddleComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	if( !fValid )
		return true;

	if( !fNotifiesSetup )
		ISetupNotifies(node, pErrMsg);

	// Add this node's object to our DynaDecalMgr.
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefTarget), plRefFlags::kActiveRef);

	return true; 
}

plPuddleComponent::plPuddleComponent()
{
	fClassDesc = &gPuddleCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plPuddleComponent::~plPuddleComponent()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


class plBulletComponent : public plFootPrintComponent
{
protected:
	virtual void	IFakeParams();
public:
	
	plBulletComponent();
	virtual ~plBulletComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plBulletComponent, gBulletCompDesc, "Bullet",  "Bullet", COMP_TYPE_FOOTPRINT, BULLET_COMP_CID)


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gBulletBk
(	
	plComponent::kBlkComp, _T("Bullet"), 0, &gBulletCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_BULLET, IDS_COMP_FP_BULLET, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 400.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_SCALE, IDC_COMP_FP_SCALE_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
//		p_default, 100.0,
//		p_range, 25.0, 400.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 10.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
		p_default, 15.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.1,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
//		p_default, 10.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
//		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.25,
		p_range, 0.1, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

void plBulletComponent::IFakeParams()
{
	fCompPB->SetValue(kLength, TimeValue(0), fCompPB->GetFloat(kWidth));
	plFootPrintComponent::IFakeParams();
}

hsBool plBulletComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return plFootPrintComponent::SetupProperties(node, pErrMsg);
}

hsBool plBulletComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		plDynaBulletMgr* bullet = TRACKED_NEW plDynaBulletMgr;

		ISetupDecalMgr(node, pErrMsg, bullet);
	}
	return true;
}

hsBool plBulletComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	if( !fValid )
		return true;

	if( !fNotifiesSetup )
		ISetupNotifies(node, pErrMsg);

	// Add this node's object to our DynaDecalMgr.
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefTarget), plRefFlags::kActiveRef);

	return true; 
}

plBulletComponent::plBulletComponent()
{
	fClassDesc = &gBulletCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plBulletComponent::~plBulletComponent()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


class plTorpedoComponent : public plRippleComponent
{
protected:
	virtual void	IFakeParams();
public:
	
	plTorpedoComponent();
	virtual ~plTorpedoComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plTorpedoComponent, gTorpedoCompDesc, "Water Bullet",  "WetBullet", COMP_TYPE_FOOTPRINT, TORPEDO_COMP_CID)


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gTorpedoBk
(	
	plComponent::kBlkComp, _T("Torpedo"), 0, &gTorpedoCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_TORPEDO, IDS_COMP_FP_TORPEDO, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_WIDTH, IDC_COMP_FP_WIDTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
//		p_default, 1.0,
//		p_range, 0.25, 10.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 0.1,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 3.5,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
		p_default, 5.0,
		p_range, 0.0, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.25,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.25,
		p_range, 0.1, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

void plTorpedoComponent::IFakeParams()
{
	fCompPB->SetValue(kLength, TimeValue(0), fCompPB->GetFloat(kWidth));
	fCompPB->SetValue(kFadeIn, TimeValue(0), 0.25f);
	fCompPB->SetValue(kFadeOut, TimeValue(0), fCompPB->GetFloat(kLifeSpan) * 0.80f);
}

hsBool plTorpedoComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return plRippleComponent::SetupProperties(node, pErrMsg);
}

hsBool plTorpedoComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		plDynaRippleMgr* torpedo;
		if( node->GetVS() )
			torpedo = TRACKED_NEW plDynaTorpedoVSMgr;
		else
			torpedo = TRACKED_NEW plDynaTorpedoMgr;

		ISetupDecalMgr(node, pErrMsg, torpedo);
		if( fValid )
		{
			torpedo->SetUVWAnim(hsVector3(5.f, 5.f, 5.f), hsVector3(1.f, 1.f, 1.f));
		}
	}
	return true;
}

hsBool plTorpedoComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return plRippleComponent::Convert(node, pErrMsg);
}

plTorpedoComponent::plTorpedoComponent()
{
	fClassDesc = &gTorpedoCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plTorpedoComponent::~plTorpedoComponent()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class plWakeComponent : public plFootPrintComponent
{
protected:
	virtual void	IFakeParams();
public:
	
	plWakeComponent();
	virtual ~plWakeComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plWakeComponent, gWakeCompDesc, "Wake",  "Wake", COMP_TYPE_FOOTPRINT, WAKE_COMP_CID)


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gWakeBk
(	
	plComponent::kBlkComp, _T("Wake"), 0, &gWakeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_WAKE, IDS_COMP_FP_WAKE, 0, 0, &gWetProc,

	plFootPrintComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_WIDTH, IDC_COMP_FP_WIDTH_SPIN, 1.0,
		end,	

	plFootPrintComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 25.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_LENGTH, IDC_COMP_FP_LENGTH_SPIN, 0.1,
		end,	

	plFootPrintComponent::kFadeOut, _T("FadeOut"), TYPE_FLOAT, 	0, 0,	
//		p_default, 3.5,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_FADEOUT, IDC_COMP_FP_FADEOUT_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLifeSpan, _T("LifeSpan"), TYPE_FLOAT, 	0, 0,	
//		p_default, 5.0,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_LIFESPAN2, IDC_COMP_FP_LIFESPAN_SPIN2, 0.1,
		end,	

	plFootPrintComponent::kFadeIn, _T("FadeIn"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.25,
//		p_range, 0.0, 300.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_RAMPEND, IDC_COMP_FP_RAMPEND_SPIN, 0.1,
		end,	

	plFootPrintComponent::kLayer,	_T("Layer"),	TYPE_TEXMAP, 0, 0,
		p_ui, TYPE_TEXMAPBUTTON, IDC_COMP_FP_TEXMAP,
		end,

	plFootPrintComponent::kBlend, _T("Blend"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 4,	IDC_RADIO_ALPHA, IDC_RADIO_MADD, IDC_RADIO_ADD,	IDC_RADIO_MULT,
		p_vals,	plFootPrintComponent::kAlpha, plFootPrintComponent::kMADD, plFootPrintComponent::kAdd, plFootPrintComponent::kMult,
		p_default, plFootPrintComponent::kAlpha,
		end,

	plFootPrintComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	plFootPrintComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plFootPrintComponent::kIntensity, _T("Intensity"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_INTENSITY, IDC_COMP_FP_INTENSITY_SPIN, 1.0,
		end,	

	plFootPrintComponent::kParticles,	_T("Particles"),	TYPE_INODE_TAB, 0,		0, 0,
//		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS2, 0, 0, IDC_DEL_TARGS2,
		end,

	plFootPrintComponent::kPartyTime, _T("PartyTime"), TYPE_FLOAT, 	0, 0,	
//		p_default, 0.25,
//		p_range, 0.1, 5.0,
//		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
//		IDC_COMP_FP_PARTYTIME, IDC_COMP_FP_PARTYTIME_SPIN, 0.1,
		end,	

	end
);

void plWakeComponent::IFakeParams()
{
	fCompPB->SetValue(kFadeIn, TimeValue(0), 0.25f);
	fCompPB->SetValue(kLifeSpan, TimeValue(0), fCompPB->GetFloat(kWidth) * 1.e-2f * 0.5f);
	fCompPB->SetValue(kFadeOut, TimeValue(0), fCompPB->GetFloat(kLifeSpan) * 0.80f);
}

hsBool plWakeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return plFootPrintComponent::SetupProperties(node, pErrMsg);
}

hsBool plWakeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	// If we haven't already, create our DynaDecalMgr and stash it away.
	if( !fDecalMgr )
	{
		plDynaWakeMgr* wake = TRACKED_NEW plDynaWakeMgr;
		ISetupDecalMgr(node, pErrMsg, wake);
		if( fValid )
		{
			wake->SetUVWAnim(hsVector3(5.f, 5.f, 5.f), hsVector3(1.f, 1.f, 1.f));
		}
	}

	return true;
}

hsBool plWakeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	if( !fValid )
		return true;

	if( !fNotifiesSetup )
		ISetupNotifies(node, pErrMsg);

	// Add this node's object to our DynaDecalMgr.
	hsgResMgr::ResMgr()->AddViaNotify(node->GetKey(), TRACKED_NEW plGenRefMsg(fDecalMgr->GetKey(), plRefMsg::kOnCreate, 0, plDynaDecalMgr::kRefTarget), plRefFlags::kActiveRef);

	return true; 
}

plWakeComponent::plWakeComponent()
{
	fClassDesc = &gWakeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plWakeComponent::~plWakeComponent()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

class plDirtyComponent : public plComponent
{
public:
	enum 
	{
		kDecals,
		kDirtyTime
	};
protected:
public:
	
	plDirtyComponent();
	virtual ~plDirtyComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plDirtyComponent, gDirtyCompDesc, "Dirty/Wet Region",  "Dirty/Wet", COMP_TYPE_FOOTPRINT, DIRTY_COMP_CID)


class plDirtyProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
			}
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_DECAL)
			{
				std::vector<Class_ID> cids;
				cids.push_back(FOOTPRINT_COMP_CID);
				cids.push_back(RIPPLE_COMP_CID);
				cids.push_back(PUDDLE_COMP_CID);
				cids.push_back(WAKE_COMP_CID);
				IParamBlock2 *pb = map->GetParamBlock();
				plPick::Node(pb, plDirtyComponent::kDecals, &cids, false, false);

				map->Invalidate(plDirtyComponent::kDecals);
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plDirtyProc gDirtyProc;


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gDirtyBk
(	
	plComponent::kBlkComp, _T("Dirty"), 0, &gDirtyCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_DIRTY, IDS_COMP_FP_DIRTY, 0, 0, &gDirtyProc,

	plDirtyComponent::kDecals,	_T("Decals"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	plDirtyComponent::kDirtyTime, _T("DirtyTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 300.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FP_DIRTYTIME, IDC_COMP_FP_DIRTYTIME_SPIN, 0.1,
		end,	

	end
);

hsBool plDirtyComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plDirtyComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plDirtyComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	// Check that this node has a physical interface, or all is for nought.
	// Should throw up a warning if it doesn't have one, seems an easy thing
	// to miss.
	if( !node->IsPhysical() )
	{
		pErrMsg->Set(true, node->GetName(), "Has no physical component to notify %s Dirty/Wet component", GetINode()->GetName()).CheckAndAsk();
		pErrMsg->Set(false);
		return true;
	}


	plDecalEnableMod* enable = TRACKED_NEW plDecalEnableMod;
	plKey modKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), enable, node->GetLocation());

	int numDecals = fCompPB->Count(kDecals);
	int i;
	for( i = 0; i < numDecals; i++ )
	{
		INode* decalNode = fCompPB->GetINode(kDecals, TimeValue(0), i);

		plDynaDecalMgr* decal = plFootPrintComponent::GetDecalMgr(decalNode);

		if( decal )
		{
			decal->SetWaitOnEnable(true);
			enable->AddDecalKey(decal->GetKey());
		}
	}
	enable->SetWetLength(fCompPB->GetFloat(kDirtyTime));
	hsgResMgr::ResMgr()->AddViaNotify(modKey, TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

	return true; 
}

plDirtyComponent::plDirtyComponent()
{
	fClassDesc = &gDirtyCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plDirtyComponent::~plDirtyComponent()
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

const Class_ID PRINTSHAPE_COMP_CID(0x208226a1, 0x2a6e67ba);

class plPrintShapeComponent : public plComponent
{
public:
	enum 
	{
		kWidth,
		kLength,
		kHeight
	};
protected:
public:
	
	plPrintShapeComponent();
	virtual ~plPrintShapeComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plPrintShapeComponent, gPrintShapeCompDesc, "Print Shape",  "PrintShape", COMP_TYPE_FOOTPRINT, PRINTSHAPE_COMP_CID)



/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gPrintShapeBk
(	
	plComponent::kBlkComp, _T("PrintShape"), 0, &gPrintShapeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_PRINTSHAPE, IDS_COMP_FP_PRINTSHAPE, 0, 0, NULL,

	plPrintShapeComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.45,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_WIDTH, IDC_COMP_WIDTH_SPIN, 0.1,
		end,	

	plPrintShapeComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.9,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LENGTH, IDC_COMP_LENGTH_SPIN, 0.1,
		end,	

	plPrintShapeComponent::kHeight, _T("Height"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_HEIGHT, IDC_COMP_HEIGHT_SPIN, 0.1,
		end,	



	end
);

hsBool plPrintShapeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	return true;
}

hsBool plPrintShapeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plPrintShapeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	plSceneObject* obj = node->GetSceneObject();
	if( !obj )
		return true;

	plPrintShape* shape = TRACKED_NEW plPrintShape();
	plKey shapeKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), shape, node->GetLocation());

	shape->SetWidth(fCompPB->GetFloat(kWidth));
	shape->SetLength(fCompPB->GetFloat(kLength));
	shape->SetHeight(fCompPB->GetFloat(kHeight));

	hsgResMgr::ResMgr()->AddViaNotify(shapeKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	return true; 
}

plPrintShapeComponent::plPrintShapeComponent()
{
	fClassDesc = &gPrintShapeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plPrintShapeComponent::~plPrintShapeComponent()
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

const Class_ID ACTIVEPRINTSHAPE_COMP_CID(0x61b52046, 0x787734a5);

class plActivePrintShapeComponent : public plComponent
{
public:
	enum 
	{
		kWidth,
		kLength,
		kHeight,
		kNotifies
	};
protected:
public:
	
	plActivePrintShapeComponent();
	virtual ~plActivePrintShapeComponent();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plActivePrintShapeComponent, gActivePrintShapeCompDesc, "Active Shape",  "ActiveShape", COMP_TYPE_FOOTPRINT, ACTIVEPRINTSHAPE_COMP_CID)


class plActiveProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
			}
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_NOTIFY2)
			{
				std::vector<Class_ID> cids;
				cids.push_back(FOOTPRINT_COMP_CID);
				cids.push_back(RIPPLE_COMP_CID);
				cids.push_back(PUDDLE_COMP_CID);
				cids.push_back(WAKE_COMP_CID);
				IParamBlock2 *pb = map->GetParamBlock();
				plPick::Node(pb, plActivePrintShapeComponent::kNotifies, &cids, false, false);

				map->Invalidate(plActivePrintShapeComponent::kNotifies);
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plActiveProc gActiveProc;


/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gActivePrintShapeBk
(	
	plComponent::kBlkComp, _T("ActivePrintShape"), 0, &gActivePrintShapeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_FP_ACTIVEPRINTSHAPE, IDS_COMP_FP_ACTIVEPRINTSHAPE, 0, 0, &gActiveProc,

	plActivePrintShapeComponent::kWidth, _T("Width"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.45,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_WIDTH, IDC_COMP_WIDTH_SPIN, 0.1,
		end,	

	plActivePrintShapeComponent::kLength, _T("Length"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.9,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LENGTH, IDC_COMP_LENGTH_SPIN, 0.1,
		end,	

	plActivePrintShapeComponent::kHeight, _T("Height"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 30.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_HEIGHT, IDC_COMP_HEIGHT_SPIN, 0.1,
		end,	

	plActivePrintShapeComponent::kNotifies,	_T("Notifies"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
		end,

	end
);

hsBool plActivePrintShapeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	return true;
}

hsBool plActivePrintShapeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plActivePrintShapeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	plSceneObject* obj = node->GetSceneObject();
	if( !obj )
		return true;

	plActivePrintShape* shape = TRACKED_NEW plActivePrintShape();
	plKey shapeKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), shape, node->GetLocation());

	shape->SetWidth(fCompPB->GetFloat(kWidth));
	shape->SetLength(fCompPB->GetFloat(kLength));
	shape->SetHeight(fCompPB->GetFloat(kHeight));

	int num = fCompPB->Count(kNotifies);
	int i;
	for( i = 0; i < num; i++ )
	{
		plDynaDecalMgr* notify = plFootPrintComponent::GetDecalMgr(fCompPB->GetINode(kNotifies, TimeValue(0), i));
		if( notify )
		{
			shape->AddDecalKey(notify->GetKey());
		}
	}


	hsgResMgr::ResMgr()->AddViaNotify(shapeKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	return true; 
}

plActivePrintShapeComponent::plActivePrintShapeComponent()
{
	fClassDesc = &gActivePrintShapeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plActivePrintShapeComponent::~plActivePrintShapeComponent()
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////////

