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
#include "plLayerTex.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../resource.h"
#include "plLayerTexBitmapPB.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

class plLayerTexClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plLayerTex(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return LAYER_TEX_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_2D; }
	const TCHAR*	InternalName()	{ return _T("PlasmaLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plLayerTexClassDesc plLayerTexDesc;
ClassDesc2* GetLayerTexDesc() { return &plLayerTexDesc; }

ParamDlg* plLayerTex::fUVGenDlg = NULL;

// For initializing paramblock descriptor
//ParamBlockDesc2 *GetBasicBlk();
ParamBlockDesc2 *GetBitmapBlk();

//#include "plLayerTexBasicPB.cpp"
#include "plLayerTexBitmapPB.cpp"

void	plLayerTex::GetClassName( TSTR &s )
{
	s = GetString( IDS_LAYER );
}

plLayerTex::plLayerTex() :
	fBitmapPB(NULL),
	//fBasicPB(NULL),
	fUVGen(NULL),
	fTexHandle(NULL),
	fTexTime(0),
	fBM(NULL),
	fIValid(NEVER)
{
#if 0
	// Initialize the paramblock descriptors only once
	static bool descInit = false;
	if (!descInit)
	{
		descInit = true;
		//GetBasicBlk()->SetClassDesc(GetLayerTexDesc());
		GetBitmapBlk()->SetClassDesc(GetLayerTexDesc());
	}
#endif

	plLayerTexDesc.MakeAutoParamBlocks(this);
	ReplaceReference(kRefUVGen, GetNewDefaultUVGen());	
}

plLayerTex::~plLayerTex()
{
	if (fBM)
		fBM->DeleteThis();

	IDiscardTexHandle();
}

//From MtlBase
void plLayerTex::Reset() 
{
	GetLayerTexDesc()->Reset(this, TRUE);	// reset all pb2's
	SetBitmap(NULL);

	fIValid.SetEmpty();
}

void plLayerTex::Update(TimeValue t, Interval& valid) 
{
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();

        fUVGen->Update(t,fIValid);
		fBitmapPB->GetValidity(t, fIValid);

//		Interval clipValid;
//		clipValid.SetInfinite();
//		float temp;
//		fBitmapPB->GetValue(kBmpClipU, t, temp, clipValid);
//		fBitmapPB->GetValue(kBmpClipV, t, temp, clipValid);
//		fBitmapPB->GetValue(kBmpClipW, t, temp, clipValid);
//		fBitmapPB->GetValue(kBmpClipH, t, temp, clipValid);
	}

	// Gonna need to do this when we support animated bm's
#if 0
	if (fBM)
	{
		if (bi.FirstFrame()!=bi.LastFrame())
			ivalid.SetInstant(t);
	}
#endif

	valid &= fIValid;
}

Interval plLayerTex::Validity(TimeValue t)
{
	//TODO: Update fIValid here

	// mf horse - Hacking this in just to get animations working.
	// No warranty on this not being stupid.
	Interval v = FOREVER;
	fBitmapPB->GetValidity(t, v);
	//fBasicPB->GetValidity(t, v);
	v &= fUVGen->Validity(t);
	return v;
}

ParamDlg* plLayerTex::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fMtlParams = imp;

	IAutoMParamDlg* masterDlg = plLayerTexDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	fUVGenDlg = fUVGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(fUVGenDlg);

	return masterDlg;	
}

BOOL plLayerTex::SetDlgThing(ParamDlg* dlg)
{	
	if (dlg == fUVGenDlg)
	{
		fUVGenDlg->SetThing(fUVGen);
		return TRUE;
	}

	return FALSE;
}

int plLayerTex::NumRefs()
{
	return 3;
}

//From ReferenceMaker
RefTargetHandle plLayerTex::GetReference(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		//case kRefBasic:		return fBasicPB;
		default: return NULL;
	}
}

void plLayerTex::SetReference(int i, RefTargetHandle rtarg) 
{
	Interval	garbage;

	switch (i)
	{
		case kRefUVGen:  
			fUVGen = (UVGen *)rtarg; 
			if( fUVGen )
				fUVGen->Update( TimeValue( 0 ), garbage );
			break;
		case kRefBitmap:
			fBitmapPB = (IParamBlock2 *)rtarg;
			// KLUDGE: If the paramblock is being set chances are we are being created or
			// loaded.  In the case of load, we want to refresh our texture.
			if (fBitmapPB)
				RefreshBitmaps();
			break;
	}
}

int	plLayerTex::NumParamBlocks()
{
	return 2;
}

