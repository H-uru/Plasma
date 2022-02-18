set(plNetLog_HEADERS
    ChunkBuffer.h
    plNetLog.h
    GateKeeper.h
    Auth.h
    Game.h
)

set(plNetLog_SOURCES
    ChunkBuffer.cpp
    plNetLog.cpp
    GateKeeper.cpp
    Auth.cpp
    Game.cpp
)

set(plNetLog_GameMsg_HEADERS
    GameMsg/Factory.h
    GameMsg/Avatar.h
    GameMsg/proEventData.h
    GameMsg/plMessage.h
    GameMsg/plNetMessage.h
)

set(plNetLog_GameMsg_SOURCES
    GameMsg/Factory.cpp
    GameMsg/Avatar.cpp
    GameMsg/proEventData.cpp
    GameMsg/plMessage.cpp
    GameMsg/plNetMessage.cpp
)

plasma_executable(plNetLog TOOL QT_GUI
    SOURCES
        ${plNetLog_HEADERS}
        ${plNetLog_SOURCES}
        ${plNetLog_GameMsg_HEADERS}
        ${plNetLog_GameMsg_SOURCES}
)

target_link_libraries(
    plNetLog
    PRIVATE
        Qt::Widgets
        ZLIB::ZLIB
)

source_group("Moc Files" FILES ${MOC_SOURCES})
source_group("GameMsg\\Header Files" FILES ${GameMsg_HEADERS})
source_group("GameMsg\\Source Files" FILES ${GameMsg_SOURCES})
