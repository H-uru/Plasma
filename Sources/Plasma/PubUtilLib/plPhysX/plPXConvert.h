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
#include "hsQuat.h"
#include "hsMatrix44.h"

#include <NxVec3.h>
#include <NxQuat.h>
#include <NxMat34.h>

// Converts to and from the PhysX types
namespace plPXConvert
{
    // The following conversions are just casts, due to the fact that the Plasma
    // and PhysX vector and quat classes don't have any virtual fuctions and have
    // all their data in the same offsets.
    inline NxVec3&         Point(hsPoint3& vec)         { return *((NxVec3*)&vec); }
    inline const NxVec3&   Point(const hsPoint3& vec)   { return *((NxVec3*)&vec); }
    inline hsPoint3&       Point(NxVec3& vec)           { return *((hsPoint3*)&vec); }
    inline const hsPoint3& Point(const NxVec3& vec)     { return *((hsPoint3*)&vec); }

    inline NxVec3&          Vector(hsVector3& vel)      { return *((NxVec3*)&vel); }
    inline const NxVec3&    Vector(const hsVector3& vel){ return *((NxVec3*)&vel); }
    inline hsVector3&       Vector(NxVec3& vec)         { return *((hsVector3*)&vec); }
    inline const hsVector3& Vector(const NxVec3& vec)   { return *((hsVector3*)&vec); }

    inline const NxQuat& Quat(const hsQuat& quat) { return *((NxQuat*)&quat); }
    inline const hsQuat& Quat(const NxQuat& quat) { return *((hsQuat*)&quat); }

    // The matrix data doesn't match up, so we have to convert it
    inline void Matrix(const hsMatrix44& fromMat, NxMat34& toMat) { toMat.setRowMajor44(&fromMat.fMap[0][0]); }
    inline void Matrix(const NxMat34& fromMat, hsMatrix44& toMat) { toMat.NotIdentity(); fromMat.getRowMajor44(&toMat.fMap[0][0]); }

};

#endif // plPXConvert_h_inc