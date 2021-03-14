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
#include "plDynaWakeMgr.h"
#include "plDynaDecal.h"

#include "plPrintShape.h"

#include "plCutter.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "plMessage/plDynaDecalEnableMsg.h"
#include "plMessage/plRippleShapeMsg.h"

#include "plMessage/plAvatarMsg.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plAvatar/plArmatureMod.h"

#include "plInterp/plAnimPath.h"

#include "hsFastMath.h"
#include "pnEncryption/plRandom.h"


int plDynaWakeMgr::INewDecal()
{
    int idx = fDecals.GetCount();

    plDynaWake* wake = new plDynaWake();
    wake->fC1U = fInitUVW.fX;
    wake->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

    wake->fC1V = fInitUVW.fY;
    wake->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

    fDecals.Append(wake);

    return idx;
}

plDynaWakeMgr::plDynaWakeMgr()
:
    fAnimPath(),
    fDefaultDir(0.f, 1.f, 0.f),
    fAnimWgt(0),
    fVelWgt(1.f)
{
}

plDynaWakeMgr::~plDynaWakeMgr()
{
    delete fAnimPath;
}

void plDynaWakeMgr::SetAnimPath(plAnimPath* a)
{
    delete fAnimPath;
    fAnimPath = a;
}

void plDynaWakeMgr::SetDefaultDir(const hsVector3& v)
{
    fDefaultDir = v;
    hsFastMath::Normalize(fDefaultDir);
}

void plDynaWakeMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaRippleMgr::Read(stream, mgr);

    fDefaultDir.Read(stream);
    fAnimPath = plAnimPath::ConvertNoRef(mgr->ReadCreatable(stream));

    fAnimWgt = stream->ReadLEFloat();
    fVelWgt = stream->ReadLEFloat();
}

void plDynaWakeMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaRippleMgr::Write(stream, mgr);

    fDefaultDir.Write(stream);
    mgr->WriteCreatable(stream, fAnimPath);

    stream->WriteLEFloat(fAnimWgt);
    stream->WriteLEFloat(fVelWgt);
}

hsVector3 plDynaWakeMgr::IGetDirection(const plDynaDecalInfo& info, const hsPoint3& pos) const
{
    hsVector3 dir = fDefaultDir;
    // If we have an animpath, figure a direction based on position here
    if( fAnimPath )
    {
        hsVector3 animDir;
        hsPoint3 p = pos;
        float t = fAnimPath->GetExtremePoint(p);
        fAnimPath->SetCurTime(t, plAnimPath::kNone);

        fAnimPath->GetVelocity(&animDir);

        animDir *= fAnimWgt;

        dir += animDir;
    }

    // Now if we want to factor in velocity, we can use (pos - info.fLastPos) / (hsTimer::GetSysSeconds() - info.fLastTime)
    float dt = float(hsTimer::GetSysSeconds() - info.fLastTime);
    const float kMinDt = 1.e-3f;
    if( (info.fFlags & plDynaDecalInfo::kImmersed) && (dt > kMinDt) )
    {
        hsVector3 velDir(&pos, &info.fLastPos);

        velDir *= 1.f / dt * fVelWgt;

        dir += velDir;
    }
    hsFastMath::Normalize(dir);

    return dir;
}

bool plDynaWakeMgr::IRippleFromShape(const plPrintShape* shape, bool force)
{
    static plRandom sRand;

    if( !shape )
        return false;

    bool retVal = false;

    plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());

    const hsMatrix44& shapeL2W = shape->GetOwner()->GetLocalToWorld();

    static float kMinDist = 1.0f;
    static float kMinTime = 0.25f;
    double t = hsTimer::GetSysSeconds();
    float dt = float(t - info.fLastTime) * sRand.RandZeroToOne();
    bool longEnough = (dt >= kMinTime);
    hsPoint3 xlate = shapeL2W.GetTranslate();
    bool farEnough = (hsVector3(&info.fLastPos, &xlate).Magnitude() > kMinDist);
    if( force || longEnough || farEnough )
    {
        hsPoint3 pos = shapeL2W.GetTranslate();

        // Base the direction on the unperturbed pos.
        hsVector3 dir = IGetDirection(info, pos);

        // We'll perturb the position a little so it doesn't look quite so regular,
        // but we perturb it more if we're just standing still
        hsVector3 randPert(sRand.RandMinusOneToOne(), sRand.RandMinusOneToOne(), 0);
        randPert.Normalize();
        if( !farEnough )
        {
            static float kRandPert = 0.05f;
            randPert *= kRandPert * shape->GetWidth();
        }
        else
        {
            static float kRandPert = 0.05f;
            randPert *= kRandPert * shape->GetWidth();
        }
        pos += randPert;

        hsVector3 up(0.f, 0.f, 1.f);

        static float kHeightScale = 1.f;
        pos.fZ += (shape->GetHeight() * fScale.fZ * kHeightScale) * 0.25f;

        pos += dir * shape->GetLength() * 0.5f * (1.f - fScale.fY);

        hsVector3 size(shape->GetWidth() * fScale.fX, shape->GetLength() * fScale.fY, shape->GetHeight() * fScale.fZ * kHeightScale);
        fCutter->SetLength(size);
        fCutter->Set(pos, dir, up);

        info.fLastTime = t;
        info.fLastPos = shapeL2W.GetTranslate();

        bool hit = ICutoutTargets(t);
        if( hit )
            retVal = true;
    }
    return retVal;
}
