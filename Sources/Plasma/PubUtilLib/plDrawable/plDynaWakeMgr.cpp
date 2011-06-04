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
#include "plDynaWakeMgr.h"
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

#include "../plInterp/plAnimPath.h"

#include "hsFastMath.h"
#include "../plMath/plRandom.h"
static plRandom sRand;


int	plDynaWakeMgr::INewDecal()
{
	int idx = fDecals.GetCount();

	plDynaWake* wake = TRACKED_NEW plDynaWake();
	wake->fC1U = fInitUVW.fX;
	wake->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

	wake->fC1V = fInitUVW.fY;
	wake->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

	fDecals.Append(wake);

	return idx;
}

plDynaWakeMgr::plDynaWakeMgr()
:
	fAnimPath(nil),
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

	fAnimWgt = stream->ReadSwapScalar();
	fVelWgt = stream->ReadSwapScalar();
}

void plDynaWakeMgr::Write(hsStream* stream, hsResMgr* mgr)
{
	plDynaRippleMgr::Write(stream, mgr);

	fDefaultDir.Write(stream);
	mgr->WriteCreatable(stream, fAnimPath);

	stream->WriteSwapScalar(fAnimWgt);
	stream->WriteSwapScalar(fVelWgt);
}

hsVector3 plDynaWakeMgr::IGetDirection(const plDynaDecalInfo& info, const hsPoint3& pos) const
{
	hsVector3 dir = fDefaultDir;
	// If we have an animpath, figure a direction based on position here
	if( fAnimPath )
	{
		hsVector3 animDir;
		hsPoint3 p = pos;
		hsScalar t = fAnimPath->GetExtremePoint(p);
		fAnimPath->SetCurTime(t, plAnimPath::kNone);

		fAnimPath->GetVelocity(&animDir);

		animDir *= fAnimWgt;

		dir += animDir;
	}

	// Now if we want to factor in velocity, we can use (pos - info.fLastPos) / (hsTimer::GetSysSeconds() - info.fLastTime)
	hsScalar dt = hsScalar(hsTimer::GetSysSeconds() - info.fLastTime);
	const hsScalar kMinDt = 1.e-3f;
	if( (info.fFlags & plDynaDecalInfo::kImmersed) && (dt > kMinDt) )
	{
		hsVector3 velDir(&pos, &info.fLastPos);

		velDir *= 1.f / dt * fVelWgt;

		dir += velDir;
	}
	hsFastMath::Normalize(dir);

	return dir;
}

hsBool plDynaWakeMgr::IRippleFromShape(const plPrintShape* shape, hsBool force)
{
	if( !shape )
		return false;

	hsBool retVal = false;

	plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());

	const hsMatrix44& shapeL2W = shape->GetOwner()->GetLocalToWorld();

	static hsScalar kMinDist = 1.0f;
	static hsScalar kMinTime = 0.25f;
	double t = hsTimer::GetSysSeconds();
	hsScalar dt = hsScalar(t - info.fLastTime) * sRand.RandZeroToOne();
	hsBool longEnough = (dt >= kMinTime);
	hsBool farEnough = (hsVector3(&info.fLastPos, &shapeL2W.GetTranslate()).Magnitude() > kMinDist);
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
			static hsScalar kRandPert = 0.05f;
			randPert *= kRandPert * shape->GetWidth();
		}
		else
		{
			static hsScalar kRandPert = 0.05f;
			randPert *= kRandPert * shape->GetWidth();
		}
		pos += randPert;

		hsVector3 up(0.f, 0.f, 1.f);

		static hsScalar kHeightScale = 1.f;
		pos.fZ += (shape->GetHeight() * fScale.fZ * kHeightScale) * 0.25f;

		pos += dir * shape->GetLength() * 0.5f * (1.f - fScale.fY);

		hsVector3 size(shape->GetWidth() * fScale.fX, shape->GetLength() * fScale.fY, shape->GetHeight() * fScale.fZ * kHeightScale);
		fCutter->SetLength(size);
		fCutter->Set(pos, dir, up);

		info.fLastTime = t;
		info.fLastPos = shapeL2W.GetTranslate();

		hsBool hit = ICutoutTargets(t);
		if( hit )
			retVal = true;
	}
	return retVal;
}