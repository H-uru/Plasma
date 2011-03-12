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
#include "plLOSRequestMsg.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"

plLOSRequestMsg::plLOSRequestMsg()
	: fRequestID(0),
	fRequestType(plSimDefs::kLOSDBNone),
	fTestType(kTestAny),
	fReportType(kReportHit),
	fCullDB(plSimDefs::kLOSDBNone),
	fWorldKey(nil)
{
	AddReceiver(hsgResMgr::ResMgr()->FindKey(kLOSObject_KEY));
	SetBCastFlag(plMessage::kPropagateToModifiers);
}

plLOSRequestMsg::plLOSRequestMsg(const plKey& sender, hsPoint3& fromPoint, hsPoint3& toPoint, plSimDefs::plLOSDB db, TestType test, ReportType report)
	: plMessage(sender, hsgResMgr::ResMgr()->FindKey(kLOSObject_KEY), nil),
	fFrom(fromPoint),
	fTo(toPoint),
	fRequestID(0),
	fRequestType(db),
	fTestType(test),
	fReportType(report),
	fCullDB(plSimDefs::kLOSDBNone),
	fWorldKey(nil)
{
	SetBCastFlag(plMessage::kPropagateToModifiers);
}
