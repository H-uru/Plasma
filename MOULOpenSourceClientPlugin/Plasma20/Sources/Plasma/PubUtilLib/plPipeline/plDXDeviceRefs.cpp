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
//	plDXDeviceRefs.cpp - Functions for the various DX DeviceRef classes		 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	4.25.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include <d3d9.h>
#include <ddraw.h>

#include "hsWinRef.h"

#include "plDXPipeline.h"
#include "plDXDeviceRef.h"
#include "plDXBufferRefs.h"
#include "plDXLightRef.h"
#include "plDXTextureRef.h"
#include "plDXRenderTargetRef.h"
#include "plGBufferGroup.h"
#include "../plDrawable/plGeometrySpan.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plGLight/plLightInfo.h"
#include "plRenderTarget.h"
#include "plCubicRenderTarget.h"
#include "plDynamicEnvMap.h"

#include "plProfile.h"
#include "../plStatusLog/plStatusLog.h"

plProfile_CreateMemCounter("Vertices", "Memory", MemVertex);
plProfile_CreateMemCounter("Indices", "Memory", MemIndex);
plProfile_CreateMemCounter("Textures", "Memory", MemTexture);

///////////////////////////////////////////////////////////////////////////////
//// Generic plDXDeviceRef Functions /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plDXDeviceRef::plDXDeviceRef()
{
	fNext = nil;
	fBack = nil;
}

plDXDeviceRef::~plDXDeviceRef()
{
	if( fNext != nil || fBack != nil )
		Unlink();
}

void	plDXDeviceRef::Unlink( void )
{
	hsAssert( fBack, "plDXDeviceRef not in list" );
	if( fNext )
		fNext->fBack = fBack;
	*fBack = fNext;

	fBack = nil;
	fNext = nil;
}

void	plDXDeviceRef::Link( plDXDeviceRef **back )
{
	hsAssert( fNext == nil && fBack == nil, "Trying to link a plDXDeviceRef that's already linked" );

	fNext = *back;
	if( *back )
		(*back)->fBack = &fNext;
	fBack = back;
	*back = this;
}


///////////////////////////////////////////////////////////////////////////////
//// plDXVertex/IndexBufferRef Funktions /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Destructors //////////////////////////////////////////////////////////////

plDXVertexBufferRef::~plDXVertexBufferRef()
{
	Release();
}

plDXIndexBufferRef::~plDXIndexBufferRef()
{
	Release();
}

//// Releases /////////////////////////////////////////////////////////////////

void	plDXVertexBufferRef::Release( void )
{
	if( fD3DBuffer != nil )
	{
		ReleaseObject(fD3DBuffer);
		if (!Volatile())
		{
			plProfile_DelMem(MemVertex, fCount * fVertexSize);
			PROFILE_POOL_MEM(D3DPOOL_MANAGED, fCount * fVertexSize, false, "VtxBuff");
			plDXPipeline::FreeManagedVertex(fCount * fVertexSize);
		}
	}
	delete [] fData;
	fData = nil;

	SetDirty( true );
}

void	plDXIndexBufferRef::Release( void )
{
	if( fD3DBuffer != nil )
	{
		plProfile_DelMem(MemIndex, fCount * sizeof(UInt16));
		PROFILE_POOL_MEM(fPoolType, fCount * sizeof(UInt16), false, "IndexBuff");
		ReleaseObject( fD3DBuffer );
	}

	SetDirty( true );
}

///////////////////////////////////////////////////////////////////////////////
//// plDXTextureRef Funktions ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Set //////////////////////////////////////////////////////////////////////

plDXTextureRef& plDXTextureRef::Set( D3DFORMAT ft, UInt32 ml, UInt32 mw, UInt32 mh, UInt32 np, 
													 UInt32 sz, UInt32 manSize, UInt32* lSz, void* pd, hsBool ed, hsBool renderTarget )
{
	if( fDataSize > 0 )
		plProfile_DelMem(MemTexture, fDataSize + sizeof(plDXTextureRef));

	if( ( fFormatType != ft || fMMLvs != ml || fMaxWidth != mw || fMaxHeight != mh ) && fD3DTexture != nil )
		ReleaseObject( fD3DTexture );
	if( !fD3DTexture )
		fUseTime = 0;

	fFormatType = ft;
	fMMLvs      = ml;
	fMaxWidth   = mw;
	fMaxHeight  = mh;
	fNumPix		= np;
	fDataSize	= manSize;
	if( fLevelSizes != nil )
		delete [] fLevelSizes;
	if( lSz )
		fLevelSizes	= lSz;
	else
	{
		fLevelSizes = TRACKED_NEW UInt32[1];
		fLevelSizes[0] = sz;
	}
	fData       = pd;
	fFlags = ( ed ? kExternData : 0 ) | ( renderTarget ? kRenderTarget : 0 );

	plProfile_NewMem(MemTexture, fDataSize + sizeof(plDXTextureRef));

	return *this;
}

