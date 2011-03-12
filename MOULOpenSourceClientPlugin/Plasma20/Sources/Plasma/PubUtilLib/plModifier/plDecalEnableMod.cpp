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
#include "plDecalEnableMod.h"
#include "../plMessage/plDynaDecalEnableMsg.h"
#include "../plMessage/plCollideMsg.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../plAvatar/plArmatureMod.h"

#include "hsTimer.h"
#include "hsStream.h"
#include "hsResMgr.h"

plDecalEnableMod::plDecalEnableMod()
{
}

plDecalEnableMod::~plDecalEnableMod()
{
}

hsBool plDecalEnableMod::MsgReceive(plMessage* msg)
{
	plCollideMsg* coll = plCollideMsg::ConvertNoRef(msg);
	if( coll )
	{
		plSceneObject* obj = plSceneObject::ConvertNoRef(coll->fOtherKey->ObjectIsLoaded());
		if( !obj )
			return true;

		const plArmatureMod* arm = (const plArmatureMod*)obj->GetModifierByType(plArmatureMod::Index());
		if( !arm )
			return true;

		plKey armKey = arm->GetKey();

		int i;
		for( i = 0; i < fDecalMgrs.GetCount(); i++ )
		{
			plDynaDecalEnableMsg* ena = TRACKED_NEW plDynaDecalEnableMsg(fDecalMgrs[i], armKey, hsTimer::GetSysSeconds(), fWetLength, !coll->fEntering);

			ena->Send();
		}
		return true;
	}

	return plSingleModifier::MsgReceive(msg);
}
	
void plDecalEnableMod::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	int n = stream->ReadSwap32();
	fDecalMgrs.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fDecalMgrs[i] = mgr->ReadKey(stream);

	fWetLength = stream->ReadSwapScalar();
}

void plDecalEnableMod::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteSwap32(fDecalMgrs.GetCount());

	int i;
	for( i = 0; i < fDecalMgrs.GetCount(); i++ )
		mgr->WriteKey(stream, fDecalMgrs[i]);

	stream->WriteSwapScalar(fWetLength);
}
