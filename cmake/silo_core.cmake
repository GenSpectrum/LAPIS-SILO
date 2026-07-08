# Shared configuration for the native and the WebAssembly builds of SILO. This holds the parts that
# would otherwise drift between the two build entry points: the C++ standard,
# common compile definitions, the source file globbing, and the core set of
# dependencies and link libraries.
#
# Toolchain-specific and target-specific setup (allocator, HTTP API, tests,
# benchmarks, python bindings, Emscripten link options, ...) stays in the
# respective CMakeLists.txt.

get_filename_component(SILO_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for configuring language servers or debugging the build
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Common compile definitions
# ---------------------------------------------------------------------------

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
add_compile_definitions(SIMDJSON_EXCEPTIONS=0)

# ---------------------------------------------------------------------------
# Core dependencies (shared by native and wasm builds)
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

set(SILO_CORE_LINK_LIBRARIES
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

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

# Collects all SILO source files (excluding test files) into the variable named
# by out_var in the caller's scope. Additional exclude patterns (relative to the
# repository root, e.g. "src/main.cpp") can be passed as extra arguments; each is
# expanded with GLOB_RECURSE and removed from the result.
function(silo_collect_sources out_var)
    file(GLOB_RECURSE core_sources CONFIGURE_DEPENDS "${SILO_ROOT_DIR}/src/*.cpp")

    set(exclude_patterns "${SILO_ROOT_DIR}/src/*.test.cpp" ${ARGN})
    foreach(pattern ${exclude_patterns})
        file(GLOB_RECURSE matching_sources CONFIGURE_DEPENDS "${pattern}")
        if(matching_sources)
            list(REMOVE_ITEM core_sources ${matching_sources})
        endif()
    endforeach()

    set(${out_var} ${core_sources} PARENT_SCOPE)
endfunction()
