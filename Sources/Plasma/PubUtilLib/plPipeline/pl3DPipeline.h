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
#ifndef _pl3DPipeline_inc_
#define _pl3DPipeline_inc_

#include <stack>

#include "plPipeline.h"
#include "plPipelineViewSettings.h"

#include "plSurface/plLayerInterface.h"

class hsGMaterial;


class pl3DPipeline : public plPipeline
{
protected:
    plPipelineViewSettings              fView;
    std::stack<plPipelineViewSettings>  fViewStack;

    hsBitVector                         fDebugFlags;

    hsGMatState                         fMatOverOn;
    hsGMatState                         fMatOverOff;
    hsTArray<hsGMaterial*>              fOverrideMat;
    hsGMaterial*                        fHoldMat;

public:
    CLASSNAME_REGISTER(pl3DPipeline);
    GETINTERFACE_ANY(pl3DPipeline, plPipeline);

    size_t GetViewStackSize() const { return fViewStack.size(); }


    /*** VIRTUAL METHODS ***/
    //virtual bool PreRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;
    //virtual bool PrepForRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;
    //virtual void Render(plDrawable* d, const hsTArray<int16_t>& visList) = 0;
    //virtual void Draw(plDrawable* d) = 0;
    //virtual plTextFont* MakeTextFont(char* face, uint16_t size) = 0;
    //virtual void CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) = 0;
    //virtual void CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) = 0;
    //virtual bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) = 0;
    //virtual bool CloseAccess(plAccessSpan& acc) = 0;
    //virtual void CheckTextureRef(plLayerInterface* lay) = 0;
    //virtual void SetDefaultFogEnviron(plFogEnvironment* fog) = 0;

    virtual const plFogEnvironment& GetDefaultFogEnviron() const {
        return fView.GetDefaultFog();
    }

    //virtual void RegisterLight(plLightInfo* light) = 0;
    //virtual void UnRegisterLight(plLightInfo* light) = 0;
    //virtual void PushRenderRequest(plRenderRequest* req) = 0;
    //virtual void PopRenderRequest(plRenderRequest* req) = 0;
    //virtual void ClearRenderTarget(plDrawable* d) = 0;
    //virtual void ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) = 0;

    virtual void SetClear(const hsColorRGBA* col=nullptr, const float* depth=nullptr) {
        fView.SetClear(col, depth);
    }

    virtual hsColorRGBA GetClearColor() const {
        return fView.GetClearColor();
    }

    virtual float GetClearDepth() const {
        return fView.GetClearDepth();
    }

    //virtual hsGDeviceRef* MakeRenderTargetRef(plRenderTarget* owner) = 0;
    //virtual void PushRenderTarget(plRenderTarget* target) = 0;
    //virtual plRenderTarget* PopRenderTarget() = 0;
    //virtual bool BeginRender() = 0;
    //virtual bool EndRender() = 0;
    //virtual void RenderScreenElements() = 0;
    //virtual void BeginVisMgr(plVisMgr* visMgr) = 0;
    //virtual void EndVisMgr(plVisMgr* visMgr) = 0;
    //virtual bool IsFullScreen() const = 0;

    virtual uint32_t Width() const {
        return GetViewTransform().GetViewPortWidth();
    }

    virtual uint32_t Height() const {
        return GetViewTransform().GetViewPortHeight();
    }

