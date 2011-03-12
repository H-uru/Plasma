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
#include "hsUtils.h"
#include "plClothingMtl.h"
#include "resource.h"
#include "Shaders.h"

#include "iparamm2.h"

#include "../MaxMain/plPlasmaRefMsgs.h"
#include "plBMSampler.h"
#include "stdmat.h"
#include "Layers/plLayerTex.h"
#include "Layers/plLayerTexBitmapPB.h"
#include "../../Plasma/PubUtilLib/plAvatar/plClothingLayout.h"

extern HINSTANCE hInstance;

class plClothingMtlClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading) { return TRACKED_NEW plClothingMtl(loading); }
	const TCHAR*	ClassName()		{ return GetString(IDS_CLOTHING_MTL); }
	SClass_ID		SuperClassID()	{ return MATERIAL_CLASS_ID; }
	Class_ID		ClassID()		{ return CLOTHING_MTL_CLASS_ID; }
	const TCHAR* 	Category()		{ return NULL; }
	const TCHAR*	InternalName()	{ return _T("ClothingMaterial"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plClothingMtlClassDesc plClothingMtlDesc;
ClassDesc2* GetClothingMtlDesc() { return &plClothingMtlDesc; }

// For initializing paramblock descriptor
ParamBlockDesc2 *GetClothingPB();
#include "plClothingMtlPBDec.h"

const UINT32 plClothingMtl::ButtonConstants[] =
{
	IDC_CLOTHING_TEXTURE1,
	IDC_CLOTHING_TEXTURE2,
	IDC_CLOTHING_TEXTURE3,
	IDC_CLOTHING_TEXTURE4
};

const UINT32 plClothingMtl::TextConstants[] =
{
	IDC_CLOTHING_TILE1_NAME,
	IDC_CLOTHING_TILE1_SIZE,
	IDC_CLOTHING_TILE2_NAME,
	IDC_CLOTHING_TILE2_SIZE,
	IDC_CLOTHING_TILE3_NAME,
	IDC_CLOTHING_TILE3_SIZE,
	IDC_CLOTHING_TILE4_NAME,
	IDC_CLOTHING_TILE4_SIZE
};

const char *plClothingMtl::LayerStrings[] = 
{
	"Base",
	"Skin",
	"Skin Blend (1)",
	"Skin Blend (2)",
	"Skin Blend (3)",
	"Skin Blend (4)",
	"Skin Blend (5)",
	"Skin Blend (6)",
	"Tint 1",
	"Tint 2"
};

const UInt8 plClothingMtl::LayerToPBIdx[] =
{
	kTexmapBase,
	kTexmapSkin,
	kTexmapSkinBlend1,
	kTexmapSkinBlend2,
	kTexmapSkinBlend3,
	kTexmapSkinBlend4,
	kTexmapSkinBlend5,
	kTexmapSkinBlend6,
	kTexmap,
	kTexmap2
};

plClothingMtl::plClothingMtl(BOOL loading) : fBasicPB(NULL)
{
	plClothingMtlDesc.MakeAutoParamBlocks(this);

	Reset();
	int i;
	for (i = 0; i < plClothingMtl::kMaxTiles; i++)
	{
		plLayerTex *tex = TRACKED_NEW plLayerTex;
		fBasicPB->SetValue(ParamID(kTexmap), 0, tex, i);
	}
	fBasicPB->SetValue(ParamID(kThumbnail), 0, TRACKED_NEW plLayerTex);
}

void plClothingMtl::Reset() 
{
	fIValid.SetEmpty();
}

ParamDlg* plClothingMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plClothingMtlDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return (ParamDlg*)masterDlg;	
}

BOOL plClothingMtl::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval plClothingMtl::Validity(TimeValue t)
{
	// mf horse - Hacking in something like real validity checking
	// to get material animations working. No warranty, this is just
	// better than nothing.
	Interval v = FOREVER;
	fBasicPB->GetValidity(t, v);

	return v;
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

/*
int plClothingMtl::NumSubs()
{
	return 2;
}

TSTR plClothingMtl::SubAnimName(int i) 
{
	switch (i)
	{
	case 0: return fBasicPB->GetLocalName();
	case 1: return "Texmap";
	}

	return "";
}

Animatable* plClothingMtl::SubAnim(int i)
{
	switch (i)
	{
	case 0: return fBasicPB;
	case 1: return fBasicPB->GetTexmap(kTexmap);
	}

	return NULL;
}
*/

int plClothingMtl::NumRefs()
{
	return 1;
}

RefTargetHandle plClothingMtl::GetReference(int i)
{
	switch (i)
	{
	case kRefBasic:  return fBasicPB;
	}

	return NULL;
}

void plClothingMtl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == kRefBasic)
		fBasicPB = (IParamBlock2 *)rtarg;
}

int	plClothingMtl::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plClothingMtl::GetParamBlock(int i)
{
	return (IParamBlock2*)GetReference(i);
}

IParamBlock2* plClothingMtl::GetParamBlockByID(BlockID id)
{
	if (fBasicPB->ID() == id)
		return fBasicPB;

	return NULL;
}

