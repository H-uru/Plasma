/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include "Factory.h"
#include "Avatar.h"
#include "plMessage.h"
#include "plNetMessage.h"

QString Factory_Name(unsigned type)
{
    static const char* s_keyedNames[] = {
        "plSceneNode", "plSceneObject", "hsKeyedObject", "plBitmap", "plMipmap",
        "plCubicEnvironmap", "plLayer", "hsGMaterial", "plParticleSystem",
        "plParticleEffect", "plParticleCollisionEffectBeat", "plParticleFadeVolumeEffect",
        "plBoundInterface", "plRenderTarget", "plCubicRenderTarget",
        "plCubicRenderTargetModifier", "plObjInterface", "plAudioInterface",
        "plAudible", "plAudibleNull", "plWinAudible", "plCoordinateInterface",
        "plDrawInterface", "plDrawable", "plDrawableMesh", "plDrawableIce",
        "plPhysical", "plPhysicalMesh", "plSimulationInterface", "plCameraModifier",
        "plModifier", "plSingleModifier", "plSimpleModifier", "pfSecurePreloader",
        "UNUSED_plRandomTMModifier", "plInterestingModifier", "plDetectorModifier",
        "plSimplePhysicalMesh", "plCompoundPhysicalMesh", "plMultiModifier",
        "plSynchedObject", "plSoundBuffer", "UNUSED_plAliasModifier",
        "plPickingDetector", "plCollisionDetector", "plLogicModifier",
        "plConditionalObject", "plANDConditionalObject", "plORConditionalObject",
        "plPickedConditionalObject", "plActivatorConditionalObject",
        "plTimerCallbackManager", "plKeyPressConditionalObject",
        "plAnimationEventConditionalObject", "plControlEventConditionalObject",
        "plObjectInBoxConditionalObject", "plLocalPlayerInBoxConditionalObject",
        "plObjectIntersectPlaneConditionalObject", "plLocalPlayerIntersectPlaneConditionalObject",
        "plPortalDrawable", "plPortalPhysical", "plSpawnModifier",
        "plFacingConditionalObject", "plPXPhysical", "plViewFaceModifier",
        "plLayerInterface", "plLayerWrapper", "plLayerAnimation", "plLayerDepth",
        "plLayerMovie", "plLayerBink", "plLayerAVI", "plSound", "plWin32Sound",
        "plLayerOr", "plAudioSystem", "plDrawableSpans", "UNUSED_plDrawablePatchSet",
        "plInputManager", "plLogicModBase", "plFogEnvironment", "plNetApp",
        "plNetClientMgr", "pl2WayWinAudible", "plLightInfo", "plDirectionalLightInfo",
        "plOmniLightInfo", "plSpotLightInfo", "plLightSpace", "plNetClientApp",
        "plNetServerApp", "plClient", "UNUSED_plCompoundTMModifier", "plCameraBrain",
        "plCameraBrain_Default", "plCameraBrain_Drive", "plCameraBrain_Fixed",
        "plCameraBrain_FixedPan", "pfGUIClickMapCtrl", "plListener", "plAvatarMod",
        "plAvatarAnim", "plAvatarAnimMgr", "plOccluder", "plMobileOccluder",
        "plLayerShadowBase", "plLimitedDirLightInfo", "plAGAnim", "plAGModifier",
        "plAGMasterMod", "plCameraBrain_Avatar", "plCameraRegionDetector",
        "plCameraBrain_FP", "plLineFollowMod", "plLightModifier", "plOmniModifier",
        "plSpotModifier", "plLtdDirModifier", "plSeekPointMod", "plOneShotMod",
        "plRandomCommandMod", "plRandomSoundMod", "plPostEffectMod",
        "plObjectInVolumeDetector", "plResponderModifier", "plAxisAnimModifier",
        "plLayerLightBase", "plFollowMod", "plTransitionMgr", "UNUSED___plInventoryMod",
        "UNUSED___plInventoryObjMod", "plLinkEffectsMgr", "plWin32StreamingSound",
        "UNUSED___plPythonMod", "plActivatorActivatorConditionalObject",
        "plSoftVolume", "plSoftVolumeSimple", "plSoftVolumeComplex", "plSoftVolumeUnion",
        "plSoftVolumeIntersect", "plSoftVolumeInvert", "plWin32LinkSound",
        "plLayerLinkAnimation", "plArmatureMod", "plCameraBrain_Freelook",
        "plHavokConstraintsMod", "plHingeConstraintMod", "plWheelConstraintMod",
        "plStrongSpringConstraintMod", "plArmatureLODMod", "plWin32StaticSound",
        "pfGameGUIMgr", "pfGUIDialogMod", "plCameraBrain1", "plVirtualCam1",
        "plCameraModifier1", "plCameraBrain1_Drive", "plCameraBrain1_POA",
        "plCameraBrain1_Avatar", "plCameraBrain1_Fixed", "plCameraBrain1_POAFixed",
        "pfGUIButtonMod", "plPythonFileMod", "pfGUIControlMod", "plExcludeRegionModifier",
        "pfGUIDraggableMod", "plVolumeSensorConditionalObject",
        "plVolActivatorConditionalObject", "plMsgForwarder", "plBlower",
        "pfGUIListBoxMod", "pfGUITextBoxMod", "pfGUIEditBoxMod", "plDynamicTextMap",
        "plSittingModifier", "pfGUIUpDownPairMod", "pfGUIValueCtrl", "pfGUIKnobCtrl",
        "plAvLadderMod", "plCameraBrain1_FirstPerson", "plCloneSpawnModifier",
        "plClothingItem", "plClothingOutfit", "plClothingBase", "plClothingMgr",
        "pfGUIDragBarCtrl", "pfGUICheckBoxCtrl", "pfGUIRadioGroupCtrl",
        "pfPlayerBookMod", "pfGUIDynDisplayCtrl", "UNUSED_plLayerProject",
        "plInputInterfaceMgr", "plRailCameraMod", "plMultistageBehMod",
        "plCameraBrain1_Circle", "plParticleWindEffect", "plAnimEventModifier",
        "plAutoProfile", "pfGUISkin", "plAVIWriter", "plParticleCollisionEffect",
        "plParticleCollisionEffectDie", "plParticleCollisionEffectBounce",
        "plInterfaceInfoModifier", "plSharedMesh", "plArmatureEffectsMgr",
        "pfMarkerMgr", "plVehicleModifier", "plParticleLocalWind",
        "plParticleUniformWind", "plInstanceDrawInterface", "plShadowMaster",
        "plShadowCaster", "plPointShadowMaster", "plDirectShadowMaster",
        "plSDLModifier", "plPhysicalSDLModifier", "plClothingSDLModifier",
        "plAvatarSDLModifier", "plAGMasterSDLModifier", "plPythonSDLModifier",
        "plLayerSDLModifier", "plAnimTimeConvertSDLModifier", "plResponderSDLModifier",
        "plSoundSDLModifier", "plResManagerHelper", "plAvatarPhysicalSDLModifier",
        "plArmatureEffect", "plArmatureEffectFootSound", "plEAXListenerMod",
        "plDynaDecalMgr", "plObjectInVolumeAndFacingDetector", "plDynaFootMgr",
        "plDynaRippleMgr", "plDynaBulletMgr", "plDecalEnableMod", "plPrintShape",
        "plDynaPuddleMgr", "pfGUIMultiLineEditCtrl", "plLayerAnimationBase",
        "plLayerSDLAnimation", "plATCAnim", "plAgeGlobalAnim",
        "plSubworldRegionDetector", "plAvatarMgr", "plNPCSpawnMod", "plActivePrintShape",
        "plExcludeRegionSDLModifier", "plLOSDispatch", "plDynaWakeMgr",
        "plSimulationMgr", "plWaveSet7", "plPanicLinkRegion", "plWin32GroupedSound",
        "plFilterCoordInterface", "plStereizer", "plCCRMgr", "plCCRSpecialist",
        "plCCRSeniorSpecialist", "plCCRShiftSupervisor", "plCCRGameOperator",
        "plShader", "plDynamicEnvMap", "plSimpleRegionSensor", "plMorphSequence",
        "plEmoteAnim", "plDynaRippleVSMgr", "UNUSED_plWaveSet6", "pfGUIProgressCtrl",
        "plMaintainersMarkerModifier", "plMorphSequenceSDLMod", "plMorphDataSet",
        "plHardRegion", "plHardRegionPlanes", "plHardRegionComplex",
        "plHardRegionUnion", "plHardRegionIntersect", "plHardRegionInvert",
        "plVisRegion", "plVisMgr", "plRegionBase", "pfGUIPopUpMenu", "pfGUIMenuItem",
        "plCoopCoordinator", "plFont", "plFontCache", "plRelevanceRegion",
        "plRelevanceMgr", "pfJournalBook", "plLayerTargetContainer", "plImageLibMod",
        "plParticleFlockEffect", "plParticleSDLMod", "plAgeLoader", "plWaveSetBase",
        "plPhysicalSndGroup", "pfBookData", "plDynaTorpedoMgr", "plDynaTorpedoVSMgr",
        "plClusterGroup", "plGameMarkerModifier", "plLODMipmap", "plSwimDetector",
        "plFadeOpacityMod", "plFadeOpacityLay", "plDistOpacityMod", "plArmatureModBase",
        "plSwimRegionInterface", "plSwimCircularCurrentRegion",
        "plParticleFollowSystemEffect", "plSwimStraightCurrentRegion",
        "pfObjectFlocker", "plGrassShaderMod", "plDynamicCamMap",
        "plRidingAnimatedPhysicalDetector", "plVolumeSensorConditionalObjectNoArbitration",
    };

    static const char* s_nonKeyedNames[] = {
        "plObjRefMsg", "plNodeRefMsg", "plMessage", "plRefMsg", "plGenRefMsg",
        "plTimeMsg", "plAnimCmdMsg", "plParticleUpdateMsg", "plLayRefMsg",
        "plMatRefMsg", "plCameraMsg", "plInputEventMsg", "plKeyEventMsg",
        "plMouseEventMsg", "plEvalMsg", "plTransformMsg", "plControlEventMsg",
        "plVaultCCRNode", "plLOSRequestMsg", "plLOSHitMsg", "plSingleModMsg",
        "plMultiModMsg", "plAvatarPhysicsEnableCallbackMsg", "plMemberUpdateMsg",
        "plNetMsgPagingRoom", "plActivatorMsg", "plDispatch", "plReceiver",
        "plMeshRefMsg", "hsGRenderProcs", "hsSfxAngleFade", "hsSfxDistFade",
        "hsSfxDistShade", "hsSfxGlobalShade", "hsSfxIntenseAlpha", "hsSfxObjDistFade",
        "hsSfxObjDistShade", "hsDynamicValue", "hsDynamicScalar",
        "hsDynamicColorRGBA", "hsDynamicMatrix33", "hsDynamicMatrix44",
        "plOmniSqApplicator", "plPreResourceMsg", "UNUSED_hsDynamicColorRGBA",
        "UNUSED_hsDynamicMatrix33", "UNUSED_hsDynamicMatrix44", "plController",
        "plLeafController", "plCompoundController", "UNUSED_plRotController",
        "UNUSED_plPosController", "UNUSED_plScalarController",
        "UNUSED_plPoint3Controller", "UNUSED_plScaleValueController",
        "UNUSED_plQuatController", "UNUSED_plMatrix33Controller",
        "UNUSED_plMatrix44Controller", "UNUSED_plEaseController",
        "UNUSED_plSimpleScaleController", "UNUSED_plSimpleRotController",
        "plCompoundRotController", "UNUSED_plSimplePosController",
        "plCompoundPosController", "plTMController", "hsFogControl", "plIntRefMsg",
        "plCollisionReactor", "plCorrectionMsg", "plPhysicalModifier", "plPickedMsg",
        "plCollideMsg", "plTriggerMsg", "plInterestingModMsg", "plDebugKeyEventMsg",
        "plPhysicalProperties_DEAD", "plSimplePhys", "plMatrixUpdateMsg",
        "plCondRefMsg", "plTimerCallbackMsg", "plEventCallbackMsg", "plSpawnModMsg",
        "plSpawnRequestMsg", "plLoadCloneMsg", "plEnableMsg", "plWarpMsg",
        "plAttachMsg", "pfConsole", "plRenderMsg", "plAnimTimeConvert",
        "plSoundMsg", "plInterestingPing", "plNodeCleanupMsg", "plSpaceTree",
        "plNetMessage", "plNetMsgJoinReq", "plNetMsgJoinAck", "plNetMsgLeave",
        "plNetMsgPing", "plNetMsgRoomsList", "plNetMsgGroupOwner",
        "plNetMsgGameStateRequest", "plNetMsgSessionReset", "plNetMsgOmnibus",
        "plNetMsgObject", "plCCRInvisibleMsg", "plLinkInDoneMsg", "plNetMsgGameMessage",
        "plNetMsgStream", "plAudioSysMsg", "plDispatchBase", "plServerReplyMsg",
        "plDeviceRecreateMsg", "plNetMsgStreamHelper", "plNetMsgObjectHelper",
        "plIMouseXEventMsg", "plIMouseYEventMsg", "plIMouseBEventMsg",
        "plLogicTriggerMsg", "plPipeline", "plDXPipeline", "plNetMsgVoice",
        "plLightRefMsg", "plNetMsgStreamedObject", "plNetMsgSharedState",
        "plNetMsgTestAndSet", "plNetMsgGetSharedState", "plSharedStateMsg",
        "plNetGenericServerTask", "plNetClientMgrMsg", "plLoadAgeMsg",
        "plMessageWithCallbacks", "plClientMsg", "plClientRefMsg",
        "plNetMsgObjStateRequest", "plCCRPetitionMsg", "plVaultCCRInitializationTask",
        "plNetServerMsg", "plNetServerMsgWithContext", "plNetServerMsgRegisterServer",
        "plNetServerMsgUnregisterServer", "plNetServerMsgStartProcess",
        "plNetServerMsgRegisterProcess", "plNetServerMsgUnregisterProcess",
        "plNetServerMsgFindProcess", "plNetServerMsgProcessFound",
        "plNetMsgRoutingInfo", "plNetServerSessionInfo", "plSimulationMsg",
        "plSimulationSynchMsg", "plHKSimulationSynchMsg", "plAvatarMsg",
        "plAvTaskMsg", "plAvSeekMsg", "plAvOneShotMsg", "plSatisfiedMsg",
        "plNetMsgObjectListHelper", "plNetMsgObjectUpdateFilter", "plProxyDrawMsg",
        "plSelfDestructMsg", "plSimInfluenceMsg", "plForceMsg", "plOffsetForceMsg",
        "plTorqueMsg", "plImpulseMsg", "plOffsetImpulseMsg", "plAngularImpulseMsg",
        "plDampMsg", "plShiftMassMsg", "plSimStateMsg", "plFreezeMsg",
        "plEventGroupMsg", "plSuspendEventMsg", "plNetMsgMembersListReq",
        "plNetMsgMembersList", "plNetMsgMemberInfoHelper", "plNetMsgMemberListHelper",
        "plNetMsgMemberUpdate", "plNetMsgServerToClient", "plNetMsgCreatePlayer",
        "plNetMsgAuthenticateHello", "plNetMsgAuthenticateChallenge",
        "plConnectedToVaultMsg", "plCCRCommunicationMsg",
        "plNetMsgInitialAgeStateSent", "plInitialAgeStateLoadedMsg",
        "plNetServerMsgFindServerBase", "plNetServerMsgFindServerReplyBase",
        "plNetServerMsgFindAuthServer", "plNetServerMsgFindAuthServerReply",
        "plNetServerMsgFindVaultServer", "plNetServerMsgFindVaultServerReply",
        "plAvTaskSeekDoneMsg", "plNCAgeJoinerMsg", "plNetServerMsgVaultTask",
        "plNetMsgVaultTask", "plAgeLinkStruct", "plVaultAgeInfoNode",
        "plNetMsgStreamableHelper", "plNetMsgReceiversListHelper",
        "plNetMsgListenListUpdate", "plNetServerMsgPing", "plNetMsgAlive",
        "plNetMsgTerminated", "plSDLModifierMsg", "plNetMsgSDLState",
        "plNetServerMsgSessionReset", "plCCRBanLinkingMsg", "plCCRSilencePlayerMsg",
        "plRenderRequestMsg", "plRenderRequestAck", "plNetMember", "plNetGameMember",
        "plNetTransportMember", "plConvexVolume", "plParticleGenerator",
        "plSimpleParticleGenerator", "plParticleEmitter", "plAGChannel",
        "plMatrixChannel", "plMatrixTimeScale", "plMatrixBlend",
        "plMatrixControllerChannel", "plQuatPointCombine", "plPointChannel",
        "plPointConstant", "plPointBlend", "plQuatChannel", "plQuatConstant",
        "plQuatBlend", "plLinkToAgeMsg", "plPlayerPageMsg", "plCmdIfaceModMsg",
        "plNetServerMsgPlsUpdatePlayer", "plListenerMsg", "plAnimPath",
        "plClothingUpdateBCMsg", "plNotifyMsg", "plFakeOutMsg", "plCursorChangeMsg",
        "plNodeChangeMsg", "UNUSED_plAvEnableMsg", "plLinkCallbackMsg",
        "plTransitionMsg", "plConsoleMsg", "plVolumeIsect", "plSphereIsect",
        "plConeIsect", "plCylinderIsect", "plParallelIsect", "plConvexIsect",
        "plComplexIsect", "plUnionIsect", "plIntersectionIsect", "plModulator",
        "UNUSED___plInventoryMsg", "plLinkEffectsTriggerMsg", "plLinkEffectBCMsg",
        "plResponderEnableMsg", "plNetServerMsgHello", "plNetServerMsgHelloReply",
        "plNetServerMember", "plResponderMsg", "plOneShotMsg",
        "plVaultAgeInfoListNode", "plNetServerMsgServerRegistered",
        "plPointTimeScale", "plPointControllerChannel", "plQuatTimeScale",
        "plAGApplicator", "plMatrixChannelApplicator", "plPointChannelApplicator",
        "plLightDiffuseApplicator", "plLightAmbientApplicator",
        "plLightSpecularApplicator", "plOmniApplicator", "plQuatChannelApplicator",
        "plScalarChannel", "plScalarTimeScale", "plScalarBlend",
        "plScalarControllerChannel", "plScalarChannelApplicator",
        "plSpotInnerApplicator", "plSpotOuterApplicator", "plNetServerMsgPlsRoutableMsg",
        "_UNUSED_plPuppetBrainMsg", "plATCEaseCurve", "plConstAccelEaseCurve",
        "plSplineEaseCurve", "plVaultAgeInfoInitializationTask", "pfGameGUIMsg",
        "plNetServerMsgVaultRequestGameState", "plNetServerMsgVaultGameState",
        "plNetServerMsgVaultGameStateSave", "plNetServerMsgVaultGameStateSaved",
        "plNetServerMsgVaultGameStateLoad", "plNetClientTask", "plNetMsgSDLStateBCast",
        "plReplaceGeometryMsg", "plNetServerMsgExitProcess",
        "plNetServerMsgSaveGameState", "plDniCoordinateInfo",
        "plNetMsgGameMessageDirected", "plLinkOutUnloadMsg", "plScalarConstant",
        "plMatrixConstant", "plAGCmdMsg", "plParticleTransferMsg",
        "plParticleKillMsg", "plExcludeRegionMsg", "plOneTimeParticleGenerator",
        "plParticleApplicator", "plParticleLifeMinApplicator",
        "plParticleLifeMaxApplicator", "plParticlePPSApplicator",
        "plParticleAngleApplicator", "plParticleVelMinApplicator",
        "plParticleVelMaxApplicator", "plParticleScaleMinApplicator",
        "plParticleScaleMaxApplicator", "plDynamicTextMsg", "plCameraTargetFadeMsg",
        "plAgeLoadedMsg", "plPointControllerCacheChannel",
        "plScalarControllerCacheChannel", "plLinkEffectsTriggerPrepMsg",
        "plLinkEffectPrepBCMsg", "plAvatarInputStateMsg", "plAgeInfoStruct",
        "plSDLNotificationMsg", "plNetClientConnectAgeVaultTask", "plLinkingMgrMsg",
        "plVaultNotifyMsg", "plPlayerInfo", "plSwapSpansRefMsg", "pfKI",
        "plDISpansMsg", "plNetMsgCreatableHelper", "plCreatableUuid",
        "plNetMsgRequestMyVaultPlayerList", "plDelayedTransformMsg",
        "plSuperVNodeMgrInitTask", "plElementRefMsg", "plClothingMsg",
        "plEventGroupEnableMsg", "pfGUINotifyMsg", "UNUSED_plAvBrain",
        "plArmatureBrain", "plAvBrainHuman", "plAvBrainCritter", "plAvBrainDrive",
        "plAvBrainSample", "plAvBrainGeneric", "plPreloaderMsg", "plAvBrainLadder",
        "plInputIfaceMgrMsg", "pfKIMsg", "plRemoteAvatarInfoMsg",
        "plMatrixDelayedCorrectionApplicator", "plAvPushBrainMsg",
        "plAvPopBrainMsg", "plRoomLoadNotifyMsg", "plAvTask", "plAvAnimTask",
        "plAvSeekTask", "plNetCommAuthConnectedMsg", "plAvOneShotTask",
        "UNUSED_plAvEnableTask", "plAvTaskBrain", "plAnimStage", "plNetClientMember",
        "plNetClientCommTask", "plNetServerMsgAuthRequest",
        "plNetServerMsgAuthReply", "plNetClientCommAuthTask", "plClientGuid",
        "plNetMsgVaultPlayerList", "plNetMsgSetMyActivePlayer",
        "plNetServerMsgRequestAccountPlayerList", "plNetServerMsgAccountPlayerList",
        "plNetMsgPlayerCreated", "plNetServerMsgVaultCreatePlayer",
        "plNetServerMsgVaultPlayerCreated", "plNetMsgFindAge", "plNetMsgFindAgeReply",
        "plNetClientConnectPrepTask", "plNetClientAuthTask",
        "plNetClientGetPlayerVaultTask", "plNetClientSetActivePlayerTask",
        "plNetClientFindAgeTask", "plNetClientLeaveTask", "plNetClientJoinTask",
        "plNetClientCalibrateTask", "plNetMsgDeletePlayer",
        "plNetServerMsgVaultDeletePlayer", "plNetCoreStatsSummary",
        "plCreatableGenericValue", "plCreatableListHelper", "plCreatableStream",
        "plAvBrainGenericMsg", "plAvTaskSeek", "plAGInstanceCallbackMsg",
        "plArmatureEffectMsg", "plArmatureEffectStateMsg", "plShadowCastMsg",
        "plBoundsIsect", "plResMgrHelperMsg", "plNetCommAuthMsg", "plNetCommFileListMsg",
        "plNetCommFileDownloadMsg", "plNetCommLinkToAgeMsg", "plNetCommPlayerListMsg",
        "plNetCommActivePlayerMsg", "plNetCommCreatePlayerMsg",
        "plNetCommDeletePlayerMsg", "plNetCommPublicAgeListMsg", "plNetCommPublicAgeMsg",
        "plNetCommRegisterAgeMsg", "plVaultAdminInitializationTask",
        "plMultistageModMsg", "plSoundVolumeApplicator", "plCutter", "plBulletMsg",
        "plDynaDecalEnableMsg", "plOmniCutoffApplicator", "plArmatureUpdateMsg",
        "plAvatarFootMsg", "plNetOwnershipMsg", "plNetMsgRelevanceRegions",
        "plParticleFlockMsg", "plAvatarBehaviorNotifyMsg", "plATCChannel",
        "plScalarSDLChannel", "plLoadAvatarMsg", "plAvatarSetTypeMsg",
        "plNetMsgLoadClone", "plNetMsgPlayerPage", "plVNodeInitTask", "plRippleShapeMsg",
        "plEventManager", "plVaultNeighborhoodInitializationTask",
        "plNetServerMsgAgentRecoveryRequest", "plNetServerMsgFrontendRecoveryRequest",
        "plNetServerMsgBackendRecoveryRequest", "plNetServerMsgAgentRecoveryData",
        "plNetServerMsgFrontendRecoveryData", "plNetServerMsgBackendRecoveryData",
        "plSubWorldMsg", "plMatrixDifferenceApp", "plAvatarSpawnNotifyMsg",
    };

    static const char* s_postDbNames[] = {
        "plVaultGameServerInitializationTask", "plNetClientFindDefaultAgeTask",
        "plVaultAgeNode", "plVaultAgeInitializationTask", "plSetListenerMsg",
        "plVaultSystemNode", "plAvBrainSwim", "plNetMsgVault", "plNetServerMsgVault",
        "plVaultTask", "plVaultConnectTask", "plVaultNegotiateManifestTask",
        "plVaultFetchNodesTask", "plVaultSaveNodeTask", "plVaultFindNodeTask",
        "plVaultAddNodeRefTask", "plVaultRemoveNodeRefTask", "plVaultSendNodeTask",
        "plVaultNotifyOperationCallbackTask", "plVNodeMgrInitializationTask",
        "plVaultPlayerInitializationTask", "plNetVaultServerInitializationTask",
        "plCommonNeighborhoodsInitTask", "plVaultNodeRef", "plVaultNode",
        "plVaultFolderNode", "plVaultImageNode", "plVaultTextNoteNode",
        "plVaultSDLNode", "plVaultAgeLinkNode", "plVaultChronicleNode",
        "plVaultPlayerInfoNode", "plVaultMgrNode", "plVaultPlayerNode",
        "plSynchEnableMsg", "plNetVaultServerNode", "plVaultAdminNode",
        "plVaultGameServerNode", "plVaultPlayerInfoListNode",
        "plAvatarStealthModeMsg", "plEventCallbackInterceptMsg",
        "plDynamicEnvMapMsg", "plClimbMsg", "plIfaceFadeAvatarMsg", "plAvBrainClimb",
        "plSharedMeshBCMsg", "plNetVoiceListMsg", "plSwimMsg", "plMorphDelta",
        "plMatrixControllerCacheChannel", "plVaultMarkerNode", "pfMarkerMsg",
        "plPipeResMakeMsg", "plPipeRTMakeMsg", "plPipeGeoMakeMsg", "plAvCoopMsg",
        "plAvBrainCoop", "plSimSuppressMsg", "plVaultMarkerListNode",
        "UNUSED_plAvTaskOrient", "plAgeBeginLoadingMsg", "plSetNetGroupIDMsg",
        "pfBackdoorMsg", "plAIMsg", "plAIBrainCreatedMsg", "plStateDataRecord",
        "plNetClientCommDeletePlayerTask", "plNetMsgSetTimeout",
        "plNetMsgActivePlayerSet", "plNetClientCommSetTimeoutTask",
        "plNetRoutableMsgOmnibus", "plNetMsgGetPublicAgeList",
        "plNetMsgPublicAgeList", "plNetMsgCreatePublicAge",
        "plNetMsgPublicAgeCreated", "plNetServerMsgEnvelope",
        "plNetClientCommGetPublicAgeListTask", "plNetClientCommCreatePublicAgeTask",
        "plNetServerMsgPendingMsgs", "plNetServerMsgRequestPendingMsgs",
        "plDbInterface", "plDbProxyInterface", "plDBGenericSQLDB", "pfGameMgrMsg",
        "pfGameCliMsg", "pfGameCli", "pfGmTicTacToe", "pfGmHeek", "pfGmMarker",
        "pfGmBlueSpiral", "pfGmClimbingWall", "plAIArrivedAtGoalMsg",
        "pfGmVarSync", "plNetMsgRemovePublicAge", "plNetMsgPublicAgeRemoved",
        "plNetClientCommRemovePublicAgeTask", "plCCRMessage", "plAvOneShotLinkTask",
        "plNetAuthDatabase", "plAvatarOpacityCallbackMsg", "plAGDetachCallbackMsg",
        "pfMovieEventMsg", "plMovieMsg", "plPipeTexMakeMsg", "plEventLog",
        "plDbEventLog", "plSyslogEventLog", "plCaptureRenderMsg", "plAgeLoaded2Msg",
        "plPseudoLinkEffectMsg", "plPseudoLinkAnimTriggerMsg",
        "plPseudoLinkAnimCallbackMsg", "__UNUSED__pfClimbingWallMsg",
        "plClimbEventMsg", "__UNUSED__plAvBrainQuab", "plAccountUpdateMsg",
        "plLinearVelocityMsg", "plAngularVelocityMsg", "plRideAnimatedPhysMsg",
        "plAvBrainRideAnimatedPhysical",
    };

    if (type < (sizeof(s_keyedNames) / sizeof(s_keyedNames[0])))
        return s_keyedNames[type];
    if (type >= 0x200 && (type - 0x200) < sizeof(s_nonKeyedNames) / sizeof(s_nonKeyedNames[0]))
        return s_nonKeyedNames[type - 0x200];
    if (type >= 0x400 && (type - 0x400) < sizeof(s_postDbNames) / sizeof(s_postDbNames[0]))
        return s_postDbNames[type - 0x400];

    OutputDebugStringW(QString("Unknown class ID (%1)\n").arg(type, 4, 16, QChar('0')).toStdWString().c_str());
    return QString("Unknown class ID (%1)").arg(type, 4, 16, QChar('0'));
}

