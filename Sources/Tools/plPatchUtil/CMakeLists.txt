include_directories("../../Plasma/Apps")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories("../../Plasma/PubUtilLib")

set(plPatchUtil_SOURCES
    plAllCreatables.cpp
    main.cpp
)

add_executable(plPatchUtil ${plPatchUtil_SOURCES})

target_link_libraries(plPatchUtil CoreLib)
target_link_libraries(plPatchUtil pnEncryption)
target_link_libraries(plPatchUtil plResMgr)
target_link_libraries(plPatchUtil pnUUID)

if(USE_VLD)
    target_link_libraries(plPatchUtil ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plPatchUtil_SOURCES})
