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

#ifndef plDistOpacityMod_inc
#define plDistOpacityMod_inc

#include "hsGeometry3.h"
#include "pnModifier/plSingleModifier.h"
#include "hsTemplates.h"

class plPipeline;
class plRenderMsg;
class plFadeOpacityLay;


class plDistOpacityMod : public plSingleModifier
{
public:

    enum {
        kTrackAvatar,
        kTrackCamera
    };
protected:

    enum {
        kRefFadeLay
    };

    // Volatile flag, whether we're setup yet or not.
    uint8_t           fSetup;

    enum {
        kNearTrans,
        kNearOpaq,
        kFarOpaq,
        kFarTrans,

        kNumDists
    };
    float           fDists[kNumDists];

    hsPoint3        fRefPos;

    hsTArray<plFadeOpacityLay*> fFadeLays;

    // We only act in response to messages.
    virtual bool IEval(double secs, float del, uint32_t dirty) { return false; }

    float ICalcOpacity(const hsPoint3& targPos, const hsPoint3& refPos) const;
    void ISetOpacity();

    void ISetup();

    void ICheckDists()
    {
        hsAssert(fDists[kNearTrans] <= fDists[kNearOpaq], "Bad transition values");
        hsAssert(fDists[kNearOpaq] <= fDists[kFarOpaq], "Bad transition values");
        hsAssert(fDists[kFarOpaq] <= fDists[kFarTrans], "Bad transition values");
    }

public:
    plDistOpacityMod();
    virtual ~plDistOpacityMod();

    CLASSNAME_REGISTER( plDistOpacityMod );
    GETINTERFACE_ANY( plDistOpacityMod, plSingleModifier );

    virtual void            SetKey(plKey k);

    virtual bool            MsgReceive(plMessage* msg);

    virtual void            Read(hsStream* s, hsResMgr* mgr);
    virtual void            Write(hsStream* s, hsResMgr* mgr);

    virtual void            SetTarget(plSceneObject* so);

    // Rules are:
    // NearTrans <= NearOpaq <= FarOpaque <= FarTrans
    void SetFarDist(float opaque, float transparent);
    void SetNearDist(float transparent, float opaque);

    float GetFarTransparent() const { return fDists[kFarTrans]; }
    float GetNearTransparent() const { return fDists[kNearTrans]; }
    float GetFarOpaque() const { return fDists[kFarOpaq]; }
    float GetNearOpaque() const { return fDists[kNearOpaq]; }
};

#endif // plDistOpacityMod_inc
