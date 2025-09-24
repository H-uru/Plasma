set(pnAsyncCore_HEADERS
    Pch.h
    pnAcCore.h
    pnAcIo.h
    pnAcLog.h
    pnAcThread.h
    pnAcTimer.h
    pnAsyncCore.h
)

set(pnAsyncCore_SOURCES
    pnAcLog.cpp
)

plasma_library(pnAsyncCore
    FOLDER NucleusLib
    SOURCES ${pnAsyncCore_HEADERS} ${pnAsyncCore_SOURCES}
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
source_group("Source Files" FILES ${pnAsyncCore_SOURCES})
