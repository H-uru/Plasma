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
#ifndef plPXConvert_h_inc
#define plPXConvert_h_inc

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsQuat.h"

#include <characterkinematic/PxExtended.h>
#include <foundation/PxQuat.h>
#include <foundation/PxTransform.h>
#include <foundation/PxVec3.h>

namespace plPXConvert
{
    // The following conversions are just casts, due to the fact that the Plasma
    // and PhysX vector and quat classes don't have any virtual fuctions and have
    // all their data in the same offsets.
    inline physx::PxVec3&       Point(hsPoint3& vec)            { return *((physx::PxVec3*)&vec); }
    inline const physx::PxVec3& Point(const hsPoint3& vec)      { return *((physx::PxVec3*)&vec); }
    inline hsPoint3&            Point(physx::PxVec3& vec)       { return *((hsPoint3*)&vec); }
    inline const hsPoint3&      Point(const physx::PxVec3& vec) { return *((hsPoint3*)&vec); }

    inline physx::PxVec3&       Vector(hsVector3& vel)           { return *((physx::PxVec3*)&vel); }
    inline const physx::PxVec3& Vector(const hsVector3& vel)     { return *((physx::PxVec3*)&vel); }
    inline hsVector3&           Vector(physx::PxVec3& vec)       { return *((hsVector3*)&vec); }
    inline const hsVector3&     Vector(const physx::PxVec3& vec) { return *((hsVector3*)&vec); }

    inline const physx::PxQuat& Quat(const hsQuat& quat) { return *((physx::PxQuat*)&quat); }
    inline const hsQuat& Quat(const physx::PxQuat& quat) { return *((hsQuat*)&quat); }

    inline physx::PxExtendedVec3 ExtPoint(const hsPoint3& vec)
    {
        return { vec.fX, vec.fY, vec.fZ };
    }

    inline hsPoint3 ExtPoint(const physx::PxExtendedVec3& vec)
    {
        return { (float)vec.x, (float)vec.y, (float)vec.z };
    }

    inline physx::PxTransform Transform(const hsPoint3& p, hsQuat q)
    {
        q.Normalize();
        return physx::PxTransform(Point(p), Quat(q));
    }

    inline physx::PxTransform Transform(const hsMatrix44& mat)
    {
        hsPoint3 p;
        hsQuat q;
        mat.DecompRigid(p, q);
        q.Normalize();
        return Transform(p, q);
    }

    inline hsMatrix44 Transform(const physx::PxTransform& pose)
    {
        return hsMatrix44(Point(pose.p), Quat(pose.q));
    }
};

#endif // plPXConvert_h_inc