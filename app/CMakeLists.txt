# The native RhyDB command-line application: the `rhydb` executable and the
# Poco-based HTTP API layer it serves. This is a consumer of the portable rhydblib
# and built by the root CMakeLists before this subproject is added
#
# app/src is added as an include root so the app's own `#include "..."` headers
# resolve here while rhydblib headers still resolve against rhydblib's src/ include
# root. The app layer lives in the flat `rhydb_app` namespace, so its sources sit
# directly in app/src rather than in a namespace-mirroring subdirectory.

# ---------------------------------------------------------------------------
# Native-only dependencies (only this subproject needs them)
# ---------------------------------------------------------------------------

find_package(Poco REQUIRED COMPONENTS Net Util JSON)

# Only use mimalloc in Release mode, because of conflicts with ASAN, and only on
# Linux, because Mac has some specialties that occur at program startup before
# mimalloc is loaded, which can lead to mixing of allocations and thus
# Segmentation Faults.
option(USE_MIMALLOC "Use mimalloc allocator" ON)
if(USE_MIMALLOC AND CMAKE_BUILD_TYPE STREQUAL "Release" AND LINUX)
    message(STATUS "Using mimalloc as allocator")
    find_package(mimalloc REQUIRED)
    set(MIMALLOC_LIB mimalloc-static)
else()
    set(MIMALLOC_LIB "")
endif()

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

file(GLOB_RECURSE RHYDB_APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE RHYDB_APP_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.test.cpp")

set(RHYDB_APP_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

list(REMOVE_ITEM RHYDB_APP_SOURCES ${RHYDB_APP_TESTS} ${RHYDB_APP_MAIN})

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

# The HTTP API layer on top of the portable core. This is the only place Poco is
# linked, so it stays a leaf dependency of the HTTP API code.
add_library(rhydb_app OBJECT ${RHYDB_APP_SOURCES})
set_target_properties(rhydb_app PROPERTIES POSITION_INDEPENDENT_CODE ON)
# PUBLIC so consumers (the rhydb executable, rhydb_app_test) also see the app/src
# include root and can resolve the app layer's `#include "..."` headers.
target_include_directories(rhydb_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(
        rhydb_app
        PUBLIC
        rhydblib
        Poco::Net
        Poco::Util
        Poco::JSON
)
# Object files do not propagate transitively through a chain of OBJECT
# libraries, so linking rhydb_app alone would not pull in rhydblib's objects.
# Carry the core's objects along as an interface source so a consumer that links
# rhydb_app gets the whole native engine (core + layer) from one edge.
target_sources(rhydb_app INTERFACE $<TARGET_OBJECTS:rhydblib>)

add_executable(rhydb ${RHYDB_APP_MAIN})
# Keep the binary at build/<cfg>/rhydb (not build/<cfg>/cli/rhydb) so tooling,
# Docker, and the Makefile find it at the top-level build directory.
set_target_properties(rhydb PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_link_libraries(
        rhydb
        PUBLIC
        ${MIMALLOC_LIB} # this should be first
        rhydb_app
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
# The subproject owns its unit tests (the API-layer tests), just as the core
# owns rhydb_test and wasm/ owns its node test.

find_package(GTest REQUIRED)
if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()

# GTest is built without its own main (gtest/*:no_main), so both test binaries
# share the harness entry point in src/main.test.cpp (repo-root cwd + arrow
# init + RUN_ALL_TESTS). Each binary compiles it to run only its own tests.
add_executable(rhydb_app_test ${RHYDB_APP_TESTS} "${CMAKE_SOURCE_DIR}/src/main.test.cpp")
set_target_properties(rhydb_app_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_include_directories(rhydb_app_test PRIVATE ${GTest_INCLUDE_DIRS})
target_link_libraries(rhydb_app_test PUBLIC ${GTest_LIBRARIES} rhydb_app)
