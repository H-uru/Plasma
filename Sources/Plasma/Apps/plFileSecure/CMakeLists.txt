include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")

set(plFileSecure_SOURCES
    main.cpp
)

add_executable(plFileSecure ${plFileSecure_SOURCES})
target_link_libraries(plFileSecure CoreLib plFile)
target_link_libraries(plFileSecure ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(plFileSecure ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plFileSecure_SOURCES})
