option(Bink_SDK_AVAILABLE "Do we have the Bink SDK?" OFF)

if(Bink_SDK_AVAILABLE)
    add_definitions(-DBINK_SDK_AVAILABLE)
endif(Bink_SDK_AVAILABLE)

find_path(Bink_INCLUDE_DIR bink.h
    C:/BinkW32
)

find_library(Bink_LIBRARY NAMES binkw32
    PATHS C:/BinkW32
)

set(Bink_LIBRARIES ${Bink_LIBRARY})