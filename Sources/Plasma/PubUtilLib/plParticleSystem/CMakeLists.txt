set(plParticleSystem_SOURCES
    plBoundInterface.cpp
    plConvexVolume.cpp
    plParticleApplicator.cpp
    plParticleEffect.cpp
    plParticleEmitter.cpp
    plParticleGenerator.cpp
    plParticleSDLMod.cpp
    plParticleSystem.cpp
)

set(plParticleSystem_HEADERS
    plBoundInterface.h
    plConvexVolume.h
    plEffectTargetInfo.h
    plParticle.h
    plParticleApplicator.h
    plParticleCreatable.h
    plParticleEffect.h
    plParticleEmitter.h
    plParticleGenerator.h
    plParticleSDLMod.h
    plParticleSystem.h
)

plasma_library(plParticleSystem
    FOLDER PubUtilLib
    SOURCES ${plParticleSystem_SOURCES} ${plParticleSystem_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    plParticleSystem
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnModifier
        pnNetCommon
        pnSceneObject
        plAnimation
        plModifier
    PRIVATE
        pnEncryption
        pnMessage
        pnNucleusInc
        pnSceneObject
        plDrawable
        plInterp
        plMessage
        plSDL
        plSurface
)

source_group("Source Files" FILES ${plParticleSystem_SOURCES})
source_group("Header Files" FILES ${plParticleSystem_HEADERS})