QString Factory_Create(QTreeWidgetItem* parent, ChunkBuffer& buffer, size_t size)
{
    unsigned short type = buffer.read<unsigned short>();

    switch (type) {
    case kNetMsgPagingRoom:
        Create_NetMsgPagingRoom(parent, buffer);
        break;
    case kNetMsgGroupOwner:
        Create_NetMsgGroupOwner(parent, buffer);
        break;
    case kNetMsgGameStateRequest:
        Create_NetMsgRoomsList(parent, buffer);
        break;
    case kNetMsgGameMessage:
        Create_NetMsgGameMessage(parent, buffer);
        break;
    case kServerReplyMsg:
        Create_ServerReplyMsg(parent, buffer);
        break;
    case kNetMsgTestAndSet:
        Create_NetMsgSharedState(parent, buffer);
        break;
    case kAvTaskMsg:
        Create_AvTaskMsg(parent, buffer);
        break;
    case kNetMsgMembersListReq:
        Create_NetMsgMembersListReq(parent, buffer);
        break;
    case kNetMsgMembersList:
        Create_NetMsgMembersList(parent, buffer);
        break;
    case kNetMsgMemberUpdate:
        Create_NetMsgMemberUpdate(parent, buffer);
        break;
    case kNetMsgInitialAgeStateSent:
        Create_NetMsgInitialAgeStateSent(parent, buffer);
        break;
    case kNetMsgSDLState:
        Create_NetMsgSDLState(parent, buffer);
        break;
    case kNotifyMsg:
        Create_NotifyMsg(parent, buffer);
        break;
    case kLinkEffectsTriggerMsg:
        Create_LinkEffectsTriggerMsg(parent, buffer);
        break;
    case kNetMsgSDLStateBCast:
        Create_NetMsgSDLState(parent, buffer);
        break;
    case kNetMsgGameMessageDirected:
        Create_NetMsgGameMessageDirected(parent, buffer);
        break;
    case kAvatarInputStateMsg:
        Create_AvatarInputStateMsg(parent, buffer);
        break;
    case kAvBrainHuman:
        Create_AvBrainHuman(parent, buffer);
        break;
    case kAvBrainCritter:
        Create_AvBrainCritter(parent, buffer);
        break;
    case kAvBrainDrive:
        Create_AvBrainDrive(parent, buffer);
        break;
    case kAvBrainGeneric:
        Create_AvBrainGeneric(parent, buffer);
        break;
    case kInputIfaceMgrMsg:
        Create_InputIfaceMgrMsg(parent, buffer);
        break;
    case kKIMessage:
        Create_KIMessage(parent, buffer);
        break;
    case kAvAnimTask:
        Create_AvAnimTask(parent, buffer);
        break;
    case kAvSeekTask:
        Create_AvSeekTask(parent, buffer);
        break;
    case kAvOneShotTask:
        Create_AvOneShotTask(parent, buffer);
        break;
    case kAvTaskBrain:
        Create_AvTaskBrain(parent, buffer);
        break;
    case kAnimStage:
        Create_AnimStage(parent, buffer);
        break;
    case kAvBrainGenericMsg:
        Create_AvBrainGenericMsg(parent, buffer);
        break;
    case kAvTaskSeek:
        Create_AvTaskSeek(parent, buffer);
        break;
    case kNetMsgRelevanceRegions:
        Create_NetMsgRelevanceRegions(parent, buffer);
        break;
    case kLoadAvatarMsg:
        Create_LoadAvatarMsg(parent, buffer);
        break;
    case kNetMsgLoadClone:
        Create_NetMsgLoadClone(parent, buffer);
        break;
    case kNetMsgPlayerPage:
        Create_NetMsgPlayerPage(parent, buffer);
        break;
    case kAvBrainSwim:
        Create_AvBrainSwim(parent, buffer);
        break;
    case kAvBrainClimb:
        Create_AvBrainClimb(parent, buffer);
        break;
    case kAvOneShotLinkTask:
        Create_AvOneShotLinkTask(parent, buffer);
        break;
    case kAvBrainRideAnimatedPhysical:
        Create_AvBrainRideAnimatedPhysical(parent, buffer);
        break;
    case 0x8000:
        return "(NULL)";
    default:
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
                << QString("Unsupported creatable (%1)").arg(type, 4, 16, QChar('0')));

            while (item) {
                QFont warnFont = item->font(0);
                warnFont.setBold(true);
                item->setFont(0, warnFont);
                item->setForeground(0, Qt::red);
                item = item->parent();
            }

            OutputDebugStringW(QString("Unsupported creatable (%1)\n")
                               .arg(type, 4, 16, QChar('0')).toStdWString().c_str());
            if (size) {
                buffer.skip(size - sizeof(unsigned short));
            } else {
                buffer.clear();
            }
        }
    }

    return Factory_Name(type);
}

