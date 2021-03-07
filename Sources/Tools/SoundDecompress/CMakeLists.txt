cmake_dependent_option(SoundDecompress_MOULa_Mode "Output wav files for MOULa" OFF [[WIN32]] OFF)
set(OUTPUT_WAV_FILES ${SoundDecompress_MOULa_Mode})
configure_file(config.h.in config.h @ONLY)

set(SoundDecompress_SOURCES
    plAllCreatables.cpp
    main.cpp
)

set(SoundDecompress_HEADERS
)

if(SoundDecompress_MOULa_Mode)
    list(APPEND SoundDecompress_SOURCES plOldAudioFileReader.cpp)
    list(APPEND SoundDecompress_HEADERS plOldAudioFileReader.h)
endif()

plasma_executable(SoundDecompress TOOL SOURCES ${SoundDecompress_SOURCES})
target_link_libraries(
    SoundDecompress
    PRIVATE
        CoreLib
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnModifier
        pnNetCommon
        pnNucleusInc
        plAgeDescription
        plAudioCore
        plMessage
        plResMgr
)

target_include_directories(SoundDecompress PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

source_group("Source Files" FILES ${SoundDecompress_SOURCES})
source_group("Header Files" FILES ${SoundDecompress_HEADERS})