IParamBlock2* plLayerTex::GetParamBlock(int i)
{
	switch (i)
	{
	case 0:	return fBitmapPB;
	//case 1:	return fBasicPB;
	default: return NULL;
	}
}

IParamBlock2* plLayerTex::GetParamBlockByID(BlockID id)
{
	if (fBitmapPB->ID() == id)
		return fBitmapPB;
	//else if (fBasicPB->ID() == id)
	//	return fBasicPB;
	else
		return NULL;
}

//From ReferenceTarget 
RefTargetHandle plLayerTex::Clone(RemapDir &remap) 
{
	plLayerTex *mnew = TRACKED_NEW plLayerTex();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	//mnew->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));
	mnew->ReplaceReference(kRefBitmap, remap.CloneRef(fBitmapPB));
	mnew->ReplaceReference(kRefUVGen, remap.CloneRef(fUVGen));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int plLayerTex::NumSubs()
{
	return 3;
}

Animatable* plLayerTex::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		//case kRefBasic:		return fBasicPB;
		default: return NULL;
	}
}

TSTR plLayerTex::SubAnimName(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return "UVGen";
		case kRefBitmap:	return fBitmapPB->GetLocalName();
		//case kRefBasic:		return fBasicPB->GetLocalName();
		default: return "";
	}
}

RefResult plLayerTex::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message) 
{
	switch (message)
	{
	case REFMSG_CHANGE:
		{
			fIValid.SetEmpty();

			if (hTarget == fBitmapPB)
			{
				// see if this message came from a changing parameter in the pblock,
				// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changingParam = fBitmapPB->LastNotifyParamID();
				fBitmapPB->GetDesc()->InvalidateUI(changingParam);

				if (changingParam != -1)
					IChanged();
			}
		}
		break;

	case REFMSG_UV_SYM_CHANGE:
		IDiscardTexHandle();  
		break;
	}

	return REF_SUCCEED;
}

BOOL plLayerTex::DiscardColor() 
{ 
	return fBitmapPB->GetInt(kBmpDiscardColor); 
}

BOOL plLayerTex::DiscardAlpha() 
{ 
	return fBitmapPB->GetInt(kBmpDiscardAlpha); 
}

void plLayerTex::IChanged()
{
	IDiscardTexHandle();
	// Texture wasn't getting updated in the viewports, and this fixes it.
	// Don't know if it's the right way though.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	// And this is so the SceneWatcher gets notified that the material on some of it's
	// referenced objects changed.
	NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_MAT);
}

#define TEX_HDR_CHUNK 0x5000
#define MAX_ASS_CHUNK 0x5500

IOResult plLayerTex::Save(ISave *isave) 
{
	IOResult res;
	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plLayerTex::Load(ILoad *iload) 
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

bool plLayerTex::HasAlpha()
{
   return (fBM != NULL && fBM->HasAlpha() != 0);
}

Bitmap* plLayerTex::GetBitmap(TimeValue t)
{
	return fBM;
}

AColor plLayerTex::EvalColor(ShadeContext& sc)
{
	if (!sc.doMaps) 
		return AColor(0.0f, 0.0f, 0.0f, 1.0f);

	AColor color;
	if (sc.GetCache(this, color)) 
		return color;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	//
	// Evaluate the Bitmap
	//
	if (fBitmapPB->GetInt(kBmpUseBitmap) && fBM)
	{
		plBMSampler mysamp(this, fBM);
		color = fUVGen->EvalUVMap(sc, &mysamp, FALSE);
		// We'd like to pass TRUE and actually filter the image, but that seems to be
		// tripping an odd crash in Max internals. *shrug*
	}
	else
		color.White();

	// Invert color if specified
	if (fBitmapPB->GetInt(kBmpInvertColor))
	{
		color.r = 1.0f - color.r;
		color.g = 1.0f - color.g;
		color.b = 1.0f - color.b;
	}
	// Discard color if specified
	if (fBitmapPB->GetInt(kBmpDiscardColor))
		color.r = color.g = color.b = 1.0f;

	// Invert alpha if specified
	if (fBitmapPB->GetInt(kBmpInvertAlpha))
		color.a = 1.0f - color.a;
	// Discard alpha if specified
	if (fBitmapPB->GetInt(kBmpDiscardAlpha))
		color.a = 1.0f;

	// If RGB output is set to alpha, show RGB as grayscale of the alpha
	if (fBitmapPB->GetInt(kBmpRGBOutput) == 1)
		color = AColor(color.a, color.a, color.a, 1.0f);

	sc.PutCache(this, color); 
	return color;
}

float plLayerTex::EvalMono(ShadeContext& sc)
{
	if (fBitmapPB->GetInt(kBmpMonoOutput) == 1)
		return EvalColor(sc).a;

	return Intens(EvalColor(sc));
}

Point3 plLayerTex::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plLayerTex::LocalRequirements(int subMtlNum)
{
	return fUVGen->Requirements(subMtlNum); 
}

#if 0
int plLayerTex::ICalcFrame(TimeValue t) 
{
	PBBitmap *pbbm = fBitmapPB->GetBitmap(kBmpBitmap);
	if (!pbbm || !pbbm->bi)
		return 0;
	BitmapInfo *bi = pbbm->bi;

	TimeValue tm, dur, td;
	int frameStart = bi->FirstFrame();
	int frameEnd = bi->LastFrame();
	int tpf = GetTicksPerFrame();
	tm = TimeValue(float(t - startTime) * pbRate);
	dur = (fend-fstart+1)*GetTicksPerFrame();

	switch (endCond)
	{
	case END_HOLD:
		if (tm <= 0)
			return frameStart;
		if (tm >= dur)
			return frameEnd;
		return tm/tpf;

	case END_PINGPONG:
		if (((tm >= 0) && ((tm / dur) & 1)) || ((tm < 0) && !(tm / dur)))
		{
			td = modt(tm, dur);
			return frameStart + frameEnd - td / tpf;
		}
		// else fall through
	case END_LOOP:
		td = modt(tm, dur);
		return td / tpf;
	}

	return 0;
}
#endif

void plLayerTex::IDiscardTexHandle() 
{
	if (fTexHandle)
	{
		fTexHandle->DeleteThis();
		fTexHandle = NULL;
	}
}

void plLayerTex::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff)
		IDiscardTexHandle();
}

