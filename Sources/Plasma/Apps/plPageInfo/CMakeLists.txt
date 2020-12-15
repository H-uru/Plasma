set(plPageInfo_SOURCES
    plAllCreatables.cpp
    plPageInfo.cpp
)

add_executable(plPageInfo ${plPageInfo_SOURCES})
target_link_libraries(plPageInfo CoreLib plResMgr plAudioCore pnUUID)
target_link_libraries(plPageInfo ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(plPageInfo ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plPageInfo_SOURCES})
