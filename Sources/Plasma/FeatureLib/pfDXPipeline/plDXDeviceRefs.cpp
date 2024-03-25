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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plDXDeviceRefs.cpp - Functions for the various DX DeviceRef classes      //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  4.25.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsWindows.h"

#include <d3d9.h>
#include <ddraw.h>

#include "plPipeline/hsWinRef.h"

#include "plDXPipeline.h"
#include "plDXDeviceRef.h"
#include "plDXBufferRefs.h"
#include "plDXLightRef.h"
#include "plDXTextureRef.h"
#include "plDXRenderTargetRef.h"
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plGeometrySpan.h"
#include "plDrawable/plDrawableSpans.h"
#include "plGLight/plLightInfo.h"
#include "plPipeline/plRenderTarget.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plDynamicEnvMap.h"

#include "plProfile.h"
#include "plStatusLog/plStatusLog.h"

plProfile_CreateMemCounter("Vertices", "Memory", MemVertex);
plProfile_CreateMemCounter("Indices", "Memory", MemIndex);
plProfile_CreateMemCounter("Textures", "Memory", MemTexture);

///////////////////////////////////////////////////////////////////////////////
//// Generic plDXDeviceRef Functions /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plDXDeviceRef::plDXDeviceRef()
{
    fNext = nullptr;
    fBack = nullptr;
}

plDXDeviceRef::~plDXDeviceRef()
{
    if (fNext != nullptr || fBack != nullptr)
        Unlink();
}

void    plDXDeviceRef::Unlink()
{
    hsAssert( fBack, "plDXDeviceRef not in list" );
    if( fNext )
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nullptr;
    fNext = nullptr;
}

