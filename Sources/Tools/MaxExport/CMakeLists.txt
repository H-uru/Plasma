include_directories("${PLASMA_TOOLS_ROOT}")
include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib/inc")
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

use_precompiled_header(Pch.h Pch.cpp MaxExport_HEADERS MaxExport_SOURCES)
add_library(MaxExport STATIC ${MaxExport_HEADERS} ${MaxExport_SOURCES})

source_group("Header Files" FILES ${MaxExport_HEADERS})
source_group("Source Files" FILES ${MaxExport_SOURCES})
