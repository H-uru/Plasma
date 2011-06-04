add_definitions(-D_LIB)

include_directories("../../Plasma/CoreLib")

set(CoreLibExe_SOURCES
    hsExeError.cpp
    hsExeMalloc.cpp
)

set(CoreLibExe_HEADERS
    Intern.h
    Pch.h
)

add_library(CoreLibExe STATIC ${CoreLibExe_SOURCES} ${CoreLibExe_HEADERS})

source_group("Source Files" FILES ${CoreLibExe_SOURCES})
source_group("Header Files" FILES ${CoreLibExe_HEADERS})
