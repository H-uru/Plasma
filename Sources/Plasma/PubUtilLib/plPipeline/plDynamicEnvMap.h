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

#ifndef plDynamicEnvMap_inc
#define plDynamicEnvMap_inc

#include "plCubicRenderTarget.h"
#include "plScene/plRenderRequest.h"
#include "hsBitVector.h"

class plRenderRequestMsg;
class hsStream;
class plMessage;
class plVisRegion;
class plGenRefMsg;
class hsResMgr;
class plCameraModifier1;
class plSceneObject;
class plBitmap;
class plLayer;

class plDynamicEnvMap : public plCubicRenderTarget
{
public:
    enum {
        kRefVisSet,
        kRefRootNode,
    };
protected:

    plRenderRequest             fReqs[6];
    plRenderRequestMsg*         fReqMsgs[6];

    plSceneObject*              fRootNode;
    hsPoint3                    fPos;
    float                    fHither;
    float                    fYon;
    float                    fFogStart;
    hsColorRGBA                 fColor;

    float                    fRefreshRate;
    double                      fLastRefresh;
    int                         fLastRender;
    int                         fOutStanding;

    hsBitVector                 fVisSet;
    hsTArray<plVisRegion*>      fVisRegions;
    hsTArray<char *>            fVisRegionNames;
    bool                        fIncCharacters;

    void    IUpdatePosition();
    bool    INeedReRender();

    void ISetupRenderRequests();
    void ISubmitRenderRequests();
    void ISubmitRenderRequest(int i);
    void ICheckForRefresh(double t, plPipeline *pipe);
    
    bool    IOnRefMsg(plGenRefMsg* refMsg);

public:
    plDynamicEnvMap();
    plDynamicEnvMap(uint16_t width, uint16_t height, uint8_t bitDepth, uint8_t zDepth = -1, uint8_t sDepth = -1);

    virtual ~plDynamicEnvMap();

    CLASSNAME_REGISTER( plDynamicEnvMap );
    GETINTERFACE_ANY( plDynamicEnvMap, plCubicRenderTarget );

    virtual void    Read(hsStream* s, hsResMgr* mgr);
    virtual void    Write(hsStream* s, hsResMgr* mgr);

    virtual bool MsgReceive(plMessage* msg);

    void ReRender();

    void Init();

    void        SetPosition(const hsPoint3& pos);
    void        SetHither(float f);
    void        SetYon(float f);
    void        SetFogStart(float f);
    void        SetColor(const hsColorRGBA& col);
    void        SetRefreshRate(float secs);

    hsPoint3    GetPosition() const;
    float    GetHither() const { return fHither; }
    float    GetYon() const { return fYon; }
    float    GetFogStart() const { return fFogStart; }
    hsColorRGBA GetColor() const { return fColor; }
    float    GetRefreshRate() const { return 6.f * fRefreshRate; }

    void        AddVisRegion(plVisRegion* reg); // Will just send a ref

    void        SetIncludeCharacters(bool b);
    bool        GetIncludeCharacters() const { return fIncCharacters; }
    void        SetVisRegionName(char *name){ fVisRegionNames.Push(name); }
};

////////////////////////////////////////////////////////////////////////////
// Yes, it's lame that a lot of this code is nearly the same as
// plDynamicEnvMap, but this derives from plRenderTarget, not plCubicRenderTarget
// and I don't want to touch multiple inheritance.
class plDynamicCamMap : public plRenderTarget
{
public:
    enum 
    {
        kRefVisSet,
        kRefCamera,
        kRefRootNode,
        kRefTargetNode,
        kRefDisableTexture,
        kRefMatLayer,
    };

    float                    fHither;
    float                    fYon;
    float                    fFogStart;
    hsColorRGBA                 fColor;

protected:
    plRenderRequest             fReq;
    plRenderRequestMsg*         fReqMsg;

    float                    fRefreshRate;
    double                      fLastRefresh;
    int                         fOutStanding;

    hsBitVector                 fVisSet;
    hsTArray<plVisRegion*>      fVisRegions;
    hsTArray<char *>            fVisRegionNames;    // this allows us to specify vis-regions in other pages.    
    bool                        fIncCharacters;
    plCameraModifier1*          fCamera;
    plSceneObject*              fRootNode;
    hsTArray<plSceneObject*>    fTargetNodes;

    // Extra info for swapping around textures when reflections are disabled.
    plBitmap*                   fDisableTexture;
    hsTArray<plLayer*>          fMatLayers;
    static uint8_t                fFlags;
    enum 
    {
        kReflectionCapable  = 0x01,
        kReflectionEnabled  = 0x02,
        kReflectionMask     = kReflectionCapable | kReflectionEnabled,
    };

    bool    INeedReRender();

    void ISetupRenderRequest(plPipeline *pipe);
    void ISubmitRenderRequest(plPipeline *pipe);
    void ICheckForRefresh(double t, plPipeline *pipe);
    void IPrepTextureLayers();

    bool    IOnRefMsg(plRefMsg* refMsg);

public:
    plDynamicCamMap();
    plDynamicCamMap(uint16_t width, uint16_t height, uint8_t bitDepth, uint8_t zDepth = -1, uint8_t sDepth = -1);

    virtual ~plDynamicCamMap();

    CLASSNAME_REGISTER( plDynamicCamMap );
    GETINTERFACE_ANY( plDynamicCamMap, plRenderTarget );

    virtual void    Read(hsStream* s, hsResMgr* mgr);
    virtual void    Write(hsStream* s, hsResMgr* mgr);

    virtual bool MsgReceive(plMessage* msg);

    void ReRender();
    void Init();

    void        SetIncludeCharacters(bool b);
    void        SetRefreshRate(float secs);
    void        AddVisRegion(plVisRegion* reg);
    void        SetVisRegionName(char *name){ fVisRegionNames.Push(name); }

    static bool     GetEnabled() { return (fFlags & kReflectionEnabled) != 0; }
    static void     SetEnabled(bool enable);
    static bool     GetCapable() { return (fFlags & kReflectionCapable) != 0; }
    static void     SetCapable(bool capable);
};

#endif // plDynamicEnvMap_inc
