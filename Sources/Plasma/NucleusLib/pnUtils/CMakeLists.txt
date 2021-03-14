set(pnUtils_HEADERS
    Pch.h
    pnUtils.h
    pnUtCoreLib.h
    pnUtAllIncludes.h
    pnUtCrypt.h
    pnUtHash.h
    pnUtList.h
    pnUtStr.h
    pnUtTime.h
)

set(pnUtils_SOURCES
    pnUtCrypt.cpp
    pnUtHash.cpp
    pnUtList.cpp
    pnUtStr.cpp
    pnUtTime.cpp
)

plasma_library(pnUtils
    SOURCES ${pnUtils_HEADERS} ${pnUtils_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    UNITY_BUILD
)
target_link_libraries(
    pnUtils
    PUBLIC
        CoreLib
        pnEncryption
    PRIVATE
        OpenSSL::Crypto
)

source_group("Header Files" FILES ${pnUtils_HEADERS})
source_group("Source Files" FILES ${pnUtils_SOURCES})