void    plDXDeviceRef::Link( plDXDeviceRef **back )
{
    hsAssert(fNext == nullptr && fBack == nullptr, "Trying to link a plDXDeviceRef that's already linked");

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

void    plDXVertexBufferRef::Release()
{
    if (fD3DBuffer != nullptr)
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
    fData = nullptr;

    SetDirty( true );
}

void    plDXIndexBufferRef::Release()
{
    if (fD3DBuffer != nullptr)
    {
        plProfile_DelMem(MemIndex, fCount * sizeof(uint16_t));
        PROFILE_POOL_MEM(fPoolType, fCount * sizeof(uint16_t), false, "IndexBuff");
        ReleaseObject( fD3DBuffer );
    }

    SetDirty( true );
}

///////////////////////////////////////////////////////////////////////////////
//// plDXTextureRef Funktions ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Set //////////////////////////////////////////////////////////////////////

plDXTextureRef& plDXTextureRef::Set( D3DFORMAT ft, uint32_t ml, uint32_t mw, uint32_t mh, uint32_t np, 
                                                     uint32_t sz, uint32_t manSize, uint32_t* lSz, void* pd, bool ed, bool renderTarget )
{
    if( fDataSize > 0 )
        plProfile_DelMem(MemTexture, fDataSize + sizeof(plDXTextureRef));

    if ((fFormatType != ft || fMMLvs != ml || fMaxWidth != mw || fMaxHeight != mh) && fD3DTexture != nullptr)
        ReleaseObject( fD3DTexture );
    if( !fD3DTexture )
        fUseTime = 0;

    fFormatType = ft;
    fMMLvs      = ml;
    fMaxWidth   = mw;
    fMaxHeight  = mh;
    fNumPix     = np;
    fDataSize   = manSize;
    if (fLevelSizes != nullptr)
        delete [] fLevelSizes;
    if( lSz )
        fLevelSizes = lSz;
    else
    {
        fLevelSizes = new uint32_t[1];
        fLevelSizes[0] = sz;
    }
    fData       = pd;
    fFlags = ( ed ? kExternData : 0 ) | ( renderTarget ? kRenderTarget : 0 );

    plProfile_NewMem(MemTexture, fDataSize + sizeof(plDXTextureRef));

    return *this;
}

//// Destructor /////////////////////////////////////////////////

plDXTextureRef::~plDXTextureRef() 
{
    Release();

    delete [] fLevelSizes; 
}

//// Release //////////////////////////////////////////////////////////////////

void    plDXTextureRef::Release()
{
    plProfile_DelMem(MemTexture, fDataSize + sizeof(plDXTextureRef));
    plProfile_Extern(ManagedMem);
    PROFILE_POOL_MEM(D3DPOOL_MANAGED, fDataSize, false, (fOwner ? fOwner->GetKey() ? fOwner->GetKey()->GetUoid().GetObjectName().c_str() : "(UnknownTexture)" : "(UnknownTexture)"));
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

void    plDXLightRef::UpdateD3DInfo( IDirect3DDevice9 *dev, plDXLightSettings *settings )
{
    plDirectionalLightInfo  *dirOwner;
    plOmniLightInfo         *omniOwner;
    plSpotLightInfo         *spotOwner;
    const float             maxRange = 32767.f;

    
    /// Properties that are set for all types
    fD3DDevice = dev;
    fParentSettings = settings;

    memset( &fD3DInfo, 0, sizeof( D3DLIGHT9 ) );
    SET_D3DCOLORVALUE( fD3DInfo.Diffuse, fOwner->GetDiffuse() );
    SET_D3DCOLORVALUE( fD3DInfo.Ambient, fOwner->GetAmbient() );
    SET_D3DCOLORVALUE( fD3DInfo.Specular, fOwner->GetSpecular() );

    if ((omniOwner = plOmniLightInfo::ConvertNoRef(fOwner)) != nullptr)
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
//          fD3DInfo.Phi = spotOwner->GetProjection() ? hsConstants::pi<float> : spotOwner->GetSpotOuter() * 2;
            // D3D doesn't seem to like a Phi of PI, even though that's supposed to be the
            // largest legal value. Symptom is an erratic, intermitant, unpredictable failure
            // of the light to light, with bizarreness like lighting one object but not the object
            // next to it, alternating which object it fails on each frame (or less often).
            // So, whatever. - mf
            fD3DInfo.Phi = spotOwner->GetSpotOuter() * 2; 
        }
    }
    else if ((dirOwner = plDirectionalLightInfo::ConvertNoRef(fOwner)) != nullptr)
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

void    plDXLightRef::Release()
{
    // Ensure that this light is disabled
    if( fD3DDevice )
    {
        fD3DDevice->LightEnable( fD3DIndex, false );
        fD3DDevice = nullptr;
    }

    if( fParentSettings )
    {
        fParentSettings->fEnabledFlags.SetBit( fD3DIndex, false );
        fParentSettings->ReleaseD3DIndex( fD3DIndex );
        fParentSettings = nullptr;
    }
    fD3DIndex = 0;

    SetDirty( true );
}


///////////////////////////////////////////////////////////////////////////////
//// plDXRenderTargetRef Functions ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Constructor //////////////////////////////////////////////////////////////

plDXRenderTargetRef::plDXRenderTargetRef( D3DFORMAT tp, uint32_t ml, plRenderTarget *owner, bool releaseDepthOnDelete )
                    : plDXTextureRef( tp, ml, owner->GetWidth(), owner->GetHeight(),
                                        owner->GetWidth() * owner->GetHeight(),
                                        owner->GetWidth() * owner->GetHeight() * ( owner->GetPixelSize() >> 3 ),
                                        0,
                                        nullptr,
                                        nullptr, true, true)
{
    fD3DColorSurface = nullptr;
    fD3DDepthSurface = nullptr;
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
    
    if (plCubicRenderTarget::ConvertNoRef(owner) != nullptr)
        fFlags |= kCubicMap;
}

//// Set //////////////////////////////////////////////////////////////////////

plDXRenderTargetRef& plDXRenderTargetRef::Set( D3DFORMAT tp, uint32_t ml, plRenderTarget *owner )
{
    fOwner = owner;

    plDXTextureRef::Set( tp, ml, owner->GetWidth(), owner->GetHeight(),
                                owner->GetWidth() * owner->GetHeight(),
                                owner->GetWidth() * owner->GetHeight() * ( owner->GetPixelSize() >> 3 ),
                                0,
                                nullptr,
                                nullptr, true, true);

    if( owner->GetFlags() & plRenderTarget::kIsTexture )
        fFlags |= kOffscreenRT;

    if( owner->GetFlags() & plRenderTarget::kIsProjected )
    {
        if( owner->GetFlags() & plRenderTarget::kIsOrtho )
            fFlags |= kOrthoProjection;
        else
            fFlags |= kPerspProjection;
    }

    if (plCubicRenderTarget::ConvertNoRef(owner) != nullptr)
        fFlags |= kCubicMap;

    return *this;
}

//// SetTexture ///////////////////////////////////////////////////////////////

void    plDXRenderTargetRef::SetTexture( IDirect3DSurface9 *surface, IDirect3DSurface9 *depth )
{
    fD3DColorSurface = surface;
    fD3DTexture = nullptr;
    fD3DDepthSurface = depth;
}

void    plDXRenderTargetRef::SetTexture( IDirect3DTexture9 *surface, IDirect3DSurface9 *depth )
{
    fD3DTexture = surface;
    fD3DColorSurface = nullptr;
    fD3DDepthSurface = depth;
}

void    plDXRenderTargetRef::SetTexture( IDirect3DCubeTexture9 *surface, IDirect3DSurface9 *depth )
{
    int                     i;
    IDirect3DSurface9       *surf;
    plDXRenderTargetRef *ref;
    plCubicRenderTarget     *cubic;
    D3DCUBEMAP_FACES        faces[ 6 ] = {  D3DCUBEMAP_FACE_NEGATIVE_X,     // Left
                                            D3DCUBEMAP_FACE_POSITIVE_X,     // Right
                                            D3DCUBEMAP_FACE_POSITIVE_Z,     // Front
                                            D3DCUBEMAP_FACE_NEGATIVE_Z,     // Back
                                            D3DCUBEMAP_FACE_POSITIVE_Y,     // Top
                                            D3DCUBEMAP_FACE_NEGATIVE_Y };   // Bottom


    fD3DTexture = surface;
    fD3DDepthSurface = depth;
    fD3DColorSurface = nullptr;

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

void    plDXRenderTargetRef::Release()
{
    int                     i;
    plCubicRenderTarget     *cubic;
    plDXRenderTargetRef *ref;


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

