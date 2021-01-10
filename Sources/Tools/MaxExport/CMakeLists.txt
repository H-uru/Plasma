include_directories(${3dsm_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

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
use_precompiled_header(MaxExport Pch.h)

source_group("Header Files" FILES ${MaxExport_HEADERS})
source_group("Source Files" FILES ${MaxExport_SOURCES})
