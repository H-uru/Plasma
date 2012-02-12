include_directories(../PubUtilLib/plUnifiedTime)    # EVIL:  For plFileUtils.cpp
add_definitions(-D_LIB)

if(NOT WCHAR_BYTES)
    include(CheckTypeSize)

    check_type_size("wchar_t" WCHAR_BYTES)
    if(NOT WCHAR_BYTES)
        message(FATAL_ERROR "Could not determine sizeof(wchar_t)")
        set(WCHAR_BYTES 0)
    endif(NOT WCHAR_BYTES)
endif(NOT WCHAR_BYTES)
add_definitions(-DWCHAR_BYTES=${WCHAR_BYTES})

set(CoreLib_SOURCES
    HeadSpin.cpp
    hsBitVector.cpp
    hsBounds.cpp
    hsCpuID.cpp
    hsCritSect.cpp
    hsExceptionStack.cpp
    hsFastMath.cpp
    hsFiles.cpp
    hsGeometry3.cpp
    hsMatrix33.cpp
    hsMatrix44.cpp
    hsMemory.cpp
    hsQuat.cpp
    hsRefCnt.cpp
    hsSafeRefCnt.cpp
    hsSTLStream.cpp
    hsStlUtils.cpp
    hsStream.cpp
    hsStringTokenizer.cpp
    hsTemplates.cpp
    hsThread.cpp
    hsWide.cpp
    pcSmallRect.cpp
    plFileUtils.cpp
    plGeneric.cpp
    plLoadMask.cpp
    plString.cpp
    plViewTransform.cpp
)

if(WIN32 AND NOT CYGWIN)
    set(CoreLib_SOURCES ${CoreLib_SOURCES}
        hsThread_Win.cpp
        hsFiles_Win.cpp
    )
endif(WIN32 AND NOT CYGWIN)
if(UNIX)
    set(CoreLib_SOURCES ${CoreLib_SOURCES}
        hsThread_Unix.cpp
        hsFiles_Unix.cpp
    )
endif(UNIX)

set(CoreLib_HEADERS
    HeadSpin.h
    hsBiExpander.h
    hsBitVector.h
    hsBounds.h
    hsColorRGBA.h
    hsCpuID.h
    hsCritSect.h
    hsExceptions.h
    hsFastMath.h
    hsFiles.h
    hsGeometry3.h
    hsHashTable.h
    hsMatrix44.h
    hsMemory.h
    hsPoint2.h
    hsQuat.h
    hsRefCnt.h
    hsSafeRefCnt.h
    hsStlSortUtils.h
    hsSTLStream.h
    hsStlUtils.h
    hsStream.h
    hsStringTokenizer.h
    hsTemplates.h
    hsThread.h
    hsWide.h
    pcSmallRect.h
    plFileUtils.h
    plGeneric.h
    plLoadMask.h
    plQuality.h
    plString.h
    plTweak.h
    plViewTransform.h
)

add_library(CoreLib STATIC ${CoreLib_SOURCES} ${CoreLib_HEADERS})
if(UNIX)
    target_link_libraries(CoreLib pthread)
endif(UNIX)

source_group("Source Files" FILES ${CoreLib_SOURCES})
source_group("Header Files" FILES ${CoreLib_HEADERS})
