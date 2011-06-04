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

#ifndef plPhysicalCreatable_inc
#define plPhysicalCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plDetectorModifier.h"

REGISTER_NONCREATABLE( plDetectorModifier );

#include "plPickingDetector.h"

REGISTER_CREATABLE( plPickingDetector );

#include "plCollisionDetector.h"

REGISTER_CREATABLE( plCollisionDetector );
REGISTER_CREATABLE( plCameraRegionDetector );
REGISTER_CREATABLE( plObjectInVolumeDetector );
REGISTER_CREATABLE( plObjectInVolumeAndFacingDetector );
REGISTER_CREATABLE( plSubworldRegionDetector );
REGISTER_CREATABLE( plPanicLinkRegion );
REGISTER_CREATABLE( plSimpleRegionSensor );
REGISTER_CREATABLE( plSwimDetector );
REGISTER_CREATABLE( plRidingAnimatedPhysicalDetector );

#include "plPhysicalSDLModifier.h"
REGISTER_CREATABLE( plPhysicalSDLModifier );

#include "plPhysicalSndGroup.h"
REGISTER_CREATABLE(plPhysicalSndGroup);

#endif // plPhysicalCreatable_inc
