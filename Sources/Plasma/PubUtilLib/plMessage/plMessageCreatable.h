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

#ifndef plMessageCreatable_inc
#define plMessageCreatable_inc

#include "pnFactory/plCreator.h"

#include "plAccountUpdateMsg.h"
REGISTER_CREATABLE(plAccountUpdateMsg);

#include "plActivatorMsg.h"
REGISTER_CREATABLE(plActivatorMsg);

#include "plAgeLoadedMsg.h"
REGISTER_CREATABLE(plAgeBeginLoadingMsg);
REGISTER_CREATABLE(plAgeLoadedMsg);
REGISTER_CREATABLE(plAgeLoaded2Msg);
REGISTER_CREATABLE(plInitialAgeStateLoadedMsg);

#include "plAngularVelocityMsg.h"
REGISTER_CREATABLE(plAngularVelocityMsg);

#include "plAnimCmdMsg.h"
REGISTER_CREATABLE(plAGCmdMsg);
REGISTER_CREATABLE(plAGDetachCallbackMsg);
REGISTER_CREATABLE(plAGInstanceCallbackMsg);
REGISTER_CREATABLE(plAnimCmdMsg);

#include "plAvatarFootMsg.h"
REGISTER_CREATABLE(plAvatarFootMsg);

#ifndef NO_AV_MSGS
    #include "plAvatarMsg.h"
    REGISTER_CREATABLE(plArmatureUpdateMsg);
    REGISTER_CREATABLE(plAvatarBehaviorNotifyMsg);
    REGISTER_CREATABLE(plAvatarOpacityCallbackMsg);
    REGISTER_CREATABLE(plAvatarMsg);
    REGISTER_CREATABLE(plAvatarPhysicsEnableCallbackMsg);
    REGISTER_CREATABLE(plAvatarSetTypeMsg);
    REGISTER_CREATABLE(plAvatarSpawnNotifyMsg);
    REGISTER_CREATABLE(plAvatarStealthModeMsg);
    REGISTER_CREATABLE(plAvBrainGenericMsg);
    REGISTER_CREATABLE(plAvOneShotMsg);
    REGISTER_CREATABLE(plAvSeekMsg);
    REGISTER_CREATABLE(plAvTaskMsg);
    REGISTER_CREATABLE(plAvTaskSeekDoneMsg);

    #ifndef SERVER
        REGISTER_CREATABLE(plAvPopBrainMsg);
        REGISTER_CREATABLE(plAvPushBrainMsg);
    #endif // ndef SERVER
#endif // ndef NO_AV_MSGS

#include "plBulletMsg.h"
REGISTER_CREATABLE(plBulletMsg);

#include "plCaptureRenderMsg.h"
REGISTER_CREATABLE(plCaptureRenderMsg);

#include "plCCRMessageCreatable.h"  // kept separately for selective server include 

#include "plClimbEventMsg.h"
REGISTER_CREATABLE(plClimbEventMsg);

#include "plClimbMsg.h"
REGISTER_CREATABLE(plClimbMsg);

#include "plCollideMsg.h"
REGISTER_CREATABLE(plCollideMsg);

#include "plCondRefMsg.h"
REGISTER_CREATABLE(plCondRefMsg);

#include "plConnectedToVaultMsg.h"
REGISTER_CREATABLE(plConnectedToVaultMsg);

#include "plConsoleMsg.h"
REGISTER_CREATABLE(plConsoleMsg);

#include "plDeviceRecreateMsg.h"
REGISTER_CREATABLE(plDeviceRecreateMsg);

#include "plDynaDecalEnableMsg.h"
REGISTER_CREATABLE(plDynaDecalEnableMsg);

#include "plDynamicEnvMapMsg.h"
REGISTER_CREATABLE(plDynamicEnvMapMsg);

#include "plDynamicTextMsg.h"
REGISTER_CREATABLE(plDynamicTextMsg);

#include "plExcludeRegionMsg.h"
REGISTER_CREATABLE(plExcludeRegionMsg);

