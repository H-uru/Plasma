set(pnAsyncCore_HEADERS
    Pch.h
    pnAsyncCore.h
)

set(pnAsyncCore_PRIVATE
    Private/pnAcAllIncludes.h
    Private/pnAcCore.h
    Private/pnAcIo.h
    Private/pnAcLog.cpp
    Private/pnAcLog.h
    Private/pnAcThread.h
    Private/pnAcTimer.h
)

plasma_library(pnAsyncCore
    FOLDER NucleusLib
    SOURCES ${pnAsyncCore_PRIVATE} ${pnAsyncCore_HEADERS}
)

target_include_directories(pnAsyncCore PRIVATE "${PLASMA_SOURCE_ROOT}/PubUtilLib")
target_link_libraries(
    pnAsyncCore
    PUBLIC
        CoreLib
        pnNetCommon
        pnUUID
    PRIVATE
        pnAsyncCoreExe # Implementation is here
        pnNetBase
        plStatusLog # :(
)

source_group("Header Files" FILES ${pnAsyncCore_HEADERS})
source_group("Private" FILES ${pnAsyncCore_PRIVATE})
