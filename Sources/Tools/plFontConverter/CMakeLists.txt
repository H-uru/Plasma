include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories(${FREETYPE_INCLUDE_DIRS})

set(plFontConverter_HEADERS
    plFontFreeType.h
)

set(plFontConverter_SOURCES
    plFontConverter.cpp
    plFontConverterProc.cpp
    plFontFreeType.cpp
)
    
set(plFontConverter_RESOURCES
    res/plFontConverter.rc
    res/resource.h
    res/icon1.ico
)
    
add_executable(plFontConverter WIN32 ${plFontConverter_SOURCES} ${plFontConverter_HEADERS} ${plFontConverter_RESOURCES})

target_link_libraries(plFontConverter CoreLib)
target_link_libraries(plFontConverter pnKeyedObject)
target_link_libraries(plFontConverter pnSceneObject)
target_link_libraries(plFontConverter pnProduct)
target_link_libraries(plFontConverter plGImage)
target_link_libraries(plFontConverter plJPEG)
target_link_libraries(plFontConverter plMessage)
target_link_libraries(plFontConverter plPipeline)
target_link_libraries(plFontConverter plResMgr)
target_link_libraries(plFontConverter ${JPEG_LIBRARY})
target_link_libraries(plFontConverter ${DirectX_LIBRARIES})
target_link_libraries(plFontConverter ${FREETYPE_LIBRARIES})

if (WIN32)
    target_link_libraries(plFontConverter Rpcrt4)
    target_link_libraries(plFontConverter Vfw32)
    target_link_libraries(plFontConverter Ws2_32)
    target_link_libraries(plFontConverter winmm)
    target_link_libraries(plFontConverter strmiids)
    target_link_libraries(plFontConverter comctl32)
    target_link_libraries(plFontConverter shlwapi)
endif(WIN32)

if(USE_VLD)
    target_link_libraries(plFontConverter ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plFontConverter_SOURCES})
source_group("Header Files" FILES ${plFontConverter_HEADERS})
source_group("Resource Files" FILES ${plFontConverter_RESOURCES})
