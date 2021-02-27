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
    hsWindows.cpp
)

if(CMAKE_USE_PTHREADS_INIT)
    list(APPEND CoreLib_SOURCES hsThread_Unix.cpp)
elseif(CMAKE_USE_WIN32_THREADS_INIT)
    list(APPEND CoreLib_SOURCES hsThread_Win.cpp)
else()
    message(FATAL_ERROR "The thread library CMake provided for your platform is not supported.")
endif()

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

add_library(CoreLib STATIC ${CoreLib_SOURCES} ${CoreLib_HEADERS})
target_link_libraries(
    CoreLib
    PUBLIC
        string_theory
        Threads::Threads
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${USE_VLD}>>:VLD::VLD>
)
target_include_directories(CoreLib PUBLIC ${CMAKE_CURRENT_LIST_DIR})

plasma_optimize_target(CoreLib PRECOMPILED_HEADERS _CoreLibPch.h)

source_group("Source Files" FILES ${CoreLib_SOURCES})
source_group("Header Files" FILES ${CoreLib_HEADERS})
