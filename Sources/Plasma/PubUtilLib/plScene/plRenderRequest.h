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

#ifndef plRenderRequest_inc
#define plRenderRequest_inc

#include "hsBitVector.h"
#include "hsColorRGBA.h"
#include "hsRefCnt.h"
#include "hsMatrix44.h"
#include "plViewTransform.h"

#include "pnKeyedObject/plKey.h"

#include "plMessage/plRenderRequestMsg.h"

class plDrawable;
class hsGMaterial;
class plPipeline;
class plPageTreeMgr;
class plRenderTarget;
class hsStream;
class hsResMgr;

class plRenderRequest : public plRenderRequestBase
{
public:
protected:
    uint32_t                  fRenderState; // Or'ed from plPipeline::RenderStateSettings::kRender*

    plDrawable*             fClearDrawable;
    plRenderTarget*         fRenderTarget;
    plPageTreeMgr*          fPageMgr;

    hsGMaterial*            fOverrideMat;
    hsGMaterial*            fEraseMat;

    plKey                   fAck;

    float                fPriority;

    uint32_t                  fDrawableMask;
    uint32_t                  fSubDrawableMask;

    hsColorRGBA             fClearColor;
    float                fClearDepth;

    float                fFogStart;

    hsMatrix44              fLocalToWorld;
    hsMatrix44              fWorldToLocal;

    plViewTransform         fViewTransform;

    hsBitVector             fVisForce;

    uint32_t                  fUserData;
    bool                    fIgnoreOccluders;

public:
    plRenderRequest();
    ~plRenderRequest();

    bool            GetRenderSelect() const { return !fVisForce.Empty(); }
    bool            GetRenderCharacters() const;

    void            SetRenderState(uint32_t st) { fRenderState = st; }
    uint32_t          GetRenderState() const { return fRenderState; }

    void            SetDrawableMask(uint32_t m) { fDrawableMask = m; }
    uint32_t          GetDrawableMask() const { return fDrawableMask; }

    void            SetSubDrawableMask(uint32_t m) { fSubDrawableMask = m; }
    uint32_t          GetSubDrawableMask() const { return fSubDrawableMask; }

    void            RequestAck(plKey key) { fAck = std::move(key); }
    plKey           GetAck() const { return fAck; }

    plDrawable*             GetClearDrawable() const { return fClearDrawable; }
    void                    SetClearDrawable(plDrawable* d) { fClearDrawable = d; }

    hsGMaterial*            GetOverrideMat() const { return fOverrideMat; }
    void                    SetOverrideMat(hsGMaterial* m) { fOverrideMat = m; }

    hsGMaterial*            GetEraseMat() const { return fEraseMat; }
    void                    SetEraseMat(hsGMaterial* m) { fEraseMat = m; }

    plRenderTarget*         GetRenderTarget() const { return fRenderTarget; }
    void                    SetRenderTarget(plRenderTarget* t);

    plPageTreeMgr*          GetPageTreeMgr() const { return fPageMgr; }
    void                    SetPageTreeMgr(plPageTreeMgr* mgr) { fPageMgr = mgr; }

    const hsBitVector&      GetVisForce() const { return fVisForce; }
    void                    SetVisForce(const hsBitVector& b);

    const hsMatrix44&   GetLocalToWorld() const { return fLocalToWorld; }
    const hsMatrix44&   GetWorldToLocal() const { return fWorldToLocal; }
    const hsMatrix44&   GetWorldToCamera() const { return fViewTransform.GetWorldToCamera(); }
    const hsMatrix44&   GetCameraToWorld() const { return fViewTransform.GetCameraToWorld(); }

    const plViewTransform&  GetViewTransform() const { return fViewTransform; }

    float GetHither() const { return fViewTransform.GetHither(); }
    float GetYon() const { return fViewTransform.GetYon(); }

    float GetFovX() const { return fViewTransform.GetFovXDeg(); }
    float GetFovY() const { return fViewTransform.GetFovYDeg(); }

    float GetSizeX() const { return fViewTransform.GetOrthoWidth(); }
    float GetSizeY() const { return fViewTransform.GetOrthoHeight(); }

    uint16_t GetScreenWidth() const { return fViewTransform.GetScreenWidth(); }
    uint16_t GetScreenHeight() const { return fViewTransform.GetScreenHeight(); }

    const hsColorRGBA& GetClearColor() const { return fClearColor; }
    float GetClearDepth() const { return fClearDepth; }
    // FogStart
    // negative => use current settings (default)
    // 0 => no fog == fog starts at yon
    // 1 => fog starts at camera.
    // Fog start greater than 1 is legal. Fog always linear.
    float GetFogStart() const { return fFogStart; }

    float GetPriority() const { return fPriority; }

    void SetLocalTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    void SetViewTransform(const plViewTransform& v) { fViewTransform = v; }

    void SetCameraTransform(const hsMatrix44& w2c, const hsMatrix44& c2w) { fViewTransform.SetCameraTransform(w2c, c2w); }

    void SetPerspective(bool on=true) { fViewTransform.SetPerspective(on); }
    void SetOrthogonal(bool on=true) { fViewTransform.SetOrthogonal(on); }

    void SetHither(float f) { fViewTransform.SetHither(f); }
    void SetYon(float f) { fViewTransform.SetYon(f); }
    
    void SetFovX(float f) { fViewTransform.SetFovXDeg(f); }
    void SetFovY(float f) { fViewTransform.SetFovYDeg(f); }

    void SetSizeX(float f) { fViewTransform.SetWidth(f); }
    void SetSizeY(float f) { fViewTransform.SetHeight(f); }

    void SetClearColor(const hsColorRGBA& c) { fClearColor = c; }
    void SetClearDepth(float d) { fClearDepth = d; }
    // FogStart
    // negative => use current settings (default)
    // 0 => no fog == fog starts at yon
    // 1 => fog starts at camera.
    // Fog start greater than 1 is legal. Fog always linear.
    void SetFogStart(float d) { fFogStart = d; } 

    void SetPriority(float p) { fPriority = p; }

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);

    void SetUserData(uint32_t n) { fUserData = n; }
    uint32_t GetUserData() const { return fUserData; }
    
    void SetIgnoreOccluders(bool b) { fIgnoreOccluders = b; }
    bool GetIgnoreOccluders() { return fIgnoreOccluders; }

    // This function is called after the render request is processed by the client
    virtual void    Render(plPipeline* pipe, plPageTreeMgr* pageMgr);
};

#endif // plRenderRequest_inc
