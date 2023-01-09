cmake_minimum_required(VERSION 3.22)
project(SILO)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")

find_package(Boost REQUIRED COMPONENTS system iostreams serialization)
find_package(LibLZMA REQUIRED)
find_package(TBB REQUIRED)


# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/include/local.cmake")

include_directories(SYSTEM
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${LIBLZMA_INCLUDE_DIRS}
        ${RAPIDJSON_INCLUDE_DIR}
        )

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------


set(SRC_CC
        src/silo.cpp
        src/db_components/Dictionary.cpp
        src/db_components/meta_store.cpp
        src/db_components/sequence_store.cpp
        src/query_engine.cpp
        src/query_engine_action.cpp
        src/database.cpp
        src/prepare_dataset.cpp
        src/benchmark.cpp
        src/roaring/roaring.c)


add_library(siloapi ${SRC_CC} ${Boost_INCLUDE_DIRS})
target_link_libraries(siloapi PUBLIC rapidjson readline ${Boost_LIBRARIES} TBB::tbb)

add_executable(silo src/main.cpp)
target_link_libraries(silo PUBLIC siloapi)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


enable_testing()
add_test(
        NAME build_meta_runs COMMAND silo "build_meta ${PROJECT_SOURCE_DIR}/Data/metadata.50k.tsv" exit
)
add_test(
        NAME sequence_raw COMMAND silo "build_raw ${PROJECT_SOURCE_DIR}/Data/aligned.50k.fasta" exit
)
add_test(
        NAME build_save_load_meta COMMAND silo "build_meta ../Data/metadata.50k.tsv" save_meta load_meta exit
)
add_test(
        NAME resolve_alias COMMAND mytest resolve_alias
)

add_test(
        NAME build_both COMMAND silo "build_meta ${PROJECT_SOURCE_DIR}/Data/metadata.50k.tsv"
        "build ${PROJECT_SOURCE_DIR}/Data/aligned.50k.fasta" exit
)

add_executable(mytest test/test.cpp)
target_link_libraries(mytest PUBLIC siloapi)


include("${CMAKE_SOURCE_DIR}/test/local.cmake")
