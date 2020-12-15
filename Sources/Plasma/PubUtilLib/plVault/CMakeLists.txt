include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

source_group("Source Files" FILES ${plVault_SOURCES})
source_group("Header Files" FILES ${plVault_HEADERS})
