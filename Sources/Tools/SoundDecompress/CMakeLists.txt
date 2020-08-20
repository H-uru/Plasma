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

if(WIN32)
    set(SoundDecompress_SOURCES plOldAudioFileReader.cpp ${SoundDecompress_SOURCES})
    set(SoundDecompress_HEADERS plOldAudioFileReader.h)
endif(WIN32)

add_executable(SoundDecompress ${SoundDecompress_SOURCES})
target_link_libraries(SoundDecompress CoreLib plResMgr plAudioCore)
target_link_libraries(SoundDecompress ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(SoundDecompress ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${SoundDecompress_SOURCES})
source_group("Header Files" FILES ${SoundDecompress_HEADERS})