BITMAPINFO *plLayerTex::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
						// FIXME
	fTexTime = 0;//CalcFrame(t);
//	texValid = clipValid;
	BITMAPINFO *bmi = NULL;
	int xflags = 0;

	if (fBitmapPB->GetInt(kBmpApply))
	{
		float clipu = fBitmapPB->GetFloat(kBmpClipU);
		float clipv = fBitmapPB->GetFloat(kBmpClipV);
		float clipw = fBitmapPB->GetFloat(kBmpClipW);
		float cliph = fBitmapPB->GetFloat(kBmpClipH);
		int discardAlpha = fBitmapPB->GetInt(kBmpDiscardAlpha);
		int alphaAsRGB = (fBitmapPB->GetInt(kBmpRGBOutput) == 1);

		int w = fBM->Width();
		int h = fBM->Height();

		Bitmap *newBM;
		BitmapInfo bi;
		bi.SetName(_T("y8798734"));
		bi.SetType(BMM_TRUE_32);
		bi.SetFlags(MAP_HAS_ALPHA);

		if (fBitmapPB->GetInt(kBmpCropPlace) == 1)
		{
			int x0, y0, nw, nh;
			int bmw = thmaker.Size();
			int bmh = int(float(bmw)*float(h)/float(w));
			bi.SetWidth(bmw);
			bi.SetHeight(bmh);
			newBM = TheManager->Create(&bi);
			newBM->Fill(0,0,0,0);
			nw = int(float(bmw)*clipw);
			nh = int(float(bmh)*cliph);
			x0 = int(float(bmw-1)*clipu);
			y0 = int(float(bmh-1)*clipv);
			
			if (nw<1) nw = 1;
			if (nh<1) nh = 1;
			PixelBuf row(nw);
			
			Bitmap *tmpBM;
			BitmapInfo bif2;
			bif2.SetName(_T("xxxx67878"));
			bif2.SetType(BMM_TRUE_32);
			bif2.SetFlags(MAP_HAS_ALPHA);
			bif2.SetWidth(nw);				
			bif2.SetHeight(nh);
			tmpBM = TheManager->Create(&bif2);
			tmpBM->CopyImage(fBM, COPY_IMAGE_RESIZE_LO_QUALITY, 0);
			BMM_Color_64*  p1 = row.Ptr();
			for (int y = 0; y<nh; y++)
			{
				tmpBM->GetLinearPixels(0,y, nw, p1);
				if (alphaAsRGB)
				{
					for (int ix =0; ix<nw; ix++) 
						p1[ix].r = p1[ix].g = p1[ix].b = p1[ix].a;
				}
				if (discardAlpha)
				{
					for (int ix = 0; ix < nw; ix++) 
						p1[ix].a = 0xffff;
				}
				newBM->PutPixels(x0, y+y0, nw, p1);
			}
			tmpBM->DeleteThis();
			bmi = thmaker.BitmapToDIB(newBM, fUVGen->SymFlags(), xflags, forceW, forceH);
			newBM->DeleteThis();
		}
		else
		{
			int x0,y0,nw,nh;
			x0 = int(float(w-1)*clipu);
			y0 = int(float(h-1)*clipv);
			nw = int(float(w)*clipw);
			nh = int(float(h)*cliph);
			if (nw<1) nw = 1;
			if (nh<1) nh = 1;
			bi.SetWidth(nw);
			bi.SetHeight(nh);
			PixelBuf row(nw);
			newBM = TheManager->Create(&bi);
			BMM_Color_64*  p1 = row.Ptr();
			for (int y = 0; y<nh; y++)
			{
				fBM->GetLinearPixels(x0,y+y0, nw, p1);
				if (alphaAsRGB)
				{
					for (int ix = 0; ix < nw; ix++) 
						p1[ix].r = p1[ix].g = p1[ix].b = p1[ix].a;
				}
				if (discardAlpha)
				{
					for (int ix = 0; ix < nw; ix++) 
						p1[ix].a = 0xffff;
				}
				newBM->PutPixels(0, y, nw, p1);
			}
			bmi = thmaker.BitmapToDIB(newBM, fUVGen->SymFlags(), xflags, forceW, forceH);
			newBM->DeleteThis();
		}
	}
	else
	{
		if (fBitmapPB->GetInt(kBmpRGBOutput) == 1)
			xflags |= EX_RGB_FROM_ALPHA;
		bmi = thmaker.BitmapToDIB(fBM, fUVGen->SymFlags(), xflags, forceW, forceH);
	}

	return bmi;
}

