cmake_minimum_required(VERSION 3.22)
project(SILO)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -D SILO_DEBUG_ASSERTIONS=1")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D SILO_DEBUG_ASSERTIONS=0")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For find_package calls in Module mode (looking for the Find<lib>.cmake files)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/generators/")
# For find_package calls in config mode / find_dependency calls
# (looking for the <lib>-config.cmake files of transitive dependencies of modules, e.g. spdlog->fmt)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/generators/")

# Under the Emscripten toolchain, find_package defaults to searching only inside
# the cross-compilation sysroot (CMAKE_FIND_ROOT_PATH), so the Conan-generated
# package config files in build/wasm/generators are ignored. Add that directory
# as a find root and allow package lookups to fall back to CMAKE_PREFIX_PATH.
if(EMSCRIPTEN)
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_BINARY_DIR}/generators")
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()

# Export compile commands for configuring language servers or debugging the build
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# silolib dependencies
# ---------------------------------------------------------------------------

find_package(Arrow REQUIRED COMPONENTS Acero)
find_package(Boost REQUIRED COMPONENTS system serialization iostreams)
find_package(FastFloat REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(re2 REQUIRED)
find_package(roaring REQUIRED)
find_package(spdlog REQUIRED)
find_package(simdjson REQUIRED)
find_package(simdutf REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(zstd REQUIRED)

# Native-only: lzma-compressed inputs. Not available for Emscripten (the browser
# build accepts raw and .zst inputs only).
if(NOT EMSCRIPTEN)
    find_package(LibLZMA REQUIRED)
endif()

# ---------------------------------------------------------------------------
# silolib
# ---------------------------------------------------------------------------

add_library(silolib_deps INTERFACE)

target_link_libraries(silolib_deps INTERFACE
        Arrow::arrow_static
        ArrowAcero::arrow_acero_static
        ${Boost_LIBRARIES}
        ${FastFloat_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${roaring_LIBRARIES}
        ${spdlog_LIBRARIES}
        ${simdjson_LIBRARIES}
        ${simdutf_LIBRARIES}
        ${yaml-cpp_LIBRARIES}
        zstd::libzstd_static
        re2::re2
)

target_include_directories(silolib_deps INTERFACE "src")

target_compile_definitions(silolib_deps INTERFACE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
        SIMDJSON_EXCEPTIONS=0
)

# WebAssembly-only usage requirements. These are INTERFACE so they compile into
# the silolib objects too: the SILO_WASM guards live in silolib sources
# (input_stream_wrapper.cpp, exec_node/table_scan.h), NO_EVOBENCH neutralises the
# evobench header pulled in by silolib, and -fexceptions must reach every silolib
# object (they throw). -pthread keeps SILO objects ABI-consistent with the
# Emscripten dependency builds.
if(EMSCRIPTEN)
    target_compile_definitions(silolib_deps INTERFACE SILO_WASM NO_EVOBENCH)
    target_compile_options(silolib_deps INTERFACE -fexceptions -pthread)
endif()

file(GLOB_RECURSE SILOLIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SILO_TEST_SOURCES CONFIGURE_DEPENDS "src/*.test.cpp")
list(REMOVE_ITEM SILOLIB_SOURCES ${SILO_TEST_SOURCES})

# The browser target does not include append mode, runtime config, or evobench.
if(EMSCRIPTEN)
    file(GLOB_RECURSE WASM_EXCLUDED_SOURCES CONFIGURE_DEPENDS
            "src/silo/append/append.cpp"
            "src/silo/config/append_config.cpp"
            "src/silo/config/runtime_config.cpp"
            "src/evobench/evobench.cpp"
    )
    list(REMOVE_ITEM SILOLIB_SOURCES ${WASM_EXCLUDED_SOURCES})
endif()

add_library(silolib OBJECT ${SILOLIB_SOURCES})
set_target_properties(silolib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(silolib PUBLIC silolib_deps)

# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------

if (EXISTS "${CMAKE_SOURCE_DIR}/release_version.txt")
    file(READ "${CMAKE_SOURCE_DIR}/release_version.txt" VERSION)
    string(STRIP "${VERSION}" VERSION)
    message(STATUS "Detected version from release_version.txt: ${VERSION}")
    add_compile_definitions(SILO_RELEASE_VERSION="${VERSION}")
else ()
    message(STATUS "No release_version.txt available for setting the version")
endif()

# ---------------------------------------------------------------------------
# Linter
# ---------------------------------------------------------------------------

option(BUILD_WITH_CLANG_TIDY "Build process clang-tidy")
if (NOT CMAKE_BUILD_TYPE STREQUAL Release AND BUILD_WITH_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-19 clang-tidy-20)
    if (NOT CLANG_TIDY_EXE)
        message(SEND_ERROR "clang-tidy not found, aborting. You can run the build with '-D BUILD_WITH_CLANG_TIDY=OFF' to disable clang-tidy.")
    else ()
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        message(STATUS "run clang tidy with: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif ()
endif ()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

# GTest is not part of the Emscripten dependency set; the browser build ships
# its own node smoke test under wasm/test instead.
if(NOT EMSCRIPTEN)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTest_INCLUDE_DIRS})

    add_executable(silo_test ${SILO_TEST_SOURCES})
    if (NOT GTest_LIBRARIES)
        set(GTest_LIBRARIES gtest gmock)
    endif ()
    target_link_libraries(silo_test PUBLIC ${GTest_LIBRARIES} silolib)
endif()

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

if(EMSCRIPTEN)
    # The browser embind module, a consumer of the shared silolib declaration.
    add_subdirectory(wasm)
else()
    # The native command-line application (the `silo` executable) is a consumer
    # of the shared silolib declaration
    add_subdirectory(app)

    # The performance benchmark targets
    add_subdirectory(performance)

    # The python binding target
    option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
    if(BUILD_PYTHON_BINDINGS)
        add_subdirectory(python)
    endif()
endif()
