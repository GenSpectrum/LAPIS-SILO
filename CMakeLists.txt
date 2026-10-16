cmake_minimum_required(VERSION 3.22)
project(RHYDB)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# CMAKE_CXX_FLAGS* are left to the toolchain and the caller (see CMakePresets.json)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

# The defaults describe the native build; the presets set them for each variant.
option(BUILD_APP "Build the rhydb command-line application" ON)
option(BUILD_BENCHMARKS "Build the performance benchmarks" ON)
option(BUILD_PYTHON_BINDINGS "Build the Python bindings" OFF)
option(BUILD_WASM_MODULE "Build the WebAssembly module (requires the Emscripten toolchain)" OFF)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)
option(ENABLE_EVOBENCH "Compile in the evobench probes" ON)
option(ENABLE_LZMA "Support lzma (.xz) compressed inputs" ON)

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

# ---------------------------------------------------------------------------
# Consumers
# ---------------------------------------------------------------------------

# Each is a consumer of the shared rhydblib declaration
if(BUILD_APP)
    add_subdirectory(app)
endif()
if(BUILD_BENCHMARKS)
    add_subdirectory(performance)
endif()
if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
if(BUILD_WASM_MODULE)
    add_subdirectory(wasm)
endif()
