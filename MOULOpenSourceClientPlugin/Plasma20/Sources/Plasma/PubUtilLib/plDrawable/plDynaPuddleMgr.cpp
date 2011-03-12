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
#include "plDynaPuddleMgr.h"

#include "plPrintShape.h"

#include "plgDispatch.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "../plMessage/plAvatarFootMsg.h"

#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plArmatureMod.h"

static const UInt32 kNumPrintIDs = 2;
static const UInt32 kPrintIDs[kNumPrintIDs] =
{
	plAvBrainHuman::RFootPrint,
	plAvBrainHuman::LFootPrint
};


plDynaPuddleMgr::plDynaPuddleMgr()
{
	fPartIDs.SetCount(kNumPrintIDs);
	int i;
	for( i = 0; i < kNumPrintIDs; i++ )
		fPartIDs[i] = kPrintIDs[i];
}

plDynaPuddleMgr::~plDynaPuddleMgr()
{
}

void plDynaPuddleMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaRippleMgr::Read(stream, mgr);

	plgDispatch::Dispatch()->RegisterForExactType(plAvatarFootMsg::Index(), GetKey());
}

hsBool plDynaPuddleMgr::MsgReceive(plMessage* msg)
{
	plAvatarFootMsg* footMsg = plAvatarFootMsg::ConvertNoRef(msg);
	if( footMsg )
	{
		int i;
		for( i = 0; i < fPartIDs.GetCount(); i++ )
		{
			plArmatureMod* armMod = footMsg->GetArmature();
			const plPrintShape* shape = IGetPrintShape(armMod, fPartIDs[i]);
			if( shape )
			{
				plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
				if( IRippleFromShape(shape, true) )
				{
					INotifyActive(info, armMod->GetKey(), fPartIDs[i]);
				}
				else
				{
					INotifyInactive(info, armMod->GetKey(), fPartIDs[i]);
				}
			}
		}
		return true;
	}

	return plDynaRippleMgr::MsgReceive(msg);
}

