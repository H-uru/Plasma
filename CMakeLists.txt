project(plNetLog)
cmake_minimum_required(VERSION 2.6)

if(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS")
else()
    message(FATAL_ERROR "This project only works on Visual Studio for now")
endif()

find_package(Qt4 COMPONENTS QtMain QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    ChunkBuffer.cpp
    plNetLog.cpp
    GateKeeper.cpp
    Auth.cpp
    Game.cpp
)

set(HEADERS
    ChunkBuffer.h
    plNetLog.h
    GateKeeper.h
    Auth.h
    Game.h
)

set(MOCSRC
    plNetLog.h
)

set(GameMsg_SOURCES
    GameMsg/Factory.cpp
    GameMsg/Factory.h
)

qt4_wrap_cpp(MOC_SOURCES ${MOCSRC})
add_executable(plNetLog WIN32 ${HEADERS} ${SOURCES} ${MOC_SOURCES}
               ${GameMsg_SOURCES}
)
target_link_libraries(plNetLog ${QT_LIBRARIES})

source_group("Moc Files" FILES ${MOC_SOURCES})
source_group("GameMsg" FILES ${GameMsg_SOURCES})
