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

#ifndef plAvatarCreatable_inc
#define plAvatarCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plAGAnim.h"

REGISTER_CREATABLE( plAGAnim );
REGISTER_CREATABLE( plATCAnim );
REGISTER_CREATABLE( plEmoteAnim );
REGISTER_CREATABLE( plAgeGlobalAnim );

#include "plAGChannel.h"

REGISTER_NONCREATABLE( plAGChannel );
REGISTER_NONCREATABLE( plAGApplicator );

#include "plMatrixChannel.h"

REGISTER_CREATABLE( plMatrixChannel );
REGISTER_CREATABLE( plMatrixConstant );
REGISTER_CREATABLE( plMatrixTimeScale );
REGISTER_CREATABLE( plMatrixBlend );
REGISTER_CREATABLE( plMatrixControllerChannel );
REGISTER_CREATABLE( plMatrixControllerCacheChannel );
REGISTER_CREATABLE( plQuatPointCombine );
REGISTER_CREATABLE( plMatrixChannelApplicator );
REGISTER_CREATABLE( plMatrixDelayedCorrectionApplicator );
REGISTER_CREATABLE( plMatrixDifferenceApp );

#include "plPointChannel.h"

REGISTER_CREATABLE( plPointChannel );
REGISTER_CREATABLE( plPointConstant );
REGISTER_CREATABLE( plPointBlend );
REGISTER_CREATABLE( plPointTimeScale );
REGISTER_CREATABLE( plPointControllerChannel );
REGISTER_CREATABLE( plPointControllerCacheChannel );
REGISTER_CREATABLE( plPointChannelApplicator );
REGISTER_CREATABLE( plLightDiffuseApplicator );
REGISTER_CREATABLE( plLightAmbientApplicator );
REGISTER_CREATABLE( plLightSpecularApplicator );

#include "plQuatChannel.h"

REGISTER_CREATABLE( plQuatChannel );
REGISTER_CREATABLE( plQuatConstant );
REGISTER_CREATABLE( plQuatBlend );
REGISTER_CREATABLE( plQuatTimeScale );
REGISTER_CREATABLE( plQuatChannelApplicator );

#include "plScalarChannel.h"
REGISTER_CREATABLE( plScalarChannel );
REGISTER_CREATABLE( plScalarConstant );
REGISTER_CREATABLE( plScalarTimeScale );
REGISTER_CREATABLE( plScalarBlend );
REGISTER_CREATABLE( plScalarControllerChannel );
REGISTER_CREATABLE( plScalarControllerCacheChannel );
REGISTER_CREATABLE( plScalarChannelApplicator );
REGISTER_CREATABLE( plSpotInnerApplicator );
REGISTER_CREATABLE( plSpotOuterApplicator );
REGISTER_CREATABLE( plATCChannel );
REGISTER_CREATABLE( plScalarSDLChannel );
REGISTER_CREATABLE( plOmniApplicator );
REGISTER_CREATABLE( plOmniSqApplicator );
REGISTER_CREATABLE( plOmniCutoffApplicator );

#include "plAGModifier.h"
REGISTER_CREATABLE( plAGModifier );

#include "plAGMasterMod.h"
REGISTER_CREATABLE( plAGMasterMod );

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

#include "plAGMasterSDLModifier.h"
REGISTER_CREATABLE( plAGMasterSDLModifier );

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

