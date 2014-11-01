include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENAL_INCLUDE_DIR})

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

add_library(plAvatar STATIC ${plAvatar_SOURCES} ${plAvatar_HEADERS})

target_link_libraries(plAvatar plAnimation)

source_group("Source Files" FILES ${plAvatar_SOURCES})
source_group("Header Files" FILES ${plAvatar_HEADERS})
