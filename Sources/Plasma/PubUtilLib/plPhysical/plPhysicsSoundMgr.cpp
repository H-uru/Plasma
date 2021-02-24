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

#include "plPhysicsSoundMgr.h"
#include "plPhysicalSndGroup.h"

#include <algorithm>
#include <iterator>

#include "plPhysical.h"

#define MIN_VOLUME 0.0001f

void plPhysicsSoundMgr::AddContact(plPhysical* phys1, plPhysical* phys2, const hsPoint3& hitPoint, const hsVector3& hitNormal)
{
    CollidePair cp(phys1->GetKey(), phys2->GetKey(), hitPoint, hitNormal);
    fCurCollisions.insert(cp);
}

void plPhysicsSoundMgr::Update()
{
    // Get all the physicals that only in the new list (started colliding)
    CollideSet startedColliding;
    std::set_difference(fCurCollisions.begin(), fCurCollisions.end(),
        fPrevCollisions.begin(), fPrevCollisions.end(),
        std::inserter(startedColliding, startedColliding.begin()));

    for (CollideSet::iterator it = startedColliding.begin(); it != startedColliding.end(); it++)
        IStartCollision(*it);

    for (CollideSet::iterator it = fPrevCollisions.begin(); it != fPrevCollisions.end(); it++)
    {
        CollideSet::iterator old = fCurCollisions.find(*it);
        if (old != fCurCollisions.end())
        {
            IUpdateCollision(*it);
        }
        else
        {
            IStopCollision(*it);
        }
    }

    fPrevCollisions = fCurCollisions;
    fCurCollisions.clear();
}

void plPhysicsSoundMgr::IStartCollision(const CollidePair& cp)
{
    hsVector3 v1, v2;
    const float strengthThreshold = 20.0f;

    plPhysical* physicalA = cp.FirstPhysical();
    plPhysical* physicalB = cp.SecondPhysical();
    if (!physicalA || !physicalB)
        return;

    plPhysicalSndGroup* sndA = physicalA->GetSoundGroup();
    plPhysicalSndGroup* sndB = physicalB->GetSoundGroup();

    // If no impact sounds were specified in max don't do anything here.
    if (!sndA->HasImpactSound(sndB->GetGroup()) &&
        !sndB->HasImpactSound(sndA->GetGroup()))
        return;

    physicalA->GetLinearVelocitySim(v1);
    physicalB->GetLinearVelocitySim(v2);
    hsVector3 vel = v1 - v2;
    float strength = vel.MagnitudeSquared();
    
    if (strength >= strengthThreshold)
    {
        if (sndA->HasImpactSound(sndB->GetGroup()))
        {
            sndA->PlayImpactSound(sndB->GetGroup());
        }
        else
        {
            sndB->PlayImpactSound(sndA->GetGroup());
        }
    }
}

void plPhysicsSoundMgr::IStopCollision(const CollidePair& cp)
{
    plPhysical* physicalA = cp.FirstPhysical();
    plPhysical* physicalB = cp.SecondPhysical();
    if (physicalA && physicalB)
    {
        plPhysicalSndGroup* sndA = physicalA->GetSoundGroup();
        plPhysicalSndGroup* sndB = physicalB->GetSoundGroup();

        if (sndA->HasSlideSound(sndB->GetGroup()))
        {
            if(sndA->IsSliding())
            {
                sndA->StopSlideSound(sndB->GetGroup()); 
            }
        }
        if (sndB->HasSlideSound(sndA->GetGroup()))
        {
            if(sndB->IsSliding())
            {
                sndB->StopSlideSound(sndA->GetGroup());     
            }
        }
    }
}

void plPhysicsSoundMgr::IUpdateCollision(const CollidePair& cp)
{
    hsVector3 v1, v2;
    plPhysical* physicalA = cp.FirstPhysical();
    plPhysical* physicalB = cp.SecondPhysical();
    if (!physicalA || !physicalB)
        return;

    plPhysicalSndGroup* sndA = physicalA->GetSoundGroup();
    plPhysicalSndGroup* sndB = physicalB->GetSoundGroup();

    physicalA->GetLinearVelocitySim(v1);
    physicalB->GetLinearVelocitySim(v2);
    hsVector3 vel = v1 - v2;
    float strength = vel.MagnitudeSquared();
    
    // scale strength to use as volume
    strength /= 16*8;
    if(strength < MIN_VOLUME)
        strength = 0;
    
    if (sndA->HasSlideSound(sndB->GetGroup()))
        IProcessSlide(sndA, sndB, strength);
    else
        IProcessSlide(sndB, sndA, strength);
}

void plPhysicsSoundMgr::IProcessSlide(plPhysicalSndGroup* sndA, plPhysicalSndGroup* sndB, float strength)
{
    sndA->SetSlideSoundVolume(sndB->GetGroup(), strength);

    if(strength > MIN_VOLUME)
    {
        if(!sndA->IsSliding())
        {
            sndA->PlaySlideSound(sndB->GetGroup());
        }
    }
}

//////////////////////////////////////////////////////////////////////////

plPhysicsSoundMgr::CollidePair::CollidePair(const plKey& firstPhys, const plKey& secondPhys, const hsPoint3& point, const hsVector3& normal)
{
    // To simplify searching and sorting, all pairs are set up with the pointer value of the
    // first element greater than the pointer value of the second.
    if (firstPhys->GetObjectPtr() < secondPhys->GetObjectPtr())
    {
        this->firstPhysKey = secondPhys;
        this->secondPhysKey = firstPhys;
    }
    else
    {
        this->firstPhysKey = firstPhys;
        this->secondPhysKey = secondPhys;
    }

    this->point = point;
    this->normalForce = normal;
}

bool plPhysicsSoundMgr::CollidePair::operator<(const CollidePair& rhs) const
{
    return (firstPhysKey->GetObjectPtr() < rhs.firstPhysKey->GetObjectPtr() 
        || (rhs.firstPhysKey->GetObjectPtr() == firstPhysKey->GetObjectPtr() && secondPhysKey->GetObjectPtr() < rhs.secondPhysKey->GetObjectPtr()));
}

bool plPhysicsSoundMgr::CollidePair::operator==(const CollidePair& rhs) const
{
    if (firstPhysKey == rhs.firstPhysKey && secondPhysKey == rhs.secondPhysKey)
        return true;
    return false;
}

plPhysical* plPhysicsSoundMgr::CollidePair::FirstPhysical() const
{
    return firstPhysKey ? static_cast<plPhysical*>(firstPhysKey->GetObjectPtr()) : nullptr;
}

plPhysical* plPhysicsSoundMgr::CollidePair::SecondPhysical() const
{
    return secondPhysKey ? static_cast<plPhysical*>(secondPhysKey->GetObjectPtr()) : nullptr;
}
