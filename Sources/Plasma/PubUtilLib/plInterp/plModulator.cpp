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
#include "plModulator.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsGeometry3.h"
#include "hsBounds.h"

#include "plController.h"

#include "../plIntersect/plVolumeIsect.h"

plModulator::plModulator()
:	fVolume(nil),
	fSoftDist(0)
{
}

plModulator::~plModulator()
{
	delete fVolume;
}

void plModulator::SetVolume(plVolumeIsect* vol)
{
	delete fVolume;
	fVolume = vol;
}

void plModulator::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	hsAssert(fVolume, "Modulator with no Volume is pretty useless");

	fVolume->SetTransform(l2w, w2l);
}

// Volume - Want to base this on the closest point on the bounds, instead of just the center.
hsScalar plModulator::Modulation(const hsBounds3Ext& bnd) const
{
	return Modulation(bnd.GetCenter());
}

hsScalar plModulator::Modulation(const hsPoint3& pos) const
{
	hsAssert(fVolume, "Modulator with no Volume is pretty useless");

	hsScalar dist = fVolume->Test(pos);

	hsScalar retVal;
	if( dist > 0 )
	{
		if( dist < fSoftDist )
		{
			dist /= fSoftDist;
			retVal = 1.f - dist;
		}
		else
		{
			retVal = 0;
		}
	}
	else
	{
		retVal = 1.f;
	}

	return retVal;
}

void plModulator::Read(hsStream* s, hsResMgr* mgr)
{
	fVolume = plVolumeIsect::ConvertNoRef(mgr->ReadCreatable(s));
	fSoftDist = s->ReadSwapScalar();
}

void plModulator::Write(hsStream* s, hsResMgr* mgr)
{
	mgr->WriteCreatable(s, fVolume);
	s->WriteSwapScalar(fSoftDist);
}


