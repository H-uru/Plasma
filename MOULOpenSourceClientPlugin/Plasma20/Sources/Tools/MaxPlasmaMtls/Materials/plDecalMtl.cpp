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
#include "plDecalMtl.h"
#include "resource.h"
//extern ClassDesc2* GetMaxLayerDesc();
#include "Shaders.h"
#include "../MaxComponent/plMaxAnimUtils.h"

#include "plPassBaseParamIDs.h"
#include "plDecalMtlBasicPB.h"
#include "plDecalMtlLayersPB.h"

#include "iparamm2.h"

#include "Layers/plLayerTex.h"
#include "Layers/plStaticEnvLayer.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

extern HINSTANCE hInstance;

class plDecalMtlClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading) { return TRACKED_NEW plDecalMtl(loading); }
	const TCHAR*	ClassName()		{ return GetString(IDS_DECAL_MTL); }
	SClass_ID		SuperClassID()	{ return MATERIAL_CLASS_ID; }
	Class_ID		ClassID()		{ return DECAL_MTL_CLASS_ID; }
	const TCHAR* 	Category()		{ return NULL; }
	const TCHAR*	InternalName()	{ return _T("PlasmaMaterial"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plDecalMtlClassDesc plDecalMtlDesc;
ClassDesc2* GetDecalMtlDesc() { return &plDecalMtlDesc; }

// For initializing paramblock descriptor
ParamBlockDesc2 *GetDecalBasicPB();
ParamBlockDesc2 *GetDecalAdvPB();
ParamBlockDesc2 *GetDecalLayersPB();

#include "plDecalMtlAdvPBDec.h"
#include "plDecalMtlBasicPBDec.h"
#include "plDecalMtlLayersPBDec.h"
#include "plDecalMtlAnimPBDec.h"

plDecalMtl::plDecalMtl(BOOL loading) : plPassMtlBase( loading )
{
	plDecalMtlDesc.MakeAutoParamBlocks( this );
	fLayersPB->SetValue( kDecalLayBase, 0, TRACKED_NEW plLayerTex );
	fLayersPB->SetValue( kDecalLayTop, 0, TRACKED_NEW plLayerTex );

	// If we do this later (like, when the dialog loads) something blows up,
	// somewhere in Max.  It didn't in 4, it does in 7.  This seems to fix it.
	if (!loading)
		IVerifyStealthPresent(ENTIRE_ANIMATION_NAME);
}

ParamDlg* plDecalMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plDecalMtlDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return (ParamDlg*)masterDlg;	
}

BOOL plDecalMtl::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval plDecalMtl::Validity(TimeValue t)
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
	fAdvPB->GetValidity(t, v);

	if( fLayersPB->GetTexmap(kDecalLayBase) )
		v &= fLayersPB->GetTexmap(kDecalLayBase)->Validity(t);
	if( fLayersPB->GetTexmap(kDecalLayTop) )
		v &= fLayersPB->GetTexmap(kDecalLayTop)->Validity(t);
	return v;
#endif // mf horse
}

//// GetReference ////////////////////////////////////////////////////////////
//	Note: need to overload because MAX for some reason writes out the
//	references by their INDEX. ARRRRGH!

RefTargetHandle plDecalMtl::GetReference( int i )
{
	switch( i )
	{
		case kRefBasic:  return fBasicPB;
		case kRefAdv:    return fAdvPB;
		case kRefLayers: return fLayersPB;
		case kRefAnim:   return fAnimPB;
	}

	return plPassMtlBase::GetReference( i );
}

//// SetReference ////////////////////////////////////////////////////////////
//	Note: need to overload because MAX for some reason writes out the
//	references by their INDEX. ARRRRGH!

void plDecalMtl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == kRefBasic)
		fBasicPB = (IParamBlock2 *)rtarg;
	else if (i == kRefAdv)
		fAdvPB = (IParamBlock2 *)rtarg;
	else if (i == kRefLayers)
		fLayersPB = (IParamBlock2 *)rtarg;
	else if (i == kRefAnim)
		fAnimPB = (IParamBlock2 *)rtarg;
	else
		plPassMtlBase::SetReference( i, rtarg );
}


/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

int plDecalMtl::NumSubs()
{
	return 6;
}

TSTR plDecalMtl::SubAnimName(int i) 
{
	switch (i)
	{
	case 0: return fBasicPB->GetLocalName();
	case 1: return fAdvPB->GetLocalName();
	case 2: return fLayersPB->GetLocalName();
	case 3: return fAnimPB->GetLocalName();
	case 4:	return "Base Layer";
	case 5: return "Top Layer";
	}

	return "";
}

