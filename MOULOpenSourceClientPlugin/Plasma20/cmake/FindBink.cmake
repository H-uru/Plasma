find_path(Bink_INCLUDE_DIR bink.h
    C:/BinkW32
)

find_library(Bink_LIBRARY NAMES binkw32
    PATHS C:/BinkW32
)

if(Bink_INCLUDE_DIR AND Bink_LIBRARY)
    set(Bink_SDK_AVAILABLE TRUE)
    add_definitions(-DBINK_SDK_AVAILABLE)
else()
    set(Bink_SDK_AVAILABLE FALSE)
endif()

set(Bink_LIBRARIES ${Bink_LIBRARY})