#include "plInputEventMsg.h"
REGISTER_CREATABLE(plAvatarInputStateMsg);
REGISTER_CREATABLE(plControlEventMsg);
REGISTER_CREATABLE(plDebugKeyEventMsg);
REGISTER_CREATABLE(plIMouseBEventMsg);
REGISTER_CREATABLE(plIMouseXEventMsg);
REGISTER_CREATABLE(plIMouseYEventMsg);
REGISTER_CREATABLE(plInputEventMsg);
REGISTER_CREATABLE(plKeyEventMsg);
REGISTER_CREATABLE(plMouseEventMsg);

#include "plInputIfaceMgrMsg.h"
REGISTER_CREATABLE(plInputIfaceMgrMsg);

#include "plInterestingPing.h"
REGISTER_CREATABLE(plInterestingModMsg);
REGISTER_CREATABLE(plInterestingPing);

#include "plLayRefMsg.h"
REGISTER_CREATABLE(plLayRefMsg);

#include "plLightRefMsg.h"
REGISTER_CREATABLE(plLightRefMsg);

#include "plLinearVelocityMsg.h"
REGISTER_CREATABLE(plLinearVelocityMsg);

#include "plLinkToAgeMsg.h"
REGISTER_CREATABLE(plLinkCallbackMsg);
REGISTER_CREATABLE(plLinkEffectBCMsg);
REGISTER_CREATABLE(plLinkEffectPrepBCMsg);
REGISTER_CREATABLE(plLinkEffectsTriggerMsg);
REGISTER_CREATABLE(plLinkEffectsTriggerPrepMsg);
REGISTER_CREATABLE(plLinkingMgrMsg);
REGISTER_CREATABLE(plLinkToAgeMsg);
REGISTER_CREATABLE(plPseudoLinkAnimCallbackMsg);
REGISTER_CREATABLE(plPseudoLinkAnimTriggerMsg);
REGISTER_CREATABLE(plPseudoLinkEffectMsg);

#include "plListenerMsg.h"
REGISTER_CREATABLE(plListenerMsg);
REGISTER_CREATABLE(plSetListenerMsg);

#include "plLoadAgeMsg.h"
REGISTER_CREATABLE(plLoadAgeMsg);
REGISTER_CREATABLE(plLinkInDoneMsg)
REGISTER_CREATABLE(plLinkOutUnloadMsg);

#include "plLOSHitMsg.h"
REGISTER_CREATABLE(plLOSHitMsg);

#include "plLOSRequestMsg.h"
REGISTER_CREATABLE(plLOSRequestMsg);

#include "plMatRefMsg.h"
REGISTER_CREATABLE(plMatRefMsg);

#include "plMatrixUpdateMsg.h"
REGISTER_CREATABLE(plMatrixUpdateMsg);

#include "plMemberUpdateMsg.h"
REGISTER_CREATABLE(plMemberUpdateMsg);

#include "plMeshRefMsg.h"
REGISTER_CREATABLE(plMeshRefMsg);

#include "plMovieMsg.h"
REGISTER_CREATABLE(plMovieMsg);

#include "plMultistageMsg.h"
REGISTER_CREATABLE(plMultistageModMsg);

#include "plNetClientMgrMsg.h"
REGISTER_CREATABLE(plNetClientMgrMsg);

#include "plNetCommMsgs.h"
REGISTER_CREATABLE(plNetCommActivePlayerMsg);
REGISTER_CREATABLE(plNetCommAuthConnectedMsg);
REGISTER_CREATABLE(plNetCommAuthMsg);
REGISTER_CREATABLE(plNetCommCreatePlayerMsg);
REGISTER_CREATABLE(plNetCommDeletePlayerMsg);
REGISTER_CREATABLE(plNetCommFileDownloadMsg);
REGISTER_CREATABLE(plNetCommFileListMsg);
REGISTER_CREATABLE(plNetCommLinkToAgeMsg);
REGISTER_CREATABLE(plNetCommPlayerListMsg);
REGISTER_CREATABLE(plNetCommPublicAgeListMsg);
REGISTER_CREATABLE(plNetCommPublicAgeMsg);
REGISTER_CREATABLE(plNetCommRegisterAgeMsg);

#include "plNetOwnershipMsg.h"
REGISTER_CREATABLE(plNetOwnershipMsg);

#include "plNetVoiceListMsg.h"
REGISTER_CREATABLE(plNetVoiceListMsg);

