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
#include "plAngleAttenLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

class plAngleAttenLayerClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plAngleAttenLayer(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_ANGLE_ATTEN_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return ANGLE_ATTEN_LAYER_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_COLMOD; }
	const TCHAR*	InternalName()	{ return _T("PlasmaAngleAttenLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plAngleAttenLayerClassDesc plAngleAttenLayerDesc;
ClassDesc2* GetAngleAttenLayerDesc() { return &plAngleAttenLayerDesc; }

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const float kDefTransp0 = 60.f;
static const float kDefOpaque0 = 90.f;
static const float kDefTransp1 = 30.f;
static const float kDefOpaque1 = 0.f;

static ParamBlockDesc2 gAngleAttenParamBlk
(
	plAngleAttenLayer::kBlkAngles, _T("angles"),  0, GetAngleAttenLayerDesc(),//NULL,
	P_AUTO_CONSTRUCT + P_AUTO_UI, plAngleAttenLayer::kRefAngles,

	IDD_ANGLE_ATTEN_LAYER, IDS_ANGLE_ATTEN_LAYER_PROPS, 0, 0, nil,

	// Texture size
	plAngleAttenLayer::kTranspAngle0,	_T("transp0"),	TYPE_FLOAT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_TRANSP_ANGLE_0, IDC_TRANSP_ANGLE_0_SPIN, SPIN_AUTOSCALE,
		p_range,		0.0, 180.0,
		p_default,		kDefTransp0,
		end,

	plAngleAttenLayer::kOpaqueAngle0,	_T("opaque0"),	TYPE_FLOAT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_OPAQUE_ANGLE_0, IDC_OPAQUE_ANGLE_0_SPIN, SPIN_AUTOSCALE,
		p_range,		0.0, 180.0,
		p_default,		kDefOpaque0,
		end,

	plAngleAttenLayer::kDoubleFade,	_T("doubleFade"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DOUBLE_FADE,
		p_enable_ctrls,		2, plAngleAttenLayer::kOpaqueAngle1, plAngleAttenLayer::kTranspAngle1,
		p_default,		false,
		end,

	plAngleAttenLayer::kOpaqueAngle1,	_T("opaque1"),	TYPE_FLOAT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_OPAQUE_ANGLE_1, IDC_OPAQUE_ANGLE_1_SPIN, SPIN_AUTOSCALE,
		p_range,		0.0, 180.0,
		p_default,		kDefTransp1,
		end,

	plAngleAttenLayer::kTranspAngle1,	_T("transp1"),	TYPE_FLOAT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_TRANSP_ANGLE_1, IDC_TRANSP_ANGLE_1_SPIN, SPIN_AUTOSCALE,
		p_range,		0.0, 180.0,
		p_default,		kDefOpaque1,
		end,

	plAngleAttenLayer::kReflect,	_T("reflect"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_REFLECT,
		p_default,		false,
		end,

	plAngleAttenLayer::kLoClamp,	_T("loClamp"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_LO_CLAMP, IDC_LO_CLAMP_SPIN, SPIN_AUTOSCALE,
		p_range,		0, 100,
		p_default,		0,
		end,

	plAngleAttenLayer::kHiClamp,	_T("hiClamp"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_HI_CLAMP, IDC_HI_CLAMP_SPIN, SPIN_AUTOSCALE,
		p_range,		0, 100,
		p_default,		100,
		end,

	end
);

plAngleAttenLayer::plAngleAttenLayer() :
	fParmsPB(NULL),
	fIValid(NEVER),
	fCosTransp0(0),
	fCosOpaque0(0),
	fCosTransp1(0),
	fCosOpaque1(0),
	fCosinesCached(false)
{
	plAngleAttenLayerDesc.MakeAutoParamBlocks(this);
}

plAngleAttenLayer::~plAngleAttenLayer()
{
}

//From MtlBase
void plAngleAttenLayer::Reset() 
{
	GetAngleAttenLayerDesc()->Reset(this, TRUE);	// reset all pb2's
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	fIValid.SetEmpty();
}

void plAngleAttenLayer::Update(TimeValue t, Interval& valid) 
{
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();

	}

	valid &= fIValid;
}

Interval plAngleAttenLayer::Validity(TimeValue t)
{
	//TODO: Update fIValid here

	Interval v = FOREVER;
	return v;
}

ParamDlg* plAngleAttenLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = plAngleAttenLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return masterDlg;	
}

BOOL plAngleAttenLayer::SetDlgThing(ParamDlg* dlg)
{	
	return FALSE;
}

int plAngleAttenLayer::NumRefs()
{
	return 1;
}

//From ReferenceMaker
RefTargetHandle plAngleAttenLayer::GetReference(int i) 
{
	switch (i)
	{
		case kRefAngles:		return fParmsPB;
		default:				return NULL;
	}
}

void plAngleAttenLayer::SetReference(int i, RefTargetHandle rtarg) 
{
	Interval	garbage;

	switch (i)
	{
		case kRefAngles:
			fParmsPB = (IParamBlock2 *)rtarg;
			break;
	}
}

int	plAngleAttenLayer::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plAngleAttenLayer::GetParamBlock(int i)
{
	switch (i)
	{
	case 0:	return fParmsPB;
	default: return NULL;
	}
}

IParamBlock2* plAngleAttenLayer::GetParamBlockByID(BlockID id)
{
	if (fParmsPB->ID() == id)
		return fParmsPB;
	else
		return NULL;
}

//From ReferenceTarget 
RefTargetHandle plAngleAttenLayer::Clone(RemapDir &remap) 
{
	plAngleAttenLayer *mnew = TRACKED_NEW plAngleAttenLayer();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(kRefAngles, remap.CloneRef(fParmsPB));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int plAngleAttenLayer::NumSubs()
{
	return 1;
}

Animatable* plAngleAttenLayer::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i)
	{
		case kRefAngles:		return fParmsPB;
		default: return NULL;
	}
}

TSTR plAngleAttenLayer::SubAnimName(int i) 
{
	switch (i)
	{
		case kRefAngles:		return "Angles";
		default: return "";
	}
}

RefResult plAngleAttenLayer::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message) 
{
	switch (message)
	{
	case REFMSG_CHANGE:
		{
			fIValid.SetEmpty();

			if (hTarget == fParmsPB)
			{
				// see if this message came from a changing parameter in the pblock,
				// if so, limit rollout update to the changing item 
				ParamID changingParam = fParmsPB->LastNotifyParamID();
				fParmsPB->GetDesc()->InvalidateUI(changingParam);

				if (changingParam != -1)
					IChanged();
			}
		}
		break;

	}

	return REF_SUCCEED;
}

void plAngleAttenLayer::IChanged()
{
	// Cut and paste insanity from DynamicTextLayer.
	// Texture wasn't getting updated in the viewports, and this fixes it.
	// Don't know if it's the right way though.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	// And this is so the SceneWatcher gets notified that the material on some of it's
	// referenced objects changed.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_MAT);

	ICacheCosines();
}

void plAngleAttenLayer::ICacheCosines()
{
	fCosTransp0 = cosf(DegToRad(fParmsPB->GetFloat(kTranspAngle0)));
	fCosOpaque0 = cosf(DegToRad(fParmsPB->GetFloat(kOpaqueAngle0)));

	if( fParmsPB->GetInt(kDoubleFade) )
	{
		fCosTransp1 = cosf(DegToRad(fParmsPB->GetFloat(kTranspAngle1)));
		fCosOpaque1 = cosf(DegToRad(fParmsPB->GetFloat(kOpaqueAngle1)));
	}
	else
	{
		fCosTransp1 = fCosOpaque1 = 0;
	}
	fCosinesCached = true;
}

#define TEX_HDR_CHUNK 0x5000

IOResult plAngleAttenLayer::Save(ISave *isave) 
{
	IOResult res;

	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plAngleAttenLayer::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk()))
	{
		if (iload->CurChunkID() == TEX_HDR_CHUNK)
		{
			res = MtlBase::Load(iload);
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}

	return IO_OK;
}


AColor plAngleAttenLayer::EvalColor(ShadeContext& sc)
{
	if( !sc.doMaps ) 
		return AColor(0.0f, 0.0f, 0.0f, 1.0f);

	AColor color;
	if (sc.GetCache(this, color)) 
		return color;

	if( !fCosinesCached )
		ICacheCosines();

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	// Evaluate the Bitmap

	Point3 normal = sc.Normal();

	if( fParmsPB->GetInt(kReflect) )
	{
		normal = sc.ReflectVector();
	}
	float dotZ = normal.z;

	float alpha = 1.f;
	if( fCosTransp0 != fCosOpaque0 )
	{
		float a = (dotZ - fCosTransp0) / (fCosOpaque0 - fCosTransp0);
		if( a < 0 )
			a = 0;
		else if( a > 1.f )
			a = 1.f;
		alpha *= a;
	}
	if( fParmsPB->GetInt(kDoubleFade) && (fCosTransp1 != fCosOpaque1) )
	{
		float a = (dotZ - fCosTransp1) / (fCosOpaque1 - fCosTransp1);
		if( a < 0 )
			a = 0;
		else if( a > 1.f )
			a = 1.f;
		if( fCosTransp0 < fCosTransp1 )
		{
			if( fCosTransp0 > fCosOpaque0 )
				alpha += a;
			else
				alpha *= a;
		}
		else
		{
			if( fCosTransp0 < fCosOpaque0 )
				alpha += a;
			else
				alpha *= a;
		}
	}
	color = AColor(1.f, 1.f, 1.f, alpha);

	sc.PutCache(this, color); 
	return color;
}

float plAngleAttenLayer::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 plAngleAttenLayer::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plAngleAttenLayer::LocalRequirements(int subMtlNum)
{
	return MTLREQ_VIEW_DEP | MTLREQ_TRANSP;
}

void plAngleAttenLayer::ActivateTexDisplay(BOOL onoff)
{
}

BITMAPINFO *plAngleAttenLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
	return nil;
						// FIXME
}

