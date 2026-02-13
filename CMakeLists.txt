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

# Export compile commands for configuring language servers or debugging the build
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Allocator
# ---------------------------------------------------------------------------

option(USE_MIMALLOC "Use mimalloc allocator" ON)

# Only use mimalloc in Release mode, because of conflicts with ASAN
# And only on Linux, because Mac has some specialties that occur
# at program startup before mimalloc is loaded, which can lead to
# mixing of allocations and thus Segmentation Faults
if(USE_MIMALLOC AND ${CMAKE_BUILD_TYPE} STREQUAL "Release" AND LINUX)
    message("Using mimalloc as allocator")
    find_package(mimalloc REQUIRED)
    set(MIMALLOC_LIB mimalloc-static)
else()
    set(MIMALLOC_LIB "")
endif()


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
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-19 clang-tidy-20)
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
set_target_properties(silolib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(
        silolib
        PUBLIC
        Arrow::arrow_static
        ArrowAcero::arrow_acero_static
        ${Boost_LIBRARIES}
        ${duckdb_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${roaring_LIBRARIES}
        ${spdlog_LIBRARIES}
        ${simdjson_LIBRARIES}
        ${yaml-cpp_LIBRARIES}
        zstd::libzstd_static
        Poco::Net
        Poco::Util
        Poco::JSON
        re2::re2
)

add_executable(silo "${CMAKE_SOURCE_DIR}/src/main.cpp")
target_link_libraries(
        silo
        PUBLIC
        ${MIMALLOC_LIB} # this should be first
        silolib
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_executable(silo_test ${SRC_TEST})
if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()
target_link_libraries(silo_test PUBLIC ${GTest_LIBRARIES} silolib)

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------

add_subdirectory(performance)

# ---------------------------------------------------------------------------
# Python Bindings
# ---------------------------------------------------------------------------

option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

    # Find Cython
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import Cython; print(Cython.__version__)"
        OUTPUT_VARIABLE CYTHON_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE CYTHON_NOT_FOUND
    )

    if(CYTHON_NOT_FOUND)
        message(FATAL_ERROR "Cython not found. Install with: pip install Cython")
    endif()

    message(STATUS "Found Cython version: ${CYTHON_VERSION}")

    # Cython modules to build
    set(CYTHON_MODULES database)

    # Build each Cython module
    foreach(module ${CYTHON_MODULES})
        set(pyx_file "${CMAKE_SOURCE_DIR}/python/pysilo/${module}.pyx")
        set(cpp_file "${CMAKE_BINARY_DIR}/python/pysilo/${module}.cpp")

        # Run Cython to generate C++ file
        add_custom_command(
            OUTPUT ${cpp_file}
            COMMAND ${Python3_EXECUTABLE} -m cython
                --cplus
                -3
                -I "${CMAKE_SOURCE_DIR}/python/pysilo"
                -o ${cpp_file}
                ${pyx_file}
            DEPENDS ${pyx_file}
            COMMENT "Cythonizing ${module}.pyx"
        )

        # Create Python extension module
        Python3_add_library(${module} MODULE ${cpp_file})

        # Set include directories
        target_include_directories(${module} PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/python/pysilo
        )

        # Link against silolib
        target_link_libraries(${module} PRIVATE silolib)

        # Set output name and location
        set_target_properties(${module} PROPERTIES
            PREFIX ""
            OUTPUT_NAME "${module}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python/pysilo"
        )

        # Install to Python package directory
        # Note: CMAKE_INSTALL_PREFIX is set by setup.py to point to the pysilo directory
        install(TARGETS ${module}
            LIBRARY DESTINATION .
        )
    endforeach()

    # Install Python package files
    install(FILES "${CMAKE_SOURCE_DIR}/python/pysilo/__init__.py"
        DESTINATION .
    )
endif()
