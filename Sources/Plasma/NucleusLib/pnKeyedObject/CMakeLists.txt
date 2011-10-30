include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")
include_directories("../../PubUtilLib")

set(pnKeyedObject_HEADERS
    hsKeyedObject.h
    plFixedKey.h
    plKey.h
    plKeyImp.h
    plMsgForwarder.h
    plReceiver.h
    plUoid.h
    pnKeyedObjectCreatable.h
)

set(pnKeyedObject_SOURCES
    hsKeyedObject.cpp
    plFixedKey.cpp
    plKey.cpp
    plKeyImp.cpp
    plMsgForwarder.cpp
    plUoid.cpp
)

add_library(pnKeyedObject STATIC ${pnKeyedObject_HEADERS} ${pnKeyedObject_SOURCES})
target_link_libraries(pnKeyedObject CoreLib pnMessage)

source_group("Header Files" FILES ${pnKeyedObject_HEADERS})
source_group("Source Files" FILES ${pnKeyedObject_SOURCES})
