add_definitions(-D_LIB)

set(CoreLib_SOURCES
    HeadSpin.cpp
    hsBitVector.cpp
    hsBounds.cpp
    hsCritSect.cpp
    hsExceptionStack.cpp
    hsFastMath.cpp
    hsGeometry3.cpp
    hsMalloc.cpp
    hsMatrix33.cpp
    hsMatrix44.cpp
    hsMemory.cpp
    hsMMIOStream.cpp
    hsQuat.cpp
    hsSafeRefCnt.cpp
    hsSTLStream.cpp
    hsStlUtils.cpp
    hsStream.cpp
    hsStringTokenizer.cpp
    hsTemplates.cpp
    hsThread.cpp
    hsUtils.cpp
    hsWide.cpp
    pcSmallRect.cpp
    plGeneric.cpp
    plLoadMask.cpp
    plViewTransform.cpp
)

if(WIN32 AND NOT CYGWIN)
    set(CoreLib_SOURCES ${CoreLib_SOURCES}
        hsThread_Win.cpp
    )
endif(WIN32 AND NOT CYGWIN)
if(UNIX)
    set(CoreLib_SOURCES ${CoreLib_SOURCES}
        hsThread_Unix.cpp
    )
endif(UNIX)
    

set(CoreLib_HEADERS
    HeadSpin.h
    hsBiExpander.h
    hsBitVector.h
    hsBounds.h
    hsColorRGBA.h
    hsConfig.h
    hsCritSect.h
    hsExceptions.h
    hsFastMath.h
    hsGeometry3.h
    hsHashTable.h
    hsMalloc.h
    hsMatrix44.h
    hsMemory.h
    hsMMIOStream.h
    hsPoint2.h
    hsQuat.h
    hsQueue.h
    hsRefCnt.h
    hsSafeRefCnt.h
    hsStlSortUtils.h
    hsSTLStream.h
    hsStlUtils.h
    hsStream.h
    hsStringTokenizer.h
    hsTemplates.h
    hsTempPointer.h
    hsThread.h
    hsTypes.h
    hsUtils.h
    hsWide.h
    hsWindowHndl.h
    hsWindows.h
    pcSmallRect.h
    plGeneric.h
    plLoadMask.h
    plQuality.h
    plRefCnt.h
    plTweak.h
    plViewTransform.h
)

add_library(CoreLib STATIC ${CoreLib_SOURCES} ${CoreLib_HEADERS})
if(UNIX)
    target_link_libraries(CoreLib pthread)
endif(UNIX)

source_group("Source Files" FILES ${CoreLib_SOURCES})
source_group("Header Files" FILES ${CoreLib_HEADERS})
