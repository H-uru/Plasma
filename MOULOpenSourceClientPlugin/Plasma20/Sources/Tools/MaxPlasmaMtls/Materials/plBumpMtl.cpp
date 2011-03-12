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
#include "plBumpMtl.h"
#include "resource.h"
//extern ClassDesc2* GetMaxLayerDesc();
#include "Shaders.h"

#include "plBumpMtlBasicPB.h"

#include "iparamm2.h"

#include "Layers/plLayerTex.h"
#include "Layers/plStaticEnvLayer.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

extern HINSTANCE hInstance;

class plBumpMtlClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading) { return TRACKED_NEW plBumpMtl(loading); }
	const TCHAR*	ClassName()		{ return GetString(IDS_BUMP_MTL); }
	SClass_ID		SuperClassID()	{ return MATERIAL_CLASS_ID; }
	Class_ID		ClassID()		{ return BUMP_MTL_CLASS_ID; }
	const TCHAR* 	Category()		{ return NULL; }
	const TCHAR*	InternalName()	{ return _T("PlasmaMaterial"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plBumpMtlClassDesc plBumpMtlDesc;
ClassDesc2* GetBumpMtlDesc() { return &plBumpMtlDesc; }

// For initializing paramblock descriptor
ParamBlockDesc2 *GetBumpBasicPB();
ParamBlockDesc2 *GetBumpLayersPB();

#include "plBumpMtlBasicPBDec.h"
#include "plBumpMtlAnimPBDec.h"

plBumpMtl::plBumpMtl(BOOL loading) : plPassMtlBase( loading )
{
	plBumpMtlDesc.MakeAutoParamBlocks( this );
	fBasicPB->SetValue( kBumpBasLayer, 0, TRACKED_NEW plLayerTex );

	// If we do this later (like, when the dialog loads) something blows up,
	// somewhere in Max.  It didn't in 4, it does in 7.  This seems to fix it.
	if (!loading)
		IVerifyStealthPresent(ENTIRE_ANIMATION_NAME);
}

ParamDlg* plBumpMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plBumpMtlDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return (ParamDlg*)masterDlg;	
}

BOOL plBumpMtl::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval plBumpMtl::Validity(TimeValue t)
{
#if 0 // mf horse
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
#else // mf horse
	const char* name = GetName();

	// mf horse - Hacking in something like real validity checking
	// to get material animations working. No warranty, this is just
	// better than nothing.
	Interval v = FOREVER;
	fBasicPB->GetValidity(t, v);

	if( fBasicPB->GetTexmap(kBumpBasLayer) )
		v &= fBasicPB->GetTexmap(kBumpBasLayer)->Validity(t);
	return v;
#endif // mf horse
}

//// GetReference ////////////////////////////////////////////////////////////
//	Note: need to overload because MAX for some reason writes out the
//	references by their INDEX. ARRRRGH!

RefTargetHandle plBumpMtl::GetReference( int i )
{
	switch( i )
	{
		case kRefBasic:  return fBasicPB;
		case kRefAnim:   return fAnimPB;
	}

	return plPassMtlBase::GetReference( i );
}

//// SetReference ////////////////////////////////////////////////////////////
//	Note: need to overload because MAX for some reason writes out the
//	references by their INDEX. ARRRRGH!

void plBumpMtl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == kRefBasic)
		fBasicPB = (IParamBlock2 *)rtarg;
	else if (i == kRefAnim)
		fAnimPB = (IParamBlock2 *)rtarg;
	else
		plPassMtlBase::SetReference( i, rtarg );
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

int plBumpMtl::NumSubs()
{
	return 3;
}

TSTR plBumpMtl::SubAnimName(int i) 
{
	switch (i)
	{
	case 0: return fBasicPB->GetLocalName();
	case 1: return fAnimPB->GetLocalName();
	case 2:	return "Base Layer";
	}

	return "";
}

Animatable* plBumpMtl::SubAnim(int i)
{
	switch (i)
	{
	case 0: return fBasicPB;
	case 1: return fAnimPB;
	case 2:	return fBasicPB->GetTexmap(kBumpBasLayer);
		break;
	}

	return NULL;
}

