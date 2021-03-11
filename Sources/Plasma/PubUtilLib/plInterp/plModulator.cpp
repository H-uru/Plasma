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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "plModulator.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsGeometry3.h"
#include "hsBounds.h"

#include "plController.h"

#include "plIntersect/plVolumeIsect.h"

plModulator::plModulator()
:   fVolume(),
    fSoftDist()
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
float plModulator::Modulation(const hsBounds3Ext& bnd) const
{
    return Modulation(bnd.GetCenter());
}

float plModulator::Modulation(const hsPoint3& pos) const
{
    hsAssert(fVolume, "Modulator with no Volume is pretty useless");

    float dist = fVolume->Test(pos);

    float retVal;
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
    fSoftDist = s->ReadLEFloat();
}

void plModulator::Write(hsStream* s, hsResMgr* mgr)
{
    mgr->WriteCreatable(s, fVolume);
    s->WriteLEFloat(fSoftDist);
}


