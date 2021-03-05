cmake_dependent_option(
    PLASMA_USE_PCH
    "Enable precompiled headers?"
    ON
    [[${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"]]
    OFF
)

cmake_dependent_option(
    PLASMA_UNITY_BUILD
    "Enable unity build?"
    ON
    [[${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"]]
    OFF
)