DWORD plAngleAttenLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) 
{
	return 0;
}

const char *plAngleAttenLayer::GetTextureName( int which )
{
	return NULL;
}

int plAngleAttenLayer::GetLoClamp()
{
	return fParmsPB->GetInt(kLoClamp);
}

int plAngleAttenLayer::GetHiClamp()
{
	return fParmsPB->GetInt(kHiClamp);
}

Box3 plAngleAttenLayer::GetFade()
{
	Point3 pmin, pmax;

	pmin.x  = fParmsPB->GetFloat(kTranspAngle0);
	pmin.y  = fParmsPB->GetFloat(kOpaqueAngle0);
	if( pmin.x < pmin.y )
		pmin.z = -1.f;
	else if( pmin.x > pmin.y )
		pmin.z = 1.f;
	else
		pmin.z = 0;

	if( fParmsPB->GetInt(kDoubleFade) )
	{
		pmax.x  = fParmsPB->GetFloat(kTranspAngle1);
		pmax.y  = fParmsPB->GetFloat(kOpaqueAngle1);
		if( pmax.x < pmax.y )
			pmax.z = -1.f;
		else if( pmax.x > pmax.y )
			pmax.z = 1.f;
		else
			pmax.z = 0;
	}
	else
	{
		pmax.x = pmax.y = pmax.z = 0;
	}
	return Box3(pmin, pmax);

}

BOOL plAngleAttenLayer::Reflect()
{
	return fParmsPB->GetInt(kReflect);
}