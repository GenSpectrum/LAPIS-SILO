cmake_minimum_required(VERSION 3.22)
project(SILO)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -static-libstdc++ -static-libgcc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/")

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Boost REQUIRED COMPONENTS system serialization iostreams)
find_package(duckdb REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Poco REQUIRED COMPONENTS Net Util JSON)
find_package(roaring REQUIRED)
find_package(spdlog REQUIRED)
find_package(TBB REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(zstd REQUIRED)

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include_directories(SYSTEM
        ${CMAKE_SOURCE_DIR}/include
        ${roaring_INCLUDE_DIRS}
)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

file(GLOB_RECURSE SRC_TEST "src/*.test.cpp")

file(GLOB_RECURSE SRC_SILO_LIB "src/silo/*.cpp")
list(REMOVE_ITEM SRC_SILO_LIB ${SRC_TEST})

file(GLOB_RECURSE SRC_SILO_API "src/silo_api/*.cpp")
list(REMOVE_ITEM SRC_SILO_API ${SRC_TEST})

# ---------------------------------------------------------------------------
# Linter
# ---------------------------------------------------------------------------

option(BUILD_WITH_CLANG_TIDY "Build process clang-tidy" ON)
if (NOT CMAKE_BUILD_TYPE STREQUAL Release AND BUILD_WITH_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-17)
    if (NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy-17 not found.")
    else ()
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        message(STATUS "run clang tidy with: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif ()
endif ()

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

add_library(silo ${SRC_SILO_LIB})
target_link_libraries(
        silo
        PUBLIC
        ${Boost_LIBRARIES}
        ${duckdb_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${roaring_LIBRARIES}
        ${spdlog_LIBRARIES}
        TBB::tbb
        ${yaml-cpp_LIBRARIES}
        zstd::libzstd_static
)

add_executable(siloApi src/silo_api/api.cpp ${SRC_SILO_API})
target_link_libraries(siloApi PUBLIC silo Poco::Net Poco::Util Poco::JSON nlohmann_json::nlohmann_json ${spdlog_LIBRARIES})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

set(SRC_SILO_API_WITHOUT_MAIN ${SRC_SILO_API})
list(REMOVE_ITEM SRC_SILO_API_WITHOUT_MAIN "${CMAKE_SOURCE_DIR}/src/silo_api/api.cpp")

add_executable(silo_test ${SRC_TEST} ${SRC_SILO_API_WITHOUT_MAIN})

if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()
target_link_libraries(silo_test ${GTest_LIBRARIES} silo Poco::Net Poco::Util Poco::JSON nlohmann_json::nlohmann_json)