int	plBumpMtl::NumParamBlocks()
{
	return 2;
}

IParamBlock2* plBumpMtl::GetParamBlock(int i)
{
	return (IParamBlock2*)GetReference(i);
}

IParamBlock2* plBumpMtl::GetParamBlockByID(BlockID id)
{
	if (fBasicPB->ID() == id)
		return fBasicPB;
	else if (fAnimPB->ID() == id)
		return fAnimPB;

	return NULL;
}

RefResult plBumpMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	return plPassMtlBase::NotifyRefChanged( changeInt, hTarget, partID, message );
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plBumpMtl::NumSubTexmaps()
{
	return 1;
}

Texmap* plBumpMtl::GetSubTexmap(int i)
{
	if (i == 0)
		return fBasicPB->GetTexmap(kBumpBasLayer);

	return NULL;
}

void plBumpMtl::SetSubTexmap(int i, Texmap *m)
{
	if (i == 0)
		fBasicPB->SetValue(kBumpBasLayer, 0, m);
}

TSTR plBumpMtl::GetSubTexmapSlotName(int i)
{
	if (i == 0)
		return "Base";

	return "";
}

TSTR plBumpMtl::GetSubTexmapTVName(int i)
{
	return GetSubTexmapSlotName(i);
}

/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle plBumpMtl::Clone(RemapDir &remap)
{
	plBumpMtl *mnew = TRACKED_NEW plBumpMtl(FALSE);
	plPassMtlBase::ICloneBase( mnew, remap );
	return (RefTargetHandle)mnew;
}

void		plBumpMtl::ICloneRefs( plPassMtlBase *target, RemapDir &remap )
{
	target->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));
	target->ReplaceReference(kRefAnim, remap.CloneRef(fAnimPB));
}

void plBumpMtl::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plBumpMtl::Update(TimeValue t, Interval& valid) 
{	
	// mf horse - Hacking in something like real validity checking
	// to get material animations working. No warranty, this is just
	// better than nothing.
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();
		if( fBasicPB->GetTexmap(kBumpBasLayer) )
			fBasicPB->GetTexmap(kBumpBasLayer)->Update(t, fIValid);

//		fLayersPB->GetValue(kMtlLayLayer1On, t, fMapOn[0], fIValid);
/*
		for (int i = 0; i < fSubTexmap.Count(); i++)
		{
			if (fSubTexmap[i]) 
				fSubTexmap[i]->Update(t,fIValid);
		}
*/
	}

	valid &= fIValid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void plBumpMtl::SetAmbient(Color c, TimeValue t) {}		
void plBumpMtl::SetDiffuse(Color c, TimeValue t) {}		
void plBumpMtl::SetSpecular(Color c, TimeValue t) {}
void plBumpMtl::SetShininess(float v, TimeValue t) {}
				
