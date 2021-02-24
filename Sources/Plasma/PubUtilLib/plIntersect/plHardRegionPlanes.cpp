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
#include "plHardRegionPlanes.h"

#include "hsStream.h"
#include "hsGeometry3.h"
#include "hsFastMath.h"
#include "hsMatrix44.h"


plHardRegionPlanes::plHardRegionPlanes()
{
}

plHardRegionPlanes::~plHardRegionPlanes()
{
}

bool plHardRegionPlanes::IIsInside(const hsPoint3& pos) const
{
    for (const HardPlane& plane : fPlanes)
    {
        if (plane.fWorldNorm.InnerProduct(pos) > plane.fWorldDist)
            return false;
    }
    return true;
}

bool plHardRegionPlanes::ICameraInside() const
{
    return IIsInside(fCamPos);
}

void plHardRegionPlanes::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    for (HardPlane& plane : fPlanes)
    {
        plane.fWorldPos = l2w * plane.fPos;

        // Normal gets transpose of inverse.
        plane.fWorldNorm.fX = w2l.fMap[0][0] * plane.fNorm.fX
                                + w2l.fMap[1][0] * plane.fNorm.fY
                                + w2l.fMap[2][0] * plane.fNorm.fZ;

        plane.fWorldNorm.fY = w2l.fMap[0][1] * plane.fNorm.fX
                                + w2l.fMap[1][1] * plane.fNorm.fY
                                + w2l.fMap[2][1] * plane.fNorm.fZ;

        plane.fWorldNorm.fZ = w2l.fMap[0][2] * plane.fNorm.fX
                                + w2l.fMap[1][2] * plane.fNorm.fY
                                + w2l.fMap[2][2] * plane.fNorm.fZ;

        hsFastMath::NormalizeAppr(plane.fWorldNorm);

        plane.fWorldDist = plane.fWorldNorm.InnerProduct(plane.fWorldPos);
    }
}

void plHardRegionPlanes::Read(hsStream* s, hsResMgr* mgr)
{
    plHardRegion::Read(s, mgr);

    uint32_t n = s->ReadLE32();
    fPlanes.resize(n);

    for (HardPlane& plane : fPlanes)
    {
        plane.fNorm.Read(s);
        plane.fPos.Read(s);

        plane.fWorldNorm.Read(s);
        plane.fWorldPos.Read(s);

        plane.fWorldDist = plane.fWorldNorm.InnerProduct(plane.fWorldPos);
    }
}

void plHardRegionPlanes::Write(hsStream* s, hsResMgr* mgr)
{
    plHardRegion::Write(s, mgr);

    s->WriteLE32((uint32_t)fPlanes.size());

    for (const HardPlane& plane : fPlanes)
    {
        plane.fNorm.Write(s);
        plane.fPos.Write(s);

        plane.fWorldNorm.Write(s);
        plane.fWorldPos.Write(s);
    }
}

void plHardRegionPlanes::AddPlane(const hsVector3& n, const hsPoint3& p)
{
    hsVector3 nNorm = n;
    hsFastMath::Normalize(nNorm);

    // First, make sure some idiot isn't adding the same plane in twice.
    // Also, look for the degenerate case of two parallel planes. In that
    // case, take the outer.
    for (HardPlane& plane : fPlanes)
    {
        const float kCloseToOne = 1.f - 1.e-4f;
        if (plane.fNorm.InnerProduct(nNorm) >= kCloseToOne)
        {
            float newDist = nNorm.InnerProduct(p);
            float oldDist = plane.fNorm.InnerProduct(plane.fPos);
            if( newDist > oldDist )
            {
                plane.fPos = p;
            }
            return;
        }
    }
    HardPlane plane;
    plane.fWorldNorm = plane.fNorm = nNorm;
    plane.fWorldPos = plane.fPos = p;
    plane.fWorldDist = plane.fWorldNorm.InnerProduct(plane.fWorldPos);

    fPlanes.emplace_back(plane);
}
