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
#include "plDynaRippleVSMgr.h"
#include "plDynaDecal.h"

#include "plPrintShape.h"

#include "plCutter.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "plWaveSetBase.h"
#include "plRipVSConsts.h"

#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include "plMessage/plDynaDecalEnableMsg.h"
#include "pnMessage/plRefMsg.h"
#include "plMessage/plRippleShapeMsg.h"

#include "plMessage/plAvatarMsg.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plAvatar/plArmatureMod.h"

#include "pnEncryption/plRandom.h"

#include "plTweak.h"

size_t plDynaRippleVSMgr::INewDecal()
{
    size_t idx = fDecals.size();

    plDynaRippleVS* rip = new plDynaRippleVS();
    rip->fC1U = fInitUVW.fX;
    rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

    rip->fC1V = fInitUVW.fY;
    rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

    fDecals.emplace_back(rip);

    return idx;
}


plDynaRippleVSMgr::plDynaRippleVSMgr()
:   fWaveSetBase()
{
}

plDynaRippleVSMgr::~plDynaRippleVSMgr()
{
}

void plDynaRippleVSMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaRippleMgr::Read(stream, mgr);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefWaveSetBase), plRefFlags::kPassiveRef);
}

void plDynaRippleVSMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaRippleMgr::Write(stream, mgr);

    mgr->WriteKey(stream, fWaveSetBase);
}

bool plDynaRippleVSMgr::MsgReceive(plMessage* msg)
{
    bool retVal = plDynaRippleMgr::MsgReceive(msg);
    if( retVal )
        return true;

    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        switch( refMsg->fType )
        {
        case kRefWaveSetBase:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fWaveSetBase = plWaveSetBase::ConvertNoRef(refMsg->GetRef());
            else
                fWaveSetBase = nullptr;
            return true;
        }
    }
    return false;
}

bool plDynaRippleVSMgr::ICheckRTMat()
{
    if( !fMatRTShade )
        return false;

    if( !fWaveSetBase )
        return false;

    if( fMatRTShade->GetLayer(0)->GetVertexShader() )
        return true;

    plRipVSConsts ripConsts;

    ripConsts.fC1U = fInitUVW.fX;
    ripConsts.fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

    ripConsts.fC1V = fInitUVW.fY;
    ripConsts.fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

    ripConsts.fInitAtten = fInitAtten;
    ripConsts.fLife = fLifeSpan;
    ripConsts.fDecay = fDecayStart;
    ripConsts.fRamp = fRampEnd;

    return fWaveSetBase->SetupRippleMat(fMatRTShade, ripConsts);
}

bool plDynaRippleVSMgr::IRippleFromShape(const plPrintShape* shape, bool force)
{
    static plRandom sRand;

    if( !ICheckRTMat() )
        return false;

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

        // Are we potentially touching the water?
        float waterHeight = fWaveSetBase->GetHeight();
        if( (pos.fZ - fScale.fZ * shape->GetHeight() < waterHeight)
            &&(pos.fZ + fScale.fZ * shape->GetHeight() > waterHeight) )
        {

            hsVector3 dir(fWaveSetBase->GetWindDir());
            hsVector3 up(0.f, 0.f, 1.f);

            float wid = std::max(shape->GetWidth(), shape->GetLength());
            
            plConst(float) kMaxWaterDepth(1000.f);

            hsVector3 size(wid * fScale.fX, wid * fScale.fY, kMaxWaterDepth);
            fCutter->SetLength(size);
            fCutter->Set(pos, dir, up);


            bool hit = ICutoutTargets(t);
            if( hit )
            {
                info.fLastTime = t;
                info.fLastPos = shapeL2W.GetTranslate();
                retVal = true;
            }
            else
            {
                retVal = false; // No-effect else just for break points.
            }
        }
    }
    return retVal;
}