    //virtual uint32_t ColorDepth() const = 0;
    //virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual bool TestVisibleWorld(const hsBounds3Ext& wBnd) {
        return fView.TestVisibleWorld(wBnd);
    }

    virtual bool TestVisibleWorld(const plSceneObject* sObj);

    virtual bool HarvestVisible(plSpaceTree* space, hsTArray<int16_t>& visList) {
        return fView.HarvestVisible(space, visList);
    }

    virtual bool SubmitOccluders(const hsTArray<const plCullPoly*>& polyList) {
        return fView.SubmitOccluders(polyList);
    }

    virtual void SetDebugFlag(uint32_t flag, bool on) {
        fDebugFlags.SetBit(flag, on);
    }

    virtual bool IsDebugFlagSet(uint32_t flag) const {
        return fDebugFlags.IsBitSet(flag);
    }

    virtual void SetMaxCullNodes(uint16_t n) {
        fView.SetMaxCullNodes(n);
    }

    virtual uint16_t GetMaxCullNodes() const {
        return fView.GetMaxCullNodes();
    }

    //virtual bool CheckResources() = 0;
    //virtual void LoadResources() = 0;
    //virtual void SetProperty(uint32_t prop, bool on) = 0;
    //virtual bool GetProperty(uint32_t prop) const = 0;
    //virtual uint32_t GetMaxLayersAtOnce() const = 0;

    virtual void SetDrawableTypeMask(uint32_t mask) {
        fView.SetDrawableTypeMask(mask);
    }

    virtual uint32_t GetDrawableTypeMask() const {
        return fView.GetDrawableTypeMask();
    }

    virtual void SetSubDrawableTypeMask(uint32_t mask) {
        fView.SetSubDrawableTypeMask(mask);
    }

    virtual uint32_t GetSubDrawableTypeMask() const {
        return fView.GetSubDrawableTypeMask();
    }

    virtual hsPoint3 GetViewPositionWorld() const {
        return GetViewTransform().GetPosition();
    }

    virtual hsVector3 GetViewAcrossWorld() const {
        return GetViewTransform().GetAcross();
    }

    virtual hsVector3 GetViewUpWorld() const {
        return GetViewTransform().GetUp();
    }

    virtual hsVector3 GetViewDirWorld() const {
        return GetViewTransform().GetDirection();
    }

    virtual void GetViewAxesWorld(hsVector3 axes[3]) const {
        axes[0] = GetViewAcrossWorld();
        axes[1] = GetViewUpWorld();
        axes[2] = GetViewDirWorld();
    }

    virtual void GetFOV(float& fovX, float& fovY) const {
        fovX = GetViewTransform().GetFovXDeg();
        fovY = GetViewTransform().GetFovYDeg();
    }

    virtual void SetFOV(float fovX, float fovY) {
        fView.SetFOV(fovX, fovY);
    }

    virtual void GetSize(float& width, float& height) const {
        width = GetViewTransform().GetScreenWidth();
        height = GetViewTransform().GetScreenHeight();
    }

    virtual void SetSize(float width, float height) {
        fView.SetSize(width, height);
    }

    virtual void GetDepth(float& hither, float& yon) const {
        GetViewTransform().GetDepth(hither, yon);
    }

    virtual void SetDepth(float hither, float yon) {
        fView.SetDepth(hither, yon);
    }

    //virtual void SetZBiasScale(float scale) = 0;
    //virtual float GetZBiasScale() const = 0;

    virtual const hsMatrix44& GetWorldToCamera() const {
        return fView.GetWorldToCamera();
    }

    virtual const hsMatrix44& GetCameraToWorld() const {
        return fView.GetCameraToWorld();
    }

    //virtual void SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w) = 0;

    virtual const hsMatrix44& GetWorldToLocal() const {
        return fView.GetWorldToLocal();
    }

    virtual const hsMatrix44& GetLocalToWorld() const {
        return fView.GetLocalToWorld();
    }

    virtual const plViewTransform& GetViewTransform() const {
        return fView.GetConstViewTransform();
    }

    //virtual void ScreenToWorldPoint(int n, uint32_t stride, int32_t* scrX, int32_t* scrY, float dist, uint32_t strideOut, hsPoint3* worldOut) = 0;
    //virtual void RefreshMatrices() = 0;
    //virtual void RefreshScreenMatrices() = 0;
    //virtual hsGMaterial* PushOverrideMaterial(hsGMaterial* mat) = 0;
    //virtual void PopOverrideMaterial(hsGMaterial* restore) = 0;

    virtual hsGMaterial* GetOverrideMaterial() const {
        return fOverrideMat.GetCount() ? fOverrideMat.Peek() : nullptr;
    }

    //virtual plLayerInterface* AppendLayerInterface(plLayerInterface* li, bool onAllLayers = false) = 0;
    //virtual plLayerInterface* RemoveLayerInterface(plLayerInterface* li, bool onAllLayers = false) = 0;

    virtual uint32_t GetMaterialOverrideOn(hsGMatState::StateIdx category) const {
        return fMatOverOn.Value(category);
    }

    virtual uint32_t GetMaterialOverrideOff(hsGMatState::StateIdx category) const {
        return fMatOverOff.Value(category);
    }

    //virtual hsGMatState PushMaterialOverride(const hsGMatState& state, bool on) = 0;
    //virtual hsGMatState PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on) = 0;
    //virtual void PopMaterialOverride(const hsGMatState& restore, bool on) = 0;

    virtual const hsGMatState& GetMaterialOverride(bool on) const {
        return on ? fMatOverOn : fMatOverOff;
    }

    //virtual void SubmitShadowSlave(plShadowSlave* slave) = 0;
    //virtual void SubmitClothingOutfit(plClothingOutfit* co) = 0;
    //virtual bool SetGamma(float eR, float eG, float eB) = 0;
    //virtual bool SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) = 0;
    //virtual bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) = 0;
    //virtual plMipmap* ExtractMipMap(plRenderTarget* targ) = 0;
    //virtual const char* GetErrorString() = 0;
    //virtual void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) = 0;
    //virtual int GetMaxAnisotropicSamples() = 0;
    //virtual int GetMaxAntiAlias(int Width, int Height, int ColorDepth) = 0;
    //virtual void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync = false  ) = 0;
};

#endif //_pl3DPipeline_inc_
