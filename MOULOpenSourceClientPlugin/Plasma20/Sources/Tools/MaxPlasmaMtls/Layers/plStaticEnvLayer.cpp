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
#include "plStaticEnvLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

class plStaticEnvLayerClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plStaticEnvLayer(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_STATIC_ENVMAP_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return STATIC_ENV_LAYER_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_ENV; }
	const TCHAR*	InternalName()	{ return _T("PlasmaStaticEnvMapLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plStaticEnvLayerClassDesc plStaticEnvLayerDesc;
ClassDesc2* GetStaticEnvLayerDesc() { return &plStaticEnvLayerDesc; }

#include "plStaticEnvLayerBitmapPB.cpp"

plStaticEnvLayer::plStaticEnvLayer() :
	fBitmapPB(NULL),
	fUVGen(NULL),
	fTexHandle(NULL),
	fTexTime(0),
	fIValid(NEVER)
{
	int	i;

	for( i = 0; i < 6; i++ )
	{
		fBitmaps[ i ] = NULL;
	}

	plStaticEnvLayerDesc.MakeAutoParamBlocks(this);
	ReplaceReference(kRefUVGen, GetNewDefaultUVGen());	
}

plStaticEnvLayer::~plStaticEnvLayer()
{
	int		i;


	for( i = 0; i < 6; i++ )
	{
		if( fBitmaps[ i ] )
			fBitmaps[ i ]->DeleteThis();
	}

	IDiscardTexHandle();
}

//From MtlBase
void plStaticEnvLayer::Reset() 
{
	GetStaticEnvLayerDesc()->Reset(this, TRUE);	// reset all pb2's
	for( int i = 0; i < 6; i++ )
	{
		SetBitmap( NULL, i );
	}

	fIValid.SetEmpty();
}

void plStaticEnvLayer::Update(TimeValue t, Interval& valid) 
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

Interval plStaticEnvLayer::Validity(TimeValue t)
{
	//TODO: Update fIValid here

	// mf horse - Hacking this in just to get animations working.
	// No warranty on this not being stupid.
	Interval v = FOREVER;
	fBitmapPB->GetValidity(t, v);
	v &= fUVGen->Validity(t);
	return v;
}

ParamDlg* plStaticEnvLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plStaticEnvLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	SELBitmapDlgProc *paramDlg =  (SELBitmapDlgProc *)gBitmapParamBlk.GetUserDlgProc();
	if( paramDlg )
		paramDlg->fMtlParams = imp;

	return masterDlg;	
}

BOOL plStaticEnvLayer::SetDlgThing(ParamDlg* dlg)
{	
	return FALSE;
}

int plStaticEnvLayer::NumRefs()
{
	return 2;
}

//From ReferenceMaker
RefTargetHandle plStaticEnvLayer::GetReference(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

void plStaticEnvLayer::SetReference(int i, RefTargetHandle rtarg) 
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

int	plStaticEnvLayer::NumParamBlocks()
{
	return 1;
}

IParamBlock2* plStaticEnvLayer::GetParamBlock(int i)
{
	switch (i)
	{
	case 0:	return fBitmapPB;
	default: return NULL;
	}
}

IParamBlock2* plStaticEnvLayer::GetParamBlockByID(BlockID id)
{
	if (fBitmapPB->ID() == id)
		return fBitmapPB;
	else
		return NULL;
}

//From ReferenceTarget 
RefTargetHandle plStaticEnvLayer::Clone(RemapDir &remap) 
{
	plStaticEnvLayer *mnew = TRACKED_NEW plStaticEnvLayer();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(kRefBitmap, remap.CloneRef(fBitmapPB));
	mnew->ReplaceReference(kRefUVGen, remap.CloneRef(fUVGen));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

int plStaticEnvLayer::NumSubs()
{
	return 2;
}

Animatable* plStaticEnvLayer::SubAnim(int i) 
{
	//TODO: Return 'i-th' sub-anim
	switch (i)
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

TSTR plStaticEnvLayer::SubAnimName(int i) 
{
	switch (i)
	{
		case kRefUVGen:		return "UVGen";
		case kRefBitmap:	return fBitmapPB->GetLocalName();
		default: return "";
	}
}

RefResult plStaticEnvLayer::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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

void plStaticEnvLayer::IChanged()
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

IOResult plStaticEnvLayer::Save(ISave *isave) 
{
	IOResult res;

	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plStaticEnvLayer::Load(ILoad *iload) 
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

AColor plStaticEnvLayer::EvalColor(ShadeContext& sc)
{
	if (!sc.doMaps) 
		return AColor(0.0f, 0.0f, 0.0f, 1.0f);

	AColor color;
	if (sc.GetCache(this, color)) 
		return color;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	// Evaluate the Bitmap

//	Point3	v = sc.VectorTo( sc.V(), REF_OBJECT );//WORLD );
	Point3	v = sc.VectorTo( sc.Normal(), REF_OBJECT );
	float	wx,wy,wz;
	Color	rcol;
	Bitmap	*refmap = NULL;
	Point3	rv;
	Point2	uv;
	int		size;

	wx = (float)fabs( v.x );  
	wy = (float)fabs( v.y );
	wz = (float)fabs( v.z ); 
	if( wx >= wy && wx >= wz )
	{
		if( v.x < 0 )
		{
			refmap = fBitmaps[ kLeftFace ];
			uv = CompUV( -v.y, -v.z,  v.x );
		}
		else
		{
			refmap = fBitmaps[ kRightFace ];
			uv = CompUV( v.y, -v.z, -v.x );
		}
	}
	else if( wy >= wx && wy >= wz ) 
	{
		if( v.y > 0 )
		{
			refmap = fBitmaps[ kBackFace ];
			uv = CompUV( -v.x, -v.z, -v.y );
		}
		else
		{
			refmap = fBitmaps[ kFrontFace ];
			uv = CompUV(  v.x, -v.z,  v.y );
		}
	}
	else if( wz >= wx && wz >= wy ) 
	{
		if( v.z < 0 )
		{	
			refmap = fBitmaps[ kBottomFace ];
			uv = CompUV( -v.x, -v.y,  v.z );
		}
		else     
		{	
			refmap = fBitmaps[ kTopFace ];
			uv = CompUV( -v.x,  v.y, -v.z );
		}
	}

	if( refmap == NULL )
		color.White();
	else
	{
		if( uv.x < 0.0f )
			uv.x = 0.0f; 
		else if( uv.x > 1.0f )
			uv.x = 1.0f;
		if( uv.y < 0.0f )
			uv.y = 0.0f; 
		else if( uv.y > 1.0f )
			uv.y = 1.0f;
		size = refmap->Width();
		int x = (int)( uv.x * (float)( size - 1 ) );
		int y = (int)( ( 1.0f - uv.y ) * (float)( size - 1 ) );

		BMM_Color_64 c;
		refmap->GetLinearPixels( x, y, 1, &c );
		color = AColor( c.r / 65535.f, c.g / 65535.f, c.b / 65535.f, c.a / 65535.f );
	}

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

	// If RGB output is set to alpha, show RGB as grayscale of the alpha
	if( fBitmapPB->GetInt( kBmpRGBOutput ) == 1 )
		color = AColor( color.a, color.a, color.a, 1.0f );

	sc.PutCache(this, color); 
	return color;
}

float plStaticEnvLayer::EvalMono(ShadeContext& sc)
{
	if (fBitmapPB->GetInt(kBmpMonoOutput) == 1)
		return EvalColor(sc).a;

	return Intens(EvalColor(sc));
}

Point3 plStaticEnvLayer::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plStaticEnvLayer::LocalRequirements(int subMtlNum)
{
	if( fBitmapPB->GetInt( kBmpUseMAXAtmosphere ) )
		return MTLREQ_VIEW_DEP;

	return MTLREQ_VIEW_DEP | MTLREQ_NOATMOS;
}

void plStaticEnvLayer::IDiscardTexHandle() 
{
	if (fTexHandle)
	{
		fTexHandle->DeleteThis();
		fTexHandle = NULL;
	}
}

void plStaticEnvLayer::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff)
		IDiscardTexHandle();
}

BITMAPINFO *plStaticEnvLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
						// FIXME
	fTexTime = 0;//CalcFrame(t);
//	texValid = clipValid;
	BITMAPINFO *bmi = NULL;
	int xflags = 0;

	if (fBitmapPB->GetInt(kBmpRGBOutput) == 1)
		xflags |= EX_RGB_FROM_ALPHA;
	bmi = thmaker.BitmapToDIB(fBitmaps[ 0 ], fUVGen->SymFlags(), xflags, forceW, forceH);

	return bmi;
}

DWORD plStaticEnvLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) 
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

const char *plStaticEnvLayer::GetTextureName( int which )
{
//	if (fBitmapPB->GetInt(kBmpUseBitmap))
	{
		PBBitmap *pbbm = fBitmapPB->GetBitmap( kBmpFrontBitmap + which );
		if (pbbm)
			return pbbm->bi.Name();
	}

	return NULL;
}

//// Set/GetBaseFilename //////////////////////////////////////////////////////

void	plStaticEnvLayer::SetBaseFilename( const TCHAR *name, TimeValue t )
{
	fBitmapPB->SetValue( kBmpBaseFilename, t, (TCHAR *)name );
}

const TCHAR	*plStaticEnvLayer::GetBaseFilename( TimeValue t )
{
	Interval		valid;
	TCHAR			*buffer;

	fBitmapPB->GetValue( kBmpBaseFilename, t, buffer, valid );
	return (const TCHAR *)buffer;
}

//// IGetViewTM ///////////////////////////////////////////////////////////////

Matrix3	plStaticEnvLayer::IGetViewTM( int i )
{
	Matrix3 m;
	m.IdentityMatrix();
	switch( i ) 
	{
		case kTopFace:
			m.RotateX( -PI );	
			break;
		case kBottomFace:
			break;
		case kLeftFace:
			m.RotateX( -.5f * PI );	
			m.RotateY( -.5f * PI );
			break;
		case kRightFace:
			m.RotateX( -.5f * PI );	
			m.RotateY( +.5f * PI );
			break;
		case kFrontFace:
			m.RotateX( -.5f * PI );	
			m.RotateY( PI );
			break;
		case kBackFace:
			m.RotateX( -.5f * PI );	
			break;
	}
	return m;
}

//// IWriteBM /////////////////////////////////////////////////////////////////

int	plStaticEnvLayer::IWriteBM( BitmapInfo *bi, Bitmap *bm, TCHAR *name )
{
	bi->SetName( name );
	if( bm->OpenOutput( bi ) == BMMRES_SUCCESS )
	{
		if( bm->Write( bi, BMM_SINGLEFRAME ) == BMMRES_SUCCESS ) 
		{
			bm->Close( bi );
			return 1;
		}
	}

	return 0;
}

//// RenderCubicMap ///////////////////////////////////////////////////////////
//	Generates the 6 faces for a cubic map based on a picked node

void	plStaticEnvLayer::RenderCubicMap( INode *node )
{
	int			res, size;
	BOOL		success = 0;
	TSTR		fname, fullname;
	Bitmap		*bm = NULL;
	TSTR		path, filename, ext, thisFilename;
	BitmapInfo	biOutFile;

	static TCHAR	suffixes[ 6 ][ 4 ] = { "_FR", "_BK", "_LF", "_RT", "_UP", "_DN" };


	Interface *ip = GetCOREInterface();
	size = fBitmapPB->GetInt( kBmpTextureSize, ip->GetTime() );
	if( size <= 0 ) 
	{
		return;
	}

	thisFilename = fBitmapPB->GetStr( kBmpBaseFilename, ip->GetTime() );
	if( thisFilename.isNull() )
	{
		return;
	}

	SplitFilename( thisFilename, &path, &filename, &ext );

	BOOL	wasHid = node->IsNodeHidden();
	node->Hide( TRUE );

	// Create a blank bitmap
	biOutFile.SetWidth( size );
	biOutFile.SetHeight( size );
	biOutFile.SetType( BMM_TRUE_64 );
	biOutFile.SetAspect( 1.0f );
	biOutFile.SetCurrentFrame( 0 );
	bm = TheManager->Create( &biOutFile );

	Matrix3 nodeTM = node->GetNodeTM( ip->GetTime() );
	Matrix3 tm;	
	INode *root = ip->GetRootNode();		
	bm->Display( GetString( IDS_CUBIC_RENDER_TITLE ) );

	/// Set up rendering contexts
	ViewParams vp;
	vp.projType = PROJ_PERSPECTIVE;
	vp.hither = .001f;
	vp.yon = 1.0e30f;
	vp.fov = PI/2.0f;
	if( fBitmapPB->GetInt( kBmpUseMAXAtmosphere ) )
	{
		vp.nearRange = 0;
		vp.farRange = fBitmapPB->GetFloat( kBmpFarDistance );
	}
	else
	{
		vp.nearRange = vp.farRange = 1.0e30f;
	}
	BOOL	saveUseEnvMap = ip->GetUseEnvironmentMap();
	ip->SetUseEnvironmentMap( false );

	res = ip->OpenCurRenderer( &vp ); 
	for( int i = 0; i < 6; i++ )
	{
		tm = IGetViewTM( i );
		tm.PreTranslate( -nodeTM.GetTrans() ); 
		vp.affineTM = tm;

		// Construct filename
		thisFilename.printf( _T( "%s\\%s%s%s" ), path, filename, suffixes[ i ], ext );

		res = ip->CurRendererRenderFrame( ip->GetTime(), bm, NULL, 1.0f, &vp );
		if( !res ) 
			goto fail;

		if( !IWriteBM( &biOutFile, bm, thisFilename ) ) 
			goto fail;
	}

	success = 1;
fail:
	ip->CloseCurRenderer();	
	ip->SetUseEnvironmentMap( saveUseEnvMap );

	bm->DeleteThis();
	node->Hide( wasHid );
	if( success )
	{
		for(int i = 0; i < 6; i++ )
		{
			BitmapInfo	bi;
			thisFilename.printf( _T( "%s\\%s%s%s" ), path, filename, suffixes[ i ], ext );
			bi.SetName( thisFilename );

			PBBitmap	pbBitmap( bi );
			fBitmapPB->SetValue( kBmpFrontBitmap + i, ip->GetTime(), &pbBitmap );
		}
		fBitmapPB->GetMap()->UpdateUI( ip->GetTime() );
	}
}

PBBitmap *plStaticEnvLayer::GetPBBitmap(int index /* = 0 */)
{ 
	return fBitmapPB->GetBitmap( ParamID( kBmpFrontBitmap + index ) ); 
}

void plStaticEnvLayer::ISetPBBitmap( PBBitmap *pbbm, int index )
{
	fBitmapPB->SetValue( ParamID( kBmpFrontBitmap + index ), 0, pbbm );
}