Color plBumpMtl::GetAmbient(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plBumpMtl::GetDiffuse(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plBumpMtl::GetSpecular(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }

float plBumpMtl::GetXParency(int mtlNum, BOOL backFace)
{
	return 0.f;
}

float plBumpMtl::GetShininess(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plBumpMtl::GetShinStr(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plBumpMtl::WireSize(int mtlNum, BOOL backFace)		{ return 0.0f; }

/////////////////////////////////////////////////////////////////

void plBumpMtl::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{
#if 0
	if (texHandleValid.InInterval(t)) {
		mtl->texture.SetCount(numTexHandlesUsed);
		for (int i=0; i<numTexHandlesUsed; i++) {
			if (texHandle[i]) {
				mtl->texture[i].textHandle = texHandle[i]->GetHandle();
				Texmap *tx = (*maps)[useSubForTex[i]].map;
				cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], tx ); 		
				SetTexOps(mtl,i,texOpsType[i]);
				}
			}
		return;
		}
#endif

#if 0	// WTF?!?!?!?
	Texmap *tx[2];
	int diffChan = stdIDToChannel[ ID_DI ];
	int opacChan = stdIDToChannel[ ID_OP ];
	tx[0] = (*maps)[diffChan].IsActive()?(*maps)[diffChan].map:NULL;
	tx[1] = (*maps)[opacChan].IsActive()?(*maps)[opacChan].map:NULL;
#endif

	int nsupport = cb.NumberTexturesSupported();
#if 0
	BITMAPINFO *bmi[NTEXHANDLES];

	int nmaps=0;
	for (int i=0; i<NTEXHANDLES; i++) {
		if (tx[i]) nmaps ++;
		bmi[i] = NULL;
		}
	mtl->texture.SetCount(nmaps);
	if (nmaps==0) 
		return;
	for (i=0; i<nmaps; i++)
		mtl->texture[i].textHandle = NULL;
	texHandleValid.SetInfinite();
	Interval  valid;
	BOOL needDecal = FALSE;
	int ntx = 0;
	int op; 

	int forceW = 0;
	int forceH = 0;
	if (tx[0]) {
		cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], tx[0]); 		
		TextureInfo &ti = mtl->texture[0];
		if (ti.tiling[0]==GW_TEX_NO_TILING||ti.tiling[1]==GW_TEX_NO_TILING)
  			needDecal = TRUE;
		op = needDecal?TXOP_ALPHABLEND:TXOP_MODULATE;
		bmi[0] = tx[0]->GetVPDisplayDIB(t,cb,valid,FALSE); 
		if (bmi[0]) {
			texHandleValid &= valid;
			useSubForTex[0] = diffChan;
			ntx = 1;
			forceW = bmi[0]->bmiHeader.biWidth;
			forceH = bmi[0]->bmiHeader.biHeight;
			}
		}
	if (tx[1]) {
		cb.GetGfxTexInfoFromTexmap(t, mtl->texture[ntx], tx[1]); 		
		if (nsupport>ntx) {
			bmi[1] = tx[1]->GetVPDisplayDIB(t,cb,valid,TRUE); 
			if (bmi[1]) {
				texHandleValid &= valid;
				StuffAlpha(bmi[1], (*maps)[opacChan].amount, GetOpacity(t),ntx?whiteCol:pShader->GetDiffuseClr(t));
				texHandle[ntx] = cb.MakeHandle(bmi[1]); 
				bmi[1] = NULL; 
				mtl->texture[ntx].textHandle = texHandle[ntx]->GetHandle();
				SetTexOps(mtl,ntx,TXOP_OPACITY);
				useSubForTex[ntx] = opacChan;
				ntx++;
				}
			}
		else {
			if (!needDecal) {
				TextureInfo ti;
//				if (SameUV(mtl->texture[0],mtl->texture[1])) {
					// Not really correct to combine channels for different UV's but what the heck.
					bmi[1] = tx[1]->GetVPDisplayDIB(t,cb,valid,TRUE, forceW, forceH); 
					if (bmi[1]) {
						texHandleValid &= valid;
						StuffAlphaInto(bmi[1], bmi[0], (*maps)[opacChan].amount, GetOpacity(t));
						op = TXOP_OPACITY;
						free(bmi[1]);
						bmi[1] = NULL;
						}
//					}
				}
			}
		}
	if (bmi[0])	{
		texHandle[0] = cb.MakeHandle(bmi[0]); 
		bmi[0] = NULL; 
		mtl->texture[0].textHandle = texHandle[0]->GetHandle();
		SetTexOps(mtl,0,op);
		}
	mtl->texture.SetCount(ntx);
	numTexHandlesUsed = ntx;
#endif
}

/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void plBumpMtl::GetInterpVtxValue(int channel, ShadeContext &sc, Point3 &val)
{
	Mesh *mesh = sc.globContext->GetRenderInstance(sc.NodeID())->mesh;
	if (mesh != nil)
	{
		Face *maxFace = &mesh->faces[ sc.FaceNumber() ];
		UVVert *map = mesh->mapVerts(channel);
		if (map != nil)
		{
			Point3 p0 = map[maxFace->getVert( 0 )];
			Point3 p1 = map[maxFace->getVert( 1 )];
			Point3 p2 = map[maxFace->getVert( 2 )];
			Point3 interp = sc.BarycentricCoords();
			val.x = interp.x * p0.x + interp.y * p1.x + interp.z * p2.x;
			val.y = interp.x * p0.y + interp.y * p1.y + interp.z * p2.y;
			val.z = interp.x * p0.z + interp.y * p1.z + interp.z * p2.z;
			return;
		}
	}

	// No value defined... set default.
	if (channel == MAP_SHADING)
		val.x = val.y = val.z = 0.0f;
	else
		val.x = val.y = val.z = 1.0f;
}

