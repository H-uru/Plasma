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

#ifndef plAvatarCreatable_inc
#define plAvatarCreatable_inc

#include "pnFactory/plCreator.h"

#include "plSeekPointMod.h"
REGISTER_CREATABLE( plSeekPointMod );

#include "plOneShotMod.h"
REGISTER_CREATABLE( plOneShotMod );

#include "plMultistageBehMod.h"
REGISTER_CREATABLE( plMultistageBehMod );

#include "plArmatureMod.h"
REGISTER_CREATABLE( plArmatureModBase );
REGISTER_CREATABLE( plArmatureMod );
REGISTER_CREATABLE( plArmatureLODMod );

#include "plArmatureEffects.h"
REGISTER_CREATABLE( plArmatureEffectsMgr );
REGISTER_NONCREATABLE( plArmatureEffect );
REGISTER_CREATABLE( plArmatureEffectFootSound );

#include "plAvBrain.h"
REGISTER_NONCREATABLE(plArmatureBrain);

#include "plAvBrainHuman.h"
REGISTER_CREATABLE(plAvBrainHuman);

#include "plAvBrainDrive.h"
REGISTER_CREATABLE(plAvBrainDrive);

#include "plAvLadderModifier.h"
REGISTER_CREATABLE( plAvLadderMod );

#include "plAvatarClothing.h"
REGISTER_CREATABLE(plClothingItem);
REGISTER_CREATABLE(plClothingOutfit);
REGISTER_CREATABLE(plClothingBase);
REGISTER_CREATABLE(plClothingMgr);

#include "plAvBrainGeneric.h"
REGISTER_CREATABLE(plAvBrainGeneric);

#include "plAvatarTasks.h"
REGISTER_NONCREATABLE( plAvTask );
REGISTER_CREATABLE( plAvAnimTask );
REGISTER_CREATABLE( plAvSeekTask )
REGISTER_CREATABLE( plAvOneShotTask );
REGISTER_CREATABLE( plAvOneShotLinkTask );

#include "plAnimStage.h"
REGISTER_CREATABLE( plAnimStage );

#include "plAvTaskSeek.h"
REGISTER_CREATABLE( plAvTaskSeek );

#include "plAvatarSDLModifier.h"
REGISTER_CREATABLE( plAvatarSDLModifier );
REGISTER_CREATABLE( plAvatarPhysicalSDLModifier );

#include "plClothingSDLModifier.h"
REGISTER_CREATABLE( plClothingSDLModifier );

#include "plAvatarMgr.h"
REGISTER_NONCREATABLE( plAvatarMgr );

#include "plNPCSpawnMod.h"
REGISTER_CREATABLE( plNPCSpawnMod );

#include "plAvBrainSwim.h"
REGISTER_CREATABLE( plAvBrainSwim );

#include "plAvBrainClimb.h"
REGISTER_CREATABLE( plAvBrainClimb );

#include "plAvBrainCoop.h"
REGISTER_CREATABLE( plAvBrainCoop );

#include "plCoopCoordinator.h"
REGISTER_CREATABLE( plCoopCoordinator );

#include "plAvTaskBrain.h"
REGISTER_CREATABLE( plAvTaskBrain );

#include "plSittingModifier.h"
REGISTER_CREATABLE( plSittingModifier );

#include "plSwimRegion.h"
REGISTER_CREATABLE( plSwimRegionInterface );
REGISTER_CREATABLE( plSwimCircularCurrentRegion );
REGISTER_CREATABLE( plSwimStraightCurrentRegion );

#include "plAvBrainCritter.h"
REGISTER_CREATABLE( plAvBrainCritter );

#include "plAvBrainRideAnimatedPhysical.h"
REGISTER_CREATABLE(plAvBrainRideAnimatedPhysical)
#endif // plAvatarCreatable_inc

