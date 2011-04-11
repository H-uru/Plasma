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
#include "../MaxMain/plMaxNode.h"
#include "hsResMgr.h"


// LightGroup component
#include "../pnSceneObject/plSceneObject.h"
#include "../plGLight/plLightInfo.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../MaxPlasmaLights/plRealTimeLightBase.h"

#include "plLightGrpComponent.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LightGroup Component
//
//

enum	
{
	kIncludeChars,
	kAffectedLightSel,
	kTest
};




CLASS_DESC(plLightGrpComponent, gLightGrpDesc, "Light Group",  "LightGroup", COMP_TYPE_GRAPHICS, LIGHTGRP_COMP_CID)



ParamBlockDesc2 gLightGrpBk
(
	plComponent::kBlkComp, _T("LightGroup"), 0, &gLightGrpDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_LIGHTINC, IDS_COMP_LIGHTINCS,  0, 0, nil,

	kIncludeChars,  _T("Include characters"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTINC_CHARS,
		end,

	kAffectedLightSel, _T("AffectedLightChoice"),	TYPE_INODE,		0, 0,
		end,

	kTest, _T("TestBox"), TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTINC_FILTER,
		end,

	end

);

plLightGrpComponent::plLightGrpComponent()
:	fValid(false)
{
	fClassDesc = &gLightGrpDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

class plLightGrpPostLoadCallback : public PostLoadCallback
{
public:
	plLightGrpComponent*	fLightGrp;

	plLightGrpPostLoadCallback(plLightGrpComponent* lg) : fLightGrp(lg) {}

	void proc(ILoad *iload) 
	{
		IParamBlock2* compPB = fLightGrp->GetParamBlock(plComponentBase::kBlkComp);
		INode* light = compPB->GetINode(kAffectedLightSel);
		if( light )
		{
			fLightGrp->AddTarget((plMaxNodeBase*)light);
			compPB->SetValue(kAffectedLightSel, TimeValue(0), (INode*)nil);
		}
		delete this;
	}
};

IOResult plLightGrpComponent::Load(ILoad* iLoad)
{
	iLoad->RegisterPostLoadCallback(new plLightGrpPostLoadCallback(this));

	return plComponent::Load(iLoad);
}

hsBool plLightGrpComponent::IAddLightsToSpans(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
	int i;
	for( i = 0; i < fLightInfos.GetCount(); i++ )
	{
		if( !fLightInfos[i] )
			continue;

		const plDrawInterface* di = pNode->GetSceneObject()->GetDrawInterface();

		int iDraw;
		for( iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++ )
		{
			plDrawableSpans* drawable = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
			if( drawable )
			{
				UInt32 diIndex = di->GetDrawableMeshIndex(iDraw);

				ISendItOff(fLightInfos[i], drawable, diIndex);
			}
		}
	}
	return true;
}

hsBool plLightGrpComponent::ISendItOff(plLightInfo* liInfo, plDrawableSpans* drawable, UInt32 diIndex)
{
	plDISpanIndex spans = drawable->GetDISpans(diIndex);

	if( spans.fFlags & plDISpanIndex::kMatrixOnly )
		return false;

	if( !fCompPB->GetInt(kTest) )
	{
		UInt8 liMsgType = liInfo->GetProjection() ? plDrawable::kMsgPermaProjDI : plDrawable::kMsgPermaLightDI;
		plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(drawable->GetKey(), plRefMsg::kOnCreate, diIndex, liMsgType);
		hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), refMsg, plRefFlags::kPassiveRef);
	}
	else
	{

		hsBitVector litSpans;
		liInfo->GetAffectedForced(drawable->GetSpaceTree(), litSpans, false);

		UInt8 liMsgType = liInfo->GetProjection() ? plDrawable::kMsgPermaProj : plDrawable::kMsgPermaLight;
		int i;
		for( i = 0; i < spans.GetCount(); i++ )
		{
			if( litSpans.IsBitSet(spans[i]) )
			{
				plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(drawable->GetKey(), plRefMsg::kOnCreate, spans[i], liMsgType);
				hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), refMsg, plRefFlags::kPassiveRef);
			}
		}
	}

	return true;
}

hsBool plLightGrpComponent::IGetLightInfos()
{
	if( !fLightInfos.GetCount() )
	{
		// Already checked that lightnodes are cool. just get the light interfaces.
		int i;
		for( i = 0; i < fLightNodes.GetCount(); i++ )
		{
			plMaxNode* lightNode = fLightNodes[i];
			plSceneObject* lightSO = lightNode->GetSceneObject();
			if( !lightSO )
				continue;

			plLightInfo* liInfo = plLightInfo::ConvertNoRef(lightSO->GetGenericInterface(plLightInfo::Index()));
			if( !liInfo )
				continue;

			liInfo->SetProperty(plLightInfo::kLPHasIncludes, true);
			if( fCompPB->GetInt(kIncludeChars) )
				liInfo->SetProperty(plLightInfo::kLPIncludesChars, true);
			fLightInfos.Append(liInfo);
		}
	}
	return fValid = (fLightInfos.GetCount() > 0);
}

const hsTArray<plLightInfo*>& plLightGrpComponent::GetLightInfos()
{
	IGetLightInfos();
	return fLightInfos;
}

plLightGrpComponent* plLightGrpComponent::GetComp(plMaxNode* node)
{
	int i;
	for( i = 0; i < node->NumAttachedComponents(); i++ )
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if( comp && comp->ClassID() == LIGHTGRP_COMP_CID )
			return (plLightGrpComponent*)comp;
	}
	return nil;
}

hsBool plLightGrpComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	const char* dbgNodeName = node->GetName();
	if( !fValid )
		return true;

	if( !IGetLightInfos() )
		return true;

	if( !node->GetDrawable() )
		return true;

	if( !node->GetSceneObject() || !node->GetSceneObject()->GetDrawInterface() )
		return true;

	// If it's shaded as a character, ignore any light groups attached.
	if( node->GetItinerant() )
		return true;

	IAddLightsToSpans(node, pErrMsg);

	return true;
}

hsBool plLightGrpComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
	fValid = false;
	fLightInfos.Reset();
	fLightNodes.Reset();

	int i;
	for( i = 0; i < NumTargets(); i++ )
	{
		plMaxNodeBase* liNode = GetTarget(i);

		if( liNode && liNode->CanConvert() )
		{
			Object *obj = liNode->GetObjectRef();
			if( obj )
			{
				Class_ID cid = obj->ClassID();

				if( (cid == RTSPOT_LIGHT_CLASSID)
					|| (cid == RTOMNI_LIGHT_CLASSID)
					|| (cid == RTDIR_LIGHT_CLASSID)
					|| (cid == RTPDIR_LIGHT_CLASSID) )
				{
					fLightNodes.Append((plMaxNode*)liNode);
				}
			}
		}

	}
	if( !fLightNodes.GetCount() )
		return true;

	fValid = true;
	return true;
}

hsBool plLightGrpComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
	if( !fValid )
		return true;

	fValid = false;

	fValid = true;


	return true;
}

