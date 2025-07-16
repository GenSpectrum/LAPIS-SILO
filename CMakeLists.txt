cmake_minimum_required(VERSION 3.22)
project(SILO)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -D SILO_DEBUG_ASSERTIONS=1")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D SILO_DEBUG_ASSERTIONS=0")

# Work-around only for MacOS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -D SILO_DEBUG_ASSERTIONS=1")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D SILO_DEBUG_ASSERTIONS=0")
endif ()

set(CMAKE_CXX_STANDARD 23)

# For find_package calls in Module mode (looking for the Find<lib>.cmake files)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/generators/")
# For find_package calls in config mode / find_dependency calls
# (looking for the <lib>-config.cmake files of transitive dependencies of modules, e.g. spdlog->fmt)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/generators/")

# ---------------------------------------------------------------------------
# Compile definitions
# ---------------------------------------------------------------------------

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
add_compile_definitions(SIMDJSON_EXCEPTIONS=0)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Arrow REQUIRED COMPONENTS Acero XYZ)
find_package(Boost REQUIRED COMPONENTS system serialization iostreams)
find_package(LibLZMA REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Poco REQUIRED COMPONENTS Net Util JSON)
find_package(re2 REQUIRED)
find_package(roaring REQUIRED)
find_package(spdlog REQUIRED)
find_package(simdjson REQUIRED)
find_package(TBB REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(zstd REQUIRED)

# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------

execute_process(
        COMMAND cat release_version.txt
        OUTPUT_VARIABLE VERSION
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE VERSION_AVAILABLE
)
if (VERSION_AVAILABLE EQUAL 0)
    message(STATUS "Detected version from release_version.txt: ${VERSION}")
    add_compile_definitions(SILO_RELEASE_VERSION="${VERSION}")
else ()
    message(STATUS "No release_version.txt available for setting the version")
endif()

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include_directories(
        ${CMAKE_SOURCE_DIR}/src
)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

file(GLOB_RECURSE SRC_TEST "src/*.test.cpp")

file(GLOB_RECURSE SRC_SILO "src/*.cpp")
list(REMOVE_ITEM SRC_SILO ${SRC_TEST})

set(SRC_SILO_WITHOUT_MAIN ${SRC_SILO})
list(REMOVE_ITEM SRC_SILO_WITHOUT_MAIN "${CMAKE_SOURCE_DIR}/src/main.cpp")

# ---------------------------------------------------------------------------
# Linter
# ---------------------------------------------------------------------------

option(BUILD_WITH_CLANG_TIDY "Build process clang-tidy")
if (NOT CMAKE_BUILD_TYPE STREQUAL Release AND BUILD_WITH_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-20)
    if (NOT CLANG_TIDY_EXE)
        message(SEND_ERROR "clang-tidy not found, aborting. You can run the build with '-D BUILD_WITH_CLANG_TIDY=OFF' to disable clang-tidy.")
    else ()
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        message(STATUS "run clang tidy with: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif ()
endif ()

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

add_library(silolib OBJECT ${SRC_SILO_WITHOUT_MAIN})
target_link_libraries(
        silolib
        PUBLIC
        Arrow::arrow_static
        Acero::arrow_acero_static
        ${Boost_LIBRARIES}
        ${duckdb_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${roaring_LIBRARIES}
        ${spdlog_LIBRARIES}
        ${simdjson_LIBRARIES}
        TBB::tbb
        ${yaml-cpp_LIBRARIES}
        zstd::libzstd_static
        Poco::Net
        Poco::Util
        Poco::JSON
        re2::re2
)

add_executable(silo "${CMAKE_SOURCE_DIR}/src/main.cpp" $<TARGET_OBJECTS:silolib>)
target_link_libraries(
        silo
        PUBLIC
        silolib
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_executable(silo_test ${SRC_TEST} $<TARGET_OBJECTS:silolib>)
if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()
target_link_libraries(silo_test PUBLIC ${GTest_LIBRARIES} silolib)
