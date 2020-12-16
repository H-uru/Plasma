set(plVault_SOURCES
    plDniCoordinateInfo.cpp
    plVaultClientApi.cpp
    plVaultConstants.cpp
    plVaultNodeAccess.cpp
)

set(plVault_HEADERS
    Intern.h
    Pch.h
    plAgeInfoSource.h
    plDniCoordinateInfo.h
    plVault.h
    plVaultClientApi.h
    plVaultConstants.h
    plVaultCreatable.h
    plVaultNodeAccess.h
)

add_library(plVault STATIC ${plVault_SOURCES} ${plVault_HEADERS})

target_include_directories(plVault PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plVault_SOURCES})
source_group("Header Files" FILES ${plVault_HEADERS})
