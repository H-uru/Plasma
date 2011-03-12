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
#include "hsTypes.h"

#include "../pnFactory/plCreator.h"

#include "plAudible.h"
REGISTER_NONCREATABLE( plAudible );

#include "plDrawable.h"
REGISTER_NONCREATABLE( plDrawable );

#include "plPhysical.h"
REGISTER_NONCREATABLE( plPhysical );

#include "plgDispatch.h"
REGISTER_NONCREATABLE( plDispatchBase );

#include "../pnDispatch/pnDispatchCreatable.h"
#include "../pnKeyedObject/pnKeyedObjectCreatable.h"
#include "../pnMessage/pnMessageCreatable.h"
#include "../pnModifier/pnModifierCreatable.h"
#include "../pnNetCommon/pnNetCommonCreatable.h"
#include "../pnTimer/pnTimerCreatable.h"

#include "../plResMgr/plResMgrCreatable.h"

#include "../plMessage/plResMgrHelperMsg.h"
REGISTER_CREATABLE(plResMgrHelperMsg);

#include "../plAudioCore/plAudioCoreCreatable.h"