Animatable* plDecalMtl::SubAnim(int i)
{
	switch (i)
	{
	case 0: return fBasicPB;
	case 1: return fAdvPB;
	case 2: return fLayersPB;
	case 3: return fAnimPB;
	case 4:	return fLayersPB->GetTexmap(kDecalLayBase);
	case 5:
		if (fLayersPB->GetInt(kDecalLayTopOn))
			return fLayersPB->GetTexmap(kDecalLayTop);
		break;
	}

	return NULL;
}

int	plDecalMtl::NumParamBlocks()
{
	return 4;
}

IParamBlock2* plDecalMtl::GetParamBlock(int i)
{
	return (IParamBlock2*)GetReference(i);
}

IParamBlock2* plDecalMtl::GetParamBlockByID(BlockID id)
{
	if (fBasicPB->ID() == id)
		return fBasicPB;
	else if (fAdvPB->ID() == id)
		return fAdvPB;
	else if (fLayersPB->ID() == id)
		return fLayersPB;
	else if (fAnimPB->ID() == id)
		return fAnimPB;

	return NULL;
}

RefResult plDecalMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	return plPassMtlBase::NotifyRefChanged( changeInt, hTarget, partID, message );
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plDecalMtl::NumSubTexmaps()
{
   return 2;
}

Texmap* plDecalMtl::GetSubTexmap(int i)
{
   if (i == 0)
      return fLayersPB->GetTexmap(kDecalLayBase);
   else if (i == 1)
      return fLayersPB->GetTexmap(kDecalLayTop);
   
   return NULL;
}

void plDecalMtl::SetSubTexmap(int i, Texmap *m)
{
   if (i == 0)
      fLayersPB->SetValue(kDecalLayBase, 0, m);
   else if (i == 1)
      fLayersPB->SetValue(kDecalLayTop, 0, m);
}

TSTR plDecalMtl::GetSubTexmapSlotName(int i)
{
   if (i == 0)
      return "Base";
   else if (i == 1)
      return "Top";
   
   return "";
}

TSTR plDecalMtl::GetSubTexmapTVName(int i)
{
   return GetSubTexmapSlotName(i);
}

int plDecalMtl::SubTexmapOn(int i)
{
   if (i == 0)
      return 1;
   else if (i == 1 && fLayersPB->GetInt(kDecalLayTopOn))
      return 1;
   
   return 0;
}


/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle plDecalMtl::Clone(RemapDir &remap)
{
	plDecalMtl *mnew = TRACKED_NEW plDecalMtl(FALSE);
	plPassMtlBase::ICloneBase( mnew, remap );
	return (RefTargetHandle)mnew;
}

void	plDecalMtl::ICloneRefs( plPassMtlBase *target, RemapDir &remap )
{
	target->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));
	target->ReplaceReference(kRefAdv, remap.CloneRef(fAdvPB));
	target->ReplaceReference(kRefLayers, remap.CloneRef(fLayersPB));
	target->ReplaceReference(kRefAnim, remap.CloneRef(fAnimPB));
}

void plDecalMtl::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plDecalMtl::Update(TimeValue t, Interval& valid) 
{	
	// mf horse - Hacking in something like real validity checking
	// to get material animations working. No warranty, this is just
	// better than nothing.
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();
		if( fLayersPB->GetTexmap(kDecalLayBase) )
			fLayersPB->GetTexmap(kDecalLayBase)->Update(t, fIValid);
		if( fLayersPB->GetTexmap(kDecalLayTop) )
			fLayersPB->GetTexmap(kDecalLayTop)->Update(t, fIValid);

//		fLayersPB->GetValue(kMtlLayLayer1On, t, fMapOn[0], fIValid);
/*
		for (int i = 0; i < fSubTexmap.Count(); i++)
		{
			if (fSubTexmap[i]) 
				fSubTexmap[i]->Update(t,fIValid);
		}
*/
	}

	// Our wonderful way of version handling--if the runtimeColor is (-1,-1,-1), we know it's
	// just been initialized, so set it to the static color (this lets us do the right thing for
	// loading old paramBlocks)
	if( fBasicPB )
	{
		Color	run = fBasicPB->GetColor( kDecalBasRunColor, 0 );
		if( run == Color(-1,-1,-1) )
		{
			fBasicPB->SetValue( kDecalBasRunColor, 0, fBasicPB->GetColor( kDecalBasColor, 0 ) );
		}

		// Also, if shineStr is anything other than -1, then it must be an old paramblock and we need
		// to convert to our new specColor (we know this because the original valid range was 0-100)
		int	shine = fBasicPB->GetInt( kDecalBasShineStr, 0 );
		if( shine != -1 )
		{
			fBasicPB->SetValue( kDecalBasSpecColor, 0, Color( (float)shine / 100.f, (float)shine / 100.f, (float)shine / 100.f ) );
			fBasicPB->SetValue( kDecalBasShineStr, 0, (int)-1 );
		}		
	}

	valid &= fIValid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void plDecalMtl::SetAmbient(Color c, TimeValue t) {}		
