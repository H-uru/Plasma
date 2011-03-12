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

#include "hsTypes.h"

#include "../plDrawable/plDrawableSpans.h"

// The AccMeshSmooth now does everything the InterMeshSmooth and AvMeshSmooth
// components did, only better and with fewer bugs.
//#include "../plDrawable/plInterMeshSmooth.h"
#include "../plDrawable/plAvMeshSmooth.h"

#include "../plDrawable/plAccMeshSmooth.h"
#include "../plDrawable/plGeometrySpan.h"
#include "../plDrawable/plSharedMesh.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"


const Class_ID CID_SMOOTHCOMP(0x7f926cbc, 0x58df5a44);
const Class_ID CID_SMOOTHAV(0xaf37a4f, 0x8c00991);
const Class_ID CID_SMOOTHBASE(0xebd3ccd, 0x8e85ea6);
const Class_ID CID_SMOOTHSNAP(0x7768074c, 0x65197b77);


void DummyCodeIncludeFuncSmooth()
{

}


//Class that accesses the paramblock below.
class plSmoothComponent : public plComponent
{
public:
	enum {
		kSmoothAngle,
		kSmoothPos,
		kSmoothColor
	};
protected:
	hsBool				fDoneThis;
	hsBool				IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans);
	hsBool				IGetSpans(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans);
	hsBool				IReShade(plErrorMsg* pErrMsg);
	hsBool				ISmoothAll(plErrorMsg* pErrMsg);
public:
	plSmoothComponent();
	void DeleteThis() { delete this; }


	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	{ fDoneThis = false; return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ fDoneThis = false; return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plSmoothComponent, gSmoothDesc, "Smooth",  "Smooth", COMP_TYPE_GRAPHICS, CID_SMOOTHCOMP)


ParamBlockDesc2 gSmoothBk
(	
 plComponent::kBlkComp, _T("Smooth"), 0, &gSmoothDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SMOOTH, IDS_COMP_SMOOTHS, 0, 0, NULL,

	plSmoothComponent::kSmoothAngle, _T("SmoothAngle"),		TYPE_FLOAT, 0, 0,
		p_default, 75.0f,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTH_ANGLE, IDC_COMP_SMOOTH_ANGLE_SPIN, 1.f,
		end,

	plSmoothComponent::kSmoothPos,  _T("SmoothPos"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SMOOTH_POS,
		end,

	plSmoothComponent::kSmoothColor,  _T("SmoothColor"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SMOOTH_COLOR,
		end,

	end
);

plSmoothComponent::plSmoothComponent()
{
	fClassDesc = &gSmoothDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSmoothComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	ISmoothAll(pErrMsg);

	return true;
}

hsBool plSmoothComponent::ISmoothAll(plErrorMsg* pErrMsg)
{
	if( !fDoneThis )
	{
		hsTArray<plGeometrySpan*> spans;

		IGetSpans(pErrMsg, spans);

		IDoSmooth(pErrMsg, spans);

		fDoneThis = true;
	}
	return true;
}

hsBool plSmoothComponent::IGetSpans(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans)
{
	spans.SetCount(0);

	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( !(node && node->CanConvert() && node->GetDrawable()) )
			continue;

		plSceneObject* obj = node->GetSceneObject();
		if( !obj )
			continue;

		const plDrawInterface* di = obj->GetDrawInterface();
		if( !di )
			continue;

		UInt8 iDraw;
		for( iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++ )
		{
			plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
			if( !dr )
				continue;

			plDISpanIndex disi = dr->GetDISpans(di->GetDrawableMeshIndex(iDraw));

			int i;
			for( i = 0; i < disi.fIndices.GetCount(); i++ )
			{
				spans.Append(dr->GetSourceSpans()[disi.fIndices[i]]);
			}
		}
	}

	return true;
}

hsBool plSmoothComponent::IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans)
{
//	if( spans.GetCount() > 1 )
	{
		plAccMeshSmooth smoother;
		smoother.SetAngle(fCompPB->GetFloat(kSmoothAngle));
		smoother.SetFlags(plAccMeshSmooth::kSmoothNorm);
		if( fCompPB->GetInt(kSmoothPos) )
			smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothPos);
		if( fCompPB->GetInt(kSmoothColor) )
			smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothDiffuse);
		smoother.Smooth(spans);
	}

	return true;
}

