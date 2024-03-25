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

plasma_library(pnKeyedObject
    SOURCES ${pnKeyedObject_HEADERS} ${pnKeyedObject_SOURCES}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    pnKeyedObject
    PUBLIC
        CoreLib
        pnFactory
        pnNucleusInc
    PRIVATE
        pnDispatch
        pnMessage
        pnNetCommon
)

source_group("Header Files" FILES ${pnKeyedObject_HEADERS})
source_group("Source Files" FILES ${pnKeyedObject_SOURCES})
