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

#ifndef plPostEffectMod_inc
#define plPostEffectMod_inc

#include "hsBitVector.h"
#include "hsMatrix44.h"

#include "pnModifier/plSingleModifier.h"


class plMessage;
class plPageTreeMgr;
class plRenderRequest;
class plRenderTarget;
class plSceneNode;
class plViewTransform;

class plPostEffectMod : public plSingleModifier
{
public:
    enum plPostEffectModStates {
        kEnabled = 0
    };

    enum {
        kNodeRef = 0x0
    };
protected:

    hsBitVector             fState;

    float                fHither;
    float                fYon;

    float                fFovX;
    float                fFovY;

    plKey                   fNodeKey;
    plPageTreeMgr*          fPageMgr;

    plRenderTarget*         fRenderTarget;
    plRenderRequest*        fRenderRequest;

    hsMatrix44              fDefaultW2C, fDefaultC2W;


    bool IEval(double secs, float del, uint32_t dirty) override; // called only by owner object's Eval()

    void            ISetupRenderRequest();
    void            IDestroyRenderRequest();
    void            IUpdateRenderRequest();

    void            IRegisterForRenderMsg(bool on);
    void            ISubmitRequest();

    void            IAddToPageMgr(plSceneNode* node);
    void            IRemoveFromPageMgr(plSceneNode* node);

    void            ISetEnable(bool on);
    bool            IIsEnabled() const;

public:
    plPostEffectMod();
    virtual ~plPostEffectMod();

    CLASSNAME_REGISTER( plPostEffectMod );
    GETINTERFACE_ANY( plPostEffectMod, plSingleModifier );


    bool    MsgReceive(plMessage* pMsg) override;
    
    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    void        GetDefaultWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w );

    // Export only
    void        SetNodeKey(plKey key) { fNodeKey = std::move(key); }
    plKey       GetNodeKey() const { return fNodeKey; }

    void        SetHither(float h) { fHither = h; }
    void        SetYon(float y) { fYon = y; }
    void        SetFovX(float f) { fFovX = f; }
    void        SetFovY(float f) { fFovY = f; }

    float    GetHither() const { return fHither; }
    float    GetYon() const { return fYon; }
    float    GetFovX() const { return fFovX; }
    float    GetFovY() const { return fFovY; }

    plPageTreeMgr* GetPageMgr() const { return fPageMgr; }

    const plViewTransform& GetViewTransform();

    // If translating from a scene object, send WorldToLocal() and LocalToWorld(), in that order
    void        SetWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w );

    // Very bad
    void        EnableLightsOnRenderRequest();
};

#endif // plPostEffectMod_inc
