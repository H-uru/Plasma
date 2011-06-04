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

#ifndef plConditionalObjectCreatable_inc
#define plConditionalObjectCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plANDConditionalObject.h"

REGISTER_CREATABLE( plANDConditionalObject );

#include "plORConditionalObject.h"

REGISTER_CREATABLE( plORConditionalObject );

#include "plPickedConditionalObject.h"

REGISTER_CREATABLE( plPickedConditionalObject );

#include "plActivatorConditionalObject.h"

REGISTER_CREATABLE( plActivatorConditionalObject );
REGISTER_CREATABLE( plActivatorActivatorConditionalObject );
REGISTER_CREATABLE( plVolActivatorConditionalObject );

#include "plKeyPressConditionalObject.h"

REGISTER_CREATABLE( plKeyPressConditionalObject );

#include "plAnimationEventConditionalObject.h"

REGISTER_CREATABLE( plAnimationEventConditionalObject );

#include "plControlEventConditionalObject.h"

REGISTER_CREATABLE( plControlEventConditionalObject );

#include "plObjectInBoxConditionalObject.h"

REGISTER_CREATABLE( plObjectInBoxConditionalObject );
REGISTER_CREATABLE( plVolumeSensorConditionalObject );
REGISTER_CREATABLE( plVolumeSensorConditionalObjectNoArbitration  );

#include "plLocalPlayerInBoxConditionalObject.h"

REGISTER_CREATABLE( plLocalPlayerInBoxConditionalObject );

#include "plObjectIntersectPlaneConditionalObject.h"

REGISTER_CREATABLE( plObjectIntersectPlaneConditionalObject );

#include "plLocalPlayerIntersectPlaneConditionalObject.h"

REGISTER_CREATABLE( plLocalPlayerIntersectPlaneConditionalObject );

#include "plFacingConditionalObject.h"

REGISTER_CREATABLE( plFacingConditionalObject );

#endif // plConditionalObjectCreatable_inc
