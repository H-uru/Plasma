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
//  plGLPipeline Class Functions                                             //
//  plPipeline derivative for OpenGL                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsWindows.h"

#include <string_theory/string>

#include "plPipeline/hsWinRef.h"

#include "plGLPipeline.h"
#include "plGLPlateManager.h"

#ifdef HS_SIMD_INCLUDE
#  include HS_SIMD_INCLUDE
#endif

plGLEnumerate plGLPipeline::enumerator;

plGLPipeline::plGLPipeline(hsWindowHndl display, hsWindowHndl window, const hsG3DDeviceModeRecord *devMode)
    : pl3DPipeline(devMode)
{
    fPlateMgr = new plGLPlateManager(this);
}

bool plGLPipeline::PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    return false;
}

bool plGLPipeline::PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    return false;
}

void plGLPipeline::Render(plDrawable* d, const std::vector<int16_t>& visList)
{}

plTextFont* plGLPipeline::MakeTextFont(ST::string face, uint16_t size)
{
    return nullptr;
}

void plGLPipeline::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{}

void plGLPipeline::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{}

bool plGLPipeline::OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly)
{
    return false;
}

bool plGLPipeline::CloseAccess(plAccessSpan& acc)
{
    return false;
}

void plGLPipeline::CheckTextureRef(plLayerInterface* lay)
{}

void plGLPipeline::PushRenderRequest(plRenderRequest* req)
{}

void plGLPipeline::PopRenderRequest(plRenderRequest* req)
{}

void plGLPipeline::ClearRenderTarget(plDrawable* d)
{}

void plGLPipeline::ClearRenderTarget(const hsColorRGBA* col, const float* depth)
{}

hsGDeviceRef* plGLPipeline::MakeRenderTargetRef(plRenderTarget* owner)
{
    return nullptr;
}

void plGLPipeline::PushRenderTarget(plRenderTarget* target)
{}

plRenderTarget* plGLPipeline::PopRenderTarget()
{
    return nullptr;
}

bool plGLPipeline::BeginRender()
{
    return false;
}

bool plGLPipeline::EndRender()
{
    return false;
}

void plGLPipeline::RenderScreenElements()
{}

bool plGLPipeline::IsFullScreen() const
{
    return false;
}

uint32_t plGLPipeline::ColorDepth() const
{
    return 0;
}

void plGLPipeline::Resize(uint32_t width, uint32_t height)
{}

bool plGLPipeline::CheckResources()
{
    return false;
}

void plGLPipeline::LoadResources()
{}

void plGLPipeline::SetZBiasScale(float scale)
{}

float plGLPipeline::GetZBiasScale() const
{
    return 0.0f;
}

void plGLPipeline::SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w)
{}

void plGLPipeline::RefreshScreenMatrices()
{}

void plGLPipeline::SubmitClothingOutfit(plClothingOutfit* co)
{}

bool plGLPipeline::SetGamma(float eR, float eG, float eB)
{
    return false;
}

bool plGLPipeline::SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB)
{
    return false;
}

bool plGLPipeline::CaptureScreen(plMipmap* dest, bool flipVertical, uint16_t desiredWidth, uint16_t desiredHeight)
{
    return false;
}

plMipmap* plGLPipeline::ExtractMipMap(plRenderTarget* targ)
{
    return nullptr;
}

void plGLPipeline::GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth)
{}

int plGLPipeline::GetMaxAnisotropicSamples()
{
    return 0;
}

int plGLPipeline::GetMaxAntiAlias(int Width, int Height, int ColorDepth)
{
    return 0;
}

void plGLPipeline::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync)
{}

void plGLPipeline::RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList)
{}