DWORD plLayerTex::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) 
{
	// FIXME: ignore validity for now
	if (fTexHandle && fIValid.InInterval(t))// && texTime == CalcFrame(t)) 
		return fTexHandle->GetHandle();
	else
	{
		IDiscardTexHandle();
		
		fTexTime = 0;//CalcFrame(t);
		fTexHandle = thmaker.MakeHandle(GetVPDisplayDIB(t, thmaker, fIValid));
		if (fTexHandle)
			return fTexHandle->GetHandle();
		else
			return 0;
	}
}

const char *plLayerTex::GetTextureName()
{
//	if (fBitmapPB->GetInt(kBmpUseBitmap))
	{
		PBBitmap *pbbm = fBitmapPB->GetBitmap(kBmpBitmap);
		if (pbbm)
			return pbbm->bi.Name();
	}

	return NULL;
}

void plLayerTex::ISetPBBitmap(PBBitmap *pbbm, int index /* = 0 */)
{ 
	fBitmapPB->SetValue(ParamID(kBmpBitmap), 0, pbbm, index); 
}

PBBitmap *plLayerTex::GetPBBitmap(int index /* = 0 */)
{ 
	return fBitmapPB->GetBitmap(ParamID(kBmpBitmap)); 
}

//// GetSamplerInfo ///////////////////////////////////////////////////////////
//	Virtual function called by plBMSampler to get various things while sampling 
//	the layer's image

bool	plLayerTex::GetSamplerInfo( plBMSamplerData *samplerData )
{
	samplerData->fClipU = fBitmapPB->GetFloat( (ParamID)kBmpClipU );
	samplerData->fClipV = fBitmapPB->GetFloat( (ParamID)kBmpClipV );
	samplerData->fClipW = fBitmapPB->GetFloat( (ParamID)kBmpClipW );
	samplerData->fClipH = fBitmapPB->GetFloat( (ParamID)kBmpClipH );

	samplerData->fEnableCrop = fBitmapPB->GetInt( (ParamID)kBmpApply ) ? true : false;
	samplerData->fCropPlacement = fBitmapPB->GetInt( (ParamID)kBmpCropPlace );

	if( fBitmapPB->GetInt( (ParamID)kBmpDiscardAlpha ) )
		samplerData->fAlphaSource = plBMSamplerData::kDiscard;
	else if( fBitmapPB->GetInt( (ParamID)kBmpRGBOutput ) == 1 )
		samplerData->fAlphaSource = plBMSamplerData::kFromRGB;
	else
		samplerData->fAlphaSource = plBMSamplerData::kFromTexture;

	return true;
}

void plLayerTex::SetExportSize(int x, int y)
{
	fBitmapPB->SetValue(kBmpExportWidth, 0, x);
	fBitmapPB->SetValue(kBmpExportLastWidth, 0, x);
	fBitmapPB->SetValue(kBmpExportHeight, 0, y);
	fBitmapPB->SetValue(kBmpExportLastHeight, 0, y);
}

