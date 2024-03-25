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

#include "plAnimStage.h"
REGISTER_CREATABLE(plAnimStage);

#include "plArmatureEffects.h"
REGISTER_NONCREATABLE(plArmatureEffect);
REGISTER_CREATABLE(plArmatureEffectFootSound);
REGISTER_CREATABLE(plArmatureEffectsMgr);

#include "plArmatureMod.h"
REGISTER_CREATABLE(plArmatureLODMod);
REGISTER_CREATABLE(plArmatureModBase);
REGISTER_CREATABLE(plArmatureMod);

#include "plAvatarClothing.h"
REGISTER_CREATABLE(plClothingBase);
REGISTER_CREATABLE(plClothingItem);
REGISTER_CREATABLE(plClothingMgr);
REGISTER_CREATABLE(plClothingOutfit);

#include "plAvatarMgr.h"
REGISTER_NONCREATABLE(plAvatarMgr);

#include "plAvatarSDLModifier.h"
REGISTER_CREATABLE(plAvatarSDLModifier);
REGISTER_CREATABLE(plAvatarPhysicalSDLModifier);

#include "plAvatarTasks.h"
REGISTER_NONCREATABLE(plAvTask);
REGISTER_CREATABLE(plAvAnimTask);
REGISTER_CREATABLE(plAvOneShotTask);
REGISTER_CREATABLE(plAvOneShotLinkTask);
REGISTER_CREATABLE(plAvSeekTask);

#include "plAvBrain.h"
REGISTER_NONCREATABLE(plArmatureBrain);

#include "plAvBrainClimb.h"
REGISTER_CREATABLE(plAvBrainClimb);

#include "plAvBrainCoop.h"
REGISTER_CREATABLE(plAvBrainCoop);

#include "plAvBrainCritter.h"
REGISTER_CREATABLE(plAvBrainCritter);

#include "plAvBrainDrive.h"
REGISTER_CREATABLE(plAvBrainDrive);

#include "plAvBrainGeneric.h"
REGISTER_CREATABLE(plAvBrainGeneric);

#include "plAvBrainHuman.h"
REGISTER_CREATABLE(plAvBrainHuman);

#include "plAvBrainRideAnimatedPhysical.h"
REGISTER_CREATABLE(plAvBrainRideAnimatedPhysical);

#include "plAvBrainSwim.h"
REGISTER_CREATABLE(plAvBrainSwim);

#include "plAvLadderModifier.h"
REGISTER_CREATABLE(plAvLadderMod);

#include "plAvTaskBrain.h"
REGISTER_CREATABLE(plAvTaskBrain);

#include "plAvTaskSeek.h"
REGISTER_CREATABLE(plAvTaskSeek);

#include "plClothingSDLModifier.h"
REGISTER_CREATABLE(plClothingSDLModifier);

#include "plCoopCoordinator.h"
REGISTER_CREATABLE(plCoopCoordinator);

#include "plMultistageBehMod.h"
REGISTER_CREATABLE(plMultistageBehMod);

#include "plNPCSpawnMod.h"
REGISTER_CREATABLE(plNPCSpawnMod);

#include "plOneShotMod.h"
REGISTER_CREATABLE(plOneShotMod);

#include "plSeekPointMod.h"
REGISTER_CREATABLE(plSeekPointMod);

#include "plSittingModifier.h"
REGISTER_CREATABLE(plSittingModifier);

#include "plSwimRegion.h"
REGISTER_CREATABLE(plSwimRegionInterface);
REGISTER_CREATABLE(plSwimCircularCurrentRegion);
REGISTER_CREATABLE(plSwimStraightCurrentRegion);

#endif // plAvatarCreatable_inc
