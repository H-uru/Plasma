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
#include "hsTypes.h"
#include "plCompositeMtl.h"
#include "plPassMtl.h"
//#include "plCompositeMtlPB.h"
#include "plCompositeMtlDlg.h"

class plCompositeClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading) { return TRACKED_NEW plCompositeMtl(loading); }
	const TCHAR*	ClassName()		{ return GetString(IDS_COMP_MTL); }
	SClass_ID		SuperClassID()	{ return MATERIAL_CLASS_ID; }
	Class_ID		ClassID()		{ return COMP_MTL_CLASS_ID; }
	const TCHAR* 	Category()		{ return NULL; }
	const TCHAR*	InternalName()	{ return _T("PlasmaComposite"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plCompositeClassDesc plCompositeMtlDesc;
ClassDesc2* GetCompMtlDesc() { return &plCompositeMtlDesc; }

#include "plCompositeMtlPBDec.h"

const char *plCompositeMtl::BlendStrings[] = // Make sure these match up in order with the Blend enum (in the header)
{
	"Vertex Alpha",
	"Inverse Vtx Alpha",
	"Vertex Illum Red",
	"Inv. Vtx Illum Red",
	"Vertex Illum Green",
	"Inv. Vtx Illum Green",
	"Vertex Illum Blue",
	"Inv. Vtx Illum Blue"
};

plCompositeMtl::plCompositeMtl(BOOL loading) : fPassesPB(NULL)
{
	plCompositeMtlDesc.MakeAutoParamBlocks(this);

	if (!loading) 
		Reset();

	int i;
	for (i = 0; i < NSUBMTLS; i++)
	{
		plPassMtl *newMtl = TRACKED_NEW plPassMtl(false);
		fPassesPB->SetValue(kCompPasses, 0, newMtl, i);
		GetCOREInterface()->AssignNewName(fPassesPB->GetMtl(kCompPasses, 0, i));
	}
}

void plCompositeMtl::Reset() 
{
	fIValid.SetEmpty();
}

ParamDlg* plCompositeMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fMtlDlg = TRACKED_NEW plCompositeMtlDlg(hwMtlEdit, imp, this);

	return fMtlDlg;	
}

void plCompositeMtl::SetParamDlg(ParamDlg *dlg)
{
	fMtlDlg = (plCompositeMtlDlg*)dlg;
}

BOOL plCompositeMtl::SetDlgThing(ParamDlg* dlg)
{
	if (dlg == fMtlDlg)
	{
		fMtlDlg->SetThing(this);
		return TRUE;
	}

	return FALSE;
}

Interval plCompositeMtl::Validity(TimeValue t)
{
	Interval valid = FOREVER;		

/*	for (int i = 0; i < fSubTexmap.Count(); i++) 
	{
		if (fSubTexmap[i]) 
			valid &= fSubTexmap[i]->Validity(t);
	}
*/	
//	float u;
//	fPBlock->GetValue(pb_spin,t,u,valid);
	return valid;
}

// The index for a face on a composite material is really just a bitmask for all the submaterials. This function only
// computes it... The mesh creator will ask it to be created if it's needed.
int plCompositeMtl::ComputeMaterialIndex(float opac[][2], int vertCount)
{
	int index = 0;
	int i;//, j;
	int bitmask = 1;
	for (i = NumSubMtls() - 1, bitmask <<= i; i >= 0; i--, bitmask >>= 1)
	{
		index |= bitmask;
		/*
		if (i == 0)
			index |= bitmask; // it's not opaqued out, so include the base layer
		else if (fPassesPB->GetInt(kCompOn, 0, i - 1)) // is the checkbox ticked? (ie, are we using it?)
		{
			bool transparent = true;
			for (j = 0; j < vertCount; j++)
			{
				if (opac[j][i - 1] != 0.0)
					transparent = false;
			}
			if (transparent)
				continue; // totally transparent for this face, skip it

			index |= bitmask; // include this material

			bool opaque = true;
			for (j = 0; j < vertCount; j++)
			{
				if (opac[j][i - 1] < 1.0)
					opaque = false;
			}
			if (opaque && !((plPassMtlBase *)fPassesPB->GetMtl(kCompPasses, 0, i))->HasAlpha())
				break; // This material is totally opaque, no sense including anything underneath it
		}
		*/
	}
	return index;
}

int plCompositeMtl::GetBlendStyle(int index)
{
	return fPassesPB->GetInt(kCompBlend, 0, index - 1);
}

// Determines whether all materials are only blending on one channel (and possibly its inverse) and therefore
// it's ok to overwrite the vertex alpha exported for the span using this material.
// Returns: -1 if we use multiple sources, otherwise the source we all agree on
int plCompositeMtl::CanWriteAlpha()
{
	int blend[3];
	blend[0] = ((((plPassMtlBase *)GetSubMtl(0))->GetOutputBlend() == plPassMtlBase::kBlendNone) ? -1 : kCompBlendVertexAlpha);
	blend[1] = (fPassesPB->GetInt(kCompOn, 0, 0) ? RemoveInverse(GetBlendStyle(1)) : -1);
	blend[2] = (fPassesPB->GetInt(kCompOn, 0, 1) ? RemoveInverse(GetBlendStyle(2)) : -1);

	int source = blend[0];
	int i;
	for (i = 1; i < 3; i++)
	{
		if (source < 0) 
		{
			source = blend[i];
			continue;
		}
		if (source >= 0 && blend[i] >= 0 && blend[i] != source) 
			return -1;
	}
	return source; 
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

int plCompositeMtl::NumSubs()
{
	return NumSubMtls();
}

TSTR plCompositeMtl::SubAnimName(int i) 
{
	return GetSubMtlSlotName(i);
}

Animatable* plCompositeMtl::SubAnim(int i)
{
	return GetSubMtl(i);
}

int plCompositeMtl::NumRefs()
{
	return 1;
}

RefTargetHandle plCompositeMtl::GetReference(int i)
{
	if (i == kRefPasses)
		return fPassesPB;

	return NULL;
}

void plCompositeMtl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == kRefPasses)
		fPassesPB = (IParamBlock2 *)rtarg;
}

