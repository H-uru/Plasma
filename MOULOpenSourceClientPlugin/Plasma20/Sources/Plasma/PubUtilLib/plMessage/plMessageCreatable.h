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

#ifndef plMessageCreatable_inc
#define plMessageCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plInterestingPing.h"

REGISTER_CREATABLE( plInterestingModMsg );
REGISTER_CREATABLE( plInterestingPing );

#include "plLayRefMsg.h"

REGISTER_CREATABLE( plLayRefMsg );

#include "plMatRefMsg.h"

REGISTER_CREATABLE( plMatRefMsg );

#include "plMeshRefMsg.h"

REGISTER_CREATABLE( plMeshRefMsg );

#include "plLOSRequestMsg.h"

REGISTER_CREATABLE( plLOSRequestMsg );

#include "plLOSHitMsg.h"

REGISTER_CREATABLE( plLOSHitMsg );

#include "plActivatorMsg.h"

REGISTER_CREATABLE( plActivatorMsg );

#include "plCondRefMsg.h"

REGISTER_CREATABLE( plCondRefMsg );

#include "plAnimCmdMsg.h"

REGISTER_CREATABLE( plAnimCmdMsg );
REGISTER_CREATABLE( plAGCmdMsg );
REGISTER_CREATABLE( plAGInstanceCallbackMsg );
REGISTER_CREATABLE( plAGDetachCallbackMsg );

#include "plParticleUpdateMsg.h"

REGISTER_CREATABLE( plParticleUpdateMsg );
REGISTER_CREATABLE( plParticleTransferMsg );
REGISTER_CREATABLE( plParticleKillMsg );
REGISTER_CREATABLE( plParticleFlockMsg );

#include "plInputEventMsg.h"

REGISTER_CREATABLE( plInputEventMsg );
REGISTER_CREATABLE( plControlEventMsg );
REGISTER_CREATABLE( plKeyEventMsg );
REGISTER_CREATABLE( plDebugKeyEventMsg );
REGISTER_CREATABLE( plMouseEventMsg );
REGISTER_CREATABLE( plIMouseXEventMsg );
REGISTER_CREATABLE( plIMouseYEventMsg );
REGISTER_CREATABLE( plIMouseBEventMsg );
REGISTER_CREATABLE( plAvatarInputStateMsg );

#include "plPickedMsg.h"

REGISTER_CREATABLE( plPickedMsg );

#include "plCollideMsg.h"

REGISTER_CREATABLE( plCollideMsg );

#include "plMatrixUpdateMsg.h"

REGISTER_CREATABLE( plMatrixUpdateMsg );

#include "plRenderMsg.h"

REGISTER_CREATABLE( plRenderMsg );
REGISTER_CREATABLE( plPreResourceMsg );

#include "plTimerCallbackMsg.h"

REGISTER_CREATABLE( plTimerCallbackMsg );

#include "plSpawnModMsg.h"

REGISTER_CREATABLE( plSpawnModMsg );

#include "plSpawnRequestMsg.h"

REGISTER_CREATABLE( plSpawnRequestMsg );

#include "plNodeCleanupMsg.h"

REGISTER_CREATABLE( plNodeCleanupMsg );

#include "plDeviceRecreateMsg.h"

REGISTER_CREATABLE( plDeviceRecreateMsg );

#include "plLightRefMsg.h"

REGISTER_CREATABLE( plLightRefMsg );

#include "plSimInfluenceMsg.h"

// REGISTER_CREATABLE( plSimInfluenceMsg );
// REGISTER_CREATABLE( plForceMsg );
// REGISTER_CREATABLE( plOffsetForceMsg );
// REGISTER_CREATABLE( plTorqueMsg );
// REGISTER_CREATABLE( plImpulseMsg );
// REGISTER_CREATABLE( plOffsetImpulseMsg );
// REGISTER_CREATABLE( plAngularImpulseMsg );
// REGISTER_CREATABLE( plDampMsg );
// REGISTER_CREATABLE( plShiftMassMsg );

#include "plSimStateMsg.h"

// REGISTER_CREATABLE( plSimStateMsg );
// REGISTER_CREATABLE( plFreezeMsg );
// REGISTER_CREATABLE( plEventGroupMsg );
// REGISTER_CREATABLE( plEventGroupEnableMsg );
// REGISTER_CREATABLE( plSuspendEventMsg );
REGISTER_CREATABLE( plSubWorldMsg );

