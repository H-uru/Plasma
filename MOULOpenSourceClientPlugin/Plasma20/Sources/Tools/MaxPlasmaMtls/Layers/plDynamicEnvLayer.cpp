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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plDynamicEnvLayer - Dynamic EnvironmentMap MAX Layer					 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.22.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plDynamicEnvLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"
#include "resource.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"


//// Externs //////////////////////////////////////////////////////////////////

extern TCHAR *GetString( int id );
extern HINSTANCE hInstance;

//// ClassDesc Definition /////////////////////////////////////////////////////

class plDynamicEnvLayerClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()		{ return TRUE; }
	void*			Create(BOOL loading = FALSE) { return TRACKED_NEW plDynamicEnvLayer(); }
	const TCHAR*	ClassName()		{ return GetString(IDS_DYNAMIC_ENVMAP_LAYER); }
	SClass_ID		SuperClassID()	{ return TEXMAP_CLASS_ID; }
	Class_ID		ClassID()		{ return DYNAMIC_ENV_LAYER_CLASS_ID; }
	const TCHAR* 	Category()		{ return TEXMAP_CAT_ENV; }
	const TCHAR*	InternalName()	{ return _T("PlasmaDynamicEnvMapLayer"); }
	HINSTANCE		HInstance()		{ return hInstance; }
};
static plDynamicEnvLayerClassDesc plDynamicEnvLayerDesc;
ClassDesc2* GetDynamicEnvLayerDesc() { return &plDynamicEnvLayerDesc; }

#include "plDynamicEnvLayerBitmapPB.cpp"

//// Constructor/Destructor ///////////////////////////////////////////////////

plDynamicEnvLayer::plDynamicEnvLayer() :
	fBitmapPB(NULL),
	fUVGen(NULL),
	fTexHandle(NULL),
	fTexTime(0),
	fIValid(NEVER)
{
	plDynamicEnvLayerDesc.MakeAutoParamBlocks(this);
	ReplaceReference(kRefUVGen, GetNewDefaultUVGen());	
}

plDynamicEnvLayer::~plDynamicEnvLayer()
{
	IDiscardTexHandle();
}

void	plDynamicEnvLayer::GetClassName( TSTR& s ) 
{
	s = GetString( IDS_DYNAMIC_ENVMAP_LAYER ); 
}

//// Reset ////////////////////////////////////////////////////////////////////

void plDynamicEnvLayer::Reset() 
{
	GetDynamicEnvLayerDesc()->Reset(this, TRUE);	// reset all pb2's
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	fIValid.SetEmpty();
}

//// Update ///////////////////////////////////////////////////////////////////

void plDynamicEnvLayer::Update(TimeValue t, Interval& valid) 
{
	if (!fIValid.InInterval(t))
	{
		fIValid.SetInfinite();

        fUVGen->Update(t,fIValid);
		fBitmapPB->GetValidity(t, fIValid);
	}

	valid &= fIValid;
}

//// Validity /////////////////////////////////////////////////////////////////

Interval plDynamicEnvLayer::Validity(TimeValue t)
{
	//TODO: Update fIValid here

	// mf horse - Hacking this in just to get animations working.
	// No warranty on this not being stupid.
	Interval v = FOREVER;
	fBitmapPB->GetValidity(t, v);
	v &= fUVGen->Validity(t);
	return v;
}

//// CreateParamDlg ///////////////////////////////////////////////////////////

ParamDlg* plDynamicEnvLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	fIMtlParams = imp;
	IAutoMParamDlg* masterDlg = plDynamicEnvLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

	return masterDlg;	
}

//// SetDlgThing //////////////////////////////////////////////////////////////

BOOL plDynamicEnvLayer::SetDlgThing(ParamDlg* dlg)
{	
	return FALSE;
}

//// Reference Functions //////////////////////////////////////////////////////

int	plDynamicEnvLayer::NumRefs()
{
	return 2;
}