//// Constructor & Destructor /////////////////////////////////////////////////

plDXTextureRef::plDXTextureRef( D3DFORMAT ft, UInt32 ml, UInt32 mw, UInt32 mh, UInt32 np, 
											    UInt32 sz, UInt32 manSize, UInt32* lSz, void* pd, hsBool ed, hsBool renderTarget )
{
	fLevelSizes = nil;
	fOwner = nil;
	fD3DTexture = nil;
	fDataSize = 0;
	fFlags = 0;
	fFormatType = D3DFMT_UNKNOWN;
	fMMLvs = 0;
	fMaxWidth = 0;
	fMaxHeight = 0;
	Set( ft, ml, mw, mh, np, sz, manSize, lSz, pd, ed, renderTarget );
}

plDXTextureRef::~plDXTextureRef() 
{
	Release();

	delete [] fLevelSizes; 
}

//// Release //////////////////////////////////////////////////////////////////

void	plDXTextureRef::Release( void )
{
	plProfile_DelMem(MemTexture, fDataSize + sizeof(plDXTextureRef));
	plProfile_Extern(ManagedMem);
	PROFILE_POOL_MEM(D3DPOOL_MANAGED, fDataSize, false, (fOwner ? fOwner->GetKey() ? fOwner->GetKey()->GetUoid().GetObjectName() : "(UnknownTexture)" : "(UnknownTexture)"));
	plDXPipeline::FreeManagedTexture(fDataSize);
	fDataSize = 0;

	ReleaseObject( fD3DTexture );
	SetDirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//// plDXLightRef Funktions //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// UpdateD3DInfo ////////////////////////////////////////////////////////////

#define SET_D3DCOLORVALUE( v, color ) { v.r = color.r; v.g = color.g; v.b = color.b; v.a = color.a; }

void	plDXLightRef::UpdateD3DInfo( IDirect3DDevice9 *dev, plDXLightSettings *settings )
{
	plDirectionalLightInfo	*dirOwner;
	plOmniLightInfo			*omniOwner;
	plSpotLightInfo			*spotOwner;
	const float				maxRange = 32767.f;

	
	/// Properties that are set for all types
	fD3DDevice = dev;
	fParentSettings = settings;

	memset( &fD3DInfo, 0, sizeof( D3DLIGHT9 ) );
	SET_D3DCOLORVALUE( fD3DInfo.Diffuse, fOwner->GetDiffuse() );
	SET_D3DCOLORVALUE( fD3DInfo.Ambient, fOwner->GetAmbient() );
	SET_D3DCOLORVALUE( fD3DInfo.Specular, fOwner->GetSpecular() );

	if( ( omniOwner = plOmniLightInfo::ConvertNoRef( fOwner ) ) != nil )
	{
		fD3DInfo.Type = D3DLIGHT_POINT;

		hsPoint3 position = omniOwner->GetWorldPosition();
		fD3DInfo.Position.x = position.fX;
		fD3DInfo.Position.y = position.fY;
		fD3DInfo.Position.z = position.fZ;

		if( omniOwner->GetRadius() == 0 )
			fD3DInfo.Range = maxRange;
		else
			fD3DInfo.Range = omniOwner->GetRadius();
		fD3DInfo.Attenuation0 = omniOwner->GetConstantAttenuation();
		fD3DInfo.Attenuation1 = omniOwner->GetLinearAttenuation();
		fD3DInfo.Attenuation2 = omniOwner->GetQuadraticAttenuation();

		// If the light is a spot, but it has a projected texture, then 
		// the cone attenuation is handled by the texture. We're only using
		// the D3D light for distance attenuation and the N*L term. So
		// we can just leave the D3D light as the cheaper and more stable 
		// Omni light. This sort of obviates the change below. - mf
		if( !omniOwner->GetProjection()
			&& (spotOwner = plSpotLightInfo::ConvertNoRef(fOwner)) )
		{
			fD3DInfo.Type = D3DLIGHT_SPOT;

			hsVector3 direction = spotOwner->GetWorldDirection();
			fD3DInfo.Direction.x = direction.fX;
			fD3DInfo.Direction.y = direction.fY;
			fD3DInfo.Direction.z = direction.fZ;

			fD3DInfo.Falloff = spotOwner->GetFalloff();
			fD3DInfo.Theta = spotOwner->GetSpotInner() * 2;
//			fD3DInfo.Phi = spotOwner->GetProjection() ? hsScalarPI : spotOwner->GetSpotOuter() * 2;
			// D3D doesn't seem to like a Phi of PI, even though that's supposed to be the
			// largest legal value. Symptom is an erratic, intermitant, unpredictable failure
			// of the light to light, with bizarreness like lighting one object but not the object
			// next to it, alternating which object it fails on each frame (or less often).
			// So, whatever. - mf
			fD3DInfo.Phi = spotOwner->GetSpotOuter() * 2; 
		}
	}
	else if( ( dirOwner = plDirectionalLightInfo::ConvertNoRef( fOwner ) ) != nil )
	{	
		fD3DInfo.Type = D3DLIGHT_DIRECTIONAL;

		hsVector3 direction = dirOwner->GetWorldDirection();
		fD3DInfo.Direction.x = direction.fX;
		fD3DInfo.Direction.y = direction.fY;
		fD3DInfo.Direction.z = direction.fZ;
	}
	else
	{
		hsAssert( false, "Unrecognized light type passed to plDXLightRef::UpdateD3DInfo()" );
		return;
	}

	fD3DDevice->SetLight( fD3DIndex, &fD3DInfo );
	fScale = 1.f;
}

//// Destructor ///////////////////////////////////////////////////////////////

plDXLightRef::~plDXLightRef()
{
	Release();
}

//// Release //////////////////////////////////////////////////////////////////

void	plDXLightRef::Release( void )
{
	// Ensure that this light is disabled
	if( fD3DDevice )
	{
		fD3DDevice->LightEnable( fD3DIndex, false );
		fD3DDevice = nil;
	}

	if( fParentSettings )
	{
		fParentSettings->fEnabledFlags.SetBit( fD3DIndex, false );
		fParentSettings->ReleaseD3DIndex( fD3DIndex );
		fParentSettings = nil;
	}
	fD3DIndex = 0;

	SetDirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//// plDXRenderTargetRef Functions ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Constructor //////////////////////////////////////////////////////////////

plDXRenderTargetRef::plDXRenderTargetRef( D3DFORMAT tp, UInt32 ml, plRenderTarget *owner, hsBool releaseDepthOnDelete )
					: plDXTextureRef( tp, ml, owner->GetWidth(), owner->GetHeight(),
										owner->GetWidth() * owner->GetHeight(),
										owner->GetWidth() * owner->GetHeight() * ( owner->GetPixelSize() >> 3 ),
										0,
										nil, 
										nil, true, true )
{
	fD3DColorSurface = nil;
	fD3DDepthSurface = nil;
	fReleaseDepth = releaseDepthOnDelete;
	fOwner = owner;

	if( owner->GetFlags() & plRenderTarget::kIsTexture )
		fFlags |= kOffscreenRT;

	if( owner->GetFlags() & plRenderTarget::kIsProjected )
	{
		if( owner->GetFlags() & plRenderTarget::kIsOrtho )
			fFlags |= kOrthoProjection;
		else
			fFlags |= kPerspProjection;
	}
	
	if( plCubicRenderTarget::ConvertNoRef( owner ) != nil )
		fFlags |= kCubicMap;
}

//// Set //////////////////////////////////////////////////////////////////////

plDXRenderTargetRef& plDXRenderTargetRef::Set( D3DFORMAT tp, UInt32 ml, plRenderTarget *owner )
{
	fOwner = owner;

	plDXTextureRef::Set( tp, ml, owner->GetWidth(), owner->GetHeight(),
								owner->GetWidth() * owner->GetHeight(),
								owner->GetWidth() * owner->GetHeight() * ( owner->GetPixelSize() >> 3 ),
								0,
								nil, 
								nil, true, true );

	if( owner->GetFlags() & plRenderTarget::kIsTexture )
		fFlags |= kOffscreenRT;

	if( owner->GetFlags() & plRenderTarget::kIsProjected )
	{
		if( owner->GetFlags() & plRenderTarget::kIsOrtho )
			fFlags |= kOrthoProjection;
		else
			fFlags |= kPerspProjection;
	}

	if( plCubicRenderTarget::ConvertNoRef( owner ) != nil )
		fFlags |= kCubicMap;

	return *this;
}

//// SetTexture ///////////////////////////////////////////////////////////////

void	plDXRenderTargetRef::SetTexture( IDirect3DSurface9 *surface, IDirect3DSurface9 *depth )
{
	fD3DColorSurface = surface;
	fD3DTexture = nil;
	fD3DDepthSurface = depth;
}

void	plDXRenderTargetRef::SetTexture( IDirect3DTexture9 *surface, IDirect3DSurface9 *depth )
{
	fD3DTexture = surface;
	fD3DColorSurface = nil;
	fD3DDepthSurface = depth;
}

void	plDXRenderTargetRef::SetTexture( IDirect3DCubeTexture9 *surface, IDirect3DSurface9 *depth )
{
	int						i;
	IDirect3DSurface9		*surf;
	plDXRenderTargetRef	*ref;
	plCubicRenderTarget		*cubic;
	D3DCUBEMAP_FACES		faces[ 6 ] = {  D3DCUBEMAP_FACE_NEGATIVE_X,		// Left
											D3DCUBEMAP_FACE_POSITIVE_X,		// Right
											D3DCUBEMAP_FACE_POSITIVE_Z,		// Front
											D3DCUBEMAP_FACE_NEGATIVE_Z,		// Back
											D3DCUBEMAP_FACE_POSITIVE_Y,		// Top
											D3DCUBEMAP_FACE_NEGATIVE_Y };	// Bottom


	fD3DTexture = surface;
	fD3DDepthSurface = depth;
	fD3DColorSurface = nil;

	/// Get the faces and assign to each of the child targets
	cubic = plCubicRenderTarget::ConvertNoRef( fOwner );
	for( i = 0; i < 6; i++ )
	{
		if( surface->GetCubeMapSurface( faces[ i ], 0, &surf ) != D3D_OK )
		{
			hsAssert( false, "Unable to get cube map surface" );
			continue;
		}
		ref = (plDXRenderTargetRef *)cubic->GetFace( i )->GetDeviceRef();

		ref->SetTexture( surf, depth );
	}
}

//// Destructor ///////////////////////////////////////////////////////////////

plDXRenderTargetRef::~plDXRenderTargetRef()
{
	Release();
}

//// Release //////////////////////////////////////////////////////////////////

void	plDXRenderTargetRef::Release( void )
{
	int						i;
	plCubicRenderTarget		*cubic;
	plDXRenderTargetRef	*ref;


	/// Get rid of the children's deviceRefs
	if( fFlags & kCubicMap )
	{
		cubic = plCubicRenderTarget::ConvertNoRef( fOwner );
		for( i = 0; i < 6; i++ )
		{
			ref = (plDXRenderTargetRef *)cubic->GetFace( i )->GetDeviceRef();
			ref->Release();
			ref->SetDirty( true );
		}
		// No need to call D3DSURF_MEMDEL on our fD3DTexture. It'll get
		// accounted for by our children's surfaces.
	}
	else
	{
		// We do internal accounting here. Actual release of fD3DTexture
		// happens in plDXTextureRef::Release()
 		D3DSURF_MEMDEL((IDirect3DTexture9*)fD3DTexture);
	}

	D3DSURF_MEMDEL(fD3DColorSurface);
	ReleaseObject( fD3DColorSurface );
	if( fReleaseDepth )
	{
		// TODO:
		// We don't know who all is sharing this depth surface, so we can't
		// confidently say this memory is free now. We're reffing and releasing
		// it properly as far as DirectX is concerned, but our internal memory
		// counter is ignoring it.
		//D3DSURF_MEMDEL(fD3DDepthSurface);
		ReleaseObject( fD3DDepthSurface );
	}

	plDXTextureRef::Release();

	SetDirty( true );
}

