include_directories(../../CoreLib)
include_directories(../../NucleusLib)

set(pnCrashExe_SOURCES

)

set(pnCrashExe_HEADERS
    Pch.h
)

if(WIN32 AND NOT CYGWIN)
    set (pnCrashExe_SOURCES ${pnCrashExe_SOURCES}
        Win32/pnCreError.cpp
        Win32/pnCreGui.cpp
        Win32/pnCreMail.cpp
        Win32/pnCreTools.cpp
    )
    
    set (pnCrashExe_HEADERS ${pnCrashExe_HEADERS}
        Win32/W32Int.h
    )
endif(WIN32 AND NOT CYGWIN)

add_library(pnCrashExe STATIC ${pnCrashExe_SOURCES} ${pnCrashExe_HEADERS})

source_group("Source Files" FILES ${pnCrashExe_SOURCES})
source_group("Header Files" FILES ${pnCrashExe_HEADERS})