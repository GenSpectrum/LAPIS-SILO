cmake_minimum_required(VERSION 3.22)
project(SILO_WASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/generators")

# Conan lays out the CMakeDeps config files directly in the build folder for the
# wasm build, so point each package there. Package names must match the exact
# case passed to find_package()/find_dependency() (CMake derives the `<Pkg>_DIR`
# variable name from that case).
foreach(package IN ITEMS
        absl Arrow Boost BZip2 FastFloat fmt Libevent nlohmann_json OpenSSL
        re2 roaring simdjson simdutf spdlog Thrift yaml-cpp ZLIB zstd)
   set(${package}_DIR "${CMAKE_BINARY_DIR}")
endforeach()

# Shared C++ standard, common compile definitions, core dependencies and source
# collection (shared with the native build).
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/silo_core.cmake)

add_compile_definitions(SILO_DEBUG_ASSERTIONS=0)
add_compile_definitions(NO_EVOBENCH)

# Collect the SILO core sources, excluding the CLI entry point, HTTP API, append
# mode, and evobench, which are not part of the browser target.
silo_collect_sources(SILO_CORE_SOURCES
    "${SILO_ROOT_DIR}/src/main.cpp"
    "${SILO_ROOT_DIR}/src/silo/api/*.cpp"
    "${SILO_ROOT_DIR}/src/silo/append/append.cpp"
    "${SILO_ROOT_DIR}/src/silo/config/append_config.cpp"
    "${SILO_ROOT_DIR}/src/silo/config/runtime_config.cpp"
    "${SILO_ROOT_DIR}/src/evobench/evobench.cpp"
)

add_executable(silo_wasm
    src/silo_wasm.cpp
    ${SILO_CORE_SOURCES}
)

target_include_directories(silo_wasm PRIVATE "${SILO_ROOT_DIR}/src")

target_compile_definitions(silo_wasm PRIVATE SILO_WASM)

target_compile_options(silo_wasm PRIVATE -fexceptions -pthread)

target_link_libraries(silo_wasm
    PRIVATE
    ${SILO_CORE_LINK_LIBRARIES}
)

target_link_options(silo_wasm
    PRIVATE
    -pthread
    -fexceptions
    -lembind
    -sALLOW_MEMORY_GROWTH=1
    -sEXIT_RUNTIME=0
    -sEXPORT_ES6=1
    -sEXPORT_NAME=createSiloModule
    -sEXPORTED_RUNTIME_METHODS=FS,getExceptionMessage
    -sFORCE_FILESYSTEM=1
    -sMODULARIZE=1
    -sPTHREAD_POOL_SIZE=4
    -sUSE_PTHREADS=1
    -sENVIRONMENT=web,worker,node
)