RefResult plClothingMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
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

int plClothingMtl::NumSubTexmaps()
{
	return plClothingElement::kLayerMax * kMaxTiles + 1; // add one for the thumbnail
}

Texmap* plClothingMtl::GetSubTexmap(int i)
{
	if (i >= 0 && i < plClothingElement::kLayerMax * kMaxTiles)
		return fBasicPB->GetTexmap(ParamID(LayerToPBIdx[i / kMaxTiles]), 0, i % kMaxTiles);
	if (i == plClothingElement::kLayerMax * kMaxTiles)
		return fBasicPB->GetTexmap(ParamID(kThumbnail));

	return NULL;
}

void plClothingMtl::SetSubTexmap(int i, Texmap *m)
{
	if (i >= 0 && i < plClothingElement::kLayerMax * kMaxTiles)
		fBasicPB->SetValue(ParamID(LayerToPBIdx[i / kMaxTiles]), 0, m, i % kMaxTiles);
	if (i == plClothingElement::kLayerMax * kMaxTiles)
		fBasicPB->SetValue(kThumbnail, 0, m);
}

TSTR plClothingMtl::GetSubTexmapSlotName(int i)
{
	if (i >= 0 && i < plClothingElement::kLayerMax * kMaxTiles)
		return "Texmap";
	if (i == plClothingElement::kLayerMax * kMaxTiles)
		return "Thumbnail";

	return "";
}

TSTR plClothingMtl::GetSubTexmapTVName(int i)
{
	return GetSubTexmapSlotName(i);
}

DllExport Texmap *plClothingMtl::GetTexmap(int index, int layer) 
{ 
	return fBasicPB->GetTexmap(ParamID(LayerToPBIdx[layer]), 0, index); 
}

/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult plClothingMtl::Save(ISave *isave)
{ 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plClothingMtl::Load(ILoad *iload)
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

RefTargetHandle plClothingMtl::Clone(RemapDir &remap)
{
	plClothingMtl *mnew = TRACKED_NEW plClothingMtl(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	mnew->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));

	BaseClone(this, mnew, remap);
	mnew->fIValid.SetEmpty();	

	return (RefTargetHandle)mnew;
}

void plClothingMtl::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plClothingMtl::Update(TimeValue t, Interval& valid) 
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

void plClothingMtl::SetAmbient(Color c, TimeValue t) {}		
void plClothingMtl::SetDiffuse(Color c, TimeValue t) {}		
void plClothingMtl::SetSpecular(Color c, TimeValue t) {}
void plClothingMtl::SetShininess(float v, TimeValue t) {}
				