#include "plLinearVelocityMsg.h"
REGISTER_CREATABLE( plLinearVelocityMsg );

#include "plAngularVelocityMsg.h"
REGISTER_CREATABLE( plAngularVelocityMsg );

#include "plRenderRequestMsg.h"

REGISTER_CREATABLE( plRenderRequestMsg );
REGISTER_CREATABLE( plRenderRequestAck );

#include "plLinkToAgeMsg.h"
REGISTER_CREATABLE(plLinkToAgeMsg);
REGISTER_CREATABLE(plLinkingMgrMsg);
REGISTER_CREATABLE(plLinkCallbackMsg);
REGISTER_CREATABLE(plLinkEffectsTriggerMsg);
REGISTER_CREATABLE(plLinkEffectBCMsg);
REGISTER_CREATABLE(plLinkEffectsTriggerPrepMsg);
REGISTER_CREATABLE(plLinkEffectPrepBCMsg);
REGISTER_CREATABLE(plPseudoLinkEffectMsg);
REGISTER_CREATABLE(plPseudoLinkAnimTriggerMsg);
REGISTER_CREATABLE(plPseudoLinkAnimCallbackMsg);

#include "plListenerMsg.h"
REGISTER_CREATABLE(plListenerMsg);
REGISTER_CREATABLE(plSetListenerMsg);

#include "plTransitionMsg.h"
REGISTER_CREATABLE(plTransitionMsg);

#include "plConsoleMsg.h"
REGISTER_CREATABLE(plConsoleMsg);

#include "plLoadAgeMsg.h"
REGISTER_CREATABLE(plLoadAgeMsg);
REGISTER_CREATABLE(plLinkOutUnloadMsg);

#include "plResponderMsg.h"
REGISTER_CREATABLE(plResponderMsg);

#include "plOneShotMsg.h"
REGISTER_CREATABLE(plOneShotMsg);

#include "plTriggerMsg.h"
REGISTER_CREATABLE( plTriggerMsg );

#ifndef NO_AV_MSGS
#include "plAvatarMsg.h"
REGISTER_CREATABLE( plAvatarMsg );
REGISTER_CREATABLE( plArmatureUpdateMsg );
REGISTER_CREATABLE( plAvatarSetTypeMsg );
REGISTER_CREATABLE( plAvTaskMsg );
REGISTER_CREATABLE( plAvSeekMsg );
REGISTER_CREATABLE( plAvOneShotMsg );
REGISTER_CREATABLE( plAvBrainGenericMsg );

#ifndef SERVER
REGISTER_CREATABLE( plAvPushBrainMsg );
REGISTER_CREATABLE( plAvPopBrainMsg );
#endif // ndef SERVER

REGISTER_CREATABLE( plAvatarStealthModeMsg );
REGISTER_CREATABLE( plAvatarBehaviorNotifyMsg );
REGISTER_CREATABLE( plAvatarOpacityCallbackMsg );
REGISTER_CREATABLE( plAvTaskSeekDoneMsg );
REGISTER_CREATABLE( plAvatarSpawnNotifyMsg );
REGISTER_CREATABLE( plAvatarPhysicsEnableCallbackMsg );
#endif // ndef NO_AV_MSGS

#include "plMultistageMsg.h"
REGISTER_CREATABLE( plMultistageModMsg );

#include "plExcludeRegionMsg.h"
REGISTER_CREATABLE(plExcludeRegionMsg);

#include "plDynamicTextMsg.h"
REGISTER_CREATABLE(plDynamicTextMsg);

#include "plInputIfaceMgrMsg.h"
REGISTER_CREATABLE(plInputIfaceMgrMsg);

#include "plRoomLoadNotifyMsg.h"
REGISTER_CREATABLE(plRoomLoadNotifyMsg);

#include "plMemberUpdateMsg.h"
REGISTER_CREATABLE(plMemberUpdateMsg);

#include "plAgeLoadedMsg.h"
REGISTER_CREATABLE(plAgeLoadedMsg);
REGISTER_CREATABLE(plAgeLoaded2Msg);
REGISTER_CREATABLE(plAgeBeginLoadingMsg);
REGISTER_CREATABLE(plInitialAgeStateLoadedMsg);
REGISTER_CREATABLE(plLinkInDoneMsg)

#include "plReplaceGeometryMsg.h"
REGISTER_CREATABLE(plReplaceGeometryMsg);
REGISTER_CREATABLE(plSwapSpansRefMsg);

