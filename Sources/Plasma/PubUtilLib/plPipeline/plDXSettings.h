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
//  plDXSettings - Header for the various settings groups for plDXPipeline //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  4.25.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXSettings_h
#define _plDXSettings_h

#include "hsMatrix44.h"
#include "plFogEnvironment.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "hsColorRGBA.h"
#include "hsBitVector.h"
#include "plStencil.h"
#include "hsPoint2.h"
#include "plCullTree.h"
#include "hsWinRef.h"
#include "plViewTransform.h"

//// General Settings /////////////////////////////////////////////////////////

class plRenderRequest;
class plRenderTarget;
struct IDirect3DSurface9;
class plDXDeviceRef;
class plDXVertexBufferRef;
class plDXIndexBufferRef;

class plDXViewSettings
{
public:
    uint32_t                  fRenderState;

    plRenderRequest*        fRenderRequest;

    uint32_t                  fDrawableTypeMask;
    uint32_t                  fSubDrawableTypeMask;

    DWORD                   fClearColor;
    float                   fClearDepth;

    plFogEnvironment        fDefaultFog;

    plCullTree              fCullTree;
    hsBool                  fCullTreeDirty;
    uint16_t                  fCullMaxNodes;

    enum XformResets
    {
        kResetProjection    = 0x01,
        kResetCamera        = 0x02,
        kResetL2W           = 0x04,

        kResetAll           = 0x07
    };

    uint8_t                   fXformResetFlags;
    hsBool                  fLocalToWorldLeftHanded;
    hsBool                  fWorldToCamLeftHanded;

    mutable hsVector3       fDirection;
    mutable hsVector3       fUp;
    mutable hsVector3       fAcross;
    hsPoint3                fWorldPos;

    mutable hsBool          fViewVectorsDirty;

    hsMatrix44              fLocalToWorld;
    hsMatrix44              fWorldToLocal;

    const hsMatrix44&       GetLocalToWorld() const { return fLocalToWorld; }
    const hsMatrix44&       GetWorldToLocal() const { return fWorldToLocal; }

    plViewTransform         fTransform;

    const hsMatrix44&       GetWorldToCamera() const { return fTransform.GetWorldToCamera(); }
    const hsMatrix44&       GetCameraToWorld() const { return fTransform.GetCameraToWorld(); }
    hsBool                  IsPerspective() const { return fTransform.GetPerspective(); }

    void            Reset();
};

class plDXGeneralSettings
{
    public:

        hsBool                  fFullscreen;
        hsWinRef                fHWnd;
        uint32_t                  fColorDepth;
        uint8_t                   fNumAASamples;
        uint32_t                  fD3DCaps, fBoardKluge, fStageEnd;
        uint32_t                  fMaxNumLights;
        uint32_t                  fMaxNumProjectors;
        uint32_t                  fMaxLayersAtOnce;
        uint32_t                  fMaxPiggyBacks;
        int32_t                   fBoundsDrawLevel;
        uint32_t                  fProperties;
        DWORD                   fClearColor;
        uint8_t                   fMaxAnisotropicSamples;
        D3DPRESENT_PARAMETERS   fPresentParams;
        hsBool                  fVeryAnnoyingTextureInvalidFlag;
        hsBool                  fNoGammaCorrect;
        int                     fMaxUVWSrc;
        hsBool                  fCantProj;
        hsBool                  fLimitedProj;
        hsBool                  fBadManaged;
        hsBool                  fShareDepth;
        hsBool                  fCurrAnisotropy;
        hsBool                  fIsIntel;

        IDirect3DSurface9       *fCurrD3DMainSurface;
        IDirect3DSurface9       *fCurrD3DDepthSurface;

        hsTArray<plDXViewSettings>      fViewStack; // One for the main view, then one for each rendertarget
        hsTArray<plRenderTarget *>      fRenderTargets;
        plRenderTarget                  *fCurrRenderTarget;
        plRenderTarget                  *fCurrBaseRenderTarget;
        plDXDeviceRef                   *fCurrRenderTargetRef;
        plDXVertexBufferRef         *fCurrVertexBuffRef;
        plDXIndexBufferRef              *fCurrIndexBuffRef;
        uint32_t                          fOrigWidth, fOrigHeight;

