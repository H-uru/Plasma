include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUtilsExe_HEADERS
    Intern.h
    Pch.h
)

set(pnUtilsExe_PRIVATE
    Private/pnUteTime.cpp
)

if(WIN32)
    set(pnUtilsExe_WIN32
        Private/Win32/pnUteW32Time.cpp
    )
else()
    set(pnUtilsExe_UNIX
        Private/Unix/pnUteUxTime.cpp
    )
endif()

add_library(pnUtilsExe STATIC ${pnUtilsExe_HEADERS} ${pnUtilsExe_PRIVATE}
                              ${pnUtilsExe_UNIX} ${pnUtilsExe_WIN32})

source_group("Header Files" FILES ${pnUtilsExe_HEADERS})
source_group("Private" FILES ${pnUtilsExe_PRIVATE})
if(WIN32)
    source_group("Private\\Win32" FILES ${pnUtilsExe_WIN32})
else()
    source_group("Private\\Unix" FILES ${pnUtilsExe_UNIX})
endif()
