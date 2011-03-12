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

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

#include "../plMessage/plDynaDecalEnableMsg.h"
#include "../plMessage/plRippleShapeMsg.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plArmatureMod.h"

#include "../plMath/plRandom.h"
static plRandom sRand;

#include "plTweak.h"

int	plDynaRippleVSMgr::INewDecal()
{
	int idx = fDecals.GetCount();

	plDynaRippleVS* rip = TRACKED_NEW plDynaRippleVS();
	rip->fC1U = fInitUVW.fX;
	rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

	rip->fC1V = fInitUVW.fY;
	rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

	fDecals.Append(rip);

	return idx;
}


plDynaRippleVSMgr::plDynaRippleVSMgr()
:	fWaveSetBase(nil)
{
}

plDynaRippleVSMgr::~plDynaRippleVSMgr()
{
}

void plDynaRippleVSMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaRippleMgr::Read(stream, mgr);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefWaveSetBase), plRefFlags::kPassiveRef);
}

void plDynaRippleVSMgr::Write(hsStream* stream, hsResMgr* mgr)
{
	plDynaRippleMgr::Write(stream, mgr);

	mgr->WriteKey(stream, fWaveSetBase);
}

hsBool plDynaRippleVSMgr::MsgReceive(plMessage* msg)
{
	hsBool retVal = plDynaRippleMgr::MsgReceive(msg);
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
				fWaveSetBase = nil;
			return true;
		}
	}
	return false;
}

hsBool plDynaRippleVSMgr::ICheckRTMat()
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

hsBool plDynaRippleVSMgr::IRippleFromShape(const plPrintShape* shape, hsBool force)
{
	if( !ICheckRTMat() )
		return false;

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

		// Are we potentially touching the water?
		hsScalar waterHeight = fWaveSetBase->GetHeight();
		if( (pos.fZ - fScale.fZ * shape->GetHeight() < waterHeight)
			&&(pos.fZ + fScale.fZ * shape->GetHeight() > waterHeight) )
		{

			hsVector3 dir(fWaveSetBase->GetWindDir());
			hsVector3 up(0.f, 0.f, 1.f);

			hsScalar wid = hsMaximum(shape->GetWidth(), shape->GetLength());
			
			plConst(hsScalar) kMaxWaterDepth(1000.f);

			hsVector3 size(wid * fScale.fX, wid * fScale.fY, kMaxWaterDepth);
			fCutter->SetLength(size);
			fCutter->Set(pos, dir, up);


			hsBool hit = ICutoutTargets(t);
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