void plBumpMtl::Shade(ShadeContext& sc) 
{
	// Get the background color
	Color backColor, backTrans;
	sc.GetBGColor(backColor, backTrans);

	ShadeWithBackground(sc, backColor);
}

//// Requirements ////////////////////////////////////////////////////////////
//	Tells MAX what we need to render ourselves properly, such as translucency,
//	two-sidedness, etc. Flags are in imtl.h in the MAX SDK.

ULONG	plBumpMtl::Requirements( int subMtlNum ) 
{
	ULONG		req = 0;


	req = Mtl::Requirements( subMtlNum );

	// Uncomment this to get the background color fed to our ShadeWithBackground()
	// (slower processing tho)
	//req |= MTLREQ_BGCOL;
	req |= MTLREQ_UV;
	req |= MTLREQ_TRANSP;

	if (req & MTLREQ_FACEMAP)
	{
		int i = 0;
	}
	return req;
}

void plBumpMtl::ShadeWithBackground(ShadeContext &sc, Color background, bool useVtxAlpha /* = true */)
{
#if 1

	// old
#if 0
	Color lightCol,rescol, diffIllum0;
	RGBA mval;
	Point3 N0,P;
	BOOL bumped = FALSE;
	int i;

	if (gbufID) 
		sc.SetGBufferID(gbufID);
	
	if (sc.mode == SCMODE_SHADOW) {
		float opac = 0.0;
		for (i=0; i < NumSubTexmaps(); i++) 	{
			if (SubTexmapOn(i)) {
				hsMaxLayerBase *hsmLay = (hsMaxLayerBase *)GetSubTexmap(i);
				opac += hsmLay->GetOpacity(t);
			}
		}
		
		float f = 1.0f - opac;
		sc.out.t = Color(f,f,f);
		return;
	}
	
	N0 = sc.Normal();
	P = sc.P();
#endif

	TimeValue t = sc.CurTime();
	Color color(0, 0, 0);
	float alpha = 0.0;

	// Evaluate Base layer
	Texmap *map = fBasicPB->GetTexmap(kBumpBasLayer);
	if (map && ( map->ClassID() == LAYER_TEX_CLASS_ID 
				|| map->ClassID() == STATIC_ENV_LAYER_CLASS_ID ) )
	{
		plLayerTex *layer = (plLayerTex*)map;
		AColor evalColor = layer->EvalColor(sc);

		color = evalColor;
		alpha = evalColor.a;
	}

#if 1
	AColor black;
	black.Black();
	AColor white;
	white.White();


	SIllumParams ip;
	//
	// Shading setup
	//

	// Setup the parameters for the shader
	ip.amb = black;
	ip.diff = white;
	ip.diffIllum = black;
	ip.specIllum = black;
	ip.N = sc.Normal();
	ip.V = sc.V();


	//
	// Specularity
	//
	ip.spec = black;
	ip.sh_str = 0;
	ip.ph_exp = 0;
	ip.shine = 0;
	ip.softThresh = 0;

	//

	// Do the shading
	Shader *myShader = GetShader(SHADER_BLINN);
	myShader->Illum(sc, ip);

	ip.diffIllum.ClampMinMax();
	ip.specIllum.ClampMinMax();
	ip.diffIllum = ip.amb * sc.ambientLight + ip.diff * ip.diffIllum;

//	AColor returnColor = AColor(opac * ip.diffIllum + ip.specIllum, opac)
#endif

	float vtxAlpha = 1.0f;
	if (useVtxAlpha && GetOutputBlend() == plPassMtlBase::kBlendAlpha)
	{
		Point3 p;
		GetInterpVtxValue(MAP_ALPHA, sc, p);
		vtxAlpha = p.x;
	}
	alpha *= vtxAlpha;

	// MAX will do the additive/alpha/no blending for us based on what Requirements()
	// we tell it. However, since MAX's formula is bgnd*sc.out.t + sc.out.c,
	// we have to multiply our output color by the alpha.
	// If we ever need a more complicated blending function, you can request the
	// background color via Requirements() (otherwise it's just black) and then do
	// the blending yourself; however, if the transparency isn't set, the shadows
	// will be opaque, so be careful.
	Color outC = ip.diffIllum + ip.specIllum;

	sc.out.c = ( outC * alpha );
	sc.out.t = Color( 1.f - alpha, 1.f - alpha, 1.f - alpha );

#endif
}

