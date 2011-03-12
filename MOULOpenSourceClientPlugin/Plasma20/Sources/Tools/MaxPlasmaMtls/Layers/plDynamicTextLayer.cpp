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
#include "plDynamicTextLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

class plDynamicTextLayerClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plDynamicTextLayer(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_DYN_TEXT_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return DYN_TEXT_LAYER_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_2D; }
	const TCHAR*	InternalName()	{ return _T("PlasmaDynamicTextLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plDynamicTextLayerClassDesc plDynamicTextLayerDesc;
ClassDesc2* GetDynamicTextLayerDesc() { return &plDynamicTextLayerDesc; }

#include "plDynamicTextLayerBitmapPB.cpp"

ParamDlg* plDynamicTextLayer::fUVGenDlg = NULL;

plDynamicTextLayer::plDynamicTextLayer() :
	fBitmapPB(NULL),
	fUVGen(NULL),
	fTexHandle(NULL),
	fTexTime(0),
	fIValid(NEVER)
{
	fInitBitmap = NULL;

	plDynamicTextLayerDesc.MakeAutoParamBlocks(this);
	ReplaceReference(kRefUVGen, GetNewDefaultUVGen());	
}

plDynamicTextLayer::~plDynamicTextLayer()
{
	if( fInitBitmap )
		fInitBitmap->DeleteThis();

	IDiscardTexHandle();
}

//From MtlBase
void plDynamicTextLayer::Reset() 
{
	GetDynamicTextLayerDesc()->Reset(this, TRUE);	// reset all pb2's
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	fIValid.SetEmpty();
}

void plDynamicTextLayer::Update(TimeValue t, Interval& valid) 
{
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();

        fUVGen->Update(t,fIValid);
		fBitmapPB->GetValidity(t, fIValid);
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

Interval plDynamicTextLayer::Validity(TimeValue t)
{
	//TODO: Update fIValid here

	// mf horse - Hacking this in just to get animations working.
	// No warranty on this not being stupid.
	Interval v = FOREVER;
	fBitmapPB->GetValidity(t, v);
	v &= fUVGen->Validity(t);
	return v;
}

ParamDlg* plDynamicTextLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plDynamicTextLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	fUVGenDlg = fUVGen->CreateParamDlg(hwMtlEdit, imp);
	masterDlg->AddDlg(fUVGenDlg);

	return masterDlg;	
}

BOOL plDynamicTextLayer::SetDlgThing(ParamDlg* dlg)
{	
	if (dlg == fUVGenDlg)
	{
		fUVGenDlg->SetThing(fUVGen);
		return TRUE;
	}

	return FALSE;
}

int plDynamicTextLayer::NumRefs()
{
	return 2;
}

//From ReferenceMaker
RefTargetHandle plDynamicTextLayer::GetReference(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

void plDynamicTextLayer::SetReference(int i, RefTargetHandle rtarg) 
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
			// loaded.  In the case of load, we want to refresh our textures.
			if (fBitmapPB)
				RefreshBitmaps();
			break;
	}
}

int	plDynamicTextLayer::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plDynamicTextLayer::GetParamBlock(int i)
{
	switch (i)
	{
	case 0:	return fBitmapPB;
	default: return NULL;
	}
}

IParamBlock2* plDynamicTextLayer::GetParamBlockByID(BlockID id)
{
	if (fBitmapPB->ID() == id)
		return fBitmapPB;
	else
		return NULL;
}

//From ReferenceTarget 
RefTargetHandle plDynamicTextLayer::Clone(RemapDir &remap) 
{
	plDynamicTextLayer *mnew = TRACKED_NEW plDynamicTextLayer();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(kRefBitmap, remap.CloneRef(fBitmapPB));
	mnew->ReplaceReference(kRefUVGen, remap.CloneRef(fUVGen));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int plDynamicTextLayer::NumSubs()
{
	return 2;
}

Animatable* plDynamicTextLayer::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

TSTR plDynamicTextLayer::SubAnimName(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return "UVGen";
		case kRefBitmap:	return fBitmapPB->GetLocalName();
		default: return "";
	}
}

RefResult plDynamicTextLayer::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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

void plDynamicTextLayer::IChanged()
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

IOResult plDynamicTextLayer::Save(ISave *isave) 
{
	IOResult res;

	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plDynamicTextLayer::Load(ILoad *iload) 
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

inline Point2 CompUV(float x, float y, float z) 
{
	return Point2( 0.5f * ( x / z + 1.0f ), 0.5f * ( y / z + 1.0f ) );
}

Bitmap	*plDynamicTextLayer::GetBitmap( TimeValue t )
{
	return fInitBitmap;
}

AColor plDynamicTextLayer::EvalColor(ShadeContext& sc)
{
	if (!sc.doMaps) 
		return AColor(0.0f, 0.0f, 0.0f, 1.0f);

	AColor color;
	if (sc.GetCache(this, color)) 
		return color;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	// Evaluate the Bitmap
	if( fBitmapPB->GetInt( kBmpUseInitImage ) && fInitBitmap )
	{
		plBMSampler mysamp( this, fInitBitmap );
		color = fUVGen->EvalUVMap( sc, &mysamp, TRUE );
	}
	else
		color.White();

	// Invert color if specified
	if( fBitmapPB->GetInt( kBmpInvertColor ) )
	{
		color.r = 1.0f - color.r;
		color.g = 1.0f - color.g;
		color.b = 1.0f - color.b;
	}
	// Discard color if specified
	if( fBitmapPB->GetInt( kBmpDiscardColor ) )
		color.r = color.g = color.b = 1.0f;

	// Invert alpha if specified
	if( fBitmapPB->GetInt( kBmpInvertAlpha ) )
		color.a = 1.0f - color.a;
	// Discard alpha if specified
	if( fBitmapPB->GetInt( kBmpDiscardAlpha ) )
		color.a = 1.0f;

	sc.PutCache(this, color); 
	return color;
}

float plDynamicTextLayer::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 plDynamicTextLayer::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plDynamicTextLayer::LocalRequirements(int subMtlNum)
{
	return fUVGen->Requirements( subMtlNum );
}

void plDynamicTextLayer::IDiscardTexHandle() 
{
	if (fTexHandle)
	{
		fTexHandle->DeleteThis();
		fTexHandle = NULL;
	}
}

void plDynamicTextLayer::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff)
		IDiscardTexHandle();
}

