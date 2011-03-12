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

#ifndef plIntersectCreatable_inc
#define plIntersectCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plVolumeIsect.h"

REGISTER_NONCREATABLE( plVolumeIsect );

REGISTER_CREATABLE( plSphereIsect );
REGISTER_CREATABLE( plConeIsect );
REGISTER_CREATABLE( plCylinderIsect );
REGISTER_CREATABLE( plParallelIsect );
REGISTER_CREATABLE( plConvexIsect );

REGISTER_CREATABLE( plBoundsIsect );

REGISTER_NONCREATABLE( plComplexIsect );

REGISTER_CREATABLE( plUnionIsect );
REGISTER_CREATABLE( plIntersectionIsect );

#include "plRegionBase.h"

REGISTER_NONCREATABLE( plRegionBase );

#include "plSoftVolume.h"

REGISTER_NONCREATABLE( plSoftVolume );

#include "plSoftVolumeTypes.h"

REGISTER_CREATABLE( plSoftVolumeSimple );

REGISTER_NONCREATABLE( plSoftVolumeComplex );

REGISTER_CREATABLE( plSoftVolumeUnion );
REGISTER_CREATABLE( plSoftVolumeIntersect );
REGISTER_CREATABLE( plSoftVolumeInvert );

#include "plHardRegion.h"

REGISTER_NONCREATABLE( plHardRegion );

#include "plHardRegionPlanes.h"

REGISTER_CREATABLE( plHardRegionPlanes );

#include "plHardRegionTypes.h"

REGISTER_NONCREATABLE( plHardRegionComplex );

REGISTER_CREATABLE( plHardRegionUnion );
REGISTER_CREATABLE( plHardRegionIntersect );
REGISTER_CREATABLE( plHardRegionInvert );

#endif // plIntersectCreatable_inc
