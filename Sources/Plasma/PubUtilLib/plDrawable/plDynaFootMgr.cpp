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
#include "plDynaFootMgr.h"
#include "plDynaDecal.h"

#include "plCutter.h"

#include "plPrintShape.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "../plMessage/plDynaDecalEnableMsg.h"

#include "../plMessage/plAvatarFootMsg.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainHuman.h"

#include "../plMath/plRandom.h"
static plRandom sRand;

static const UInt32 kNumPrintIDs = 2;
static const UInt32 kPrintIDs[kNumPrintIDs] =
{
	plAvBrainHuman::RFootPrint,
	plAvBrainHuman::LFootPrint
};


int	plDynaFootMgr::INewDecal()
{
	int idx = fDecals.GetCount();
	fDecals.Append(TRACKED_NEW plDynaSplot());

	return idx;
}

plDynaFootMgr::plDynaFootMgr()
{
	fPartIDs.SetCount(kNumPrintIDs);
	int i;
	for( i = 0; i < kNumPrintIDs; i++ )
		fPartIDs[i] = kPrintIDs[i];
}

plDynaFootMgr::~plDynaFootMgr()
{
}

void plDynaFootMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaDecalMgr::Read(stream, mgr);

	plgDispatch::Dispatch()->RegisterForExactType(plAvatarFootMsg::Index(), GetKey());
}

void plDynaFootMgr::Write(hsStream* stream, hsResMgr* mgr)
{
	plDynaDecalMgr::Write(stream, mgr);
}


hsBool plDynaFootMgr::MsgReceive(plMessage* msg)
{
	plAvatarFootMsg* footMsg = plAvatarFootMsg::ConvertNoRef(msg);
	if( footMsg )
	{
		UInt32 id = footMsg->IsLeft() ? plAvBrainHuman::LFootPrint : plAvBrainHuman::RFootPrint;

		plArmatureMod* armMod = footMsg->GetArmature();
		const plPrintShape* shape = IGetPrintShape(armMod, id);
		if( shape )
		{
			plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
			if( IPrintFromShape(shape, footMsg->IsLeft()) )
			{
				INotifyActive(info, armMod->GetKey(), id);
			}
			else
			{
				INotifyInactive(info, armMod->GetKey(), id);
			}
		}

		return true;
	}

	return plDynaDecalMgr::MsgReceive(msg);
}

hsBool plDynaFootMgr::IPrintFromShape(const plPrintShape* shape, hsBool flip)
{
	hsBool retVal = false;

	if( shape )
	{
		plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());

		double secs = hsTimer::GetSysSeconds();
		hsScalar wetness = IHowWet(info, secs);
		fInitAtten = wetness;

		if( wetness <= 0 )
			return true;

		hsMatrix44 shapeL2W = shape->GetOwner()->GetLocalToWorld();
		hsPoint3 newPos = shapeL2W.GetTranslate();
		hsVector3 newDir = shapeL2W.GetAxis(hsMatrix44::kView);
		hsVector3 newUp(0.f, 0.f, 1.f);

		hsVector3 size(shape->GetWidth() * fScale.fX, shape->GetLength() * fScale.fY, shape->GetHeight() * fScale.fZ * 2.f);
		fCutter->SetLength(size);
		fCutter->Set(newPos, newDir, newUp, flip);

		// Should this be moved inside the if( ICutout() ) clause? I think so. Probably doesn't
		// matter for foot prints, but it seems more correct, since fLastPos/fLastTime is the
		// last time and position we actually dropped a print, not tried to.
		info.fLastPos = newPos;
		info.fLastTime = secs;

		if( ICutoutTargets(secs) )
		{
			retVal = true;
		}
	}
	return retVal;
}