BITMAPINFO *plDynamicTextLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
						// FIXME
	fTexTime = 0;//CalcFrame(t);
//	texValid = clipValid;
	BITMAPINFO *bmi = NULL;
	int xflags = 0;

	// Create a bitmap to write into via Windows
	BITMAPINFO tempBMI;
	memset( &tempBMI.bmiHeader, 0, sizeof( BITMAPINFOHEADER ) );
	tempBMI.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	tempBMI.bmiHeader.biWidth = fBitmapPB->GetInt( kBmpExportWidth );
	tempBMI.bmiHeader.biHeight = -(int)fBitmapPB->GetInt( kBmpExportHeight );
	tempBMI.bmiHeader.biPlanes = 1;
	tempBMI.bmiHeader.biCompression = BI_RGB;
	tempBMI.bmiHeader.biBitCount = 32;

	DWORD		*bitmapBits;
	HDC winDC = CreateCompatibleDC( nil );
	HBITMAP bitmap = CreateDIBSection( winDC, &tempBMI, DIB_RGB_COLORS, (void **)&bitmapBits, nil, 0 );

	HBITMAP old = (HBITMAP)SelectObject( winDC, bitmap );

	// Write into it now
	RECT	r;
	SetRect( &r, 0, 0, fBitmapPB->GetInt( kBmpExportWidth ) - 1, fBitmapPB->GetInt( kBmpExportHeight ) - 1 );
	HBRUSH brush = CreateSolidBrush( RGB( 255, 0, 0 ) );
	FrameRect( winDC, &r, brush );
	DeleteObject( brush );

	SetMapMode( winDC, MM_TEXT );
	SetBkMode( winDC, TRANSPARENT );
	SetTextAlign( winDC, TA_TOP | TA_LEFT );

	// Background letters
	int nHeight = -MulDiv( 72, GetDeviceCaps( winDC, LOGPIXELSY ), 72 );
	HFONT winFont = CreateFont( nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, "Times New Roman" );
	if( winFont != nil )
	{
		HFONT origFont = (HFONT)SelectObject( winDC, winFont );
		SetTextColor( winDC, RGB( 32, 32, 32 ) );
		char str2[] = "ABCDEFG";
		::TextOut( winDC, 0, 0, str2, strlen( str2 ) );
		SelectObject( winDC, origFont );
		DeleteObject( winFont );
	}

	nHeight = -MulDiv( 8, GetDeviceCaps( winDC, LOGPIXELSY ), 72 );
	winFont = CreateFont( nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, "Arial" );
	if( winFont != nil )
	{
		HFONT origFont = (HFONT)SelectObject( winDC, winFont );

		SetTextColor( winDC, RGB( 255, 255, 255 ) );
		char str[] = "Dynamic Text";
		::TextOut( winDC, 0, 0, str, strlen( str ) );
		char str3[] = "This is 8 point Arial";
		::TextOut( winDC, 0, 12, str3, strlen( str3 ) );

		SelectObject( winDC, origFont );
		DeleteObject( winFont );
	}


	/// Create a MAX bitmap and copy over the data, the painful way
	Bitmap	*maxBmp;
	BitmapInfo maxInfo;

	maxInfo.SetType( BMM_TRUE_32 );
	maxInfo.SetWidth( fBitmapPB->GetInt( kBmpExportWidth ) );
	maxInfo.SetHeight( fBitmapPB->GetInt( kBmpExportHeight ) );
	maxInfo.SetFlags( MAP_HAS_ALPHA );
	maxInfo.SetCustomFlag( 0 );
	maxBmp = TheManager->Create( &maxInfo );

	PixelBuf l64( fBitmapPB->GetInt( kBmpExportWidth ) );
	for( int y = 0; y < fBitmapPB->GetInt( kBmpExportHeight ); y++ )
	{
		BMM_Color_64 *p64 = l64.Ptr();
		for( int x = 0; x < fBitmapPB->GetInt( kBmpExportWidth ); x++, p64++ )
		{
			COLORREF color = GetPixel( winDC, x, y );		

			if( color == RGB( 0, 0, 0 ) )
			{
				if( fBitmapPB->GetInt( kBmpUseInitImage ) && fInitBitmap != nil )
					fInitBitmap->GetLinearPixels( x, y, 1, p64 );
				else
					p64->r = p64->g = p64->b = 0.f;
			}
			else
			{
				p64->r = GetRValue( color ) << 8;
				p64->g = GetGValue( color ) << 8;
				p64->b = GetBValue( color ) << 8;
			}
			p64->a = 0xffff;
		}
		maxBmp->PutPixels( 0, y, fBitmapPB->GetInt( kBmpExportWidth ), l64.Ptr() );
	}

	// Done with these now
	SelectObject( winDC, old );
	DeleteObject( bitmap );
	DeleteObject( winDC );

	// Convert to a BITMAPINFO. Go figure.
	bmi = thmaker.BitmapToDIB( maxBmp, 0, xflags, forceW, forceH );

	return bmi;
}

DWORD plDynamicTextLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) 
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

const char *plDynamicTextLayer::GetTextureName( int which )
{
	PBBitmap *pbbm = fBitmapPB->GetBitmap( kBmpInitBitmap );
	if( pbbm )
		return pbbm->bi.Name();
	return NULL;
}

void plDynamicTextLayer::ISetPBBitmap(PBBitmap *pbbm, int index /* = 0 */)
{ 
	fBitmapPB->SetValue( (ParamID)kBmpInitBitmap, 0, pbbm ); 
}

PBBitmap *plDynamicTextLayer::GetPBBitmap(int index /* = 0 */)
{ 
	return fBitmapPB->GetBitmap( (ParamID)kBmpInitBitmap ); 
}

//// GetSamplerInfo ///////////////////////////////////////////////////////////
//	Virtual function called by plBMSampler to get various things while sampling 
//	the layer's image

bool	plDynamicTextLayer::GetSamplerInfo( plBMSamplerData *samplerData )
{
	if( fBitmapPB->GetInt( (ParamID)kBmpDiscardAlpha ) )
		samplerData->fAlphaSource = plBMSamplerData::kDiscard;
	else
		samplerData->fAlphaSource = plBMSamplerData::kFromTexture;

	return true;
}

