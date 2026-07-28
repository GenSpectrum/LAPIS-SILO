# The native SILO command-line application: the `silo` executable and the
# Poco-based HTTP API layer it serves. This is a consumer of the portable silolib
# and built by the root CMakeLists before this subproject is added
#
# app/src is added as an include root so the app's own `#include "..."` headers
# resolve here while silolib headers still resolve against silolib's src/ include
# root. The app layer lives in the flat `silo_app` namespace, so its sources sit
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

file(GLOB_RECURSE SILO_APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE SILO_APP_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.test.cpp")

set(SILO_APP_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

list(REMOVE_ITEM SILO_APP_SOURCES ${SILO_APP_TESTS} ${SILO_APP_MAIN})

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

# The HTTP API layer on top of the portable core. This is the only place Poco is
# linked, so it stays a leaf dependency of the HTTP API code.
add_library(silo_app OBJECT ${SILO_APP_SOURCES})
set_target_properties(silo_app PROPERTIES POSITION_INDEPENDENT_CODE ON)
# PUBLIC so consumers (the silo executable, silo_app_test) also see the app/src
# include root and can resolve the app layer's `#include "..."` headers.
target_include_directories(silo_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(
        silo_app
        PUBLIC
        silolib
        Poco::Net
        Poco::Util
        Poco::JSON
)
# Object files do not propagate transitively through a chain of OBJECT
# libraries, so linking silo_app alone would not pull in silolib's objects.
# Carry the core's objects along as an interface source so a consumer that links
# silo_app gets the whole native engine (core + layer) from one edge.
target_sources(silo_app INTERFACE $<TARGET_OBJECTS:silolib>)

add_executable(silo ${SILO_APP_MAIN})
# Keep the binary at build/<cfg>/silo (not build/<cfg>/cli/silo) so tooling,
# Docker, and the Makefile find it at the historical path.
set_target_properties(silo PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_link_libraries(
        silo
        PUBLIC
        ${MIMALLOC_LIB} # this should be first
        silo_app
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
# The subproject owns its unit tests (the API-layer tests), just as the core
# owns silo_test and wasm/ owns its node test.

find_package(GTest REQUIRED)
if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()

# GTest is built without its own main (gtest/*:no_main), so both test binaries
# share the harness entry point in src/main.test.cpp (repo-root cwd + arrow
# init + RUN_ALL_TESTS). Each binary compiles it to run only its own tests.
add_executable(silo_app_test ${SILO_APP_TESTS} "${CMAKE_SOURCE_DIR}/src/main.test.cpp")
set_target_properties(silo_app_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_include_directories(silo_app_test PRIVATE ${GTest_INCLUDE_DIRS})
target_link_libraries(silo_app_test PUBLIC ${GTest_LIBRARIES} silo_app)
