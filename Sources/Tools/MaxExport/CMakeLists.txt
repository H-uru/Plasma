include_directories("../")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
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
