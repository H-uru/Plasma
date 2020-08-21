if(WIN32)
    option(SoundDecompress_MOULa_Mode "Enable legacy MOULa mode in SoundDecompress" OFF)
    if(SoundDecompress_MOULa_Mode)
        set(OUTPUT_WAV_FILES ON)
    endif()
else(WIN32)
    set(OUTPUT_WAV_FILES OFF)
endif(WIN32)
configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories("../../Plasma/PubUtilLib")

set(SoundDecompress_SOURCES
    plAllCreatables.cpp
    main.cpp
)

set(SoundDecompress_HEADERS
)

if(SoundDecompress_MOULa_Mode)
    set(SoundDecompress_SOURCES plOldAudioFileReader.cpp ${SoundDecompress_SOURCES})
    set(SoundDecompress_HEADERS plOldAudioFileReader.h)
endif()

add_executable(SoundDecompress ${SoundDecompress_SOURCES})
target_link_libraries(SoundDecompress CoreLib plResMgr plAudioCore)
target_link_libraries(SoundDecompress ${STRING_THEORY_LIBRARIES})

if(SoundDecompress_MOULa_Mode)
    target_link_libraries(SoundDecompress ${DirectX_LIBRARIES})
endif()

if(USE_VLD)
    target_link_libraries(SoundDecompress ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${SoundDecompress_SOURCES})
source_group("Header Files" FILES ${SoundDecompress_HEADERS})

