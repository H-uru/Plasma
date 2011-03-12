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
#include "../MaxExport/plExportProgressBar.h"

#include "hsTypes.h"

#include "plLODFadeComponent.h"

#include "../pfSurface/plFadeOpacityMod.h"
#include "../pfSurface/plDistOpacityMod.h"

void DummyCodeIncludeFuncLODFade()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// LODFadeComponent first, similar and related BlendOnto component further on in file.
CLASS_DESC(plLODFadeComponent, gLODFadeCompDesc, "LOD Blend",  "LODBlend", COMP_TYPE_GRAPHICS, LODFADE_COMP_CID)

ParamBlockDesc2 gLODFadeBk
(	
	plComponent::kBlkComp, _T("LODFade"), 0, &gLODFadeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_LODFADE, IDS_COMP_LODFADE, 0, 0, NULL,

	plLODFadeComponent::kHasBase,	_T("HasBase"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LODFADE_HASBASE,
		end,

	plLODFadeComponent::kBase, _T("Base"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_LODFADE_BASE,
		p_prompt, IDS_COMP_LODFADE_BASE,
		end,

	plLODFadeComponent::kDistance, _T("Distance"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LODFADE_DISTANCE, IDC_COMP_LODFADE_DISTANCE_SPIN, 1.0,
		end,	

	plLODFadeComponent::kTransition, _T("Transition"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_LODFADE_TRANSITION, IDC_COMP_LODFADE_TRANSITION_SPIN, 1.0,
		end,	

	plLODFadeComponent::kFadeBase,	_T("FadeBase"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LODFADE_FADEBASE,
		p_enable_ctrls,		1, plLODFadeComponent::kBaseFirst,
		end,

	plLODFadeComponent::kBaseFirst,	_T("BaseFirst"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_LODFADE_BASEFIRST,
		end,


	end
);

void plLODFadeComponent::ISetToFadeBase(plMaxNode* node, plMaxNode* base, plErrorMsg* pErrMsg)
{
	if( fCompPB->GetInt(kBaseFirst) )
		node->AddRenderDependency(base);
	else
		base->AddRenderDependency(node);

	Box3 fade = base->GetFade();
	Point3 maxs = fade.Max();
	float fadeInStart = fCompPB->GetFloat(kDistance) - fCompPB->GetFloat(kTransition);
	if( fadeInStart < 0 )
		fadeInStart = 0;
	float fadeInEnd = fCompPB->GetFloat(kDistance);
	Point3 mins(fadeInStart, fadeInEnd, -1.f);
	fade = Box3(mins, maxs);
	base->SetFade(fade);

	node->SetNoDeferDraw(true);
	base->SetNoDeferDraw(true);
}

hsBool plLODFadeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( fCompPB->GetInt(kHasBase) )
	{
		plMaxNode* base = (plMaxNode*)fCompPB->GetINode(kBase, TimeValue(0));
		if( base )
		{
			if( fCompPB->GetInt(kFadeBase) )
			{
				ISetToFadeBase(node, base, pErrMsg);
			}
			else
			{
				node->AddRenderDependency(base);
				node->SetNoDeferDraw(true);
			}
		}
	}
	Box3 fade = node->GetFade();
	Point3 mins = fade.Min();
	float fadeOutStart = fCompPB->GetFloat(kDistance);
	float fadeOutEnd = fadeOutStart + fCompPB->GetFloat(kTransition);
	Point3 maxs(fadeOutEnd, fadeOutStart, 1.f);
	fade = Box3(mins, maxs);
	node->SetFade(fade);

	return true;
}

hsBool plLODFadeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plLODFadeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plLODFadeComponent::plLODFadeComponent()
{
	fClassDesc = &gLODFadeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// BlendOnto component next.
CLASS_DESC(plBlendOntoComponent, gBlendOntoCompDesc, "Blend Onto",  "BlendOnto", COMP_TYPE_GRAPHICS, BLENDONTO_COMP_CID)

ParamBlockDesc2 gBlendOntoBk
(	
	plComponent::kBlkComp, _T("BlendOnto"), 0, &gBlendOntoCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_BLENDONTO, IDS_COMP_BLENDONTO, 0, 0, NULL,

	plBlendOntoComponent::kBaseNodes,	_T("BaseNodes"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, IDC_ADD_TARGS, 0, IDC_DEL_TARGS,
		p_classID,		triObjectClassID,
		end,

	plBlendOntoComponent::kSortFaces,	_T("SortFaces"),	TYPE_BOOL, 0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_BLENDONTO_SORTFACES,
		end,


	end
);

hsBool plBlendOntoComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	hsBool someBase = false;
	int numBase = fCompPB->Count(kBaseNodes);
	int i;
	for( i = 0; i < numBase; i++ )
	{
		plMaxNode* base = (plMaxNode*)fCompPB->GetINode(kBaseNodes, TimeValue(0), i);

		if( base )
		{
			node->AddRenderDependency(base);
			node->SetNoDeferDraw(true);
			if( !fCompPB->GetInt(kSortFaces) )
				node->SetNoFaceSort(true);

			someBase = true;
		}
	}
	if( !someBase )
	{
		node->SetBlendToFB(true);
	}

	return true;
}

hsBool plBlendOntoComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plBlendOntoComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plBlendOntoComponent::plBlendOntoComponent()
{
	fClassDesc = &gBlendOntoCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// BlendOntoAdv component next.
CLASS_DESC(plBlendOntoAdvComponent, gBlendOntoAdvCompDesc, "Blend Onto Advanced",  "BlendOntoAdv", COMP_TYPE_GRAPHICS, BLENDONTOADV_COMP_CID)

ParamBlockDesc2 gBlendOntoAdvBk
(	
	plComponent::kBlkComp, _T("BlendOntoAdv"), 0, &gBlendOntoAdvCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_BLENDONTOADV, IDS_COMP_BLENDONTOADV, 0, 0, NULL,

	plBlendOntoAdvComponent::kBaseNodes,	_T("BaseNodes"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, IDC_ADD_TARGS, 0, IDC_DEL_TARGS,
		p_classID,		triObjectClassID,
		end,

	plBlendOntoAdvComponent::kSortFaces,	_T("SortFaces"),	TYPE_BOOL, 0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_BLENDONTOADV_SORTFACES,
		end,

	plBlendOntoAdvComponent::kSortObjects,	_T("SortObjects"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_BLENDONTOADV_SORTOBJECTS,
		end,

	plBlendOntoAdvComponent::kOntoBlending,	_T("OntoBlending"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_BLENDONTOADV_ONTOBLENDING,
		end,


	end
);

hsBool plBlendOntoAdvComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	hsBool someBase = false;
	int numBase = fCompPB->Count(kBaseNodes);
	int i;
	for( i = 0; i < numBase; i++ )
	{
		plMaxNode* base = (plMaxNode*)fCompPB->GetINode(kBaseNodes, TimeValue(0), i);

		if( base )
		{
			node->AddRenderDependency(base);
			if( !fCompPB->GetInt(kOntoBlending) )
				node->SetNoDeferDraw(true);
			if( !fCompPB->GetInt(kSortFaces) )
				node->SetNoFaceSort(true);

			someBase = true;
		}
	}
	if( !someBase )
	{
		node->SetBlendToFB(true);
	}

	return true;
}

hsBool plBlendOntoAdvComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plBlendOntoAdvComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plBlendOntoAdvComponent::plBlendOntoAdvComponent()
{
	fClassDesc = &gBlendOntoAdvCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Force drawing before the avatar.

const Class_ID B4AV_COMP_CID(0x14536d5b, 0x17dc623b);

class plB4AvComponent : public plComponent
{
public:
	plB4AvComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plB4AvComponent, gB4AvCompDesc, "Draw B4 Avatar",  "B4Av", COMP_TYPE_GRAPHICS, B4AV_COMP_CID)

ParamBlockDesc2 gB4AvBk
(	
	plComponent::kBlkComp, _T("B4Av"), 0, &gB4AvCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SORT_AS_OPAQUE, IDS_COMP_SORT_AS_OPAQUE, 0, 0, NULL,

	end
);

hsBool plB4AvComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetSortAsOpaque(true);
	return true;
}

hsBool plB4AvComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plB4AvComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plB4AvComponent::plB4AvComponent()
{
	fClassDesc = &gB4AvCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// DistFadeComponent - just feeding them a bit more rope.
CLASS_DESC(plDistFadeComponent, gDistFadeCompDesc, "Distance Fade",  "DistFade", COMP_TYPE_GRAPHICS, DISTFADE_COMP_CID)

ParamBlockDesc2 gDistFadeBk
(	
	plComponent::kBlkComp, _T("DistFade"), 0, &gDistFadeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DISTFADE, IDS_COMP_DISTFADE, 0, 0, NULL,

	plDistFadeComponent::kFadeInActive,	_T("FadeInActive"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTFADE_IN_ACTIVE,
		p_enable_ctrls,		2, plDistFadeComponent::kFadeInStart, plDistFadeComponent::kFadeInEnd,
		end,

	plDistFadeComponent::kFadeInStart, _T("FadeInStart"), TYPE_FLOAT, 	0, 0,	
		p_default, 5.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTFADE_INSTART, IDC_COMP_DISTFADE_INSTART_SPIN, 1.0,
		end,	

	plDistFadeComponent::kFadeInEnd, _T("FadeInEnd"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTFADE_INEND, IDC_COMP_DISTFADE_INEND_SPIN, 1.0,
		end,	

	plDistFadeComponent::kFadeOutActive,	_T("FadeOutActive"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTFADE_OUT_ACTIVE,
		p_enable_ctrls,		2, plDistFadeComponent::kFadeOutStart, plDistFadeComponent::kFadeOutEnd,
		end,

	plDistFadeComponent::kFadeOutStart, _T("FadeOutStart"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTFADE_OUTSTART, IDC_COMP_DISTFADE_OUTSTART_SPIN, 1.0,
		end,	

	plDistFadeComponent::kFadeOutEnd, _T("FadeOutEnd"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTFADE_OUTEND, IDC_COMP_DISTFADE_OUTEND_SPIN, 1.0,
		end,	


	end
);

hsBool plDistFadeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	// If we're turned off, just return.
	if( !fCompPB->GetInt(kFadeInActive) && !fCompPB->GetInt(kFadeOutActive) )
		return true;

	Box3 fade;
	Point3 mins(0.f, 0.f, 0.f);
	Point3 maxs(0.f, 0.f, 0.f);


	if( fCompPB->GetInt(kFadeInActive) )
	{
		mins[0] = fCompPB->GetFloat(kFadeInStart);
		mins[1] = fCompPB->GetFloat(kFadeInEnd);
	}
	if( fCompPB->GetInt(kFadeOutActive) )
	{
		maxs[0] = fCompPB->GetFloat(kFadeOutStart);
		maxs[1] = fCompPB->GetFloat(kFadeOutEnd);
	}

	// We're not really sure how the artist has artistically interpreted
	// the parameters. What we want is:
	//		Nearest point where object starts to fade in == mins[0]
	//		Nearest point where object is opaque == mins[1]
	//		Farthest point where object is opaque == maxs[1]
	//		Farthest point where object fades out completely = maxs[0]
	//
	// If the artist says they want it to start off opaque, fade out, then
	// fade back in once it's far away, we'll explain in person why that's stupid,
	// and in the meantime prevent them from doing that by assuming they just
	// have the parameter order flipped.
	//
	// So, they've either given us 2 distances or 2 pairs of distances.
	// If they've just given 2 distances, we use as is,
	// but if they've given 2 pairs, we have to arrange them to fit the
	// above model.
	//
	// So first thing to do is figure out how many (valid) distances we have.
	if( (mins[0] == 0) && (mins[1] == 0) && (maxs[0] == 0) && (maxs[1] == 0) )
	{
		// Okay, they gave no valid distances. Just pretend we were never here.
		return true;
	}
	if( (mins[0] == 0) && (mins[1] == 0) )
	{
		// maxs must be valid, just go with them.
		fade = IFadeFromPoint(maxs);
	}
	else if( (maxs[0] == 0) && (maxs[1] == 0) )
	{
		// mins must be valid, just go with them.
		fade = IFadeFromPoint(mins);
	}
	else
	{
		// They're both "valid". Give it a shot.
		fade = IFadeFromPair(mins, maxs);
	}

	node->SetFade(fade);

	return true;
}

void plDistFadeComponent::ISwap(float& p0, float& p1)
{
	float t = p0;
	p0 = p1;
	p1 = t;
}

Box3 plDistFadeComponent::IFadeFromPoint(Point3& mins)
{
	Point3 maxs(0.f, 0.f, 0.f);
	if( mins[0] < mins[1] )
		mins[2] = -1.f;
	else if( mins[0] > mins[1] )
		mins[2] = 1.f;
	else
		mins[2] = 0;

	return Box3(mins, maxs);
}

Box3 plDistFadeComponent::IFadeFromPair(Point3& mins, Point3& maxs)
{
	if( mins[0] > maxs[0] )
	{
		ISwap(mins[0], maxs[0]);
		ISwap(mins[1], maxs[1]);
	}
	if( mins[0] > mins[1] )
	{
		ISwap(mins[0], mins[1]);
	}
	if( maxs[0] < maxs[1] )
	{
		// Poor confused bastard, take a guess what he wants.
		ISwap(maxs[0], maxs[1]);
	}
	if( mins[0] < mins[1] )
		mins[2] = -1.f;
	else
		mins[2] = 0;
	if( maxs[0] > maxs[1] )
		maxs[2] = 1.f;
	else
		maxs[2] = 0;
	return Box3(mins, maxs);
}


hsBool plDistFadeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plDistFadeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plDistFadeComponent::plDistFadeComponent()
{
	fClassDesc = &gDistFadeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOS Fade component next.

class plLOSFadeComponent : public plComponent
{
public:
	enum
	{
		kBoundsCenter,
		kFadeInTime,
		kFadeOutTime
	};

public:
	plLOSFadeComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plLOSFadeComponent, gLOSFadeCompDesc, "LOS Fade",  "LOSFade", COMP_TYPE_GRAPHICS, LOSFADE_COMP_CID)

ParamBlockDesc2 gLOSFadeBk
(	
	plComponent::kBlkComp, _T("LOSFade"), 0, &gLOSFadeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_LOSFADE, IDS_COMP_LOSFADE, 0, 0, NULL,

	plLOSFadeComponent::kBoundsCenter,	_T("BoundsCenter"),	TYPE_BOOL, 0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_BOUNDSCENTER,
		end,

	plLOSFadeComponent::kFadeInTime, _T("kFadeInTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.5,
		p_range, 0.0, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FADEINTIME, IDC_COMP_FADEINTIME_SPIN, 1.0,
		end,	
	

	plLOSFadeComponent::kFadeOutTime, _T("kFadeOutTime"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.0, 5.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_FADEOUTTIME, IDC_COMP_FADEOUTTIME_SPIN, 1.0,
		end,	
	

	end
);

hsBool plLOSFadeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceMatShade(true);
	node->SetForceLocal(true);
	node->SetForceMaterialCopy(true);
	return true;
}

hsBool plLOSFadeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plLOSFadeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	plFadeOpacityMod* fade = TRACKED_NEW plFadeOpacityMod;
	
	if( fCompPB->GetInt(kBoundsCenter) )
		fade->SetFlag(plFadeOpacityMod::kBoundsCenter);
	else
		fade->ClearFlag(plFadeOpacityMod::kBoundsCenter);

	fade->SetFadeUp(fCompPB->GetFloat(kFadeInTime));
	fade->SetFadeDown(fCompPB->GetFloat(kFadeOutTime));

	node->AddModifier(fade, node->GetKey()->GetName());

	return true; 
}

plLOSFadeComponent::plLOSFadeComponent()
{
	fClassDesc = &gLOSFadeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// GZ Marker Fade component next.

const Class_ID GZFADE_COMP_CID(0x27173270, 0x4f4486f);

class plGZFadeComponent : public plComponent
{
public:
	enum
	{
		kOpaque,
		kTransp
	};

public:
	plGZFadeComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plGZFadeComponent, gGZFadeCompDesc, "GZ Fade",  "GZFade", COMP_TYPE_GRAPHICS, GZFADE_COMP_CID)

ParamBlockDesc2 gGZFadeBk
(	
	plComponent::kBlkComp, _T("GZFade"), 0, &gGZFadeCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_GZFADE, IDS_COMP_GZ_FADE, 0, 0, NULL,

	plGZFadeComponent::kOpaque, _T("kOpaque"), TYPE_FLOAT, 	0, 0,	
		p_default, 15.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_GZ_OPAQUE, IDC_COMP_GZ_OPAQUE_SPIN, 1.0,
		end,	
	
	plGZFadeComponent::kTransp, _T("kTransp"), TYPE_FLOAT, 	0, 0,	
		p_default, 20.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_GZ_TRANSP, IDC_COMP_GZ_TRANSP_SPIN, 1.0,
		end,	
	

	end
);

hsBool plGZFadeComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	float opaq = fCompPB->GetFloat(kOpaque);
	float transp = fCompPB->GetFloat(kTransp);

	pErrMsg->Set(transp <= opaq, node->GetName(), "Distance obj goes transparent must be greater than distance it's opaque").CheckAndAsk();
	pErrMsg->Set(false);

	node->SetForceMatShade(true);
	node->SetForceLocal(true);
	node->SetForceMaterialCopy(true);
	return true;
}

hsBool plGZFadeComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plGZFadeComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	plDistOpacityMod* fade = TRACKED_NEW plDistOpacityMod;

	float opaq = fCompPB->GetFloat(kOpaque);
	float transp = fCompPB->GetFloat(kTransp);

	fade->SetFarDist(opaq, transp);

	node->AddModifier(fade, node->GetKey()->GetName());

	return true; 
}

plGZFadeComponent::plGZFadeComponent()
{
	fClassDesc = &gGZFadeCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Force dynamic material - keeps the material colors from getting burnt into the verts.

const Class_ID DYNMAT_COMP_CID(0x2ea4671f, 0x163b12ac);

class plDynMatComponent : public plComponent
{
public:
	enum
	{
		kOpaque,
		kTransp
	};

public:
	plDynMatComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


CLASS_DESC(plDynMatComponent, gDynMatCompDesc, "Force Dyn Mat",  "DynMat", COMP_TYPE_GRAPHICS, DYNMAT_COMP_CID)

ParamBlockDesc2 gDynMatBk
(	
	plComponent::kBlkComp, _T("DynMat"), 0, &gDynMatCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DYNMAT, IDS_COMP_DYNMAT, 0, 0, NULL,

	end
);

hsBool plDynMatComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceMatShade(true);
	return true;
}

hsBool plDynMatComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plDynMatComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
	return true; 
}

plDynMatComponent::plDynMatComponent()
{
	fClassDesc = &gDynMatCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}


