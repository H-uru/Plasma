include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
