cmake_minimum_required(VERSION 3.22)
project(SILO)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -static-libstdc++ -static-libgcc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/")

set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Boost REQUIRED COMPONENTS system serialization iostreams)
find_package(Poco REQUIRED COMPONENTS Net Util JSON)
find_package(LibLZMA REQUIRED)
find_package(TBB REQUIRED)
find_package(RapidJSON REQUIRED INTERFACE)
find_package(nlohmann_json REQUIRED)

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include_directories(SYSTEM
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${LIBLZMA_INCLUDE_DIRS}
        ${RapidJSON_INCLUDE_DIRS}
        ${readline_INCLUDE_DIRS}
        )

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

file(GLOB SRC_TEST "src/*/*.test.cpp", "src/*.test.cpp")

set(SRC_CC
        src/storage/Dictionary.cpp
        src/storage/meta_store.cpp
        src/storage/sequence_store.cpp
        src/query_engine/query_engine.cpp
        src/query_engine/query_simplification.cpp
        src/query_engine/query_engine_action.cpp
        src/query_engine/QueryParseException.cpp
        src/database.cpp
        src/prepare_dataset.cpp
        src/roaring/roaring.c
        src/preprocessing/preprocessing_config.cpp
        src/preprocessing/preprocessing_exception.cpp
        src/common/InputStreamWrapper.cpp)

file(GLOB SRC_API_CC "src/silo_api/*")
list(REMOVE_ITEM SRC_API_CC ${SRC_TEST})

add_library(silo ${SRC_CC})
target_link_libraries(silo PUBLIC ${Boost_LIBRARIES} TBB::tbb)

add_executable(siloApi src/api.cpp ${SRC_API_CC})
target_link_libraries(siloApi PUBLIC silo Poco::Net Poco::Util Poco::JSON nlohmann_json::nlohmann_json)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_executable(silo_test ${SRC_TEST} ${SRC_API_CC})

if(NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif()
target_link_libraries(silo_test ${GTest_LIBRARIES} silo Poco::Net Poco::Util Poco::JSON nlohmann_json::nlohmann_json)