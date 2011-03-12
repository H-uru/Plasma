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
#include "../plMessage/plSimInfluenceMsg.h"
#include "../../CoreLib/hsStream.h"
/*

void plSimInfluenceMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimulationMsg::Read(stream, mgr);
}

void plSimInfluenceMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plSimulationMsg::Write(stream, mgr);
}


// PLFORCEMSG
void plForceMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fForce.Read(stream);
}

void plForceMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fForce.Write(stream);
}

// PLOFFSETFORCEMSG
void plOffsetForceMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plForceMsg::Read(stream, mgr);

	fPoint.Read(stream);
}

void plOffsetForceMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plForceMsg::Write(stream, mgr);

	fPoint.Write(stream);
}


// PLTORQUEMSG
void plTorqueMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fTorque.Read(stream);
}

void plTorqueMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fTorque.Write(stream);
}


// PLIMPULSE
void plImpulseMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fImpulse.Read(stream);
}

void plImpulseMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fImpulse.Write(stream);
}


// PLOFFSETIMPULSE
void plOffsetImpulseMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fPoint.Read(stream);
}

void plOffsetImpulseMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fPoint.Write(stream);
}


// PLANGULARIMPULSE
void plAngularImpulseMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fImpulse.Read(stream);
}

void plAngularImpulseMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fImpulse.Write(stream);
}


// PLDAMPMSG
void plDampMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	stream->WriteSwapScalar(fDamp);
}

void plDampMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fDamp = stream->ReadSwapScalar();
}


// PLSHIFTCENTERMSG
void plShiftMassMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Read(stream, mgr);

	fOffset.Read(stream);
}

void plShiftMassMsg::Write(hsStream * stream, hsResMgr *mgr)
{
	plSimInfluenceMsg::Write(stream, mgr);

	fOffset.Write(stream);
}
*/