Color plClothingMtl::GetAmbient(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plClothingMtl::GetDiffuse(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }
Color plClothingMtl::GetSpecular(int mtlNum, BOOL backFace)	{ return Color(0,0,0); }

float plClothingMtl::GetXParency(int mtlNum, BOOL backFace)
{
	/*
	int			opacity = fBasicPB->GetInt( kOpacity, 0 );
	float		alpha = 1.0f - ( (float)opacity / 100.0f );

	return alpha;
	*/
	return 0;
}

float plClothingMtl::GetShininess(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plClothingMtl::GetShinStr(int mtlNum, BOOL backFace)	{ return 0.0f; }
float plClothingMtl::WireSize(int mtlNum, BOOL backFace)		{ return 0.0f; }

/////////////////////////////////////////////////////////////////

void plClothingMtl::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{
}

/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void plClothingMtl::Shade(ShadeContext& sc) 
{
	// Get the background color
	Color backColor, backTrans;
	sc.GetBGColor(backColor, backTrans);

	ShadeWithBackground(sc, backColor);
}

//// Requirements ////////////////////////////////////////////////////////////
//	Tells MAX what we need to render ourselves properly, such as translucency,
//	two-sidedness, etc. Flags are in imtl.h in the MAX SDK.

ULONG	plClothingMtl::Requirements( int subMtlNum ) 
{
	ULONG		req = 0;


	req = Mtl::Requirements( subMtlNum );

	// Uncomment this to get the background color fed to our ShadeWithBackground()
	// (slower processing tho)
//	req |= MTLREQ_BGCOL;

	//int blendType = fBasicPB->GetInt( kBlend );
	//if( blendType == kBlendAdd )
	//	req |= MTLREQ_ADDITIVE_TRANSP | MTLREQ_TRANSP;
	//else if( blendType == kBlendAlpha )
	//	req |= MTLREQ_TRANSP;
	//else if( fBasicPB->GetInt( kOpacity, 0 ) != 100 )
	//	req |= MTLREQ_TRANSP;

	return req;
}

void plClothingMtl::ShadeWithBackground(ShadeContext &sc, Color background)
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
	//if( fBasicPB->GetInt( kNormal ) == kEmissive )
	//{
		// Emissive objects don't get shaded
	//	ip.diffIllum = fBasicPB->GetColor(kColorAmb, t) * color;
	//	ip.diffIllum.ClampMinMax();
	//	ip.specIllum = black;
	//}
	//else
	{
		//
		// Shading setup
		//

		// Setup the parameters for the shader
		ip.amb = black;
		ip.diff = white; //fBasicPB->GetColor(kColor, t) * color;
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
	float opac = 1.0f; //float(fBasicPB->GetInt(kOpacity, t)) / 100.0f;
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

float plClothingMtl::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval plClothingMtl::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();

	return iv;	
}

plClothingElement *plClothingMtl::FindElementByName(char *name)
{
	int i;
	for (i = 0; i < fElements.GetCount(); i++)
	{
		if (!strcmp(fElements.Get(i)->fName, name))
			return fElements.Get(i);
	}
	return nil;	
}

void plClothingMtl::InitTilesets()
{
	hsAssert(fElements.GetCount() == 0, "Tilesets already initialized");
	fElements.Reset();
	fTilesets.SetCountAndZero(plClothingLayout::kMaxTileset);

	plClothingElement::GetElements(fElements);
/*
	plClothingTileset *tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("shirt");
	tileset->AddElement(FindElementByName("shirt-chest"));
	tileset->AddElement(FindElementByName("shirt-sleeve"));
	fTilesets[plClothingLayout::kSetShirt] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("face");
	tileset->AddElement(FindElementByName("face"));
	fTilesets[plClothingLayout::kSetFace] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("eye");
	tileset->AddElement(FindElementByName("eyeball"));
	fTilesets[plClothingLayout::kSetEye] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("shoe");
	tileset->AddElement(FindElementByName("shoe-top"));
	tileset->AddElement(FindElementByName("shoe-bottom"));
	fTilesets[plClothingLayout::kSetShoe] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("pants");
	tileset->AddElement(FindElementByName("pants"));
	fTilesets[plClothingLayout::kSetPants] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("hand");
	tileset->AddElement(FindElementByName("hand-LOD"));
	tileset->AddElement(FindElementByName("hand-square"));
	tileset->AddElement(FindElementByName("hand-wide"));
	fTilesets[plClothingLayout::kSetHand] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("playerbook");
	tileset->AddElement(FindElementByName("playerbook"));
	fTilesets[plClothingLayout::kSetPlayerBook] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("backpack");
	tileset->AddElement(FindElementByName("backpack"));
	fTilesets[plClothingLayout::kSetBackpack] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("glasses");
	tileset->AddElement(FindElementByName("glasses-front"));
	tileset->AddElement(FindElementByName("glasses-side"));
	fTilesets[plClothingLayout::kSetGlasses] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("KI");
	tileset->AddElement(FindElementByName("KI"));
	fTilesets[plClothingLayout::kSetKI] = tileset;
	*/
	plClothingTileset *tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Torso");
	tileset->AddElement(FindElementByName("Chest"));
	tileset->AddElement(FindElementByName("Arm"));
	fTilesets[plClothingLayout::kSetShirt] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Head");
	tileset->AddElement(FindElementByName("Eye"));	
	tileset->AddElement(FindElementByName("Extra Hair"));
	tileset->AddElement(FindElementByName("Face"));
	tileset->AddElement(FindElementByName("Hat"));
	fTilesets[plClothingLayout::kSetFace] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Foot");
	tileset->AddElement(FindElementByName("Foot"));
	fTilesets[plClothingLayout::kSetShoe] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Legs");
	tileset->AddElement(FindElementByName("Legs"));
	fTilesets[plClothingLayout::kSetPants] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Hand");
	tileset->AddElement(FindElementByName("Finger"));
	tileset->AddElement(FindElementByName("LOD"));
	tileset->AddElement(FindElementByName("Palm"));
	fTilesets[plClothingLayout::kSetHand] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Player Book");
	tileset->AddElement(FindElementByName("Player Book"));
	fTilesets[plClothingLayout::kSetPlayerBook] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("Glasses");
	tileset->AddElement(FindElementByName("Glasses"));
	fTilesets[plClothingLayout::kSetGlasses] = tileset;
	
	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("KI");
	tileset->AddElement(FindElementByName("KI"));
	fTilesets[plClothingLayout::kSetKI] = tileset;	

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("(unused)");
	fTilesets[plClothingLayout::kSetEye] = tileset;

	tileset = TRACKED_NEW plClothingTileset();
	tileset->SetName("(unused)");
	fTilesets[plClothingLayout::kSetBackpack] = tileset;	
}

void plClothingMtl::ReleaseTilesets()
{
	while (fElements.GetCount() > 0)
		delete fElements.Pop();
	while (fTilesets.GetCount() > 0)
		delete fTilesets.Pop();
}

/////////////////////////////////////////////////////////////////////////////////

plClothingTileset::plClothingTileset() : fName(nil) 
{
	fElements.Reset();
}

plClothingTileset::~plClothingTileset()
{
	delete [] fName;
}

void plClothingTileset::AddElement(plClothingElement *element)
{
	fElements.Append(element);	
}

void plClothingTileset::SetName(char *name)
{
	delete fName; fName = hsStrcpy(name);
}

