set(plVault_SOURCES
    plDniCoordinateInfo.cpp
    plVaultClientApi.cpp
    plVaultConstants.cpp
    plVaultNodeAccess.cpp
)

set(plVault_HEADERS
    Intern.h
    Pch.h
    plDniCoordinateInfo.h
    plVault.h
    plVaultClientApi.h
    plVaultConstants.h
    plVaultCreatable.h
    plVaultNodeAccess.h
)

plasma_library(plVault
    FOLDER PubUtilLib
    SOURCES ${plVault_SOURCES} ${plVault_HEADERS}
    PRECOMPILED_HEADERS Pch.h
    UNITY_BUILD
)
target_link_libraries(
    plVault
    PUBLIC
        CoreLib
        pnNetBase
        pnNetProtocol # plVaultNodeAccess
    PRIVATE
        pnDispatch
        pnNucleusInc
        plGImage
        plMessage
        plNetCommon
        plNetGameLib
        plSDL
        plStatusLog
        plUnifiedTime
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plVault_SOURCES})
source_group("Header Files" FILES ${plVault_HEADERS})
