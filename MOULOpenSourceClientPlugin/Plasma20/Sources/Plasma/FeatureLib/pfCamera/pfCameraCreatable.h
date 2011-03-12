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

#ifndef pfCameraCreatable_inc
#define pfCameraCreatable_inc

#include "../pnFactory/plCreatable.h"

#include "plCameraBrain.h"

REGISTER_CREATABLE( plCameraBrain1 );
REGISTER_CREATABLE( plCameraBrain1_Drive );
REGISTER_CREATABLE( plCameraBrain1_Avatar );
REGISTER_CREATABLE( plCameraBrain1_FirstPerson);
REGISTER_CREATABLE( plCameraBrain1_Fixed );
REGISTER_CREATABLE( plCameraBrain1_Circle );

#include "plCameraModifier.h"

REGISTER_CREATABLE( plCameraModifier1 );

#include "plInterestingModifier.h"

REGISTER_CREATABLE( plInterestingModifier );

#include "plVirtualCamNeu.h"

REGISTER_CREATABLE( plVirtualCam1 );


#endif // pfCameraCreatable_inc
