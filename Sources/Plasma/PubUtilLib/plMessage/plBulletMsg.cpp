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

#include "hsStream.h"

#include "plBulletMsg.h"
#include "hsFastMath.h"

void plBulletMsg::Read(hsStream* stream, hsResMgr* mgr) 
{ 
	IMsgRead(stream, mgr); 

	fCmd = Cmd(stream->ReadByte());

	fFrom.Read(stream);
	fDir.Read(stream);
	fRange = stream->ReadSwapScalar();
	fRadius = stream->ReadSwapScalar();
	fPartyTime = stream->ReadSwapScalar();
}

void plBulletMsg::Write(hsStream* stream, hsResMgr* mgr) 
{ 
	IMsgWrite(stream, mgr); 

	stream->WriteByte(UInt8(fCmd));

	fFrom.Write(stream);
	fDir.Write(stream);
	stream->WriteSwapScalar(fRange);
	stream->WriteSwapScalar(fRadius);
	stream->WriteSwapScalar(fPartyTime);
}

void plBulletMsg::FireShot(const hsPoint3& from, const hsVector3& dir, hsScalar radius, hsScalar range, hsScalar psecs)
{
	fFrom = from;
	fDir = dir;
	fRange = range;
	fRadius = radius;
	fPartyTime = psecs;

	fCmd = kShot;
}

void plBulletMsg::FireShot(const hsPoint3& from, const hsPoint3& at, hsScalar radius, hsScalar psecs)
{
	hsVector3 dir(&at, &from);
	hsScalar invLen = hsFastMath::InvSqrt(dir.MagnitudeSquared());
	hsAssert(invLen > 0, "degenerate from and at to fire bullet");
	dir *= invLen;
	hsScalar range = 1.f / invLen;

	FireShot(from, dir, radius, range, psecs);
}

