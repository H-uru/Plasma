include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")

set(plPageInfo_SOURCES
    plAllCreatables.cpp
    plPageInfo.cpp
)

add_executable(plPageInfo ${plPageInfo_SOURCES})
target_link_libraries(plPageInfo CoreLib pnProduct plResMgr plAudioCore)

if(USE_VLD)
    target_link_libraries(plPageInfo ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plPageInfo_SOURCES})
