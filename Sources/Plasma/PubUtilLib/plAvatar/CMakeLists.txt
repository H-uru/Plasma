set(plAvatar_SOURCES
    plAnimStage.cpp
    plArmatureEffects.cpp
    plArmatureMod.cpp
    plAvatarClothing.cpp
    plAvatarMgr.cpp
    plAvatarSDLModifier.cpp
    plAvatarTasks.cpp
    plAvBehaviors.cpp
    plAvBrain.cpp
    plAvBrainClimb.cpp
    plAvBrainCoop.cpp
    plAvBrainCritter.cpp
    plAvBrainDrive.cpp
    plAvBrainGeneric.cpp
    plAvBrainHuman.cpp
    plAvBrainRideAnimatedPhysical.cpp
    plAvBrainSwim.cpp
    plAvLadderModifier.cpp
    plAvTaskBrain.cpp
    plAvTaskSeek.cpp
    plClothingSDLModifier.cpp
    plCoopCoordinator.cpp
    plMultistageBehMod.cpp
    plNPCSpawnMod.cpp
    plOneShotMod.cpp
    plPhysicalControllerCore.cpp
    plSeekPointMod.cpp
    plSittingModifier.cpp
    plSwimRegion.cpp
)

set(plAvatar_HEADERS
    plAnimStage.h
    plArmatureEffects.h
    plArmatureMod.h
    plAvatarClothing.h
    plAvatarCreatable.h
    plAvatarMgr.h
    plAvatarSDLModifier.h
    plAvatarTasks.h
    plAvBehaviors.h
    plAvBrain.h
    plAvBrainClimb.h
    plAvBrainCoop.h
    plAvBrainCritter.h
    plAvBrainDrive.h
    plAvBrainGeneric.h
    plAvBrainHuman.h
    plAvBrainRideAnimatedPhysical.h
    plAvBrainSwim.h
    plAvDefs.h
    plAvLadderModifier.h
    plAvTask.h
    plAvTaskBrain.h
    plAvTaskSeek.h
    plClothingLayout.h
    plClothingSDLModifier.h
    plCoopCoordinator.h
    plCritterCommands.h
    plMultistageBehMod.h
    plNPCSpawnMod.h
    plOneShotMod.h
    plPhysicalControllerCore.h
    plSeekPointMod.h
    plSittingModifier.h
    plSwimRegion.h
)

plasma_library(plAvatar
    SOURCES ${plAvatar_SOURCES} ${plAvatar_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    plAvatar
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnModifier
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        pnTimer
        plAnimation
        plMessage
        plModifier
        plPhysical
    PRIVATE
        pnEncryption
        pnMessage
        plAudio
        plDrawable
        plGImage
        plInputCore
        plInterp
        plMessage
        plMessageBox
        plNetClient
        plNetCommon
        plNetMessage
        plNetTransport
        plParticleSystem
        plPipeline
        plResMgr
        plScene
        plSDL
        plStatusLog
        plSurface
        plVault
        pfAudio # :(
        pfCamera # :(
        pfMessage # :(
)

target_include_directories(plAvatar PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plAvatar_SOURCES})
source_group("Header Files" FILES ${plAvatar_HEADERS})
