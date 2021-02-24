find_path(ASIO_INCLUDE_DIR
    NAMES asio/version.hpp
)

mark_as_advanced(ASIO_INCLUDE_DIR)

if(ASIO_INCLUDE_DIR AND EXISTS "${ASIO_INCLUDE_DIR}/asio/version.hpp")
    file(STRINGS "${ASIO_INCLUDE_DIR}/asio/version.hpp" _asio_version_line
         REGEX "^#[ \t]*define[ \t]+ASIO_VERSION[ \t]+([0-9]+).*")
    string(REGEX REPLACE "^.*ASIO_VERSION[ \t]+([0-9]+).*$" "\\1"
           _asio_version "${_asio_version_line}")
    math(EXPR _asio_major "${_asio_version} / 100000")
    math(EXPR _asio_minor "(${_asio_version} / 100) % 1000")
    math(EXPR _asio_patch "${_asio_version} % 100")
    set(ASIO_VERSION "${_asio_major}.${_asio_minor}.${_asio_patch}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASIO
    REQUIRED_VARS ASIO_INCLUDE_DIR
    VERSION_VAR ASIO_VERSION
)

if(ASIO_FOUND AND NOT TARGET ASIO::ASIO)
    find_package(Threads REQUIRED)

    add_library(ASIO::ASIO INTERFACE IMPORTED)
    set_target_properties(ASIO::ASIO PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS ASIO_STANDALONE
        INTERFACE_LINK_LIBRARIES Threads::Threads
    )
endif()