hsBool plSmoothComponent::IReShade(plErrorMsg* pErrMsg)
{
	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( node )
			node->ShadeMesh(pErrMsg, nil);
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// Now an avatar specific version
///////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plSmoothAvComponent : public plComponent
{
public:
	enum {
		kSmoothAngle,
		kDistTol,
		kSmoothPos
	};
protected:
	hsBool				fDoneThis;
	hsBool				IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans);
	hsBool				IGetSpans(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans);
	hsBool				IReShade(plErrorMsg* pErrMsg);
	hsBool				ISmoothAll(plErrorMsg* pErrMsg);
public:
	plSmoothAvComponent();
	void DeleteThis() { delete this; }


	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	{ fDoneThis = false; return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ fDoneThis = false; return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plSmoothAvComponent, gSmoothAvDesc, "Avatar Smooth",  "AvSmooth", COMP_TYPE_GRAPHICS, CID_SMOOTHAV)


ParamBlockDesc2 gSmoothAvBk
(	
 plComponent::kBlkComp, _T("SmoothAv"), 0, &gSmoothAvDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SMOOTHAV, IDS_COMP_SMOOTHAV, 0, 0, NULL,

	plSmoothAvComponent::kSmoothAngle, _T("SmoothAngle"),		TYPE_FLOAT, 0, 0,
		p_default, 75.0f,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHAV_ANGLE, IDC_COMP_SMOOTHAV_ANGLE_SPIN, 1.f,
		end,

	plSmoothAvComponent::kDistTol, _T("DistTol"),		TYPE_FLOAT, 0, 0,
		p_default, 0.001f,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHAV_DIST, IDC_COMP_SMOOTHAV_DIST_SPIN, 0.01f,
		end,

	plSmoothAvComponent::kSmoothPos,  _T("SmoothPos"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SMOOTHAV_POS,
		end,

	end
);

plSmoothAvComponent::plSmoothAvComponent()
{
	fClassDesc = &gSmoothAvDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSmoothAvComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	ISmoothAll(pErrMsg);

	return true;
}

hsBool plSmoothAvComponent::ISmoothAll(plErrorMsg* pErrMsg)
{
	if( !fDoneThis )
	{
		hsTArray<plGeometrySpan*> spans;

		IGetSpans(pErrMsg, spans);

		IDoSmooth(pErrMsg, spans);

		fDoneThis = true;
	}
	return true;
}

hsBool plSmoothAvComponent::IGetSpans(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans)
{
	spans.SetCount(0);

	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( !node )
			continue;

		plSharedMesh* sharedMesh = node->GetSwappableGeom();
		if( !sharedMesh )
			continue;

		int j;
		for( j = 0; j < sharedMesh->fSpans.GetCount(); j++ )
		{
			if( sharedMesh->fSpans[j] )
				spans.Append(sharedMesh->fSpans[j]);
		}
	}

	return true;
}

hsBool plSmoothAvComponent::IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plGeometrySpan*>& spans)
{
	if( spans.GetCount() > 1 )
	{
		plAccMeshSmooth smoother;
		smoother.SetAngle(fCompPB->GetFloat(kSmoothAngle));
		smoother.SetDistTol(fCompPB->GetFloat(kDistTol));
		smoother.SetFlags(plAccMeshSmooth::kSmoothNorm);
		if( fCompPB->GetInt(kSmoothPos) )
			smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothPos);
		smoother.Smooth(spans);
	}

	return true;
}

