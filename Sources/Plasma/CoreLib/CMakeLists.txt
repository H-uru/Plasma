add_definitions(-D_LIB)

add_definitions(-DPRODUCT_BRANCH_ID=${PRODUCT_BRANCH_ID})
add_definitions(-DPRODUCT_BUILD_ID=${PRODUCT_BUILD_ID})
add_definitions(-DPRODUCT_BUILD_TYPE=${PRODUCT_BUILD_TYPE})
add_definitions(-DPRODUCT_CORE_NAME="${PRODUCT_CORE_NAME}")
add_definitions(-DPRODUCT_SHORT_NAME="${PRODUCT_SHORT_NAME}")
add_definitions(-DPRODUCT_LONG_NAME="${PRODUCT_LONG_NAME}")
add_definitions(-DPRODUCT_UUID="${PRODUCT_UUID}")

set(CoreLib_SOURCES
    HeadSpin.cpp
    hsBitVector.cpp
    hsBounds.cpp
    hsCpuID.cpp
    hsExceptions.cpp
    hsExceptionStack.cpp
    hsFastMath.cpp
    hsGeometry3.cpp
    hsMatrix33.cpp
    hsMatrix44.cpp
    hsMemory.cpp
    hsQuat.cpp
    hsRefCnt.cpp
    hsSTLStream.cpp
    hsStream.cpp
    hsStringTokenizer.cpp
    hsTemplates.cpp
    hsThread.cpp
    hsWide.cpp
    pcSmallRect.cpp
    plCmdParser.cpp
    plFileSystem.cpp
    plGeneric.cpp
    plLoadMask.cpp
    plProduct.cpp
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
    hsAlignedAllocator.hpp
    hsBiExpander.h
    hsBitVector.h
    hsBounds.h
    hsColorRGBA.h
    hsCpuID.h
    hsExceptions.h
    hsExceptionStack.h
    hsFastMath.h
    hsGeometry3.h
    hsHashTable.h
    hsLockGuard.h
    hsMatrix44.h
    hsMemory.h
    hsPoint2.h
    hsQuat.h
    hsRefCnt.h
    hsSTLStream.h
    hsStream.h
    hsStringTokenizer.h
    hsTemplates.h
    hsThread.h
    hsWide.h
    hsWindows.h
    pcSmallRect.h
    plCmdParser.h
    plFileSystem.h
    plGeneric.h
    plLoadMask.h
    plProduct.h
    plQuality.h
    plRenderLevel.h
    plTweak.h
    plViewTransform.h
)

use_precompiled_header(_CoreLibPch.h Pch.cpp CoreLib_HEADERS CoreLib_SOURCES)
add_library(CoreLib STATIC ${CoreLib_SOURCES} ${CoreLib_HEADERS})

if(UNIX)
    target_link_libraries(CoreLib pthread)
endif(UNIX)

source_group("Source Files" FILES ${CoreLib_SOURCES})
source_group("Header Files" FILES ${CoreLib_HEADERS})