#include "plShadowCastMsg.h"
REGISTER_CREATABLE(plShadowCastMsg);

#include "plResMgrHelperMsg.h"
REGISTER_CREATABLE(plResMgrHelperMsg);

#include "plBulletMsg.h"
REGISTER_CREATABLE(plBulletMsg);

#include "plDynaDecalEnableMsg.h"
REGISTER_CREATABLE(plDynaDecalEnableMsg);

#include "plDynamicEnvMapMsg.h"
REGISTER_CREATABLE(plDynamicEnvMapMsg);

#include "plAvatarFootMsg.h"
REGISTER_CREATABLE(plAvatarFootMsg);

#include "plRippleShapeMsg.h"
REGISTER_CREATABLE(plRippleShapeMsg);

#include "plNetOwnershipMsg.h"
REGISTER_CREATABLE(plNetOwnershipMsg);

#include "plCCRMessageCreatable.h"	// kept separately for selective server include 

#include "plConnectedToVaultMsg.h"
REGISTER_CREATABLE(plConnectedToVaultMsg);

#include "plClimbMsg.h"
REGISTER_CREATABLE(plClimbMsg);

#include "plNetVoiceListMsg.h"
REGISTER_CREATABLE(plNetVoiceListMsg);

#include "plSwimMsg.h"
REGISTER_CREATABLE(plSwimMsg);

#include "plVaultNotifyMsg.h"
REGISTER_CREATABLE(plVaultNotifyMsg);

#include "plSynchEnableMsg.h"
REGISTER_CREATABLE(plSynchEnableMsg);

#include "plMovieMsg.h"
REGISTER_CREATABLE(plMovieMsg);

#include "plCaptureRenderMsg.h"
REGISTER_CREATABLE(plCaptureRenderMsg);

#include "plClimbEventMsg.h"
REGISTER_CREATABLE(plClimbEventMsg);

#include "plNetCommMsgs.h"
REGISTER_CREATABLE(plNetCommAuthConnectedMsg);
REGISTER_CREATABLE(plNetCommAuthMsg);
REGISTER_CREATABLE(plNetCommFileListMsg);
REGISTER_CREATABLE(plNetCommFileDownloadMsg);
REGISTER_CREATABLE(plNetCommLinkToAgeMsg);
REGISTER_CREATABLE(plNetCommPlayerListMsg);
REGISTER_CREATABLE(plNetCommActivePlayerMsg);
REGISTER_CREATABLE(plNetCommCreatePlayerMsg);
REGISTER_CREATABLE(plNetCommDeletePlayerMsg);
REGISTER_CREATABLE(plNetCommPublicAgeListMsg);
REGISTER_CREATABLE(plNetCommPublicAgeMsg);
REGISTER_CREATABLE(plNetCommRegisterAgeMsg);

#include "plPreloaderMsg.h"
REGISTER_CREATABLE(plPreloaderMsg);

#include "plNetClientMgrMsg.h"
REGISTER_CREATABLE(plNetClientMgrMsg);

#include "plNCAgeJoinerMsg.h"
REGISTER_CREATABLE(plNCAgeJoinerMsg);

#include "plAccountUpdateMsg.h"
REGISTER_CREATABLE(plAccountUpdateMsg);

#include "plRideAnimatedPhysMsg.h"
REGISTER_CREATABLE(plRideAnimatedPhysMsg);

#ifndef SERVER
#ifndef NO_AV_MSGS
#include "plAIMsg.h"
REGISTER_CREATABLE(plAIMsg);
REGISTER_CREATABLE(plAIBrainCreatedMsg);
REGISTER_CREATABLE(plAIArrivedAtGoalMsg);
#endif // NO_AV_MSGS
#endif // SERVER

/*****************************************************************************
*
*   Messages excluded from SERVER build, and the NoAvMsgs build configurations
*
***/

#ifndef NO_AV_MSGS
#ifndef SERVER

# include "plLoadCloneMsg.h"
REGISTER_CREATABLE(plLoadCloneMsg);

# include "plLoadAvatarMsg.h"
REGISTER_CREATABLE(plLoadAvatarMsg);

# include "plAvCoopMsg.h"
REGISTER_CREATABLE(plAvCoopMsg);

#endif // ndef SERVER
#endif // ndef NO_AV_MSGS


#endif // plMessageCreatable_inc