hsBool plSmoothAvComponent::IReShade(plErrorMsg* pErrMsg)
{
	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( node )
			node->ShadeMesh(pErrMsg, nil);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Tiny component to reference a bunch of meshes. These meshes will be used for reference,
// but not actually exported (unless someone else grabs them too.)
///////////////////////////////////////////////////////////////////////////////////////////////////

class plSmoothBaseComponent : public plComponent
{
protected:
	hsTArray<plAvMeshSmooth::XfmSpan>	fSpans;

public:
	enum {
		kSmoothAngle,
		kDistTol,
		kSmoothPos
	};

	plSmoothBaseComponent();
	void DeleteThis() { delete this; }

	hsTArray<plAvMeshSmooth::XfmSpan>& GetSpans(plErrorMsg* pErrMsg);

	float GetSmoothAngle() const { return fCompPB->GetFloat(kSmoothAngle); }
	float GetDistTol() const { return fCompPB->GetFloat(kDistTol); }
	bool SmoothPosition() const { return 0 != fCompPB->GetInt(kSmoothPos); }

	static plSmoothBaseComponent* GetSmoothBaseComp(INode* node);
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plSmoothBaseComponent, gSmoothBaseDesc, "Smoothing Base",  "SmoothBase", COMP_TYPE_GRAPHICS, CID_SMOOTHBASE)

ParamBlockDesc2 gSmoothBaseBk
(	
	plComponent::kBlkComp, _T("SmoothBase"), 0, &gSmoothBaseDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SMOOTHBASE, IDS_COMP_SMOOTHBASE, 0, 0, NULL,

	plSmoothBaseComponent::kSmoothAngle, _T("SmoothAngle"),		TYPE_FLOAT, 0, 0,
		p_default, 75.0f,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHBASE_ANGLE, IDC_COMP_SMOOTHBASE_ANGLE_SPIN, 1.f,
		end,

	plSmoothBaseComponent::kDistTol, _T("DistTol"),		TYPE_FLOAT, 0, 0,
		p_default, 0.001f,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHBASE_DIST, IDC_COMP_SMOOTHBASE_DIST_SPIN, 0.01f,
		end,

	plSmoothBaseComponent::kSmoothPos,  _T("SmoothPos"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SMOOTHBASE_POS,
		end,

	end
);

plSmoothBaseComponent::plSmoothBaseComponent()
{
	fClassDesc = &gSmoothBaseDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSmoothBaseComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	node->SetDrawable(false);
	if (!node->GetSwappableGeom())
		node->SetSwappableGeom(new plSharedMesh);

	fSpans.SetCount(0);

	return true;
}
	
hsBool plSmoothBaseComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	GetSpans(pErrMsg);
	return true;
}

plSmoothBaseComponent* plSmoothBaseComponent::GetSmoothBaseComp(INode* node)
{
	if( !node )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( comp == nil )
		return nil;

	if( comp->ClassID() == CID_SMOOTHBASE )
		return (plSmoothBaseComponent*)comp;

	return nil;
}


// Only valid after the MakeMesh phase.
hsTArray<plAvMeshSmooth::XfmSpan>& plSmoothBaseComponent::GetSpans(plErrorMsg* pErrMsg)
{
	if( !fSpans.GetCount() )
	{
		UInt32 count = NumTargets();
		UInt32 i;
		hsTArray<plGeometrySpan*> spans;

		for( i = 0; i < count; i++ )
		{
			plMaxNode *node = (plMaxNode*)GetTarget(i);
			if( !node )
				continue;

			plSharedMesh* sharedMesh = node->GetSwappableGeom();
			if( !sharedMesh )
				continue;

			int j;
			for( j = 0; j < sharedMesh->fSpans.GetCount(); j++ )
			{
				if( sharedMesh->fSpans[j] )
				{
					spans.Append(sharedMesh->fSpans[j]);
					plAvMeshSmooth::XfmSpan xfmSpan;
					xfmSpan.fSpan = sharedMesh->fSpans[j];

					xfmSpan.fSpanToNeutral = node->GetOTM44() * node->GetLocalToVert44();
					xfmSpan.fSpanToNeutral.GetInverse(&xfmSpan.fNeutralToSpan);

					xfmSpan.fSpanToNeutral.GetTranspose(&xfmSpan.fNormNeutralToSpan);
					xfmSpan.fNeutralToSpan.GetTranspose(&xfmSpan.fNormSpanToNeutral);

					fSpans.Append(xfmSpan);
				}
			}
		}
		if( spans.GetCount() > 1 )
		{
			plAccMeshSmooth smoother;
			smoother.SetAngle(fCompPB->GetFloat(kSmoothAngle));
			smoother.SetDistTol(fCompPB->GetFloat(kDistTol));
			smoother.SetFlags(plAccMeshSmooth::kSmoothNorm);
			if( fCompPB->GetInt(kSmoothPos) )
				smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothPos);
			smoother.Smooth(spans);
		}
	}

	return fSpans;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// Now another avatar specific version
///////////////////////////////////////////////////////////////////////////////////////////////

//Class that accesses the paramblock below.
class plSmoothSnapComponent : public plComponent
{
public:
	enum {
		kSmoothBase,
		kSmoothAngle,
		kDistTol,
		kSmoothPos
	};
protected:
	hsBool				fDoneThis;

	hsTArray<plAvMeshSmooth::XfmSpan>& IGetSrcSpans(plErrorMsg* pErrMsg);

	hsBool				IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plAvMeshSmooth::XfmSpan>& srcSpans, hsTArray<plAvMeshSmooth::XfmSpan>& dstSpans);
	hsBool				IGetDstSpans(plErrorMsg* pErrMsg, hsTArray<plAvMeshSmooth::XfmSpan>& spans);
	hsBool				IReShade(plErrorMsg* pErrMsg);
	hsBool				ISmoothAll(plErrorMsg* pErrMsg);
public:
	plSmoothSnapComponent();
	void DeleteThis() { delete this; }


	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	{ fDoneThis = false; return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ fDoneThis = false; return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


class plSmoothBaseSelProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
};

#include "plPickNode.h"

BOOL plSmoothBaseSelProc::DlgProc(TimeValue t, IParamMap2 *paramMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			IParamBlock2 *pb = paramMap->GetParamBlock();
			INode* node = pb->GetINode(plSmoothSnapComponent::kSmoothBase);
			TSTR newName(node ? node->GetName() : "Pick");
			::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_SMOOTH_CHOSE), newName);
		}
		return true;

	case WM_COMMAND:
		if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_COMP_SMOOTH_CHOSE) )
		{
			IParamBlock2 *pb = paramMap->GetParamBlock();
			std::vector<Class_ID> cids;
			cids.push_back(CID_SMOOTHBASE);
			if( plPick::Node(pb, plSmoothSnapComponent::kSmoothBase, &cids, true, true) )
			{
				INode* node = pb->GetINode(plSmoothSnapComponent::kSmoothBase);
				TSTR newName(node ? node->GetName() : "Pick");
				::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_SMOOTH_CHOSE), newName);
				paramMap->Invalidate(plSmoothSnapComponent::kSmoothBase);
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_SHOW);
			}

			return false;
		}
		return true;
	}

	return false;
}