        IDirect3DVertexShader9          *fCurrVertexShader;
        IDirect3DPixelShader9           *fCurrPixelShader;
        DWORD                           fCurrFVFFormat;

        HRESULT                 fDXError;
        char                    fErrorStr[ 256 ];

        void    Reset( void );
};

//// Tweak Settings ///////////////////////////////////////////////////////////

class plDXTweakSettings
{
    public:
        float   fDefaultPerspLayerScale;
        float   fPerspLayerScale;
        float   fPerspLayerTrans;
        float   fDefaultLODBias;
        float   fFogExpApproxStart;
        float   fFogExp2ApproxStart;
        float   fFogEndBias;

        float   fExp2FogKnee;
        float   fExp2FogKneeVal;
        float   fExpFogKnee;
        float   fExpFogKneeVal;
        
        void    Reset( void )
        {
            fDefaultPerspLayerScale = 0.00001f;
            fPerspLayerScale = 0.00001f;
            fPerspLayerTrans = 0.00002f;
            fDefaultLODBias = -0.25f;
            fFogExpApproxStart = 0.0f;
            fFogExp2ApproxStart = 0.0f;
            fFogEndBias = 0.0f;

            fExpFogKnee = fExp2FogKnee = 0.5f;
            fExpFogKneeVal = fExp2FogKneeVal = 0.15f;
        }
};

//// Fog Settings /////////////////////////////////////////////////////////////

class plDXFogSettings
{
    public:
        plFogEnvironment*   fEnvPtr;        // nil means no fog
        D3DFOGMODE          fMode;
        uint8_t               fIsVertex;
        uint8_t               fIsShader;
        uint32_t              fHexColor;
        float               fStart;
        float               fEnd;
        float               fDensity;
        hsColorRGBA         fColor;

        void    Reset( void )
        {
            fEnvPtr = nil;
            fMode = D3DFOG_NONE;
            fIsVertex = 0;
            fIsShader = 0;
            fHexColor = 0;
            fStart = fEnd = fDensity = 0.0f;
            fColor.Set( 0, 0, 0, 0 );
        }
};

//// Light Settings ///////////////////////////////////////////////////////////

class plDXLightRef;
class plDXPipeline;
class plDXLightSettings
{
    public:
        hsBitVector             fUsedFlags;
        hsBitVector             fEnabledFlags;
        hsBitVector             fHoldFlags;
        uint32_t                  fNextIndex, fLastIndex;
        uint16_t                  fTime;
        plLightInfo*            fActiveList;
        plDXLightRef*           fRefList;
        plDXPipeline*           fPipeline;
        hsTArray<plLightInfo*>  fProjEach;
        hsTArray<plLightInfo*>  fProjAll;

        hsTArray<plLightInfo*>  fCharLights;
        hsTArray<plLightInfo*>  fVisLights;

        uint32_t                          fNextShadowLight;
        hsTArray<plDXLightRef*>     fShadowLights;

        plDXLightSettings();

        // Sets member variables to initial states. Does NOT release anything.
        void    Reset( plDXPipeline *pipe );
        // Releases/deletes anything associated with these settings
        void    Release( void );
        // Reserve a D3D light index
        uint32_t  ReserveD3DIndex( void );
        // Release a reserved D3D light index
        void    ReleaseD3DIndex( uint32_t idx );
};

//// Stencil Settings /////////////////////////////////////////////////////////

class plDXStencilSettings
{
    public:
        uint8_t   fDepth;
        hsBool  fEnabled;
        uint8_t   fCmpFunc;
        uint8_t   fFailOp, fPassOp, fPassButZFailOp;
        uint32_t  fRefValue;
        uint32_t  fMask;
        uint32_t  fWriteMask;

        void    Reset( void )
        {
            fEnabled = false;
            fCmpFunc = 0;
            fFailOp = fPassOp = fPassButZFailOp = 0;
            fRefValue = 0;
            fMask = 0xffffffff;
            fWriteMask = 0xffffffff;
        }
};

#endif // _plDXSettings_h
