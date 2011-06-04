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

#ifndef pfMessageCreatable_inc
#define pfMessageCreatable_inc

#include "../pnFactory/plCreator.h"

#if 0
#include "plTriggerMsg.h"

REGISTER_CREATABLE( plTriggerMsg );

#include "plPlayerMsg.h"

REGISTER_CREATABLE( plPlayerMsg );

#include "plAvatarMsg.h"

REGISTER_CREATABLE( plAvatarMsg );
REGISTER_NONCREATABLE( plAvTaskMsg );
REGISTER_CREATABLE( plAvSeekMsg );
REGISTER_CREATABLE( plAvOneShotMsg );
REGISTER_CREATABLE( plAvEnableMsg );
#endif


#include "pfGameGUIMsg.h"

REGISTER_CREATABLE( pfGameGUIMsg );

#include "pfGUINotifyMsg.h"

REGISTER_CREATABLE( pfGUINotifyMsg );

#include "plClothingMsg.h"

REGISTER_CREATABLE( plClothingMsg );
REGISTER_CREATABLE( plElementRefMsg );
REGISTER_CREATABLE( plClothingUpdateBCMsg );

#include "plArmatureEffectMsg.h"

REGISTER_CREATABLE( plArmatureEffectMsg );
REGISTER_CREATABLE( plArmatureEffectStateMsg );

#include "pfKIMsg.h"

REGISTER_CREATABLE( pfKIMsg );

#include "pfMarkerMsg.h"

REGISTER_CREATABLE(pfMarkerMsg);

#include "pfBackdoorMsg.h"

REGISTER_CREATABLE(pfBackdoorMsg);

#include "pfMovieEventMsg.h"

REGISTER_CREATABLE(pfMovieEventMsg);

#endif pfMessageCreatable_inc
