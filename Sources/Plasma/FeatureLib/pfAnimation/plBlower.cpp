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

#include "hsTypes.h"
#include "plBlower.h"
#include "plgDispatch.h"
#include "hsFastMath.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnMessage/plTimeMsg.h"
#include "hsTimer.h"

plRandom plBlower::fRandom;

static const hsScalar kDefaultMasterPower = 20.f;
static const hsScalar kDefaultMasterFrequency = 2.f;
static const hsScalar kDefaultDirectRate = 1.f;
static const hsScalar kDefaultImpulseRate = 1.e2f;
static const hsScalar kDefaultSpringKonst = 20.f;
static const hsScalar kDefaultBias = 0.25f;
static const hsScalar kInitialMaxOffDist = 1.f;

plBlower::plBlower()
:   
    fMasterPower(kDefaultMasterPower),
    fMasterFrequency(kDefaultMasterFrequency),
    fDirectRate(kDefaultDirectRate),
    fImpulseRate(kDefaultImpulseRate),
    fSpringKonst(kDefaultSpringKonst),
    fBias(kDefaultBias),
    fMaxOffsetDist(kInitialMaxOffDist),
    fAccumTime(0)
{
    fRestPos.Set(0,0,0);
    fLocalRestPos.Set(0,0,0);
    fCurrDel.Set(0,0,0);

    fDirection.Set(fRandom.RandMinusOneToOne(), fRandom.RandMinusOneToOne(), 0);
    hsFastMath::NormalizeAppr(fDirection);
}

plBlower::~plBlower()
{
}

void plBlower::IBlow(double secs, hsScalar delSecs)
{
    hsPoint3 worldPos = fTarget->GetLocalToWorld().GetTranslate();
    hsPoint3 localPos = fTarget->GetLocalToParent().GetTranslate();

    // fast oscillation vs slow

    // Completely random walk in the rotation

    // Strength = Strength + rnd01 * (MaxStrength - Strength)

    hsScalar t = (fAccumTime += delSecs);

    hsScalar strength = 0;
    int i;
    for( i = 0; i < fOscillators.GetCount(); i++ )
    {
        hsScalar c, s;
        t *= fOscillators[i].fFrequency * fMasterFrequency;
        t += fOscillators[i].fPhase;
        hsFastMath::SinCosAppr(t, s, c);
        c += fBias;
        strength += c * fOscillators[i].fPower;
    }
    strength *= fMasterPower;

    if( strength < 0 )
        strength = 0;

    fDirection.fX += fRandom.RandMinusOneToOne() * delSecs * fDirectRate;
    fDirection.fY += fRandom.RandMinusOneToOne() * delSecs * fDirectRate;

    hsFastMath::NormalizeAppr(fDirection);

    hsScalar offDist = hsVector3(&fRestPos, &worldPos).Magnitude();
    if( offDist > fMaxOffsetDist )
        fMaxOffsetDist = offDist;

    hsVector3 force = fDirection * strength;

    static hsScalar kOffsetDistFrac = 0.5f; // make me const
    if( offDist > fMaxOffsetDist * kOffsetDistFrac )
    {
        offDist /= fMaxOffsetDist;
        offDist *= fMasterPower;

        hsScalar impulse = offDist * delSecs * fImpulseRate;

        force.fX += impulse * fRandom.RandMinusOneToOne();
        force.fY += impulse * fRandom.RandMinusOneToOne();
        force.fZ += impulse * fRandom.RandMinusOneToOne();
    }
    const hsScalar kOffsetDistDecay = 0.999f;
    fMaxOffsetDist *= kOffsetDistDecay;

    hsVector3 accel = force;
    accel += fSpringKonst * hsVector3(&fLocalRestPos, &localPos);

    hsVector3 del = accel * (delSecs * delSecs);

    fCurrDel = del;
}

hsBool plBlower::IEval(double secs, hsScalar delSecs, UInt32 dirty)
{
    const hsScalar kMaxDelSecs = 0.1f;
    if( delSecs > kMaxDelSecs )
        delSecs = kMaxDelSecs;
    IBlow(secs, delSecs);

    ISetTargetTransform();

    return true;
}

void plBlower::ISetTargetTransform()
{
    plCoordinateInterface* ci = IGetTargetCoordinateInterface(0);
    if( ci )
    {
        hsMatrix44 l2p = ci->GetLocalToParent();
        hsMatrix44 p2l = ci->GetParentToLocal();

        hsPoint3 pos = l2p.GetTranslate();
        pos += fCurrDel;

        hsPoint3 neg = -pos;
        l2p.SetTranslate(&pos);
        p2l.SetTranslate(&neg);
    
        ci->SetLocalToParent(l2p, p2l);
    }
}

void plBlower::SetTarget(plSceneObject* so)
{
    plSingleModifier::SetTarget(so);

    if( fTarget )
    {
        fRestPos = fTarget->GetLocalToWorld().GetTranslate();
        fLocalRestPos = fTarget->GetLocalToParent().GetTranslate();
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
    }

    IInitOscillators();
}

void plBlower::Read(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Read(s, mgr);

    fMasterPower = s->ReadLEScalar();
    fDirectRate = s->ReadLEScalar();
    fImpulseRate = s->ReadLEScalar();
    fSpringKonst = s->ReadLEScalar();
}

void plBlower::Write(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Write(s, mgr);

    s->WriteLEScalar(fMasterPower);
    s->WriteLEScalar(fDirectRate);
    s->WriteLEScalar(fImpulseRate);
    s->WriteLEScalar(fSpringKonst);
}

void plBlower::IInitOscillators()
{
    const hsScalar kBasePower = 5.f;
    fOscillators.SetCount(5);
    int i;
    for( i = 0; i < fOscillators.GetCount(); i++ )
    {
        hsScalar fi = hsScalar(i+1);
        fOscillators[i].fFrequency = fi / hsScalarPI * fRandom.RandRangeF(0.75f, 1.25f);
//      fOscillators[i].fFrequency = 1.f / hsScalarPI * fRandom.RandRangeF(0.5f, 1.5f);
        fOscillators[i].fPhase = fRandom.RandZeroToOne();
        fOscillators[i].fPower = kBasePower * fRandom.RandRangeF(0.75f, 1.25f);
    }
}

void plBlower::SetConstancy(hsScalar f)
{
    if( f < 0 )
        f = 0;
    else if( f > 1.f )
        f = 1.f;

    fBias = f;
}

hsScalar plBlower::GetConstancy() const
{
    return fBias;
}
