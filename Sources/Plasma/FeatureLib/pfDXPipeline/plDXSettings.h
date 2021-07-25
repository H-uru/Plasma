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

#include <vector>
#include <d3d9.h>

#include "hsBitVector.h"
#include "hsColorRGBA.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsPoint2.h"
#include "hsPoolVector.h"
#include "plViewTransform.h"

#include "plPipeline/plFogEnvironment.h"
#include "plPipeline/plStencil.h"
#include "plPipeline/plCullTree.h"
#include "plPipeline/hsWinRef.h"

//// General Settings /////////////////////////////////////////////////////////

class plRenderRequest;
class plRenderTarget;
struct IDirect3DSurface9;
class plDXDeviceRef;
class plDXVertexBufferRef;
class plDXIndexBufferRef;

class plDXGeneralSettings
{
    public:

        bool                    fFullscreen;
        uint8_t                   fNumAASamples;
        uint32_t                  fD3DCaps, fBoardKluge, fStageEnd;
        int32_t                   fBoundsDrawLevel;
        DWORD                   fClearColor;
        uint8_t                   fMaxAnisotropicSamples;
        D3DPRESENT_PARAMETERS   fPresentParams;
        bool                    fVeryAnnoyingTextureInvalidFlag;
        bool                    fNoGammaCorrect;
        int                     fMaxUVWSrc;
        bool                    fCantProj;
        bool                    fLimitedProj;
        bool                    fBadManaged;
        bool                    fShareDepth;
        bool                    fCurrAnisotropy;
        bool                    fIsIntel;

        plDXVertexBufferRef         *fCurrVertexBuffRef;
        plDXIndexBufferRef              *fCurrIndexBuffRef;

        IDirect3DVertexShader9          *fCurrVertexShader;
        IDirect3DPixelShader9           *fCurrPixelShader;
        DWORD                           fCurrFVFFormat;

        hsCOMError              fDXError;
        char                    fErrorStr[ 256 ];

        void    Reset();
};

//// Fog Settings /////////////////////////////////////////////////////////////

class plDXFogSettings
{
    public:
        const plFogEnvironment*   fEnvPtr;        // nil means no fog
        D3DFOGMODE          fMode;
        uint8_t               fIsVertex;
        uint8_t               fIsShader;
        uint32_t              fHexColor;
        float               fStart;
        float               fEnd;
        float               fDensity;
        hsColorRGBA         fColor;

        void    Reset()
        {
            fEnvPtr = nullptr;
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
        std::vector<plLightInfo*> fProjEach;
        std::vector<plLightInfo*> fProjAll;

        hsPoolVector<plDXLightRef*> fShadowLights;

        plDXLightSettings()
            : fActiveList(), fRefList(), fPipeline(),
              fNextIndex(), fLastIndex(), fTime()
        { }

        // Sets member variables to initial states. Does NOT release anything.
        void    Reset( plDXPipeline *pipe );
        // Releases/deletes anything associated with these settings
        void    Release();
        // Reserve a D3D light index
        uint32_t  ReserveD3DIndex();
        // Release a reserved D3D light index
        void    ReleaseD3DIndex( uint32_t idx );
};

//// Stencil Settings /////////////////////////////////////////////////////////

class plDXStencilSettings
{
    public:
        uint8_t   fDepth;
        bool    fEnabled;
        uint8_t   fCmpFunc;
        uint8_t   fFailOp, fPassOp, fPassButZFailOp;
        uint32_t  fRefValue;
        uint32_t  fMask;
        uint32_t  fWriteMask;

        void    Reset()
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
