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
#include "plDynaDecalEnableMsg.h"

#include "hsResMgr.h"
#include "hsStream.h"

plDynaDecalEnableMsg::plDynaDecalEnableMsg()
:	plMessage(nil, nil, nil),
	fKey(nil),
	fFlags(0),
	fConTime(0),
	fID(UInt32(-1))
{
}

plDynaDecalEnableMsg::plDynaDecalEnableMsg(const plKey& r, const plKey& a, double t, hsScalar w, hsBool end, UInt32 id, hsBool isArm)
:	plMessage(nil, r, nil),
	fKey(a),
	fFlags(0),
	fConTime(t),
	fWetLength(w),
	fID(id)
{
	if( end )
		fFlags |= kAtEnd;
	if( isArm )
		fFlags |= kArmature;
}

plDynaDecalEnableMsg::~plDynaDecalEnableMsg()
{
}

void plDynaDecalEnableMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	IMsgRead(stream, mgr);

	fKey = mgr->ReadKey(stream);

	fConTime = stream->ReadSwapDouble();

	fFlags = stream->ReadSwap32();

	fID = stream->ReadSwap32();
}

void plDynaDecalEnableMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	IMsgWrite(stream, mgr);

	mgr->WriteKey(stream, fKey);

	stream->WriteSwapDouble(fConTime);

	stream->WriteSwap32(fFlags);

	stream->WriteSwap32(fID);
}

