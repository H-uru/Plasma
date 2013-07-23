add_definitions(-D_LIB)

add_definitions(-DPRODUCT_BRANCH_ID=${PRODUCT_BRANCH_ID})
add_definitions(-DPRODUCT_BUILD_ID=${PRODUCT_BUILD_ID})
add_definitions(-DPRODUCT_BUILD_TYPE=${PRODUCT_BUILD_TYPE})
add_definitions(-DPRODUCT_CORE_NAME="${PRODUCT_CORE_NAME}")
add_definitions(-DPRODUCT_SHORT_NAME="${PRODUCT_SHORT_NAME}")
add_definitions(-DPRODUCT_LONG_NAME="${PRODUCT_LONG_NAME}")
add_definitions(-DPRODUCT_UUID="${PRODUCT_UUID}")

if(NOT WCHAR_BYTES)
    include(CheckTypeSize)

    check_type_size("wchar_t" WCHAR_BYTES)
    if(NOT WCHAR_BYTES)
        message(FATAL_ERROR "Could not determine sizeof(wchar_t)")
        set(WCHAR_BYTES 0)
    endif(NOT WCHAR_BYTES)
endif(NOT WCHAR_BYTES)
add_definitions(-DWCHAR_BYTES=${WCHAR_BYTES})

try_compile(HAVE_CPUID ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_cpuid.cpp
            OUTPUT_VARIABLE OUTPUT)
if(HAVE_CPUID)
    add_definitions(-DHAVE_CPUID)
    message("CPUID header found -- using hardware math acceleration when available")
else()
    message("CPUID header not found -- using software math")
endif()

set(CoreLib_SOURCES
    HeadSpin.cpp
    hsBitVector.cpp
    hsBounds.cpp
    hsCpuID.cpp
    hsCritSect.cpp
    hsExceptionStack.cpp
    hsFastMath.cpp
    hsGeometry3.cpp
    hsMatrix33.cpp
    hsMatrix44.cpp
    hsMemory.cpp
    hsQuat.cpp
    hsRefCnt.cpp
    hsSafeRefCnt.cpp
    hsSTLStream.cpp
    hsStream.cpp
    hsStringTokenizer.cpp
    hsTemplates.cpp
    hsThread.cpp
    hsWide.cpp
    pcSmallRect.cpp
    plFileSystem.cpp
    plGeneric.cpp
    plLoadMask.cpp
    plProduct.cpp
    plString.cpp
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
    hsCritSect.h
    hsExceptions.h
    hsFastMath.h
    hsGeometry3.h
    hsHashTable.h
    hsMatrix44.h
    hsMemory.h
    hsPoint2.h
    hsQuat.h
    hsRefCnt.h
    hsSafeRefCnt.h
    hsSTLStream.h
    hsStream.h
    hsStringTokenizer.h
    hsTemplates.h
    hsThread.h
    hsWide.h
    hsWindows.h
    pcSmallRect.h
    plFileSystem.h
    plGeneric.h
    plLoadMask.h
    plProduct.h
    plQuality.h
    plString.h
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
