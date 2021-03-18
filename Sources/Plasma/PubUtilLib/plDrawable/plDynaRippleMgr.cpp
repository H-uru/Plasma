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

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"
#include "plTweak.h"

#include "plCutter.h"
#include "plDynaRippleMgr.h"
#include "plDynaDecal.h"
#include "plPrintShape.h"

#include "pnEncryption/plRandom.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plDynaDecalEnableMsg.h"
#include "plMessage/plRippleShapeMsg.h"

size_t plDynaRippleMgr::INewDecal()
{
    size_t idx = fDecals.size();

#if 1
    plDynaRipple* rip = new plDynaRipple();
    rip->fC1U = fInitUVW.fX;
    rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

    rip->fC1V = fInitUVW.fY;
    rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

    fDecals.emplace_back(rip);
#else
    plDynaWave* wave = new plDynaWave();
    static float kDefScrollRate = 0.1f;
    wave->fScrollRate = kDefScrollRate;
    fDecals.emplace_back(wave);
#endif

    return idx;
}

plDynaRippleMgr::plDynaRippleMgr()
:
    fInitUVW(1.f,1.f,1.f),
    fFinalUVW(1.f,1.f,1.f)
{
    fPartIDs = {
        plAvBrainHuman::TrunkPrint,
        plAvBrainHuman::LHandPrint,
        plAvBrainHuman::RHandPrint,
        plAvBrainHuman::LFootPrint,
        plAvBrainHuman::RFootPrint
    };
}

plDynaRippleMgr::~plDynaRippleMgr()
{
}

void plDynaRippleMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Read(stream, mgr);

    fInitUVW.Read(stream);
    fFinalUVW.Read(stream);

//  plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey()); // ###HACKTEST
//  plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey()); // ###HACKTEST
    plgDispatch::Dispatch()->RegisterForExactType(plArmatureUpdateMsg::Index(), GetKey());
}

void plDynaRippleMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Write(stream, mgr);

    fInitUVW.Write(stream);
    fFinalUVW.Write(stream);
}

bool plDynaRippleMgr::MsgReceive(plMessage* msg)
{
    plArmatureUpdateMsg* armMsg = plArmatureUpdateMsg::ConvertNoRef(msg);
    if( armMsg && !armMsg->IsInvis())
    {
        for (uint32_t partID : fPartIDs)
        {
            const plPrintShape* shape = IGetPrintShape(armMsg->fArmature, partID);
            if( shape )
            {
                plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());
                if( IRippleFromShape(shape, false) )
                {
                    INotifyActive(info, armMsg->fArmature->GetKey(), partID);
                }
                else
                {
                    INotifyInactive(info, armMsg->fArmature->GetKey(), partID);
                }
            }
        }
        return true;
    }
    plRippleShapeMsg* shapeMsg = plRippleShapeMsg::ConvertNoRef(msg);
    if( shapeMsg )
    {
        const plPrintShape* shape = shapeMsg->GetShape();
        if( shape )
        {
            // Note we don't care about the return value here, because we only send notifies
            // for avatar based ripples.
            IRippleFromShape(shape);
        }
        return true;
    }

    return plDynaDecalMgr::MsgReceive(msg);
}

bool plDynaRippleMgr::IRippleFromShape(const plPrintShape* shape, bool force)
{
    static plRandom sRand;

    if( !shape )
        return false;

    bool retVal = false;

    plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());

    const hsMatrix44& shapeL2W = shape->GetOwner()->GetLocalToWorld();

    plConst(float) kMinDist(2.0f);
    plConst(float) kMinTime(1.5f);
    double t = hsTimer::GetSysSeconds();
    float dt = float(t - info.fLastTime) * sRand.RandZeroToOne();
    bool longEnough = (dt >= kMinTime);
    hsPoint3 xlate = shapeL2W.GetTranslate();
    bool farEnough = (hsVector3(&info.fLastPos, &xlate).Magnitude() > kMinDist);
    if( force || longEnough || farEnough )
    {
        hsPoint3 pos = shapeL2W.GetTranslate();

        // We'll perturb the position a little so it doesn't look quite so regular,
        // but we perturb it more if we're just standing still
        hsVector3 randPert(sRand.RandMinusOneToOne(), sRand.RandMinusOneToOne(), 0);
        randPert.Normalize();
        if( !farEnough )
        {
            plConst(float) kRandPert = 0.5f;
            randPert *= kRandPert;
        }
        else
        {
            plConst(float) kRandPert = 0.15f;
            randPert *= kRandPert;
        }
        pos += randPert;

        hsVector3 dir(0.f, 1.f, 0.f);
        hsVector3 up(0.f, 0.f, 1.f);

        plConst(float) kHeightScale = 1.f;
        pos.fZ += (shape->GetHeight() * fScale.fZ * kHeightScale) * 0.25f;

        float wid = std::max(shape->GetWidth(), shape->GetLength());
        hsVector3 size(wid * fScale.fX, wid * fScale.fY, shape->GetHeight() * fScale.fZ * kHeightScale);
        fCutter->SetLength(size);
        fCutter->Set(pos, dir, up);


        bool hit = ICutoutTargets(t);
        if( hit )
        {
            retVal = true;
        }
        else
        {
            retVal = false; // No-effect else just for break points.
        }
        // This isn't ideal, but it's a quick fix. ICutoutTargets returns true if the center
        // of our cutter hit a face, which is what we want for notifies. But here we want to
        // know whether any decal faces were generated at all. At some point, I'll have ICutoutTargets
        // return a bit field, with separate bits for different interesting cases. But for now, we'll
        // just keep track of when we last TRIED to throw down a decal. That'll get rid of the
        // current problem of tons of decals piling up when you are standing next to a puddle,
        // so the center isn't covered, so last time doesn't get set, and there's no throttle
        // on how frequently we throw down decals.
        info.fLastTime = t;
        info.fLastPos = shapeL2W.GetTranslate();
    }
    return retVal;
}
