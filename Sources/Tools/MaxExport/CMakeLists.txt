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

add_library(MaxExport STATIC ${MaxExport_HEADERS} ${MaxExport_SOURCES})
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

plasma_optimize_target(MaxExport PRECOMPILED_HEADERS Pch.h)

source_group("Header Files" FILES ${MaxExport_HEADERS})
source_group("Source Files" FILES ${MaxExport_SOURCES})
