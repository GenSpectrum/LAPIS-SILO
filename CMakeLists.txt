cmake_minimum_required(VERSION 3.22)
project(SILO)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(Boost_DEBUG ON)
find_package(Boost COMPONENTS ALL)
find_package(Boost REQUIRED COMPONENTS system iostreams serialization)
find_package(LibLZMA)



# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/include/local.cmake")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${LIBLZMA_INCLUDE_DIRS}
)


# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------


set(SRC_CC src/silo.cpp src/meta_store.cpp src/sequence_store.cpp
        src/query_engine.cpp src/roaring/roaring.c)

add_library(silolib STATIC ${SRC_CC} ${INCLUDE_H})

add_executable(silo src/main.cpp ${SRC_CC} ${Boost_INCLUDE_DIRS})

target_link_libraries(silo PUBLIC silolib ${Boost_LIBRARIES})
