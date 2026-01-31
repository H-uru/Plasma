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
#ifndef _plGLPipeline_inc_
#define _plGLPipeline_inc_

#include "plGLDevice.h"
#include "plPipeline/pl3DPipeline.h"
#include "plPipeline/hsG3DDeviceSelector.h"

class plGLEnumerate
{
public:
    plGLEnumerate() {
        hsG3DDeviceSelector::AddDeviceEnumerator(&plGLEnumerate::Enumerate);
    }

private:
    static void Enumerate(std::vector<hsG3DDeviceRecord>& records, hsDisplayHndl display);
};

class plGLPipeline : public pl3DPipeline<plGLDevice>
{
    friend class plGLPlateManager;

public:
    plGLPipeline(hsDisplayHndl display, hsWindowHndl window, const hsG3DDeviceModeRecord *devMode);
    virtual ~plGLPipeline() = default;

    CLASSNAME_REGISTER(plGLPipeline);
    GETINTERFACE_ANY(plGLPipeline, plPipeline);


    /* All of these virtual methods are not implemented by pl3DPipeline and
     * need to be re-implemented here!
     */

    /*** VIRTUAL METHODS ***/
    bool PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;
    bool PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;
    void Render(plDrawable* d, const std::vector<int16_t>& visList) override;
    plTextFont* MakeTextFont(ST::string face, uint16_t size) override;
    void CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    void CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) override;
    bool CloseAccess(plAccessSpan& acc) override;
    void CheckTextureRef(plLayerInterface* lay) override;
    void PushRenderRequest(plRenderRequest* req) override;
    void PopRenderRequest(plRenderRequest* req) override;
    void ClearRenderTarget(plDrawable* d) override;
    void ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) override;
    hsGDeviceRef* MakeRenderTargetRef(plRenderTarget* owner) override;
    void PushRenderTarget(plRenderTarget* target) override;
    plRenderTarget* PopRenderTarget() override;
    bool BeginRender() override;
    bool EndRender() override;
    void RenderScreenElements() override;
    bool IsFullScreen() const override;
    uint32_t ColorDepth() const override;
    void Resize(uint32_t width, uint32_t height) override;
    bool CheckResources() override;
    void LoadResources() override;
    void SetZBiasScale(float scale) override;
    float GetZBiasScale() const override;
    void SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w) override;
    void RefreshScreenMatrices() override;
    void SubmitClothingOutfit(plClothingOutfit* co) override;
    bool SetGamma(float eR, float eG, float eB) override;
    bool SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) override;
    bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) override;
    plMipmap* ExtractMipMap(plRenderTarget* targ) override;
    void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) override;
    int GetMaxAnisotropicSamples() override;
    int GetMaxAntiAlias(int Width, int Height, int ColorDepth) override;
    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync = false  ) override;
    void RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList) override;

private:
    static plGLEnumerate enumerator;
};

#endif // _plGLPipeline_inc_
