cmake_minimum_required(VERSION 3.22)
project(RHYDB)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Append to keep the toolchain's flags
string(APPEND CMAKE_CXX_FLAGS " -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -D RHYDB_DEBUG_ASSERTIONS=1")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D RHYDB_DEBUG_ASSERTIONS=0")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For find_package calls in Module mode (looking for the Find<lib>.cmake files)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/generators/")
# For find_package calls in config mode / find_dependency calls
# (looking for the <lib>-config.cmake files of transitive dependencies of modules, e.g. spdlog->fmt)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/generators/")

# Export compile commands for configuring language servers or debugging the build
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------

if (EXISTS "${CMAKE_SOURCE_DIR}/release_version.txt")
    file(READ "${CMAKE_SOURCE_DIR}/release_version.txt" VERSION)
    string(STRIP "${VERSION}" VERSION)
    message(STATUS "Detected version from release_version.txt: ${VERSION}")
    add_compile_definitions(RHYDB_RELEASE_VERSION="${VERSION}")
else ()
    message(STATUS "No release_version.txt available for setting the version")
endif()

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------

# rhydblib and its dependencies, plus the rhydb_test unit tests. The consumers
# below link against rhydblib.
add_subdirectory(src)

if(EMSCRIPTEN)
    # The browser build does not profile. -pthread/-fexceptions come from the toolchain.
    target_compile_definitions(rhydblib PUBLIC NO_EVOBENCH)
endif()

# ---------------------------------------------------------------------------
# Consumers
# ---------------------------------------------------------------------------

if(EMSCRIPTEN)
    # The browser embind module, a consumer of the shared rhydblib declaration.
    add_subdirectory(wasm)
else()
    # The native command-line application (the `rhydb` executable) is a consumer
    # of the shared rhydblib declaration
    add_subdirectory(app)

    # The performance benchmark targets
    add_subdirectory(performance)

    # The python binding target
    option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
    if(BUILD_PYTHON_BINDINGS)
        add_subdirectory(python)
    endif()
endif()
