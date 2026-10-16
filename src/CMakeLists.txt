# Core RhyDB library (rhydblib). Consumed by several targets.
# Should be agnostic to those targets.

# ---------------------------------------------------------------------------
# rhydblib dependencies
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

if(ENABLE_LZMA)
    find_package(LibLZMA REQUIRED)
endif()

# ---------------------------------------------------------------------------
# rhydblib
# ---------------------------------------------------------------------------

add_library(rhydblib_deps INTERFACE)

target_link_libraries(rhydblib_deps INTERFACE
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

target_include_directories(rhydblib_deps INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(rhydblib_deps INTERFACE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
        SIMDJSON_EXCEPTIONS=0
        # Read in rhydb/common/panic.h, so consumers need it too
        RHYDB_DEBUG_ASSERTIONS=$<IF:$<CONFIG:Debug>,1,0>
)

file(GLOB_RECURSE RHYDBLIB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE RHYDB_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")
list(REMOVE_ITEM RHYDBLIB_SOURCES ${RHYDB_TEST_SOURCES})

add_library(rhydblib OBJECT ${RHYDBLIB_SOURCES})
set_target_properties(rhydblib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(rhydblib PUBLIC rhydblib_deps)
# Only read in .cpp files
target_compile_definitions(rhydblib PRIVATE RHYDB_WITH_LZMA=$<BOOL:${ENABLE_LZMA}>)
if(NOT ENABLE_EVOBENCH)
    # Read in evobench.hpp, so consumers need it too
    target_compile_definitions(rhydblib PUBLIC NO_EVOBENCH)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(BUILD_UNIT_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(rhydb_test ${RHYDB_TEST_SOURCES})
    target_include_directories(rhydb_test PRIVATE ${GTest_INCLUDE_DIRS})
    if (NOT GTest_LIBRARIES)
        set(GTest_LIBRARIES gtest gmock)
    endif ()
    target_link_libraries(rhydb_test PUBLIC ${GTest_LIBRARIES} rhydblib)
endif()
