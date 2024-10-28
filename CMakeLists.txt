cmake_minimum_required(VERSION 3.22)
project(SILO)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(spdlog REQUIRED)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

file(GLOB_RECURSE SRC_SILO "src/*.cpp")

set(SRC_SILO_WITHOUT_MAIN ${SRC_SILO})
list(REMOVE_ITEM SRC_SILO_WITHOUT_MAIN "${CMAKE_SOURCE_DIR}/src/silo_api/api.cpp")

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

add_executable(siloApi "${CMAKE_SOURCE_DIR}/src/silo_api/api.cpp" ${SRC_SILO_WITHOUT_MAIN})
target_link_libraries(
	siloApi
	PUBLIC
	${spdlog_LIBRARIES}
)

