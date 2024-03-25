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

#ifndef plShadowCaster_inc
#define plShadowCaster_inc

#include <vector>

#include "pnModifier/plMultiModifier.h"
#include "hsBounds.h"

class plDrawableSpans;
class plSpan;
class plMessage;
class hsStream;
class hsResMgr;
class plShadowMaster;
class plRenderMsg;

class plShadowCaster : public plMultiModifier
{
public:
    enum {
        kNone           = 0x0,
        kSelfShadow     = 0x1,
        kPerspective    = 0x2,
        kLimitRes       = 0x4
    };
    class DrawSpan
    {
    public:
        DrawSpan& Set(plDrawableSpans* dr, const plSpan* sp, uint32_t idx) { fDraw = (dr); fSpan = (sp); fIndex = (idx); return *this; }

        plDrawableSpans*    fDraw;
        const plSpan*       fSpan;
        uint32_t              fIndex;
    };
protected:

    // Global state to just turn off the whole gig. Not just
    // debugging, we'll probably want a user option for this.
    static bool         fShadowCastDisabled;
    static bool         fCanShadowCast;


    // Properties really just to be read and written,
    // never expected to change. Anything that might be
    // triggered should go into plMultiModifier::fProps,
    // to be network synced.
    uint8_t               fCastFlags;

    float            fBoost;
    float            fAttenScale;
    float            fBlurScale;

    // Casting attributes calculated each frame.
    float            fMaxOpacity;
    std::vector<DrawSpan> fSpans;

    friend class plShadowMaster;

    void ICollectAllSpans();

    bool IOnRenderMsg(plRenderMsg* msg);

    friend class plDXPipeline;
    static void SetCanShadowCast(bool b) { fCanShadowCast = b; }
public:
    plShadowCaster();
    virtual ~plShadowCaster();

    CLASSNAME_REGISTER( plShadowCaster );
    GETINTERFACE_ANY( plShadowCaster, plMultiModifier );
    
    bool IEval(double secs, float del, uint32_t dirty) override { return true; }

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    float MaxOpacity() const { return fMaxOpacity; }
    const std::vector<DrawSpan>& Spans() const { return fSpans; }

    bool    GetSelfShadow() const { return 0 != (fCastFlags & kSelfShadow); }
    void    SetSelfShadow(bool on) { if(on) fCastFlags |= kSelfShadow; else fCastFlags &= ~kSelfShadow; }

    bool    GetPerspective() const { return 0 != (fCastFlags & kPerspective); }
    void    SetPerspective(bool on) { if(on) fCastFlags |= kPerspective; else fCastFlags &= ~kPerspective; }

    bool    GetLimitRes() const { return 0 != (fCastFlags & kLimitRes); }
    void    SetLimitRes(bool on) { if(on) fCastFlags |= kLimitRes; else fCastFlags &= ~kLimitRes; }

    float GetAttenScale() const { return fAttenScale; }
    void SetAttenScale(float s) { fAttenScale = s; }

    float GetBlurScale() const { return fBlurScale; }
    void SetBlurScale(float s) { fBlurScale = s; }

    float GetBoost() const { return fBoost; }
    void SetBoost(float s) { fBoost = s; }

    // These are usually handled internally, activating on read and deactivating
    // on destruct. Made public in case they need to be manually handled, like
    // on dynamic construction and use.
    void Deactivate() const;
    void Activate() const;

    static void DisableShadowCast(bool on=true) { fShadowCastDisabled = on; }
    static void EnableShadowCast(bool on=true) { fShadowCastDisabled = !on; }
    static void ToggleShadowCast() { fShadowCastDisabled = !fShadowCastDisabled; }
    static bool ShadowCastDisabled() { return !CanShadowCast() || fShadowCastDisabled; }

    static bool CanShadowCast() { return fCanShadowCast; }
};

typedef plShadowCaster::DrawSpan plShadowCastSpan;

#endif // plShadowCaster_inc