#include "plNodeCleanupMsg.h"
REGISTER_CREATABLE(plNodeCleanupMsg);

#include "plOneShotMsg.h"
REGISTER_CREATABLE(plOneShotMsg);

#include "plParticleUpdateMsg.h"
REGISTER_CREATABLE(plParticleFlockMsg);
REGISTER_CREATABLE(plParticleKillMsg);
REGISTER_CREATABLE(plParticleTransferMsg);
REGISTER_CREATABLE(plParticleUpdateMsg);

#include "plPickedMsg.h"
REGISTER_CREATABLE(plPickedMsg);

#include "plRenderMsg.h"
REGISTER_CREATABLE(plPreResourceMsg);
REGISTER_CREATABLE(plRenderMsg);

#include "plRenderRequestMsg.h"
REGISTER_CREATABLE(plRenderRequestAck);
REGISTER_CREATABLE(plRenderRequestMsg);

#include "plReplaceGeometryMsg.h"
REGISTER_CREATABLE(plReplaceGeometryMsg);
REGISTER_CREATABLE(plSwapSpansRefMsg);

#include "plResMgrHelperMsg.h"
REGISTER_CREATABLE(plResMgrHelperMsg);

#include "plResPatcherMsg.h"
REGISTER_CREATABLE(plResPatcherMsg);

#include "plResponderMsg.h"
REGISTER_CREATABLE(plResponderMsg);

#include "plRideAnimatedPhysMsg.h"
REGISTER_CREATABLE(plRideAnimatedPhysMsg);

#include "plRippleShapeMsg.h"
REGISTER_CREATABLE(plRippleShapeMsg);

#include "plRoomLoadNotifyMsg.h"
REGISTER_CREATABLE(plRoomLoadNotifyMsg);

#include "plShadowCastMsg.h"
REGISTER_CREATABLE(plShadowCastMsg);

#include "plSimStateMsg.h"
// REGISTER_CREATABLE(plSimStateMsg);
// REGISTER_CREATABLE(plFreezeMsg);
// REGISTER_CREATABLE(plEventGroupMsg);
// REGISTER_CREATABLE(plEventGroupEnableMsg);
// REGISTER_CREATABLE(plSuspendEventMsg);
REGISTER_CREATABLE(plSubWorldMsg);

#include "plSpawnModMsg.h"
REGISTER_CREATABLE(plSpawnModMsg);

#include "plSpawnRequestMsg.h"
REGISTER_CREATABLE(plSpawnRequestMsg);

#include "plSwimMsg.h"
REGISTER_CREATABLE(plSwimMsg);

#include "plSynchEnableMsg.h"
REGISTER_CREATABLE(plSynchEnableMsg);

#include "plTimerCallbackMsg.h"
REGISTER_CREATABLE(plTimerCallbackMsg);

#include "plTransitionMsg.h"
REGISTER_CREATABLE(plTransitionMsg);

#include "plTriggerMsg.h"
REGISTER_CREATABLE(plTriggerMsg);

#include "plVaultNotifyMsg.h"
REGISTER_CREATABLE(plVaultNotifyMsg);

#ifndef SERVER
    #ifndef NO_AV_MSGS
        #include "plAIMsg.h"
        REGISTER_CREATABLE(plAIArrivedAtGoalMsg);
        REGISTER_CREATABLE(plAIBrainCreatedMsg);
        REGISTER_CREATABLE(plAIMsg);
    #endif // NO_AV_MSGS
#endif // SERVER

/*****************************************************************************
*
*   Messages excluded from SERVER build, and the NoAvMsgs build configurations
*
***/

#ifndef NO_AV_MSGS
    #ifndef SERVER
        #include "plAvCoopMsg.h"
        REGISTER_CREATABLE(plAvCoopMsg);

        #include "plLoadAvatarMsg.h"
        REGISTER_CREATABLE(plLoadAvatarMsg);

        #include "plLoadCloneMsg.h"
        REGISTER_CREATABLE(plLoadCloneMsg);

        #include "plLoadClothingMsg.h"
        REGISTER_CREATABLE(plLoadClothingMsg);
    #endif // ndef SERVER
#endif // ndef NO_AV_MSGS

#endif // plMessageCreatable_inc