int	plCompositeMtl::NumParamBlocks()
{
	return 1;
}

IParamBlock2 *plCompositeMtl::GetParamBlock(int i)
{
	if (i == kRefPasses)
		return fPassesPB;

	return NULL;
}

IParamBlock2 *plCompositeMtl::GetParamBlockByID(BlockID id)
{
	if (fPassesPB->ID() == id)
		return fPassesPB;

	return NULL;
}

RefResult plCompositeMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	switch (message)
	{
	case REFMSG_CHANGE:
		fIValid.SetEmpty();
		if (hTarget == fPassesPB)
		{
			ParamID changingParam = fPassesPB->LastNotifyParamID();
			fPassesPB->GetDesc()->InvalidateUI(changingParam);
		}
		break;
	}

	return REF_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plCompositeMtl::NumSubMtls()
{
//	return fPassesPB->GetInt(kMultCount);
	return NSUBMTLS;
}

Mtl *plCompositeMtl::GetSubMtl(int i)
{
	if (i < NumSubMtls())
		return fPassesPB->GetMtl(kCompPasses, 0, i);

	return NULL;
}

void plCompositeMtl::SetSubMtl(int i, Mtl *m)
{
	if (i < NumSubMtls())
		fPassesPB->SetValue(kCompPasses, 0, m, i);
}

TSTR plCompositeMtl::GetSubMtlSlotName(int i)
{
	TSTR str;
	str.printf("Pass %d", i+1);
	return str;
}

TSTR plCompositeMtl::GetSubMtlTVName(int i)
{
	return GetSubMtlSlotName(i);
}


