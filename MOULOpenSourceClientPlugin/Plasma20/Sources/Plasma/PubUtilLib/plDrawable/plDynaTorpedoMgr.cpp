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
#include "plDynaTorpedoMgr.h"

#include "../plMessage/plBulletMsg.h"

#include "plCutter.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plTweak.h"

#include "../plMath/plRandom.h"

static const UInt32 kNumPrintIDs = 0;

static plRandom sRand;

plDynaTorpedoMgr::plDynaTorpedoMgr()
{
	fPartIDs.SetCount(kNumPrintIDs);
}

plDynaTorpedoMgr::~plDynaTorpedoMgr()
{
}

void plDynaTorpedoMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaRippleMgr::Read(stream, mgr);

	plgDispatch::Dispatch()->RegisterForExactType(plBulletMsg::Index(), GetKey());
}

hsBool plDynaTorpedoMgr::IHandleShot(plBulletMsg* bull)
{
	hsScalar partyTime = fPartyTime;

	plConst(int) kNumShots(3);
	int i;
	for( i = 0; i < kNumShots; i++ )
	{
		hsVector3 up = IRandomUp(bull->Dir());
		hsVector3 pert = bull->Dir() % up;

		plConst(hsScalar) kMaxPert(1.f);
		hsScalar maxPert = i ? kMaxPert * bull->Radius() : 0;
		pert *= sRand.RandMinusOneToOne() * maxPert * fScale.fX;

		pert += up * (sRand.RandMinusOneToOne() * maxPert * fScale.fY);

		hsPoint3 pos = bull->From() + bull->Dir() * (bull->Range() * 0.5f);
		pos += pert;

		hsScalar scaleX = bull->Radius() * fScale.fX * fInitUVW.fX;
		hsScalar scaleY = bull->Radius() * fScale.fY * fInitUVW.fY;

#if 0
		plConst(hsScalar) kMinScale(0.5f);
		if( i )
		{
			scaleX *= sRand.RandRangeF(kMinScale, 1.f);
			scaleY *= sRand.RandRangeF(kMinScale, 1.f);
		}
#elif 0
		hsScalar div = 1.f / (1.f + hsScalar(i));
		scaleX *= div;
		scaleY *= div;
#else
		plConst(hsScalar) kMinScale(0.25f);
		plConst(hsScalar) kMaxScale(0.75f);
		if( i ) 
		{
			hsScalar scale = sRand.RandRangeF(kMinScale, kMaxScale);
			scaleX *= scale;
			scaleY *= scale;
		}
#endif

		fCutter->SetLength(hsVector3(scaleX, scaleY, bull->Range()));
		fCutter->Set(pos, up, -bull->Dir());

		plDynaDecalInfo& info = IGetDecalInfo(UInt32(this), GetKey());

		if( bull->PartyTime() > 0 )
			fPartyTime = bull->PartyTime();

		double secs = hsTimer::GetSysSeconds();

		if( ICutoutTargets(secs) )
			info.fLastTime = secs;

		fPartyTime = 0;
	
	}
	fPartyTime = partyTime;

	return true;
}

hsBool plDynaTorpedoMgr::MsgReceive(plMessage* msg)
{
	plBulletMsg* bullMsg = plBulletMsg::ConvertNoRef(msg);
	if( bullMsg )
	{
		if( bullMsg->Shot() )
		{
			return IHandleShot(bullMsg);
		}
		return true;
	}

	return plDynaRippleMgr::MsgReceive(msg);
}

