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


#include "plDistOpacityMod.h"

#include "HeadSpin.h"
#include "plPipeline.h"
#include "plgDispatch.h"
#include "hsQuat.h"
#include "hsResMgr.h"

#include "plFadeOpacityLay.h"

#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plArmatureMod.h"
#include "plDrawable/plAccessGeometry.h"
#include "plDrawable/plAccessSpan.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plMatRefMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plSurface/hsGMaterial.h"

plDistOpacityMod::plDistOpacityMod()
:   fSetup(false)
{
    fDists[kNearTrans] = 0;
    fDists[kNearOpaq] = 0;
    fDists[kFarOpaq] = 0;
    fDists[kFarTrans] = 0;
}

plDistOpacityMod::~plDistOpacityMod()
{
}

void plDistOpacityMod::SetKey(plKey k)
{
    plSingleModifier::SetKey(k);

    plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plArmatureUpdateMsg::Index(), GetKey());
}


float plDistOpacityMod::ICalcOpacity(const hsPoint3& targPos, const hsPoint3& refPos) const
{
    float dist = hsVector3(&targPos, &refPos).Magnitude();

    if( dist > fDists[kFarTrans] )
        return 0;
    if( dist < fDists[kNearTrans] )
        return 0;

    if( dist > fDists[kFarOpaq] )
    {
        dist -= fDists[kFarOpaq];
        dist /= (fDists[kFarTrans] - fDists[kFarOpaq]);
        hsAssert(dist >= 0, "unexpected interpolation param - neg");
        hsAssert(dist <= 1.f, "unexpected interpolation param - > one");

        return 1.f - dist;
    }

    if( dist < fDists[kNearOpaq] )
    {
        dist -= fDists[kNearTrans];
        dist /= (fDists[kNearOpaq] - fDists[kNearTrans]);
        hsAssert(dist >= 0, "unexpected interpolation param - neg");
        hsAssert(dist <= 1.f, "unexpected interpolation param - > one");

        return dist;
    }

    return 1.f;
}

void plDistOpacityMod::ISetOpacity()
{
    if( !GetTarget() )
        return;

    if( !fSetup )
        ISetup();

    float opacity = ICalcOpacity(GetTarget()->GetLocalToWorld().GetTranslate(), fRefPos);

    for (plFadeOpacityLay* lay : fFadeLays)
        lay->SetOpacity(opacity);
}

bool plDistOpacityMod::MsgReceive(plMessage* msg)
{
    plArmatureUpdateMsg* arm = plArmatureUpdateMsg::ConvertNoRef(msg);
    if( arm && arm->IsLocal() )
    {
        arm->fArmature->GetPositionAndRotationSim(&fRefPos, nullptr);

        return true;
    }

    plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
    if( rend )
    {
        if( HasFlag(kTrackCamera) )
            fRefPos = rend->Pipeline()->GetViewPositionWorld();

        ISetOpacity();

        return true;
    }

    plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
    if( ref )
    {
        switch(ref->fType)
        {
        case kRefFadeLay:
            if( ref->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                plFadeOpacityLay* lay = plFadeOpacityLay::ConvertNoRef(ref->GetRef());
                auto idx = std::find(fFadeLays.cbegin(), fFadeLays.cend(), lay);
                if (idx != fFadeLays.end())
                    fFadeLays.erase(idx);
            }
            break;
        };
        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}

void plDistOpacityMod::Read(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Read(s, mgr);

    s->ReadLEFloat(kNumDists, fDists);

    ICheckDists();

    fSetup = false;
}

void plDistOpacityMod::Write(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Write(s, mgr);

    s->WriteLEFloat(kNumDists, fDists);
}

void plDistOpacityMod::SetTarget(plSceneObject* so)
{
    plSingleModifier::SetTarget(so);

    fSetup = false;
}

void plDistOpacityMod::SetFarDist(float opaque, float transparent)
{
    fDists[kFarOpaq] = opaque;
    fDists[kFarTrans] = transparent;

    ICheckDists();
}

void plDistOpacityMod::SetNearDist(float transparent, float opaque)
{
    fDists[kNearOpaq] = opaque;
    fDists[kNearTrans] = transparent;

    ICheckDists();
}

using MatLayer = std::tuple<hsGMaterial*, plLayerInterface*>;

void plDistOpacityMod::ISetup()
{
    fFadeLays.clear();

    plSceneObject* so = GetTarget();
    if( !so )
        return;

    const plDrawInterface* di = so->GetDrawInterface();
    if( !di )
        return;

    std::vector<MatLayer> todo;

    std::vector<plAccessSpan> src;
    plAccessGeometry::Instance()->OpenRO(di, src, false);

    // We are guaranteed that each Max object will be given a unique
    // copy of materials and associated layers. But within an object,
    // a given layer may be shared across materials.
    // So we'll build a list of the layers that need a FadeOpacityLay applied,
    // making sure we don't include any layer more than once (strip repeats).
    // This would be grossly inefficient if the numbers involved weren't all
    // very small. So an n^2 search isn't bad if n <= 2.
    for (const plAccessSpan& span : src)
    {
        hsGMaterial* mat = span.GetMaterial();

        for (size_t j = 0; j < mat->GetNumLayers(); j++)
        {
            plLayerInterface* lay = mat->GetLayer(j);
            if( !j || !(lay->GetZFlags() & hsGMatState::kZNoZWrite) || (lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere) )
            {
                auto it = std::find_if(todo.cbegin(), todo.cend(),
                                       [lay](const MatLayer& matLayer) {
                                           return lay == std::get<1>(matLayer);
                                       });
                if (it == todo.end())
                    todo.emplace_back(mat, lay);
            }
        }
    }

    plAccessGeometry::Instance()->Close(src);

    for (size_t i = 0; i < todo.size(); ++i)
    {
        // We need i below...
        const auto [mat, lay] = todo[i];

        plFadeOpacityLay* fade = new plFadeOpacityLay;

        hsgResMgr::ResMgr()->NewKey(lay->GetKey()->GetName(), fade, lay->GetKey()->GetUoid().GetLocation());

        fade->AttachViaNotify(lay);

        // We should add a ref or something here if we're going to hold on to this (even though we created and "own" it).
        fFadeLays.emplace_back(fade);

        plMatRefMsg* msg = new plMatRefMsg(mat->GetKey(), plRefMsg::kOnReplace, i, plMatRefMsg::kLayer);
        msg->SetOldRef(lay);
        hsgResMgr::ResMgr()->SendRef(fade, msg, plRefFlags::kActiveRef);

        plGenRefMsg* toMe = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefFadeLay);
        hsgResMgr::ResMgr()->SendRef(fade, toMe, plRefFlags::kPassiveRef);
    }

    fSetup = true;
}
