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

#ifndef plPXSimDefs_H
#define plPXSimDefs_H

/**
 * \file plPXSimDefs.h
 * Private include file to convert plSimDefs to PhysX filter data.
 */

#include "plPhysXAPI.h"

#include "plPhysical/plSimDefs.h"

static_assert(plSimDefs::kGroupMax - 1 <= sizeof(physx::PxFilterData::word0) * 8,
              "plSimDefs::Group bitmask must fit in a PxFilterData word");

class plPXFilterData : public physx::PxFilterData
{
protected:
    void IToggleWord(physx::PxU32& dest, physx::PxU32 mask, bool on)
    {
        if (on)
            dest |= mask;
        else
            dest &= ~mask;
    }

public:
    plPXFilterData() : physx::PxFilterData() { }
    plPXFilterData(const physx::PxFilterData& copy) : physx::PxFilterData(copy) { }

public:
    physx::PxU32 GetGroupMask() const { return word0; }
    physx::PxU32 GetReportOn() const { return word1; }

public:
    void SetGroups(physx::PxU32 groups)
    {
        word0 = groups;
    }

    void SetReportOn(physx::PxU32 reportOn)
    {
        word1 = reportOn;
    }

    void SetLOSDBs(plSimDefs::plLOSDB losDBs)
    {
        word2 = (physx::PxU32)losDBs;
    }

    /**
     * Sets the generic flag bitfield.
     * Generic flags are an implementation detail and should only be relied on by either
     * the main simulation or the owner of the actor. Here be dragons.
     */
    void SetFlags(uint32_t flag)
    {
        word3 = flag;
    }

    bool TestGroup(plSimDefs::Group group) const
    {
        return (word0 & (1 << (physx::PxU32)group));
    }

    bool TestReportOn(plSimDefs::Group reportOn) const
    {
        return (word1 & (1 << (physx::PxU32)reportOn));
    }

    bool TestLOSDBs(plSimDefs::plLOSDB losDBs) const
    {
        return word2 & losDBs;
    }

    /**
     * Tests a generic flag bit.
     * Generic flags are an implementation detail and should only be relied on by either
     * the main simulation or the owner of the actor. Here be dragons.
     */
    bool TestFlag(uint32_t flag) const
    {
        return (word3 & flag);
    }

    /** Toggles all member/collision groups on or off. */
    void ToggleAllGroups(bool on=true)
    {
        for (physx::PxU32 i = 0; i < plSimDefs::kGroupMax; ++i)
            IToggleWord(word0, 1 << i, on);
    }

    /** Toggles all report groups on or off. */
    void ToggleAllReportOn(bool on=true)
    {
        for (physx::PxU32 i = 0; i < plSimDefs::kGroupMax; ++i)
            IToggleWord(word1, 1 << i, on);
    }

    /** Toggles all LOSDBs on or off. */
    void ToggleAllLOSDBs(bool on=true)
    {
        for (physx::PxU32 bit = 1<<0; bit < plSimDefs::kLOSDBMax; bit <<= 1)
            IToggleWord(word2, bit, on);
    }

    void ToggleGroup(plSimDefs::Group group, bool on=true)
    {
        IToggleWord(word0, 1 << (physx::PxU32)group, on);
    }

    void ToggleReportOn(plSimDefs::Group reportOn, bool on=true)
    {
        IToggleWord(word1, 1 << (physx::PxU32)reportOn, on);
    }

    void ToggleLOSDB(plSimDefs::plLOSDB db, bool on=true)
    {
        IToggleWord(word2, db, on);
    }

    /**
     * Toggles a generic flag bit.
     * Generic flags are an implementation detail and should only be relied on by either
     * the main simulation or the owner of the actor. Here be dragons.
     */
    void ToggleFlag(uint32_t flag, bool on=true)
    {
        IToggleWord(word3, flag, on);
    }

public:
    /** Changes the query and simulation group of a single actor. */
    static void SetActorGroup(physx::PxRigidActor* actor, plSimDefs::Group group)
    {
        physx::PxShape* shape;
        actor->getShapes(&shape, 1);
        {
            plPXFilterData filter = shape->getSimulationFilterData();
            filter.SetGroups(1 << group);
            shape->setSimulationFilterData(filter);
        }
        {
            plPXFilterData filter = shape->getQueryFilterData();
            filter.SetGroups(1 << group);
            shape->setQueryFilterData(filter);
        }
    }

    /**
     * Changes the query and simulation generic flags of a single actor.
     * Generic flags are an implementation detail and should only be relied on by either
     * the main simulation or the owner of the actor. Here be dragons.
     */
    static void SetActorFlags(physx::PxRigidActor* actor, uint32_t flags)
    {
        physx::PxShape* shape;
        actor->getShapes(&shape, 1);
        {
            plPXFilterData filter = shape->getSimulationFilterData();
            filter.SetFlags(flags);
            shape->setSimulationFilterData(filter);
        }
        {
            plPXFilterData filter = shape->getQueryFilterData();
            filter.SetFlags(flags);
            shape->setQueryFilterData(filter);
        }
    }

    /**
     * Initializes the filter data for a single actor.
     */
    static void Initialize(physx::PxRigidActor* actor, plSimDefs::Group collideGroup,
                           physx::PxU32 reportOnMask, plSimDefs::plLOSDB losDBs,
                           uint32_t flags=0)
    {
        plPXFilterData data;
        data.ToggleGroup(collideGroup);
        data.SetReportOn(reportOnMask);
        data.SetLOSDBs(losDBs);
        data.SetFlags(flags);

        physx::PxShape* shape;
        actor->getShapes(&shape, 1);
        shape->setSimulationFilterData(data);
        shape->setQueryFilterData(data);
    }
};

#endif
