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
#include "plParticleMtl.h"
#include "resource.h"
//extern ClassDesc2* GetMaxLayerDesc();
#include "Shaders.h"

#include "iparamm2.h"

#include "../MaxMain/plPlasmaRefMsgs.h"
#include "plBMSampler.h"
#include "stdmat.h"
#include "Layers/plLayerTex.h"
#include "Layers/plLayerTexBitmapPB.h"

extern HINSTANCE hInstance;

class plParticleMtlClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading) { return TRACKED_NEW plParticleMtl(loading); }
	const TCHAR*	ClassName()		{ return GetString(IDS_PARTICLE_MTL); }
	SClass_ID		SuperClassID()	{ return MATERIAL_CLASS_ID; }
	Class_ID		ClassID()		{ return PARTICLE_MTL_CLASS_ID; }
	const TCHAR* 	Category()		{ return NULL; }
	const TCHAR*	InternalName()	{ return _T("ParticleMaterial"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plParticleMtlClassDesc plParticleMtlDesc;
ClassDesc2* GetParticleMtlDesc() { return &plParticleMtlDesc; }

// For initializing paramblock descriptor
ParamBlockDesc2 *GetParticlePB();
#include "plParticleMtlPBDec.h"

const char *plParticleMtl::NormalStrings[] = // Make sure these match up in order with the Normal enum (in the header)
{
	"Normal: View Facing",
	"Normal: Up",
	"Normal: Nearest Light",
	"Normal: From Center",
	"Normal: Vel x Up x Vel",
	"Emissive"
};

plParticleMtl::plParticleMtl(BOOL loading) : fBasicPB(NULL)//, fBM(NULL), fUVGen(NULL)
{
#if 0 // This wasn't working on load
	// Initialize the paramblock descriptors only once
	static bool descInit = false;
	if (!descInit)
	{
		descInit = true;
		GetParticlePB()->SetClassDesc(GetParticleMtlDesc());
	}
#endif

	plParticleMtlDesc.MakeAutoParamBlocks(this);

//	if (!loading)
	{
		Reset();
		plLayerTex *tex = TRACKED_NEW plLayerTex;
		//tex->GetParamBlockByID(kBlkBasic)->SetValue(kBmpUseBitmap, 0, 1);
		fBasicPB->SetValue(kTexmap, 0, tex);

	}
	//fUVGen = GetNewDefaultUVGen();
}

void plParticleMtl::Reset() 
{
	fIValid.SetEmpty();
}

ParamDlg* plParticleMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plParticleMtlDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return (ParamDlg*)masterDlg;	
}

BOOL plParticleMtl::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval plParticleMtl::Validity(TimeValue t)
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

	return v;
#endif // mf horse
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

int plParticleMtl::NumSubs()
{
	return 2;
}

TSTR plParticleMtl::SubAnimName(int i) 
{
	switch (i)
	{
	case 0: return fBasicPB->GetLocalName();
	case 1: return "Texmap";
	}

	return "";
}

Animatable* plParticleMtl::SubAnim(int i)
{
	switch (i)
	{
	case 0: return fBasicPB;
	case 1: return fBasicPB->GetTexmap(kTexmap);
	}

	return NULL;
}

int plParticleMtl::NumRefs()
{
	return 1;
}

RefTargetHandle plParticleMtl::GetReference(int i)
{
	switch (i)
	{
	case kRefBasic:  return fBasicPB;
	}

	return NULL;
}

void plParticleMtl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == kRefBasic)
		fBasicPB = (IParamBlock2 *)rtarg;
}

int	plParticleMtl::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plParticleMtl::GetParamBlock(int i)
{
	return (IParamBlock2*)GetReference(i);
}

IParamBlock2* plParticleMtl::GetParamBlockByID(BlockID id)
{
	if (fBasicPB->ID() == id)
		return fBasicPB;

	return NULL;
}

RefResult plParticleMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	switch (message)
	{
		case REFMSG_CHANGE:
			fIValid.SetEmpty();

			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item
			if (hTarget == fBasicPB)
			{
				IParamBlock2 *pb = (IParamBlock2*)hTarget;

				ParamID changingParam = pb->LastNotifyParamID();
				pb->GetDesc()->InvalidateUI(changingParam);
				
				// And let the SceneWatcher know that the material on some of it's
				// referenced objects changed.
				NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_MAT);
			}
			break;
	}

	return REF_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plParticleMtl::NumSubTexmaps()
{
	return 1;
}

Texmap* plParticleMtl::GetSubTexmap(int i)
{
	if (i == 0)
		return fBasicPB->GetTexmap(kTexmap);

	return NULL;
}

void plParticleMtl::SetSubTexmap(int i, Texmap *m)
{
	if (i == 0)
		fBasicPB->SetValue(kTexmap, 0, m);
}

TSTR plParticleMtl::GetSubTexmapSlotName(int i)
{
	if (i == 0)
		return "Texmap";

	return "";
}

TSTR plParticleMtl::GetSubTexmapTVName(int i)
{
	return GetSubTexmapSlotName(i);
}


