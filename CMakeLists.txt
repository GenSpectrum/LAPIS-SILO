cmake_minimum_required(VERSION 3.22)
project(CoronaQueries)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(Boost REQUIRED COMPONENTS system iostreams serialization )

include_directories(
        ${Boost_INCLUDE_DIRS}
)

add_executable(silo src/main.cpp ${Boost_INCLUDE_DIRS})
target_link_libraries(silo PUBLIC ${Boost_LIBRARIES})
