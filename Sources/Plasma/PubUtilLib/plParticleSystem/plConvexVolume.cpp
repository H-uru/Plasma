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

#include "plConvexVolume.h"

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsStream.h"

plConvexVolume::plConvexVolume()
{
    fLocalPlanes = nullptr;
    fWorldPlanes = nullptr;
    fNumPlanes = 0;
}

plConvexVolume::~plConvexVolume()
{
    IClear();
}

void plConvexVolume::IClear()
{
    delete [] fLocalPlanes;
    delete [] fWorldPlanes;
}

bool plConvexVolume::AddPlane(const hsPlane3 &plane)
{
    // First check for a redundant plane (since we're convex, a comparison of normals should do)
    int i;
    // Start the comparison with the most recently added plane, it's most likely to match
    for (i = fNumPlanes - 1; i >= 0; i--)
    {
        const float MIN_COS_THETA = 0.99999f; // translates to < 0.25 degree angle
        // If the angle betwen the normals is close enough, count them as equal.        
        if (fLocalPlanes[i].fN.InnerProduct(plane.fN) >= MIN_COS_THETA)
            return false; // no need to add it
    }
    fNumPlanes++;

    hsPlane3 *tempPlanes = new hsPlane3[fNumPlanes];
    for (i = 0; i < fNumPlanes - 1; i++)
    {
        tempPlanes[i] = fLocalPlanes[i];
    }
    tempPlanes[fNumPlanes - 1] = plane;

    delete [] fLocalPlanes;
    fLocalPlanes = tempPlanes;
    delete [] fWorldPlanes;
    fWorldPlanes = new hsPlane3[fNumPlanes];
    
    return true;
}

void plConvexVolume::Update(const hsMatrix44 &l2w)
{
    int i;
    hsPoint3 planePt;
    for (i = 0; i < fNumPlanes; i++)
    {
        // Since fN is an hsVector3, it will only apply the rotational aspect of the transform...
        fWorldPlanes[i].fN = l2w * fLocalPlanes[i].fN; 
        hsVector3 tmp = fLocalPlanes[i].fN * fLocalPlanes[i].fD;
        planePt.Set(&tmp);
        fWorldPlanes[i].fD = -(l2w * planePt).InnerProduct(fWorldPlanes[i].fN);
    }
}

void plConvexVolume::SetNumPlanesAndClear(const uint32_t num)
{
    IClear();
    fLocalPlanes = new hsPlane3[num];
    fWorldPlanes = new hsPlane3[num];
    fNumPlanes = num;
}

void plConvexVolume::SetPlane(const hsPlane3 &plane, const uint32_t index)
{
    fLocalPlanes[index] = plane;
}

bool plConvexVolume::IsInside(const hsPoint3 &pos) const
{
    int i;
    for( i = 0; i < fNumPlanes; i++ )
    {
        if (!TestPlane(pos, fWorldPlanes[i]))
            return false;
    }

    return true;
}

bool plConvexVolume::ResolvePoint(hsPoint3 &pos) const
{
    float minDist = 1.e33f;
    int32_t minIndex = -1;

    float currDist;
    int i;
    for (i = 0; i < fNumPlanes; i++)
    {
        currDist = -fWorldPlanes[i].fD - fWorldPlanes[i].fN.InnerProduct(pos);
        if (currDist < 0)
            return false; // We're not inside this plane, and thus outside the volume

        if (currDist < minDist)
        {
            minDist = currDist;
            minIndex = i;
        }
    }
    pos += (-fWorldPlanes[minIndex].fD - fWorldPlanes[minIndex].fN.InnerProduct(pos)) * fWorldPlanes[minIndex].fN;
    return true;
}

bool plConvexVolume::BouncePoint(hsPoint3 &pos, hsVector3 &velocity, float bounce, float friction) const
{
    float minDist = 1.e33f;
    int32_t minIndex = -1;

    float currDist;
    int i;
    for (i = 0; i < fNumPlanes; i++)
    {
        currDist = -fWorldPlanes[i].fD - fWorldPlanes[i].fN.InnerProduct(pos);
        if (currDist < 0)
            return false; // We're not inside this plane, and thus outside the volume

        if (currDist < minDist)
        {
            minDist = currDist;
            minIndex = i;
        }
    }
    pos += (-fWorldPlanes[minIndex].fD - fWorldPlanes[minIndex].fN.InnerProduct(pos)) * fWorldPlanes[minIndex].fN;
    hsVector3 bnc = -velocity.InnerProduct(fWorldPlanes[minIndex].fN) * fWorldPlanes[minIndex].fN;
    velocity += bnc;
    velocity *= 1.f - friction;
    velocity += bnc * bounce;
//  velocity += (velocity.InnerProduct(fWorldPlanes[minIndex].fN) * -(1.f + bounce)) * fWorldPlanes[minIndex].fN;
    return true;
}

void plConvexVolume::Read(hsStream* s, hsResMgr *mgr)
{
    SetNumPlanesAndClear(s->ReadLE32());
    int i;
    for (i = 0; i < fNumPlanes; i++)
    {
        fLocalPlanes[i].Read(s);
    }
}

void plConvexVolume::Write(hsStream* s, hsResMgr *mgr)
{
    s->WriteLE32(fNumPlanes);
    int i;
    for (i = 0; i < fNumPlanes; i++)
    {
        fLocalPlanes[i].Write(s);
    }
}