/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult plParticleMtl::Save(ISave *isave)
{ 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plParticleMtl::Load(ILoad *iload)
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

RefTargetHandle plParticleMtl::Clone(RemapDir &remap)
{
	plParticleMtl *mnew = TRACKED_NEW plParticleMtl(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	mnew->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));

	BaseClone(this, mnew, remap);
	mnew->fIValid.SetEmpty();	

	return (RefTargetHandle)mnew;
}

void plParticleMtl::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plParticleMtl::Update(TimeValue t, Interval& valid) 
{	
	//StdUVGen *gen = (StdUVGen *)fUVGen;
	//gen->SetUScl(1.0f, t);
	//gen->SetVScl(1.0f, t);
	//gen->Update(t, fIValid);
	valid &= fIValid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void plParticleMtl::SetAmbient(Color c, TimeValue t) {}		
void plParticleMtl::SetDiffuse(Color c, TimeValue t) {}		
void plParticleMtl::SetSpecular(Color c, TimeValue t) {}
void plParticleMtl::SetShininess(float v, TimeValue t) {}
				
Color plParticleMtl::GetAmbient(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plParticleMtl::GetDiffuse(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plParticleMtl::GetSpecular(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }

float plParticleMtl::GetXParency(int mtlNum, BOOL backFace)
{
	int			opacity = fBasicPB->GetInt( kOpacity, 0 );
	float		alpha = 1.0f - ( (float)opacity / 100.0f );

	return alpha;
}

float plParticleMtl::GetShininess(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plParticleMtl::GetShinStr(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plParticleMtl::WireSize(int mtlNum, BOOL backFace)		{ return 0.0f; }

/////////////////////////////////////////////////////////////////

void plParticleMtl::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
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

void plParticleMtl::Shade(ShadeContext& sc) 
{
	// Get the background color
	Color backColor, backTrans;
	sc.GetBGColor(backColor, backTrans);

	ShadeWithBackground(sc, backColor);
}

//// Requirements ////////////////////////////////////////////////////////////
//	Tells MAX what we need to render ourselves properly, such as translucency,
//	two-sidedness, etc. Flags are in imtl.h in the MAX SDK.

ULONG	plParticleMtl::Requirements( int subMtlNum ) 
{
	ULONG		req = 0;


	req = Mtl::Requirements( subMtlNum );

	// Uncomment this to get the background color fed to our ShadeWithBackground()
	// (slower processing tho)
//	req |= MTLREQ_BGCOL;

	int blendType = fBasicPB->GetInt( kBlend );
	if( blendType == kBlendAdd )
		req |= MTLREQ_ADDITIVE_TRANSP | MTLREQ_TRANSP;
	else if( blendType == kBlendAlpha )
		req |= MTLREQ_TRANSP;
	else if( fBasicPB->GetInt( kOpacity, 0 ) != 100 )
		req |= MTLREQ_TRANSP;

	return req;
}

void plParticleMtl::ShadeWithBackground(ShadeContext &sc, Color background)
{
#if 1
	TimeValue t = sc.CurTime();
	Color color(0, 0, 0);
	float alpha = 0.0;

	// Evaluate Base layer
	Texmap *map = fBasicPB->GetTexmap(kTexmap);
	if (map && map->ClassID() == LAYER_TEX_CLASS_ID)
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
	if( fBasicPB->GetInt( kNormal ) == kEmissive )
	{
		// Emissive objects don't get shaded
		ip.diffIllum = fBasicPB->GetColor(kColorAmb, t) * color;
		ip.diffIllum.ClampMinMax();
		ip.specIllum = black;
	}
	else
	{
		//
		// Shading setup
		//

		// Setup the parameters for the shader
		ip.amb = black;
		ip.diff = fBasicPB->GetColor(kColor, t) * color;
		ip.spec = white;
		ip.diffIllum = black;
		ip.specIllum = black;
		ip.N = sc.Normal();
		ip.V = sc.V();


		//
		// Specularity
		//
		ip.sh_str = 0;
		ip.ph_exp = 0;
		ip.shine = 0;

		ip.softThresh = 0;



		// Do the shading
		Shader *myShader = GetShader(SHADER_BLINN);
		myShader->Illum(sc, ip);

		ip.diffIllum.ClampMinMax();
		ip.specIllum.ClampMinMax();
		ip.diffIllum = ip.amb * sc.ambientLight + ip.diff * ip.diffIllum;
	}

//	AColor returnColor = AColor(opac * ip.diffIllum + ip.specIllum, opac)
#endif

	// Get opacity and combine with alpha
	float opac = float(fBasicPB->GetInt(kOpacity, t)) / 100.0f;
	//float opac = 1.0f;
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

float plParticleMtl::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval plParticleMtl::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();

	return iv;	
}

Control *plParticleMtl::GetAmbColorController() { return fBasicPB->GetController(ParamID(kColorAmb)); }
Control *plParticleMtl::GetColorController() { return fBasicPB->GetController(ParamID(kColor)); }
Control *plParticleMtl::GetOpacityController() { return fBasicPB->GetController(ParamID(kOpacity)); }
Control *plParticleMtl::GetWidthController() { return fBasicPB->GetController(ParamID(kWidth)); }
Control *plParticleMtl::GetHeightController() { return fBasicPB->GetController(ParamID(kHeight)); }
