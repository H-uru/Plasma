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
#ifndef plNetCommonCreatable_inc
#define plNetCommonCreatable_inc

#include "../pnFactory/plCreator.h"

#ifndef SERVER
#include "plNetMember.h"
REGISTER_NONCREATABLE( plNetMember );
#endif // SERVER

#include "plNetCommonHelpers.h"
#ifndef SERVER
REGISTER_CREATABLE( plNetCoreStatsSummary );
#endif // SERVER
REGISTER_CREATABLE( plCreatableListHelper );

// HACK: plUUID should have it's own creatable include
#include "../plUUID/plUUID.h"
REGISTER_CREATABLE( plCreatableUuid );

#include "plClientGuid.h"
REGISTER_CREATABLE( plClientGuid );
#include "plNetServerSessionInfo.h"
REGISTER_CREATABLE( plNetServerSessionInfo );
REGISTER_CREATABLE( plAgeInfoStruct );
REGISTER_CREATABLE( plAgeLinkStruct );

#endif // plNetCommonCreatable_inc


