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

#include "plDXDevice.h"
#include "plDXPipeline.h"

#include "plPipeline/plRenderTarget.h"

#include "plDXBufferRefs.h"
#include "plDXTextureRef.h"
#include "plDXLightRef.h"
#include "plDXRenderTargetRef.h"
#include "plDXVertexShader.h"
#include "plDXPixelShader.h"

#include <string_theory/string>

//// Macros for D3D error handling
#define INIT_ERROR_CHECK(cond, errMsg) if (FAILED(fSettings.fDXError = cond)) { return ICreateFail(ST_LITERAL(errMsg)); }

#if 1       // DEBUG
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fPipeline->fSettings.fDXError = cond ) ) { fPipeline->IGetD3DError(); fPipeline->IShowErrorMessage(); }   
#define WEAK_ERROR_CHECK( cond )    STRONG_ERROR_CHECK( cond )
#else
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fPipeline->fSettings.fDXError = cond ) ) { fPipeline->IGetD3DError(); }    
#define WEAK_ERROR_CHECK( cond )    cond
#endif

static D3DMATRIX d3dIdentityMatrix{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};


D3DMATRIX& IMatrix44ToD3DMatrix(D3DMATRIX& dst, const hsMatrix44& src)
{
    if (src.fFlags & hsMatrix44::kIsIdent) {
        dst = d3dIdentityMatrix;
    } else {
        dst.m[0][0] = src.fMap[0][0];
        dst.m[1][0] = src.fMap[0][1];
        dst.m[2][0] = src.fMap[0][2];
        dst.m[3][0] = src.fMap[0][3];

        dst.m[0][1] = src.fMap[1][0];
        dst.m[1][1] = src.fMap[1][1];
        dst.m[2][1] = src.fMap[1][2];
        dst.m[3][1] = src.fMap[1][3];

        dst.m[0][2] = src.fMap[2][0];
        dst.m[1][2] = src.fMap[2][1];
        dst.m[2][2] = src.fMap[2][2];
        dst.m[3][2] = src.fMap[2][3];

        dst.m[0][3] = src.fMap[3][0];
        dst.m[1][3] = src.fMap[3][1];
        dst.m[2][3] = src.fMap[3][2];
        dst.m[3][3] = src.fMap[3][3];
    }

    return dst;
}


plDXDevice::plDXDevice()
:   fD3DDevice(nullptr),
    fD3DMainSurface(nullptr),
    fD3DDepthSurface(nullptr),
    fD3DBackBuff(nullptr),
    fCurrCullMode(D3DCULL_CW)
{
}

void plDXDevice::SetRenderTarget(plRenderTarget* target)
{
    IDirect3DSurface9* main;
    IDirect3DSurface9* depth;
    plDXRenderTargetRef* ref = nullptr;


    if (target != nullptr)
    {
        ref = (plDXRenderTargetRef*)target->GetDeviceRef();
        if (ref == nullptr || ref->IsDirty())
            ref = (plDXRenderTargetRef*)fPipeline->MakeRenderTargetRef(target);
    }

    if (ref == nullptr || ref->GetColorSurface() == nullptr)
    {
        /// Set to main screen
        main = fD3DMainSurface;
        depth = fD3DDepthSurface;
    }
    else
    {
        /// Set to this target
        main = ref->GetColorSurface();
        depth = ref->fD3DDepthSurface;
    }

    if (main != fCurrD3DMainSurface || depth != fCurrD3DDepthSurface)
    {
        fCurrD3DMainSurface = main;
        fCurrD3DDepthSurface = depth;
        fD3DDevice->SetRenderTarget(0, main);
        fD3DDevice->SetDepthStencilSurface(depth);
    }

    fPipeline->IInvalidateState();

    SetViewport();
}

void plDXDevice::SetViewport()
{
    D3DVIEWPORT9 vp = { (DWORD)fPipeline->GetViewTransform().GetViewPortLeft(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortTop(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortWidth(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortHeight(),
                        0.f, 1.f };

    WEAK_ERROR_CHECK(fD3DDevice->SetViewport(&vp));
}


void plDXDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
}

void plDXDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_VIEW, &mat);
}

void plDXDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_WORLD, &mat);
}

ST::string plDXDevice::GetErrorString() const
{
    return fPipeline->fSettings.fErrorStr;
}