/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult plCompositeMtl::Save(ISave *isave)
{ 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plCompositeMtl::Load(ILoad *iload)
{
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(id = iload->CurChunkID())
		{
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}


/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle plCompositeMtl::Clone(RemapDir &remap)
{
	plCompositeMtl *mnew = TRACKED_NEW plCompositeMtl(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	mnew->ReplaceReference(kRefPasses, remap.CloneRef(fPassesPB));

	mnew->fIValid.SetEmpty();	
	BaseClone(this, mnew, remap);

	return (RefTargetHandle)mnew;
}

void plCompositeMtl::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plCompositeMtl::Update(TimeValue t, Interval& valid) 
{	
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();
//		fPassesPB->GetValue(kMtlLayLayer1On, t, fMapOn[0], fIValid);

		for (int i = 0; i < NumSubMtls(); i++)
		{
			if (GetSubMtl(i))
				GetSubMtl(i)->Update(t, fIValid);
		}
	}

	valid &= fIValid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/
void plCompositeMtl::SetAmbient(Color c, TimeValue t) {}		
void plCompositeMtl::SetDiffuse(Color c, TimeValue t) {}		
void plCompositeMtl::SetSpecular(Color c, TimeValue t) {}
void plCompositeMtl::SetShininess(float v, TimeValue t) {}
				
Color plCompositeMtl::GetAmbient(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plCompositeMtl::GetDiffuse(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plCompositeMtl::GetSpecular(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
float plCompositeMtl::GetXParency(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plCompositeMtl::GetShininess(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plCompositeMtl::GetShinStr(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plCompositeMtl::WireSize(int mtlNum, BOOL backFace)		{ return 0.0f; }

/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void plCompositeMtl::Shade(ShadeContext& sc) 
{
	// Get the background color
	Color backColor, backTrans;
	//sc.GetBGColor(backColor, backTrans);
	backColor.Black();
	backTrans.White();

	Point3 vtxIllum, vtxAlpha;
	plPassMtl::GetInterpVtxValue(MAP_ALPHA, sc, vtxAlpha);
	plPassMtl::GetInterpVtxValue(MAP_SHADING, sc, vtxIllum);
	int count = NumSubMtls();
	for (int i = 0; i < count; i++)
	{
		if (i > 0 && fPassesPB->GetInt(kCompOn, 0, i - 1) == 0) // Material is unchecked, skip it.
			continue;

		Mtl *mtl = GetSubMtl(i);
		if (mtl == NULL || mtl->ClassID() != PASS_MTL_CLASS_ID)
			continue;

		float opacity;
		if (i == 0)
		{
			opacity = 1.0f;
		}
		else
		{
			int blendMethod = fPassesPB->GetInt(kCompBlend, 0, i - 1);
			switch(blendMethod)
			{
			case kCompBlendVertexAlpha:
			case kCompBlendInverseVtxAlpha:
				opacity = vtxAlpha.x;
				break;
			case kCompBlendVertexIllumRed:
			case kCompBlendInverseVtxIllumRed:
				opacity = vtxIllum.x;
				break;
			case kCompBlendVertexIllumGreen:
			case kCompBlendInverseVtxIllumGreen:
				opacity = vtxIllum.y;
				break;
			case kCompBlendVertexIllumBlue:
			case kCompBlendInverseVtxIllumBlue:
				opacity = vtxIllum.z;
				break;
			default:
				opacity = 1.0f;
				break;
			}
			if (IsInverseBlend(blendMethod))
				opacity = 1 - opacity;
		}

		plPassMtl *passMtl = (plPassMtl*)mtl;
		passMtl->ShadeWithBackground(sc, backColor, false); // Don't include the vtx alpha, that's OUR job
		float currTrans = (1 - (1 - sc.out.t.r) * opacity);
		backTrans *= currTrans;
		backColor = backColor * currTrans + sc.out.c * opacity; 
	}

	sc.out.t = backTrans;
	sc.out.c = backColor;
}

float plCompositeMtl::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval plCompositeMtl::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();

	return iv;	
}

/*
void plCompositeMtl::SetNumSubMtls(int num)
{
	TimeValue t = GetCOREInterface()->GetTime();
	int curNum = fPassesPB->GetInt(kMultCount);

	fPassesPB->SetValue(kMultCount, 0, num);

	fPassesPB->SetCount(kMultPasses, num);
	fPassesPB->SetCount(kMultOn, num);

	for (int i = curNum; i < num; i++)
	{
		plPassMtl *newMtl = TRACKED_NEW plPassMtl(false);
		fPassesPB->SetValue(kMultPasses, t, newMtl, i);
		fPassesPB->SetValue(kMultOn, t, TRUE, i);
		GetCOREInterface()->AssignNewName(fPassesPB->GetMtl(kMultPasses, t, i));
	}
}
*/


void plCompositeMtl::SetOpacityVal(float *target, UVVert *alpha, UVVert *illum, int method)
{
	if (method == kCompBlendVertexAlpha || method == kCompBlendInverseVtxAlpha)
	{
		if (alpha == NULL)
			*target = 1.0f;
		else
			*target = alpha->x;
	}
	else if (method == kCompBlendVertexIllumRed || method == kCompBlendInverseVtxIllumRed)
	{
		if (illum == NULL)
			*target = 1.0f;
		else
			*target = illum->x;
	}
	else if (method == kCompBlendVertexIllumGreen || method == kCompBlendInverseVtxIllumGreen)
	{
		if (illum == NULL)
			*target = 1.0f;
		else
			*target = illum->y;
	}
	else if (method == kCompBlendVertexIllumBlue || method == kCompBlendInverseVtxIllumBlue)
	{
		if (illum == NULL)
			*target = 1.0f;
		else
			*target = illum->z;
	}
	else
	{
		*target = 1.0f;
	}

	if (method == kCompBlendInverseVtxAlpha ||
		method == kCompBlendInverseVtxIllumRed ||
		method == kCompBlendInverseVtxIllumGreen ||
		method == kCompBlendInverseVtxIllumBlue)
	{
		*target = 1.0f - *target;
	}
}
/*
int plCompositeMtl::UVChannelsNeeded(bool makeAlphaLayer)
{
	if (makeAlphaLayer)
		return 1;

	// Otherwise, we do have vertex alpha... can we get by without taking up a UV channel?
	int channels = 0;

	int i;
	for (i = 0; i < NumSubMtls() - 1; i++)
	{
		if (!fPassesPB->GetInt(kCompOn, 0, i))
			continue; // The material is unchecked, no need to see if it needs a channel

		int method = fPassesPB->GetInt(kCompBlend, 0, i);
		if (!(method == kCompBlendVertexAlpha || method1 == kCompBlendInverseVtxAlpha))
		{
			channels = 1;
			break;
		}
	}

	return channels;
}
*/