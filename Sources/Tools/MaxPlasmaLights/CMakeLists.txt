set(MaxPlasmaLights_HEADERS
    plRealTimeLightBase.h
    plRealTimeLights.h
    plRealTimeLightsPBDec.h
    plRTObjLightDesc.h
    plRTPlasmaLights.def
    plRTProjDirLight.h
    plRTProjDirLightClassDesc.h
    plRTProjDirLightPBDec.h
)

set(MaxPlasmaLights_RESOURCES
    plRTLights.rc
    resource.h
)


set(MaxPlasmaLights_SOURCES
    DLLEntry.cpp
    plRealTimeLightBase.cpp
    plRealTimeLights.cpp
    plRTProjDirLight.cpp
)

plasma_library(MaxPlasmaLights SHARED
    SOURCES ${MaxPlasmaLights_HEADERS} ${MaxPlasmaLights_RESOURCES} ${MaxPlasmaLights_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    NO_SANITIZE # The 3ds Max headers are full of clang diagnostic errors.
)
target_link_libraries(
    MaxPlasmaLights
    PUBLIC
        CoreLib
        pnNucleusInc
    PRIVATE
        pnKeyedObject
        pnMessage
        pnNetCommon
        plGLight
        plSurface

        MaxComponent
        MaxPlasmaMtls

        3dsm
        Comctl32
)

set_target_properties(MaxPlasmaLights PROPERTIES SUFFIX ".dlo")

install(
    TARGETS MaxPlasmaLights
    DESTINATION 3dsmax
)

source_group("Header Files" FILES ${MaxPlasmaLights_HEADERS})
source_group("Resource Files" FILES ${MaxPlasmaLights_RESOURCES})
source_group("Source Files" FILES ${MaxPlasmaLights_SOURCES})