void FlagField(QTreeWidgetItem* parent, const QString& title,
               unsigned flags, const char* names[])
{
    QTreeWidgetItem* top = new QTreeWidgetItem(parent, QStringList()
        << QString("%1: 0x%2").arg(title).arg(flags, 8, 16, QChar('0')));

    int idx = 0;
    while (flags) {
        if (flags & 0x1)
            new QTreeWidgetItem(top, QStringList() << names[idx]);
        flags >>= 1;
        idx += 1;
    }
}

void Location(QTreeWidgetItem* parent, const QString& title, ChunkBuffer& buffer)
{
    new QTreeWidgetItem(parent, QStringList()
        << QString("%1: %2 (Flags: %3)").arg(title)
           .arg(buffer.read<unsigned>(), 8, 16, QChar('0'))
           .arg(buffer.read<unsigned short>(), 4, 16, QChar('0')));
}

enum UoidContents
{
    kHasCloneIDs    = (1<<0),
    kHasLoadMask    = (1<<1),
};

void Uoid(QTreeWidgetItem* parent, const QString& title, ChunkBuffer& buffer)
{
    static const char* s_uoidContents[] = {
        "kHasCloneIDs", "kHasLoadMask", "(1<<2)", "(1<<3)",
        "(1<<4)", "(1<<5)", "(1<<6)", "(1<<7)"
    };

    QTreeWidgetItem* top = new QTreeWidgetItem(parent, QStringList() << title);
    unsigned char contents = buffer.read<unsigned char>();
    FlagField(top, "Contents", contents, s_uoidContents);
    Location(top, "Location", buffer);
    if (contents & kHasLoadMask) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Load Mask: %2")
               .arg(buffer.read<unsigned char>(), 2, 16, QChar('0')));
    }
    new QTreeWidgetItem(top, QStringList()
        << QString("Type: %1").arg(Factory_Name(buffer.read<unsigned short>())));
    new QTreeWidgetItem(top, QStringList()
        << QString("ID: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(top, QStringList()
        << QString("Name: %1").arg(buffer.readSafeString()));
    if (contents & kHasCloneIDs) {
        new QTreeWidgetItem(top, QStringList()
            << QString("Clone ID: %1").arg(buffer.read<unsigned>()));
        new QTreeWidgetItem(top, QStringList()
            << QString("Clone Player ID: %1").arg(buffer.read<unsigned>()));
    }
}

void Key(QTreeWidgetItem* parent, const QString& title, ChunkBuffer& buffer)
{
    if (buffer.read<bool>()) {
        Uoid(parent, title, buffer);
    } else {
        new QTreeWidgetItem(parent, QStringList()
            << QString("%1: (NULL)").arg(title));
    }
}

void BitVector(QTreeWidgetItem* parent, const QString& title, ChunkBuffer& buffer)
{
    QTreeWidgetItem* top = new QTreeWidgetItem(parent, QStringList() << title);

    unsigned wordCount = buffer.read<unsigned>();
    for (unsigned i = 0; i < wordCount; ++i) {
        unsigned value = (i * 32);
        unsigned bits = buffer.read<unsigned>();
        while (bits) {
            if (bits & 0x1)
                new QTreeWidgetItem(top, QStringList() << QString("%1").arg(value));
            bits >>= 1;
            ++value;
        }
    }
}
