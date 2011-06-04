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

#ifndef plGLightCreatable_inc
#define plGLightCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plLightInfo.h"

REGISTER_NONCREATABLE( plLightInfo );
REGISTER_CREATABLE( plDirectionalLightInfo );
REGISTER_CREATABLE( plLimitedDirLightInfo );
REGISTER_CREATABLE( plOmniLightInfo );
REGISTER_CREATABLE( plSpotLightInfo );

#include "plLightSpace.h"

REGISTER_CREATABLE( plLightSpace );

#include "plShadowMaster.h"

REGISTER_NONCREATABLE( plShadowMaster );

#include "plPointShadowMaster.h"

REGISTER_CREATABLE( plPointShadowMaster );

#include "plDirectShadowMaster.h"

REGISTER_CREATABLE( plDirectShadowMaster );

#include "plShadowCaster.h"

REGISTER_CREATABLE( plShadowCaster );

#endif // plGLightCreatable_inc
