set(plSDL_SOURCES
    plSDLMgr.cpp
    plSDLParser.cpp
    plStateChangeNotifier.cpp
    plStateDataRecord.cpp
    plStateDescriptor.cpp
    plStateVariable.cpp
    plVarDescriptor.cpp
)

set(plSDL_HEADERS
    plSDL.h
    plSDLCreatable.h
    plSDLDescriptor.h
)

set(plSDL_SDLFILES
    SDL/animation.sdl
    SDL/avatar.sdl
    SDL/cloneMessage.sdl
    SDL/clothing.sdl
    SDL/morph.sdl
    SDL/particle.sdl
    SDL/physical.sdl
    SDL/responder.sdl
    SDL/sound.sdl
    SDL/xregion.sdl
)

plasma_library(plSDL
    SOURCES ${plSDL_SOURCES} ${plSDL_HEADERS} ${plSDL_SDLFILES}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(plSDL
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        plUnifiedTime
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plFile
        plNetMessage
        plResMgr
)

source_group("Source Files" FILES ${plSDL_SOURCES})
source_group("Header Files" FILES ${plSDL_HEADERS})
source_group("SDL" FILES ${plSDL_SDLFILES})