void plDecalMtl::SetDiffuse(Color c, TimeValue t) {}		
void plDecalMtl::SetSpecular(Color c, TimeValue t) {}
void plDecalMtl::SetShininess(float v, TimeValue t) {}
				
Color plDecalMtl::GetAmbient(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plDecalMtl::GetDiffuse(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plDecalMtl::GetSpecular(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }

float plDecalMtl::GetXParency(int mtlNum, BOOL backFace)
{
	int			opacity = fBasicPB->GetInt( kDecalBasOpacity, 0 );
	float		alpha = 1.0f - ( (float)opacity / 100.0f );

	return alpha;
}

float plDecalMtl::GetShininess(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plDecalMtl::GetShinStr(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plDecalMtl::WireSize(int mtlNum, BOOL backFace)		{ return 0.0f; }

/////////////////////////////////////////////////////////////////

void plDecalMtl::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
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

void plDecalMtl::Shade(ShadeContext& sc) 
{
	// Get the background color
	Color backColor, backTrans;
	sc.GetBGColor(backColor, backTrans);

	ShadeWithBackground(sc, backColor);
}

//// Requirements ////////////////////////////////////////////////////////////
//	Tells MAX what we need to render ourselves properly, such as translucency,
//	two-sidedness, etc. Flags are in imtl.h in the MAX SDK.

ULONG	plDecalMtl::Requirements( int subMtlNum ) 
{
	ULONG		req = 0;


	req = Mtl::Requirements( subMtlNum );

	// Uncomment this to get the background color fed to our ShadeWithBackground()
	// (slower processing tho)
//	req |= MTLREQ_BGCOL;
	req |= MTLREQ_UV;

	int blendType = fLayersPB->GetInt( kDecalLayOutputBlend );
	if( blendType == kBlendAdd )
		req |= MTLREQ_ADDITIVE_TRANSP | MTLREQ_TRANSP;
	else if( blendType == kBlendAlpha )
		req |= MTLREQ_TRANSP;
	else if( fBasicPB->GetInt( kDecalBasOpacity, 0 ) != 100 )
		req |= MTLREQ_TRANSP;

	if( fAdvPB->GetInt( kPBAdvTwoSided ) )
		req |= MTLREQ_2SIDE;

	return req;
}

void plDecalMtl::ShadeWithBackground(ShadeContext &sc, Color background)
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
	Texmap *map = fLayersPB->GetTexmap(kDecalLayBase);
	if (map && ( map->ClassID() == LAYER_TEX_CLASS_ID 
				|| map->ClassID() == STATIC_ENV_LAYER_CLASS_ID ) )
	{
		plLayerTex *layer = (plLayerTex*)map;
		AColor evalColor = layer->EvalColor(sc);

		color = evalColor;
		alpha = evalColor.a;
	}

	// Evaluate Top layer, if it's on
	if (fLayersPB->GetInt(kDecalLayTopOn))
	{
		Texmap *map = fLayersPB->GetTexmap(kDecalLayTop);
		if (map && ( map->ClassID() == LAYER_TEX_CLASS_ID 
					|| map->ClassID() == STATIC_ENV_LAYER_CLASS_ID ) )
		{
			plLayerTex *layer = (plLayerTex*)map;
			AColor evalColor = layer->EvalColor(sc);

			// Blend layers
			int blendType = fLayersPB->GetInt(kDecalLayBlend);
			switch (blendType)
			{
			case kBlendAdd:
				color += evalColor * evalColor.a;
				break;
			case kBlendAlpha:
				color = (1.0f - evalColor.a) * color + evalColor.a * evalColor;
				alpha = 1 - (1 - evalColor.a) * (1 - alpha);
				break;
			case kBlendMult:
				color *= evalColor;
				break;
			default:	// No blend...
				color = evalColor;
				alpha = 1.0;
				break;
			}
		}
	}

#if 1
	AColor black;
	black.Black();
	AColor white;
	white.White();


	SIllumParams ip;
	if (fBasicPB->GetInt(kDecalBasEmissive))
	{
		// Emissive objects don't get shaded
		ip.diffIllum = fBasicPB->GetColor(kDecalBasColorAmb, t) * color;
		ip.diffIllum.ClampMinMax();
		ip.specIllum = black;
	}
	else
	{
		//
		// Shading setup
		//

		// Setup the parameters for the shader
		ip.amb = fBasicPB->GetColor(kDecalBasColorAmb, t);
		ip.diff = fBasicPB->GetColor(kDecalBasColor, t) * color;
		ip.diffIllum = black;
		ip.specIllum = black;
		ip.N = sc.Normal();
		ip.V = sc.V();


		//
		// Specularity
		//
		if (fBasicPB->GetInt(kDecalBasUseSpec, t))
		{
			ip.sh_str = 1.f;
			ip.spec = fBasicPB->GetColor( kDecalBasSpecColor, t );
			ip.ph_exp = (float)pow(2.0f,float(fBasicPB->GetInt(kDecalBasShine, t)) / 10.0f);
			ip.shine = float(fBasicPB->GetInt(kDecalBasShine, t)) / 100.0f;
		}
		else
		{
			ip.sh_str = 0;
			ip.ph_exp = 0;
			ip.shine = 0;
			ip.spec = black;
		}
		ip.softThresh = 0;

		//

		// Do the shading
		Shader *myShader = GetShader(SHADER_BLINN);
		myShader->Illum(sc, ip);

		// Override shader parameters
		if (fAdvPB->GetInt(kPBAdvNoShade))
		{
			ip.diffIllum = black;
			ip.specIllum = black;
		}
		if (fAdvPB->GetInt(kPBAdvWhite))
		{
			ip.diffIllum = white;
			ip.specIllum = black;
		}

		ip.diffIllum.ClampMinMax();
		ip.specIllum.ClampMinMax();
		ip.diffIllum = ip.amb * sc.ambientLight + ip.diff * ip.diffIllum;
	}

//	AColor returnColor = AColor(opac * ip.diffIllum + ip.specIllum, opac)
#endif

	// Get opacity and combine with alpha
	float opac = float(fBasicPB->GetInt(kDecalBasOpacity, t)) / 100.0f;
	alpha *= opac;

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

float plDecalMtl::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval plDecalMtl::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();

	return iv;	
}

bool plDecalMtl::HasAlpha()
{
	return ((plLayerTex *)fLayersPB->GetTexmap(kDecalLayBase))->HasAlpha();
}

// Massive list of inherited accessor functions for ParamBlock data

// Advanced Block
int		plDecalMtl::GetBasicWire() { return fAdvPB->GetInt(kPBAdvWire); }
int		plDecalMtl::GetMeshOutlines() { return fAdvPB->GetInt(kPBAdvMeshOutlines); }
int		plDecalMtl::GetTwoSided() { return fAdvPB->GetInt(kPBAdvTwoSided); }
int		plDecalMtl::GetSoftShadow() { return fAdvPB->GetInt(kPBAdvSoftShadow); }
int		plDecalMtl::GetNoProj() { return fAdvPB->GetInt(kPBAdvNoProj); }
int		plDecalMtl::GetVertexShade() { return fAdvPB->GetInt(kPBAdvVertexShade); }
int		plDecalMtl::GetNoShade() { return fAdvPB->GetInt(kPBAdvNoShade); }
int		plDecalMtl::GetNoFog() { return fAdvPB->GetInt(kPBAdvNoFog); }
int		plDecalMtl::GetWhite() { return fAdvPB->GetInt(kPBAdvWhite); }
int		plDecalMtl::GetZOnly() { return fAdvPB->GetInt(kPBAdvZOnly); }
int		plDecalMtl::GetZClear() { return fAdvPB->GetInt(kPBAdvZClear); }
int		plDecalMtl::GetZNoRead() { return fAdvPB->GetInt(kPBAdvZNoRead); }
int		plDecalMtl::GetZNoWrite() { return fAdvPB->GetInt(kPBAdvZNoWrite); }
int		plDecalMtl::GetZInc() { return fAdvPB->GetInt(kPBAdvZInc); }
int		plDecalMtl::GetAlphaTestHigh() { return fAdvPB->GetInt(kPBAdvAlphaTestHigh); }

// Animation block
char *	plDecalMtl::GetAnimName() { return fAnimPB->GetStr(kPBAnimName); }
int		plDecalMtl::GetAutoStart() { return fAnimPB->GetInt(kPBAnimAutoStart); }
int		plDecalMtl::GetLoop() { return fAnimPB->GetInt(kPBAnimLoop); }
char *	plDecalMtl::GetAnimLoopName() { return fAnimPB->GetStr(kPBAnimLoopName); }
int		plDecalMtl::GetEaseInType() { return fAnimPB->GetInt(kPBAnimEaseInType); }
float	plDecalMtl::GetEaseInNormLength() { return fAnimPB->GetFloat(kPBAnimEaseInLength); }
float	plDecalMtl::GetEaseInMinLength() { return fAnimPB->GetFloat(kPBAnimEaseInMin); }
float	plDecalMtl::GetEaseInMaxLength() { return fAnimPB->GetFloat(kPBAnimEaseInMax); }
int		plDecalMtl::GetEaseOutType() { return fAnimPB->GetInt(kPBAnimEaseOutType); }
float	plDecalMtl::GetEaseOutNormLength() { return fAnimPB->GetFloat(kPBAnimEaseOutLength); }
float	plDecalMtl::GetEaseOutMinLength() { return fAnimPB->GetFloat(kPBAnimEaseOutMin); }
float	plDecalMtl::GetEaseOutMaxLength() { return fAnimPB->GetFloat(kPBAnimEaseOutMax); }
int		plDecalMtl::GetUseGlobal() { return fAnimPB->GetInt(ParamID(kPBAnimUseGlobal)); }
char *	plDecalMtl::GetGlobalVarName() { return fAnimPB->GetStr(ParamID(kPBAnimGlobalName)); }	

// Basic block
int		plDecalMtl::GetColorLock() { return fBasicPB->GetInt(kDecalBasColorLock); }
Color	plDecalMtl::GetAmbColor() { return fBasicPB->GetColor(kDecalBasColorAmb); }
Color	plDecalMtl::GetColor() { return fBasicPB->GetColor(kDecalBasColor); }
int		plDecalMtl::GetOpacity() { return fBasicPB->GetInt(kDecalBasOpacity); }
int		plDecalMtl::GetEmissive() { return fBasicPB->GetInt(kDecalBasEmissive); }
int		plDecalMtl::GetUseSpec() { return fBasicPB->GetInt(kDecalBasUseSpec); }
int		plDecalMtl::GetShine() { return fBasicPB->GetInt(kDecalBasShine); }
Color	plDecalMtl::GetSpecularColor() { return fBasicPB->GetColor(kDecalBasSpecColor); }
Control *plDecalMtl::GetPreshadeColorController() { return fBasicPB->GetController(ParamID(kDecalBasColor)); }
Control *plDecalMtl::GetAmbColorController() { return fBasicPB->GetController(ParamID(kDecalBasColorAmb)); }
Control *plDecalMtl::GetOpacityController() { return fBasicPB->GetController(ParamID(kDecalBasOpacity)); }
Control *plDecalMtl::GetSpecularColorController() { return fBasicPB->GetController(ParamID(kDecalBasSpecColor)); }
int		plDecalMtl::GetDiffuseColorLock() { return fBasicPB->GetInt(kDecalBasDiffuseLock); }
Color	plDecalMtl::GetRuntimeColor() { return fBasicPB->GetColor(kDecalBasRunColor); }
Control *plDecalMtl::GetRuntimeColorController() { return fBasicPB->GetController(ParamID(kDecalBasRunColor)); }

// Layer block
Texmap *plDecalMtl::GetBaseLayer() { return fLayersPB->GetTexmap(kDecalLayBase); }
int		plDecalMtl::GetTopLayerOn() { return fLayersPB->GetInt(kDecalLayTopOn); }
Texmap *plDecalMtl::GetTopLayer() { return fLayersPB->GetTexmap(kDecalLayTop); }
int		plDecalMtl::GetLayerBlend() { return fLayersPB->GetInt(kDecalLayBlend); }
int		plDecalMtl::GetOutputAlpha() { return fLayersPB->GetInt(kDecalLayOutputAlpha); }
int		plDecalMtl::GetOutputBlend() { return fLayersPB->GetInt(kDecalLayOutputBlend); }