RefTargetHandle	plDynamicEnvLayer::GetReference( int i ) 
{
	switch( i )
	{
		case kRefUVGen:		return fUVGen;
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

void	plDynamicEnvLayer::SetReference( int i, RefTargetHandle rtarg ) 
{
	Interval	garbage;

	switch( i )
	{
		case kRefUVGen:  
			fUVGen = (UVGen *)rtarg; 
			if( fUVGen )
				fUVGen->Update( TimeValue( 0 ), garbage );
			break;
		case kRefBitmap:
			fBitmapPB = (IParamBlock2 *)rtarg;
			break;
	}
}

//// ParamBlock Functions /////////////////////////////////////////////////////

int	plDynamicEnvLayer::NumParamBlocks()
{
	return 1;
}

IParamBlock2	*plDynamicEnvLayer::GetParamBlock( int i )
{
	switch( i )
	{
		case 0:	return fBitmapPB;
		default: return NULL;
	}
}

IParamBlock2	*plDynamicEnvLayer::GetParamBlockByID( BlockID id )
{
	if( fBitmapPB->ID() == id )
		return fBitmapPB;
	else
		return NULL;
}

//// Clone ////////////////////////////////////////////////////////////////////

RefTargetHandle	plDynamicEnvLayer::Clone( RemapDir &remap ) 
{
	plDynamicEnvLayer *mnew = TRACKED_NEW plDynamicEnvLayer();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	mnew->ReplaceReference(kRefBitmap, remap.CloneRef(fBitmapPB));
	mnew->ReplaceReference(kRefUVGen, remap.CloneRef(fUVGen));
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

//// SubAnim Functions ////////////////////////////////////////////////////////

int	plDynamicEnvLayer::NumSubs()
{
	return 1;
}

Animatable	*plDynamicEnvLayer::SubAnim( int i ) 
{
	switch( i )
	{
		case kRefBitmap:	return fBitmapPB;
		default: return NULL;
	}
}

TSTR	plDynamicEnvLayer::SubAnimName( int i ) 
{
	switch( i )
	{
		case kRefBitmap:	return fBitmapPB->GetLocalName();
		default: return "";
	}
}

//// NotifyRefChanged /////////////////////////////////////////////////////////

RefResult	plDynamicEnvLayer::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
												   PartID& partID, RefMessage message ) 
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
			}
		}
		break;

		case REFMSG_UV_SYM_CHANGE:
			IDiscardTexHandle();  
			break;
	}

	return REF_SUCCEED;
}

//// Save/Load ////////////////////////////////////////////////////////////////

#define TEX_HDR_CHUNK 0x5000

IOResult plDynamicEnvLayer::Save(ISave *isave) 
{
	IOResult res;

	isave->BeginChunk(TEX_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK)
		return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult plDynamicEnvLayer::Load(ILoad *iload) 
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

//// EvalColor ////////////////////////////////////////////////////////////////

inline Point2 CompUV(float x, float y, float z) 
{
	return Point2( 0.5f * ( x / z + 1.0f ), 0.5f * ( y / z + 1.0f ) );
}

AColor plDynamicEnvLayer::EvalColor(ShadeContext& sc)
{
	if (!sc.doMaps) 
		return AColor(0.0f, 0.0f, 0.0f, 1.0f);

	AColor color;
	if (sc.GetCache(this, color)) 
		return color;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	color.White();

	sc.PutCache(this, color); 
	return color;
}

float plDynamicEnvLayer::EvalMono(ShadeContext& sc)
{
	return Intens(EvalColor(sc));
}

Point3 plDynamicEnvLayer::EvalNormalPerturb(ShadeContext& sc)
{
	// Return the perturbation to apply to a normal for bump mapping
	return Point3(0, 0, 0);
}

ULONG plDynamicEnvLayer::LocalRequirements(int subMtlNum)
{
	return MTLREQ_VIEW_DEP;
}

void plDynamicEnvLayer::IDiscardTexHandle() 
{
	if (fTexHandle)
	{
		fTexHandle->DeleteThis();
		fTexHandle = NULL;
	}
}

void plDynamicEnvLayer::ActivateTexDisplay(BOOL onoff)
{
	if (!onoff)
		IDiscardTexHandle();
}

BITMAPINFO *plDynamicEnvLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
	return NULL;
}

DWORD plDynamicEnvLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) 
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

//// MustBeUnique /////////////////////////////////////////////////////////////
//	Fun stuff here. If our anchor is set to nil (i.e. "self"), then we must be 
//	unique for each object we're applied to. However, that means the material 
//	must *ALSO* be unique. Hence why this function is called by
//	hsMaterialConverter::IMustBeUniqueMaterial().

bool	plDynamicEnvLayer::MustBeUnique( void )
{
	if( fBitmapPB->GetINode( kBmpAnchorNode ) == nil )
		return true;

	return false;
}

