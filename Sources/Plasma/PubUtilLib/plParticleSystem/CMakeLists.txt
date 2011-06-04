include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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

add_library(plParticleSystem STATIC ${plParticleSystem_SOURCES} ${plParticleSystem_HEADERS})

source_group("Source Files" FILES ${plParticleSystem_SOURCES})
source_group("Header Files" FILES ${plParticleSystem_HEADERS})
