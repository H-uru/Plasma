set(MaxExport_HEADERS
    plErrorMsg.h
    plExportDlg.h
    plExportErrorMsg.h
    plExportLogErrorMsg.h
    plExportProgressBar.h
    plProgressBar.h
    SimpleExport.h
)

set(MaxExport_SOURCES
    plErrorMsg.cpp
    plExportDlg.cpp
    plExportErrorMsg.cpp
    plExportLogErrorMsg.cpp
    plExportProgressBar.cpp
    SimpleExport.cpp
)

plasma_library(MaxExport
    SOURCES ${MaxExport_HEADERS} ${MaxExport_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    NO_SANITIZE # The 3ds Max headers are full of clang diagnostic errors.
)
target_link_libraries(
    MaxExport
    PUBLIC
        CoreLib
        pnNucleusInc
    PRIVATE
        pnKeyedObject
        plAvatar
        plGImage
        plPhysX
        plResMgr
        plScene
        plSDL
        plStatusLog
        pfPython

        3dsm
)

source_group("Header Files" FILES ${MaxExport_HEADERS})
source_group("Source Files" FILES ${MaxExport_SOURCES})