float plBumpMtl::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval plBumpMtl::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();

	return iv;	
}

bool plBumpMtl::HasAlpha()
{
	return ((plLayerTex *)fBasicPB->GetTexmap(kBumpBasLayer))->HasAlpha();
}
// Massive list of inherited accessor functions for ParamBlock data

// Advanced Block
int		plBumpMtl::GetBasicWire() { return 0; }
int		plBumpMtl::GetMeshOutlines() { return 0; }
int		plBumpMtl::GetTwoSided() { return 0; }
int		plBumpMtl::GetSoftShadow() { return 0; }
int		plBumpMtl::GetNoProj() { return 0; }
int		plBumpMtl::GetVertexShade() { return 0; }
int		plBumpMtl::GetNoShade() { return 0; }
int		plBumpMtl::GetNoFog() { return 0; }
int		plBumpMtl::GetWhite() { return 0; }
int		plBumpMtl::GetZOnly() { return 0; }
int		plBumpMtl::GetZClear() { return 0; }
int		plBumpMtl::GetZNoRead() { return 0; }
int		plBumpMtl::GetZNoWrite() { return 0; }
int		plBumpMtl::GetZInc() { return 0; }
int		plBumpMtl::GetAlphaTestHigh() { return 0; }

// Animation block
char *	plBumpMtl::GetAnimName() { return fAnimPB->GetStr(kPBAnimName); }
int		plBumpMtl::GetAutoStart() { return fAnimPB->GetInt(kPBAnimAutoStart); }
int		plBumpMtl::GetLoop() { return fAnimPB->GetInt(kPBAnimLoop); }
char *	plBumpMtl::GetAnimLoopName() { return fAnimPB->GetStr(kPBAnimLoopName); }

// Basic block
int		plBumpMtl::GetColorLock() { return 0; }
Color	plBumpMtl::GetAmbColor() { return Color(0,0,0); }
Color	plBumpMtl::GetColor() { return Color(0,0,0); }
int		plBumpMtl::GetOpacity() { return 100; }
int		plBumpMtl::GetEmissive() { return 0; }
int		plBumpMtl::GetUseSpec() { return 0; }
int		plBumpMtl::GetShine() { return 0; }
Color	plBumpMtl::GetSpecularColor() { return Color(0,0,0); }
int		plBumpMtl::GetDiffuseColorLock() { return 0; }
Color	plBumpMtl::GetRuntimeColor() { return fBasicPB->GetColor(kBumpBasRunColor); }
Control *plBumpMtl::GetPreshadeColorController() { return nil; }
Control *plBumpMtl::GetAmbColorController() { return nil; }
Control *plBumpMtl::GetOpacityController() { return nil; }
Control *plBumpMtl::GetSpecularColorController() { return nil; }
Control *plBumpMtl::GetRuntimeColorController() { return fBasicPB->GetController(ParamID(kBumpBasRunColor)); }

// Layer block
Texmap *plBumpMtl::GetBaseLayer() { return fBasicPB->GetTexmap(kBumpBasLayer); }
int		plBumpMtl::GetTopLayerOn() { return 0; }
Texmap *plBumpMtl::GetTopLayer() { return nil; }
int		plBumpMtl::GetLayerBlend() { return 0; }
int		plBumpMtl::GetOutputAlpha() { return 0; }
int		plBumpMtl::GetOutputBlend() { return fBasicPB->GetInt( kBumpBasSpecular ) ? plPassMtlBase::kBlendAdd : plPassMtlBase::kBlendAlpha; }