plSmoothBaseSelProc gSmoothBaseSelProc;

//Max desc stuff necessary below.
CLASS_DESC(plSmoothSnapComponent, gSmoothSnapDesc, "Snap to Base",  "SnapTo", COMP_TYPE_GRAPHICS, CID_SMOOTHSNAP)


ParamBlockDesc2 gSmoothSnapBk
(	
 plComponent::kBlkComp, _T("SmoothSnap"), 0, &gSmoothSnapDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SMOOTHSNAP, IDS_COMP_SMOOTHSNAP, 0, 0, &gSmoothBaseSelProc,

	plSmoothSnapComponent::kSmoothBase, _T("SmoothBase"),	TYPE_INODE,		0, 0,
		end,

	plSmoothSnapComponent::kSmoothAngle, _T("SmoothAngle"),		TYPE_FLOAT, 0, 0,
		p_default, 75.0f,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHSNAP_ANGLE, IDC_COMP_SMOOTHSNAP_ANGLE_SPIN, 1.f,
		end,

	plSmoothSnapComponent::kDistTol, _T("DistTol"),		TYPE_FLOAT, 0, 0,
		p_default, 0.01f,
		p_range, 0.0, 1.0,
		p_ui,	TYPE_SPINNER, EDITTYPE_FLOAT, 
		IDC_COMP_SMOOTHSNAP_DIST, IDC_COMP_SMOOTHSNAP_DIST_SPIN, 0.01f,
		end,

	plSmoothSnapComponent::kSmoothPos,  _T("SmoothPos"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_SMOOTHSNAP_POS,
		end,

	end
);

plSmoothSnapComponent::plSmoothSnapComponent()
{
	fClassDesc = &gSmoothSnapDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plSmoothSnapComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	ISmoothAll(pErrMsg);

	return true;
}

hsBool plSmoothSnapComponent::ISmoothAll(plErrorMsg* pErrMsg)
{
	if( !fDoneThis )
	{
		fDoneThis = true;

		hsTArray<plAvMeshSmooth::XfmSpan>& srcSpans = IGetSrcSpans(pErrMsg);

		if( !srcSpans.GetCount() )
			return true;

		hsTArray<plAvMeshSmooth::XfmSpan> dstSpans;

		if( !IGetDstSpans(pErrMsg, dstSpans) )
			return true;

		IDoSmooth(pErrMsg, srcSpans, dstSpans);

	}
	return true;
}

hsTArray<plAvMeshSmooth::XfmSpan>& plSmoothSnapComponent::IGetSrcSpans(plErrorMsg* pErrMsg)
{
	static hsTArray<plAvMeshSmooth::XfmSpan> emptySpans;

	plSmoothBaseComponent* baseComp = plSmoothBaseComponent::GetSmoothBaseComp(fCompPB->GetINode(kSmoothBase, 0, 0));
	if( !baseComp )
		return emptySpans;

	return baseComp->GetSpans(pErrMsg);
}

hsBool plSmoothSnapComponent::IGetDstSpans(plErrorMsg* pErrMsg, hsTArray<plAvMeshSmooth::XfmSpan>& spans)
{
	hsTArray<plGeometrySpan*> geoSpans;
	spans.SetCount(0);

	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( !node )
			continue;

		plSharedMesh* sharedMesh = node->GetSwappableGeom();
		if( !sharedMesh )
			continue;

		int j;
		for( j = 0; j < sharedMesh->fSpans.GetCount(); j++ )
		{
			if( sharedMesh->fSpans[j] )
			{
				geoSpans.Append(sharedMesh->fSpans[j]);

				plAvMeshSmooth::XfmSpan xfmSpan;
				xfmSpan.fSpan = sharedMesh->fSpans[j];

				xfmSpan.fSpanToNeutral = node->GetOTM44() * node->GetLocalToVert44();
				xfmSpan.fSpanToNeutral.GetInverse(&xfmSpan.fNeutralToSpan);

				xfmSpan.fSpanToNeutral.GetTranspose(&xfmSpan.fNormNeutralToSpan);
				xfmSpan.fNeutralToSpan.GetTranspose(&xfmSpan.fNormSpanToNeutral);

				spans.Append(xfmSpan);
			}
		}
	}

	// Smooth them with themselves before we pass them off to be snapped to the base.
	// We'll use the base component's parameters, because ours will be sloppier to
	// ensure proper snapping.
	if( geoSpans.GetCount() )
	{
		plSmoothBaseComponent* baseComp = plSmoothBaseComponent::GetSmoothBaseComp(fCompPB->GetINode(kSmoothBase, 0, 0));
		if( !baseComp )
			return 0;

		plAccMeshSmooth smoother;
		smoother.SetAngle(baseComp->GetSmoothAngle());
		smoother.SetDistTol(baseComp->GetDistTol());
		smoother.SetFlags(plAccMeshSmooth::kSmoothNorm);
		if( baseComp->SmoothPosition() )
			smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothPos);
		smoother.Smooth(geoSpans);
	}

	return spans.GetCount();
}

hsBool plSmoothSnapComponent::IDoSmooth(plErrorMsg* pErrMsg, hsTArray<plAvMeshSmooth::XfmSpan>& srcSpans, hsTArray<plAvMeshSmooth::XfmSpan>& dstSpans)
{
	if( srcSpans.GetCount() && dstSpans.GetCount() )
	{
		plAvMeshSmooth smoother;
		smoother.SetAngle(fCompPB->GetFloat(kSmoothAngle));
		smoother.SetDistTol(fCompPB->GetFloat(kDistTol));
		smoother.SetFlags(plAccMeshSmooth::kSmoothNorm);
		if( fCompPB->GetInt(kSmoothPos) )
			smoother.SetFlags(smoother.GetFlags() | plAccMeshSmooth::kSmoothPos);
		smoother.Smooth(srcSpans, dstSpans);
	}

	return true;
}

hsBool plSmoothSnapComponent::IReShade(plErrorMsg* pErrMsg)
{
	UInt32 count = NumTargets();
	UInt32 i;
	for( i = 0; i < count; i++ )
	{
		plMaxNode *node = (plMaxNode*)GetTarget(i);
		if( node )
			node->ShadeMesh(pErrMsg, nil);
	}
	return true;
}

