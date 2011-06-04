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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsTypes.h"
#include "plDynaRippleMgr.h"
#include "plDynaDecal.h"

#include "plPrintShape.h"

#include "plCutter.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "../plMessage/plDynaDecalEnableMsg.h"
#include "../plMessage/plRippleShapeMsg.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plArmatureMod.h"

#include "../plMath/plRandom.h"
static plRandom sRand;

#include "plTweak.h"

static const UInt32 kNumPrintIDs = 5;
static const UInt32 kPrintIDs[kNumPrintIDs] =
{
	plAvBrainHuman::TrunkPrint,
	plAvBrainHuman::LHandPrint,
	plAvBrainHuman::RHandPrint,
	plAvBrainHuman::LFootPrint,
	plAvBrainHuman::RFootPrint
};


int	plDynaRippleMgr::INewDecal()
{
	int idx = fDecals.GetCount();

#if 1
	plDynaRipple* rip = TRACKED_NEW plDynaRipple();
	rip->fC1U = fInitUVW.fX;
	rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

	rip->fC1V = fInitUVW.fY;
	rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

	fDecals.Append(rip);
#else
	plDynaWave* wave = TRACKED_NEW plDynaWave();
	static hsScalar kDefScrollRate = 0.1f;
	wave->fScrollRate = kDefScrollRate;
	fDecals.Append(wave);
#endif

	return idx;
}

plDynaRippleMgr::plDynaRippleMgr()
:
	fInitUVW(1.f,1.f,1.f),
	fFinalUVW(1.f,1.f,1.f)
{
	fPartIDs.SetCount(kNumPrintIDs);
	int i;
	for( i = 0; i < kNumPrintIDs; i++ )
		fPartIDs[i] = kPrintIDs[i];
}

plDynaRippleMgr::~plDynaRippleMgr()
{
}

void plDynaRippleMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaDecalMgr::Read(stream, mgr);

	fInitUVW.Read(stream);
	fFinalUVW.Read(stream);

//	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey()); // ###HACKTEST
//	plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey()); // ###HACKTEST
	plgDispatch::Dispatch()->RegisterForExactType(plArmatureUpdateMsg::Index(), GetKey());
}

void plDynaRippleMgr::Write(hsStream* stream, hsResMgr* mgr)
{
	plDynaDecalMgr::Write(stream, mgr);

	fInitUVW.Write(stream);
	fFinalUVW.Write(stream);
}

hsBool plDynaRippleMgr::MsgReceive(plMessage* msg)
{
	plArmatureUpdateMsg* armMsg = plArmatureUpdateMsg::ConvertNoRef(msg);
	if( armMsg && !armMsg->IsInvis())
	{
		int i;
		for( i = 0; i < fPartIDs.GetCount(); i++ )
		{
			const plPrintShape* shape = IGetPrintShape(armMsg->fArmature, fPartIDs[i]);
			if( shape )
			{
				plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
				if( IRippleFromShape(shape, false) )
				{
					INotifyActive(info, armMsg->fArmature->GetKey(), fPartIDs[i]);
				}
				else
				{
					INotifyInactive(info, armMsg->fArmature->GetKey(), fPartIDs[i]);
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

hsBool plDynaRippleMgr::IRippleFromShape(const plPrintShape* shape, hsBool force)
{
	if( !shape )
		return false;

	hsBool retVal = false;

	plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());

	const hsMatrix44& shapeL2W = shape->GetOwner()->GetLocalToWorld();

	plConst(hsScalar) kMinDist(2.0f);
	plConst(hsScalar) kMinTime(1.5f);
	double t = hsTimer::GetSysSeconds();
	hsScalar dt = hsScalar(t - info.fLastTime) * sRand.RandZeroToOne();
	hsBool longEnough = (dt >= kMinTime);
	hsBool farEnough = (hsVector3(&info.fLastPos, &shapeL2W.GetTranslate()).Magnitude() > kMinDist);
	if( force || longEnough || farEnough )
	{
		hsPoint3 pos = shapeL2W.GetTranslate();

		// We'll perturb the position a little so it doesn't look quite so regular,
		// but we perturb it more if we're just standing still
		hsVector3 randPert(sRand.RandMinusOneToOne(), sRand.RandMinusOneToOne(), 0);
		randPert.Normalize();
		if( !farEnough )
		{
			plConst(hsScalar) kRandPert = 0.5f;
			randPert *= kRandPert;
		}
		else
		{
			plConst(hsScalar) kRandPert = 0.15f;
			randPert *= kRandPert;
		}
		pos += randPert;

		hsVector3 dir(0.f, 1.f, 0.f);
		hsVector3 up(0.f, 0.f, 1.f);

		plConst(hsScalar) kHeightScale = 1.f;
		pos.fZ += (shape->GetHeight() * fScale.fZ * kHeightScale) * 0.25f;

		hsScalar wid = hsMaximum(shape->GetWidth(), shape->GetLength());
		hsVector3 size(wid * fScale.fX, wid * fScale.fY, shape->GetHeight() * fScale.fZ * kHeightScale);
		fCutter->SetLength(size);
		fCutter->Set(pos, dir, up);


		hsBool hit = ICutoutTargets(t);
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

